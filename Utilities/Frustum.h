/* LightHouse3D tutorial referenced in implementation of view frustum culling.
 http://www.lighthouse3d.com/tutorials/view-frustum-culling/ */

#pragma once

#include "gl.h"

#include <glm/glm.hpp>

#include "Object.h"

/** Defines a clipping plane */
struct Plane
{
    glm::vec3 normal;
    glm::vec3 position;
};

/** Defines a view frustum. Used for culling
 objects outside the viewing area. */
class Frustum
{
public:
    Frustum() {}
    
    /** Loads view information for this frustum */
    void LoadView(float FOV, float ratio, float near, float far);
    
    /** Loads camera position and orientation information
      for this frustum */
    void LoadCam(glm::vec3 eye, glm::vec3 apple, glm::vec3 up);
    
    /** Determines if an object is within
     this viewing frustum .*/
    bool Contains(Object& object) const;
    
private:
    // Planes
    // 0 - top
    // 1 - bottom
    // 2 - left
    // 3 - right
    // 4 - near
    // 5 - far
    Plane planes[6];
    
    // Corners defining far plane
    glm::vec3 ftl, ftr, fll, flr;
    
    // Corners defining near plane
    glm::vec3 ntl, ntr, nll, nlr;
    
    // View information
    float FOV, ratio, near, far;
    
    // Camera position
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 left;
    glm::vec3 direction;
    
    void ComputeCorners();
    void ComputePlanes();
};