//
//  LeapListener.cpp
//  GameOfDrones
//
//  Created by Ben-han Sung on 7/29/13.
//
//

#include "LeapListener.h"

using namespace Leap;

void LeapListener::onInit(const Controller& controller) {
    std::cout << "Initialized" << std::endl;
}

void LeapListener::onConnect(const Controller& controller) {
    std::cout << "Connected" << std::endl;
    controller.enableGesture(Gesture::TYPE_CIRCLE);
}

void LeapListener::onDisconnect(const Controller& controller) {
    //Note: not dispatched when running in a debugger.
    std::cout << "Disconnected" << std::endl;
}

void LeapListener::onExit(const Controller& controller) {
    std::cout << "Exited" << std::endl;
}

void LeapListener::onFrame(const Controller& controller) {
    // std::cout << "On Frame" << std::endl;
}

void LeapListener::onFocusGained(const Controller& controller) {
    std::cout << "Focus Gained" << std::endl;
}

void LeapListener::onFocusLost(const Controller& controller) {
    std::cout << "Focus Lost" << std::endl;
}
