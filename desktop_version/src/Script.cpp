#include "Script.h"
#include <shunting-yard.h>
#include <algorithm>
#include <builtin-features.inc>
#include <cstdlib>
#include <cstring>
#include <iostream>
#include <optional>
#include <stdexcept>
#include "Graphics.h"
#include "ScriptX.h"

#include "Entity.h"
#include "FileSystemUtils.h"
#include "KeyPoll.h"
#include "Map.h"
#include "Music.h"
#include "Utilities.h"
#include "Maths.h"

scriptclass::scriptclass() {
    // Init
    b = 0;
    g = 0;
    i = 0;
    j = 0;
    k = 0;
    r = 0;
    textx = 0;
    texty = 0;
    textcenterline = 0;
    txtnumlines = 0;

    variables.clear();

    // I really hate this file, by the way
}

void scriptclass::clearcustom() { customscript.clear(); }

packToken cparse_rand(TokenMap scope) {
    int N = scope["N"].asInt();
    int result = fRandom() * N;
    return result;
}

std::string scriptclass::evalvar(std::string expr) {
    static bool CPARSE_INITIALIZED = false;
    if (!CPARSE_INITIALIZED) {
        cparse_startup();
        CPARSE_INITIALIZED = true;
    }
    TokenMap vars;
    for (auto variable : script.variables) {
        if (variable.first == "") continue;
        try {
            auto contents = std::stod(variable.second);
            vars[variable.first] = contents;
        } catch (const std::invalid_argument& ex) {
            auto contents = variable.second;
            vars[variable.first] = contents;
        }
    }
    vars["rand"] = CppFunction(&cparse_rand, {"N"}, "rand");
    auto token = calculator::calculate(expr.c_str(), vars);
    try {
        return token.asString();
    } catch (const bad_cast& ex) {
        try {
            return dtos(token.asDouble());
        } catch (const bad_cast& ex) {
            return token.str();
        }
    }
}

int scriptclass::getimage(Game& game, std::string n) {
    for (std::size_t i = 0; i < game.script_images.size(); i++) {
        if (game.script_image_names[i] == n) {
            return i;
        }
    }
    return -1;
}

enum specialvar_type { INT_SPECIALVAR, STR_SPECIALVAR };

template <specialvar_type TYPE, typename T>
static void try_set_lvalue(T& ref, std::string value, int offset) {
    if constexpr (TYPE == INT_SPECIALVAR) {
        ref = ss_toi(value) + offset;
    } else if constexpr (TYPE == STR_SPECIALVAR) {
        ref = value;
    }
}

template <specialvar_type TYPE, typename T>
static void try_set_lvalue(const T&& ref, std::string value, int offset) {}

template <specialvar_type TYPE, typename T>
static std::string get_specialvar(const T&& ref, int offset) {
    if constexpr (TYPE == INT_SPECIALVAR) {
        return std::to_string(ref - offset);
    } else if constexpr (TYPE == STR_SPECIALVAR) {
        return ref;
    }
}

void scriptclass::setvar(std::string n, std::string c) {
    variables[n] = c;

#define X(t, k, v, i, s)                  \
    if (n == k) {                         \
        try_set_lvalue<(t)>((v), c, (i)); \
        return;                           \
    }
    SPECIALVARS
#undef X
}

std::string scriptclass::processvars(std::string t) {
    std::string tempstring = "";
    std::string tempvar = "";
    std::string currentletter = "";
    bool readingvar = false;
    for (size_t i = 0; i < t.length(); i++) {
        currentletter = t.substr(i, 1);
        if (readingvar) {
            if (currentletter == "%") {
                readingvar = false;
                std::string temp = "%" + tempvar + "%";
                if (variables.find(tempvar) != variables.end()) {
                    temp = variables[tempvar];
                } else {
                    bool special = false;
#define X(t, k, v, i, s)                                 \
    if (s && (k) == tempvar) {                           \
        temp = get_specialvar<(t)>(std::move((v)), (i)); \
        special = true;                                  \
    }
                    SPECIALVARS
#undef X
                    if (!special) {
                        try {
                            auto eval = evalvar(tempvar);
                            temp = eval;
                        } catch (const std::exception& ex) {
                        }
                    }
                }
                tempstring += temp;
                tempvar = "";
            } else {
                tempvar += currentletter;
            }
        } else if (currentletter == "%") {
            readingvar = true;
        } else {
            tempstring += currentletter;
        }
    }
    if (readingvar) {
        tempstring += "%" + tempvar;
    }
    return tempstring;
}

