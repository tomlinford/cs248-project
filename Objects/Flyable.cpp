#include "Flyable.h"

Flyable::Flyable(Model *model) : Object(model)
{
}

Flyable::Flyable(const string& filename) : Object(filename)
{
}

Ship::Ship(Model *model) : Flyable(model)
{
}

Ship::Ship(const string& filename) : Flyable(filename)
{
}