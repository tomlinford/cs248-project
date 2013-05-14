#pragma once

#include "gl.h"

#include <iostream>
#include "Program.h"

class Screen
{
public:
    Screen();
    ~Screen();
    
    void Draw(Program& program, glm::mat4 viewProjection);
    
private:
    GLuint handle, tex, indices;
    GLint vertexID, textureID;
};
