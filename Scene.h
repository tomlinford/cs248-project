#pragma once

#include "gl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Level.h"

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
    
    /** Setup functions */
    void LoadLevel(Level *level);
    
    /** Move objects, test collision, etc. */
    void Update();
    
    /** Draws the scene to the window */
    void Render();
    
    /** View matrices */
    void SetView(glm::mat4 v);
    void SetProjection(glm::mat4 p);
    
private:
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 cameraPos;
    
    Level *level;
    
    Player player;
    
    FBO *fbo;
    Program *main;
};

