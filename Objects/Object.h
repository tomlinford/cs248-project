#pragma once

#include "Model.h"
#include "../Utilities/OBJFile.h"

#include <assert.h> 

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

/* The Object class represents an object in the game.
   It has an associated 3D model and gameplay-related
   object information including position, orientation,
   and bounding box. */
class Object
{
public:
    Object();
    Object(Model *m);
    Object(const string& filename);
    ~Object();
    
    /** Returns whether the object's AABB
     intersects another object's AABB */
    virtual bool Intersects(Object other);
    
    /** Position in world space */
    virtual glm::vec3 GetPosition() { return position; };
    virtual void SetPosition(glm::vec3 p) { position = p; };
    
    /** Orientation (rotation) in model space */
    virtual glm::quat GetOrientation() { return orientation; };
    virtual void SetOrientation(glm::quat o) { orientation = o; };
    
    /** Returns axis-aligned world-space bounding box
     by finding the min/max of the transformed
     model-space bounds */
    Bounds GetBounds() const;
    
    /** Rotate by an angle represented by a vec3 where
     x = yaw
     y = pitch
     z = roll */
    void Rotate(glm::vec3 angles)
    {
        orientation = orientation * glm::fquat(angles.x, 0, 1, 0);
        orientation = orientation * glm::fquat(angles.y, 1, 0, 0);
        orientation = orientation * glm::fquat(angles.z, 0, 0, 1);
        orientation = glm::normalize(orientation);
    };
    
    /** Draws the object */
    virtual void Draw(const Program& p, const glm::mat4& viewProjection,
                      const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES) const;
    
protected:
    Model *model;
    glm::vec3 position;
    glm::quat orientation;
    
    /** Helper function for finding object's AABB
     (axis-aligned bounding box) */
    void UpdateMinMax(glm::vec3& min, glm::vec3& max, glm::vec4 test);
    
    /* Draws the object's AABB */
    void DrawAABB(const Program& p, const glm::mat4& viewProjection,
                  const glm::vec3& cameraPos, GLenum mode) const;
};
