/**************************************************************************/
/*  nav_obstacle_debug_2d.cpp                                             */
/**************************************************************************/
/*                         This file is part of:                          */
/*                             GODOT ENGINE                               */
/*                        https://godotengine.org                         */
/**************************************************************************/
/* Copyright (c) 2014-present Godot Engine contributors (see AUTHORS.md). */
/* Copyright (c) 2007-2014 Juan Linietsky, Ariel Manzur.                  */
/*                                                                        */
/* Permission is hereby granted, free of charge, to any person obtaining  */
/* a copy of this software and associated documentation files (the        */
/* "Software"), to deal in the Software without restriction, including    */
/* without limitation the rights to use, copy, modify, merge, publish,    */
/* distribute, sublicense, and/or sell copies of the Software, and to     */
/* permit persons to whom the Software is furnished to do so, subject to  */
/* the following conditions:                                              */
/*                                                                        */
/* The above copyright notice and this permission notice shall be         */
/* included in all copies or substantial portions of the Software.        */
/*                                                                        */
/* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,        */
/* EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF     */
/* MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. */
/* IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY   */
/* CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,   */
/* TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE      */
/* SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                 */
/**************************************************************************/

#include "nav_obstacle_debug_2d.h"

#include "../nav_map.h"
#include "../nav_obstacle.h"

#include "core/math/geometry_2d.h"
#include "servers/rendering_server.h"
#include "servers/navigation_server_2d.h"

void NavObstacleDebug2D::debug_set_enabled(bool p_enabled) {
	if (debug_enabled  == p_enabled) {
		return;
	}

	debug_enabled = p_enabled;

	//debug_mesh_dirty = true;
	debug_update_mesh();
}

void NavObstacleDebug2D::debug_update() {
	debug_update_canvas();
	//debug_update_transform();
	//debug_update_mesh();
	//debug_update_material();
}

void NavObstacleDebug2D::debug_update_canvas() {
	debug_canvas_dirty = true;
	if (obstacle->map) {
		request_sync();
	} else {
		_debug_update_canvas();
	}
}

void NavObstacleDebug2D::_debug_update_canvas() {
	ERR_FAIL_COND(!debug_canvas_item_rid.is_valid());

	if (obstacle->map && obstacle->map->get_debug_2d()->debug_get_canvas().is_valid()) {
		RenderingServer::get_singleton()->canvas_item_set_parent(debug_canvas_item_rid, obstacle->map->get_debug_2d()->debug_get_canvas());
	} else {
		RenderingServer::get_singleton()->canvas_item_set_parent(debug_canvas_item_rid, RID());
	}
}

void NavObstacleDebug2D::debug_update_transform() {
	debug_transform_dirty = true;
	request_sync();
}

void NavObstacleDebug2D::_debug_update_transform() {
	if (!debug_transform_dirty) {
		return;
	}
	debug_transform_dirty = false;

	RenderingServer::get_singleton()->canvas_item_set_transform(debug_canvas_item_rid, Transform2D(0.0, Vector2(obstacle->position.x, obstacle->position.z)));
};

void NavObstacleDebug2D::debug_update_mesh() {
	debug_mesh_dirty = true;
	request_sync();
}

