/**************************************************************************/
/*  nav_map_debug_2d.cpp                                                  */
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

#include "nav_map_debug_2d.h"

#include "../nav_map.h"
#include "../nav_link.h"
#include "../nav_region.h"
#include "../nav_obstacle.h"
#include "../nav_agent.h"

#include "servers/rendering_server.h"
#include "scene/main/scene_tree.h"
#include "scene/main/viewport.h"
#include "scene/main/window.h"
#include "scene/resources/world_2d.h"
#include "servers/navigation_server_2d.h"

void NavMapDebug2D::debug_set_canvas(RID p_canvas) {
	if (debug_canvas == p_canvas) {
		return;
	}
	debug_canvas = p_canvas;
	debug_canvas_dirty = true;
	debug_update_canvas();
};

RID NavMapDebug2D::debug_get_canvas() const {
	if (!map->is_active()) {
		return RID();
	}

	if (debug_canvas.is_valid()) {
		return debug_canvas;
	}

	Viewport *viewport = Object::cast_to<Viewport>(SceneTree::get_singleton()->get_root());
	ERR_FAIL_NULL_V(viewport, RID());
	ERR_FAIL_NULL_V(viewport->get_world_2d(), RID());
	if (!viewport->get_world_2d()->get_canvas().is_valid()) {
		return RID();
	}
	return RID();
	return viewport->get_world_2d()->get_canvas();
}

void NavMapDebug2D::debug_update_canvas() {
	if (!map->is_usage_2d()) {
		return;
	}

	if (debug_get_canvas().is_valid()) {
		RenderingServer::get_singleton()->canvas_item_set_parent(debug_canvas_item_rid, debug_get_canvas());
	} else {
		RenderingServer::get_singleton()->canvas_item_set_parent(debug_canvas_item_rid, RID());
	}

	if (!debug_canvas_dirty) {
		return;
	}
	debug_canvas_dirty = false;

	for (NavRegion *region : map->regions) {
		region->get_debug_2d()->debug_update_canvas();
	}
	for (NavLink *link : map->links) {
		link->get_debug_2d()->debug_update_canvas();
	}
	for (NavObstacle *obstacle : map->obstacles) {
		obstacle->get_debug_2d()->debug_update_canvas();
	}
}

