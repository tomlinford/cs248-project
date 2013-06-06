#pragma once

#include "Model.h"
#include "../Utilities/OBJFile.h"

#include <assert.h> 
#include <unordered_map>

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
    Object(const Object& other);
    ~Object();
    
    /** Returns whether the object's AABB
     intersects another object's AABB */
    virtual bool Intersects(Object& other);
    
    /** Color */
    virtual glm::vec3 GetColor() { return color; }
    virtual void SetColor(glm::vec3 c) { color = c; }
    
    /** Position in world space */
    virtual glm::vec3 GetPosition() { return position; };
    virtual void SetPosition(glm::vec3 p)
    {
        position = p;
        M = glm::translate(glm::mat4(1), position) *
            glm::mat4_cast(orientation) *
            glm::scale(glm::mat4(1), glm::vec3(scale));
    };
    
    /** Scale */
    virtual float GetScale() { return scale; }
    virtual void SetScale(float s) {
        scale = s;
        M = glm::translate(glm::mat4(1), position) *
            glm::mat4_cast(orientation) *
            glm::scale(glm::mat4(1), glm::vec3(scale));
    }
    
    /** Orientation (rotation) in model space */
    virtual glm::quat GetOrientation() { return orientation; };
    virtual void SetOrientation(glm::quat o)
    {
        orientation = o;
        M = glm::translate(glm::mat4(1), position) *
            glm::mat4_cast(orientation) *
            glm::scale(glm::mat4(1), glm::vec3(scale));
    };
    
    /** Returns the object's model matrix */
    virtual glm::mat4 GetModelMatrix() { return M; }
    
    /** Returns axis-aligned world-space bounding box
     by finding the min/max of the transformed
     model-space bounds */
    virtual Bounds GetBounds() const;
    
    /** Rotate by an angle represented by a vec3 where
     x = yaw
     y = pitch
     z = roll */
    void Rotate(glm::vec3 angles)
    {
        orientation = glm::normalize(glm::fquat(angles));
    };
    
    /** Draws the object */
    virtual void Draw(const Program& p, const glm::mat4& viewProjection,
                      const glm::vec3& cameraPos, GLenum mode = GL_TRIANGLES) const;
    
protected:
    /** Associated 3D model */
    Model *model;
    
    /** Properties */
    float scale;
    glm::mat4 M;
    glm::vec3 color;
    glm::vec3 position;
    glm::quat orientation;
    string model_file;
    
    /** Helper function for finding object's AABB
     (axis-aligned bounding box) */
    void UpdateMinMax(glm::vec3& min, glm::vec3& max, glm::vec4 test);
    
    /* Draws the object's AABB */
    void DrawAABB(const Program& p, const glm::mat4& viewProjection) const;
};
