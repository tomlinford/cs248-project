#pragma once

#include "gl.h"

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/timer/timer.hpp>

#include "Level.h"

#include "Utilities/FBO.h"
#include "Utilities/Program.h"
#include "Utilities/Buffer.h"
#include "Utilities/Model.h"
#include "Utilities/OBJFile.h"
#include "Utilities/ParticleSystem.h"

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
    void SetView(glm::mat4 v) { view = v; }
    void SetProjection(glm::mat4 p) { projection = p; }
    
    /** Key events (Player 1) */
    bool keyLeft, keyRight, keyUp, keyDown;
    
    /** FPS direction in spherical coordinates (Player 2) */
    float theta, phi;
    
private:
    /** View matrices. These should be
     player-specific. The camera for player 1
     follows the ship movement and the camera
     for player follows the mouse aiming
     movement. */
    glm::mat4 view;
    glm::mat4 projection;
    glm::vec3 cameraPosition;
    glm::vec3 lightPosition;
    
    /** The current level */
    Level *level;
    
    /** Player information */
    Player player;
    glm::vec2 shipOffset;
    
    /** Global animation timer */
    boost::timer::cpu_timer timer;
    boost::timer::cpu_times times;
    
    /** Shaders and FBOs */
    Program *main;
    Program *terrain;
    FBO *fbo;
    
    /** Global objects */
    ParticleSystem particle_sys;
    
    /** Update helpers */
    void HandleKeys();
    void UpdateObjects(float elapsedSeconds);
    void HandleCollisions();
    void UpdateView(float elapsedSeconds);
};

