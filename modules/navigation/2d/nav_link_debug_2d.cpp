/**************************************************************************/
/*  nav_link_debug_2d.cpp                                                 */
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

#include "nav_link_debug_2d.h"

#include "../nav_link.h"
#include "../nav_map.h"

#include "servers/rendering_server.h"
#include "servers/navigation_server_2d.h"

void NavLinkDebug2D::debug_set_enabled(bool p_enabled) {
	if (debug_enabled  == p_enabled) {
		return;
	}

	debug_enabled = p_enabled;

	debug_mesh_dirty = true;
	debug_update_mesh();
}

void NavLinkDebug2D::debug_update() {
	debug_update_canvas();
	//debug_update_transform();
	//debug_update_mesh();
	//debug_update_material();
}

void NavLinkDebug2D::debug_update_canvas() {
	ERR_FAIL_COND(!debug_canvas_item_rid.is_valid());

	if (link->map && link->map->get_debug_2d()->debug_get_canvas().is_valid()) {
		RenderingServer::get_singleton()->canvas_item_set_parent(debug_canvas_item_rid, link->map->get_debug_2d()->debug_get_canvas());
	} else {
		RenderingServer::get_singleton()->canvas_item_set_parent(debug_canvas_item_rid, RID());
	}
}

void NavLinkDebug2D::debug_update_transform() {
	if (!debug_transform_dirty) {
		return;
	}
	debug_transform_dirty = false;
};

void NavLinkDebug2D::debug_update_mesh() {
	if (!debug_mesh_dirty) {
		return;
	}
	debug_mesh_dirty = false;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	ERR_FAIL_COND(!debug_canvas_item_rid.is_valid());
	rs->canvas_item_clear(debug_canvas_item_rid);

	if (!debug_enabled) {
		return;
	}

	ERR_FAIL_NULL(link->map);

	rs->canvas_item_set_parent(debug_canvas_item_rid, link->map->get_debug_2d()->debug_get_canvas());

	const float search_radius = link->map->get_link_connection_radius();

	NavigationServer2D *ns2d = NavigationServer2D::get_singleton();
	ERR_FAIL_NULL(ns2d);

	Color color;
	if (link->enabled) {
		color = ns2d->get_debug_navigation_link_connection_color();
	} else {
		color = ns2d->get_debug_navigation_link_connection_disabled_color();
	}

	const Vector3 link_start_position = link->start_position;
	const Vector3 link_end_position = link->end_position;

	Vector2 start_position_vec2 = Vector2(link_start_position.x, link_start_position.z);
	Vector2 end_position_vec2 = Vector2(link_end_position.x, link_end_position.z);

	rs->canvas_item_add_line(debug_canvas_item_rid, start_position_vec2, end_position_vec2, color);
	rs->canvas_item_add_circle(debug_canvas_item_rid, start_position_vec2, search_radius, color);
	rs->canvas_item_add_circle(debug_canvas_item_rid, end_position_vec2, search_radius, color);

	debug_material_dirty = true;
	debug_update_material();
	debug_update_canvas();
}

void NavLinkDebug2D::debug_update_material() {
	if (!debug_material_dirty) {
		return;
	}
	debug_material_dirty = false;
}

void NavLinkDebug2D::debug_free() {
	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	if (debug_canvas_item_rid.is_valid()) {
		rs->free(debug_canvas_item_rid);
		debug_canvas_item_rid = RID();
	}
}

void NavLinkDebug2D::sync() {
	
}

void NavLinkDebug2D::request_sync() {
	if (link->map && !sync_dirty_request_list_element.in_list()) {
		link->map->get_debug_2d()->add_link_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

void NavLinkDebug2D::cancel_sync_request() {
	if (link->map && sync_dirty_request_list_element.in_list()) {
		link->map->get_debug_2d()->remove_link_sync_dirty_request(&sync_dirty_request_list_element);
	}
}

NavLinkDebug2D::NavLinkDebug2D(NavLink *p_link) :
		sync_dirty_request_list_element(this) {
	ERR_FAIL_NULL(p_link);
	link = p_link;

	RenderingServer *rs = RenderingServer::get_singleton();
	ERR_FAIL_NULL(rs);

	debug_canvas_item_rid = rs->canvas_item_create();
}

NavLinkDebug2D::~NavLinkDebug2D() {
	cancel_sync_request();
	debug_free();
}
