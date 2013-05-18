#pragma once

#include "Model.h"

#include <assert.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

/** Bounding box */
struct Bounds
{
    glm::vec3 lower; // Lower corner in model space
    glm::vec3 upper; // Upper (opposing) corner in model space
};

class Object
{
public:
    Object();
    Object(Model *m);
    ~Object();
    
    virtual bool Intersects(Object other);
    
    /** Position in world space */
    glm::vec3 GetPosition() { return position; };
    void SetPosition(glm::vec3 p) { position = p; };
    
    /** Orientation (rotation) in model space */
    glm::quat GetOrientation() { return orientation; };
    void SetOrientation(glm::quat o) { orientation = o; };
    
    /** Returns axis-aligned world-space bounds */
    Bounds GetBounds();
    
    /** Rotate by an angle represented by a vec3 where
     * x = yaw
     * y = pitch
     * z = roll
     */
    void Rotate(glm::vec3 angles)
    {
        orientation = orientation * glm::fquat(angles.x, 0, 1, 0);
        orientation = orientation * glm::fquat(angles.y, 1, 0, 0);
        orientation = orientation * glm::fquat(angles.z, 0, 0, 1);
        orientation = glm::normalize(orientation);
    };
    
    /** Draws the object */
    void Draw(const Program& p, const glm::mat4& viewProjection,
              const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES) const;
    
protected:
    Model *model;
    Bounds bounds;
    glm::vec3 position;
    glm::quat orientation;
};
