#if !defined(TRIANGLE)
#define TRIANGLE

#include "base_geometry.hpp"
#include <cstdint>

namespace szilv {

    typedef struct {
        Vertex p1;
        Vertex p2;
        Vertex p3;
    } TrianglePrimitive;

    class Triangle2D {
        public:
            Triangle2D(Vertex v1, Vertex v2, Vertex v3);
            Triangle2D(TrianglePrimitive trg_prm);
            Triangle2D(Triangle2D *trg);

            virtual Vertex getCenter();
            virtual uint32_t getRadiusOfTheOuterCircle();
            virtual bool pointInTriangle(Vertex point);
            TrianglePrimitive getPrimitive() { return tr; }
            virtual void setPrimitive(TrianglePrimitive trg_prm);
            virtual void translateToNewCenter(Vertex new_centroid);
            virtual void rotateAroundTheCenter(double angle);

        private:
            TrianglePrimitive tr;

            virtual double distance(Vertex p1, Vertex p2);
    };
}

#endif /* !defined(TRIANGLE) */
