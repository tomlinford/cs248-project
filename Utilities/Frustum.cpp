#include "Frustum.h"

using namespace::glm;

void Frustum::LoadView(float FOV, float ratio, float near, float far)
{
    Frustum::FOV = FOV;
    Frustum::ratio = ratio;
    Frustum::near = near;
    Frustum::far = far;
}

void Frustum::LoadCam(glm::vec3 eye, glm::vec3 apple, glm::vec3 up) {
    position = eye;
    direction = normalize(apple - eye);
    Frustum::up = normalize(up);
    left = normalize(cross(up, direction));
    
    ComputeCorners();
    ComputePlanes();
}

/** Computes the 8 corners of the frustum based on
 the FOV and near/far planes. */
void Frustum::ComputeCorners()
{
    // Compute corners of near clipping plane
    vec3 midNear = position + near * direction;
    float yNear = near * atan(FOV * M_PI / 180);
    float xNear = yNear * ratio;
    
    ntl = midNear + up * yNear - left * xNear;
    ntr = midNear + up * yNear + left * xNear;
    nll = midNear - up * yNear; - left * xNear;
    nlr = midNear - up * yNear + left * xNear;
    
    // Compute corners of far clipping plane
    vec3 midFar =  position + far * direction;
    float yFar = far * atan(FOV * M_PI / 180);
    float xFar = yFar * ratio;
    
    ftl = midFar + up * yFar - left * xFar;
    ftr = midFar + up * yFar + left * xFar;
    fll = midFar - up * yFar - left * xFar;
    flr = midFar - up * yFar + left * xFar;
}

/** Computes clipping planes based on the 8 corners
 of the frustum */
void Frustum::ComputePlanes()
{
    // Top
    Plane top;
    top.normal = normalize(cross(ntr - ntl, ftl - ntl));
    top.position = ntl;
    planes[0] = top;
    
    // Bottom
    Plane bottom;
    bottom.normal = normalize(cross(fll - nll, nlr - nll));
    bottom.position = nll;
    planes[1] = bottom;
    
    // Left
    Plane left;
    left.normal = normalize(cross(ftl - ntl, nll - ntl));
    left.position = ntl;
    planes[2] = left;
    
    // Right
    Plane right;
    right.normal = normalize(cross(nlr - ntr, ftr - ntr));
    right.position = ntr;
    planes[3] = right;
    
    // Near
    Plane near;
    near.normal = normalize(cross(ntr - nlr, nll - nlr));
    near.position = nll;
    planes[4] = near;
    
    // Far
    Plane far;
    far.normal = normalize(cross(fll - flr, ftr - flr));
    far.position = flr;
    planes[5] = far;
}

bool Frustum::Contains(Object& object) const
{
    Bounds bounds = object.GetBounds();
    for (int i = 0; i < 6; i++) {
        Plane plane = planes[i];
    }
    
    return length(position - object.GetPosition()) < 100;
}