#ifndef WARPCLASS_H
#define WARPCLASS_H

#include "Game.h"
#include "Entity.h"

#include <string>
#include <vector>
#include "Game.h"

class warpclass
{
public:
	growing_vector<std::string> loadlevel(int rx, int ry);
	std::string roomname;
	int coin, rcol = 0;
	bool warpx, warpy = 0;
};

#endif /* WARPCLASS_H */
