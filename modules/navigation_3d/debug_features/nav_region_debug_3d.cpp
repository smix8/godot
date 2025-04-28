/**************************************************************************/
/*  nav_region_debug_3d.cpp                                               */
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

#include "nav_region_debug_3d.h"

#include "../nav_map_3d.h"
#include "../nav_region_3d.h"

#include "core/math/random_pcg.h"
#include "servers/navigation/navigation_debug_3d.h"
#include "servers/navigation_server_3d.h"
#include "servers/rendering_server.h"

void NavRegionDebug3D::debug_set_enabled(bool p_enabled) {
	if (debug_enabled == p_enabled) {
		return;
	}

	debug_enabled = p_enabled;

	//debug_mesh_dirty = true;
	debug_update_mesh();
}

void NavRegionDebug3D::debug_update() {
	debug_update_scenario();
	//debug_update_transform();
	//debug_update_mesh();
	//debug_update_material();
}

void NavRegionDebug3D::debug_update_scenario() {
	debug_scenario_dirty = true;
	if (region->map) {
		request_sync();
	} else {
		_debug_update_scenario();
	}
}

void NavRegionDebug3D::_debug_update_scenario() {
	ERR_FAIL_COND(!debug_instance_rid.is_valid());

	if (region->map && region->map->get_debug()->debug_get_scenario().is_valid()) {
		RenderingServer::get_singleton()->instance_set_scenario(debug_instance_rid, region->map->get_debug()->debug_get_scenario());
	} else {
		RenderingServer::get_singleton()->instance_set_scenario(debug_instance_rid, RID());
	}
}

void NavRegionDebug3D::debug_update_transform() {
	debug_transform_dirty = true;
	request_sync();
}

void NavRegionDebug3D::_debug_update_transform() {
	if (!debug_transform_dirty) {
		return;
	}
	debug_transform_dirty = false;
	ERR_FAIL_COND(!debug_instance_rid.is_valid());

	const Transform3D region_transform = region->transform;
	RenderingServer::get_singleton()->instance_set_transform(debug_instance_rid, region_transform);
}

void NavRegionDebug3D::debug_update_mesh() {
	debug_mesh_dirty = true;
	request_sync();
}

void NavRegionDebug3D::_debug_update_mesh() {
	if (!debug_mesh_dirty) {
		return;
	}
	debug_mesh_dirty = false;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);
	NavigationServer3D *ns3d = NavigationServer3D::get_singleton();
	ERR_FAIL_NULL(ns3d);

	ERR_FAIL_COND(!debug_mesh_rid.is_valid());
	ERR_FAIL_COND(!debug_instance_rid.is_valid());

	rs->mesh_clear(debug_mesh_rid);

	if (!debug_enabled || !NavigationDebug3D::debug_global_are_regions_enabled() || !region->map || !region->map->is_active() || !region->map->get_debug()->debug_is_enabled()) {
		return;
	}

	Vector<Vector3> navmesh_vertices;
	Vector<Vector<int>> navmesh_polygons;

	if (region->navigation_mesh.is_valid()) {
		region->navmesh_rwlock.read_lock();
		region->navigation_mesh->get_data(navmesh_vertices, navmesh_polygons);
		region->navmesh_rwlock.read_unlock();
	}

	const Vector<Vector3> &vertices = navmesh_vertices;
	if (vertices.is_empty()) {
		return;
	}

	int polygon_count = navmesh_polygons.size();
	if (polygon_count == 0) {
		return;
	}

	bool enabled_geometry_face_random_color = NavigationDebug3D::get_navmesh_geometry_face_random_color_enabled();
	bool enabled_edge_lines = NavigationDebug3D::get_navmesh_edge_lines_enabled();
	Color geometry_face_color = NavigationDebug3D::get_navmesh_geometry_face_color();

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
	Color polygon_color = geometry_face_color;

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
			polygon_color.set_hsv(geometry_face_color.get_h() + rand.random(-1.0, 1.0) * 0.1, geometry_face_color.get_s(), geometry_face_color.get_v() + rand.random(-1.0, 1.0) * 0.2);
			polygon_color.a = geometry_face_color.a;
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

	Array face_mesh_array;
	face_mesh_array.resize(RS::ARRAY_MAX);
	face_mesh_array[RS::ARRAY_VERTEX] = face_vertex_array;
	if (enabled_geometry_face_random_color) {
		face_mesh_array[RS::ARRAY_COLOR] = face_color_array;
	}
	rs->mesh_add_surface_from_arrays(debug_mesh_rid, RS::PRIMITIVE_TRIANGLES, face_mesh_array);

	if (enabled_edge_lines) {
		Array line_mesh_array;
		line_mesh_array.resize(RS::ARRAY_MAX);
		line_mesh_array[RS::ARRAY_VERTEX] = line_vertex_array;
		rs->mesh_add_surface_from_arrays(debug_mesh_rid, RS::PRIMITIVE_LINES, line_mesh_array);
	}

	debug_material_dirty = true;
	_debug_update_material();
	debug_scenario_dirty = true;
	_debug_update_scenario();
}

