/**************************************************************************/
/*  nav_map_debug_3d.cpp                                                  */
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

#include "nav_map_debug_3d.h"

#include "../nav_agent_3d.h"
#include "../nav_link_3d.h"
#include "../nav_map_3d.h"
#include "../nav_obstacle_3d.h"
#include "../nav_region_3d.h"

#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"
#include "scene/main/window.h"
#include "scene/resources/3d/world_3d.h"
#include "servers/navigation/navigation_debug_3d.h"
#include "servers/navigation_server_3d.h"
#include "servers/rendering_server.h"

void NavMapDebug3D::debug_set_canvas(RID p_canvas) {
	if (debug_canvas == p_canvas) {
		return;
	}
	debug_canvas = p_canvas;
	//debug_scenario_dirty = true;
	debug_update_canvas();
}

RID NavMapDebug3D::debug_get_canvas() const {
	return debug_canvas;
}

void NavMapDebug3D::debug_update_canvas() {
	debug_canvas_dirty = true;
}

void NavMapDebug3D::_debug_update_canvas() {
	if (!debug_canvas_dirty) {
		return;
	}
	debug_canvas_dirty = false;
}

void NavMapDebug3D::debug_set_scenario(RID p_scenario) {
	if (debug_scenario == p_scenario) {
		return;
	}
	debug_scenario = p_scenario;
	//debug_scenario_dirty = true;
	debug_update_scenario();
}

RID NavMapDebug3D::debug_get_scenario() const {
	if (!map->is_active()) {
		return RID();
	}

	if (debug_scenario.is_valid()) {
		return debug_scenario;
	}

	// Attempt fallback to root viewport scenario.
	Viewport *viewport = Object::cast_to<Viewport>(SceneTree::get_singleton()->get_root());
	if (viewport == nullptr) {
		return RID();
	}

	Ref<World3D> world_3d = viewport->get_world_3d();
	if (!world_3d.is_valid()) {
		return RID();
	}

	return world_3d->get_scenario();
}

void NavMapDebug3D::debug_update_scenario() {
	debug_scenario_dirty = true;
}

void NavMapDebug3D::_debug_update_scenario() {
	if (debug_get_scenario().is_valid()) {
		RenderingServer::get_singleton()->instance_set_scenario(debug_instance_rid, debug_get_scenario());
	} else {
		RenderingServer::get_singleton()->instance_set_scenario(debug_instance_rid, RID());
	}

	if (!debug_scenario_dirty) {
		return;
	}
	debug_scenario_dirty = false;

	for (NavRegion3D *region : map->regions) {
		region->get_debug()->debug_update_scenario();
	}
	for (NavLink3D *link : map->links) {
		link->get_debug()->debug_update_scenario();
	}
	for (NavObstacle3D *obstacle : map->obstacles) {
		obstacle->get_debug()->debug_update_scenario();
	}
}

void NavMapDebug3D::debug_update_mesh() {
	debug_mesh_dirty = true;
}

