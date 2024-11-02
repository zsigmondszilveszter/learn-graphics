#include <cmath>
#include <algorithm>
#include "2D_triangle.hpp"

namespace szilv {

    Triangle2D::Triangle2D(Vertex v1, Vertex v2, Vertex v3) {
        this->tr = { v1, v2, v3 };
    }
    
    Triangle2D::Triangle2D(TrianglePrimitive trg_prm) {
        setPrimitive(trg_prm);
    }

    Triangle2D::Triangle2D(Triangle2D *trg) {
        this->tr = {
            trg->getPrimitive().p1,
            trg->getPrimitive().p2,
            trg->getPrimitive().p3
        };
    }

    Vertex Triangle2D::getCenter() {
        double centerX = (tr.p1.x + tr.p2.x + tr.p3.x) / 3;
        double centerY = (tr.p1.y + tr.p2.y + tr.p3.y) / 3;
        return {centerX, centerY, 0.0};
    }

    uint32_t Triangle2D::getRadiusOfTheOuterCircle() {
        Vertex centroid = getCenter();
        double d1 = distance(centroid, this->tr.p1);
        double d2 = distance(centroid, tr.p2);
        double d3 = distance(centroid, tr.p3);
        double m = std::max({d1, d2, d3});
        return (uint32_t)std::round(m);
    }

    void Triangle2D::translateToNewCenter(Vertex new_centeroid) {
        // get the current center 
        Vertex current_centeroid = getCenter();
        // calculate the translation numbers
        double x_trans = new_centeroid.x - current_centeroid.x;
        double y_trans = new_centeroid.y - current_centeroid.y;
        // translate the vertices
        tr.p1 = BaseGeometry::translate3D(tr.p1, x_trans, y_trans, 0);
        tr.p2 = BaseGeometry::translate3D(tr.p2, x_trans, y_trans, 0);
        tr.p3 = BaseGeometry::translate3D(tr.p3, x_trans, y_trans, 0);
    }

    void Triangle2D::rotateAroundTheCenter(double angle) {
        Vertex centeroid = getCenter();
        tr.p1 = BaseGeometry::rotate2D(tr.p1, centeroid, angle);
        tr.p2 = BaseGeometry::rotate2D(tr.p2, centeroid, angle);
        tr.p3 = BaseGeometry::rotate2D(tr.p3, centeroid, angle);
    }

    bool Triangle2D::pointInTriangle(Vertex point) {
        double d1, d2, d3;
        bool has_neg, has_pos;

        d1 = BaseGeometry::sign(point, tr.p1, tr.p2);
        d2 = BaseGeometry::sign(point, tr.p2, tr.p3);
        d3 = BaseGeometry::sign(point, tr.p3, tr.p1);

        has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(has_neg && has_pos);
    }

    void Triangle2D::setPrimitive(TrianglePrimitive trg_prm) {
        this->tr = trg_prm;
    }

    double Triangle2D::distance(Vertex p1, Vertex p2) {
        return sqrt(pow(p2.x - p1.x, 2) + pow(p2.y - p1.y, 2));
    }
}
