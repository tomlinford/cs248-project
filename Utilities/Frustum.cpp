#include "Frustum.h"

using namespace::glm;

void Frustum::LoadView(float FOV, float ratio, float near, float far)
{
    /*ntl =
    ntr =
    nll =
    nlr =
    
    ftl =
    ftr =
    fll =
    flr =*/
}

void Frustum::LoadCam(glm::vec3 eye, glm::vec3 apple, glm::vec3 up) {
    position = eye;
    direction = apple - eye;
    Frustum::up = up;
}

bool Frustum::Contains(Object& object) const
{
    return length(position - object.GetPosition()) < 100;
}