void NavRegionDebug3D::debug_update_material() {
	debug_material_dirty = true;
	request_sync();
}

void NavRegionDebug3D::_debug_update_material() {
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
		if (region->enabled) {
			Ref<StandardMaterial3D> face_material = NavigationDebug3D::get_navmesh_geometry_face_material();
			rs->instance_set_surface_override_material(debug_instance_rid, 0, face_material->get_rid());
		} else {
			Ref<StandardMaterial3D> disabled_face_material = NavigationDebug3D::get_navmesh_geometry_face_disabled_material();
			rs->instance_set_surface_override_material(debug_instance_rid, 0, disabled_face_material->get_rid());
		}
	}

	bool enabled_edge_lines = NavigationDebug3D::get_navmesh_edge_connections_enabled();
	if (enabled_edge_lines) {
		if (surface_count > 1) {
			if (region->enabled) {
				Ref<StandardMaterial3D> line_material = NavigationDebug3D::get_navmesh_geometry_edge_material();
				rs->instance_set_surface_override_material(debug_instance_rid, 1, line_material->get_rid());
			} else {
				Ref<StandardMaterial3D> disabled_line_material = NavigationDebug3D::get_navmesh_geometry_edge_disabled_material();
				rs->instance_set_surface_override_material(debug_instance_rid, 1, disabled_line_material->get_rid());
			}
		}
	}
}

void NavRegionDebug3D::debug_make_dirty() {
	debug_scenario_dirty = true;
	debug_transform_dirty = true;
	debug_mesh_dirty = true;
	debug_material_dirty = true;
	request_sync();
}

void NavRegionDebug3D::debug_free() {
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
}

void NavRegionDebug3D::sync() {
	_debug_update_scenario();
	_debug_update_transform();
	_debug_update_mesh();
	_debug_update_material();
}

void NavRegionDebug3D::request_sync() {
	if (region->map && !sync_dirty_request_list_element.in_list()) {
		region->map->get_debug()->add_region_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

void NavRegionDebug3D::cancel_sync_request() {
	if (region->map && sync_dirty_request_list_element.in_list()) {
		region->map->get_debug()->remove_region_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

NavRegionDebug3D::NavRegionDebug3D(NavRegion3D *p_region) :
		sync_dirty_request_list_element(this) {
	ERR_FAIL_NULL(p_region);
	region = p_region;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	debug_mesh_rid = rs->mesh_create();
	debug_instance_rid = rs->instance_create();

	rs->instance_set_base(debug_instance_rid, debug_mesh_rid);
	rs->instance_geometry_set_cast_shadows_setting(debug_instance_rid, RS::ShadowCastingSetting::SHADOW_CASTING_SETTING_OFF);
	//rs->instance_set_layer_mask(debug_instance_rid, 1 << 26); // GIZMO_EDIT_LAYER
	rs->instance_geometry_set_flag(debug_instance_rid, RS::INSTANCE_FLAG_IGNORE_OCCLUSION_CULLING, true);
	rs->instance_geometry_set_flag(debug_instance_rid, RS::INSTANCE_FLAG_USE_BAKED_LIGHT, false);
}

NavRegionDebug3D::~NavRegionDebug3D() {
	cancel_sync_request();
	debug_free();
}