void scriptclass::updatevars() {
#define X(t, k, v, i, s) \
    if (!(s)) setvar(k, get_specialvar<(t)>(std::move(v), (i)));
    SPECIALVARS
#undef X
}

void scriptclass::run(KeyPoll& key, Graphics& dwgfx, Game& game, mapclass& map,
                      entityclass& obj, UtilityClass& help, musicclass& music) {
    for (auto script : active_scripts) {
        script.run(key, dwgfx, game, map, obj, help, music);
    }
}

void scriptclass::resetgametomenu(Graphics& dwgfx, Game& game, mapclass& map,
                                  entityclass& obj, UtilityClass& help,
                                  musicclass& music) {
    game.gamestate = TITLEMODE;
    FILESYSTEM_unmountassets(dwgfx);
    dwgfx.flipmode = false;
    obj.nentity = 0;
    dwgfx.fademode = 4;
    game.createmenu("gameover");
}

void scriptclass::startgamemode(int t, KeyPoll& key, Graphics& dwgfx,
                                Game& game, mapclass& map, entityclass& obj,
                                UtilityClass& help, musicclass& music) {
    dwgfx.noclear = false;
    dwgfx.mapimage = std::nullopt;
    active_scripts.clear();

    scriptx scr;

    switch (t) {
        case 0:  // Normal new game
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.start(obj, music);
            game.jumpheld = true;
            dwgfx.showcutscenebars = true;
            dwgfx.cutscenebarspos = 320;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;

            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            scr.load("intro");
            active_scripts.push_back(scr);
            break;
        case 1:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.start(obj, music);
            game.loadtele(map, obj, music);
            game.gravitycontrol = game.savegc;
            game.jumpheld = true;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;

            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            dwgfx.fademode = 4;
            break;
        case 2:  // Load Quicksave
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.start(obj, music);
            game.loadquick(map, obj, music);
            game.gravitycontrol = game.savegc;
            game.jumpheld = true;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;

            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            // a very special case for here needs to ensure that the tower is
            // set correctly
            if (map.towermode) {
                map.resetplayer(dwgfx, game, obj, music);

                i = obj.getplayer();
                map.ypos = obj.entities[i].yp - 120;
                map.bypos = map.ypos / 2;
                map.cameramode = 0;
                map.colsuperstate = 0;
            }
            dwgfx.fademode = 4;
            break;
        case 3:
            // Start Time Trial 1
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.nocutscenes = true;
            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 0;
            game.timetrialpar = 75;
            game.timetrialshinytarget = 2;

            music.fadeout();
            game.gamestate = GAMEMODE;
            game.starttrial(game.timetriallevel, obj, music);
            game.jumpheld = true;

            if (dwgfx.setflipmode) dwgfx.flipmode = true;  // set flipmode
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            dwgfx.fademode = 4;
            break;
        case 4:
            // Start Time Trial 2
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.nocutscenes = true;
            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 1;
            game.timetrialpar = 165;
            game.timetrialshinytarget = 4;

            music.fadeout();
            game.gamestate = GAMEMODE;
            game.starttrial(game.timetriallevel, obj, music);
            game.jumpheld = true;

            if (dwgfx.setflipmode) dwgfx.flipmode = true;  // set flipmode
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            dwgfx.fademode = 4;
            break;
        case 5:
            // Start Time Trial 3 tow
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.nocutscenes = true;
            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 2;
            game.timetrialpar = 105;
            game.timetrialshinytarget = 2;

            music.fadeout();
            game.gamestate = GAMEMODE;
            game.starttrial(game.timetriallevel, obj, music);
            game.jumpheld = true;

            if (dwgfx.setflipmode) dwgfx.flipmode = true;  // set flipmode
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            dwgfx.fademode = 4;
            break;
        case 6:
            // Start Time Trial 4 station
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.nocutscenes = true;
            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 3;
            game.timetrialpar = 200;
            game.timetrialshinytarget = 5;

            music.fadeout();
            game.gamestate = GAMEMODE;
            game.starttrial(game.timetriallevel, obj, music);
            game.jumpheld = true;

            if (dwgfx.setflipmode) dwgfx.flipmode = true;  // set flipmode
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            dwgfx.fademode = 4;
            break;
        case 7:
            // Start Time Trial 5 warp
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.nocutscenes = true;
            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 4;
            game.timetrialpar = 120;
            game.timetrialshinytarget = 1;

            music.fadeout();
            game.gamestate = GAMEMODE;
            game.starttrial(game.timetriallevel, obj, music);
            game.jumpheld = true;

            if (dwgfx.setflipmode) dwgfx.flipmode = true;  // set flipmode
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            dwgfx.fademode = 4;
            break;
        case 8:
            // Start Time Trial 6// final level!
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.nocutscenes = true;
            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 5;
            game.timetrialpar = 135;
            game.timetrialshinytarget = 1;

            music.fadeout();
            map.finalmode = true;  // Enable final level mode
            map.finalx = 46;
            map.finaly = 54;  // Current
            map.final_colormode = false;
            map.final_mapcol = 0;
            map.final_colorframe = 0;
            game.gamestate = GAMEMODE;
            game.starttrial(game.timetriallevel, obj, music);
            game.jumpheld = true;

            if (dwgfx.setflipmode) dwgfx.flipmode = true;  // set flipmode
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            dwgfx.fademode = 4;
            break;
        case 9:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.nodeathmode = true;
            game.start(obj, music);
            game.jumpheld = true;
            dwgfx.showcutscenebars = true;
            dwgfx.cutscenebarspos = 320;
            // game.starttest(obj, music);
            // music.play(4);

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;

            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            scr.load("intro");
            active_scripts.push_back(scr);
            break;
        case 10:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.nodeathmode = true;
            game.nocutscenes = true;

            game.start(obj, music);
            game.jumpheld = true;
            dwgfx.showcutscenebars = true;
            dwgfx.cutscenebarspos = 320;
            // game.starttest(obj, music);
            // music.play(4);

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;

            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            scr.load("intro");
            active_scripts.push_back(scr);
            break;
        case 11:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);

            game.startspecial(0, obj, music);
            game.jumpheld = true;

            // Secret lab, so reveal the map, give them all 20 trinkets
            for (int j = 0; j < ed.maxheight; j++)
                for (i = 0; i < ed.maxwidth; i++)
                    map.explored[i + (j * ed.maxwidth)] = 1;

            for (int j = 0; j < 20; j++) obj.collect[j] = true;
            game.trinkets = 20;
            game.insecretlab = true;
            map.showteleporters = true;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;

            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            music.play(11);
            dwgfx.fademode = 4;
            break;
        case 12:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            music.fadeout();

            game.lastsaved = 2;

            game.crewstats[game.lastsaved] = true;
            game.inintermission = true;
            game.companion = 11;
            game.supercrewmate = true;
            game.scmprogress = 0;
            map.finalmode = true;
            map.finalx = 41;
            map.finaly = 56;
            map.final_colormode = false;
            map.final_mapcol = 0;
            map.final_colorframe = 0;
            game.startspecial(1, obj, music);
            game.jumpheld = true;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            scr.load("intermission_1");
            active_scripts.push_back(scr);
            break;
        case 13:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            music.fadeout();

            game.lastsaved = 3;

            game.crewstats[game.lastsaved] = true;
            game.inintermission = true;
            game.companion = 11;
            game.supercrewmate = true;
            game.scmprogress = 0;
            map.finalmode = true;
            map.finalx = 41;
            map.finaly = 56;
            map.final_colormode = false;
            map.final_mapcol = 0;
            map.final_colorframe = 0;
            game.startspecial(1, obj, music);
            game.jumpheld = true;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            scr.load("intermission_1");
            active_scripts.push_back(scr);
            break;
        case 14:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            music.fadeout();

            game.lastsaved = 4;

            game.crewstats[game.lastsaved] = true;
            game.inintermission = true;
            game.companion = 11;
            game.supercrewmate = true;
            game.scmprogress = 0;
            map.finalmode = true;
            map.finalx = 41;
            map.finaly = 56;
            map.final_colormode = false;
            map.final_mapcol = 0;
            map.final_colorframe = 0;
            game.startspecial(1, obj, music);
            game.jumpheld = true;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            scr.load("intermission_1");
            active_scripts.push_back(scr);
            break;
        case 15:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            music.fadeout();

            game.lastsaved = 5;

            game.crewstats[game.lastsaved] = true;
            game.inintermission = true;
            game.companion = 11;
            game.supercrewmate = true;
            game.scmprogress = 0;
            map.finalmode = true;
            map.finalx = 41;
            map.finaly = 56;
            map.final_colormode = false;
            map.final_mapcol = 0;
            map.final_colorframe = 0;
            game.startspecial(1, obj, music);
            game.jumpheld = true;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            scr.load("intermission_1");
            active_scripts.push_back(scr);
            break;
        case 16:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            music.fadeout();

            game.lastsaved = 2;

            game.crewstats[game.lastsaved] = true;
            game.inintermission = true;
            map.finalmode = true;
            map.finalx = 41;
            map.finaly = 56;
            map.final_colormode = false;
            map.final_mapcol = 0;
            map.final_colorframe = 0;
            game.startspecial(1, obj, music);
            game.jumpheld = true;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            scr.load("intermission_2");
            active_scripts.push_back(scr);
            break;
        case 17:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            music.fadeout();

            game.lastsaved = 3;

            game.crewstats[game.lastsaved] = true;
            game.inintermission = true;
            map.finalmode = true;
            map.finalx = 41;
            map.finaly = 56;
            map.final_colormode = false;
            map.final_mapcol = 0;
            map.final_colorframe = 0;
            game.startspecial(1, obj, music);
            game.jumpheld = true;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            scr.load("intermission_2");
            active_scripts.push_back(scr);
            break;
        case 18:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            music.fadeout();

            game.lastsaved = 4;

            game.crewstats[game.lastsaved] = true;
            game.inintermission = true;
            map.finalmode = true;
            map.finalx = 41;
            map.finaly = 56;
            map.final_colormode = false;
            map.final_mapcol = 0;
            map.final_colorframe = 0;
            game.startspecial(1, obj, music);
            game.jumpheld = true;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            scr.load("intermission_2");
            active_scripts.push_back(scr);
            break;
        case 19:
            game.gamestate = GAMEMODE;
            hardreset(key, dwgfx, game, map, obj, help, music);
            music.fadeout();

            game.lastsaved = 5;

            game.crewstats[game.lastsaved] = true;
            game.inintermission = true;
            map.finalmode = true;
            map.finalx = 41;
            map.finaly = 56;
            map.final_colormode = false;
            map.final_mapcol = 0;
            map.final_colorframe = 0;
            game.startspecial(1, obj, music);
            game.jumpheld = true;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            scr.load("intermission_2");
            active_scripts.push_back(scr);
            break;
#if !defined(NO_CUSTOM_LEVELS)
        case 20:
            // Level editor
            hardreset(key, dwgfx, game, map, obj, help, music);
            ed.reset();
            music.fadeout();

            map.teleporters.clear();
            game.gamestate = EDITORMODE;
            game.jumpheld = true;

            if (dwgfx.setflipmode) dwgfx.flipmode = true;  // set flipmode
            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            ed.generatecustomminimap(dwgfx, map);
            dwgfx.fademode = 4;
            break;
        case 21:  // play custom level (in editor)
            game.gamestate = GAMEMODE;
            music.fadeout();
            hardreset(key, dwgfx, game, map, obj, help, music);
            // If warpdir() is used during playtesting, we need to set it back
            // after!
            for (int j = 0; j < ed.maxheight; j++) {
                for (int i = 0; i < ed.maxwidth; i++) {
                    ed.kludgewarpdir[i + (j * ed.maxwidth)] =
                        ed.level[i + (j * ed.maxwidth)].warpdir;
                }
            }
            game.customstart(obj, music);
            game.jumpheld = true;

            ed.vceversion = VCEVERSION;
            ed.ghosts.clear();

            map.custommode = true;
            map.customx = 100;
            map.customy = 100;

            // dwgfx.showcutscenebars = true;
            // dwgfx.cutscenebarspos = 320;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;

            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            if (map.towermode) {
                // Undo player x/y adjustments and realign camera on checkpoint
                map.resetplayer(dwgfx, game, obj, music);
                map.realign_tower();  // resetplayer only realigns if room
                                      // differs
            }
            if (ed.levmusic > 0) {
                music.play(ed.levmusic);
            } else {
                music.currentsong = -1;
            }
            // call("intro");
            break;
        case 22:  // play custom level (in game)
            // Initilise the level
            // First up, find the start point
            ed.weirdloadthing(ed.ListOfMetaData[game.playcustomlevel].filename,
                              dwgfx, map, game);
            ed.findstartpoint(game);

            game.gamestate = GAMEMODE;
            music.fadeout();
            hardreset(key, dwgfx, game, map, obj, help, music);
            game.customstart(obj, music);
            game.jumpheld = true;

            map.custommodeforreal = true;
            map.custommode = true;
            map.customx = 100;
            map.customy = 100;

            // dwgfx.showcutscenebars = true;
            // dwgfx.cutscenebarspos = 320;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;

            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

            ed.generatecustomminimap(dwgfx, map);
            map.customshowmm = true;
            if (ed.levmusic > 0) {
                music.play(ed.levmusic);
            } else {
                music.currentsong = -1;
            }
            dwgfx.fademode = 4;
            // call("intro");
            break;
        case 23:  // Continue in custom level
                  // Initilise the level
            // First up, find the start point
            ed.weirdloadthing(ed.ListOfMetaData[game.playcustomlevel].filename,
                              dwgfx, map, game);
            ed.findstartpoint(game);

            game.gamestate = GAMEMODE;
            music.fadeout();
            hardreset(key, dwgfx, game, map, obj, help, music);
            map.custommodeforreal = true;
            map.custommode = true;
            map.customx = 100;
            map.customy = 100;

            game.customstart(obj, music);
            game.customloadquick(
                ed.ListOfMetaData[game.playcustomlevel].filename, map, obj,
                music, dwgfx, game);
            game.jumpheld = true;
            game.gravitycontrol = game.savegc;

            // dwgfx.showcutscenebars = true;
            // dwgfx.cutscenebarspos = 320;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;

            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            /* Handled by load
            if(ed.levmusic>0){
              music.play(ed.levmusic);
            }else{
              music.currentsong=-1;
                }
                */
            ed.generatecustomminimap(dwgfx, map);
            dwgfx.fademode = 4;
            // call("intro");
            break;

        case 24:  // Custom level time trial!
            // Load the level first
            game.incustomtrial = true;
            ed.weirdloadthing(ed.ListOfMetaData[game.playcustomlevel].filename,
                              dwgfx, map, game);
            // ...then find the start point
            ed.findstartpoint(game);

            game.gamestate = GAMEMODE;  // Set the gamemode
            music.fadeout();            // Fade out the music
            hardreset(key, dwgfx, game, map, obj, help,
                      music);              // Reset everything!!
            map.custommodeforreal = true;  // Yep, it's technically
            map.custommode = true;         // a custom level

            map.customx = 100;
            map.customy = 100;

            game.customstart(obj,
                             music);  // I honestly have no idea what this does
            // Actually, the level is already loaded! This is just to be safe...
            game.customloadquick(
                ed.ListOfMetaData[game.playcustomlevel].filename, map, obj,
                music, dwgfx, game);
            game.jumpheld = true;
            game.gravitycontrol = game.savegc;

            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 21;
            game.timetrialpar = ed.customtrials[game.currenttrial].par;
            game.timetrialshinytarget =
                ed.customtrials[game.currenttrial].trinkets;

            game.savex = ed.customtrials[game.currenttrial].startx;
            game.savey = ed.customtrials[game.currenttrial].starty;
            game.saverx = ed.customtrials[game.currenttrial].roomx;
            game.savery = ed.customtrials[game.currenttrial].roomy;
            game.savegc = ed.customtrials[game.currenttrial].startf;
            game.savedir = 1;
            game.savepoint = 0;
            game.gravitycontrol = ed.customtrials[game.currenttrial].startf;
            game.coins = 0;
            game.trinkets = 0;
            game.crewmates = 0;
            game.state = 0;
            game.deathseq = -1;
            game.lifeseq = 0;

            // set flipmode
            if (dwgfx.setflipmode) dwgfx.flipmode = true;

            if (obj.nentity == 0) {
                obj.createentity(game, game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer(dwgfx, game, obj, music);
            }
            map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
            // music.play(-1);
            music.currentsong = -1;
            ed.generatecustomminimap(dwgfx, map);
            dwgfx.fademode = 4;
            // call("intro");
            break;

#endif
        case 100:
            game.savestats(map, dwgfx, music);

            SDL_Quit();
            exit(0);
            break;
    }
}

