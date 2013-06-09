#pragma once

#include "gl.h"

#include <thread>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <boost/timer/timer.hpp>

#include "Level.h"
#include "Flyable.h"
#include "Turret.h"

#include "Utilities/FBO.h"
#include "Utilities/Screen.h"
#include "Utilities/Program.h"
#include "Utilities/Buffer.h"
#include "Utilities/Model.h"
#include "Utilities/OBJFile.h"
#include "Utilities/ParticleSystem.h"
#include "Utilities/Frustum.h"

typedef enum
{
    PLAYER1,
    PLAYER2
} Player;

class Scene
{
public:
    Scene();
    ~Scene();
    
    /** Setup functions */
    void LoadLevel(Level *l, Player p);
    
    /** Move objects, test collision, etc. 
     Also forward information to server. Perhaps
     invoke a separate networking class. */
    void Update();
    
    /** Draws the scene to the window */
    void Render();
    
    /** Accessors for view matrices */
    void SetFrustum(float FOV, float ratio, float near, float far) {
        frustum->LoadView(FOV, ratio, near, far);
    }
    void SetView(glm::mat4 v) { view = v; }
    void SetProjection(glm::mat4 p) { projection = p; }
    void SetMinimapView(glm::mat4 v) { minimapView = v; }
    void SetMinimapProjection(glm::mat4 p) { minimapProjection = p; }
    
    /** Get minimap texture */
    Texture *GetMinimapTexture() { return minimapTexture; }
    
    /** Update for FBO */
    void UpdateFBO(GLuint width, GLuint height);

    /** Ship offset control */
	glm::vec2 GetShipOffset() { return shipOffset; }
	void SetShipOffset(glm::vec2 offs) { shipOffset = offs; }

	/** Lightning effects */
	void AddLightning(bool acquireLock);
	float GetLastLightning() { return lastLightning; }

	/** Get last time */
	float GetTime() { return lastTime; }
    
    /** Control information. 
     Key events (Player 1)
     Ship offset (Player 1) 
     Mouse events (Player 2)
     Viewing angles (Player 2) */
    bool keyLeft, keyRight, keyUp, keyDown;
    glm::vec2 shipOffset;
    bool mouseLeft, mouseRight;
    float theta, phi;
    
    /** Game over / level over. If the game is over but
     the level is not, then the player died. If the game
     is over and the level is also over, the player should 
     continue to the next level */
    bool gameOver, levelOver;
    
    /** Player information */
    Player player;
    int score;
    int totalScore;
    
    /** The current level */
    Level *level;
    
private:
    /** View matrices. These should be
     player-specific. The camera for player 1
     follows the ship movement and the camera
     for player follows the mouse aiming
     movement. */
    glm::mat4 view;
    glm::mat4 projection;
    glm::mat4 minimapView;
    glm::mat4 minimapProjection;
    glm::mat4 viewProjection;
    glm::mat4 prevViewProjection;
    glm::vec3 cameraPosition;
    glm::vec3 lightPosition;
    
    /* Game update thread */
    thread *updateThread;
    bool finished;
    
    /* Level loading synchronization */
    std::condition_variable cond;
    std::mutex mutex;
    
    /* View frustrum */
    Frustum *frustum;
    
    /** Global animation timer */
    boost::timer::cpu_timer *timer;
    boost::timer::cpu_times times;
    float lastTime;

	/** Last lightning */
	float lastLightning;
    
    /** Shaders and FBO stuff */
    Program *main;
    Program *terrain;
    Program *screenProgram;
    Program *vblur;
    Program *hblur;
    Program *mblur;
    Program *velocity;
    Texture *depthTexture;
    Texture *glowTexture;
    Texture *hblurTexture;
    Texture *vblurTexture;
    Texture *mblurTexture;
    Texture *sceneTexture;
    Texture *velocityTexture;
    Texture *combinedTexture;
    Texture *minimapTexture;
    FBO *fbo;
    
    /** Global objects */
    ParticleSystem particle_sys;
    Screen *screen;
    
    /** Update helpers */
    void HandleKeys(float elapsedSeconds);
    void HandleMouse(float elapsedSeconds);
    void UpdateObjects(float elapsedSeconds);
    void HandleCollisions(float elapsedSeconds);
    void UpdateView(float elapsedSeconds);
    void AddMissiles();
    void Reset();
    
    /** Rendering helpers */
    void RenderObjects(DrawMode mode);
    void RenderGlowMap();
    void RenderVelocityTexture();
    void RenderScene();
    void RenderMinimap();
    void PostProcess();
};

