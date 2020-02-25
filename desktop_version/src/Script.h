#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include <vector>
#include <unordered_map>
#include "Game.h"
#include "ScriptX.h"
#include "Enums.h"


class KeyPoll; class Graphics; class Game; class mapclass; class entityclass; class UtilityClass;class musicclass;

struct stackframe {
    std::string script;
    int line;
};

class scriptclass
{
public:


    scriptclass();

	void load(std::string t);
	void call(std::string t);
	void loadother(std::string t);


    void inline add(std::string t)
    {
        commands[scriptlength] = t;
        scriptlength++;
    }

    void clearcustom();

    int getimage(Game& game, std::string n);

    void setvar(std::string n, std::string c);

    void updatevars();

    std::string evalvar(std::string t);

    std::string processvars(std::string t);

    void tokenize(std::string t);

    void run(KeyPoll& key, Graphics& dwgfx, Game& game, mapclass& map,
             entityclass& obj, UtilityClass& help, musicclass& music);

    void resetgametomenu(Graphics& dwgfx, Game& game,mapclass& map,
                         entityclass& obj, UtilityClass& help, musicclass& music);

    void startgamemode(int t, KeyPoll& key, Graphics& dwgfx, Game& game, mapclass& map,
                       entityclass& obj, UtilityClass& help, musicclass& music);

    void teleport(Graphics& dwgfx, Game& game, mapclass& map,
                  entityclass& obj, UtilityClass& help, musicclass& music);

    void hardreset(KeyPoll& key, Graphics& dwgfx, Game& game,mapclass& map,
                   entityclass& obj, UtilityClass& help, musicclass& music);

    void callback(std::string name);

    //Script contents
    growing_vector<std::string> commands;
    growing_vector<std::string> words;
    growing_vector<std::string> txt;
    std::string scriptname;
    int position, scriptlength = 0;
    int looppoint, loopcount = 0;

    int scriptdelay = 0;
    bool running = false;
    bool nointerrupt = false;
    bool passive = false;
    std::string tempword;
    std::string currentletter;

    //Textbox stuff
    int textx = 0;
    int texty = 0;
    int textcenterline = 0;
    int r,g,b = 0;
    int txtnumlines = 0;

    //Misc
    int i, j, k = 0;

    //Custom level stuff
     growing_vector <std::string>  customscript;

    growing_vector<scriptimage> scriptrender;

    bool loopdelay = false;

    std::unordered_map<std::string, int> labels; // key is name, value is position
    std::unordered_map<std::string, std::string> variables; // key is name, value is contents
    std::unordered_map<std::string, std::string> callbacks; // key is name, value is script

    std::vector<stackframe> callstack;

    int getpixelx = -1;
    int getpixely = -1;

    growing_vector<scriptx> active_scripts;

    bool killedviridian = false;
    int killtimer = 0;
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
    X(INT_SPECIALVAR, "trinkets", game.trinkets, 0, 0) \
    X(INT_SPECIALVAR, "crewmates", game.crewmates, 0, 0) \
    X(INT_SPECIALVAR, "coins", game.coins, 0, 0) \
    X(INT_SPECIALVAR, "battery_level", battery_level(), 0, 1) \
    X(INT_SPECIALVAR, "on_battery", ((int) on_battery()), 0, 1) \
    X(INT_SPECIALVAR, "unix_time", ((int) unix_time()), 0, 0) \
    X(STR_SPECIALVAR, "hhmmss_time", hhmmss_time(), 0, 0)

#endif /* SCRIPT_H */
