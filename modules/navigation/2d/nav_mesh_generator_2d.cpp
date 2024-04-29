/**************************************************************************/
/*  nav_mesh_generator_2d.cpp                                             */
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

#ifdef CLIPPER2_ENABLED

#include "nav_mesh_generator_2d.h"

#include "core/config/project_settings.h"
#include "scene/resources/2d/navigation_mesh_source_geometry_data_2d.h"
#include "scene/resources/2d/navigation_polygon.h"

#include "thirdparty/clipper2/include/clipper2/clipper.h"
#include "thirdparty/misc/polypartition.h"

NavMeshGenerator2D *NavMeshGenerator2D::singleton = nullptr;
Mutex NavMeshGenerator2D::baking_navmesh_mutex;
Mutex NavMeshGenerator2D::generator_task_mutex;
RWLock NavMeshGenerator2D::generator_rid_rwlock;
bool NavMeshGenerator2D::use_threads = true;
bool NavMeshGenerator2D::baking_use_multiple_threads = true;
bool NavMeshGenerator2D::baking_use_high_priority_threads = true;
HashSet<Ref<NavigationPolygon>> NavMeshGenerator2D::baking_navmeshes;
HashMap<WorkerThreadPool::TaskID, NavMeshGenerator2D::NavMeshGeneratorTask2D *> NavMeshGenerator2D::generator_tasks;
RID_Owner<NavMeshGenerator2D::NavMeshGeometryParser2D> NavMeshGenerator2D::generator_parser_owner;
LocalVector<NavMeshGenerator2D::NavMeshGeometryParser2D *> NavMeshGenerator2D::generator_parsers;

NavMeshGenerator2D *NavMeshGenerator2D::get_singleton() {
	return singleton;
}

NavMeshGenerator2D::NavMeshGenerator2D() {
	ERR_FAIL_COND(singleton != nullptr);
	singleton = this;

	baking_use_multiple_threads = GLOBAL_GET("navigation/baking/thread_model/baking_use_multiple_threads");
	baking_use_high_priority_threads = GLOBAL_GET("navigation/baking/thread_model/baking_use_high_priority_threads");

	// Using threads might cause problems on certain exports or with the Editor on certain devices.
	// This is the main switch to turn threaded navmesh baking off should the need arise.
	use_threads = baking_use_multiple_threads;
}

NavMeshGenerator2D::~NavMeshGenerator2D() {
	cleanup();
}

void NavMeshGenerator2D::sync() {
	if (generator_tasks.size() == 0) {
		return;
	}

	baking_navmesh_mutex.lock();
	generator_task_mutex.lock();

	LocalVector<WorkerThreadPool::TaskID> finished_task_ids;

	for (KeyValue<WorkerThreadPool::TaskID, NavMeshGeneratorTask2D *> &E : generator_tasks) {
		if (WorkerThreadPool::get_singleton()->is_task_completed(E.key)) {
			WorkerThreadPool::get_singleton()->wait_for_task_completion(E.key);
			finished_task_ids.push_back(E.key);

			NavMeshGeneratorTask2D *generator_task = E.value;
			DEV_ASSERT(generator_task->status == NavMeshGeneratorTask2D::TaskStatus::BAKING_FINISHED);

			baking_navmeshes.erase(generator_task->navigation_mesh);
			if (generator_task->callback.is_valid()) {
				generator_emit_callback(generator_task->callback);
			}
			memdelete(generator_task);
		}
	}

	for (WorkerThreadPool::TaskID finished_task_id : finished_task_ids) {
		generator_tasks.erase(finished_task_id);
	}

	generator_task_mutex.unlock();
	baking_navmesh_mutex.unlock();
}

void NavMeshGenerator2D::cleanup() {
	baking_navmesh_mutex.lock();
	generator_task_mutex.lock();

	baking_navmeshes.clear();

	for (KeyValue<WorkerThreadPool::TaskID, NavMeshGeneratorTask2D *> &E : generator_tasks) {
		WorkerThreadPool::get_singleton()->wait_for_task_completion(E.key);
		NavMeshGeneratorTask2D *generator_task = E.value;
		memdelete(generator_task);
	}
	generator_tasks.clear();

	generator_rid_rwlock.write_lock();
	for (NavMeshGeometryParser2D *parser : generator_parsers) {
		generator_parser_owner.free(parser->self);
	}
	generator_parsers.clear();
	generator_rid_rwlock.write_unlock();

	generator_task_mutex.unlock();
	baking_navmesh_mutex.unlock();
}

