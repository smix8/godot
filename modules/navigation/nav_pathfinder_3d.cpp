/**************************************************************************/
/*  nav_pathfinder_3d.cpp                                                 */
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

#ifndef _3D_DISABLED

#include "nav_pathfinder_3d.h"

#include "godot_navigation_server.h"

#include "core/config/project_settings.h"
#include "core/variant/callable.h"

#include "servers/navigation/navigation_utilities.h"
#include "servers/navigation_server_3d.h"

using namespace NavigationUtilities;

NavPathfinder3D *NavPathfinder3D::singleton = nullptr;
Mutex NavPathfinder3D::updating_query_results_mutex;
Mutex NavPathfinder3D::pathfinder_task_mutex;
bool NavPathfinder3D::use_threads = true;
bool NavPathfinder3D::pathfinding_use_multiple_threads = true;
HashMap<WorkerThreadPool::TaskID, NavPathfinder3D::NavMapPathfinderTask3D *> NavPathfinder3D::pathfinder_tasks;

NavPathfinder3D *NavPathfinder3D::get_singleton() {
	return singleton;
}

NavPathfinder3D::NavPathfinder3D() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;

	pathfinding_use_multiple_threads = GLOBAL_GET("navigation/pathfinding/thread_model/pathfinding_use_multiple_threads");

	use_threads = true;
}

NavPathfinder3D::~NavPathfinder3D() {
	finish();
}

void NavPathfinder3D::sync() {
}

void NavPathfinder3D::_pathfinder_thread_query_path(void *p_arg) {
	NavMapPathfinderTask3D *pathfinder_task = static_cast<NavMapPathfinderTask3D *>(p_arg);

	pathfindering_find_path_astar(pathfinder_task);

	pathfinder_task->status = NavMapPathfinderTask3D::TaskStatus::PATHFINDING_FINISHED;
}

void NavPathfinder3D::init() {
}

void NavPathfinder3D::finish() {
	pathfinder_task_mutex.lock();

	for (KeyValue<WorkerThreadPool::TaskID, NavMapPathfinderTask3D *> &E : pathfinder_tasks) {
		WorkerThreadPool::get_singleton()->wait_for_task_completion(E.key);
		NavMapPathfinderTask3D *pathfinder_task = E.value;
		memdelete(pathfinder_task);
	}
	pathfinder_tasks.clear();

	pathfinder_task_mutex.unlock();
}

bool NavPathfinder3D::pathfinder_emit_callback(const Callable &p_callback) {
	ERR_FAIL_COND_V(!p_callback.is_valid(), false);

	Callable::CallError ce;
	Variant result;
	p_callback.callp(nullptr, 0, result, ce);

	return ce.error == Callable::CallError::CALL_OK;
}

void NavPathfinder3D::pathfindering_find_path_astar(NavMapPathfinderTask3D *pathfinder_task) {

}

#endif // _3D_DISABLED
