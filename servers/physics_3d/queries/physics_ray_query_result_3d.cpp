/**************************************************************************/
/*  physics_ray_query_result_3d.cpp                                       */
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

#include "physics_ray_query_result_3d.h"

#include "core/object/class_db.h"

uint32_t PhysicsRayQueryResult3D::size() const {
	RWLockRead read_lock(data_rwlock);
	return results.size();
}

bool PhysicsRayQueryResult3D::is_empty() const {
	RWLockRead read_lock(data_rwlock);
	return results.is_empty();
}

Vector3 PhysicsRayQueryResult3D::get_collision_point(uint32_t p_index) const {
	RWLockRead read_lock(data_rwlock);
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, results.size(), Vector3());
	return results[p_index].position;
}

Vector3 PhysicsRayQueryResult3D::get_collision_normal(uint32_t p_index) const {
	RWLockRead read_lock(data_rwlock);
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, results.size(), Vector3());
	return results[p_index].normal;
}

ObjectID PhysicsRayQueryResult3D::get_collider_id(uint32_t p_index) const {
	RWLockRead read_lock(data_rwlock);
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, results.size(), ObjectID());
	return results[p_index].collider_id;
}

RID PhysicsRayQueryResult3D::get_collider_rid(uint32_t p_index) const {
	RWLockRead read_lock(data_rwlock);
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, results.size(), RID());
	return results[p_index].rid;
}

int PhysicsRayQueryResult3D::get_collider_shape_index(uint32_t p_index) const {
	RWLockRead read_lock(data_rwlock);
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, results.size(), -1);
	return results[p_index].shape;
}

RID PhysicsRayQueryResult3D::get_collider_shape_rid(uint32_t p_index) const {
	RWLockRead read_lock(data_rwlock);
	ERR_FAIL_UNSIGNED_INDEX_V(p_index, results.size(), RID());
	return results[p_index].shape_rid;
}

void PhysicsRayQueryResult3D::set_data(const LocalVector<PS3DT::RayResult> &p_results) {
	RWLockWrite write_lock(data_rwlock);
	results.clear();
	results.resize(p_results.size());

	const PS3DT::RayResult *p_results_ptr = p_results.ptr();
	PS3DT::RayResult *results_ptrw = results.ptr();

	for (uint32_t i = 0; i < results.size(); i++) {
		results_ptrw[i] = p_results_ptr[i];
	}
}

void PhysicsRayQueryResult3D::_bind_methods() {
	ClassDB::bind_method(D_METHOD("size"), &PhysicsRayQueryResult3D::size);
	ClassDB::bind_method(D_METHOD("is_empty"), &PhysicsRayQueryResult3D::is_empty);

	ClassDB::bind_method(D_METHOD("get_collision_point", "index"), &PhysicsRayQueryResult3D::get_collision_point);
	ClassDB::bind_method(D_METHOD("get_collision_normal", "index"), &PhysicsRayQueryResult3D::get_collision_normal);

	ClassDB::bind_method(D_METHOD("get_collider_id", "index"), &PhysicsRayQueryResult3D::get_collider_id);
	ClassDB::bind_method(D_METHOD("get_collider_rid", "index"), &PhysicsRayQueryResult3D::get_collider_rid);

	ClassDB::bind_method(D_METHOD("get_collider_shape_index", "index"), &PhysicsRayQueryResult3D::get_collider_shape_index);
	ClassDB::bind_method(D_METHOD("get_collider_shape_rid", "index"), &PhysicsRayQueryResult3D::get_collider_shape_rid);
}