void scriptclass::teleport(Graphics& dwgfx, Game& game, mapclass& map,
                           entityclass& obj, UtilityClass& help,
                           musicclass& music) {
    // er, ok! Teleport to a new area, so!
    // A general rule of thumb: if you teleport with a companion, get rid of
    // them!
    game.companion = 0;

    i = obj.getplayer();  // less likely to have a serious collision error if
                          // the player is centered
    obj.entities[i].xp = 150;
    obj.entities[i].yp = 110;
    if (!map.custommode)
        if (game.teleport_to_x == 17 && game.teleport_to_y == 17)
            obj.entities[i].xp = 88;  // prevent falling!

    if (game.teleportscript == "levelonecomplete") {
        game.teleport_to_x = 2;
        game.teleport_to_y = 11;
    } else if (game.teleportscript == "gamecomplete") {
        game.teleport_to_x = 2;
        game.teleport_to_y = 11;
    }

    game.gravitycontrol = 0;
    map.gotoroom(100 + game.teleport_to_x, 100 + game.teleport_to_y, dwgfx,
                 game, obj, music);
    j = obj.getteleporter();
    obj.entities[j].state = 2;
    game.teleport_to_new_area = false;

    game.savepoint = obj.entities[j].para;
    game.savex = obj.entities[j].xp + 44;
    game.savey = obj.entities[j].yp + 44;
    game.savegc = 0;

    game.saverx = game.roomx;
    game.savery = game.roomy;
    game.savedir = obj.entities[obj.getplayer()].dir;

    if (!map.custommode) {
        if (game.teleport_to_x == 0 && game.teleport_to_y == 0) {
            game.state = 4020;
        } else if (game.teleport_to_x == 0 && game.teleport_to_y == 16) {
            game.state = 4030;
        } else if (game.teleport_to_x == 7 && game.teleport_to_y == 9) {
            game.state = 4040;
        } else if (game.teleport_to_x == 8 && game.teleport_to_y == 11) {
            game.state = 4050;
        } else if (game.teleport_to_x == 14 && game.teleport_to_y == 19) {
            game.state = 4030;
        } else if (game.teleport_to_x == 17 && game.teleport_to_y == 12) {
            game.state = 4020;
        } else if (game.teleport_to_x == 17 && game.teleport_to_y == 17) {
            game.state = 4020;
        } else if (game.teleport_to_x == 18 && game.teleport_to_y == 7) {
            game.state = 4060;
        } else {
            game.state = 4010;
        }
    } else {
        game.state = 4010;
    }

    if (game.teleportscript != "") {
        game.state = 0;
        scriptx scr;
        scr.load(game.teleportscript);
        active_scripts.push_back(scr);
        game.teleportscript = "";
    } else {
        // change music based on location
        if (!map.custommode) {
            if (dwgfx.setflipmode && game.teleport_to_x == 11 &&
                game.teleport_to_y == 4) {
                music.niceplay(9);
            } else {
                music.changemusicarea(game.teleport_to_x, game.teleport_to_y);
            }
        }
        if (!game.intimetrial && !game.nodeathmode && !game.inintermission) {
            std::string gamesavedtext = "    Game Saved    ";
            if (!map.custommodeforreal && map.custommode) {
                gamesavedtext = " Game (Not) Saved ";
            }
            if (dwgfx.flipmode) {
                dwgfx.createtextbox(gamesavedtext, -1, 202, 174, 174, 174);
                dwgfx.textboxtimer(25);
            } else {
                dwgfx.createtextbox(gamesavedtext, -1, 12, 174, 174, 174);
                dwgfx.textboxtimer(25);
            }
            if (map.custommodeforreal)
                game.customsavequick(
                    ed.ListOfMetaData[game.playcustomlevel].filename, map, obj,
                    music, dwgfx);
            else if (!map.custommode)
                game.savetele(map, obj, music);
        }
    }
}

