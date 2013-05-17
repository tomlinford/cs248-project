#pragma once

#include "gl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Utilities/FBO.h"
#include "Utilities/Program.h"
#include "Utilities/Buffer.h"
#include "Utilities/Model.h"
#include "Utilities/OBJFile.h"

typedef enum
{
    PLAYER1,
    PLAYER2
} Player;

class Scene
{
public:
    Scene(Player player);
    ~Scene();
    
    /** Draw functions */
    void Render();
    
    /** View matrices */
    void SetView(glm::mat4 v);
    void SetProjection(glm::mat4 p);
    
private:
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 cameraPos;
    
    Model *ship;
    
    Player player;
    
    FBO *fbo;
    Program *main;
};

