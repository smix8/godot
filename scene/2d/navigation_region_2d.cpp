/**************************************************************************/
/*  navigation_region_2d.cpp                                              */
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

#include "navigation_region_2d.h"

#include "scene/resources/world_2d.h"
#include "servers/navigation_server_2d.h"

RID NavigationRegion2D::get_rid() const {
	return region;
}

void NavigationRegion2D::set_enabled(bool p_enabled) {
	if (enabled == p_enabled) {
		return;
	}

	enabled = p_enabled;

	NavigationServer2D::get_singleton()->region_set_enabled(region, enabled);
}

bool NavigationRegion2D::is_enabled() const {
	return enabled;
}

void NavigationRegion2D::set_use_edge_connections(bool p_enabled) {
	if (use_edge_connections == p_enabled) {
		return;
	}

	use_edge_connections = p_enabled;

	NavigationServer2D::get_singleton()->region_set_use_edge_connections(region, use_edge_connections);
}

bool NavigationRegion2D::get_use_edge_connections() const {
	return use_edge_connections;
}

void NavigationRegion2D::set_navigation_layers(uint32_t p_navigation_layers) {
	if (navigation_layers == p_navigation_layers) {
		return;
	}

	navigation_layers = p_navigation_layers;

	NavigationServer2D::get_singleton()->region_set_navigation_layers(region, navigation_layers);
}

uint32_t NavigationRegion2D::get_navigation_layers() const {
	return navigation_layers;
}

void NavigationRegion2D::set_navigation_layer_value(int p_layer_number, bool p_value) {
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

bool NavigationRegion2D::get_navigation_layer_value(int p_layer_number) const {
	ERR_FAIL_COND_V_MSG(p_layer_number < 1, false, "Navigation layer number must be between 1 and 32 inclusive.");
	ERR_FAIL_COND_V_MSG(p_layer_number > 32, false, "Navigation layer number must be between 1 and 32 inclusive.");

	return get_navigation_layers() & (1 << (p_layer_number - 1));
}

void NavigationRegion2D::set_enter_cost(real_t p_enter_cost) {
	ERR_FAIL_COND_MSG(p_enter_cost < 0.0, "The enter_cost must be positive.");
	if (Math::is_equal_approx(enter_cost, p_enter_cost)) {
		return;
	}

	enter_cost = p_enter_cost;

	NavigationServer2D::get_singleton()->region_set_enter_cost(region, enter_cost);
}

real_t NavigationRegion2D::get_enter_cost() const {
	return enter_cost;
}

void NavigationRegion2D::set_travel_cost(real_t p_travel_cost) {
	ERR_FAIL_COND_MSG(p_travel_cost < 0.0, "The travel_cost must be positive.");
	if (Math::is_equal_approx(travel_cost, p_travel_cost)) {
		return;
	}

	travel_cost = p_travel_cost;

	NavigationServer2D::get_singleton()->region_set_travel_cost(region, travel_cost);
}

real_t NavigationRegion2D::get_travel_cost() const {
	return travel_cost;
}

RID NavigationRegion2D::get_region_rid() const {
	return get_rid();
}

#ifdef DEBUG_ENABLED
Rect2 NavigationRegion2D::_edit_get_rect() const {
	return navigation_polygon.is_valid() ? navigation_polygon->_edit_get_rect() : Rect2();
}

bool NavigationRegion2D::_edit_is_selected_on_click(const Point2 &p_point, double p_tolerance) const {
	return navigation_polygon.is_valid() ? navigation_polygon->_edit_is_selected_on_click(p_point, p_tolerance) : false;
}
#endif // DEBUG_ENABLED

void NavigationRegion2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_ENTER_TREE: {
			_region_enter_navigation_map();
		} break;

		case NOTIFICATION_TRANSFORM_CHANGED: {
			set_physics_process_internal(true);
		} break;

		case NOTIFICATION_EXIT_TREE: {
			_region_exit_navigation_map();
		} break;

		case NOTIFICATION_INTERNAL_PHYSICS_PROCESS: {
			set_physics_process_internal(false);
			_region_update_transform();
		} break;
	}
}

void NavigationRegion2D::set_navigation_polygon(const Ref<NavigationPolygon> &p_navigation_polygon) {
	if (navigation_polygon.is_valid()) {
		navigation_polygon->disconnect_changed(callable_mp(this, &NavigationRegion2D::_navigation_polygon_changed));
	}

	navigation_polygon = p_navigation_polygon;

	if (navigation_polygon.is_valid()) {
		navigation_polygon->connect_changed(callable_mp(this, &NavigationRegion2D::_navigation_polygon_changed));
	}

	_navigation_polygon_changed();

	NavigationServer2D::get_singleton()->region_set_navigation_polygon(region, p_navigation_polygon);

	emit_signal(SNAME("navigation_polygon_changed"));

	update_configuration_warnings();
}

