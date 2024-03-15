// Code adapted from embree:
// https://stackoverflow.com/questions/2924795/fastest-way-to-compute-point-to-triangle-distance-in-3d
//
// To find the distance, first one has to compute the closest point in triangle. The following code for finding the closest point in 3D triangle ABC to point P is taken from highly optimized Embree library:
//
// Vec3fa changed to glm::vec3    anl, 2023.10.31

#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>

glm::vec3 closestPointTriangle(glm::vec3 const& p, glm::vec3 const& a, glm::vec3 const& b, glm::vec3 const& c) {
   const glm::vec3 ab = b - a;
   const glm::vec3 ac = c - a;
   const glm::vec3 ap = p - a;

   const float d1 = dot(ab, ap);
   const float d2 = dot(ac, ap);
   if (d1 <= 0.f && d2 <= 0.f) return a; //#1

   const glm::vec3 bp = p - b;
   const float d3 = dot(ab, bp);
   const float d4 = dot(ac, bp);
   if (d3 >= 0.f && d4 <= d3) return b; //#2

   const glm::vec3 cp = p - c;
   const float d5 = dot(ab, cp);
   const float d6 = dot(ac, cp);
   if (d6 >= 0.f && d5 <= d6) return c; //#3

   const float vc = d1 * d4 - d3 * d2;
   if (vc <= 0.f && d1 >= 0.f && d3 <= 0.f) {
      const float v = d1 / (d1 - d3);
      return a + v * ab; //#4
   }

   const float vb = d5 * d2 - d1 * d6;
   if (vb <= 0.f && d2 >= 0.f && d6 <= 0.f) {
      const float v = d2 / (d2 - d6);
      return a + v * ac; //#5
   }

   const float va = d3 * d6 - d5 * d4;
   if (va <= 0.f && (d4 - d3) >= 0.f && (d5 - d6) >= 0.f) {
      const float v = (d4 - d3) / ((d4 - d3) + (d5 - d6));
      return b + v * (c - b); //#6
   }

   const float denom = 1.f / (va + vb + vc);
   const float v = vb * denom;
   const float w = vc * denom;
   return a + v * ab + w * ac; //#0
}

// int main () { // Simple two tests for closest point func.
//    glm::vec3 a(1,0,0), b(0,1,0), c(0,0,1), res;

//    glm::vec3 p1(10,10,1);
//    res = closestPointTriangle(p1, a, b, c);
//    std::cout  << res << std::endl;

//    glm::vec3 p2(10,12,0);
//    res = closestPointTriangle(p2, a, b, c);
//    std::cout  << res << std::endl;

//    return 0;
// }
