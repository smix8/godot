/**************************************************************************/
/*  nav_agent_debug_3d.cpp                                                */
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

#include "nav_agent_debug_3d.h"

#include "../nav_agent_3d.h"
#include "../nav_map_3d.h"

#include "servers/navigation/navigation_debug_3d.h"
#include "servers/navigation_server_3d.h"
#include "servers/rendering_server.h"

void NavAgentDebug3D::debug_set_enabled(bool p_enabled) {
	if (debug_enabled == p_enabled) {
		return;
	}

	debug_enabled = p_enabled;

	//debug_mesh_dirty = true;
	debug_update_mesh();
}

void NavAgentDebug3D::debug_update() {
	debug_update_scenario();
	//debug_update_transform();
	//debug_update_mesh();
	//debug_update_material();
}

void NavAgentDebug3D::debug_update_scenario() {
	debug_scenario_dirty = true;
	if (agent->map) {
		request_sync();
	} else {
		_debug_update_scenario();
	}
}

void NavAgentDebug3D::_debug_update_scenario() {
	ERR_FAIL_COND(!debug_instance_rid.is_valid());

	if (agent->map && agent->map->get_debug()->debug_get_scenario().is_valid()) {
		RenderingServer::get_singleton()->instance_set_scenario(debug_instance_rid, agent->map->get_debug()->debug_get_scenario());
	} else {
		RenderingServer::get_singleton()->instance_set_scenario(debug_instance_rid, RID());
	}
}

void NavAgentDebug3D::debug_update_transform() {
	debug_transform_dirty = true;
	request_sync();
}

void NavAgentDebug3D::_debug_update_transform() {
	if (!debug_transform_dirty) {
		return;
	}
	debug_transform_dirty = false;
	ERR_FAIL_COND(!debug_instance_rid.is_valid());
	//RenderingServer::get_singleton()->instance_set_transform(debug_instance_rid, transform);
}

void NavAgentDebug3D::debug_update_mesh() {
	debug_mesh_dirty = true;
	request_sync();
}

void NavAgentDebug3D::_debug_update_mesh() {
	if (!debug_mesh_dirty) {
		return;
	}
	debug_mesh_dirty = false;

	ERR_FAIL_COND(!debug_mesh_rid.is_valid());
	ERR_FAIL_COND(!debug_instance_rid.is_valid());

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	rs->mesh_clear(debug_mesh_rid);

	if (!debug_enabled || !NavigationDebug3D::debug_global_are_agents_enabled() || !agent->map || !agent->map->get_debug()->debug_is_enabled()) {
		return;
	}

	rs->instance_set_scenario(debug_instance_rid, agent->map->get_debug()->debug_get_scenario());

	if (agent->nav_path.size() < 2) {
		return;
	}

	Array mesh_array;
	mesh_array.resize(RS::ARRAY_MAX);
	mesh_array[RS::ARRAY_VERTEX] = Vector<Vector3>(agent->nav_path);
	rs->mesh_add_surface_from_arrays(debug_mesh_rid, RS::PRIMITIVE_LINE_STRIP, mesh_array);
	rs->mesh_add_surface_from_arrays(debug_mesh_rid, RS::PRIMITIVE_POINTS, mesh_array);

	debug_material_dirty = true;
	_debug_update_material();
	debug_scenario_dirty = true;
	_debug_update_scenario();
}

void NavAgentDebug3D::debug_update_material() {
	debug_material_dirty = true;
	request_sync();
}

void NavAgentDebug3D::_debug_update_material() {
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
		Ref<StandardMaterial3D> line_material = NavigationDebug3D::get_navagent_path_line_material();
		rs->instance_set_surface_override_material(debug_instance_rid, 0, line_material->get_rid());
	}
	if (surface_count > 1) {
			Ref<StandardMaterial3D> point_material = NavigationDebug3D::get_navagent_path_point_material();
			rs->instance_set_surface_override_material(debug_instance_rid, 1, point_material->get_rid());
	}
}

void NavAgentDebug3D::debug_make_dirty() {
	debug_scenario_dirty = true;
	debug_transform_dirty = true;
	debug_mesh_dirty = true;
	debug_material_dirty = true;
	request_sync();
}

void NavAgentDebug3D::debug_free() {
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

void NavAgentDebug3D::sync() {
	_debug_update_scenario();
	_debug_update_transform();
	_debug_update_mesh();
	_debug_update_material();
}

void NavAgentDebug3D::request_sync() {
	if (agent->map && !sync_dirty_request_list_element.in_list()) {
		agent->map->get_debug()->add_agent_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

void NavAgentDebug3D::cancel_sync_request() {
	if (agent->map && sync_dirty_request_list_element.in_list()) {
		agent->map->get_debug()->remove_agent_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

NavAgentDebug3D::NavAgentDebug3D(NavAgent3D *p_agent) :
		sync_dirty_request_list_element(this) {
	ERR_FAIL_NULL(p_agent);
	agent = p_agent;

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

NavAgentDebug3D::~NavAgentDebug3D() {
	cancel_sync_request();
	debug_free();
}