void NavMeshGenerator2D::finish() {
	cleanup();
}

void NavMeshGenerator2D::parse_source_geometry_data(Ref<NavigationPolygon> p_navigation_mesh, Ref<NavigationMeshSourceGeometryData2D> p_source_geometry_data, Node *p_root_node, const Callable &p_callback) {
	ERR_FAIL_COND(!Thread::is_main_thread());
	ERR_FAIL_COND(!p_navigation_mesh.is_valid());
	ERR_FAIL_NULL(p_root_node);
	ERR_FAIL_COND(!p_root_node->is_inside_tree());
	ERR_FAIL_COND(!p_source_geometry_data.is_valid());

	generator_parse_source_geometry_data(p_navigation_mesh, p_source_geometry_data, p_root_node);

	if (p_callback.is_valid()) {
		generator_emit_callback(p_callback);
	}
}

void NavMeshGenerator2D::bake_from_source_geometry_data(Ref<NavigationPolygon> p_navigation_mesh, Ref<NavigationMeshSourceGeometryData2D> p_source_geometry_data, const Callable &p_callback) {
	ERR_FAIL_COND(!p_navigation_mesh.is_valid());
	ERR_FAIL_COND(!p_source_geometry_data.is_valid());

	if (p_navigation_mesh->get_outline_count() == 0 && !p_source_geometry_data->has_data()) {
		p_navigation_mesh->clear();
		if (p_callback.is_valid()) {
			generator_emit_callback(p_callback);
		}
		return;
	}

	if (is_baking(p_navigation_mesh)) {
		ERR_FAIL_MSG("NavigationPolygon is already baking. Wait for current bake to finish.");
	}
	baking_navmesh_mutex.lock();
	baking_navmeshes.insert(p_navigation_mesh);
	baking_navmesh_mutex.unlock();

	generator_bake_from_source_geometry_data(p_navigation_mesh, p_source_geometry_data);

	baking_navmesh_mutex.lock();
	baking_navmeshes.erase(p_navigation_mesh);
	baking_navmesh_mutex.unlock();

	if (p_callback.is_valid()) {
		generator_emit_callback(p_callback);
	}
}

void NavMeshGenerator2D::bake_from_source_geometry_data_async(Ref<NavigationPolygon> p_navigation_mesh, Ref<NavigationMeshSourceGeometryData2D> p_source_geometry_data, const Callable &p_callback) {
	ERR_FAIL_COND(!p_navigation_mesh.is_valid());
	ERR_FAIL_COND(!p_source_geometry_data.is_valid());

	if (p_navigation_mesh->get_outline_count() == 0 && !p_source_geometry_data->has_data()) {
		p_navigation_mesh->clear();
		if (p_callback.is_valid()) {
			generator_emit_callback(p_callback);
		}
		return;
	}

	if (!use_threads) {
		bake_from_source_geometry_data(p_navigation_mesh, p_source_geometry_data, p_callback);
		return;
	}

	if (is_baking(p_navigation_mesh)) {
		ERR_FAIL_MSG("NavigationPolygon is already baking. Wait for current bake to finish.");
	}
	baking_navmesh_mutex.lock();
	baking_navmeshes.insert(p_navigation_mesh);
	baking_navmesh_mutex.unlock();

	generator_task_mutex.lock();
	NavMeshGeneratorTask2D *generator_task = memnew(NavMeshGeneratorTask2D);
	generator_task->navigation_mesh = p_navigation_mesh;
	generator_task->source_geometry_data = p_source_geometry_data;
	generator_task->callback = p_callback;
	generator_task->status = NavMeshGeneratorTask2D::TaskStatus::BAKING_STARTED;
	generator_task->thread_task_id = WorkerThreadPool::get_singleton()->add_native_task(&NavMeshGenerator2D::generator_thread_bake, generator_task, NavMeshGenerator2D::baking_use_high_priority_threads, "NavMeshGeneratorBake2D");
	generator_tasks.insert(generator_task->thread_task_id, generator_task);
	generator_task_mutex.unlock();
}

