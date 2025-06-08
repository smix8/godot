/**************************************************************************/
/*  nav_pathfinder_3d.h                                                   */
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

#ifndef _3D_DISABLED

#include "nav_map.h"
#include "nav_utils.h"

#include "core/object/class_db.h"
#include "core/object/worker_thread_pool.h"
#include "core/variant/callable.h"
#include "servers/navigation/navigation_path_query_parameters_3d.h"
#include "servers/navigation/navigation_path_query_result_3d.h"
#include "servers/navigation/navigation_utilities.h"

class NavPathfinder3D : public Object {
	friend class NavMap;

	static NavPathfinder3D *singleton;

	static Mutex updating_query_results_mutex;
	static Mutex pathfinder_task_mutex;

	static bool use_threads;
	static bool pathfinding_use_multiple_threads;

	struct NavMapPathfinderTask3D {
		enum TaskStatus {
			PATHFINDING_REQUIRED,
			PATHFINDING_STARTED,
			PATHFINDING_FINISHED,
			PATHFINDING_FAILED,
			CALLBACK_DISPATCHED,
			CALLBACK_FAILED,
		};

		Ref<NavigationPathQueryParameters3D> query_parameters;
		Ref<NavigationPathQueryResult3D> query_result;
		NavigationUtilities::PathQueryParameters parameters;
		NavigationUtilities::PathQueryResult result;
		Callable callback = Callable();
		NavMap *map = nullptr;
		NavMapPathfinderTask3D::TaskStatus status = NavMapPathfinderTask3D::TaskStatus::PATHFINDING_REQUIRED;
	};

	//LocalVector<Ref<NavigationPathQueryResult3D>> processing_pathquery_results;

	HashMap<NavMapPathfinderTask3D *, WorkerThreadPool::TaskID> pathfinding_task_to_threadpool_task_id;
	static HashMap<WorkerThreadPool::TaskID, NavMapPathfinderTask3D *> pathfinder_tasks;

	static bool pathfinder_emit_callback(const Callable &p_callback);

	static void pathfindering_find_path_astar(NavMapPathfinderTask3D *pathfinder_task);

	static HashSet<Ref<NavigationPathQueryResult3D>> updating_query_results;

	static void _pathfinder_thread_query_path(void *p_arg);

	Vector<Vector3> get_legacy_path(Vector3 p_origin, Vector3 p_destination, bool p_optimize, uint32_t p_navigation_layers);

public:
	static NavPathfinder3D *get_singleton();

	static void init();
	static void sync();
	static void finish();

	static void query_path(Ref<NavigationPathQueryParameters3D> p_query_parameters, Ref<NavigationPathQueryResult3D> p_query_result, const Callable &p_callback = Callable());

	NavPathfinder3D();
	~NavPathfinder3D();
};

#endif // _3D_DISABLED

#endif // NAV_PATHFINDER_3D_H
