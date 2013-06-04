#pragma once

#include "gl.h"

#include <iostream>
#include <vector>
#include "Buffer.h"
#include "Model.h"
#include "Program.h"

class Screen
{
public:
    Screen();
    ~Screen();
    
    void Draw(Program& program);
    
private:
    Model *quad;
};
