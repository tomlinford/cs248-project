//
//  LeapListener.h
//  GameOfDrones
//
//  Created by Ben-han Sung on 7/29/13.
//
//

#ifndef __GameOfDrones__LeapListener__
#define __GameOfDrones__LeapListener__

#include "Leap.h"
#include <iostream>

class LeapListener : public Leap::Listener
{
public:
    virtual void onInit(const Leap::Controller&);
    virtual void onConnect(const Leap::Controller&);
    virtual void onDisconnect(const Leap::Controller&);
    virtual void onExit(const Leap::Controller&);
    virtual void onFrame(const Leap::Controller&);
    virtual void onFocusGained(const Leap::Controller&);
    virtual void onFocusLost(const Leap::Controller&);
};

#endif /* defined(__GameOfDrones__LeapListener__) */
