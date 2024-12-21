/**************************************************************************/
/*  nav_region_debug_2d.cpp                                               */
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

#include "nav_region_debug_2d.h"

#include "../nav_region.h"
#include "../nav_map.h"

#include "servers/rendering_server.h"
#include "servers/navigation_server_2d.h"

void NavRegionDebug2D::debug_set_enabled(bool p_enabled) {
	if (debug_enabled  == p_enabled) {
		return;
	}

	debug_enabled = p_enabled;

	debug_mesh_dirty = true;
	debug_update_mesh();
}

void NavRegionDebug2D::debug_update() {
	debug_update_canvas();
	//debug_update_transform();
	//debug_update_mesh();
	//debug_update_material();
}

void NavRegionDebug2D::debug_update_canvas() {
	ERR_FAIL_COND(!debug_canvas_item_rid.is_valid());

	if (region->map && region->map->get_debug_2d()->debug_get_canvas().is_valid()) {
		RenderingServer::get_singleton()->canvas_item_set_parent(debug_canvas_item_rid, region->map->get_debug_2d()->debug_get_canvas());
	} else {
		RenderingServer::get_singleton()->canvas_item_set_parent(debug_canvas_item_rid, RID());
	}
}

void NavRegionDebug2D::debug_update_transform() {
	if (!debug_transform_dirty) {
		return;
	}
	debug_transform_dirty = false;
	ERR_FAIL_COND(!debug_canvas_item_rid.is_valid());

	const Transform3D region_transform = region->transform;
	Vector3 o = region_transform.get_origin();
	Vector3 nx = region_transform.xform(Vector3(1, 0, 0)) - o;
	Vector3 nz = region_transform.xform(Vector3(0, 0, 1)) - o;
	RenderingServer::get_singleton()->canvas_item_set_transform(debug_canvas_item_rid, Transform2D(nx.x, nx.z, nz.x, nz.z, o.x, o.z));
};

