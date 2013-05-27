#pragma once

#include "Object.h"
// #include "../Grid/grid.h" // Stanford's CS106B grid class
// WARNING: you won't be able to make a variable called "in" if this file is included

#include <assert.h>

typedef enum
{
    TESSLVL_32,
    TESSLVL_64,
    TESSLVL_128
} TessLevel;

/** Represents a terrain map. A maps's model space should be
 the same as its world space; this restriction makes sense
 since the map really is the game world. It also simplifies
 collision detection greatly by avoiding the need to transform
 vertex data by the model matrix. */
class Map : public Object
{
public:
    /** We should have several plane .obj files at different
     tesselation levels. Perhaps one 128 x 128, one
     64 x 64, and one 32 x 32. We should select the
     object to use based on how much detail we need
     in the map. */
    Map(float *heightMap, size_t size, int x, int y);
    
    /** Determine if an object and Map intersects by
     testing the z-value of the object's bounding box's
     bottom against the Map's height field. Note that
     this only works because procedural height-field
     terrain does not support overhangs or concave
     geometry. */
    virtual bool Intersects(Object other);
    
    /** Returns the map's position */
    virtual glm::vec3 GetPosition();

	void Draw(const glm::mat4& viewProjection, const glm::vec3& cameraPos,
              const glm::vec3& lightPos) const;

	void AddTerrain(float *heightMap, size_t size, int x, int y);

	void SetColor(const glm::vec3& color) { this->color = color; }
    
private:
    Texture heightField;
	Program p;
    
	ModelBuffer *triangleMB;
	ModelBuffer *lineMB;
    
    /** Helper method for sampling the height field. 
     x and y are wrapped around so that they will always
     fall within the height field. */
    GLfloat Sample(GLfloat *map, GLuint width, GLuint height,
                   int x, int y);
};