void NavMapDebug3D::_debug_update_mesh() {
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

	if (!debug_enabled || !NavigationDebug3D::debug_global_are_maps_enabled()) {
		return;
	}

	if (!map->get_use_edge_connections()) {
		return;
	}

	if (!NavigationDebug3D::get_navmesh_edge_connections_enabled()) {
		return;
	}

	int connections_count = 0;

	for (NavRegion3D *region : map->regions) {
		if (!region->get_use_edge_connections()) {
			continue;
		}
		connections_count += map->get_region_connections_count(region);
	}

	if (connections_count == 0) {
		return;
	}

	float half_edge_connection_margin = map->get_edge_connection_margin() * 0.5;

	Vector<Vector3> debug_mesh_array_vertex;
	debug_mesh_array_vertex.resize(connections_count * 6);
	Vector3 *vertex_array_ptrw = debug_mesh_array_vertex.ptrw();
	int vertex_array_index = 0;

	for (NavRegion3D *region : map->regions) {
		if (!region->get_use_edge_connections()) {
			continue;
		}

		int region_connections_count = map->get_region_connections_count(region);

		for (int i = 0; i < region_connections_count; i++) {
			const Vector3 connection_pathway_start = map->get_region_connection_pathway_start(region, i);
			const Vector3 connection_pathway_end = map->get_region_connection_pathway_end(region, i);

			Vector3 direction_start_end = connection_pathway_start.direction_to(connection_pathway_end);
			Vector3 direction_end_start = connection_pathway_end.direction_to(connection_pathway_start);

			Vector3 start_right_dir = direction_start_end.cross(Vector3(0, 1, 0));
			Vector3 start_left_dir = -start_right_dir;

			Vector3 end_right_dir = direction_end_start.cross(Vector3(0, 1, 0));
			Vector3 end_left_dir = -end_right_dir;

			Vector3 left_start_pos = connection_pathway_start + (start_left_dir * half_edge_connection_margin);
			Vector3 right_start_pos = connection_pathway_start + (start_right_dir * half_edge_connection_margin);
			Vector3 left_end_pos = connection_pathway_end + (end_right_dir * half_edge_connection_margin);
			Vector3 right_end_pos = connection_pathway_end + (end_left_dir * half_edge_connection_margin);

			vertex_array_ptrw[vertex_array_index++] = connection_pathway_start;
			vertex_array_ptrw[vertex_array_index++] = connection_pathway_end;
			vertex_array_ptrw[vertex_array_index++] = left_start_pos;
			vertex_array_ptrw[vertex_array_index++] = right_start_pos;
			vertex_array_ptrw[vertex_array_index++] = left_end_pos;
			vertex_array_ptrw[vertex_array_index++] = right_end_pos;
		}
	}

	if (debug_mesh_array_vertex.is_empty()) {
		return;
	}

	Array mesh_array;
	mesh_array.resize(RS::ARRAY_MAX);
	mesh_array[RS::ARRAY_VERTEX] = debug_mesh_array_vertex;

	rs->mesh_clear(debug_mesh_rid);
	rs->mesh_add_surface_from_arrays(debug_mesh_rid, RS::PRIMITIVE_LINES, mesh_array);

	Ref<StandardMaterial3D> edge_connections_material = NavigationDebug3D::get_navmesh_edge_connections_material();
	rs->mesh_surface_set_material(debug_mesh_rid, 0, edge_connections_material->get_rid());
	rs->instance_set_base(debug_instance_rid, debug_mesh_rid);
	rs->instance_set_scenario(debug_instance_rid, debug_get_scenario());

	debug_material_dirty = true;
	debug_update_material();
	debug_scenario_dirty = true;
	debug_update_scenario();
}

void NavMapDebug3D::debug_update_material() {
	debug_material_dirty = true;
}

void NavMapDebug3D::_debug_update_material() {
	if (!debug_material_dirty) {
		return;
	}
	debug_material_dirty = false;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);
	NavigationServer3D *ns3d = NavigationServer3D::get_singleton();
	ERR_FAIL_NULL(ns3d);

	int surface_count = rs->mesh_get_surface_count(debug_mesh_rid);
	if (surface_count > 0) {
		Ref<StandardMaterial3D> edge_connections_material = NavigationDebug3D::get_navmesh_edge_connections_material();
		rs->mesh_surface_set_material(debug_mesh_rid, 0, edge_connections_material->get_rid());
	}
}

void NavMapDebug3D::debug_update() {
	if (!map->is_active()) {
		if (debug_scenario.is_valid()) {
			debug_scenario_dirty = true;
			debug_update_scenario();
		}
		return;
	}

	debug_update_scenario();
	debug_update_mesh();
	debug_update_material();
}

void NavMapDebug3D::debug_make_dirty_all() {
	debug_mesh_dirty = true;
	debug_material_dirty = true;

	for (NavRegion3D *region : map->regions) {
		region->get_debug()->debug_make_dirty();
	}
	for (NavLink3D *link : map->links) {
		link->get_debug()->debug_make_dirty();
	}
	for (NavAgent3D *agent : map->agents) {
		agent->get_debug()->debug_make_dirty();
	}
	for (NavObstacle3D *obstacle : map->obstacles) {
		obstacle->get_debug()->debug_make_dirty();
	}
}

void NavMapDebug3D::debug_set_enabled(bool p_enabled) {
	if (debug_enabled == p_enabled) {
		return;
	}

	debug_enabled = p_enabled;

	debug_make_dirty_all();
	debug_update_mesh();
}

void NavMapDebug3D::debug_set_navigation_enabled(bool p_enabled) {
	for (NavRegion3D *region : map->regions) {
		region->get_debug()->debug_make_dirty();
	}
	for (NavLink3D *link : map->links) {
		link->get_debug()->debug_make_dirty();
	}
	for (NavAgent3D *agent : map->agents) {
		agent->get_debug()->debug_make_dirty();
	}
	//for (NavObstacle3D *obstacle : obstacles) {
	//	obstacle->debug_make_dirty();
	//}

	debug_mesh_dirty = true;
	debug_update_mesh();
}

