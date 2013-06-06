#pragma once

#include "Scene.h"

namespace Networking {
	void Init(Scene *currScene, Level *currentLevel, std::string ip_addr, const char *p);
	void KeyAction(int key, int action, glm::vec2 shipOffset);
	void AddBullet(glm::vec3 position, glm::vec3 velocity);
	void AddLightning();
};