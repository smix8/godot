/**************************************************************************/
/*  physics_point_query_result_2d.h                                       */
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

#include "core/object/ref_counted.h"
#include "core/os/rw_lock.h"
#include "core/templates/local_vector.h"
#include "servers/physics_2d/physics_server_2d_types.h"

class PhysicsPointQueryResult2D : public RefCounted {
	GDCLASS(PhysicsPointQueryResult2D, RefCounted);
	RWLock data_rwlock;

	LocalVector<PS2DT::ShapeResult> results;

protected:
	static void _bind_methods();

public:
	uint32_t size() const;
	bool is_empty() const;

	ObjectID get_collider_id(uint32_t p_index) const;
	RID get_collider_rid(uint32_t p_index) const;

	int get_collider_shape_index(uint32_t p_index) const;
	RID get_collider_shape_rid(uint32_t p_index) const;

	void set_data(const LocalVector<PS2DT::ShapeResult> &p_results);
};
