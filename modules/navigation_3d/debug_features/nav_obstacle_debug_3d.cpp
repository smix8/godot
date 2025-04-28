/**************************************************************************/
/*  nav_obstacle_debug_3d.cpp                                             */
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

#include "nav_obstacle_debug_3d.h"

#include "../nav_map_3d.h"
#include "../nav_obstacle_3d.h"

#include "core/math/geometry_2d.h"
#include "servers/navigation/navigation_debug_3d.h"
#include "servers/navigation_server_3d.h"
#include "servers/rendering_server.h"

void NavObstacleDebug3D::debug_set_enabled(bool p_enabled) {
	if (debug_enabled == p_enabled) {
		return;
	}

	debug_enabled = p_enabled;

	//debug_mesh_dirty = true;
	debug_update_mesh();
}

void NavObstacleDebug3D::debug_update() {
	debug_update_scenario();
	//debug_update_transform();
	//debug_update_mesh();
	//debug_update_material();
}

void NavObstacleDebug3D::debug_update_scenario() {
	debug_scenario_dirty = true;
	if (obstacle->map) {
		request_sync();
	} else {
		_debug_update_scenario();
	}
}

void NavObstacleDebug3D::_debug_update_scenario() {
	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	ERR_FAIL_COND(!debug_instance_rid.is_valid());
	ERR_FAIL_COND(!debug_radius_instance_rid.is_valid());

	if (obstacle->map && obstacle->map->get_debug()->debug_get_scenario().is_valid()) {
		rs->instance_set_scenario(debug_instance_rid, obstacle->map->get_debug()->debug_get_scenario());
		rs->instance_set_scenario(debug_radius_instance_rid, obstacle->map->get_debug()->debug_get_scenario());
	} else {
		rs->instance_set_scenario(debug_instance_rid, RID());
		rs->instance_set_scenario(debug_radius_instance_rid, RID());
	}
}

void NavObstacleDebug3D::debug_update_transform() {
	debug_transform_dirty = true;
	request_sync();
}

void NavObstacleDebug3D::_debug_update_transform() {
	if (!debug_transform_dirty) {
		return;
	}
	debug_transform_dirty = false;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	ERR_FAIL_COND(!debug_instance_rid.is_valid());
	ERR_FAIL_COND(!debug_radius_instance_rid.is_valid());

	const Transform3D obstacle_transform = Transform3D(Basis(), obstacle->position);
	rs->instance_set_transform(debug_instance_rid, obstacle_transform);
	rs->instance_set_transform(debug_radius_instance_rid, obstacle_transform);
}

void NavObstacleDebug3D::debug_update_mesh() {
	debug_mesh_dirty = true;
	request_sync();
}

