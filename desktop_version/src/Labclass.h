#ifndef LABCLASS_H
#define LABCLASS_H

#include "Game.h"
#include "Entity.h"


class labclass
{
public:
    const int* loadlevel(int rx, int ry);

    std::string roomname;
    int coin, rcol = 0;
};
#endif /* LABCLASS_H */
