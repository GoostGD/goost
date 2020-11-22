#include "poly_mesh_2d.h"

#include "core/engine.h"
#include "core/math/geometry.h"

#include "goost/core/math/2d/geometry/goost_geometry_2d.h"
#include "goost/core/math/2d/geometry/poly/decomp/poly_decomp.h"

Vector<Vector<Point2>> PolyMesh2D::_collect_outlines() {
    Vector<Vector<Point2>> outlines;
    for (int i = 0; i < get_child_count(); ++i) {
        PolyNode2D *n = Object::cast_to<PolyNode2D>(get_child(i));
        if (!n) {
            continue;
        }
        outlines.append_array(n->build_outlines());
    }
    return outlines;
}

Vector<Vector<Point2>> PolyMesh2D::_build_shapes() {
    shapes.clear();
    const Vector<Vector<Point2>> &outlines = _collect_outlines();
    if (outlines.empty()) {
        return shapes;
    }
	switch (build_mode) {
		case BUILD_TRIANGLES: {
            shapes.append_array(PolyDecomp2D::triangulate_polygons(outlines));
		} break;
		case BUILD_CONVEX: {
			shapes.append_array(PolyDecomp2D::decompose_polygons_into_convex(outlines));
		} break;
		case BUILD_CONCAVE: {
			// Concave shapes cannot have inner outlines, so filter those out.
            for (int i = 0; i < outlines.size(); ++i) {
                const Vector<Point2> &points = outlines[i];
                if (!Geometry::is_polygon_clockwise(points)) {
                    shapes.push_back(points);
                }
            }
		} break;
	}
    return shapes;
}

void PolyMesh2D::add_child_notify(Node *p_child) {
    Node2D::add_child_notify(p_child);

	PolyNode2D *n = Object::cast_to<PolyNode2D>(p_child);
	if (!n) {
		return;
    }
    n->connect("outlines_updated", this, "_update_shapes");
    call_deferred("_update_shapes");
}

void PolyMesh2D::remove_child_notify(Node *p_child) {
    Node2D::remove_child_notify(p_child);

	PolyNode2D *n = Object::cast_to<PolyNode2D>(p_child);
	if (!n) {
		return;
    }
    n->disconnect("outlines_updated", this, "_update_shapes");
    call_deferred("_update_shapes");
}

void PolyMesh2D::_update_shapes() {
    _build_shapes();
	_apply_shapes();
    update();
}

void PolyMesh2D::_notification(int p_what) {
	switch (p_what) {
		case NOTIFICATION_PARENTED: {
		} break;
		case NOTIFICATION_ENTER_TREE: {
			_build_shapes();
		} break;
		case NOTIFICATION_UNPARENTED: {
		} break;
		case NOTIFICATION_DRAW: {
			if (!Engine::get_singleton()->is_editor_hint() && !get_tree()->is_debugging_collisions_hint()) {
				break;
			}
            if (shapes.empty()) {
                break;
            }
            // Colors synced with `CollisionPolygon2D` node.
			Color c(0.4, 0.9, 0.1);
			for (int i = 0; i < shapes.size(); ++i) {
                const Vector<Point2> &points = shapes[i];
                if (points.size() < 3) {
                    continue;
                }
				c.set_hsv(Math::fmod(c.get_h() + 0.738, 1), c.get_s(), c.get_v(), 0.5);
				draw_colored_polygon(points, c);
			}
		} break;
	}
}

void PolyMesh2D::set_build_mode(BuildMode p_mode) {
	ERR_FAIL_INDEX((int)p_mode, 3);
	build_mode = p_mode;
    _update_shapes();
}

String PolyMesh2D::get_configuration_warning() const {
    String warning = Node2D::get_configuration_warning();

	bool found = false;
	for (int i = 0; i < get_child_count(); ++i) {
		PolyNode2D *n = Object::cast_to<PolyNode2D>(get_child(i));
        if (n) {
            found = true;
            break;
        }
	}
    if (!found) {
		if (!warning.empty()) {
			warning += "\n\n";
		}
		warning += TTR("PolyNode2D is required to build a mesh. Add PolyNode2D as a child.");
    }
	return warning;
}

void PolyMesh2D::_bind_methods() {
    ClassDB::bind_method(D_METHOD("_update_shapes"), &PolyMesh2D::_update_shapes);

	ClassDB::bind_method(D_METHOD("set_build_mode", "build_mode"), &PolyMesh2D::set_build_mode);
	ClassDB::bind_method(D_METHOD("get_build_mode"), &PolyMesh2D::get_build_mode);

	ADD_PROPERTY(PropertyInfo(Variant::INT, "build_mode", PROPERTY_HINT_ENUM, "Triangles,Convex,Segments"), "set_build_mode", "get_build_mode");

	BIND_ENUM_CONSTANT(BUILD_TRIANGLES);
	BIND_ENUM_CONSTANT(BUILD_CONVEX);
	BIND_ENUM_CONSTANT(BUILD_CONCAVE);
}