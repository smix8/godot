/**************************************************************************/
/*  navigation_debug_3d.cpp                                               */
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

#ifdef DEBUG_ENABLED

#include "navigation_debug_3d.h"

#include "core/config/project_settings.h"
#include "servers/navigation_server_3d.h"

bool NavigationDebug3D::debug_global_enabled = true;

bool NavigationDebug3D::debug_global_navigation_enabled = true;
bool NavigationDebug3D::debug_global_avoidance_enabled = true;

bool NavigationDebug3D::debug_global_maps_enabled = true;
bool NavigationDebug3D::debug_global_regions_enabled = true;
bool NavigationDebug3D::debug_global_links_enabled = true;
bool NavigationDebug3D::debug_global_obstacles_enabled = true;
bool NavigationDebug3D::debug_global_agents_enabled = true;

bool NavigationDebug3D::navigation_debug_dirty = true;
bool NavigationDebug3D::avoidance_debug_dirty = true;

Color NavigationDebug3D::navmesh_edge_connection_color = Color(1.0, 0.0, 1.0, 1.0);
Color NavigationDebug3D::navmesh_geometry_edge_color = Color(0.5, 1.0, 1.0, 1.0);
Color NavigationDebug3D::navmesh_geometry_face_color = Color(0.5, 1.0, 1.0, 0.4);
Color NavigationDebug3D::navmesh_geometry_edge_disabled_color = Color(0.5, 0.5, 0.5, 1.0);
Color NavigationDebug3D::navmesh_geometry_face_disabled_color = Color(0.5, 0.5, 0.5, 0.4);
Color NavigationDebug3D::navlink_connection_color = Color(1.0, 0.5, 1.0, 1.0);
Color NavigationDebug3D::navlink_connection_disabled_color = Color(0.5, 0.5, 0.5, 1.0);
Color NavigationDebug3D::navagent_path_color = Color(1.0, 0.0, 0.0, 1.0);

real_t NavigationDebug3D::navagent_path_point_size = 4.0;

Color NavigationDebug3D::navagent_avoidance_radius_color = Color(1.0, 1.0, 0.0, 0.25);
Color NavigationDebug3D::navobstacle_avoidance_radius_color = Color(1.0, 0.5, 0.0, 0.25);

Color NavigationDebug3D::navobstacle_avoidance_static_pushin_face_color = Color(1.0, 0.0, 0.0, 0.0);
Color NavigationDebug3D::navobstacle_avoidance_static_pushin_pushout_face_color = Color(1.0, 1.0, 0.0, 0.5);
Color NavigationDebug3D::navobstacle_avoidance_static_pushin_pushin_edge_color = Color(1.0, 0.0, 0.0, 1.0);
Color NavigationDebug3D::navobstacle_avoidance_static_pushin_pushout_edge_color = Color(1.0, 1.0, 0.0, 1.0);

bool NavigationDebug3D::navmesh_edge_connections_enabled = true;
bool NavigationDebug3D::navmesh_edge_connections_enabled_xray = true;
bool NavigationDebug3D::navmesh_edge_lines_enabled = true;
bool NavigationDebug3D::navmesh_edge_lines_enabled_xray = true;
bool NavigationDebug3D::navmesh_geometry_face_random_color_enabled = true;
bool NavigationDebug3D::navlink_link_connections_enabled = true;
bool NavigationDebug3D::navlink_link_connections_enabled_xray = true;
bool NavigationDebug3D::navagent_paths_enabled = true;
bool NavigationDebug3D::navagent_paths_enabled_xray = true;

bool NavigationDebug3D::navagent_avoidance_radius_enabled = true;
bool NavigationDebug3D::navobstacle_avoidance_radius_enabled = true;
bool NavigationDebug3D::navobstacle_avoidance_static_enabled = true;