Ref<NavigationPolygon> NavigationRegion2D::get_navigation_polygon() const {
	return navigation_polygon;
}

void NavigationRegion2D::set_navigation_map(RID p_navigation_map) {
	if (map_override == p_navigation_map) {
		return;
	}

	map_override = p_navigation_map;

	NavigationServer2D::get_singleton()->region_set_map(region, map_override);
}

RID NavigationRegion2D::get_navigation_map() const {
	if (map_override.is_valid()) {
		return map_override;
	} else if (is_inside_tree()) {
		return get_world_2d()->get_navigation_map();
	}
	return RID();
}

void NavigationRegion2D::bake_navigation_polygon(bool p_on_thread) {
	ERR_FAIL_COND_MSG(!Thread::is_main_thread(), "The SceneTree can only be parsed on the main thread. Call this function from the main thread or use call_deferred().");
	ERR_FAIL_COND_MSG(navigation_polygon.is_null(), "Baking the navigation polygon requires a valid `NavigationPolygon` resource.");

	Ref<NavigationMeshSourceGeometryData2D> source_geometry_data;
	source_geometry_data.instantiate();

	NavigationServer2D::get_singleton()->parse_source_geometry_data(navigation_polygon, source_geometry_data, this);

	if (p_on_thread) {
		NavigationServer2D::get_singleton()->bake_from_source_geometry_data_async(navigation_polygon, source_geometry_data, callable_mp(this, &NavigationRegion2D::_bake_finished).bind(navigation_polygon));
	} else {
		NavigationServer2D::get_singleton()->bake_from_source_geometry_data(navigation_polygon, source_geometry_data, callable_mp(this, &NavigationRegion2D::_bake_finished).bind(navigation_polygon));
	}
}

void NavigationRegion2D::_bake_finished(Ref<NavigationPolygon> p_navigation_polygon) {
	if (!Thread::is_main_thread()) {
		callable_mp(this, &NavigationRegion2D::_bake_finished).call_deferred(p_navigation_polygon);
		return;
	}

	set_navigation_polygon(p_navigation_polygon);
	emit_signal(SNAME("bake_finished"));
}

bool NavigationRegion2D::is_baking() const {
	return NavigationServer2D::get_singleton()->is_baking_navigation_polygon(navigation_polygon);
}

void NavigationRegion2D::_navigation_polygon_changed() {

}

PackedStringArray NavigationRegion2D::get_configuration_warnings() const {
	PackedStringArray warnings = Node2D::get_configuration_warnings();

	if (is_visible_in_tree() && is_inside_tree()) {
		if (navigation_polygon.is_null()) {
			warnings.push_back(RTR("A NavigationMesh resource must be set or created for this node to work. Please set a property or draw a polygon."));
		}
	}

	return warnings;
}

