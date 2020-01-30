#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include <vector>
#include "Game.h"

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

    void settile_special(int x, int y, int tile, Graphics& dwgfx, Game& game, mapclass& map, entityclass& obj);

    int getimage(Game& game, std::string n);

    int getvar(std::string n);
    
    void setvar(std::string n, std::string c);

    void updatevars(Game& game, entityclass& obj);

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

    int getlabelnum(std::string thelabel);

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

    growing_vector<std::string> labelnames;
    growing_vector<int> labelpositions;
    int nlabels;

    growing_vector<std::string> variablenames;
    growing_vector<std::string> variablecontents;

    std::vector<stackframe> callstack;

    int getpixelx = -1;
    int getpixely = -1;
};

#endif /* SCRIPT_H */
