#ifndef SCRIPTX_H
#define SCRIPTX_H

#include <string>
#include <vector>
#include <unordered_map>
#include "Game.h"
#include "ScriptX.h"
#include "Script.h"
#include "Enums.h"
#include "GrowingVector.h"


class KeyPoll; class Graphics; class Game; class mapclass; class entityclass; class UtilityClass;class musicclass;

struct stackframe {
    std::string script;
    int line;
};

class scriptx
{
public:


    scriptx();

	void load(std::string t);
	void call(std::string t);
	void loadother(std::string t);


    void inline add(std::string t)
    {
        commands[scriptlength] = t;
        scriptlength++;
    }

    void tokenize(std::string t);

    void run(KeyPoll& key, Graphics& dwgfx, Game& game, mapclass& map,
             entityclass& obj, UtilityClass& help, musicclass& music);

    //Script contents
    growing_vector<std::string> commands;
    growing_vector<std::string> words;
    growing_vector<std::string> txt;
    std::string scriptname;
    int position, scriptlength = 0;
    int looppoint, loopcount = 0;

    int scriptdelay = 0;
    bool running = false;
    bool passive = false;
    std::string tempword;
    std::string currentletter;

    //Misc
    int i, j, k = 0;

    //Custom level stuff
    bool loopdelay = false;

    std::unordered_map<std::string, int> labels; // key is name, value is position

    std::vector<stackframe> callstack;

    int getpixelx = -1;
    int getpixely = -1;

    bool killedviridian = false;
    int killtimer = 0;
};

#endif /* SCRIPTX_H */
