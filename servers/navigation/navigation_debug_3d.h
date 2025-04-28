/**************************************************************************/
/*  navigation_debug_3d.h                                                 */
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

#pragma once

#ifdef DEBUG_ENABLED

#include "scene/resources/material.h"

class NavigationDebug3D {
	static bool debug_global_enabled;

	static bool debug_global_navigation_enabled;
	static bool debug_global_avoidance_enabled;

	static bool debug_global_maps_enabled;
	static bool debug_global_regions_enabled;
	static bool debug_global_links_enabled;
	static bool debug_global_obstacles_enabled;
	static bool debug_global_agents_enabled;

	static bool navigation_debug_dirty;
	static bool avoidance_debug_dirty;

	static Color navmesh_edge_connection_color;
	static Color navmesh_geometry_edge_color;
	static Color navmesh_geometry_face_color;
	static Color navmesh_geometry_edge_disabled_color;
	static Color navmesh_geometry_face_disabled_color;
	static Color navlink_connection_color;
	static Color navlink_connection_disabled_color;
	static Color navagent_path_color;

	static real_t navagent_path_point_size;

	static Color navagent_avoidance_radius_color;
	static Color navobstacle_avoidance_radius_color;

	static Color navobstacle_avoidance_static_pushin_face_color;
	static Color navobstacle_avoidance_static_pushin_pushout_face_color;
	static Color navobstacle_avoidance_static_pushin_pushin_edge_color;
	static Color navobstacle_avoidance_static_pushin_pushout_edge_color;

	static bool navmesh_edge_connections_enabled;
	static bool navmesh_edge_connections_enabled_xray;
	static bool navmesh_edge_lines_enabled;
	static bool navmesh_edge_lines_enabled_xray;
	static bool navmesh_geometry_face_random_color_enabled;
	static bool navlink_link_connections_enabled;
	static bool navlink_link_connections_enabled_xray;
	static bool navagent_paths_enabled;
	static bool navagent_paths_enabled_xray;

	static bool navagent_avoidance_radius_enabled;
	static bool navobstacle_avoidance_radius_enabled;
	static bool navobstacle_avoidance_static_enabled;

	static Ref<StandardMaterial3D> navmesh_geometry_edge_material;
	static Ref<StandardMaterial3D> navmesh_geometry_face_material;
	static Ref<StandardMaterial3D> navmesh_geometry_edge_disabled_material;
	static Ref<StandardMaterial3D> navmesh_geometry_face_disabled_material;
	static Ref<StandardMaterial3D> navmesh_edge_connections_material;
	static Ref<StandardMaterial3D> navlink_connections_material;
	static Ref<StandardMaterial3D> navlink_connections_disabled_material;
	static Ref<StandardMaterial3D> navagent_avoidance_radius_material;
	static Ref<StandardMaterial3D> navobstacle_avoidance_radius_material;

	static Ref<StandardMaterial3D> navobstacle_avoidance_static_pushin_face_material;
	static Ref<StandardMaterial3D> navobstacle_avoidance_static_pushout_face_material;
	static Ref<StandardMaterial3D> navobstacle_avoidance_static_pushin_edge_material;
	static Ref<StandardMaterial3D> navobstacle_avoidance_static_pushout_edge_material;

	static Ref<StandardMaterial3D> navagent_path_line_material;
	static Ref<StandardMaterial3D> navagent_path_point_material;

public:
	static void init();
	static void finish();
	static void sync();

	static void debug_global_set_enabled(bool p_enabled);
	static bool debug_global_is_enabled();

	static void debug_global_set_navigation_enabled(bool p_enabled);
	static bool debug_global_is_navigation_enabled();

	static void debug_global_set_avoidance_enabled(bool p_enabled);
	static bool debug_global_is_avoidance_enabled();

	static void debug_global_set_maps_enabled(bool p_enabled);
	static void debug_global_set_regions_enabled(bool p_enabled);
	static void debug_global_set_links_enabled(bool p_enabled);
	static void debug_global_set_obstacles_enabled(bool p_enabled);
	static void debug_global_set_agents_enabled(bool p_enabled);

	static bool debug_global_are_maps_enabled();
	static bool debug_global_are_regions_enabled();
	static bool debug_global_are_links_enabled();
	static bool debug_global_are_obstacles_enabled();
	static bool debug_global_are_agents_enabled();

	static void set_navmesh_edge_connection_color(const Color &p_color);
	static Color get_navmesh_edge_connection_color();

	static void set_navmesh_geometry_edge_color(const Color &p_color);
	static Color get_navmesh_geometry_edge_color();

	static void set_navmesh_geometry_face_color(const Color &p_color);
	static Color get_navmesh_geometry_face_color();