bool NavMeshGenerator2D::is_baking(Ref<NavigationPolygon> p_navigation_polygon) {
	baking_navmesh_mutex.lock();
	bool baking = baking_navmeshes.has(p_navigation_polygon);
	baking_navmesh_mutex.unlock();
	return baking;
}

void NavMeshGenerator2D::generator_thread_bake(void *p_arg) {
	NavMeshGeneratorTask2D *generator_task = static_cast<NavMeshGeneratorTask2D *>(p_arg);

	generator_bake_from_source_geometry_data(generator_task->navigation_mesh, generator_task->source_geometry_data);

	generator_task->status = NavMeshGeneratorTask2D::TaskStatus::BAKING_FINISHED;
}

void NavMeshGenerator2D::generator_parse_geometry_node(Ref<NavigationPolygon> p_navigation_mesh, Ref<NavigationMeshSourceGeometryData2D> p_source_geometry_data, Node *p_node, bool p_recurse_children) {
	generator_rid_rwlock.read_lock();
	for (const NavMeshGeometryParser2D *parser : generator_parsers) {
		if (!parser->callback.is_valid()) {
			continue;
		}
		parser->callback.call(p_navigation_mesh, p_source_geometry_data, p_node);
	}
	generator_rid_rwlock.read_unlock();

	if (p_recurse_children) {
		for (int i = 0; i < p_node->get_child_count(); i++) {
			generator_parse_geometry_node(p_navigation_mesh, p_source_geometry_data, p_node->get_child(i), p_recurse_children);
		}
	}
}

void NavMeshGenerator2D::generator_parse_source_geometry_data(Ref<NavigationPolygon> p_navigation_mesh, Ref<NavigationMeshSourceGeometryData2D> p_source_geometry_data, Node *p_root_node) {
	List<Node *> parse_nodes;

	if (p_navigation_mesh->get_source_geometry_mode() == NavigationPolygon::SOURCE_GEOMETRY_ROOT_NODE_CHILDREN) {
		parse_nodes.push_back(p_root_node);
	} else {
		p_root_node->get_tree()->get_nodes_in_group(p_navigation_mesh->get_source_geometry_group_name(), &parse_nodes);
	}

	Transform2D root_node_transform = Transform2D();
	if (Object::cast_to<Node2D>(p_root_node)) {
		root_node_transform = Object::cast_to<Node2D>(p_root_node)->get_global_transform().affine_inverse();
	}

	p_source_geometry_data->clear();
	p_source_geometry_data->root_node_transform = root_node_transform;

	bool recurse_children = p_navigation_mesh->get_source_geometry_mode() != NavigationPolygon::SOURCE_GEOMETRY_GROUPS_EXPLICIT;

	for (Node *E : parse_nodes) {
		generator_parse_geometry_node(p_navigation_mesh, p_source_geometry_data, E, recurse_children);
	}
};

static void generator_recursive_process_polytree_items(List<TPPLPoly> &p_tppl_in_polygon, const Clipper2Lib::PolyPathD *p_polypath_item) {
	using namespace Clipper2Lib;

	Vector<Vector2> polygon_vertices;

	for (const PointD &polypath_point : p_polypath_item->Polygon()) {
		polygon_vertices.push_back(Vector2(static_cast<real_t>(polypath_point.x), static_cast<real_t>(polypath_point.y)));
	}

	TPPLPoly tp;
	tp.Init(polygon_vertices.size());
	for (int j = 0; j < polygon_vertices.size(); j++) {
		tp[j] = polygon_vertices[j];
	}

	if (p_polypath_item->IsHole()) {
		tp.SetOrientation(TPPL_ORIENTATION_CW);
		tp.SetHole(true);
	} else {
		tp.SetOrientation(TPPL_ORIENTATION_CCW);
	}
	p_tppl_in_polygon.push_back(tp);

	for (size_t i = 0; i < p_polypath_item->Count(); i++) {
		const PolyPathD *polypath_item = p_polypath_item->Child(i);
		generator_recursive_process_polytree_items(p_tppl_in_polygon, polypath_item);
	}
}

