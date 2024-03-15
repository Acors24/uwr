#include <fstream>

#include "Util.hpp"
#include "ModelData.hpp"

// https://stackoverflow.com/a/14266139
std::vector<std::string> util::split(std::string string, const std::string &delimeter) {
  size_t pos = 0;
  std::string token;
  std::vector<std::string> tokens;
  while ((pos = string.find(delimeter)) != std::string::npos) {
      token = string.substr(0, pos);
      tokens.push_back(token);
      string.erase(0, pos + delimeter.length());
  }
  tokens.push_back(string);
  return tokens;
}

ModelData util::loadObj(const std::string &path) {
    std::ifstream file(path);
    std::string line;

    ModelData modelData;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    int i = 0;
    int materialId = 0;

    while (std::getline(file, line)) {
      if (line.at(0) == '#') {
        continue;
      }

      auto tokens = split(line, " ");

      if (tokens[0] == "mtllib") {
        std::string materialName = tokens[1];
        modelData.materials = loadMtl(path.substr(0, path.find_last_of('.')) + ".mtl");
      } else if (tokens[0] == "usemtl") {
        std::string materialName = tokens[1];
        // std::cout << materialName << std::endl;
        for (int mId = 0; mId < modelData.materials.size(); mId++) {
            // std::cout << modelData.materials.at(mId).name << std::endl;
            if (modelData.materials.at(mId).name == materialName) {
                materialId = mId;
                break;
            }
        }
      } else if (tokens[0] == "v") {
        float x = std::stof(tokens[1]);
        float y = std::stof(tokens[2]);
        float z = std::stof(tokens[3]);
        vertices.push_back(glm::vec3(x, y, z));
      } else if (tokens[0] == "vn") {
        float x = std::stof(tokens[1]);
        float y = std::stof(tokens[2]);
        float z = std::stof(tokens[3]);
        normals.push_back(glm::vec3(x, y, z));
      } else if (tokens[0] == "f") {
        for (int t = 1; t < tokens.size(); t++) {
            int vertexIndex, normalIndex;

            auto res = split(tokens[t], "/");
            vertexIndex = std::stoi(res[0]) - 1;
            modelData.vertices.push_back(vertices.at(vertexIndex));
            modelData.indices.push_back(i++);
            normalIndex = std::stoi(res[2]) - 1;
            modelData.normals.push_back(normals.at(normalIndex));
            modelData.materialIds.push_back(materialId);
        }

        modelData.indices.push_back(UINT32_MAX);
        // modelData.vertices.push_back(glm::vec3(0.0f));
        // modelData.normals.push_back(glm::vec3(0.0f));
      }
    }

    return modelData;
}

std::vector<Material> util::loadMtl(const std::string &path) {
    std::ifstream file(path);
    std::string line;
    std::vector<Material> materials;
    int materialIndex = -1;
    while (std::getline(file, line)) {
      if (line.size() == 0 || line.at(0) == '#') {
        continue;
      }

      auto tokens = split(line, " ");

      if (tokens[0] == "newmtl") {
        materials.push_back(Material());
        materialIndex++;
        materials.at(materialIndex).name = tokens[1];
      } else if (tokens[0] == "Ka") {
        float r = std::stof(tokens[1]);
        float g = std::stof(tokens[2]);
        float b = std::stof(tokens[3]);
        materials.at(materialIndex).Ka = glm::vec3(r, g, b);
      } else if (tokens[0] == "Kd") {
        float r = std::stof(tokens[1]);
        float g = std::stof(tokens[2]);
        float b = std::stof(tokens[3]);
        materials.at(materialIndex).Kd = glm::vec3(r, g, b);
      } else if (tokens[0] == "Ks") {
        float r = std::stof(tokens[1]);
        float g = std::stof(tokens[2]);
        float b = std::stof(tokens[3]);
        materials.at(materialIndex).Ks = glm::vec3(r, g, b);
      } else if (tokens[0] == "Ns") {
        materials.at(materialIndex).Ns = std::stof(tokens[1]);
      } else if (tokens[0] == "Ni") {
        materials.at(materialIndex).Ni = std::stof(tokens[1]);
      } else if (tokens[0] == "d") {
        materials.at(materialIndex).d = std::stof(tokens[1]);
      }
    }

    // std::cout << "Name: " << material.name << std::endl;
    // std::cout << "Ka: " << material.Ka.x << ", " << material.Ka.y << ", " << material.Ka.z << std::endl;
    // std::cout << "Kd: " << material.Kd.x << ", " << material.Kd.y << ", " << material.Kd.z << std::endl;
    // std::cout << "Ks: " << material.Ks.x << ", " << material.Ks.y << ", " << material.Ks.z << std::endl;
    // std::cout << "Ns: " << material.Ns << std::endl;
    // std::cout << "Ni: " << material.Ni << std::endl;
    
    return materials;
}