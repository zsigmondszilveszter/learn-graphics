#if !defined(BASE_GEOMETRY)
#define BASE_GEOMETRY

namespace GM {

    typedef struct {
        double x;
        double y;
        double z;
    } Vertex;

    class BaseGeometry {
        public:
            static double sign (Vertex p1, Vertex p2, Vertex p3);
            static Vertex rotate (Vertex p, Vertex around, double angle);
    };
}

#endif /* !defined(BASE_GEOMETRY) */
