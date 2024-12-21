/**************************************************************************/
/*  nav_region.h                                                          */
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

#ifndef NAV_REGION_H
#define NAV_REGION_H

#include "nav_base.h"
#include "nav_utils.h"

#include "core/os/rw_lock.h"
#include "scene/resources/navigation_mesh.h"

#ifdef DEBUG_ENABLED
#include "2d/nav_region_debug_2d.h"
#include "3d/nav_region_debug_3d.h"
#endif // DEBUG_ENABLED

struct NavRegionIteration;

class NavRegion : public NavBase {
	friend class NavRegionDebug2D;
	friend class NavRegionDebug3D;

	RWLock region_rwlock;

	NavMap *map = nullptr;
	Transform3D transform;
	bool enabled = true;

	bool use_edge_connections = true;

	bool region_dirty = true;
	bool polygons_dirty = true;

	LocalVector<gd::Polygon> navmesh_polygons;

	real_t surface_area = 0.0;
	AABB bounds;

	RWLock navmesh_rwlock;
	bool navmesh_data_dirty = false;
	Vector<Vector3> pending_navmesh_vertices;
	Vector<Vector<int>> pending_navmesh_polygons;

	SelfList<NavRegion> sync_dirty_request_list_element;

	Vector<Vector3> navmesh_vertices;
	Vector<Vector<int>> navmesh_polygons;

public:
	NavRegion();
	~NavRegion();

	void scratch_polygons() {
		polygons_dirty = true;
	}

	void set_enabled(bool p_enabled);
	bool get_enabled() const { return enabled; }

	void set_map(NavMap *p_map);
	NavMap *get_map() const {
		return map;
	}

	void set_use_edge_connections(bool p_enabled);
	bool get_use_edge_connections() const {
		return use_edge_connections;
	}

	void set_transform(Transform3D transform);
	const Transform3D &get_transform() const {
		return transform;
	}

	void set_navigation_mesh(Ref<NavigationMesh> p_navigation_mesh);

	LocalVector<gd::Polygon> const &get_polygons() const {
		return navmesh_polygons;
	}

	Vector3 get_closest_point_to_segment(const Vector3 &p_from, const Vector3 &p_to, bool p_use_collision) const;
	gd::ClosestPointQueryResult get_closest_point_info(const Vector3 &p_point) const;
	Vector3 get_random_point(uint32_t p_navigation_layers, bool p_uniformly) const;

	real_t get_surface_area() const { return surface_area; }
	AABB get_bounds() const { return bounds; }

	bool sync();
	void request_sync();
	void cancel_sync_request();

	void get_iteration_update(NavRegionIteration &r_iteration);

private:
	void update_polygons();

#ifdef DEBUG_ENABLED
private:
	NavRegionDebug3D *debug = nullptr;
	NavRegionDebug2D *debug_2d = nullptr;

	bool debug_enabled = true;

public:
	NavRegionDebug3D *get_debug() { return debug; }
	NavRegionDebug2D *get_debug_2d() { return debug_2d; }
#endif // DEBUG_ENABLED
};

#endif // NAV_REGION_H