void NavObstacleDebug3D::_debug_update_mesh() {
	if (!debug_mesh_dirty) {
		return;
	}
	debug_mesh_dirty = false;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	ERR_FAIL_COND(!debug_mesh_rid.is_valid());
	ERR_FAIL_COND(!debug_instance_rid.is_valid());

	rs->mesh_clear(debug_mesh_rid);

	ERR_FAIL_COND(!debug_radius_mesh_rid.is_valid());
	ERR_FAIL_COND(!debug_radius_instance_rid.is_valid());

	rs->mesh_clear(debug_radius_mesh_rid);

	if (!debug_enabled || !NavigationDebug3D::debug_global_are_obstacles_enabled() || !obstacle->map || !obstacle->map->is_active() || !obstacle->map->get_debug()->debug_is_enabled()) {
		return;
	}

	const real_t obstacle_radius = obstacle->radius;
	const real_t obstacle_height = obstacle->height;

	int vertex_count = obstacle->vertices.size();

	if (vertex_count > 0 && NavigationDebug3D::get_navobstacle_avoidance_static_enabled()) {
		const Vector3 *vertices_ptr = obstacle->vertices.ptr();

		Vector<Vector3> edge_vertex_array;
		edge_vertex_array.resize(vertex_count * 8);
		Vector3 *edge_vertex_array_ptrw = edge_vertex_array.ptrw();

		int vertex_index = 0;

		for (int i = 0; i < vertex_count; i++) {
			const Vector3 &vertex = vertices_ptr[i];
			const Vector3 &vertex_next = vertices_ptr[(i + 1) % vertex_count];

			Vector3 direction = vertex_next.direction_to(vertex);
			Vector3 arrow_dir = direction.cross(Vector3(0.0, 1.0, 0.0));
			Vector3 edge_middle = vertex + ((vertex_next - vertex) * 0.5);

			edge_vertex_array_ptrw[vertex_index++] = edge_middle;
			edge_vertex_array_ptrw[vertex_index++] = edge_middle + (arrow_dir * 0.5);

			edge_vertex_array_ptrw[vertex_index++] = vertex;
			edge_vertex_array_ptrw[vertex_index++] = vertex_next;

			edge_vertex_array_ptrw[vertex_index++] = Vector3(vertex.x, obstacle_height, vertex.z);
			edge_vertex_array_ptrw[vertex_index++] = Vector3(vertex_next.x, obstacle_height, vertex_next.z);

			edge_vertex_array_ptrw[vertex_index++] = vertex;
			edge_vertex_array_ptrw[vertex_index++] = Vector3(vertex.x, obstacle_height, vertex.z);
		}

		Array edge_mesh_array;
		edge_mesh_array.resize(Mesh::ARRAY_MAX);
		edge_mesh_array[Mesh::ARRAY_VERTEX] = edge_vertex_array;

		rs->mesh_add_surface_from_arrays(debug_mesh_rid, RS::PRIMITIVE_LINES, edge_mesh_array);
	}

	if (obstacle_radius > 0.0 && NavigationDebug3D::get_navobstacle_avoidance_radius_enabled()) {
		Vector<Vector3> face_vertex_array;
		Vector<int> face_indices_array;

		int i, j, prevrow, thisrow, point;
		float x, y, z;

		int rings = 16;
		int radial_segments = 32;

		point = 0;

		thisrow = 0;
		prevrow = 0;
		for (j = 0; j <= (rings + 1); j++) {
			float v = j;
			float w;

			v /= (rings + 1);
			w = sin(Math::PI * v);
			y = (obstacle_radius)*cos(Math::PI * v);

			for (i = 0; i <= radial_segments; i++) {
				float u = i;
				u /= radial_segments;

				x = sin(u * Math::TAU);
				z = cos(u * Math::TAU);

				Vector3 p = Vector3(x * obstacle_radius * w, y, z * obstacle_radius * w);
				face_vertex_array.push_back(p);

				point++;

				if (i > 0 && j > 0) {
					face_indices_array.push_back(prevrow + i - 1);
					face_indices_array.push_back(prevrow + i);
					face_indices_array.push_back(thisrow + i - 1);

					face_indices_array.push_back(prevrow + i);
					face_indices_array.push_back(thisrow + i);
					face_indices_array.push_back(thisrow + i - 1);
				};
			};

			prevrow = thisrow;
			thisrow = point;
		};

		Array face_mesh_array;
		face_mesh_array.resize(Mesh::ARRAY_MAX);
		face_mesh_array[Mesh::ARRAY_VERTEX] = face_vertex_array;
		face_mesh_array[Mesh::ARRAY_INDEX] = face_indices_array;

		rs->mesh_add_surface_from_arrays(debug_radius_mesh_rid, RS::PRIMITIVE_TRIANGLES, face_mesh_array);
	}

	debug_material_dirty = true;
	_debug_update_material();
	debug_scenario_dirty = true;
	_debug_update_scenario();
}

void NavObstacleDebug3D::debug_update_material() {
	debug_material_dirty = true;
	request_sync();
}