bool NavMeshGenerator2D::generator_emit_callback(const Callable &p_callback) {
	ERR_FAIL_COND_V(!p_callback.is_valid(), false);

	Callable::CallError ce;
	Variant result;
	p_callback.callp(nullptr, 0, result, ce);

	return ce.error == Callable::CallError::CALL_OK;
}

RID NavMeshGenerator2D::source_geometry_parser_create() {
	RWLockWrite write_lock(generator_rid_rwlock);

	RID rid = generator_parser_owner.make_rid();

	NavMeshGeometryParser2D *parser = generator_parser_owner.get_or_null(rid);
	parser->self = rid;

	generator_parsers.push_back(parser);

	return rid;
}

void NavMeshGenerator2D::source_geometry_parser_set_callback(RID p_parser, const Callable &p_callback) {
	RWLockWrite write_lock(generator_rid_rwlock);

	NavMeshGeometryParser2D *parser = generator_parser_owner.get_or_null(p_parser);
	ERR_FAIL_NULL(parser);

	parser->callback = p_callback;
}

bool NavMeshGenerator2D::owns(RID p_object) {
	RWLockRead read_lock(generator_rid_rwlock);
	return generator_parser_owner.owns(p_object);
}

void NavMeshGenerator2D::free(RID p_object) {
	RWLockWrite write_lock(generator_rid_rwlock);

	if (generator_parser_owner.owns(p_object)) {
		NavMeshGeometryParser2D *parser = generator_parser_owner.get_or_null(p_object);

		generator_parsers.erase(parser);

		generator_parser_owner.free(p_object);
	} else {
		ERR_PRINT("Attempted to free a NavMeshGenerator2D RID that did not exist (or was already freed).");
	}
}

