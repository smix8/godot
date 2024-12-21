/**************************************************************************/
/*  nav_map_debug_3d.h                                                    */
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

#ifndef NAV_MAP_DEBUG_3D_H
#define NAV_MAP_DEBUG_3D_H

#include "core/object/class_db.h"
#include "core/templates/self_list.h"

class NavMap;
class NavLinkDebug3D;
class NavRegionDebug3D;
class NavAgentDebug3D;
class NavObstacleDebug3D;

class NavMapDebug3D {
	NavMap *map;

	RID debug_scenario;

	RID debug_mesh_rid;
	RID debug_instance_rid;

	bool debug_enabled = true;
	bool debug_navigation_enabled = true;
	bool debug_avoidance_enabled = true;

	bool debug_canvas_dirty = false;
	bool debug_scenario_dirty = false;

	bool debug_canvas_item_dirty = false;
	bool debug_mesh_dirty = false;
	bool debug_material_dirty = false;

	struct {
		SelfList<NavRegionDebug3D>::List regions;
		SelfList<NavLinkDebug3D>::List links;
		//SelfList<NavAgentDebug3D>::List agents;
		SelfList<NavObstacleDebug3D>::List obstacles;
	} sync_dirty_requests;

public:
	void debug_set_scenario(RID p_scenario);
	RID debug_get_scenario() const;

	void debug_update();
	void debug_update_scenario();
	void debug_update_mesh();
	void debug_update_material();
	void debug_make_dirty() {
		//debug_transform_dirty = true;
		debug_mesh_dirty = true;
		debug_material_dirty = true;
	};
	void debug_render_edge_connections();
	void debug_set_enabled(bool p_enabled);
	void debug_set_navigation_enabled(bool p_enabled);
	bool debug_is_navigation_enabled() const { return debug_enabled && debug_navigation_enabled; };
	void debug_set_avoidance_enabled(bool p_enabled);
	bool debug_is_avoidance_enabled() const { return debug_enabled && debug_avoidance_enabled; };
	void debug_free();

	bool debug_settings_dirty = true;
	bool project_settings_dirty = true;
	void project_settings_changed();

	//void debug_update_queue_region(NavRegion *p_region, bool p_transform, bool p_mesh, bool p_material);
	//void debug_update_queue_link(NavLink *p_link, bool p_transform, bool p_mesh, bool p_material);
	//void debug_update_queue_obstacle(NavObstacle *p_obstacle, bool p_transform, bool p_mesh, bool p_material);

	void _sync_dirty_update_requests();

	void add_region_sync_dirty_request(SelfList<NavRegionDebug3D> *p_sync_request);
	void add_link_sync_dirty_request(SelfList<NavLinkDebug3D> *p_sync_request);
	//void add_agent_sync_dirty_request(SelfList<NavAgentDebug3D> *p_sync_request);
	void add_obstacle_sync_dirty_request(SelfList<NavObstacleDebug3D> *p_sync_request);

	void remove_region_sync_dirty_request(SelfList<NavRegionDebug3D> *p_sync_request);
	void remove_link_sync_dirty_request(SelfList<NavLinkDebug3D> *p_sync_request);
	//void remove_agent_sync_dirty_request(SelfList<NavAgentDebug3D> *p_sync_request);
	void remove_obstacle_sync_dirty_request(SelfList<NavObstacleDebug3D> *p_sync_request);

public:
	void sync();

	NavMapDebug3D(NavMap *p_map);
	~NavMapDebug3D();
};

#endif // NAV_MAP_DEBUG_3D_H
