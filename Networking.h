#pragma once

#include "Scene.h"

namespace Networking {
	void Init(Scene *currScene, Level *currentLevel, std::string ip_addr, char *p);
	void KeyAction(int key, int action, glm::vec2 shipOffset);
};