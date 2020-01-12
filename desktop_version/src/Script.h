#ifndef SCRIPT_H
#define SCRIPT_H

#include <string>
#include <vector>
#include "Game.h"

#include "Enums.h"


class KeyPoll; class Graphics; class Game; class mapclass; class entityclass; class UtilityClass;class musicclass;


class scriptclass
{
public:


    scriptclass();

	void load(std::string t);
	void loadother(std::string t);


    void inline add(std::string t)
    {
        commands[scriptlength] = t;
        scriptlength++;
    }

    void clearcustom();

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

    //Script contents
    growing_vector<std::string> commands;
    growing_vector<std::string> words;
    growing_vector<std::string> txt;
    std::string scriptname;
    int position, scriptlength;
    int looppoint, loopcount;

    int scriptdelay;
    bool running;
    bool passive;
    std::string tempword;
    std::string currentletter;

    //Textbox stuff
    int textx;
    int texty;
    int r,g,b;
    int txtnumlines;

    //Misc
    int i, j, k;

    //Custom level stuff
     growing_vector <std::string>  customscript;
};

#endif /* SCRIPT_H */
