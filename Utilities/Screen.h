//
//  Screen.h
//  CubeRunner
//
//  Created by Ben-han Sung on 4/18/13.
//
//

#pragma once

#include "gl.h"
#include <iostream>
#include "Program.h"

class Screen
{
public:
    Screen();
    ~Screen();
    
    void Draw(Program& program);
    
private:
    GLuint handle, tex, indices;
    GLint vertexID, textureID;
};