void scriptclass::hardreset(KeyPoll& key, Graphics& dwgfx, Game& game,
                            mapclass& map, entityclass& obj, UtilityClass& help,
                            musicclass& music) {
    // Game:
    game.hascontrol = true;
    game.gravitycontrol = 0;
    game.teleport = false;
    game.companion = 0;
    game.roomchange = false;
    game.roomx = 0;
    game.roomy = 0;
    game.prevroomx = 0;
    game.prevroomy = 0;
    game.teleport_to_new_area = false;
    game.teleport_to_x = 0;
    game.teleport_to_y = 0;
    game.teleportscript = "";

    game.tapleft = 0;
    game.tapright = 0;
    game.startscript = false;
    game.newscript = "";
    game.alarmon = false;
    game.alarmdelay = 0;
    game.blackout = false;
    game.useteleporter = false;
    game.teleport_to_teleporter = 0;

    game.nodeathmode = false;
    game.nocutscenes = false;

    for (i = 0; i < 6; i++) {
        game.crewstats[i] = false;
    }
    game.crewstats[0] = true;
    game.lastsaved = 0;

    game.deathcounts = 0;
    game.gameoverdelay = 0;
    game.frames = 0;
    game.seconds = 0;
    game.minutes = 0;
    game.hours = 0;
    game.gamesaved = false;
    game.savetime = "00:00";
    game.savearea = "nowhere";
    game.savetrinkets = 0;
    game.saverx = 0;
    game.savery = 0;

    game.intimetrial = false;
    game.timetrialcountdown = 0;
    game.timetrialshinytarget = 0;
    game.timetrialparlost = false;
    game.timetrialpar = 0;
    game.timetrialresulttime = 0;

    game.totalflips = 0;
    game.hardestroom = "Welcome Aboard";
    game.hardestroomdeaths = 0;
    game.currentroomdeaths = 0;

    game.swnmode = false;
    game.swntimer = 0;
    game.swngame = 0;  // Not playing sine wave ninja!
    game.swnstate = 0;
    game.swnstate2 = 0;
    game.swnstate3 = 0;
    game.swnstate4 = 0;
    game.swndelay = 0;
    game.swndeaths = 0;
    game.supercrewmate = false;
    game.scmhurt = false;
    game.scmprogress = 0;
    game.scmmoveme = false;
    game.swncolstate = 0;
    game.swncoldelay = 0;
    game.swnrank = 0;
    game.swnmessage = 0;
    game.creditposx = 0;
    game.creditposy = 0;
    game.creditposdelay = 0;

    game.inintermission = false;
    game.insecretlab = false;

    game.crewmates = 0;

    game.state = 0;
    game.statedelay = 0;

    game.hascontrol = true;
    game.advancetext = false;

    game.pausescript = false;

    game.noflip = false;
    game.infiniflip = false;

    game.nosuicide = false;

    game.onetimescripts.clear();

    // dwgraphicsclass
    dwgfx.backgrounddrawn = false;
    dwgfx.textboxremovefast();
    dwgfx.flipmode = false;  // This will be reset if needs be elsewhere
    dwgfx.showcutscenebars = false;
    dwgfx.cutscenebarspos = 0;
    dwgfx.screenbuffer->badSignalEffect = game.fullScreenEffect_badSignal;

    // mapclass
    map.warpx = false;
    map.warpy = false;
    map.showteleporters = false;
    map.showtargets = false;
    map.showtrinkets = false;
    map.finalmode = false;
    map.finalstretch = false;
    map.finalx = 50;
    map.finaly = 50;
    map.final_colormode = false;
    map.final_colorframe = 0;
    map.final_colorframedelay = 0;
    map.final_mapcol = 0;
    map.final_aniframe = 0;
    map.final_aniframedelay = 0;
    map.rcol = 0;
    map.resetnames();
    map.custommode = false;
    map.custommodeforreal = false;
    map.towermode = false;
    map.cameraseekframe = 0;
    map.resumedelay = 0;
    map.customshowmm = true;
    map.dimension = -1;

    for (j = 0; j < ed.maxheight; j++)
        for (i = 0; i < ed.maxwidth; i++) {
            map.roomdeaths[i + j * ed.maxwidth] = 0;
            map.explored[i + j * ed.maxwidth] = 0;
        }

    for (j = 0; j < 20; j++)
        for (i = 0; i < 20; i++) map.roomdeathsfinal[i + j * 20] = 0;

    // entityclass
    obj.nearelephant = false;
    obj.upsetmode = false;
    obj.upset = 0;

    obj.trophytext = 0;
    obj.trophytype = 0;
    obj.altstates = 0;

    for (size_t i = 0; i < obj.flags.size(); i++) {
        obj.flags[i] = false;
    }

    for (i = 0; i < 6; i++) {
        obj.customcrewmoods[i] = 1;
    }

    for (i = 0; i < 100; i++) {
        obj.collect[i] = 0;
        obj.customcollect[i] = 0;
    }

    obj.coincollect.clear();
    obj.coincollect.resize(100);
    game.nocoincounter = false;

    if (obj.getplayer() > -1) {
        obj.entities[obj.getplayer()].tile = 0;
    }

    obj.kludgeonetimescript = false;

    // Script Stuff
    variables.clear();
    callbacks.clear();

    scriptrender.clear();
    game.script_images.clear();
    game.script_image_names.clear();

    active_scripts.clear();

    // WARNING: Don't reset teleporter locations, at this point we've already
    // loaded the level!
}

void scriptclass::callback(std::string name) {
    if (callbacks.find(name) == callbacks.end() || callbacks[name].empty())
        return;

    scriptx script;
    script.load("custom_" + callbacks[name]);
    active_scripts.push_back(script);
}

bool scriptclass::running() {
    for (auto script : active_scripts) {
        if (script.running) return true;
    }
    return false;
}

bool scriptclass::passive() {
    for (auto script : active_scripts) {
        if (!script.passive) return false;
    }
    return true;
}
