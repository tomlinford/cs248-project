#include "Flyable.h"

Flyable::Flyable(Model *model) : Object(model)
{
}

Ship::Ship(Model *model) : Flyable(model)
{
}