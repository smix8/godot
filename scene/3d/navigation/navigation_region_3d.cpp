/**************************************************************************/
/*  navigation_region_3d.cpp                                              */
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

#include "navigation_region_3d.h"

#include "core/math/random_pcg.h"
#include "scene/resources/3d/navigation_mesh_source_geometry_data_3d.h"
#include "servers/navigation_server_3d.h"

RID NavigationRegion3D::get_rid() const {
	return region;
}

void NavigationRegion3D::set_enabled(bool p_enabled) {
	if (enabled == p_enabled) {
		return;
	}

	enabled = p_enabled;

	NavigationServer3D::get_singleton()->region_set_enabled(region, enabled);

	update_gizmos();
}

bool NavigationRegion3D::is_enabled() const {
	return enabled;
}

void NavigationRegion3D::set_use_edge_connections(bool p_enabled) {
	if (use_edge_connections == p_enabled) {
		return;
	}

	use_edge_connections = p_enabled;

	NavigationServer3D::get_singleton()->region_set_use_edge_connections(region, use_edge_connections);
}

bool NavigationRegion3D::get_use_edge_connections() const {
	return use_edge_connections;
}

void NavigationRegion3D::set_navigation_layers(uint32_t p_navigation_layers) {
	if (navigation_layers == p_navigation_layers) {
		return;
	}

	navigation_layers = p_navigation_layers;

	NavigationServer3D::get_singleton()->region_set_navigation_layers(region, navigation_layers);
}

uint32_t NavigationRegion3D::get_navigation_layers() const {
	return navigation_layers;
}

void NavigationRegion3D::set_navigation_layer_value(int p_layer_number, bool p_value) {
	ERR_FAIL_COND_MSG(p_layer_number < 1, "Navigation layer number must be between 1 and 32 inclusive.");
	ERR_FAIL_COND_MSG(p_layer_number > 32, "Navigation layer number must be between 1 and 32 inclusive.");

	uint32_t _navigation_layers = get_navigation_layers();

	if (p_value) {
		_navigation_layers |= 1 << (p_layer_number - 1);
	} else {
		_navigation_layers &= ~(1 << (p_layer_number - 1));
	}

	set_navigation_layers(_navigation_layers);
}

bool NavigationRegion3D::get_navigation_layer_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1, false, "Navigation layer number must be between 1 and 32 inclusive.");
	ERR_FAIL_COND_V_MSG(p_layer_number > 32, false, "Navigation layer number must be between 1 and 32 inclusive.");

	return get_navigation_layers() & (1 << (p_layer_number - 1));
}

void NavigationRegion3D::set_enter_cost(real_t p_enter_cost) {
	ERR_FAIL_COND_MSG(p_enter_cost < 0.0, "The enter_cost must be positive.");
	if (Math::is_equal_approx(enter_cost, p_enter_cost)) {
		return;
	}

	enter_cost = p_enter_cost;

	NavigationServer3D::get_singleton()->region_set_enter_cost(region, enter_cost);
}

real_t NavigationRegion3D::get_enter_cost() const {
	return enter_cost;
}

void NavigationRegion3D::set_travel_cost(real_t p_travel_cost) {
	ERR_FAIL_COND_MSG(p_travel_cost < 0.0, "The travel_cost must be positive.");
	if (Math::is_equal_approx(travel_cost, p_travel_cost)) {
		return;
	}

	travel_cost = p_travel_cost;

	NavigationServer3D::get_singleton()->region_set_travel_cost(region, travel_cost);
}

real_t NavigationRegion3D::get_travel_cost() const {
	return travel_cost;
}

RID NavigationRegion3D::get_region_rid() const {
	return get_rid();
}

void NavigationRegion3D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_region_enter_navigation_map();
		} break;

		case NOTIFICATION_TRANSFORM_CHANGED: {
			set_physics_process_internal(true);
		} break;

		case NOTIFICATION_VISIBILITY_CHANGED: {
			NavigationServer3D::get_singleton()->debug_region_set_enabled(get_rid(), is_visible_in_tree());
		} break;

		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			set_physics_process_internal(false);
			_region_update_transform();
		} break;

		case NOTIFICATION_EXIT_TREE: {
			_region_exit_navigation_map();
		} break;
	}
}

void NavigationRegion3D::set_navigation_mesh(const Ref<NavigationMesh> &p_navigation_mesh) {
	if (navigation_mesh.is_valid()) {
		navigation_mesh->disconnect_changed(callable_mp(this, &NavigationRegion3D::_navigation_mesh_changed));
	}

	navigation_mesh = p_navigation_mesh;

	if (navigation_mesh.is_valid()) {
		navigation_mesh->connect_changed(callable_mp(this, &NavigationRegion3D::_navigation_mesh_changed));
	}

	_navigation_mesh_changed();
}