void NavigationRegion2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("get_rid"), &NavigationRegion2D::get_rid);

	ClassDB::bind_method(D_METHOD("set_navigation_polygon", "navigation_polygon"), &NavigationRegion2D::set_navigation_polygon);
	ClassDB::bind_method(D_METHOD("get_navigation_polygon"), &NavigationRegion2D::get_navigation_polygon);

	ClassDB::bind_method(D_METHOD("set_enabled", "enabled"), &NavigationRegion2D::set_enabled);
	ClassDB::bind_method(D_METHOD("is_enabled"), &NavigationRegion2D::is_enabled);

	ClassDB::bind_method(D_METHOD("set_navigation_map", "navigation_map"), &NavigationRegion2D::set_navigation_map);
	ClassDB::bind_method(D_METHOD("get_navigation_map"), &NavigationRegion2D::get_navigation_map);

	ClassDB::bind_method(D_METHOD("set_use_edge_connections", "enabled"), &NavigationRegion2D::set_use_edge_connections);
	ClassDB::bind_method(D_METHOD("get_use_edge_connections"), &NavigationRegion2D::get_use_edge_connections);

	ClassDB::bind_method(D_METHOD("set_navigation_layers", "navigation_layers"), &NavigationRegion2D::set_navigation_layers);
	ClassDB::bind_method(D_METHOD("get_navigation_layers"), &NavigationRegion2D::get_navigation_layers);

	ClassDB::bind_method(D_METHOD("set_navigation_layer_value", "layer_number", "value"), &NavigationRegion2D::set_navigation_layer_value);
	ClassDB::bind_method(D_METHOD("get_navigation_layer_value", "layer_number"), &NavigationRegion2D::get_navigation_layer_value);

	ClassDB::bind_method(D_METHOD("get_region_rid"), &NavigationRegion2D::get_region_rid);

	ClassDB::bind_method(D_METHOD("set_enter_cost", "enter_cost"), &NavigationRegion2D::set_enter_cost);
	ClassDB::bind_method(D_METHOD("get_enter_cost"), &NavigationRegion2D::get_enter_cost);

	ClassDB::bind_method(D_METHOD("set_travel_cost", "travel_cost"), &NavigationRegion2D::set_travel_cost);
	ClassDB::bind_method(D_METHOD("get_travel_cost"), &NavigationRegion2D::get_travel_cost);

	ClassDB::bind_method(D_METHOD("bake_navigation_polygon", "on_thread"), &NavigationRegion2D::bake_navigation_polygon, DEFVAL(true));
	ClassDB::bind_method(D_METHOD("is_baking"), &NavigationRegion2D::is_baking);

	ClassDB::bind_method(D_METHOD("_navigation_polygon_changed"), &NavigationRegion2D::_navigation_polygon_changed);

	ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "navigation_polygon", PROPERTY_HINT_RESOURCE_TYPE, "NavigationPolygon"), "set_navigation_polygon", "get_navigation_polygon");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "enabled"), "set_enabled", "is_enabled");
	ADD_PROPERTY(PropertyInfo(Variant::BOOL, "use_edge_connections"), "set_use_edge_connections", "get_use_edge_connections");
	ADD_PROPERTY(PropertyInfo(Variant::INT, "navigation_layers", PROPERTY_HINT_LAYERS_2D_NAVIGATION), "set_navigation_layers", "get_navigation_layers");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "enter_cost"), "set_enter_cost", "get_enter_cost");
	ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "travel_cost"), "set_travel_cost", "get_travel_cost");

	ADD_SIGNAL(MethodInfo("navigation_polygon_changed"));
	ADD_SIGNAL(MethodInfo("bake_finished"));
}

#ifndef DISABLE_DEPRECATED
// Compatibility with earlier 4.0 betas.
bool NavigationRegion2D::_set(const StringName &p_name, const Variant &p_value) {
	if (p_name == "navpoly") {
		set_navigation_polygon(p_value);
		return true;
	}
	return false;
}

bool NavigationRegion2D::_get(const StringName &p_name, Variant &r_ret) const {
	if (p_name == "navpoly") {
		r_ret = get_navigation_polygon();
		return true;
	}
	return false;
}
#endif // DISABLE_DEPRECATED

NavigationRegion2D::NavigationRegion2D() {
	set_notify_transform(true);
	set_hide_clip_children(true);

	region = NavigationServer2D::get_singleton()->region_create();
	NavigationServer2D::get_singleton()->region_set_owner_id(region, get_instance_id());
	NavigationServer2D::get_singleton()->region_set_enter_cost(region, get_enter_cost());
	NavigationServer2D::get_singleton()->region_set_travel_cost(region, get_travel_cost());
	NavigationServer2D::get_singleton()->region_set_navigation_layers(region, navigation_layers);
	NavigationServer2D::get_singleton()->region_set_use_edge_connections(region, use_edge_connections);
	NavigationServer2D::get_singleton()->region_set_enabled(region, enabled);
}

NavigationRegion2D::~NavigationRegion2D() {
	ERR_FAIL_NULL(NavigationServer2D::get_singleton());
	NavigationServer2D::get_singleton()->free(region);
}

void NavigationRegion2D::_region_enter_navigation_map() {
	if (!is_inside_tree()) {
		return;
	}

	if (map_override.is_valid()) {
		NavigationServer2D::get_singleton()->region_set_map(region, map_override);
	} else {
		NavigationServer2D::get_singleton()->region_set_map(region, get_world_2d()->get_navigation_map());
	}

	current_global_transform = get_global_transform();
	NavigationServer2D::get_singleton()->region_set_transform(region, current_global_transform);

	NavigationServer2D::get_singleton()->region_set_enabled(region, enabled);
}

void NavigationRegion2D::_region_exit_navigation_map() {
	NavigationServer2D::get_singleton()->region_set_map(region, RID());
}

void NavigationRegion2D::_region_update_transform() {
	if (!is_inside_tree()) {
		return;
	}

	Transform2D new_global_transform = get_global_transform();
	if (current_global_transform != new_global_transform) {
		current_global_transform = new_global_transform;
		NavigationServer2D::get_singleton()->region_set_transform(region, current_global_transform);
	}
}