Ref<StandardMaterial3D> NavigationDebug3D::navmesh_geometry_edge_material;
Ref<StandardMaterial3D> NavigationDebug3D::navmesh_geometry_face_material;
Ref<StandardMaterial3D> NavigationDebug3D::navmesh_geometry_edge_disabled_material;
Ref<StandardMaterial3D> NavigationDebug3D::navmesh_geometry_face_disabled_material;
Ref<StandardMaterial3D> NavigationDebug3D::navmesh_edge_connections_material;
Ref<StandardMaterial3D> NavigationDebug3D::navlink_connections_material;
Ref<StandardMaterial3D> NavigationDebug3D::navlink_connections_disabled_material;
Ref<StandardMaterial3D> NavigationDebug3D::navagent_avoidance_radius_material;
Ref<StandardMaterial3D> NavigationDebug3D::navobstacle_avoidance_radius_material;

Ref<StandardMaterial3D> NavigationDebug3D::navobstacle_avoidance_static_pushin_face_material;
Ref<StandardMaterial3D> NavigationDebug3D::navobstacle_avoidance_static_pushout_face_material;
Ref<StandardMaterial3D> NavigationDebug3D::navobstacle_avoidance_static_pushin_edge_material;
Ref<StandardMaterial3D> NavigationDebug3D::navobstacle_avoidance_static_pushout_edge_material;

Ref<StandardMaterial3D> NavigationDebug3D::navagent_path_line_material;
Ref<StandardMaterial3D> NavigationDebug3D::navagent_path_point_material;

void NavigationDebug3D::sync() {
	ERR_FAIL_NULL(NavigationServer3D::get_singleton());

	if (navigation_debug_dirty) {
		NavigationServer3D::get_singleton()->emit_signal(SNAME("navigation_debug_changed"));
	}
	if (avoidance_debug_dirty) {
		NavigationServer3D::get_singleton()->emit_signal(SNAME("avoidance_debug_changed"));
	}
	navigation_debug_dirty = false;
	avoidance_debug_dirty = false;
}

void NavigationDebug3D::debug_global_set_enabled(bool p_enabled) {
	debug_global_enabled = p_enabled;
}

bool NavigationDebug3D::debug_global_is_enabled() {
	return debug_global_enabled;
}

void NavigationDebug3D::debug_global_set_navigation_enabled(bool p_enabled) {
	debug_global_navigation_enabled = p_enabled;
}

bool NavigationDebug3D::debug_global_is_navigation_enabled() {
	return debug_global_navigation_enabled;
}

void NavigationDebug3D::debug_global_set_avoidance_enabled(bool p_enabled) {
	debug_global_avoidance_enabled = p_enabled;
}

bool NavigationDebug3D::debug_global_is_avoidance_enabled() {
	return debug_global_avoidance_enabled;
}

void NavigationDebug3D::debug_global_set_maps_enabled(bool p_enabled) {
	debug_global_maps_enabled = p_enabled;
}

void NavigationDebug3D::debug_global_set_regions_enabled(bool p_enabled) {
	debug_global_regions_enabled = p_enabled;
}

void NavigationDebug3D::debug_global_set_links_enabled(bool p_enabled) {
	debug_global_links_enabled = p_enabled;
}

void NavigationDebug3D::debug_global_set_obstacles_enabled(bool p_enabled) {
	debug_global_obstacles_enabled = p_enabled;
}

void NavigationDebug3D::debug_global_set_agents_enabled(bool p_enabled) {
	debug_global_agents_enabled = p_enabled;
}

bool NavigationDebug3D::debug_global_are_maps_enabled() {
	return debug_global_maps_enabled;
}

bool NavigationDebug3D::debug_global_are_regions_enabled() {
	return debug_global_regions_enabled;
}

bool NavigationDebug3D::debug_global_are_links_enabled() {
	return debug_global_links_enabled;
}

bool NavigationDebug3D::debug_global_are_obstacles_enabled() {
	return debug_global_obstacles_enabled;
}