void NavObstacleDebug3D::_debug_update_material() {
	if (!debug_material_dirty) {
		return;
	}
	debug_material_dirty = false;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);
	NavigationServer3D *ns3d = NavigationServer3D::get_singleton();
	ERR_FAIL_NULL(ns3d);

	ERR_FAIL_COND(!debug_mesh_rid.is_valid());
	ERR_FAIL_COND(!debug_instance_rid.is_valid());

	int surface_count = rs->mesh_get_surface_count(debug_mesh_rid);
	if (surface_count > 0) {
		Ref<StandardMaterial3D> edge_material = NavigationDebug3D::get_navobstacle_avoidance_static_pushout_edge_material();
		rs->mesh_surface_set_material(debug_mesh_rid, 0, edge_material->get_rid());
	}

	ERR_FAIL_COND(!debug_radius_mesh_rid.is_valid());
	ERR_FAIL_COND(!debug_radius_instance_rid.is_valid());

	int radius_mesh_surface_count = rs->mesh_get_surface_count(debug_radius_mesh_rid);
	if (radius_mesh_surface_count > 0) {
		Ref<StandardMaterial3D> face_material = NavigationDebug3D::get_navobstacle_avoidance_radius_material();
		rs->mesh_surface_set_material(debug_radius_mesh_rid, 0, face_material->get_rid());
	}
}

void NavObstacleDebug3D::debug_make_dirty() {
	debug_scenario_dirty = true;
	debug_transform_dirty = true;
	debug_mesh_dirty = true;
	debug_material_dirty = true;
	request_sync();
}

void NavObstacleDebug3D::debug_free() {
	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	if (debug_mesh_rid.is_valid()) {
		rs->free(debug_mesh_rid);
		debug_mesh_rid = RID();
	}
	if (debug_instance_rid.is_valid()) {
		rs->free(debug_instance_rid);
		debug_instance_rid = RID();
	}

	if (debug_radius_mesh_rid.is_valid()) {
		rs->free(debug_radius_mesh_rid);
		debug_radius_mesh_rid = RID();
	}
	if (debug_radius_instance_rid.is_valid()) {
		rs->free(debug_radius_instance_rid);
		debug_radius_instance_rid = RID();
	}
}

void NavObstacleDebug3D::sync() {
	_debug_update_scenario();
	_debug_update_transform();
	_debug_update_mesh();
	_debug_update_material();
}

void NavObstacleDebug3D::request_sync() {
	if (obstacle->map && !sync_dirty_request_list_element.in_list()) {
		obstacle->map->get_debug()->add_obstacle_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

void NavObstacleDebug3D::cancel_sync_request() {
	if (obstacle->map && sync_dirty_request_list_element.in_list()) {
		obstacle->map->get_debug()->remove_obstacle_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

NavObstacleDebug3D::NavObstacleDebug3D(NavObstacle3D *p_obstacle) :
		sync_dirty_request_list_element(this) {
	ERR_FAIL_NULL(p_obstacle);
	obstacle = p_obstacle;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	debug_mesh_rid = rs->mesh_create();
	debug_instance_rid = rs->instance_create();

	rs->instance_set_base(debug_instance_rid, debug_mesh_rid);
	rs->instance_geometry_set_cast_shadows_setting(debug_instance_rid, RS::ShadowCastingSetting::SHADOW_CASTING_SETTING_OFF);
	//rs->instance_set_layer_mask(debug_instance_rid, 1 << 26); // GIZMO_EDIT_LAYER
	rs->instance_geometry_set_flag(debug_instance_rid, RS::INSTANCE_FLAG_IGNORE_OCCLUSION_CULLING, true);
	rs->instance_geometry_set_flag(debug_instance_rid, RS::INSTANCE_FLAG_USE_BAKED_LIGHT, false);

	debug_radius_mesh_rid = rs->mesh_create();
	debug_radius_instance_rid = rs->instance_create();

	rs->instance_set_base(debug_radius_instance_rid, debug_radius_mesh_rid);
	rs->instance_geometry_set_cast_shadows_setting(debug_radius_instance_rid, RS::ShadowCastingSetting::SHADOW_CASTING_SETTING_OFF);
	//rs->instance_set_layer_mask(debug_radius_instance_rid, 1 << 26); // GIZMO_EDIT_LAYER
	rs->instance_geometry_set_flag(debug_radius_instance_rid, RS::INSTANCE_FLAG_IGNORE_OCCLUSION_CULLING, true);
	rs->instance_geometry_set_flag(debug_radius_instance_rid, RS::INSTANCE_FLAG_USE_BAKED_LIGHT, false);
}

NavObstacleDebug3D::~NavObstacleDebug3D() {
	cancel_sync_request();
	debug_free();
}
