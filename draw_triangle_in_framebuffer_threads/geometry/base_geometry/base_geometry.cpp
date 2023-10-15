#include "base_geometry.h"
#include <math.h>

namespace GM {

    double BaseGeometry::sign (Vertex p1, Vertex p2, Vertex p3) {
        return (p1.x - p3.x) * (p2.y - p3.y) - (p2.x - p3.x) * (p1.y - p3.y);
    }

    Vertex BaseGeometry::rotate (Vertex p, Vertex around, double angle) {
        double x = cos(angle) * (p.x - around.x) - sin(angle) * (p.y - around.y) + around.x;
        double y = sin(angle) * (p.x - around.x) + cos(angle) * (p.y - around.y) + around.y;
        return {x, y, 0};
    }
}
