#include "triangle.h"

namespace GM {

    Triangle::Triangle(Vertex v1, Vertex v2, Vertex v3) {
        this->tr = { v1, v2, v3 };
    }
    
    Triangle::Triangle(TrianglePrimitive trg_prm) {
        setPrimitive(trg_prm);
    }

    Triangle::Triangle(Triangle *trg) {
        this->tr = {
            trg->getPrimitive().p1,
            trg->getPrimitive().p2,
            trg->getPrimitive().p3
        };
    }

    Vertex Triangle::getCenter() {
        double centerX = (tr.p1.x + tr.p2.x + tr.p3.x) / 3;
        double centerY = (tr.p1.y + tr.p2.y + tr.p3.y) / 3;
        return {centerX, centerY, 0.0};
    }

    bool Triangle::pointInTriangle(Vertex point) {
        double d1, d2, d3;
        bool has_neg, has_pos;

        d1 = BaseGeometry::sign(point, tr.p1, tr.p2);
        d2 = BaseGeometry::sign(point, tr.p2, tr.p3);
        d3 = BaseGeometry::sign(point, tr.p3, tr.p1);

        has_neg = (d1 < 0) || (d2 < 0) || (d3 < 0);
        has_pos = (d1 > 0) || (d2 > 0) || (d3 > 0);

        return !(has_neg && has_pos);
    }

    void Triangle::setPrimitive(TrianglePrimitive trg_prm) {
        this->tr = trg_prm;
    }
}
