/**************************************************************************/
/*  navigation_path_query_result_2d.cpp                                   */
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

#include "navigation_path_query_result_2d.h"

#include "core/core_string_names.h"

void NavigationPathQueryResult2D::emit_changed() {
	emit_signal(CoreStringNames::get_singleton()->changed);
}

void NavigationPathQueryResult2D::emit_pending_update() {
	emit_signal(SNAME("pending_update"));
}

void NavigationPathQueryResult2D::set_path(const Vector<Vector2> &p_path) {
	RWLockWrite r(rwlock);
	path = p_path;
}

const Vector<Vector2> &NavigationPathQueryResult2D::get_path() const {
	RWLockRead r(rwlock);
	return path;
}

void NavigationPathQueryResult2D::set_path_types(const Vector<int32_t> &p_path_types) {
	RWLockWrite r(rwlock);
	path_types = p_path_types;
}

const Vector<int32_t> &NavigationPathQueryResult2D::get_path_types() const {
	RWLockRead r(rwlock);
	return path_types;
}

void NavigationPathQueryResult2D::set_path_rids(const TypedArray<RID> &p_path_rids) {
	RWLockWrite r(rwlock);
	path_rids = p_path_rids;
}

TypedArray<RID> NavigationPathQueryResult2D::get_path_rids() const {
	RWLockRead r(rwlock);
	return path_rids;
}

void NavigationPathQueryResult2D::set_path_owner_ids(const Vector<int64_t> &p_path_owner_ids) {
	RWLockWrite r(rwlock);
	path_owner_ids = p_path_owner_ids;
}

const Vector<int64_t> &NavigationPathQueryResult2D::get_path_owner_ids() const {
	RWLockRead r(rwlock);
	return path_owner_ids;
}

void NavigationPathQueryResult2D::reset() {
	RWLockWrite r(rwlock);

	path.clear();
	path_types.clear();
	path_rids.clear();
	path_owner_ids.clear();

	pending_path.clear();
	pending_path_types.clear();
	pending_path_rids.clear();
	pending_path_owner_ids.clear();
}

bool NavigationPathQueryResult2D::has_pending_update() {
	RWLockRead r(rwlock);
	return requires_sync;
}

void NavigationPathQueryResult2D::sync() {
	if (!requires_sync) {
		return;
	}
	rwlock.write_lock();

	requires_sync = false;

	SWAP(path, pending_path);
	SWAP(path_types, pending_path_types);
	SWAP(path_rids, pending_path_rids);
	SWAP(path_owner_ids, pending_path_owner_ids);

	pending_path.clear();
	pending_path_types.clear();
	pending_path_rids.clear();
	pending_path_owner_ids.clear();

	rwlock.write_unlock();

	emit_changed();
}

void NavigationPathQueryResult2D::set_pending_update(const NavigationUtilities::PathQueryResult &p_query_result) {
	rwlock.write_lock();

	pending_path.clear();
	pending_path.resize(p_query_result.path.size());
	for (int i(0); i < p_query_result.path.size(); i++) {
		pending_path.write[i] = Vector2(p_query_result.path[i].x, p_query_result.path[i].z);
	}
	//pending_path = p_query_resultget_path_as_2d();
	pending_path_types = p_query_result.path_types;
	pending_path_rids = p_query_result.path_rids;
	pending_path_owner_ids = p_query_result.path_owner_ids;
	requires_sync = true;

	rwlock.write_unlock();

	emit_pending_update();
}

void NavigationPathQueryResult2D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("sync"), &NavigationPathQueryResult2D::sync);

	ClassDB::bind_method(D_METHOD("set_path", "path"), &NavigationPathQueryResult2D::set_path);
	ClassDB::bind_method(D_METHOD("get_path"), &NavigationPathQueryResult2D::get_path);

	ClassDB::bind_method(D_METHOD("set_path_types", "path_types"), &NavigationPathQueryResult2D::set_path_types);
	ClassDB::bind_method(D_METHOD("get_path_types"), &NavigationPathQueryResult2D::get_path_types);

	ClassDB::bind_method(D_METHOD("set_path_rids", "path_rids"), &NavigationPathQueryResult2D::set_path_rids);
	ClassDB::bind_method(D_METHOD("get_path_rids"), &NavigationPathQueryResult2D::get_path_rids);

	ClassDB::bind_method(D_METHOD("set_path_owner_ids", "path_owner_ids"), &NavigationPathQueryResult2D::set_path_owner_ids);
	ClassDB::bind_method(D_METHOD("get_path_owner_ids"), &NavigationPathQueryResult2D::get_path_owner_ids);

	ClassDB::bind_method(D_METHOD("reset"), &NavigationPathQueryResult2D::reset);

	ADD_PROPERTY(PropertyInfo(Variant::PACKED_VECTOR2_ARRAY, "path"), "set_path", "get_path");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT32_ARRAY, "path_types"), "set_path_types", "get_path_types");
	ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "path_rids", PROPERTY_HINT_ARRAY_TYPE, "RID"), "set_path_rids", "get_path_rids");
	ADD_PROPERTY(PropertyInfo(Variant::PACKED_INT64_ARRAY, "path_owner_ids"), "set_path_owner_ids", "get_path_owner_ids");

	ADD_SIGNAL(MethodInfo("changed"));
	ADD_SIGNAL(MethodInfo("pending_update"));

	BIND_ENUM_CONSTANT(PATH_SEGMENT_TYPE_REGION);
	BIND_ENUM_CONSTANT(PATH_SEGMENT_TYPE_LINK);
}
