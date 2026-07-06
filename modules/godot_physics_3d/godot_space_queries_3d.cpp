/**************************************************************************/
/*  godot_space_queries_3d.cpp                                            */
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

#include "godot_space_queries_3d.h"

#include "godot_space_3d.h"

#include "core/variant/typed_array.h"
#include "servers/physics_3d/queries/physics_point_query_parameters_3d.h"
#include "servers/physics_3d/queries/physics_point_query_result_3d.h"
#include "servers/physics_3d/queries/physics_ray_query_parameters_3d.h"
#include "servers/physics_3d/queries/physics_ray_query_result_3d.h"
#include "servers/physics_3d/queries/physics_shape_query_parameters_3d.h"
#include "servers/physics_3d/queries/physics_shape_query_result_3d.h"

void GodotSpaceQueries3D::space_intersect_point(GodotSpace3D *p_space, const Ref<PhysicsPointQueryParameters3D> &p_query_parameters, Ref<PhysicsPointQueryResult3D> p_query_result) {
	ERR_FAIL_NULL(p_space);
	ERR_FAIL_COND(p_query_parameters.is_null());
	ERR_FAIL_COND(p_query_result.is_null());

	GodotPhysicsDirectSpaceState3D *space_state = p_space->get_direct_state();
	ERR_FAIL_NULL(space_state);

	const uint32_t results_max = MAX(p_query_parameters->get_results_max(), uint32_t(1));

	// The route through DirectSpace here with all that copying is a temporary solution.

	Vector<PS3DT::ShapeResult> legacy_results;
	legacy_results.resize(results_max);

	int results_count = space_state->intersect_point(p_query_parameters->get_parameters(), legacy_results.ptrw(), legacy_results.size());

	LocalVector<PS3DT::ShapeResult> results;

	if (results_count > 0) {
		results.resize(results_count);

		const PS3DT::ShapeResult *legacy_results_ptr = legacy_results.ptr();
		PS3DT::ShapeResult *results_ptrw = results.ptr();

		for (uint32_t i = 0; i < results.size(); i++) {
			results_ptrw[i] = legacy_results_ptr[i];
		}
	}

	p_query_result->set_data(results);
}

void GodotSpaceQueries3D::space_intersect_ray(GodotSpace3D *p_space, const Ref<PhysicsRayQueryParameters3D> &p_query_parameters, Ref<PhysicsRayQueryResult3D> p_query_result) {
	ERR_FAIL_NULL(p_space);
	ERR_FAIL_COND(p_query_parameters.is_null());
	ERR_FAIL_COND(p_query_result.is_null());

	GodotPhysicsDirectSpaceState3D *space_state = p_space->get_direct_state();
	ERR_FAIL_NULL(space_state);

	const uint32_t results_max = MAX(p_query_parameters->get_results_max(), uint32_t(1));

	// The route through DirectSpace here with all that copying is a temporary solution.

	if (results_max == 1) {
		PS3DT::RayResult result;

		bool has_collided = space_state->intersect_ray(p_query_parameters->get_parameters(), result);

		LocalVector<PS3DT::RayResult> results;
		if (has_collided) {
			results.push_back(result);
		}
		p_query_result->set_data(results);
		return;
	}

	// Need to copy parameters here to not override the user array
	// when we add already hit objects to the exclude filter.
	PS3DT::RayParameters _parameters;
	_parameters.from = p_query_parameters->get_from();
	_parameters.to = p_query_parameters->get_to();
	_parameters.collision_mask = p_query_parameters->get_collision_mask();
	_parameters.collide_with_areas = p_query_parameters->is_collide_with_areas_enabled();
	_parameters.collide_with_bodies = p_query_parameters->is_collide_with_bodies_enabled();
	_parameters.hit_from_inside = p_query_parameters->is_hit_from_inside_enabled();
	const TypedArray<RID> &excluded_rids = p_query_parameters->get_exclude();
	for (const RID excluded_rid : excluded_rids) {
		_parameters.exclude.insert(excluded_rid);
	}

	LocalVector<PS3DT::RayResult> results;
	results.reserve(results_max);

	bool intersected = true;
	while (intersected && results.size() < results_max) {
		PS3DT::RayResult result;
		intersected = space_state->intersect_ray(_parameters, result);
		if (intersected) {
			results.push_back(result);
			_parameters.exclude.insert(result.rid);
		}
	}

	p_query_result->set_data(results);
}

void GodotSpaceQueries3D::space_intersect_shape(GodotSpace3D *p_space, const Ref<PhysicsShapeQueryParameters3D> &p_query_parameters, Ref<PhysicsShapeQueryResult3D> p_query_result) {
	ERR_FAIL_NULL(p_space);
	ERR_FAIL_COND(p_query_parameters.is_null());
	ERR_FAIL_COND(p_query_result.is_null());

	GodotPhysicsDirectSpaceState3D *space_state = p_space->get_direct_state();
	ERR_FAIL_NULL(space_state);

	const uint32_t results_max = MAX(p_query_parameters->get_results_max(), uint32_t(1));

	// The route through DirectSpace here with all that copying is a temporary solution.

	Vector<PS3DT::ShapeResult> legacy_results;
	legacy_results.resize(results_max);

	int results_count = space_state->intersect_shape(p_query_parameters->get_parameters(), legacy_results.ptrw(), legacy_results.size());

	LocalVector<PS3DT::ShapeResult> results;

	if (results_count > 0) {
		results.resize(results_count);

		const PS3DT::ShapeResult *legacy_results_ptr = legacy_results.ptr();
		PS3DT::ShapeResult *results_ptrw = results.ptr();

		for (uint32_t i = 0; i < results.size(); i++) {
			results_ptrw[i] = legacy_results_ptr[i];
		}
	}

	p_query_result->set_data(results);
}
