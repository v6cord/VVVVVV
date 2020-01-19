#ifndef OTHERLEVEL_H
#define OTHERLEVEL_H

#include "Game.h"
#include "Entity.h"

#include <string>
#include <vector>
#include "Game.h"

class otherlevelclass
{
public:
    enum
    {
        BLOCK = 0,
        TRIGGER,
        DAMAGE,
        DIRECTIONAL,
        SAFE,
        ACTIVITY
    };

    otherlevelclass();
    void addline(std::string t);
    growing_vector<std::string> loadlevel(int rx, int ry , Game& game, entityclass& obj);

    std::string roomname;

    int roomtileset = 0;
    int i = 0;

    // roomtext thing in other level
    bool roomtexton = false;
    int roomtextx, roomtexty, roomtextnumlines = 0;
    growing_vector<std::string> roomtext;
};

#endif /* OTHERLEVEL_H */