Ref<NavigationMesh> NavigationRegion3D::get_navigation_mesh() const {
	return navigation_mesh;
}

void NavigationRegion3D::set_navigation_map(RID p_navigation_map) {
	if (map_override == p_navigation_map) {
		return;
	}

	map_override = p_navigation_map;

	NavigationServer3D::get_singleton()->region_set_map(region, map_override);
}

RID NavigationRegion3D::get_navigation_map() const {
	if (map_override.is_valid()) {
		return map_override;
	} else if (is_inside_tree()) {
		return get_world_3d()->get_navigation_map();
	}
	return RID();
}

void NavigationRegion3D::bake_navigation_mesh(bool p_on_thread) {
	ERR_FAIL_COND_MSG(!Thread::is_main_thread(), "The SceneTree can only be parsed on the main thread. Call this function from the main thread or use call_deferred().");
	ERR_FAIL_COND_MSG(navigation_mesh.is_null(), "Baking the navigation mesh requires a valid `NavigationMesh` resource.");

	Ref<NavigationMeshSourceGeometryData3D> source_geometry_data;
	source_geometry_data.instantiate();

	NavigationServer3D::get_singleton()->parse_source_geometry_data(navigation_mesh, source_geometry_data, this);

	if (p_on_thread) {
		NavigationServer3D::get_singleton()->bake_from_source_geometry_data_async(navigation_mesh, source_geometry_data, callable_mp(this, &NavigationRegion3D::_bake_finished));
	} else {
		NavigationServer3D::get_singleton()->bake_from_source_geometry_data(navigation_mesh, source_geometry_data, callable_mp(this, &NavigationRegion3D::_bake_finished));
	}
}

void NavigationRegion3D::_bake_finished() {
	if (!Thread::is_main_thread()) {
		callable_mp(this, &NavigationRegion3D::_bake_finished).call_deferred();
		return;
	}

	emit_signal(SNAME("bake_finished"));
}

bool NavigationRegion3D::is_baking() const {
	return NavigationServer3D::get_singleton()->is_baking_navigation_mesh(navigation_mesh);
}

PackedStringArray NavigationRegion3D::get_configuration_warnings() const {
	PackedStringArray warnings = Node3D::get_configuration_warnings();

	if (is_visible_in_tree() && is_inside_tree()) {
		if (navigation_mesh.is_null()) {
			warnings.push_back(RTR("A NavigationMesh resource must be set or created for this node to work."));
		}
	}

	return warnings;
}

void NavigationRegion3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_rid"), &NavigationRegion3D::get_rid);

	ClassDB::bind_method(D_METHOD("set_navigation_mesh", "navigation_mesh"), &NavigationRegion3D::set_navigation_mesh);
	ClassDB::bind_method(D_METHOD("get_navigation_mesh"), &NavigationRegion3D::get_navigation_mesh);

	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &NavigationRegion3D::set_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &NavigationRegion3D::is_enabled);

	ClassDB::bind_method(D_METHOD("set_navigation_map", "navigation_map"), &NavigationRegion3D::set_navigation_map);
	ClassDB::bind_method(D_METHOD("get_navigation_map"), &NavigationRegion3D::get_navigation_map);

	ClassDB::bind_method(D_METHOD("set_use_edge_connections", "enabled"), &NavigationRegion3D::set_use_edge_connections);
	ClassDB::bind_method(D_METHOD("get_use_edge_connections"), &NavigationRegion3D::get_use_edge_connections);

	ClassDB::bind_method(D_METHOD("set_navigation_layers", "navigation_layers"), &NavigationRegion3D::set_navigation_layers);
	ClassDB::bind_method(D_METHOD("get_navigation_layers"), &NavigationRegion3D::get_navigation_layers);

	ClassDB::bind_method(D_METHOD("set_navigation_layer_value", "layer_number", "value"), &NavigationRegion3D::set_navigation_layer_value);
	ClassDB::bind_method(D_METHOD("get_navigation_layer_value", "layer_number"), &NavigationRegion3D::get_navigation_layer_value);

	ClassDB::bind_method(D_METHOD("get_region_rid"), &NavigationRegion3D::get_region_rid);

	ClassDB::bind_method(D_METHOD("set_enter_cost", "enter_cost"), &NavigationRegion3D::set_enter_cost);
	ClassDB::bind_method(D_METHOD("get_enter_cost"), &NavigationRegion3D::get_enter_cost);

	ClassDB::bind_method(D_METHOD("set_travel_cost", "travel_cost"), &NavigationRegion3D::set_travel_cost);
	ClassDB::bind_method(D_METHOD("get_travel_cost"), &NavigationRegion3D::get_travel_cost);

	ClassDB::bind_method(D_METHOD("bake_navigation_mesh", "on_thread"), &NavigationRegion3D::bake_navigation_mesh, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("is_baking"), &NavigationRegion3D::is_baking);

	ClassDB::bind_method(D_METHOD("get_bounds"), &NavigationRegion3D::get_bounds);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "navigation_mesh", PROPERTY_HINT_RESOURCE_TYPE, "NavigationMesh"), "set_navigation_mesh", "get_navigation_mesh");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "is_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_edge_connections"), "set_use_edge_connections", "get_use_edge_connections");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "navigation_layers", PROPERTY_HINT_LAYERS_3D_NAVIGATION), "set_navigation_layers", "get_navigation_layers");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "enter_cost"), "set_enter_cost", "get_enter_cost");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "travel_cost"), "set_travel_cost", "get_travel_cost");

	ADD_SIGNAL(MethodInfo("navigation_mesh_changed"));
	ADD_SIGNAL(MethodInfo("bake_finished"));
}

