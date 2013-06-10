#include "Sound.h"

#include <iostream>
#include <fmod.hpp>
#include <fmod_errors.h>

using namespace FMOD;
using namespace std;

namespace Sound {
	// main FMOD system
	static System *system;

	// various sounds
	static FMOD::Sound *menuMusic;
	static FMOD::Sound *thunder;
	static FMOD::Sound *laser;
    static FMOD::Sound *select;
    static FMOD::Sound *back;
    
	// FMOD channel
	static Channel *channel = NULL;

	static void ERRCHECK(FMOD_RESULT result) {
		if (result != FMOD_OK) {
			cerr << "FMOD error: " << FMOD_ErrorString(result) << endl;
		}
	}

	extern void Init() {
		ERRCHECK(System_Create(&system));
		ERRCHECK(system->init(32, FMOD_INIT_NORMAL, 0));
		ERRCHECK(system->createSound("Sounds/Graphiqs_Groove_-_09_-_Deep_Sky_Blue.mp3", FMOD_HARDWARE, 0, &menuMusic));
		ERRCHECK(system->createSound("Sounds/thunder.mp3", FMOD_HARDWARE, 0, &thunder));
		ERRCHECK(system->createSound("Sounds/laser.wav", FMOD_HARDWARE, 0, &laser));
        ERRCHECK(system->createSound("Sounds/menu_select.wav", FMOD_HARDWARE, 0, &select));
        ERRCHECK(system->createSound("Sounds/menu_back.wav", FMOD_HARDWARE, 0, &back));
		ERRCHECK(thunder->setMode(FMOD_LOOP_OFF));
		//ERRCHECK(system->playSound(FMOD_CHANNEL_FREE, menuMusic, false, &channel));
	}

	extern void PlayThunder() {
		ERRCHECK(system->playSound(FMOD_CHANNEL_FREE, thunder, false, &channel));
	}

	extern void PlayLaser() {
		ERRCHECK(system->playSound(FMOD_CHANNEL_FREE, laser, false, &channel));
	}
    
    extern void PlaySelect() {
		ERRCHECK(system->playSound(FMOD_CHANNEL_FREE, select, false, &channel));
	}
    
    extern void PlayBack() {
		ERRCHECK(system->playSound(FMOD_CHANNEL_FREE, back, false, &channel));
	}
};