void NavRegionDebug2D::debug_update_mesh() {
	if (!debug_mesh_dirty) {
		return;
	}
	debug_mesh_dirty = false;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);
	NavigationServer2D *ns2d = NavigationServer2D::get_singleton();
	ERR_FAIL_NULL(ns2d);

	ERR_FAIL_COND(!debug_canvas_item_rid.is_valid());
	ERR_FAIL_COND(!debug_mesh2d_rid.is_valid());

	rs->canvas_item_clear(debug_canvas_item_rid);
	rs->mesh_clear(debug_mesh2d_rid);

	if (!debug_enabled) {
		return;
	}

	ERR_FAIL_NULL(region->map);

	rs->canvas_item_set_parent(debug_canvas_item_rid, region->map->get_debug_2d()->debug_get_canvas());

	const Vector<Vector3> &navmesh_vertices = region->navmesh_vertices;
	const Vector<Vector<int>> &navmesh_polygons = region->navmesh_polygons;

	const Vector<Vector3> &vertices = navmesh_vertices;
	if (vertices.is_empty()) {
		return;
	}

	int polygon_count = navmesh_polygons.size();
	if (polygon_count == 0) {
		return;
	}

	bool enabled_geometry_face_random_color = ns2d->get_debug_navigation_enable_geometry_face_random_color();
	bool enabled_edge_lines = ns2d->get_debug_navigation_enable_edge_lines();
	Color debug_navigation_geometry_face_color = ns2d->get_debug_navigation_geometry_face_color();

	int vertex_count = 0;
	int line_count = 0;

	for (int i = 0; i < polygon_count; i++) {
		const Vector<int> &polygon = navmesh_polygons[i];
		int polygon_size = polygon.size();
		if (polygon_size < 3) {
			continue;
		}
		line_count += polygon_size * 2;
		vertex_count += (polygon_size - 2) * 3;
	}

	Vector<Vector3> face_vertex_array;
	face_vertex_array.resize(vertex_count);

	Vector<Color> face_color_array;
	if (enabled_geometry_face_random_color) {
		face_color_array.resize(vertex_count);
	}

	Vector<Vector3> line_vertex_array;
	if (enabled_edge_lines) {
		line_vertex_array.resize(line_count);
	}

	RandomPCG rand;
	Color polygon_color = debug_navigation_geometry_face_color;

	int face_vertex_index = 0;
	int line_vertex_index = 0;

	Vector3 *face_vertex_array_ptrw = face_vertex_array.ptrw();
	Color *face_color_array_ptrw = face_color_array.ptrw();
	Vector3 *line_vertex_array_ptrw = line_vertex_array.ptrw();

	for (int polygon_index = 0; polygon_index < polygon_count; polygon_index++) {
		const Vector<int> &polygon_indices = navmesh_polygons[polygon_index];
		int polygon_indices_size = polygon_indices.size();
		if (polygon_indices_size < 3) {
			continue;
		}

		if (enabled_geometry_face_random_color) {
			// Generate the polygon color, slightly randomly modified from the settings one.
			polygon_color.set_hsv(debug_navigation_geometry_face_color.get_h() + rand.random(-1.0, 1.0) * 0.1, debug_navigation_geometry_face_color.get_s(), debug_navigation_geometry_face_color.get_v() + rand.random(-1.0, 1.0) * 0.2);
			polygon_color.a = debug_navigation_geometry_face_color.a;
		}

		for (int polygon_indices_index = 0; polygon_indices_index < polygon_indices_size - 2; polygon_indices_index++) {
			face_vertex_array_ptrw[face_vertex_index] = vertices[polygon_indices[0]];
			face_vertex_array_ptrw[face_vertex_index + 1] = vertices[polygon_indices[polygon_indices_index + 1]];
			face_vertex_array_ptrw[face_vertex_index + 2] = vertices[polygon_indices[polygon_indices_index + 2]];
			if (enabled_geometry_face_random_color) {
				face_color_array_ptrw[face_vertex_index] = polygon_color;
				face_color_array_ptrw[face_vertex_index + 1] = polygon_color;
				face_color_array_ptrw[face_vertex_index + 2] = polygon_color;
			}
			face_vertex_index += 3;
		}

		if (enabled_edge_lines) {
			for (int polygon_indices_index = 0; polygon_indices_index < polygon_indices_size; polygon_indices_index++) {
				line_vertex_array_ptrw[line_vertex_index] = vertices[polygon_indices[polygon_indices_index]];
				line_vertex_index += 1;
				if (polygon_indices_index + 1 == polygon_indices_size) {
					line_vertex_array_ptrw[line_vertex_index] = vertices[polygon_indices[0]];
					line_vertex_index += 1;
				} else {
					line_vertex_array_ptrw[line_vertex_index] = vertices[polygon_indices[polygon_indices_index + 1]];
					line_vertex_index += 1;
				}
			}
		}
	}

	Color debug_face_color = ns2d->get_debug_navigation_geometry_face_color();
	Color debug_edge_color = ns2d->get_debug_navigation_geometry_edge_color();

	if (!enabled_geometry_face_random_color) {
		face_color_array.resize(face_vertex_array.size());
		face_color_array.fill(debug_face_color);
	}

	Vector<Vector2> face_vertex_array_2d;
	face_vertex_array_2d.resize(face_vertex_array.size());
	for (int i(0); i < face_vertex_array.size(); i++) {
		face_vertex_array_2d.write[i] = Vector2(face_vertex_array[i].x, face_vertex_array[i].z);
	}

	Array face_mesh_array_2d;
	face_mesh_array_2d.resize(RS::ARRAY_MAX);
	face_mesh_array_2d[RS::ARRAY_VERTEX] = face_vertex_array_2d;
	face_mesh_array_2d[RS::ARRAY_COLOR] = face_color_array;

	rs->mesh_add_surface_from_arrays(debug_mesh2d_rid, RS::PRIMITIVE_TRIANGLES, face_mesh_array_2d, Array(), Dictionary(), RS::ARRAY_FLAG_USE_2D_VERTICES);

	if (enabled_edge_lines) {
		Vector<Vector2> line_vertex_array_2d;
		line_vertex_array_2d.resize(line_vertex_array.size());
		for (int i(0); i < line_vertex_array.size(); i++) {
			line_vertex_array_2d.write[i] = Vector2(line_vertex_array[i].x, line_vertex_array[i].z);
		}

		Vector<Color> line_color_array_2d;
		line_color_array_2d.resize(line_vertex_array_2d.size());
		line_color_array_2d.fill(debug_edge_color);

		Array line_mesh_array;
		line_mesh_array.resize(RS::ARRAY_MAX);
		line_mesh_array[RS::ARRAY_VERTEX] = line_vertex_array_2d;
		line_mesh_array[RS::ARRAY_COLOR] = line_color_array_2d;

		rs->mesh_add_surface_from_arrays(debug_mesh2d_rid, RS::PRIMITIVE_LINES, line_mesh_array, Array(), Dictionary(), RS::ARRAY_FLAG_USE_2D_VERTICES);
	}

	rs->canvas_item_add_mesh(debug_canvas_item_rid, debug_mesh2d_rid, Transform2D());

	debug_material_dirty = true;
	debug_update_material();
	debug_update_canvas();
};

void NavRegionDebug2D::debug_update_material() {
	if (!debug_material_dirty) {
		return;
	}
	debug_material_dirty = false;
};

void NavRegionDebug2D::debug_free() {
	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	if (debug_canvas_item_rid.is_valid()) {
		rs->free(debug_canvas_item_rid);
		debug_canvas_item_rid = RID();
	}
	if (debug_mesh2d_rid.is_valid()) {
		rs->free(debug_mesh2d_rid);
		debug_mesh2d_rid = RID();
	}
}

void NavRegionDebug2D::sync() {
	
}

void NavRegionDebug2D::request_sync() {
	if (region->map && !sync_dirty_request_list_element.in_list()) {
		region->map->get_debug_2d()->add_region_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

void NavRegionDebug2D::cancel_sync_request() {
	if (region->map && sync_dirty_request_list_element.in_list()) {
		region->map->get_debug_2d()->remove_region_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

NavRegionDebug2D::NavRegionDebug2D(NavRegion *p_region) :
		sync_dirty_request_list_element(this) {
	ERR_FAIL_NULL(p_region);
	region = p_region;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	debug_canvas_item_rid = rs->canvas_item_create();
	debug_mesh2d_rid = rs->mesh_create();
}

NavRegionDebug2D::~NavRegionDebug2D() {
	cancel_sync_request();
	debug_free();
}