void NavObstacleDebug2D::_debug_update_mesh() {
	if (!debug_mesh_dirty) {
		return;
	}
	debug_mesh_dirty = false;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	ERR_FAIL_COND(!debug_canvas_item_rid.is_valid());
	ERR_FAIL_COND(!debug_mesh_rid.is_valid());

	rs->canvas_item_clear(debug_canvas_item_rid);
	rs->mesh_clear(debug_mesh_rid);

	if (!debug_enabled || !obstacle->map) {
		return;
	}

	const real_t obstacle_radius = obstacle->radius;

	int vertex_count = obstacle->vertices.size();

	Vector<Vector2> polygon_2d_vertices;
	polygon_2d_vertices.resize(vertex_count);
	Vector2 *polygon_2d_vertices_ptrw = polygon_2d_vertices.ptrw();

	if (vertex_count > 0) {
		const Vector3 *vertices_ptr = obstacle->vertices.ptr();

		for (int i = 0; i < vertex_count; ++i) {
			const Vector3 &vertex_3d_ground_from = vertices_ptr[i];

			polygon_2d_vertices_ptrw[i] = Vector2(vertex_3d_ground_from.x, vertex_3d_ground_from.z);
		}
	}

	if (vertex_count > 2 && NavigationServer2D::get_singleton()->get_debug_navigation_avoidance_enable_obstacles_static()) {
		bool obstacle_pushes_inward = Geometry2D::is_polygon_clockwise(polygon_2d_vertices);

		Color debug_static_obstacle_face_color;

		if (obstacle_pushes_inward) {
			debug_static_obstacle_face_color = NavigationServer2D::get_singleton()->get_debug_navigation_avoidance_static_obstacle_pushin_face_color();
		} else {
			debug_static_obstacle_face_color = NavigationServer2D::get_singleton()->get_debug_navigation_avoidance_static_obstacle_pushout_face_color();
		}

		Vector<Vector2> debug_obstacle_polygon_vertices = polygon_2d_vertices;

		Vector<Color> debug_obstacle_polygon_colors;
		debug_obstacle_polygon_colors.resize(debug_obstacle_polygon_vertices.size());
		debug_obstacle_polygon_colors.fill(debug_static_obstacle_face_color);

		rs->canvas_item_add_polygon(debug_canvas_item_rid, debug_obstacle_polygon_vertices, debug_obstacle_polygon_colors);

		Color debug_static_obstacle_edge_color;

		if (obstacle_pushes_inward) {
			debug_static_obstacle_edge_color = NavigationServer2D::get_singleton()->get_debug_navigation_avoidance_static_obstacle_pushin_edge_color();
		} else {
			debug_static_obstacle_edge_color = NavigationServer2D::get_singleton()->get_debug_navigation_avoidance_static_obstacle_pushout_edge_color();
		}

		Vector<Vector2> debug_obstacle_line_vertices = polygon_2d_vertices;
		debug_obstacle_line_vertices.push_back(debug_obstacle_line_vertices[0]);
		debug_obstacle_line_vertices.resize(debug_obstacle_line_vertices.size());

		Vector<Color> debug_obstacle_line_colors;
		debug_obstacle_line_colors.resize(debug_obstacle_line_vertices.size());
		debug_obstacle_line_colors.fill(debug_static_obstacle_edge_color);

		rs->canvas_item_add_polyline(debug_canvas_item_rid, debug_obstacle_line_vertices, debug_obstacle_line_colors, 4.0);

		rs->mesh_add_surface_from_arrays(debug_mesh_rid, RS::PRIMITIVE_LINES, face_mesh_array_2d, Array(), Dictionary(), RS::ARRAY_FLAG_USE_2D_VERTICES);
	}

	if (obstacle_radius > 0.0 && NavigationServer2D::get_singleton()->get_debug_navigation_avoidance_enable_obstacles_radius()) {
		Color debug_radius_color = NavigationServer2D::get_singleton()->get_debug_navigation_avoidance_obstacles_radius_color();
		rs->canvas_item_add_circle(debug_canvas_item_rid, Vector2(), obstacle_radius, debug_radius_color);
	}

	debug_material_dirty = true;
	debug_update_material();
	debug_update_canvas();
}

void NavObstacleDebug2D::debug_update_material() {
	debug_material_dirty = true;
	request_sync();
}

void NavObstacleDebug2D::_debug_update_material() {
	if (!debug_material_dirty) {
		return;
	}
	debug_material_dirty = false;
}

void NavObstacleDebug2D::debug_make_dirty() {
	debug_canvas_dirty = true;
	debug_transform_dirty = true;
	debug_mesh_dirty = true;
	debug_material_dirty = true;
};

void NavObstacleDebug2D::debug_free() {
	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	if (debug_canvas_item_rid.is_valid()) {
		rs->free(debug_canvas_item_rid);
		debug_canvas_item_rid = RID();
	}
}

void NavObstacleDebug2D::sync() {
	_debug_update_canvas();
	_debug_update_transform();
	_debug_update_mesh();
	_debug_update_material();
}

void NavObstacleDebug2D::request_sync() {
	if (obstacle->map && !sync_dirty_request_list_element.in_list()) {
		obstacle->map->get_debug_2d()->add_obstacle_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

void NavObstacleDebug2D::cancel_sync_request() {
	if (obstacle->map && sync_dirty_request_list_element.in_list()) {
		obstacle->map->get_debug_2d()->remove_obstacle_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

NavObstacleDebug2D::NavObstacleDebug2D(NavObstacle *p_obstacle) :
		sync_dirty_request_list_element(this) {
	ERR_FAIL_NULL(p_obstacle);
	obstacle = p_obstacle;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	debug_canvas_item_rid = rs->canvas_item_create();
}

NavObstacleDebug2D::~NavObstacleDebug2D() {
	cancel_sync_request();
	debug_free();
}