bool NavigationDebug3D::debug_global_are_agents_enabled() {
	return debug_global_agents_enabled;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navmesh_geometry_face_material() {
	if (navmesh_geometry_face_material.is_valid()) {
		return navmesh_geometry_face_material;
	}

	bool enabled_geometry_face_random_color = get_navmesh_geometry_face_random_color_enabled();

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
	material->set_albedo(get_navmesh_geometry_face_color());
	material->set_cull_mode(StandardMaterial3D::CULL_DISABLED);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	if (enabled_geometry_face_random_color) {
		material->set_flag(StandardMaterial3D::FLAG_SRGB_VERTEX_COLOR, true);
		material->set_flag(StandardMaterial3D::FLAG_ALBEDO_FROM_VERTEX_COLOR, true);
	}

	navmesh_geometry_face_material = material;

	return navmesh_geometry_face_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navmesh_geometry_edge_material() {
	if (navmesh_geometry_edge_material.is_valid()) {
		return navmesh_geometry_edge_material;
	}

	bool enabled_edge_lines_xray = get_navmesh_edge_lines_enabled_xray();

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_albedo(get_navmesh_geometry_edge_color());
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	if (enabled_edge_lines_xray) {
		material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
	}

	navmesh_geometry_edge_material = material;

	return navmesh_geometry_edge_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navmesh_geometry_face_disabled_material() {
	if (navmesh_geometry_face_disabled_material.is_valid()) {
		return navmesh_geometry_face_disabled_material;
	}

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
	material->set_albedo(get_navmesh_geometry_face_disabled_color());
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);

	navmesh_geometry_face_disabled_material = material;

	return navmesh_geometry_face_disabled_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navmesh_geometry_edge_disabled_material() {
	if (navmesh_geometry_edge_disabled_material.is_valid()) {
		return navmesh_geometry_edge_disabled_material;
	}

	bool enabled_edge_lines_xray = get_navmesh_edge_lines_enabled_xray();

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_albedo(get_navmesh_geometry_edge_disabled_color());
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	if (enabled_edge_lines_xray) {
		material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
	}

	navmesh_geometry_edge_disabled_material = material;

	return navmesh_geometry_edge_disabled_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navmesh_edge_connections_material() {
	if (navmesh_edge_connections_material.is_valid()) {
		return navmesh_edge_connections_material;
	}

	bool enabled_edge_connections_xray = get_navmesh_edge_connections_enabled_xray();

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_albedo(get_navmesh_edge_connection_color());
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	if (enabled_edge_connections_xray) {
		material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
	}
	material->set_render_priority(StandardMaterial3D::RENDER_PRIORITY_MAX - 2);

	navmesh_edge_connections_material = material;

	return navmesh_edge_connections_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navlink_connections_material() {
	if (navlink_connections_material.is_valid()) {
		return navlink_connections_material;
	}

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_albedo(navlink_connection_color);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	if (navlink_link_connections_enabled_xray) {
		material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
	}
	material->set_render_priority(StandardMaterial3D::RENDER_PRIORITY_MAX - 2);

	navlink_connections_material = material;
	return navlink_connections_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navlink_connections_disabled_material() {
	if (navlink_connections_disabled_material.is_valid()) {
		return navlink_connections_disabled_material;
	}

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_albedo(navlink_connection_disabled_color);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	if (navlink_link_connections_enabled_xray) {
		material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
	}
	material->set_render_priority(StandardMaterial3D::RENDER_PRIORITY_MAX - 2);

	navlink_connections_disabled_material = material;
	return navlink_connections_disabled_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navagent_path_line_material() {
	if (navagent_path_line_material.is_valid()) {
		return navagent_path_line_material;
	}

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_albedo(navagent_path_color);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	if (navagent_paths_enabled_xray) {
		material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
	}
	material->set_render_priority(StandardMaterial3D::RENDER_PRIORITY_MAX - 2);

	navagent_path_line_material = material;
	return navagent_path_line_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navagent_path_point_material() {
	if (navagent_path_point_material.is_valid()) {
		return navagent_path_point_material;
	}

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_albedo(navagent_path_color);
	material->set_flag(StandardMaterial3D::FLAG_USE_POINT_SIZE, true);
	material->set_point_size(navagent_path_point_size);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	if (navagent_paths_enabled_xray) {
		material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);
	}
	material->set_render_priority(StandardMaterial3D::RENDER_PRIORITY_MAX - 2);

	navagent_path_point_material = material;
	return navagent_path_point_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navagent_avoidance_radius_material() {
	if (navagent_avoidance_radius_material.is_valid()) {
		return navagent_avoidance_radius_material;
	}

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
	material->set_cull_mode(StandardMaterial3D::CULL_DISABLED);
	material->set_albedo(navagent_avoidance_radius_color);
	material->set_render_priority(StandardMaterial3D::RENDER_PRIORITY_MIN + 2);

	navagent_avoidance_radius_material = material;
	return navagent_avoidance_radius_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navobstacle_avoidance_radius_material() {
	if (navobstacle_avoidance_radius_material.is_valid()) {
		return navobstacle_avoidance_radius_material;
	}

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	material->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
	material->set_cull_mode(StandardMaterial3D::CULL_DISABLED);
	material->set_albedo(navobstacle_avoidance_radius_color);
	material->set_render_priority(StandardMaterial3D::RENDER_PRIORITY_MIN + 2);

	navobstacle_avoidance_radius_material = material;
	return navobstacle_avoidance_radius_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navobstacle_avoidance_static_pushin_face_material() {
	if (navobstacle_avoidance_static_pushin_face_material.is_valid()) {
		return navobstacle_avoidance_static_pushin_face_material;
	}

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	material->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
	material->set_cull_mode(StandardMaterial3D::CULL_DISABLED);
	material->set_albedo(navobstacle_avoidance_static_pushin_face_color);
	material->set_render_priority(StandardMaterial3D::RENDER_PRIORITY_MIN + 2);

	navobstacle_avoidance_static_pushin_face_material = material;
	return navobstacle_avoidance_static_pushin_face_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navobstacle_avoidance_static_pushout_face_material() {
	if (navobstacle_avoidance_static_pushout_face_material.is_valid()) {
		return navobstacle_avoidance_static_pushout_face_material;
	}

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	material->set_transparency(StandardMaterial3D::TRANSPARENCY_ALPHA);
	material->set_cull_mode(StandardMaterial3D::CULL_DISABLED);
	material->set_albedo(navobstacle_avoidance_static_pushin_pushout_face_color);
	material->set_render_priority(StandardMaterial3D::RENDER_PRIORITY_MIN + 2);

	navobstacle_avoidance_static_pushout_face_material = material;
	return navobstacle_avoidance_static_pushout_face_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navobstacle_avoidance_static_pushin_edge_material() {
	if (navobstacle_avoidance_static_pushin_edge_material.is_valid()) {
		return navobstacle_avoidance_static_pushin_edge_material;
	}

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	material->set_albedo(navobstacle_avoidance_static_pushin_pushin_edge_color);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);

	navobstacle_avoidance_static_pushin_edge_material = material;
	return navobstacle_avoidance_static_pushin_edge_material;
}

Ref<StandardMaterial3D> NavigationDebug3D::get_navobstacle_avoidance_static_pushout_edge_material() {
	if (navobstacle_avoidance_static_pushout_edge_material.is_valid()) {
		return navobstacle_avoidance_static_pushout_edge_material;
	}

	Ref<StandardMaterial3D> material;
	material.instantiate();
	material->set_shading_mode(StandardMaterial3D::SHADING_MODE_UNSHADED);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_FOG, true);
	material->set_albedo(navobstacle_avoidance_static_pushin_pushout_edge_color);
	material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, true);

	navobstacle_avoidance_static_pushout_edge_material = material;
	return navobstacle_avoidance_static_pushout_edge_material;
}

void NavigationDebug3D::set_navmesh_edge_connection_color(const Color &p_color) {
	navmesh_edge_connection_color = p_color;
	if (navmesh_edge_connections_material.is_valid()) {
		navmesh_edge_connections_material->set_albedo(navmesh_edge_connection_color);
	}
}

Color NavigationDebug3D::get_navmesh_edge_connection_color() {
	return navmesh_edge_connection_color;
}

void NavigationDebug3D::set_navmesh_geometry_edge_color(const Color &p_color) {
	navmesh_geometry_edge_color = p_color;
	if (navmesh_geometry_edge_material.is_valid()) {
		navmesh_geometry_edge_material->set_albedo(navmesh_geometry_edge_color);
	}
}

Color NavigationDebug3D::get_navmesh_geometry_edge_color() {
	return navmesh_geometry_edge_color;
}

void NavigationDebug3D::set_navmesh_geometry_face_color(const Color &p_color) {
	navmesh_geometry_face_color = p_color;
	if (navmesh_geometry_face_material.is_valid()) {
		navmesh_geometry_face_material->set_albedo(navmesh_geometry_face_color);
	}
}

Color NavigationDebug3D::get_navmesh_geometry_face_color() {
	return navmesh_geometry_face_color;
}

void NavigationDebug3D::set_navmesh_geometry_edge_disabled_color(const Color &p_color) {
	navmesh_geometry_edge_disabled_color = p_color;
	if (navmesh_geometry_edge_disabled_material.is_valid()) {
		navmesh_geometry_edge_disabled_material->set_albedo(navmesh_geometry_edge_disabled_color);
	}
}

Color NavigationDebug3D::get_navmesh_geometry_edge_disabled_color() {
	return navmesh_geometry_edge_disabled_color;
}

void NavigationDebug3D::set_navmesh_geometry_face_disabled_color(const Color &p_color) {
	navmesh_geometry_face_disabled_color = p_color;
	if (navmesh_geometry_face_disabled_material.is_valid()) {
		navmesh_geometry_face_disabled_material->set_albedo(navmesh_geometry_face_disabled_color);
	}
}

Color NavigationDebug3D::get_navmesh_geometry_face_disabled_color() {
	return navmesh_geometry_face_disabled_color;
}

void NavigationDebug3D::set_navlink_connection_color(const Color &p_color) {
	navlink_connection_color = p_color;
	if (navlink_connections_material.is_valid()) {
		navlink_connections_material->set_albedo(navlink_connection_color);
	}
}

Color NavigationDebug3D::get_navlink_connection_color() {
	return navlink_connection_color;
}

void NavigationDebug3D::set_navlink_connection_disabled_color(const Color &p_color) {
	navlink_connection_disabled_color = p_color;
	if (navlink_connections_disabled_material.is_valid()) {
		navlink_connections_disabled_material->set_albedo(navlink_connection_disabled_color);
	}
}

Color NavigationDebug3D::get_navlink_connection_disabled_color() {
	return navlink_connection_disabled_color;
}

void NavigationDebug3D::set_navagent_path_point_size(real_t p_point_size) {
	navagent_path_point_size = MAX(0.1, p_point_size);
	if (navagent_path_point_material.is_valid()) {
		navagent_path_point_material->set_point_size(navagent_path_point_size);
	}
}

real_t NavigationDebug3D::get_navagent_path_point_size() {
	return navagent_path_point_size;
}

void NavigationDebug3D::set_navagent_path_color(const Color &p_color) {
	navagent_path_color = p_color;
	if (navagent_path_line_material.is_valid()) {
		navagent_path_line_material->set_albedo(navagent_path_color);
	}
	if (navagent_path_point_material.is_valid()) {
		navagent_path_point_material->set_albedo(navagent_path_color);
	}
}

Color NavigationDebug3D::get_navagent_path_color() {
	return navagent_path_color;
}

void NavigationDebug3D::set_navmesh_edge_connections_enabled(const bool p_value) {
	navmesh_edge_connections_enabled = p_value;
	navigation_debug_dirty = true;
}

bool NavigationDebug3D::get_navmesh_edge_connections_enabled() {
	return navmesh_edge_connections_enabled;
}

void NavigationDebug3D::set_navmesh_edge_connections_enabled_xray(const bool p_value) {
	navmesh_edge_connections_enabled_xray = p_value;
	if (navmesh_edge_connections_material.is_valid()) {
		navmesh_edge_connections_material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, navmesh_edge_connections_enabled_xray);
	}
}

bool NavigationDebug3D::get_navmesh_edge_connections_enabled_xray() {
	return navmesh_edge_connections_enabled_xray;
}

void NavigationDebug3D::set_navmesh_edge_lines_enabled(const bool p_value) {
	navmesh_edge_lines_enabled = p_value;
	navigation_debug_dirty = true;
}

bool NavigationDebug3D::get_navmesh_edge_lines_enabled() {
	return navmesh_edge_lines_enabled;
}

void NavigationDebug3D::set_navmesh_edge_lines_enabled_xray(const bool p_value) {
	navmesh_edge_lines_enabled_xray = p_value;
	if (navmesh_geometry_edge_material.is_valid()) {
		navmesh_geometry_edge_material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, navmesh_edge_lines_enabled_xray);
	}
}

bool NavigationDebug3D::get_navmesh_edge_lines_enabled_xray() {
	return navmesh_edge_lines_enabled_xray;
}

void NavigationDebug3D::set_navmesh_geometry_face_random_color_enabled(const bool p_value) {
	navmesh_geometry_face_random_color_enabled = p_value;
	navigation_debug_dirty = true;
}

bool NavigationDebug3D::get_navmesh_geometry_face_random_color_enabled() {
	return navmesh_geometry_face_random_color_enabled;
}

void NavigationDebug3D::set_navlink_link_connections_enabled(const bool p_value) {
	navlink_link_connections_enabled = p_value;
	navigation_debug_dirty = true;
}

bool NavigationDebug3D::get_navlink_link_connections_enabled() {
	return navlink_link_connections_enabled;
}

void NavigationDebug3D::set_navlink_link_connections_enabled_xray(const bool p_value) {
	navlink_link_connections_enabled_xray = p_value;
	if (navlink_connections_material.is_valid()) {
		navlink_connections_material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, navlink_link_connections_enabled_xray);
	}
}

bool NavigationDebug3D::get_navlink_link_connections_enabled_xray() {
	return navlink_link_connections_enabled_xray;
}

void NavigationDebug3D::set_navagent_avoidance_radius_enabled(const bool p_value) {
	navagent_avoidance_radius_enabled = p_value;
	avoidance_debug_dirty = true;
}

bool NavigationDebug3D::get_navagent_avoidance_radius_enabled() {
	return navagent_avoidance_radius_enabled;
}

void NavigationDebug3D::set_navobstacle_avoidance_radius_enabled(const bool p_value) {
	navobstacle_avoidance_radius_enabled = p_value;
	avoidance_debug_dirty = true;
}

bool NavigationDebug3D::get_navobstacle_avoidance_radius_enabled() {
	return navobstacle_avoidance_radius_enabled;
}

void NavigationDebug3D::set_navobstacle_avoidance_static_enabled(const bool p_value) {
	navobstacle_avoidance_static_enabled = p_value;
	avoidance_debug_dirty = true;
}

bool NavigationDebug3D::get_navobstacle_avoidance_static_enabled() {
	return navobstacle_avoidance_static_enabled;
}

void NavigationDebug3D::set_navagent_avoidance_radius_color(const Color &p_color) {
	navagent_avoidance_radius_color = p_color;
	if (navagent_avoidance_radius_material.is_valid()) {
		navagent_avoidance_radius_material->set_albedo(navagent_avoidance_radius_color);
	}
}

Color NavigationDebug3D::get_navagent_avoidance_radius_color() {
	return navagent_avoidance_radius_color;
}

void NavigationDebug3D::set_navobstacle_avoidance_radius_color(const Color &p_color) {
	navobstacle_avoidance_radius_color = p_color;
	if (navobstacle_avoidance_radius_material.is_valid()) {
		navobstacle_avoidance_radius_material->set_albedo(navobstacle_avoidance_radius_color);
	}
}

Color NavigationDebug3D::get_navobstacle_avoidance_radius_color() {
	return navobstacle_avoidance_radius_color;
}

void NavigationDebug3D::set_navobstacle_avoidance_static_pushin_face_color(const Color &p_color) {
	navobstacle_avoidance_static_pushin_face_color = p_color;
	if (navobstacle_avoidance_static_pushin_face_material.is_valid()) {
		navobstacle_avoidance_static_pushin_face_material->set_albedo(navobstacle_avoidance_static_pushin_face_color);
	}
}

Color NavigationDebug3D::get_navobstacle_avoidance_static_pushin_face_color() {
	return navobstacle_avoidance_static_pushin_face_color;
}

void NavigationDebug3D::set_navobstacle_avoidance_static_pushin_pushout_face_color(const Color &p_color) {
	navobstacle_avoidance_static_pushin_pushout_face_color = p_color;
	if (navobstacle_avoidance_static_pushout_face_material.is_valid()) {
		navobstacle_avoidance_static_pushout_face_material->set_albedo(navobstacle_avoidance_static_pushin_pushout_face_color);
	}
}

Color NavigationDebug3D::get_navobstacle_avoidance_static_pushin_pushout_face_color() {
	return navobstacle_avoidance_static_pushin_pushout_face_color;
}

void NavigationDebug3D::set_navobstacle_avoidance_static_pushin_pushin_edge_color(const Color &p_color) {
	navobstacle_avoidance_static_pushin_pushin_edge_color = p_color;
	if (navobstacle_avoidance_static_pushin_edge_material.is_valid()) {
		navobstacle_avoidance_static_pushin_edge_material->set_albedo(navobstacle_avoidance_static_pushin_pushin_edge_color);
	}
}

Color NavigationDebug3D::get_navobstacle_avoidance_static_pushin_pushin_edge_color() {
	return navobstacle_avoidance_static_pushin_pushin_edge_color;
}

void NavigationDebug3D::set_navobstacle_avoidance_static_pushin_pushout_edge_color(const Color &p_color) {
	navobstacle_avoidance_static_pushin_pushout_edge_color = p_color;
	if (navobstacle_avoidance_static_pushout_edge_material.is_valid()) {
		navobstacle_avoidance_static_pushout_edge_material->set_albedo(navobstacle_avoidance_static_pushin_pushout_edge_color);
	}
}

Color NavigationDebug3D::get_navobstacle_avoidance_static_pushin_pushout_edge_color() {
	return navobstacle_avoidance_static_pushin_pushout_edge_color;
}

void NavigationDebug3D::set_navagent_paths_enabled(const bool p_value) {
	if (navagent_paths_enabled == p_value) {
		return;
	}

	navagent_paths_enabled = p_value;
	navigation_debug_dirty = true;
}

bool NavigationDebug3D::get_navagent_paths_enabled() {
	return navagent_paths_enabled;
}

void NavigationDebug3D::set_navagent_paths_enabled_xray(const bool p_value) {
	navagent_paths_enabled_xray = p_value;
	if (navagent_path_line_material.is_valid()) {
		navagent_path_line_material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, navagent_paths_enabled_xray);
	}
	if (navagent_path_point_material.is_valid()) {
		navagent_path_point_material->set_flag(StandardMaterial3D::FLAG_DISABLE_DEPTH_TEST, navagent_paths_enabled_xray);
	}
}

bool NavigationDebug3D::get_navagent_paths_enabled_xray() {
	return navagent_paths_enabled_xray;
}

NavigationDebug3D::NavigationDebug3D() {}

void NavigationDebug3D::init() {
	navmesh_edge_connection_color = GLOBAL_DEF("navigation/3d/debug/navmesh/edge_connection_color", Color(1.0, 0.0, 1.0, 1.0));
	navmesh_geometry_edge_color = GLOBAL_DEF("navigation/3d/debug/navmesh/geometry_edge_color", Color(0.5, 1.0, 1.0, 1.0));
	navmesh_geometry_face_color = GLOBAL_DEF("navigation/3d/debug/navmesh/geometry_face_color", Color(0.5, 1.0, 1.0, 0.4));
	navmesh_geometry_edge_disabled_color = GLOBAL_DEF("navigation/3d/debug/navmesh/geometry_edge_disabled_color", Color(0.5, 0.5, 0.5, 1.0));
	navmesh_geometry_face_disabled_color = GLOBAL_DEF("navigation/3d/debug/navmesh/geometry_face_disabled_color", Color(0.5, 0.5, 0.5, 0.4));
	navlink_connection_color = GLOBAL_DEF("navigation/3d/debug/links/connection_color", Color(1.0, 0.5, 1.0, 1.0));
	navlink_connection_disabled_color = GLOBAL_DEF("navigation/3d/debug/links/connection_disabled_color", Color(0.5, 0.5, 0.5, 1.0));
	navagent_path_color = GLOBAL_DEF("navigation/3d/debug/agents/path_color", Color(1.0, 0.0, 0.0, 1.0));

	navmesh_edge_connections_enabled = GLOBAL_DEF("navigation/3d/debug/navmesh/enable_edge_connections", true);
	navmesh_edge_connections_enabled_xray = GLOBAL_DEF("navigation/3d/debug/navmesh/enable_edge_connections_xray", true);
	navmesh_edge_lines_enabled = GLOBAL_DEF("navigation/3d/debug/navmesh/enable_edge_lines", true);
	navmesh_edge_lines_enabled_xray = GLOBAL_DEF("navigation/3d/debug/navmesh/enable_edge_lines_xray", true);
	navmesh_geometry_face_random_color_enabled = GLOBAL_DEF("navigation/3d/debug/navmesh/enable_geometry_face_random_color", true);
	navlink_link_connections_enabled = GLOBAL_DEF("navigation/3d/debug/links/enable_connections", true);
	navlink_link_connections_enabled_xray = GLOBAL_DEF("navigation/3d/debug/links/enable_connections_xray", true);

	navagent_paths_enabled = GLOBAL_DEF("navigation/3d/debug/agents/enable_paths", true);
	navagent_paths_enabled_xray = GLOBAL_DEF("navigation/3d/debug/agents/enable_paths_xray", true);
	navagent_path_point_size = GLOBAL_DEF(PropertyInfo(Variant::FLOAT, "navigation/3d/debug/agents/path_point_size", PROPERTY_HINT_RANGE, "0.01,10,0.001,or_greater"), 4.0);

	navagent_avoidance_radius_color = GLOBAL_DEF("navigation/3d/debug/agents/radius_color", Color(1.0, 1.0, 0.0, 0.25));
	navobstacle_avoidance_radius_color = GLOBAL_DEF("navigation/3d/debug/obstacles/radius_color", Color(1.0, 0.5, 0.0, 0.25));
	navobstacle_avoidance_static_pushin_face_color = GLOBAL_DEF("navigation/3d/debug/obstacles/static_face_pushin_color", Color(1.0, 0.0, 0.0, 0.0));
	navobstacle_avoidance_static_pushin_pushin_edge_color = GLOBAL_DEF("navigation/3d/debug/obstacles/static_edge_pushin_color", Color(1.0, 0.0, 0.0, 1.0));
	navobstacle_avoidance_static_pushin_pushout_face_color = GLOBAL_DEF("navigation/3d/debug/obstacles/static_face_pushout_color", Color(1.0, 1.0, 0.0, 0.5));
	navobstacle_avoidance_static_pushin_pushout_edge_color = GLOBAL_DEF("navigation/3d/debug/obstacles/static_edge_pushout_color", Color(1.0, 1.0, 0.0, 1.0));
	navagent_avoidance_radius_enabled = GLOBAL_DEF("navigation/3d/debug/agents/enable_radius", true);
	navobstacle_avoidance_radius_enabled = GLOBAL_DEF("navigation/3d/debug/obstacles/enable_radius", true);
	navobstacle_avoidance_static_enabled = GLOBAL_DEF("navigation/3d/debug/obstacles/enable_static", true);
}

void NavigationDebug3D::finish() {
	navmesh_geometry_edge_material.unref();
	navmesh_geometry_face_material.unref();
	navmesh_geometry_edge_disabled_material.unref();
	navmesh_geometry_face_disabled_material.unref();
	navmesh_edge_connections_material.unref();
	navlink_connections_material.unref();
	navlink_connections_disabled_material.unref();
	navagent_avoidance_radius_material.unref();
	navobstacle_avoidance_radius_material.unref();

	navobstacle_avoidance_static_pushin_face_material.unref();
	navobstacle_avoidance_static_pushout_face_material.unref();
	navobstacle_avoidance_static_pushin_edge_material.unref();
	navobstacle_avoidance_static_pushout_edge_material.unref();

	navagent_path_line_material.unref();
	navagent_path_point_material.unref();
}

NavigationDebug3D::~NavigationDebug3D() {
}

#endif // DEBUG_ENABLED
