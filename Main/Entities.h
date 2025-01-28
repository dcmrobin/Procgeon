#ifndef ENTITIES_H
#define ENTITIES_H

#include <Arduino.h>

struct Damsel {
    float x, y;
    float speed;
    bool dead;
    bool followingPlayer;
};

extern Damsel damsel[1];

#endif