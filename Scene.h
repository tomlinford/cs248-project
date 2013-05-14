#pragma once

#include "gl.h"

#include <glm/glm.hpp>

#include "FBO.h"

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
    
    Player player;
    
    FBO *fbo;
};

