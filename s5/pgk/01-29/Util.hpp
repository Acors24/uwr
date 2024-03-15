#ifndef UTIL_HPP
#define UTIL_HPP

#include <string>
#include <vector>
#include "ModelData.hpp"

namespace util {
    std::vector<std::string> split(std::string string, const std::string &delimeter);
    ModelData loadObj(const std::string &path);
    std::vector<Material> loadMtl(const std::string &path);
}

#endif