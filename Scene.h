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
#include "Utilities/Terrain.h"

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
    
    /** Move objects, test collision, etc. 
     Also forward information to server. Perhaps
     invoke a separate networking class. */
    void Update();
    
    /** Draws the scene to the window */
    void Render();
    
    /** Accessors for view matrices */
    void SetView(glm::mat4 v);
    void SetProjection(glm::mat4 p);

	void SetTerrain(float *terrainMap, size_t size);
    
private:
    /** View matrices. These should be
     player-specific. The camera for player 1
     follows the ship movement and the camera
     for player follows the mouse aiming
     movement. */
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 cameraPos;
    
    /** The current level */
    Level *level;
    
    /** The current player view */
    Player player;
    
    /** For post-processing effects */
    FBO *fbo;
    
    /** Main shader */
    Program *main;

	Terrain *terrain;
};

