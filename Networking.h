#pragma once

#include "Scene.h"

namespace Networking {
	void Init(Scene *currScene, Level *currentLevel, std::string ip_addr, const char *p, int currLevelNum);

	/** Returns true if server responds to ping request */
	bool PingServer(std::string ip_addr);

	void KeyAction(int key, int action, glm::vec2 shipOffset);
	void AddBullet(glm::vec3 position, glm::vec3 velocity);
	void AddLightning();
	void SetHealth(float health);
	void SetScore(int score);
	void GameOver();
};