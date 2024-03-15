#ifndef MODELDATA_HPP
#define MODELDATA_HPP

#include <vector>
#include "Material.hpp"

struct ModelData {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<unsigned int> indices;
    std::vector<unsigned int> materialIds;
    std::vector<Material> materials;
};

#endif