#if !defined(BASE_GEOMETRY)
#define BASE_GEOMETRY

#include <cstdint>

namespace GM {

    typedef struct {
        double x;
        double y;
        double z;
    } Vertex;

    class BaseGeometry {
        public:
            static double sign (Vertex p1, Vertex p2, Vertex p3);
            static Vertex rotate2D (Vertex p, Vertex around, double angle);
            static Vertex translate3D (Vertex p, int32_t x, int32_t y, int32_t z);
    };
}

#endif /* !defined(BASE_GEOMETRY) */
