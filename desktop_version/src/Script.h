#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include <vector>
#include <list>
#include <unordered_map>
#include "Game.h"
#include "Enums.h"
#include "LuaScript.h"

#define filllines(lines) commands.insert(commands.end(), lines, lines + sizeof(lines)/sizeof(lines[0]))


struct stackframe {
    std::string script;
    int line;
};

struct Script {
    std::string name;
    std::vector<std::string> contents;
    bool lua;
};

#define LAYERNAMES \
    X(belowtiles) \
    X(belowentities) \
    X(belowroomname) \
    X(belowroomtext) \
    X(belowcoincounter) \
    X(top)

namespace Layer {
    enum LayerName {
#define X(name) name,
        LAYERNAMES
#undef X
    };
};

// Script drawing stuff
struct scriptimage {
    int type = 0; // 0 for text, 1 for image, 2 for rect
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int r = 0;
    int g = 0;
    int b = 0;
    int index = 0;
    int mask_index = 0;
    int mask_x = 0;
    int mask_y = 0;
    std::string text;
    bool center = false;
    int bord = false;
    int sc = 2;
    bool persistent = false;
    int alpha = 0;
    enum Layer::LayerName layer = Layer::top;
    SDL_BlendMode blend = SDL_BLENDMODE_BLEND;
};

class scriptclass
{
public:


    scriptclass();

    void load(std::string t);
    void call(std::string t);
    void loadother(std::string t);
    void loadcustom(std::string t);


    void inline add(std::string t)
    {
        commands.push_back(t);
    }

    void clearcustom();

    int getimage(std::string n);

    void setvar(std::string n, std::string c);

    void updatevars();

    std::string evalvar(std::string t);

    std::string processvars(std::string t);

    void tokenize(std::string t);

    void renderimages(enum Layer::LayerName layer);
    void run();

    void resetgametomenu();

    void startgamemode(int t);

    void teleport();

    void hardreset();

    void callback(std::string name);

    //Script contents
    std::vector<std::string> commands;
    std::vector<std::string> words;
    std::vector<std::string> txt;
    std::string scriptname;
    int position = 0;
    int looppoint, loopcount = 0;

    int scriptdelay = 0;
    bool running, dontrunnextframe = false;
    bool nointerrupt = false;
    bool passive = false;

    //Textbox stuff
    int textx = 0;
    int texty = 0;
    int textcenterline = 0;
    int r,g,b = 0;

    //Misc
    int i, j, k = 0;

    //Custom level stuff
    std::vector<Script> customscripts;

    std::vector<scriptimage> scriptrender;

    bool loopdelay = false;

    std::unordered_map<std::string, int> labels; // key is name, value is position
    std::unordered_map<std::string, std::string> variables; // key is name, value is contents
    std::unordered_map<std::string, std::string> callbacks; // key is name, value is script

    std::vector<stackframe> callstack;

    int getpixelx = -1;
    int getpixely = -1;

    bool killedviridian = false;
    int killtimer = 0;

    bool keepcolor = false;

    const std::unordered_map<std::string, Layer::LayerName> layername_to_enum = {
#define X(name) {#name, Layer::name},
        LAYERNAMES
#undef X
    };

    std::list<lua_script> lua_scripts;

    bool is_running();

    void quit();
    void stop();
};

// Syntax: X(<type>, <name>, <value> (has to be a valid rvalue, and can only be set if a valid lvalue), <offset/indexing>, <slow, 1/0>)
// Slow variables cannot be used in expressions, but are not automatically updated every frame
#define SPECIALVARS \
    X(INT_SPECIALVAR, "deaths", game.deathcounts, 0, 0) \
    X(INT_SPECIALVAR, "player_x", obj.entities[obj.getplayer()].xp, -6, 0) \
    X(INT_SPECIALVAR, "player_y", obj.entities[obj.getplayer()].yp, -2, 0) \
    X(INT_SPECIALVAR, "player_onground", obj.entities[obj.getplayer()].onground, 0, 0) \
    X(INT_SPECIALVAR, "player_onroof", obj.entities[obj.getplayer()].onroof, 0, 0) \
    X(INT_SPECIALVAR, "gravitycontrol", game.gravitycontrol, 0, 0) \
    X(INT_SPECIALVAR, "room_x", game.roomx, 100, 0) \
    X(INT_SPECIALVAR, "room_y", game.roomy, 100, 0) \
    X(INT_SPECIALVAR, "trinkets", game.trinkets(), 0, 0) \
    X(INT_SPECIALVAR, "crewmates", game.crewmates(), 0, 0) \
    X(INT_SPECIALVAR, "coins", game.coins, 0, 0) \
    X(INT_SPECIALVAR, "battery_level", battery_level(), 0, 1) \
    X(INT_SPECIALVAR, "on_battery", ((int) on_battery()), 0, 1) \
    X(INT_SPECIALVAR, "unix_time", ((int) unix_time()), 0, 0) \
    X(STR_SPECIALVAR, "hhmmss_time", hhmmss_time(), 0, 0) \
    X(INT_SPECIALVAR, "entities", obj.entities.size(), 0, 0) \
    X(INT_SPECIALVAR, "altstate", obj.altstates, 0, 0)

extern scriptclass script;

#endif /* SCRIPT_H */
