#pragma once

#include <map>
#include <string>
#include <glm/glm.hpp>

/** Defines a material. */
struct Material {
    float Ns;       /// specular component
    float d;        /// dissolve component
    glm::vec3 Ka;   /// ambient color
    glm::vec3 Kd;   /// diffuse color
    glm::vec3 Ks;   /// specular color
    glm::vec3 Ke;   /// emissivity

    static std::map<std::string, Material> ParseFile(const char *filename);
};

typedef std::map<std::string, Material> MTLFile;
