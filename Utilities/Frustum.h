#pragma once

#include "gl.h"

#include <glm/glm.hpp>

#include "Object.h"

class Frustum
{
public:
    void LoadView(float FOV, float ratio, float near, float far);
    void LoadCam(glm::vec3 eye, glm::vec3 apple, glm::vec3 up);
    bool Contains(Object& object) const;
    
private:
    // Corners defining far plane
    glm::vec3 ftl, ftr, fll, flr;
    
    // Corners defining near plane
    glm::vec3 ntl, ntr, nll, nlr;
    
    // Camera position
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 direction;
};