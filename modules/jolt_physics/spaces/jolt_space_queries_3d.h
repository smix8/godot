/**************************************************************************/
/*  jolt_space_queries_3d.h                                               */
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

class JoltSpace3D;
class PhysicsPointQueryParameters3D;
class PhysicsPointQueryResult3D;
class PhysicsRayQueryParameters3D;
class PhysicsRayQueryResult3D;
class PhysicsShapeQueryParameters3D;
class PhysicsShapeQueryResult3D;

class JoltSpaceQueries3D {
public:
	static void space_intersect_point(JoltSpace3D *p_space, const Ref<PhysicsPointQueryParameters3D> &p_query_parameters, Ref<PhysicsPointQueryResult3D> p_query_result);
	static void space_intersect_ray(JoltSpace3D *p_space, const Ref<PhysicsRayQueryParameters3D> &p_query_parameters, Ref<PhysicsRayQueryResult3D> p_query_result);
	static void space_intersect_shape(JoltSpace3D *p_space, const Ref<PhysicsShapeQueryParameters3D> &p_query_parameters, Ref<PhysicsShapeQueryResult3D> p_query_result);
};
