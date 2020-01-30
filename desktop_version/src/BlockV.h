#ifndef BLOCKV_H
#define BLOCKV_H

#include "SDL.h"
#include <string>

class blockclass
{
public:
    blockclass();

    void clear();

    void rectset(const int xi, const int yi, const int wi, const int hi);
public:
    //Fundamentals
    bool active = false;
    SDL_Rect rect = {0};
    int type = 0;
    int trigger = 0;
    int xp, yp, wp, hp = 0;
    std::string script, prompt;
    int r, g, b = 0;

    //These would come from the sprite in the flash
    float x = 0.0;
    float y = 0.0;

    bool onetime = false;
};

#endif /* BLOCKV_H */