void NavMapDebug2D::debug_update_mesh() {
	if (!debug_mesh_dirty) {
		return;
	}
	debug_mesh_dirty = false;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	ERR_FAIL_COND(!debug_canvas_item_rid.is_valid());
	ERR_FAIL_COND(!debug_mesh2d_rid.is_valid());
	rs->canvas_item_clear(debug_canvas_item_rid);
	rs->mesh_clear(debug_mesh2d_rid);

	if (!debug_enabled) {
		return;
	}

	if (!map->get_use_edge_connections()) {
		return;
	}

	int connections_count = 0;

	for (NavRegion *region : map->regions) {
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

	for (NavRegion *region : map->regions) {
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

	NavigationServer2D *ns2d = NavigationServer2D::get_singleton();
	ERR_FAIL_NULL(ns2d);

	bool enable_edge_connections = map->get_use_edge_connections() && ns2d->get_debug_navigation_enable_edge_connections();
	Color debug_edge_connection_color = ns2d->get_debug_navigation_edge_connection_color();

	Vector<Vector2> line_vertex_array_2d;
	line_vertex_array_2d.resize(debug_mesh_array_vertex.size());
	for (int i(0); i < debug_mesh_array_vertex.size(); i++) {
		line_vertex_array_2d.write[i] = Vector2(debug_mesh_array_vertex[i].x, debug_mesh_array_vertex[i].z);
	}

	Vector<Color> line_color_array_2d;
	line_color_array_2d.resize(line_vertex_array_2d.size());
	line_color_array_2d.fill(debug_edge_connection_color);

	Array mesh_array_2d;
	mesh_array_2d.resize(RS::ARRAY_MAX);
	mesh_array_2d[RS::ARRAY_VERTEX] = line_vertex_array_2d;
	mesh_array_2d[RS::ARRAY_COLOR] = line_color_array_2d;

	rs->mesh_add_surface_from_arrays(debug_mesh2d_rid, RS::PRIMITIVE_LINES, mesh_array_2d, Array(), Dictionary(), RS::ARRAY_FLAG_USE_2D_VERTICES);
	rs->canvas_item_add_mesh(debug_canvas_item_rid, debug_mesh2d_rid, Transform2D());

	debug_material_dirty = true;
	debug_update_material();
	debug_update_canvas();
}

void NavMapDebug2D::debug_update_material() {
	if (map->is_usage_2d()) {
		return;
	}

	if (!debug_material_dirty) {
		return;
	}
	debug_material_dirty = false;
}

void NavMapDebug2D::debug_update() {
	if (!map->is_active()) {
		if (debug_canvas.is_valid()) {
			debug_canvas_dirty = true;
			debug_update_canvas();
		}
		return;
	}

	debug_update_canvas();
	debug_update_mesh();
	debug_update_material();
}

void NavMapDebug2D::debug_set_enabled(bool p_enabled) {
	if (debug_enabled == p_enabled) {
		return;
	}

	debug_enabled = p_enabled;

	for (NavRegion *region : map->regions) {
		region->get_debug_2d()->debug_set_enabled(debug_enabled);
	}
	for (NavLink *link : map->links) {
		link->get_debug_2d()->debug_set_enabled(debug_enabled);
	}
	for (NavObstacle *obstacle : map->obstacles) {
		obstacle->get_debug_2d()->debug_set_enabled(debug_enabled);
	}

	debug_mesh_dirty = true;
	debug_update_mesh();
}

void NavMapDebug2D::debug_set_navigation_enabled(bool p_enabled) {
	if (debug_navigation_enabled == p_enabled) {
		return;
	}

	debug_navigation_enabled = p_enabled;

	for (NavRegion *region : map->regions) {
		region->get_debug_2d()->debug_set_enabled(debug_enabled && debug_navigation_enabled);
	}
	for (NavLink *link : map->links) {
		link->get_debug_2d()->debug_set_enabled(debug_enabled && debug_navigation_enabled);
	}
	//for (NavAgent *agent : map->agents) {
	//	agent->get_debug_2d()->debug_set_enabled(debug_navigation_enabled);
	//}
	//for (NavObstacle *obstacle : obstacles) {
	//	obstacle->get_debug_2d()->debug_set_enabled(debug_navigation_enabled);
	//}

	debug_mesh_dirty = true;
	debug_update_mesh();
}

void NavMapDebug2D::debug_set_avoidance_enabled(bool p_enabled) {
	if (debug_avoidance_enabled == p_enabled) {
		return;
	}

	debug_avoidance_enabled = p_enabled;

	//for (NavRegion *region : regions) {
	//	region->get_debug_2d()->debug_set_enabled(debug_avoidance_enabled );
	//}
	//for (NavLink *link : links) {
	//	link->get_debug_2d()->debug_set_enabled(debug_avoidance_enabled );
	//}
	//for (NavAgent *agent : map->agents) {
	//	agent->get_debug_2d()->debug_set_enabled(debug_enabled && debug_avoidance_enabled);
	//}
	for (NavObstacle *obstacle : map->obstacles) {
		obstacle->get_debug_2d()->debug_set_enabled(debug_enabled && debug_avoidance_enabled);
	}

	//debug_mesh_dirty = true;
	//debug_update_mesh();
}

void NavMapDebug2D::debug_free() {
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

void NavMapDebug2D::project_settings_changed() {
}

void NavMapDebug2D::add_region_sync_dirty_request(SelfList<NavRegionDebug2D> *p_sync_request) {
	if (p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.regions.add(p_sync_request);
}

void NavMapDebug2D::add_link_sync_dirty_request(SelfList<NavLinkDebug2D> *p_sync_request) {
	if (p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.links.add(p_sync_request);
}

/*
void NavMapDebug2D::add_agent_sync_dirty_request(SelfList<NavAgentDebug2D> *p_sync_request) {
	if (p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.agents.add(p_sync_request);
}
*/

void NavMapDebug2D::add_obstacle_sync_dirty_request(SelfList<NavObstacleDebug2D> *p_sync_request) {
	if (p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.obstacles.add(p_sync_request);
}

void NavMapDebug2D::remove_region_sync_dirty_request(SelfList<NavRegionDebug2D> *p_sync_request) {
	if (!p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.regions.remove(p_sync_request);
}

void NavMapDebug2D::remove_link_sync_dirty_request(SelfList<NavLinkDebug2D> *p_sync_request) {
	if (!p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.links.remove(p_sync_request);
}

/*
void NavMapDebug2D::remove_agent_sync_dirty_request(SelfList<NavAgentDebug2D> *p_sync_request) {
	if (!p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.agents.remove(p_sync_request);
}
*/

void NavMapDebug2D::remove_obstacle_sync_dirty_request(SelfList<NavObstacleDebug2D> *p_sync_request) {
	if (!p_sync_request->in_list()) {
		return;
	}
	sync_dirty_requests.obstacles.remove(p_sync_request);
}

void NavMapDebug2D::sync() {
	if (project_settings_dirty || debug_settings_dirty) {
		debug_settings_dirty = false;
		project_settings_dirty = false;
		project_settings_changed();

		for (NavRegion *region : map->regions) {
			region->get_debug_2d()->debug_make_dirty();
		}
		for (NavLink *link : map->links) {
			link->get_debug_2d()->debug_make_dirty();
		}
		/*
		for (NavAgent *agent : map->agents) {
			agent->get_debug_2d()->debug_make_dirty();
		}
		*/
		for (NavObstacle *obstacle : map->obstacles) {
			obstacle->get_debug_2d()->debug_make_dirty();
		}

		debug_make_dirty();
	}

	_sync_dirty_update_requests();

	debug_update();
}

void NavMapDebug2D::_sync_dirty_update_requests() {
	// Sync NavRegions.
	for (SelfList<NavRegionDebug2D> *element = sync_dirty_requests.regions.first(); element; element = element->next()) {
		element->self()->sync();
	}
	sync_dirty_requests.regions.clear();

	// Sync NavLinks.
	for (SelfList<NavLinkDebug2D> *element = sync_dirty_requests.links.first(); element; element = element->next()) {
		element->self()->sync();
	}
	sync_dirty_requests.links.clear();

	// Sync NavAgents.
	/*
	for (SelfList<NavAgentDebug2D> *element = sync_dirty_requests.agents.first(); element; element = element->next()) {
		element->self()->sync();
	}
	sync_dirty_requests.agents.clear();
	*/

	// Sync NavObstacles.
	for (SelfList<NavObstacleDebug2D> *element = sync_dirty_requests.obstacles.first(); element; element = element->next()) {
		element->self()->sync();
	}
	sync_dirty_requests.obstacles.clear();
}

NavMapDebug2D::NavMapDebug2D(NavMap *p_map) {
	ERR_FAIL_NULL(p_map);
	map = p_map;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	debug_canvas_item_rid = rs->canvas_item_create();
	debug_mesh2d_rid = rs->mesh_create();
}

NavMapDebug2D::~NavMapDebug2D() {
	debug_free();
}