void NavMapDebug3D::debug_set_avoidance_enabled(bool p_enabled) {
	//for (NavRegion3D *region : regions) {
	//	region->get_debug()->debug_make_dirty();
	//}
	//for (NavLink3D *link : links) {
	//	link->get_debug()->debug_set_enabled();
	//}
	for (NavAgent3D *agent : map->agents) {
		agent->get_debug()->debug_make_dirty();
	}
	for (NavObstacle3D *obstacle : map->obstacles) {
		obstacle->get_debug()->debug_make_dirty();
	}

	//debug_mesh_dirty = true;
	//debug_update_mesh();
}

void NavMapDebug3D::debug_free() {
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

void NavMapDebug3D::project_settings_changed() {
	project_settings_dirty = true;
}

void NavMapDebug3D::debug_settings_changed() {
	debug_settings_dirty = true;
}

void NavMapDebug3D::add_region_sync_dirty_request(SelfList<NavRegionDebug3D> *p_sync_request) {
	if (p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.regions.add(p_sync_request);
}

void NavMapDebug3D::add_link_sync_dirty_request(SelfList<NavLinkDebug3D> *p_sync_request) {
	if (p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.links.add(p_sync_request);
}

void NavMapDebug3D::add_agent_sync_dirty_request(SelfList<NavAgentDebug3D> *p_sync_request) {
	if (p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.agents.add(p_sync_request);
}

void NavMapDebug3D::add_obstacle_sync_dirty_request(SelfList<NavObstacleDebug3D> *p_sync_request) {
	if (p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.obstacles.add(p_sync_request);
}

void NavMapDebug3D::remove_region_sync_dirty_request(SelfList<NavRegionDebug3D> *p_sync_request) {
	if (!p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.regions.remove(p_sync_request);
}

void NavMapDebug3D::remove_link_sync_dirty_request(SelfList<NavLinkDebug3D> *p_sync_request) {
	if (!p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.links.remove(p_sync_request);
}

void NavMapDebug3D::remove_agent_sync_dirty_request(SelfList<NavAgentDebug3D> *p_sync_request) {
	if (!p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.agents.remove(p_sync_request);
}

void NavMapDebug3D::remove_obstacle_sync_dirty_request(SelfList<NavObstacleDebug3D> *p_sync_request) {
	if (!p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.obstacles.remove(p_sync_request);
}

void NavMapDebug3D::sync() {
	if (project_settings_dirty || debug_settings_dirty) {
		debug_settings_dirty = false;
		project_settings_dirty = false;

		for (NavRegion3D *region : map->regions) {
			region->get_debug()->debug_make_dirty();
		}
		for (NavLink3D *link : map->links) {
			link->get_debug()->debug_make_dirty();
		}
		for (NavAgent3D *agent : map->agents) {
			agent->get_debug()->debug_make_dirty();
		}
		for (NavObstacle3D *obstacle : map->obstacles) {
			obstacle->get_debug()->debug_make_dirty();
		}

		debug_make_dirty();
	}

	_debug_update_scenario();
	_debug_update_mesh();
	_debug_update_material();

	_sync_dirty_update_requests();
}

void NavMapDebug3D::_sync_dirty_update_requests() {
	// Sync NavRegions.
	for (SelfList<NavRegionDebug3D> *element = sync_dirty_requests.regions.first(); element; element = element->next()) {
		element->self()->sync();
	}
	sync_dirty_requests.regions.clear();

	// Sync NavLinks.
	for (SelfList<NavLinkDebug3D> *element = sync_dirty_requests.links.first(); element; element = element->next()) {
		element->self()->sync();
	}
	sync_dirty_requests.links.clear();

	// Sync NavAgents.
	for (SelfList<NavAgentDebug3D> *element = sync_dirty_requests.agents.first(); element; element = element->next()) {
		element->self()->sync();
	}
	sync_dirty_requests.agents.clear();

	// Sync NavObstacles.
	for (SelfList<NavObstacleDebug3D> *element = sync_dirty_requests.obstacles.first(); element; element = element->next()) {
		element->self()->sync();
	}
	sync_dirty_requests.obstacles.clear();
}

NavMapDebug3D::NavMapDebug3D(NavMap3D *p_map) {
	ERR_FAIL_NULL(p_map);
	map = p_map;

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

NavMapDebug3D::~NavMapDebug3D() {
	debug_free();
}