#ifndef DISABLE_DEPRECATED
// Compatibility with earlier 4.0 betas.
bool NavigationRegion3D::_set(const StringName &p_name, const Variant &p_value) {
	if (p_name == "navmesh") {
		set_navigation_mesh(p_value);
		return true;
	}
	return false;
}

bool NavigationRegion3D::_get(const StringName &p_name, Variant &r_ret) const {
	if (p_name == "navmesh") {
		r_ret = get_navigation_mesh();
		return true;
	}
	return false;
}
#endif // DISABLE_DEPRECATED

void NavigationRegion3D::_navigation_mesh_changed() {
	_update_bounds();

	NavigationServer3D::get_singleton()->region_set_navigation_mesh(region, navigation_mesh);

	emit_signal(SNAME("navigation_mesh_changed"));

	update_gizmos();
	update_configuration_warnings();
}

void NavigationRegion3D::_region_enter_navigation_map() {
	if (!is_inside_tree()) {
		return;
	}

	if (map_override.is_valid()) {
		NavigationServer3D::get_singleton()->region_set_map(region, map_override);
	} else {
		NavigationServer3D::get_singleton()->region_set_map(region, get_world_3d()->get_navigation_map());
	}

	current_global_transform = get_global_transform();
	NavigationServer3D::get_singleton()->region_set_transform(region, current_global_transform);

	NavigationServer3D::get_singleton()->region_set_enabled(region, enabled);
}

void NavigationRegion3D::_region_exit_navigation_map() {
	NavigationServer3D::get_singleton()->region_set_map(region, RID());
}

void NavigationRegion3D::_region_update_transform() {
	if (!is_inside_tree()) {
		return;
	}

	Transform3D new_global_transform = get_global_transform();
	if (current_global_transform != new_global_transform) {
		current_global_transform = new_global_transform;
		NavigationServer3D::get_singleton()->region_set_transform(region, current_global_transform);
	}
}

NavigationRegion3D::NavigationRegion3D() {
	set_notify_transform(true);

	region = NavigationServer3D::get_singleton()->region_create();
	NavigationServer3D::get_singleton()->region_set_owner_id(region, get_instance_id());
	NavigationServer3D::get_singleton()->region_set_enter_cost(region, get_enter_cost());
	NavigationServer3D::get_singleton()->region_set_travel_cost(region, get_travel_cost());
	NavigationServer3D::get_singleton()->region_set_navigation_layers(region, navigation_layers);
	NavigationServer3D::get_singleton()->region_set_use_edge_connections(region, use_edge_connections);
	NavigationServer3D::get_singleton()->region_set_enabled(region, enabled);
}

NavigationRegion3D::~NavigationRegion3D() {
	if (navigation_mesh.is_valid()) {
		navigation_mesh->disconnect_changed(callable_mp(this, &NavigationRegion3D::_navigation_mesh_changed));
	}
	ERR_FAIL_NULL(NavigationServer3D::get_singleton());
	NavigationServer3D::get_singleton()->free(region);
}

void NavigationRegion3D::_update_bounds() {
	if (navigation_mesh.is_null()) {
		bounds = AABB();
		return;
	}

	const Vector<Vector3> &vertices = navigation_mesh->get_vertices();
	if (vertices.is_empty()) {
		bounds = AABB();
		return;
	}

	const Transform3D gt = is_inside_tree() ? get_global_transform() : get_transform();

	AABB new_bounds;
	new_bounds.position = gt.xform(vertices[0]);

	for (const Vector3 &vertex : vertices) {
		new_bounds.expand_to(gt.xform(vertex));
	}
	bounds = new_bounds;
}
