/**************************************************************************/
/*  nav_obstacle_debug_2d.h                                               */
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

#ifndef NAV_OBSTACLE_DEBUG_2D_H
#define NAV_OBSTACLE_DEBUG_2D_H

#include "core/object/class_db.h"
#include "core/templates/self_list.h"

class NavObstacle;

class NavObstacleDebug2D {
	NavObstacle *obstacle = nullptr;

	RID debug_canvas_item_rid;

	bool debug_enabled = true;
	bool debug_transform_dirty = false;
	bool debug_mesh_dirty = false;
	bool debug_material_dirty = false;

	SelfList<NavObstacleDebug2D> sync_dirty_request_list_element;

public:
	void debug_set_enabled(bool p_enabled);
	void debug_update();
	void debug_update_canvas();
	void debug_update_transform();
	void debug_update_mesh();
	void debug_update_material();
	void debug_make_dirty() {
		debug_transform_dirty = true;
		debug_mesh_dirty = true;
		debug_material_dirty = true;
	};
	void debug_free();

	void sync();
	void request_sync();
	void cancel_sync_request();

	NavObstacleDebug2D(NavObstacle *p_obstacle);
	~NavObstacleDebug2D();
};

#endif // NAV_OBSTACLE_DEBUG_2D_H
