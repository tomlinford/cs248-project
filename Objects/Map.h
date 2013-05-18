#pragma once

#include "Object.h"

typedef enum
{
    TESSLVL_32,
    TESSLVL_64,
    TESSLVL_128
} TessLevel;

class Map : public Object
{
public:
    /** We should have several plane .obj files at different
     tesselation levels. Perhaps one 128 x 128, one
     64 x 64, and one 32 x 32. We should select the
     object to use based on how much detail we need
     in the map. */
    Map(TessLevel level);
    
    /** We'll need to override this as map collisions
     will have to be more robust than bounding-box */
    virtual bool Intersects(Object other);
    
private:
    Texture *heightField;
};
