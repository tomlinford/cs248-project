#include "Map.h"

// TODO: Implement
#pragma mark -
Map::Map(TessLevel level) : Object()
{
}

GLfloat Map::Sample(GLfloat *map, GLuint width, GLuint height, int x, int y)
{
    return map[((y & (height - 1)) * height) + (x & (width - 1))];
}

bool Map::Intersects(Object other)
{
    GLenum format = heightField->GetFormat();
    assert(format == GL_LUMINANCE); // Single component
    
    int width = heightField->GetWidth();
    int height = heightField->GetHeight();
    GLfloat *data = heightField->GetData();
    
    Bounds bounds = other.GetBounds();
    
    for (int y = bounds.b1.z; y < bounds.f3.z; y++) {
        for (int x = bounds.b1.x; x < bounds.f3.x; x++) {
            if (Sample(data, width, height, x, y) > bounds.b1.z)
                return true;
        }
    }
    
    return false;
}