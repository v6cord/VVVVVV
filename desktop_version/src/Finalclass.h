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
    std::vector<int> loadlevel(int rx, int ry);

    std::string roomname;
    int coin, rcol = 0;
    bool warpx, warpy = false;
};

#endif /* FINALCLASS_H */
