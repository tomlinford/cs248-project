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
    Map(TessLevel level);
    
    /** We'll need to override this as map collisions
        will have to be more robust than bounding-box */
    virtual bool Intersects(Object other);
    
private:
    Texture *heightField;
};