	static void set_navmesh_geometry_edge_disabled_color(const Color &p_color);
	static Color get_navmesh_geometry_edge_disabled_color();

	static void set_navmesh_geometry_face_disabled_color(const Color &p_color);
	static Color get_navmesh_geometry_face_disabled_color();

	static void set_navlink_connection_color(const Color &p_color);
	static Color get_navlink_connection_color();

	static void set_navlink_connection_disabled_color(const Color &p_color);
	static Color get_navlink_connection_disabled_color();

	static void set_navagent_path_color(const Color &p_color);
	static Color get_navagent_path_color();

	static void set_navagent_avoidance_radius_color(const Color &p_color);
	static Color get_navagent_avoidance_radius_color();

	static void set_navobstacle_avoidance_radius_color(const Color &p_color);
	static Color get_navobstacle_avoidance_radius_color();

	static void set_navobstacle_avoidance_static_pushin_face_color(const Color &p_color);
	static Color get_navobstacle_avoidance_static_pushin_face_color();

	static void set_navobstacle_avoidance_static_pushin_pushout_face_color(const Color &p_color);
	static Color get_navobstacle_avoidance_static_pushin_pushout_face_color();

	static void set_navobstacle_avoidance_static_pushin_pushin_edge_color(const Color &p_color);
	static Color get_navobstacle_avoidance_static_pushin_pushin_edge_color();

	static void set_navobstacle_avoidance_static_pushin_pushout_edge_color(const Color &p_color);
	static Color get_navobstacle_avoidance_static_pushin_pushout_edge_color();

	static void set_navmesh_edge_connections_enabled(const bool p_value);
	static bool get_navmesh_edge_connections_enabled();

	static void set_navmesh_edge_connections_enabled_xray(const bool p_value);
	static bool get_navmesh_edge_connections_enabled_xray();

	static void set_navmesh_edge_lines_enabled(const bool p_value);
	static bool get_navmesh_edge_lines_enabled();

	static void set_navmesh_edge_lines_enabled_xray(const bool p_value);
	static bool get_navmesh_edge_lines_enabled_xray();

	static void set_navmesh_geometry_face_random_color_enabled(const bool p_value);
	static bool get_navmesh_geometry_face_random_color_enabled();

	static void set_navlink_link_connections_enabled(const bool p_value);
	static bool get_navlink_link_connections_enabled();

	static void set_navlink_link_connections_enabled_xray(const bool p_value);
	static bool get_navlink_link_connections_enabled_xray();

	static void set_navagent_paths_enabled(const bool p_value);
	static bool get_navagent_paths_enabled();

	static void set_navagent_paths_enabled_xray(const bool p_value);
	static bool get_navagent_paths_enabled_xray();

	static void set_navagent_path_point_size(real_t p_point_size);
	static real_t get_navagent_path_point_size();

	static void set_navagent_avoidance_radius_enabled(const bool p_value);
	static bool get_navagent_avoidance_radius_enabled();

	static void set_navobstacle_avoidance_radius_enabled(const bool p_value);
	static bool get_navobstacle_avoidance_radius_enabled();

	static void set_navobstacle_avoidance_static_enabled(const bool p_value);
	static bool get_navobstacle_avoidance_static_enabled();

	static Ref<StandardMaterial3D> get_navmesh_geometry_face_material();
	static Ref<StandardMaterial3D> get_navmesh_geometry_edge_material();
	static Ref<StandardMaterial3D> get_navmesh_geometry_face_disabled_material();
	static Ref<StandardMaterial3D> get_navmesh_geometry_edge_disabled_material();
	static Ref<StandardMaterial3D> get_navmesh_edge_connections_material();
	static Ref<StandardMaterial3D> get_navlink_connections_material();
	static Ref<StandardMaterial3D> get_navlink_connections_disabled_material();

	static Ref<StandardMaterial3D> get_navagent_path_line_material();
	static Ref<StandardMaterial3D> get_navagent_path_point_material();

	static Ref<StandardMaterial3D> get_navagent_avoidance_radius_material();
	static Ref<StandardMaterial3D> get_navobstacle_avoidance_radius_material();

	static Ref<StandardMaterial3D> get_navobstacle_avoidance_static_pushin_face_material();
	static Ref<StandardMaterial3D> get_navobstacle_avoidance_static_pushout_face_material();
	static Ref<StandardMaterial3D> get_navobstacle_avoidance_static_pushin_edge_material();
	static Ref<StandardMaterial3D> get_navobstacle_avoidance_static_pushout_edge_material();

	NavigationDebug3D();
	~NavigationDebug3D();
};

#endif // DEBUG_ENABLED
