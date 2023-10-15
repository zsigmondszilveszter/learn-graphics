#if !defined(TRIANGLE)
#define TRIANGLE

#include "base_geometry.h"

namespace GM {

    typedef struct {
        Vertex p1;
        Vertex p2;
        Vertex p3;
    } TrianglePrimitive;

    class Triangle {
        public:
            Triangle(Vertex v1, Vertex v2, Vertex v3);
            Triangle(TrianglePrimitive trg_prm);
            Triangle(Triangle *trg);

            virtual Vertex getCenter();
            virtual bool pointInTriangle(Vertex point);

            TrianglePrimitive getPrimitive() { return tr; }
            virtual void setPrimitive(TrianglePrimitive trg_prm);

        private:
            TrianglePrimitive tr;
    };
}

#endif /* !defined(TRIANGLE) */
