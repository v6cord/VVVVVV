#ifndef FINALCLASS_H
#define FINALCLASS_H

#include "Game.h"
#include "Entity.h"

#include <string>
#include <vector>
#include "Game.h"

class finalclass
{
public:
    growing_vector<std::string> loadlevel(int rx, int ry, Game& game, entityclass& obj);

    std::string roomname;
    int coin, rcol;
    bool warpx, warpy;
};

#endif /* FINALCLASS_H */