void NavMeshGenerator2D::generator_bake_from_source_geometry_data(Ref<NavigationPolygon> p_navigation_mesh, Ref<NavigationMeshSourceGeometryData2D> p_source_geometry_data) {
	if (p_navigation_mesh.is_null() || p_source_geometry_data.is_null()) {
		return;
	}

	if (p_navigation_mesh->get_outline_count() == 0 && !p_source_geometry_data->has_data()) {
		return;
	}

	int outline_count = p_navigation_mesh->get_outline_count();
	const Vector<Vector<Vector2>> &traversable_outlines = p_source_geometry_data->_get_traversable_outlines();
	const Vector<Vector<Vector2>> &obstruction_outlines = p_source_geometry_data->_get_obstruction_outlines();

	if (outline_count == 0 && traversable_outlines.size() == 0) {
		return;
	}

	using namespace Clipper2Lib;

	PathsD traversable_polygon_paths;
	PathsD obstruction_polygon_paths;

	traversable_polygon_paths.reserve(outline_count + traversable_outlines.size());
	obstruction_polygon_paths.reserve(obstruction_outlines.size());

	for (int i = 0; i < outline_count; i++) {
		const Vector<Vector2> &traversable_outline = p_navigation_mesh->get_outline(i);
		PathD subject_path;
		subject_path.reserve(traversable_outline.size());
		for (const Vector2 &traversable_point : traversable_outline) {
			const PointD &point = PointD(traversable_point.x, traversable_point.y);
			subject_path.push_back(point);
		}
		traversable_polygon_paths.push_back(subject_path);
	}

	for (const Vector<Vector2> &traversable_outline : traversable_outlines) {
		PathD subject_path;
		subject_path.reserve(traversable_outline.size());
		for (const Vector2 &traversable_point : traversable_outline) {
			const PointD &point = PointD(traversable_point.x, traversable_point.y);
			subject_path.push_back(point);
		}
		traversable_polygon_paths.push_back(subject_path);
	}

	for (const Vector<Vector2> &obstruction_outline : obstruction_outlines) {
		PathD clip_path;
		clip_path.reserve(obstruction_outline.size());
		for (const Vector2 &obstruction_point : obstruction_outline) {
			const PointD &point = PointD(obstruction_point.x, obstruction_point.y);
			clip_path.push_back(point);
		}
		obstruction_polygon_paths.push_back(clip_path);
	}

	const Vector<NavigationMeshSourceGeometryData2D::ProjectedObstruction> &projected_obstructions = p_source_geometry_data->_get_projected_obstructions();

	if (!projected_obstructions.is_empty()) {
		for (const NavigationMeshSourceGeometryData2D::ProjectedObstruction &projected_obstruction : projected_obstructions) {
			if (projected_obstruction.carve) {
				continue;
			}
			if (projected_obstruction.vertices.is_empty() || projected_obstruction.vertices.size() % 2 != 0) {
				continue;
			}

			PathD clip_path;
			clip_path.reserve(projected_obstruction.vertices.size() / 2);
			for (int i = 0; i < projected_obstruction.vertices.size() / 2; i++) {
				const PointD &point = PointD(projected_obstruction.vertices[i * 2], projected_obstruction.vertices[i * 2 + 1]);
				clip_path.push_back(point);
			}
			if (!IsPositive(clip_path)) {
				std::reverse(clip_path.begin(), clip_path.end());
			}
			obstruction_polygon_paths.push_back(clip_path);
		}
	}

	Rect2 baking_rect = p_navigation_mesh->get_baking_rect();
	if (baking_rect.has_area()) {
		Vector2 baking_rect_offset = p_navigation_mesh->get_baking_rect_offset();

		const int rect_begin_x = baking_rect.position[0] + baking_rect_offset.x;
		const int rect_begin_y = baking_rect.position[1] + baking_rect_offset.y;
		const int rect_end_x = baking_rect.position[0] + baking_rect.size[0] + baking_rect_offset.x;
		const int rect_end_y = baking_rect.position[1] + baking_rect.size[1] + baking_rect_offset.y;

		RectD clipper_rect = RectD(rect_begin_x, rect_begin_y, rect_end_x, rect_end_y);

		traversable_polygon_paths = RectClip(clipper_rect, traversable_polygon_paths);
		obstruction_polygon_paths = RectClip(clipper_rect, obstruction_polygon_paths);
	}

	PathsD path_solution;

	// first merge all traversable polygons according to user specified fill rule
	PathsD dummy_clip_path;
	traversable_polygon_paths = Union(traversable_polygon_paths, dummy_clip_path, FillRule::NonZero);
	// merge all obstruction polygons, don't allow holes for what is considered "solid" 2D geometry
	obstruction_polygon_paths = Union(obstruction_polygon_paths, dummy_clip_path, FillRule::NonZero);

	path_solution = Difference(traversable_polygon_paths, obstruction_polygon_paths, FillRule::NonZero);

	real_t agent_radius_offset = p_navigation_mesh->get_agent_radius();
	if (agent_radius_offset > 0.0) {
		path_solution = InflatePaths(path_solution, -agent_radius_offset, JoinType::Miter, EndType::Polygon);
	}

	if (!projected_obstructions.is_empty()) {
		obstruction_polygon_paths.resize(0);
		for (const NavigationMeshSourceGeometryData2D::ProjectedObstruction &projected_obstruction : projected_obstructions) {
			if (!projected_obstruction.carve) {
				continue;
			}
			if (projected_obstruction.vertices.is_empty() || projected_obstruction.vertices.size() % 2 != 0) {
				continue;
			}

			PathD clip_path;
			clip_path.reserve(projected_obstruction.vertices.size() / 2);
			for (int i = 0; i < projected_obstruction.vertices.size() / 2; i++) {
				const PointD &point = PointD(projected_obstruction.vertices[i * 2], projected_obstruction.vertices[i * 2 + 1]);
				clip_path.push_back(point);
			}
			if (!IsPositive(clip_path)) {
				std::reverse(clip_path.begin(), clip_path.end());
			}
			obstruction_polygon_paths.push_back(clip_path);
		}
		if (obstruction_polygon_paths.size() > 0) {
			path_solution = Difference(path_solution, obstruction_polygon_paths, FillRule::NonZero);
		}
	}

	//path_solution = RamerDouglasPeucker(path_solution, 0.025); //

	real_t border_size = p_navigation_mesh->get_border_size();
	if (baking_rect.has_area() && border_size > 0.0) {
		Vector2 baking_rect_offset = p_navigation_mesh->get_baking_rect_offset();

		const int rect_begin_x = baking_rect.position[0] + baking_rect_offset.x + border_size;
		const int rect_begin_y = baking_rect.position[1] + baking_rect_offset.y + border_size;
		const int rect_end_x = baking_rect.position[0] + baking_rect.size[0] + baking_rect_offset.x - border_size;
		const int rect_end_y = baking_rect.position[1] + baking_rect.size[1] + baking_rect_offset.y - border_size;

		RectD clipper_rect = RectD(rect_begin_x, rect_begin_y, rect_end_x, rect_end_y);

		path_solution = RectClip(clipper_rect, path_solution);
	}

	Vector<Vector<Vector2>> new_baked_outlines;

	for (const PathD &scaled_path : path_solution) {
		Vector<Vector2> polypath;
		for (const PointD &scaled_point : scaled_path) {
			polypath.push_back(Vector2(static_cast<real_t>(scaled_point.x), static_cast<real_t>(scaled_point.y)));
		}
		new_baked_outlines.push_back(polypath);
	}

	if (new_baked_outlines.size() == 0) {
		p_navigation_mesh->set_vertices(Vector<Vector2>());
		p_navigation_mesh->clear_polygons();
		return;
	}

	PathsD polygon_paths;
	polygon_paths.reserve(new_baked_outlines.size());

	for (const Vector<Vector2> &baked_outline : new_baked_outlines) {
		PathD polygon_path;
		for (const Vector2 &baked_outline_point : baked_outline) {
			const PointD &point = PointD(baked_outline_point.x, baked_outline_point.y);
			polygon_path.push_back(point);
		}
		polygon_paths.push_back(polygon_path);
	}

	ClipType clipper_cliptype = ClipType::Union;

	List<TPPLPoly> tppl_in_polygon, tppl_out_polygon;

	PolyTreeD polytree;
	ClipperD clipper_D;

	clipper_D.AddSubject(polygon_paths);
	clipper_D.Execute(clipper_cliptype, FillRule::NonZero, polytree);

	for (size_t i = 0; i < polytree.Count(); i++) {
		const PolyPathD *polypath_item = polytree[i];
		generator_recursive_process_polytree_items(tppl_in_polygon, polypath_item);
	}

	TPPLPartition tpart;
	if (tpart.ConvexPartition_HM(&tppl_in_polygon, &tppl_out_polygon) == 0) { //failed!
		ERR_PRINT("NavigationPolygon Convex partition failed. Unable to create a valid NavigationMesh from defined polygon outline paths.");
		p_navigation_mesh->set_vertices(Vector<Vector2>());
		p_navigation_mesh->clear_polygons();
		return;
	}

	Vector<Vector2> new_vertices;
	Vector<Vector<int>> new_polygons;

	HashMap<Vector2, int> points;
	for (List<TPPLPoly>::Element *I = tppl_out_polygon.front(); I; I = I->next()) {
		TPPLPoly &tp = I->get();

		Vector<int> new_polygon;

		for (int64_t i = 0; i < tp.GetNumPoints(); i++) {
			HashMap<Vector2, int>::Iterator E = points.find(tp[i]);
			if (!E) {
				E = points.insert(tp[i], new_vertices.size());
				new_vertices.push_back(tp[i]);
			}
			new_polygon.push_back(E->value);
		}

		new_polygons.push_back(new_polygon);
	}

	p_navigation_mesh->set_vertices(new_vertices);
	p_navigation_mesh->clear_polygons();
	for (int i = 0; i < new_polygons.size(); i++) {
		p_navigation_mesh->add_polygon(new_polygons[i]);
	}
}

#endif // CLIPPER2_ENABLED
