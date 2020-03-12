#include "Script.h"
#include "ScriptX.h"
#include <algorithm>
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

scriptx::scriptx() {
    // Init
    commands.resize(500);
    words.resize(40);
    txt.resize(40);

    position = 0;
    scriptlength = 0;
    scriptdelay = 0;
    running = false;
    passive = false;

    i = 0;
    j = 0;
    k = 0;
    loopcount = 0;
    looppoint = 0;

    labels.clear();

    scriptname = "";

    // I really hate this file, script.by the way
}

void scriptx::call(std::string script) {
    if (script.rfind("custom_@", 0) == 0) {
        script = script.erase(7, 1);
    } else if (script[0] == '@') {
        script = script.substr(1);
    } else {
        callstack.push_back(stackframe{.script = scriptname, .line = position});
    }
    load(script);
}

void scriptx::tokenize(std::string t) {
    j = 0;
    tempword = "";
    words.clear();

    std::string varname = "";
    std::string op = "";
    std::string rest = "";
    bool readop = false;
    bool readingrest = false;
    bool parseops = true;

    for (size_t i = 0; i < t.length(); i++) {
        currentletter = t.substr(i, 1);
        if ((currentletter == "(") || (currentletter == ",") ||
            (currentletter == ")")) {
            parseops = false;
            break;
        } else if (((currentletter == "=") || (currentletter == "+") ||
                    (currentletter == "-")) &&
                   !readop) {
            op += currentletter;
            readingrest = true;
        } else if (currentletter != " ") {
            if (readingrest) {
                rest += currentletter;
                readop = true;
            } else {
                varname += currentletter;
            }
        }
    }
    if (parseops) {
        if (op == "+=") t = "addvar(" + varname + "," + rest + ")";
        if (op == "++") t = "addvar(" + varname + ",1)";
        if (op == "-=") t = "addvar(" + varname + ",-" + rest + ")";
        if (op == "--") t = "addvar(" + varname + ",-1)";
        if (op == "=") t = "setvar(" + varname + "," + rest + ")";
    }

    tempword = "";

    t = script.processvars(t);

    for (size_t i = 0; i < t.length(); i++) {
        currentletter = t.substr(i, 1);
        if (currentletter == "(" || currentletter == ")" ||
            currentletter == ",") {
            words.push_back(tempword);
            std::transform(words[j].begin(), words[j].end(), words[j].begin(),
                           ::tolower);
            tempword = "";
        } else if (currentletter == " ") {
            // don't do anything - i.e. strip out spaces.
        } else {
            tempword += currentletter;
        }
    }

    words.push_back(tempword);
    words.push_back("");
}

template<class T>
T parse(std::string val) {
    return static_cast<T>(ss_toi(val));
}

template<>
bool parse<bool>(std::string val) {
    return parsebool(val);
}

template<class T>
std::string stringify(T val) {
    return std::to_string(val);
}

template<>
std::string stringify<bool>(bool val) {
    return val ? "true" : "false";
}

void quit() {
    if(map.custommodeforreal) {
        graphics.flipmode = false;
        game.gamestate = TITLEMODE;
        FILESYSTEM_unmountassets(graphics);
        graphics.fademode = 4;
        music.niceplay(6);
        graphics.backgrounddrawn = true;
        map.tdrawback = true;
        game.createmenu("levellist");
        game.state = 0;
    } else {
        game.gamestate = EDITORMODE;
        graphics.backgrounddrawn=false;
        if(!game.muted && ed.levmusic>0) music.fadeMusicVolumeIn(3000);
        if(ed.levmusic>0) music.fadeout();
    }
}

void scriptx::run(KeyPoll& key, Graphics& dwgfx, Game& game, mapclass& map,
                      entityclass& obj, UtilityClass& help, musicclass& music) {
    try {
        if (scriptdelay == 0) {
            passive = false;
        }
        while (running && scriptdelay <= 0 && !game.pausescript) {
            if (position < scriptlength) {
                // Let's split or command in an array of words

                script.updatevars();

                tokenize(commands[position]);

                // For script assisted input
                game.press_left = false;
                game.press_right = false;
                game.press_action = false;
                game.press_map = false;

                // Ok, now we run a command based on that string
                if (words[0] == "moveplayer") {
                    // USAGE: moveplayer(x offset, y offset)
                    int player = obj.getplayer();
                    obj.entities[player].xp += ss_toi(words[1]);
                    obj.entities[player].yp += ss_toi(words[2]);
                    scriptdelay = 1;
                }
#if !defined(NO_CUSTOM_LEVELS)
                if (words[0] == "warpdir") {
                    int temprx = ss_toi(words[1]) - 1;
                    int tempry = ss_toi(words[2]) - 1;
                    int curlevel = temprx + (ed.maxwidth * (tempry));
                    ed.level[curlevel].warpdir = ss_toi(words[3]);
                    // If screen warping, then override all that:
                    dwgfx.backgrounddrawn = false;

                    // Do we update our own room?
                    if (game.roomx - 100 == temprx && game.roomy - 100 == tempry) {
                        map.warpx = false;
                        map.warpy = false;
                        if (ed.level[curlevel].warpdir == 0) {
                            map.background = 1;
                            // Be careful, we could be in a Lab or Warp Zone or
                            // Tower room...
                            if (ed.level[curlevel].tileset == 2) {
                                // Lab
                                map.background = 2;
                                dwgfx.rcol = ed.level[curlevel].tilecol;
                            } else if (ed.level[curlevel].tileset == 3) {
                                // Warp Zone
                                map.background = 6;
                            } else if (ed.level[curlevel].tileset == 5) {
                                // Tower
                                map.background = 10;
                            }
                        } else if (ed.level[curlevel].warpdir == 1) {
                            map.warpx = true;
                            map.background = 3;
                            dwgfx.rcol = ed.getwarpbackground(temprx, tempry);
                        } else if (ed.level[curlevel].warpdir == 2) {
                            map.warpy = true;
                            map.background = 4;
                            dwgfx.rcol = ed.getwarpbackground(temprx, tempry);
                        } else if (ed.level[curlevel].warpdir == 3) {
                            map.warpx = true;
                            map.warpy = true;
                            map.background = 5;
                            dwgfx.rcol = ed.getwarpbackground(temprx, tempry);
                        }
                    }
                }
                if (words[0] == "ifwarp") {
                    if (ed.level[ss_toi(words[1]) - 1 +
                                (ed.maxwidth * (ss_toi(words[2]) - 1))]
                            .warpdir == ss_toi(words[3])) {
                        call("custom_" + words[4]);
                        position--;
                    }
                }
#endif
                if (words[0] == "destroy") {
                    if (words[1] == "gravitylines") {
                        for (int edi = 0; edi < obj.nentity; edi++) {
                            if (obj.entities[edi].type == 9)
                                obj.entities[edi].active = false;
                            if (obj.entities[edi].type == 10)
                                obj.entities[edi].active = false;
                        }
                    } else if (words[1] == "warptokens") {
                        for (int edi = 0; edi < obj.nentity; edi++) {
                            if (obj.entities[edi].type == 11)
                                obj.entities[edi].active = false;
                        }
                    } else if (words[1] == "platforms" ||
                            words[1] == "platformsreal") {
                        // destroy(platforms) is buggy, doesn't remove platforms'
                        // blocks
                        for (int edi = 0; edi < obj.nentity; edi++)
                            if (obj.entities[edi].rule == 2 &&
                                obj.entities[edi].animate == 100) {
                                obj.entities[edi].active = false;
                                // but destroy(platformsreal) is less buggy
                                if (words[1] == "platformsreal")
                                    obj.removeblockat(obj.entities[edi].xp,
                                                    obj.entities[edi].yp);
                            }

                        if (words[1] == "platformsreal") {
                            obj.horplatforms = false;
                            obj.vertplatforms = false;
                        }
                    } else if (words[1] == "enemies") {
                        for (int eni = 0; eni < obj.nentity; eni++)
                            if (obj.entities[eni].rule == 1)
                                obj.entities[eni].active = false;
                    } else if (words[1] == "trinkets") {
                        for (int eti = 0; eti < obj.nentity; eti++)
                            if (obj.entities[eti].type == 7)
                                obj.entities[eti].active = false;
                    } else if (words[1] == "warplines") {
                        for (int ewi = 0; ewi < obj.nentity; ewi++)
                            if (obj.entities[ewi].type >= 51 &&
                                obj.entities[ewi].type <= 54)
                                obj.entities[ewi].active = false;

                        obj.customwarpmode = false;
                        obj.customwarpmodevon = false;
                        obj.customwarpmodehon = false;

                        // If we had a warp background before, warp lines undid it
                        switch (ed.level[game.roomx - 100 +
                                        ed.maxwidth * (game.roomy - 100)]
                                    .warpdir) {
                            case 1:
                                map.warpx = true;
                                break;
                            case 2:
                                map.warpy = true;
                                break;
                            case 3:
                                map.warpx = true;
                                map.warpy = true;
                                break;
                        }
                    } else if (words[1] == "checkpoints") {
                        for (int eci = 0; eci < obj.nentity; eci++)
                            if (obj.entities[eci].type == 8)
                                obj.entities[eci].active = false;
                    } else if (words[1] == "all" || words[1] == "everything") {
                        // Don't want to use obj.removeallblocks(), it'll remove all
                        // spikes and one-ways too
                        for (int bl = 0; bl < obj.nblocks; bl++)
                            if (obj.blocks[bl].type != DAMAGE &&
                                obj.blocks[bl].type != DIRECTIONAL)
                                obj.removeblock(bl);

                        // Too bad there's no obj.removeallentities()
                        // (Wouldn't want to use it anyway, we need to take care of
                        // the conveyors' tile 1s)
                        for (int ei = 0; ei < obj.nentity; ei++) {
                            if (obj.entities[ei].rule ==
                                0)  // Destroy everything except the player
                                continue;

                            obj.entities[ei].active = false;

                            // Actually hold up, maybe this is an edentity conveyor,
                            // we want to remove all the tile 1s under it before
                            // deactivating it Of course this could be a
                            // createentity conveyor and someone placed tile 1s
                            // under it manually, script.but I don't care Also I don't care
                            // if there's not actually any tile 1s under it
                            if (!obj.entities[ei].active ||
                                obj.entities[ei].type != 1 ||
                                (obj.entities[ei].behave != 8 &&
                                obj.entities[ei].behave != 9))
                                continue;

                            // Ok, we've found a conveyor, is it aligned with the
                            // grid?
                            if (obj.entities[ei].xp % 8 != 0 ||
                                obj.entities[ei].yp % 8 != 0)
                                continue;

                            // Is its top-left corner outside the map?
                            if (obj.entities[ei].xp < 0 ||
                                obj.entities[ei].xp >= 320 ||
                                obj.entities[ei].yp < 0 ||
                                obj.entities[ei].yp >= 240)
                                continue;

                            // Very well then, we might have an edentity conveyor...

                            int thisxp = obj.entities[ei].xp / 8;
                            int thisyp = obj.entities[ei].yp / 8;

                            int usethislength;
                            // Map.cpp uses this exact check to place 8 tiles down
                            // instead of 4, hope this conveyor's width didn't
                            // change in the meantime
                            if (obj.entities[ei].w == 64)
                                usethislength = 8;
                            else
                                usethislength = 4;

                            // Ok, finally fix the tiles
                            // I don't care enough to check for what was actually
                            // behind the tiles originally
                            for (int tilex = thisxp; tilex < thisxp + usethislength;
                                tilex++)
                                map.settile(tilex, thisyp, 0);

                            // And of course, we have to force the game to redraw
                            // the room
                            dwgfx.foregrounddrawn = false;
                        }

                        // Copy-pasted from above
                        obj.horplatforms = false;
                        obj.vertplatforms = false;
                        obj.customwarpmode = false;
                        obj.customwarpmodevon = false;
                        obj.customwarpmodehon = false;
                        // If we had a warp background before, warp lines undid it
                        switch (ed.level[game.roomx - 100 +
                                        ed.maxwidth * (game.roomy - 100)]
                                    .warpdir) {
                            case 1:
                                map.warpx = true;
                                break;
                            case 2:
                                map.warpy = true;
                                break;
                            case 3:
                                map.warpx = true;
                                map.warpy = true;
                                break;
                        }

                        // Don't forget about roomtext
                        map.roomtexton = false;
                        map.roomtext.clear();
                    } else if (words[1] == "conveyors") {
                        // Copy-pasted from above
                        for (int edc = 0; edc < obj.nentity; edc++) {
                            if (!obj.entities[edc].active ||
                                obj.entities[edc].type != 1 ||
                                (obj.entities[edc].behave != 8 &&
                                obj.entities[edc].behave != 9))
                                continue;

                            for (int ii = 0; ii < obj.nblocks; ii++)
                                if (obj.blocks[ii].xp == obj.entities[edc].xp &&
                                    obj.blocks[ii].yp == obj.entities[edc].yp)
                                    obj.blocks[ii].clear();
                            obj.entities[edc].active = false;

                            // Important: set width and height to 0, or there will
                            // still be collision
                            obj.entities[edc].w = 0;
                            obj.entities[edc].h = 0;

                            // Actually hold up, maybe this is an edentity conveyor,
                            // we want to remove all the tile 1s under it before
                            // deactivating it Of course this could be a
                            // createentity conveyor and someone placed tile 1s
                            // under it manually, script.but I don't care Also I don't care
                            // if there's not actually any tile 1s under it, even if
                            // it's a spike/one-way that's now invisible and can be
                            // touched by the player

                            // Ok, is it aligned with the grid?
                            if (obj.entities[edc].xp % 8 != 0 ||
                                obj.entities[edc].yp % 8 != 0)
                                continue;

                            // Is its top-left corner outside the map?
                            if (obj.entities[edc].xp < 0 ||
                                obj.entities[edc].xp >= 320 ||
                                obj.entities[edc].yp < 0 ||
                                obj.entities[edc].yp >= 240)
                                continue;

                            // Very well then, we might have an edentity conveyor...

                            int thisxp = obj.entities[edc].xp / 8;
                            int thisyp = obj.entities[edc].yp / 8;

                            int usethislength;
                            // Map.cpp uses this exact check to place 8 tiles down
                            // instead of 4, hope this conveyor's width didn't
                            // change in the meantime
                            if (obj.entities[edc].w == 64)
                                usethislength = 8;
                            else
                                usethislength = 4;

                            // Ok, finally fix the tiles
                            // I don't care enough to check for what was actually
                            // behind the tiles originally
                            for (int tilex = thisxp; tilex < thisxp + usethislength;
                                tilex++)
                                map.settile(tilex, thisyp, 0);

                            // And of course, we have to force the game to redraw
                            // the room
                            dwgfx.foregrounddrawn = false;
                        }
                    } else if (words[1] == "terminals") {
                        for (int eti = 0; eti < obj.nentity; eti++)
                            if (obj.entities[eti].type == 13)
                                obj.entities[eti].active = false;

                        for (int bti = 0; bti < obj.nblocks; bti++)
                            if (obj.blocks[bti].type == ACTIVITY &&
                                (obj.blocks[bti].prompt ==
                                    "Press ENTER to activate terminal" ||
                                obj.blocks[bti].prompt ==
                                    "Press ENTER to activate terminals"))
                                obj.blocks[bti].active = false;
                    } else if (words[1] == "scriptboxes") {
                        for (int bsi = 0; bsi < obj.nblocks; bsi++)
                            if (obj.blocks[bsi].type == TRIGGER)
                                obj.removetrigger(obj.blocks[bsi].trigger);
                    } else if (words[1] == "disappearingplatforms" ||
                            words[1] == "quicksand") {
                        for (int epi = 0; epi < obj.nentity; epi++)
                            if (obj.entities[epi].type == 2) {
                                obj.entities[epi].active = false;
                                obj.removeblockat(obj.entities[epi].xp,
                                                obj.entities[epi].yp);
                            }
                    } else if (words[1] == "1x1quicksand" ||
                            words[1] == "1x1disappearingplatforms") {
                        for (int eqi = 0; eqi < obj.nentity; eqi++)
                            if (obj.entities[eqi].type == 3) {
                                obj.entities[eqi].active = false;
                                obj.removeblockat(obj.entities[eqi].xp,
                                                obj.entities[eqi].yp);
                            }
                    } else if (words[1] == "coins") {
                        for (int eci = 0; eci < obj.nentity; eci++)
                            if (obj.entities[eci].type == 6)
                                obj.entities[eci].active = false;
                    } else if (words[1] == "gravitytokens" ||
                            words[1] == "fliptokens") {
                        for (int egi = 0; egi < obj.nentity; egi++)
                            if (obj.entities[egi].type == 4)
                                obj.entities[egi].active = false;
                    } else if (words[1] == "roomtext") {
                        map.roomtexton = false;
                        map.roomtext.clear();
                    } else if (words[1] == "crewmates") {
                        for (int eci = 0; eci < obj.nentity; eci++)
                            if (obj.entities[eci].type == 12 ||
                                obj.entities[eci].type == 14)
                                obj.entities[eci].active = false;
                    } else if (words[1] == "customcrewmates") {
                        for (int eci = 0; eci < obj.nentity; eci++)
                            if (obj.entities[eci].type == 55)
                                obj.entities[eci].active = false;
                    } else if (words[1] == "teleporter" ||
                            words[1] == "teleporters") {
                        for (int eti = 0; eti < obj.nentity; eti++)
                            if (obj.entities[eti].type == 100)
                                obj.entities[eti].active = false;

                        game.activetele = false;
                    } else if (words[1] == "activityzones") {
                        for (int bai = 0; bai < obj.nblocks; bai++)
                            if (obj.blocks[bai].type == ACTIVITY)
                                obj.removeblock(bai);
                    }

                    obj.cleanup();
                    int n = obj.nblocks - 1;
                    while (n >= 0 && !obj.blocks[n].active) {
                        obj.nblocks--;
                        n--;
                    }
                }
                if (words[0] == "customiftrinkets") {
                    if (game.trinkets >= ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        position--;
                    }
                }
                if (words[0] == "customiftrinketsless") {
                    if (game.trinkets < ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        position--;
                    }
                } else if (words[0] == "customifflag") {
                    if (obj.flags[ss_toi(words[1])] == 1) {
                        call("custom_" + words[2]);
                        position--;
                    }
                }
                if (words[0] == "ifflipmode") {
                    if (dwgfx.setflipmode) {
                        call("custom_" + words[1]);
                        position--;
                    }
                }
                if (words[0] == "custommap") {
                    if (words[1] == "on") {
                        map.customshowmm = true;
                    } else if (words[1] == "off") {
                        map.customshowmm = false;
                    }
                }
                if (words[0] == "delay") {
                    // USAGE: delay(frames)
                    scriptdelay = ss_toi(words[1]);
                    loopdelay = true;
                }
                if (words[0] == "pdelay") {
                    // USAGE: pdelay(frames)
                    passive = true;
                    scriptdelay = ss_toi(words[1]);
                    loopdelay = true;
                }
                if (words[0] == "script.nointerrupt") {
                    script.nointerrupt = true;
                }
                if (words[0] == "yesinterrupt") {
                    script.nointerrupt = false;
                }
                if (words[0] == "settile") {
                    // settile(x,y,tile)
                    int x = ss_toi(words[1]);
                    int y = ss_toi(words[2]);
                    int tile = ss_toi(words[3]);
                    map.settile_special(x, y, tile);
                }
                if (words[0] == "replacetiles") {
                    // replacetiles(tile 1,tile 2)
                    int tile1 = ss_toi(words[1]);
                    int tile2 = ss_toi(words[2]);
                    std::vector<int>& contents = map.contents;
                    for (int i = 0; i < 1200; i++) {
                        int x = i % 40;
                        int y = i / 40;
                        if (contents[i] == tile1) {
                            map.settile_special(x, y, tile2);
                        }
                    }
                } else if (words[0] == "endtrial") {
                    game.state = 82;
                    game.statedelay = 0;
                } else if (words[0] == "iftrial") {
                    if (game.intimetrial) {
                        if (words[2] != "") {
                            if (game.currenttrial == ss_toi(words[1])) {
                                call("custom_" + words[2]);
                                position--;
                            }
                        } else {
                            call("custom_" + words[1]);
                            position--;
                        }
                    }
                }
                if (words[0] == "setroomname") {
                    // setroomname()
                    position++;
                    map.roomname = script.processvars(commands[position]);
                }
                if (words[0] == "ifkey") {
                    bool up = false;
                    bool down = false;
                    bool left = false;
                    bool right = false;
                    bool flip = false;
                    bool action = false;
                    if (key.isDown(KEYBOARD_LEFT) || key.isDown(KEYBOARD_a) ||
                        key.controllerWantsLeft(false))
                        left = true;
                    if (key.isDown(KEYBOARD_RIGHT) || key.isDown(KEYBOARD_d) ||
                        key.controllerWantsRight(false))
                        right = true;
                    if (key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_w) ||
                        key.controllerWantsUp())
                        up = true;
                    if (key.isDown(KEYBOARD_DOWN) || key.isDown(KEYBOARD_s) ||
                        key.controllerWantsDown())
                        down = true;
                    if (key.isDown(KEYBOARD_z) || key.isDown(KEYBOARD_SPACE) ||
                        key.isDown(KEYBOARD_v) ||
                        key.isDown(game.controllerButton_flip))
                        action = true;

                    if (action || up || down)
                        flip = true;

                    if ((words[1] == "left" && left) ||
                        (words[1] == "right" && right) ||
                        (words[1] == "up" && up) ||
                        (words[1] == "down" && down) ||
                        (words[1] == "action" && action) ||
                        (words[1] == "flip" && flip)) {
                        call("custom_" + words[2]);
                        position--;
                    } else {
                        const Uint8* state = SDL_GetKeyboardState(NULL);
                        if (words[1] == "rleft") words[1] = "left";
                        if (words[1] == "rright") words[1] = "right";
                        if (words[1] == "rup") words[1] = "up";
                        if (words[1] == "rdown") words[1] = "down";
                        SDL_Keycode key = SDL_GetKeyFromName(words[1].c_str());
                        if (state[SDL_GetScancodeFromKey(key)]) {
                            call("custom_" + words[2]);
                            position--;
                        }
                    }
                }
                if (words[0] == "setvar") {
                    // setvar(name, contents)
                    // OR
                    // setvar(name)
                    // <contents>
                    if (words[2] == "") {
                        position++;
                        script.setvar(words[1], script.processvars(commands[position]));
                    } else {
                        script.setvar(words[1], words[2]);
                    }
                }
                if (words[0] == "getvar") {
                    // like setvar, script.but interprets content as a variable name
                    if (words[2] == "") {
                        position++;
                        script.setvar(words[1], script.processvars(script.processvars(
                                            "%" + commands[position] + "%")));
                    } else {
                        script.setvar(words[1], script.processvars("%" + words[2] + "%"));
                    }
                }
                if (words[0] == "addvar") {
                    // addvar(name, add)
                    // OR
                    // addvar(name)
                    // <add>

                    std::string var = words[1];
                    std::string tempcontents;
                    if (script.variables.find(var) != script.variables.end()) {
                        if (words[2] == "") {
                            position++;
                            tempcontents = script.variables[var];
                            tempcontents += script.processvars(commands[position]);
                        } else {
                            if (is_number(script.variables[var]) && is_number(words[2])) {
                                tempcontents = std::to_string(stod(script.variables[var]) +
                                                            stod(words[2]));
                                tempcontents.erase(
                                    tempcontents.find_last_not_of('0') + 1,
                                    std::string::npos);
                                tempcontents.erase(
                                    tempcontents.find_last_not_of('.') + 1,
                                    std::string::npos);
                            } else {
                                tempcontents = script.variables[var] + words[2];
                            }
                        }
                        script.setvar(words[1], tempcontents);
                    }
                }
                if (words[0] == "delchar") {
                    std::string var = words[1];
                    if (script.variables.find(var) != script.variables.end() &&
                        is_number(words[2]) && !is_number(script.variables[var]) &&
                        script.variables[var].length() + 1 > stod(words[2])) {
                        script.variables[var].erase(script.variables[var].end() - stod(words[2]),
                                            script.variables[var].end());
                        script.setvar(words[1], script.variables[var]);
                    }
                }
                if ((words[0] == "ifvar") || (words[0] == "if")) {
                    std::string var = words[1];
                    if (script.variables.find(var) != script.variables.end()) {
                        if (words[4] ==
                            "")  // fourth argument doesn't exist: this is a string
                        {
                            position++;
                            if ((words[2] == "equal") || (words[2] == "equals") ||
                                (words[2] == "eq") || (words[2] == "=") ||
                                (words[2] == "==")) {
                                if (script.variables[var] ==
                                    script.processvars(commands[position])) {
                                    call("custom_" + words[3]);
                                    position--;
                                }
                            }
                            if ((words[2] == "notequal") || (words[2] == "noteq") ||
                                (words[2] == "neq") || (words[2] == "not") ||
                                (words[2] == "notequal") || (words[2] == "!=")) {
                                if (script.variables[var] !=
                                    script.processvars(commands[position])) {
                                    call("custom_" + words[3]);
                                    position--;
                                }
                            }

                        } else {  // fourth argument does exist, this is an integer
                            if ((words[2] == "equal") || (words[2] == "equals") ||
                                (words[2] == "eq") || (words[2] == "=") ||
                                (words[2] == "==")) {
                                if (script.variables[var] == words[3]) {
                                    call("custom_" + words[4]);
                                    position--;
                                }
                            }
                            if ((words[2] == "notequal") || (words[2] == "noteq") ||
                                (words[2] == "neq") || (words[2] == "not") ||
                                (words[2] == "notequal") || (words[2] == "!=")) {
                                if (script.variables[var] != words[3]) {
                                    call("custom_" + words[4]);
                                    position--;
                                }
                            }
                            if ((words[2] == "less") || (words[2] == "lt") ||
                                (words[2] == "<")) {
                                if (script.variables[var] < words[3]) {
                                    call("custom_" + words[4]);
                                    position--;
                                }
                            }
                            if ((words[2] == "lesseq") || (words[2] == "leq") ||
                                (words[2] == "<=")) {
                                if (script.variables[var] <= words[3]) {
                                    call("custom_" + words[4]);
                                    position--;
                                }
                            }
                            if ((words[2] == "greater") || (words[2] == "gt") ||
                                (words[2] == ">")) {
                                if (script.variables[var] > words[3]) {
                                    call("custom_" + words[4]);
                                    position--;
                                }
                            }
                            if ((words[2] == "greatereq") || (words[2] == "geq") ||
                                (words[2] == ">=")) {
                                if (script.variables[var] >= words[3]) {
                                    call("custom_" + words[4]);
                                    position--;
                                }
                            }
                        }
                    }
                }
                if (words[0] == "setcallback") {
                    script.callbacks[words[1]] = words[2];
                }
                if (words[0] == "stop") {
                    dwgfx.showcutscenebars = false;
                    call("stop");
                }
                if (words[0] == "clearon") {
                    dwgfx.noclear = false;
                }
                if (words[0] == "clearoff") {
                    dwgfx.noclear = true;
                }
                if (words[0] == "clear") {
                    auto alpha = ss_toi(words[1]);
                    if (words[1] == "") alpha = 255;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
                    auto rmask = 0xff000000;
                    auto gmask = 0x00ff0000;
                    auto bmask = 0x0000ff00;
                    auto amask = 0x000000ff;
#else
                    auto rmask = 0x000000ff;
                    auto gmask = 0x0000ff00;
                    auto bmask = 0x00ff0000;
                    auto amask = 0xff000000;
#endif

                    auto s = SDL_CreateRGBSurface(0, dwgfx.backBuffer->w,
                                                dwgfx.backBuffer->h, 32, rmask,
                                                gmask, bmask, amask);
                    SDL_FillRect(s, nullptr,
                                SDL_MapRGBA(s->format, 0, 0, 0, alpha));
                    SDL_BlitSurface(s, nullptr, dwgfx.backBuffer, nullptr);
                    SDL_FreeSurface(s);
                }
                if (words[0] == "drawentities") {
                    dwgfx.drawentities(map, obj, help);
                }
                if (words[0] == "debuggetpixel") {
                    getpixelx = ss_toi(words[1]);
                    getpixely = ss_toi(words[2]);
                }
                if (words[0] == "debugprint") {
                    position++;
                    std::cerr << script.processvars(commands[position]) << std::endl;
                }
                if (words[0] == "debugexit") {
                    auto code = ss_toi(words[1]);
                    std::exit(code);
                }
                if (words[0] == "debugsetglow") {
                    auto glow = ss_toi(words[1]);
                    help.freezeglow = true;
                    help.glow = glow;
                }
                if (words[0] == "debugseedrng") {
                    auto s1 = ss_toi(words[1]);
                    auto s2 = ss_toi(words[2]);
                    auto s3 = ss_toi(words[3]);
                    auto s4 = ss_toi(words[4]);
                    seed_xoshiro(s1, s2, s3, s4);
                }
                if (words[0] == "drawtext") {
                    // drawtext(x,y,r,g,b,centered)
                    scriptimage temp;
                    temp.type = 0;
                    temp.x = ss_toi(words[1]);
                    temp.y = ss_toi(words[2]);
                    temp.r = ss_toi(words[3]);
                    temp.g = ss_toi(words[4]);
                    temp.b = ss_toi(words[5]);
                    if (words[7] == "true")
                        words[7] = "1";
                    else if (words[7] == "false" || words[7] == "")
                        words[7] = "0";
                    if (words[7] == "2") {
                        if (words[8] != "")
                            temp.sc = ss_toi(words[8]);
                        else
                            temp.sc = 2;
                    }
                    temp.center = parsebool(words[6]);
                    temp.bord = ss_toi(words[7]);
                    position++;
                    temp.text = script.processvars(commands[position]);
                    script.scriptrender.push_back(temp);
                }
                if (words[0] == "drawrect") {
                    // drawrect(x,y,w,h,r,g,b)
                    scriptimage temp;
                    temp.type = 2;
                    temp.x = ss_toi(words[1]);
                    temp.y = ss_toi(words[2]);
                    temp.w = ss_toi(words[3]);
                    temp.h = ss_toi(words[4]);
                    temp.r = ss_toi(words[5]);
                    temp.g = ss_toi(words[6]);
                    temp.b = ss_toi(words[7]);
                    if (words[8] != "") {
                        temp.alpha = ss_toi(words[8]);
                    } else {
                        temp.alpha = 255;
                    }
                    script.scriptrender.push_back(temp);
                }
                if (words[0] == "drawpixel") {
                    // drawpixel(x,y,r,g,b)
                    scriptimage temp;
                    temp.type = 1;
                    temp.x = ss_toi(words[1]);
                    temp.y = ss_toi(words[2]);
                    temp.r = ss_toi(words[3]);
                    temp.g = ss_toi(words[4]);
                    temp.b = ss_toi(words[5]);
                    script.scriptrender.push_back(temp);
                }
                if (words[0] == "loadimage") {
                    game.script_images.push_back(LoadImage(words[1].c_str()));
                    game.script_image_names.push_back(words[1]);
                }
                if ((words[0] == "drawimage") || (words[0] == "drawimagepersist")) {
                    // drawimage(x,y,name[, centered])
                    int tempindex = script.getimage(game, words[3]);
                    if (tempindex == -1) {
                        game.script_images.push_back(LoadImage(words[3].c_str()));
                        game.script_image_names.push_back(words[3]);
                        tempindex = (int)game.script_images.size() - 1;
                    }
                    if ((tempindex <= (int)game.script_images.size()) &&
                        tempindex >= 0) {
                        scriptimage temp;
                        temp.type = 3;
                        temp.x = ss_toi(words[1]);
                        temp.y = ss_toi(words[2]);
                        temp.index = tempindex;
                        temp.center = parsebool(words[4]);
                        if (words[5] != "") {
                            temp.alpha = ss_toi(words[5]);
                            temp.background = parsebool(words[6]);
                        } else {
                            temp.alpha = 255;
                        }
                        if (words[6] == "none") {
                            temp.blend = SDL_BLENDMODE_NONE;
                        } else if (words[6] == "add") {
                            temp.blend = SDL_BLENDMODE_ADD;
                        } else if (words[6] == "mod") {
                            temp.blend = SDL_BLENDMODE_MOD;
                        } else {
                            temp.blend = SDL_BLENDMODE_BLEND;
                        }
                        if (words[0] == "drawimagepersist") temp.persistent = true;
                        if (words[0] == "drawimagepersist") script.setvar("return", std::to_string((int)script.scriptrender.size()));
                        script.scriptrender.push_back(temp);
                    }
                }
                if (words[0] == "setblendmode") {
                    SDL_BlendMode blend = SDL_BLENDMODE_BLEND;
                    if (words[1] == "none") {
                        blend = SDL_BLENDMODE_NONE;
                    } else if (words[1] == "add") {
                        blend = SDL_BLENDMODE_ADD;
                    } else if (words[1] == "mod") {
                        blend = SDL_BLENDMODE_MOD;
                    }
                    dwgfx.blendmode = blend;
                }
                if (words[0] == "removeimage") {
                    // removeimage(id), to be used with drawimagepersist
                    script.scriptrender.erase(script.scriptrender.begin() + ss_toi(words[1]));
                }
                if (words[0] == "flag") {
                    if (ss_toi(words[1]) >= 0 && ss_toi(words[1]) < 1000) {
                        if (words[2] == "on") {
                            obj.changeflag(ss_toi(words[1]), 1);
                        } else if (words[2] == "off") {
                            obj.changeflag(ss_toi(words[1]), 0);
                        }
                    }
                }
                if (words[0] == "flash") {
                    // USAGE: flash(frames)
                    game.flashlight = ss_toi(words[1]);
                }
                if (words[0] == "shake") {
                    // USAGE: shake(frames)
                    game.screenshake = ss_toi(words[1]);
                }
                if (words[0] == "analogue") {
                    if (parsebool(words[1]) && !game.noflashingmode)
                        dwgfx.screenbuffer->badSignalEffect = true;
                    else
                        dwgfx.screenbuffer->badSignalEffect =
                            game.fullScreenEffect_badSignal;
                }
                if (words[0] == "walk") {
                    // USAGE: walk(dir,frames)
                    if (words[1] == "left") {
                        game.press_left = true;
                    } else if (words[1] == "right") {
                        game.press_right = true;
                    }
                    scriptdelay = ss_toi(words[2]);
                }
                if (words[0] == "flip") {
                    game.press_action = true;
                    scriptdelay = 1;
                }
                if (words[0] == "tofloor") {
                    if (obj.entities[obj.getplayer()].onroof > 0) {
                        game.press_action = true;
                        scriptdelay = 1;
                    }
                }
                if (words[0] == "toceil") {
                    if (obj.entities[obj.getplayer()].onground > 0) {
                        game.press_action = true;
                        scriptdelay = 1;
                    }
                }
                // secret
                if (words[0] == "cute") {
                    game.cutemode = true;
                    SDL_SetWindowTitle(graphics.screenbuffer->m_window, "<3");
                }
                // im gay
                if (words[0] == "ally") {
                    game.allymode = true;
                    SDL_SetWindowTitle(graphics.screenbuffer->m_window, "<3");
                }
                // im trans
                if (words[0] == "misa") {
                    game.misamode = true;
                    SDL_SetWindowTitle(graphics.screenbuffer->m_window, "<3");
                }
                if (words[0] == "bruh") {
                    int i = obj.getplayer();
                    obj.entities[i].active = false;
                    obj.entities[i].rule = -1;
                    game.hascontrol = false;
                    music.fadeout();
                    music.playfile("pop.wav", "");
                    SDL_SetWindowTitle(graphics.screenbuffer->m_window, "");
                    dwgfx.showcutscenebars = false;
                    running = false;
                    script.nointerrupt = false;
                    killedviridian = true;
                    killtimer = 120;
                }
                if (words[0] == "markmap") {
                    game.scriptmarkers.push_back(scriptmarker{
                        .x = ss_toi(words[1]),
                        .y = ss_toi(words[2]),
                        .tile = ss_toi(words[3]),
                    });
                }
                if (words[0] == "unmarkmap") {
                    auto x = ss_toi(words[1]);
                    auto y = ss_toi(words[2]);
                    game.scriptmarkers.erase(std::remove_if(
                        game.scriptmarkers.begin(), game.scriptmarkers.end(),
                        [=](const scriptmarker& marker) {
                            return marker.x == x && marker.y == y;
                        }));
                }
                if (words[0] == "hidemarkers") {
                    game.hidemarkers = true;
                }
                if (words[0] == "showmarkers") {
                    game.hidemarkers = false;
                }
                if (words[0] == "mapimage") {
                    SDL_FreeSurface(dwgfx.images[12]);
                    dwgfx.images[12] = LoadImage(words[1].c_str());
                    dwgfx.mapimage = words[1];
                }
                if (words[0] == "automapimage") {
                    ed.generatecustomminimap(dwgfx, map);
                    dwgfx.mapimage = std::nullopt;
                }
                if (words[0] == "disablefog") {
                    map.nofog = true;
                }
                if (words[0] == "enablefog") {
                    map.nofog = false;
                }
                if (words[0] == "finalstretch") {
                    if (parsebool(words[1])) {
                        map.finalstretch = true;
                        map.final_colormode = true;
                        map.final_colorframe = 1;
                        map.colsuperstate = 1;
                    } else {
                        map.finalstretch = false;
                        map.final_colormode = false;
                        map.final_mapcol = 0;
                        map.colsuperstate = 0;
                        dwgfx.foregrounddrawn = false;
                    }
                }
                if (words[0] == "disableflip") {
                    game.noflip = true;
                }
                if (words[0] == "enableflip") {
                    game.noflip = false;
                }
                if (words[0] == "enableinfiniflip") {
                    game.infiniflip = true;
                }
                if (words[0] == "disableinfiniflip") {
                    game.infiniflip = false;
                }
                if (words[0] == "disablesuicide") {
                    game.nosuicide = true;
                }
                if (words[0] == "enablesuicide") {
                    game.nosuicide = false;
                }
                if (words[0] == "setspeed") {
                    game.playerspeed = std::stoi(words[1]);
                }
                if (words[0] == "setvelocity") {
                    game.nofriction = true;
                    obj.entities[obj.getplayer()].ax = std::stoi(words[1]);
                }
                if (words[0] == "playef") {
                    music.playef(ss_toi(words[1]), ss_toi(words[2]));
                }
                if (words[0] == "playmusicfile") {
                    music.playmusicfile(words[1].c_str());
                }
                if (words[0] == "playfile") {
                    music.playfile(words[1].c_str(), words[2]);
                }
                const char* word;
                if (strcmp((word = words[0].c_str()), "playfile")) {
                    music.playfile(++word, word, true);
                }
                if (words[0] == "stopfile") {
                    music.stopfile(words[1]);
                }
                if (words[0] == "play") {
                    auto fadeintime = 3000;
                    if (words[2] != "") {
                        fadeintime = ss_toi(words[2]);
                    }
                    music.play(ss_toi(words[1]), fadeintime);
                }
                if (words[0] == "niceplay") {
                    music.niceplay(ss_toi(words[1]));
                }
                if (words[0] == "stopmusic") {
                    music.haltdasmusik();
                }
                if (words[0] == "resumemusic") {
                    music.play(music.resumesong);
                }
                if (words[0] == "musicfadeout") {
                    music.fadeout();
                    music.dontquickfade = true;
                }
                if (words[0] == "musicfadein") {
                    music.musicfadein = 90;
                    // if(!game.muted) music.fadeMusicVolumeIn(3000);
                }
                if (words[0] == "trinketscriptmusic") {
                    music.play(4);
                }
                if (words[0] == "realign_tower") {
                    map.realign_tower();
                }
                if (words[0] == "gotoposition") {
                    // USAGE: gotoposition(x position, y position, gravity position)
                    int player = obj.getplayer();
                    relativepos(&obj.entities[player].xp, words[1]);
                    relativepos(&obj.entities[player].yp, words[2]);
                    if (words[3] != "") {
                        if (words[3] == "~" ||
                            (words[3].substr(0, 1) == "~" &&
                            ss_toi(words[3].substr(1, std::string::npos)) == 0))
                            ;  // Keep the current gravity control
                        else if (words[3].substr(0, 1) == "~")
                            // Invert the gravity control
                            game.gravitycontrol = !game.gravitycontrol;
                        else
                            game.gravitycontrol = ss_toi(words[3]);
                    } else {
                        game.gravitycontrol = 0;
                    }
                }
                if (words[0] == "gotodimension") {
                    relativepos(&map.dimension, words[1]);
                    ed.generatecustomminimap(dwgfx, map);
                }
                if (words[0] == "gotoroom") {
                    // USAGE: gotoroom(x,y) (manually add 100)
                    map.gotoroom(relativepos(game.roomx - 100, words[1]) + 100,
                                relativepos(game.roomy - 100, words[2]) + 100,
                                dwgfx, game, obj, music);
                }
                if (words[0] == "reloadroom") {
                    // USAGE: reloadroom()
                    map.gotoroom(game.roomx, game.roomy, dwgfx, game, obj, music);
                }
                if (words[0] == "reloadscriptboxes") {
                    for (int brs = 0; brs < obj.nresurrectblocks; brs++)
                        if (obj.resurrectblocks[brs].active &&
                            obj.resurrectblocks[brs].type == TRIGGER) {
                            obj.createblock(obj.resurrectblocks[brs].type,
                                            obj.resurrectblocks[brs].x,
                                            obj.resurrectblocks[brs].y,
                                            obj.resurrectblocks[brs].wp,
                                            obj.resurrectblocks[brs].hp,
                                            obj.resurrectblocks[brs].trigger);
                            obj.resurrectblocks[brs].clear();
                        }
                    obj.cleanupresurrectblocks();
                }
                if (words[0] == "reloadcustomactivityzones") {
                    // "Custom" here being defined as activity zones whose prompts
                    // are NOT terminals' prompts, e.g. "Press ENTER to activate
                    // terminal" or "Press ENTER to activate terminals"
                    for (int brz = 0; brz < obj.nresurrectblocks; brz++)
                        if (obj.resurrectblocks[brz].active &&
                            obj.resurrectblocks[brz].type == ACTIVITY &&
                            obj.resurrectblocks[brz].prompt !=
                                "Press ENTER to activate terminal" &&
                            obj.resurrectblocks[brz].prompt !=
                                "Press ENTER to activate terminals") {
                            obj.customprompt = obj.resurrectblocks[brz].prompt;
                            if (obj.resurrectblocks[brz].script.length() < 7 ||
                                obj.resurrectblocks[brz].script.substr(0, 7) !=
                                    "custom_") {
                                // It's a main game activity zone, we won't reload
                                // it
                                obj.resurrectblocks[brz].clear();
                                continue;
                            }
                            obj.customscript =
                                obj.resurrectblocks[brz].script.substr(
                                    7, std::string::npos);
                            obj.customr = obj.resurrectblocks[brz].r;
                            obj.customg = obj.resurrectblocks[brz].g;
                            obj.customb = obj.resurrectblocks[brz].b;

                            obj.createblock(obj.resurrectblocks[brz].type,
                                            obj.resurrectblocks[brz].x,
                                            obj.resurrectblocks[brz].y,
                                            obj.resurrectblocks[brz].wp,
                                            obj.resurrectblocks[brz].hp, 101);
                            obj.resurrectblocks[brz].clear();
                        }
                    obj.cleanupresurrectblocks();
                }
                if (words[0] == "reloadterminalactivityzones") {
                    // Copied and pasted from the above, with some slight tweaks
                    // "Terminal" here being defined as activity zones whose prompts
                    // are terminals' prompts, e.g. "Press ENTER to activate
                    // terminal" or "Press ENTER to activate terminals"
                    for (int brt = 0; brt < obj.nresurrectblocks; brt++)
                        if (obj.resurrectblocks[brt].active &&
                            obj.resurrectblocks[brt].type == ACTIVITY &&
                            (obj.resurrectblocks[brt].prompt ==
                                "Press ENTER to activate terminal" ||
                            obj.resurrectblocks[brt].prompt ==
                                "Press ENTER to activate terminals")) {
                            obj.customprompt = obj.resurrectblocks[brt].prompt;
                            if (obj.resurrectblocks[brt].script.length() < 7 ||
                                obj.resurrectblocks[brt].script.substr(0, 7) !=
                                    "custom_") {
                                // It's a main game activity zone, we won't reload
                                // it
                                obj.resurrectblocks[brt].clear();
                                continue;
                            }
                            obj.customscript =
                                obj.resurrectblocks[brt].script.substr(
                                    7, std::string::npos);
                            obj.customr = obj.resurrectblocks[brt].r;
                            obj.customg = obj.resurrectblocks[brt].g;
                            obj.customb = obj.resurrectblocks[brt].b;

                            obj.createblock(obj.resurrectblocks[brt].type,
                                            obj.resurrectblocks[brt].x,
                                            obj.resurrectblocks[brt].y,
                                            obj.resurrectblocks[brt].wp,
                                            obj.resurrectblocks[brt].hp, 101);
                            obj.resurrectblocks[brt].clear();
                        }
                    obj.cleanupresurrectblocks();
                }
                if (words[0] == "reloadactivityzones") {
                    // Copied and pasted from the above, with some slight tweaks
                    // (again)
                    for (int brz = 0; brz < obj.nresurrectblocks; brz++)
                        if (obj.resurrectblocks[brz].active &&
                            obj.resurrectblocks[brz].type == ACTIVITY) {
                            obj.customprompt = obj.resurrectblocks[brz].prompt;
                            if (obj.resurrectblocks[brz].script.length() < 7 ||
                                obj.resurrectblocks[brz].script.substr(0, 7) !=
                                    "custom_") {
                                // It's a main game activity zone, we won't reload
                                // it
                                obj.resurrectblocks[brz].clear();
                                continue;
                            }
                            obj.customscript =
                                obj.resurrectblocks[brz].script.substr(
                                    7, std::string::npos);
                            obj.customr = obj.resurrectblocks[brz].r;
                            obj.customg = obj.resurrectblocks[brz].g;
                            obj.customb = obj.resurrectblocks[brz].b;

                            obj.createblock(obj.resurrectblocks[brz].type,
                                            obj.resurrectblocks[brz].x,
                                            obj.resurrectblocks[brz].y,
                                            obj.resurrectblocks[brz].wp,
                                            obj.resurrectblocks[brz].hp, 101);
                            obj.resurrectblocks[brz].clear();
                        }
                    obj.cleanupresurrectblocks();
                }
                if (words[0] == "cutscene") {
                    dwgfx.showcutscenebars = true;
                }
                if (words[0] == "endcutscene") {
                    dwgfx.showcutscenebars = false;
                }
                if (words[0] == "cutscenefast") {
                    dwgfx.showcutscenebars = true;
                    dwgfx.cutscenebarspos = 360;
                }
                if (words[0] == "endcutscenefast") {
                    dwgfx.showcutscenebars = false;
                    dwgfx.cutscenebarspos = 0;
                }
                if (words[0] == "untilbars" || words[0] == "puntilbars") {
                    if (dwgfx.showcutscenebars) {
                        if (dwgfx.cutscenebarspos < 360) {
                            scriptdelay = 1;
                            if (words[0] == "puntilbars") passive = true;
                            position--;
                        }
                    } else {
                        if (dwgfx.cutscenebarspos > 0) {
                            scriptdelay = 1;
                            if (words[0] == "puntilbars") passive = true;
                            position--;
                        }
                    }
                } else if (words[0] == "text") {
                    // oh boy
                    // first word is the colour.
                    if (words[1] == "cyan") {
                        script.r = 164;
                        script.g = 164;
                        script.b = 255;
                    } else if (words[1] == "player") {
                        script.r = 164;
                        script.g = 164;
                        script.b = 255;
                    } else if (words[1] == "red") {
                        script.r = 255;
                        script.g = 60;
                        script.b = 60;
                    } else if (words[1] == "green") {
                        script.r = 144;
                        script.g = 255;
                        script.b = 144;
                    } else if (words[1] == "yellow") {
                        script.r = 255;
                        script.g = 255;
                        script.b = 134;
                    } else if (words[1] == "blue") {
                        script.r = 95;
                        script.g = 95;
                        script.b = 255;
                    } else if (words[1] == "purple") {
                        script.r = 255;
                        script.g = 134;
                        script.b = 255;
                    } else if (words[1] == "orange") {
                        script.r = 255;
                        script.g = 130;
                        script.b = 20;
                    } else if (words[1] == "gray") {
                        script.r = 174;
                        script.g = 174;
                        script.b = 174;
                    } else {
                        // use a gray
                        script.r = 174;
                        script.g = 174;
                        script.b = 174;
                    }

                    // Time to abuse the assignment-is-an-expression trick
                    int a = 1;
                    if (words[5] != "" && words[6] != "") {
                        a = 0;  // Off-by-one

                        // We have 2 extra args, so there must be 3 color args
                        // instead of 1! 3 color args for R, G, and B
                        script.r = ss_toi(words[++a]);
                        script.g = ss_toi(words[++a]);
                        script.b = ss_toi(words[++a]);
                    }

                    // next are the x,y coordinates
                    script.textx = ss_toi(words[++a]);
                    script.texty = ss_toi(words[++a]);

                    // Number of lines for the textbox!
                    if (!words[++a].empty())
                        script.txtnumlines = ss_toi(words[a]);
                    else
                        script.txtnumlines = 1;

                    for (int i = 0; i < script.txtnumlines; i++) {
                        position++;
                        txt[i] = script.processvars(commands[position]);
                    }
                } else if (words[0] == "position") {
                    // are we facing left or right? for some objects we don't care,
                    // default at 0.
                    j = 0;

                    // the first word is the object to position relative to
                    if (words[1] == "centerx") {
                        if (words[2] != "")
                            script.textcenterline = ss_toi(words[2]);
                        else
                            script.textcenterline = 0;

                        words[2] = "donothing";
                        j = -1;
                        script.textx = -500;
                    } else if (words[1] == "centery") {
                        if (words[2] != "")
                            script.textcenterline = ss_toi(words[2]);
                        else
                            script.textcenterline = 0;

                        words[2] = "donothing";
                        j = -1;
                        script.texty = -500;
                    } else if (words[1] == "center") {
                        words[2] = "donothing";
                        j = -1;
                        script.textx = -500;
                        script.texty = -500;
                    } else {
                        i = obj.getcrewman(words[1]);
                        j = obj.entities[i].dir;
                    }

                    // next is whether to position above or below
                    if (words[2] == "above") {
                        if (j == 1)  // left
                        {
                            script.textx = obj.entities[i].xp -
                                    10000;  // tells the box to be oriented
                                            // correctly later
                            script.texty = obj.entities[i].yp - 16 - (script.txtnumlines * 8);
                        } else if (j == 0)  // Right
                        {
                            script.textx = obj.entities[i].xp - 16;
                            script.texty = obj.entities[i].yp - 18 - (script.txtnumlines * 8);
                        }
                    } else {
                        if (j == 1)  // left
                        {
                            script.textx = obj.entities[i].xp -
                                    10000;  // tells the box to be oriented
                                            // correctly later
                            script.texty = obj.entities[i].yp + 26;
                        } else if (j == 0)  // Right
                        {
                            script.textx = obj.entities[i].xp - 16;
                            script.texty = obj.entities[i].yp + 26;
                        }
                    }
                } else if (words[0] == "customposition") {
                    // are we facing left or right? for some objects we don't care,
                    // default at 0.
                    j = 0;

                    // the first word is the object to position relative to
                    if (words[1] == "player") {
                        i = obj.getcustomcrewman(0);
                        j = obj.entities[i].dir;
                    } else if (words[1] == "cyan") {
                        i = obj.getcustomcrewman(0);
                        j = obj.entities[i].dir;
                    } else if (words[1] == "purple") {
                        i = obj.getcustomcrewman(1);
                        j = obj.entities[i].dir;
                    } else if (words[1] == "yellow") {
                        i = obj.getcustomcrewman(2);
                        j = obj.entities[i].dir;
                    } else if (words[1] == "red") {
                        i = obj.getcustomcrewman(3);
                        j = obj.entities[i].dir;
                    } else if (words[1] == "green") {
                        i = obj.getcustomcrewman(4);
                        j = obj.entities[i].dir;
                    } else if (words[1] == "blue") {
                        i = obj.getcustomcrewman(5);
                        j = obj.entities[i].dir;
                    } else if (words[1] == "centerx") {
                        words[2] = "donothing";
                        j = -1;
                        script.textx = -500;
                    } else if (words[1] == "centery") {
                        words[2] = "donothing";
                        j = -1;
                        script.texty = -500;
                    } else if (words[1] == "center") {
                        words[2] = "donothing";
                        j = -1;
                        script.textx = -500;
                        script.texty = -500;
                    }

                    if (i == 0 && words[1] != "player" && words[1] != "cyan") {
                        // Requested crewmate is not actually on screen
                        words[2] = "donothing";
                        j = -1;
                        script.textx = -500;
                        script.texty = -500;
                    }

                    // next is whether to position above or below
                    if (words[2] == "above") {
                        if (j == 1)  // left
                        {
                            script.textx = obj.entities[i].xp -
                                    10000;  // tells the box to be oriented
                                            // correctly later
                            script.texty = obj.entities[i].yp - 16 - (script.txtnumlines * 8);
                        } else if (j == 0)  // Right
                        {
                            script.textx = obj.entities[i].xp - 16;
                            script.texty = obj.entities[i].yp - 18 - (script.txtnumlines * 8);
                        }
                    } else {
                        if (j == 1)  // left
                        {
                            script.textx = obj.entities[i].xp -
                                    10000;  // tells the box to be oriented
                                            // correctly later
                            script.texty = obj.entities[i].yp + 26;
                        } else if (j == 0)  // Right
                        {
                            script.textx = obj.entities[i].xp - 16;
                            script.texty = obj.entities[i].yp + 26;
                        }
                    }
                } else if (words[0] == "backgroundtext") {
                    game.backgroundtext = true;
                } else if (words[0] == "flipme") {
                    if (dwgfx.flipmode)
                        script.texty += 2 * (120 - script.texty) - 8 * (script.txtnumlines + 2);
                } else if (words[0] == "speak_active") {
                    // Ok, actually display the textbox we've initilised now!
                    dwgfx.createtextbox(txt[0], script.textx, script.texty, script.r, script.g, script.b);
                    if (script.txtnumlines > 1) {
                        for (i = 1; i < script.txtnumlines; i++) {
                            dwgfx.addline(txt[i]);
                        }
                    }

                    // the textbox cannot be outside the screen. Fix if it is.
                    if (script.textx <= -1000) {
                        // position to the left of the player
                        script.textx += 10000;
                        script.textx -= dwgfx.textboxwidth();
                        script.textx += 16;
                        dwgfx.textboxmoveto(script.textx);
                    }

                    if (script.textx == -500 || script.textx == -1) {
                        if (script.textcenterline != 0)
                            dwgfx.textboxcenterx(script.textcenterline);
                        else
                            dwgfx.textboxcenterx(160);

                        // So it doesn't use the same line but Y instead of X for
                        // script.texty=-500
                        script.textcenterline = 0;
                    }

                    if (script.texty == -500) {
                        if (script.textcenterline != 0)
                            dwgfx.textboxcentery(script.textcenterline);
                        else
                            dwgfx.textboxcentery(120);

                        script.textcenterline = 0;
                    }

                    script.textcenterline = 0;

                    dwgfx.textboxadjust();
                    dwgfx.textboxactive();

                    if (!game.backgroundtext) {
                        game.advancetext = true;
                        game.hascontrol = false;
                        game.pausescript = true;
                        if (key.isDown(90) || key.isDown(32) || key.isDown(86) ||
                            key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN))
                            game.jumpheld = true;
                    }
                    game.backgroundtext = false;
                } else if (words[0] == "speak") {
                    // Exactly as above, except don't make the textbox active (so we
                    // can use multiple textboxes)
                    dwgfx.createtextbox(txt[0], script.textx, script.texty, script.r, script.g, script.b);
                    if (script.txtnumlines > 1) {
                        for (i = 1; i < script.txtnumlines; i++) {
                            dwgfx.addline(txt[i]);
                        }
                    }

                    // the textbox cannot be outside the screen. Fix if it is.
                    if (script.textx <= -1000) {
                        // position to the left of the player
                        script.textx += 10000;
                        script.textx -= dwgfx.textboxwidth();
                        script.textx += 16;
                        dwgfx.textboxmoveto(script.textx);
                    }

                    if (script.textx == -500 || script.textx == -1) {
                        if (script.textcenterline != 0)
                            dwgfx.textboxcenterx(script.textcenterline);
                        else
                            dwgfx.textboxcenterx();

                        // So it doesn't use the same line but Y instead of X for
                        // script.texty=-500
                        script.textcenterline = 0;
                    }

                    if (script.texty == -500) {
                        if (script.textcenterline != 0)
                            dwgfx.textboxcentery(script.textcenterline);
                        else
                            dwgfx.textboxcentery();

                        script.textcenterline = 0;
                    }

                    script.textcenterline = 0;

                    dwgfx.textboxadjust();
                    // dwgfx.textboxactive();

                    if (!game.backgroundtext) {
                        game.advancetext = true;
                        game.hascontrol = false;
                        game.pausescript = true;
                        if (key.isDown(90) || key.isDown(32) || key.isDown(86) ||
                            key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN))
                            game.jumpheld = true;
                    }
                    game.backgroundtext = false;
                } else if (words[0] == "speak_active_fast") {
                    // Copied and pasted from the above
                    // Ok, actually display the textbox we've initilised now!
                    dwgfx.createtextbox(txt[0], script.textx, script.texty, script.r, script.g, script.b);
                    if (script.txtnumlines > 1) {
                        for (i = 1; i < script.txtnumlines; i++) {
                            dwgfx.addline(txt[i]);
                        }
                    }

                    // the textbox cannot be outside the screen. Fix if it is.
                    if (script.textx <= -1000) {
                        // position to the left of the player
                        script.textx += 10000;
                        script.textx -= dwgfx.textboxwidth();
                        script.textx += 16;
                        dwgfx.textboxmoveto(script.textx);
                    }

                    if (script.textx == -500 || script.textx == -1) {
                        if (script.textcenterline != 0)
                            dwgfx.textboxcenterx(script.textcenterline);
                        else
                            dwgfx.textboxcenterx(160);

                        // So it doesn't use the same line but Y instead of X for
                        // script.texty=-500
                        script.textcenterline = 0;
                    }

                    if (script.texty == -500) {
                        if (script.textcenterline != 0)
                            dwgfx.textboxcentery(script.textcenterline);
                        else
                            dwgfx.textboxcentery(120);

                        script.textcenterline = 0;
                    }

                    script.textcenterline = 0;

                    dwgfx.textboxadjust();
                    dwgfx.textboxactive();
                    dwgfx.textboxcreatefast();

                    if (!game.backgroundtext) {
                        game.advancetext = true;
                        game.hascontrol = false;
                        game.pausescript = true;
                        if (key.isDown(90) || key.isDown(32) || key.isDown(86) ||
                            key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN))
                            game.jumpheld = true;
                    }
                    game.backgroundtext = false;
                } else if (words[0] == "speak_fast") {
                    // Copied and pasted from the above, again
                    // Exactly as above, except don't make the textbox active (so we
                    // can use multiple textboxes)
                    dwgfx.createtextbox(txt[0], script.textx, script.texty, script.r, script.g, script.b);
                    if (script.txtnumlines > 1) {
                        for (i = 1; i < script.txtnumlines; i++) {
                            dwgfx.addline(txt[i]);
                        }
                    }

                    // the textbox cannot be outside the screen. Fix if it is.
                    if (script.textx <= -1000) {
                        // position to the left of the player
                        script.textx += 10000;
                        script.textx -= dwgfx.textboxwidth();
                        script.textx += 16;
                        dwgfx.textboxmoveto(script.textx);
                    }

                    if (script.textx == -500 || script.textx == -1) {
                        if (script.textcenterline != 0)
                            dwgfx.textboxcenterx(script.textcenterline);
                        else
                            dwgfx.textboxcenterx();

                        // So it doesn't use the same line but Y instead of X for
                        // script.texty=-500
                        script.textcenterline = 0;
                    }

                    if (script.texty == -500) {
                        if (script.textcenterline != 0)
                            dwgfx.textboxcentery(script.textcenterline);
                        else
                            dwgfx.textboxcentery();

                        script.textcenterline = 0;
                    }

                    script.textcenterline = 0;

                    dwgfx.textboxadjust();
                    // dwgfx.textboxactive();
                    dwgfx.textboxcreatefast();

                    if (!game.backgroundtext) {
                        game.advancetext = true;
                        game.hascontrol = false;
                        game.pausescript = true;
                        if (key.isDown(90) || key.isDown(32) || key.isDown(86) ||
                            key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN))
                            game.jumpheld = true;
                    }
                    game.backgroundtext = false;
                } else if (words[0] == "endtext") {
                    dwgfx.textboxremove();
                    game.hascontrol = true;
                    game.advancetext = false;
                } else if (words[0] == "endtextfast") {
                    dwgfx.textboxremovefast();
                    game.hascontrol = true;
                    game.advancetext = false;
                } else if (words[0] == "textboxtimer") {
                    dwgfx.textboxtimer(ss_toi(words[1]));
                } else if (words[0] == "do") {
                    // right, loop from this point
                    looppoint = position;
                    loopcount = ss_toi(words[1]);
                } else if (words[0] == "inf") {
                    // right, loop from this point
                    looppoint = position;
                    loopcount = -1;
                } else if (words[0] == "pinf") {
                    // right, loop from this point
                    looppoint = position;
                    loopcount = -2;
                } else if (words[0] == "loop") {
                    // right, loop from this point
                    if (loopcount > 1) {
                        loopcount--;
                        position = looppoint;
                    } else if (loopcount < 0) {
                        position = looppoint;
                        if (loopcount == -2 && !loopdelay) {
                            passive = true;
                            scriptdelay = 1;
                        }
                    }
                    loopdelay = false;
                } else if (words[0] == "vvvvvvman") {
                    // Create the super VVVVVV combo!
                    i = obj.getplayer();
                    obj.entities[i].xp = 30;
                    obj.entities[i].yp = 46;
                    obj.entities[i].size = 13;
                    obj.entities[i].colour = 23;
                    obj.entities[i].cx = 36;       // 6;
                    obj.entities[i].cy = 12 + 80;  // 2;
                    obj.entities[i].h = 126 - 80;  // 21;
                } else if (words[0] == "undovvvvvvman") {
                    // Create the super VVVVVV combo!
                    i = obj.getplayer();
                    obj.entities[i].xp = 100;
                    obj.entities[i].size = 0;
                    obj.entities[i].colour = 0;
                    obj.entities[i].cx = 6;
                    obj.entities[i].cy = 2;
                    obj.entities[i].h = 21;
                } else if (words[0] == "createentity") {
                    auto k = obj.createentity(game, ss_toi(words[1]),
                                            ss_toi(words[2]), ss_toi(words[3]),
                                            ss_toi(words[4]), ss_toi(words[5]));
                    if (words[6] != "") {
                        if (words[7] != "") {
                            switch (ss_toi(words[7])) {
                                case 0:
                                    obj.setenemyroom(k, 4 + 100, 0 + 100);
                                    break;
                                case 1:
                                    obj.setenemyroom(k, 2 + 100, 0 + 100);
                                    break;
                                case 2:
                                    obj.setenemyroom(k, 12 + 100, 3 + 100);
                                    break;
                                case 3:
                                    obj.setenemyroom(k, 13 + 100, 12 + 100);
                                    break;
                                case 4:
                                    obj.setenemyroom(k, 16 + 100, 9 + 100);
                                    break;
                                case 5:
                                    obj.setenemyroom(k, 19 + 100, 1 + 100);
                                    break;
                                case 6:
                                    obj.setenemyroom(k, 19 + 100, 2 + 100);
                                    break;
                                case 7:
                                    obj.setenemyroom(k, 18 + 100, 3 + 100);
                                    break;
                                case 8:
                                    obj.setenemyroom(k, 16 + 100, 0 + 100);
                                    break;
                                case 9:
                                    obj.setenemyroom(k, 14 + 100, 2 + 100);
                                    break;
                                case 10:
                                    obj.setenemyroom(k, 10 + 100, 7 + 100);
                                    break;
                                case 11:
                                    obj.setenemyroom(k, 12 + 100, 5 + 100);
                                    break;  // yes man
                                case 12:
                                    obj.setenemyroom(k, 15 + 100, 3 + 100);
                                    break;  // STOP
                                case 13:
                                    obj.setenemyroom(k, 13 + 100, 3 + 100);
                                    break;  // wave duude
                                case 14:
                                    obj.setenemyroom(k, 15 + 100, 2 + 100);
                                    break;  // numbers
                                case 15:
                                    obj.setenemyroom(k, 16 + 100, 2 + 100);
                                    break;  // the dudes that walk
                                case 16:
                                    obj.setenemyroom(k, 18 + 100, 2 + 100);
                                    break;  // boob
                                case 17:
                                    obj.setenemyroom(k, 18 + 100, 0 + 100);
                                    break;  // OBEY
                                case 18:
                                    obj.setenemyroom(k, 17 + 100, 3 + 100);
                                    break;  // edge games
                                case 19:
                                    obj.setenemyroom(k, 13 + 100, 6 + 100);
                                    break;  // sent over the bottom gottem lmao
                                case 20:
                                    obj.setenemyroom(k, 16 + 100, 7 + 100);
                                    break;  // ghos
                                case 21:
                                    obj.setenemyroom(k, 17 + 100, 7 + 100);
                                    break;  // they b walkin 2.0
                                case 22:
                                    obj.setenemyroom(k, 14 + 100, 8 + 100);
                                    break;  // what the fuck is this
                                case 23:
                                    obj.setenemyroom(k, 11 + 100, 13 + 100);
                                    break;  // TRUTH
                                case 24:
                                    obj.setenemyroom(k, 14 + 100, 13 + 100);
                                    break;  // DABBING SKELETON
                                case 25:
                                    obj.setenemyroom(k, 44, 51);
                                    break;  // enemy in vertigo
                                default:
                                    obj.setenemyroom(k, 4 + 100, 0 + 100);
                                    break;
                            }
                        }
                        obj.entities[k].colour = ss_toi(words[6]);
                    }
                    script.setvar("return", std::to_string(k));
                } else if (words[0] == "fatal_left") {
                    obj.fatal_left();
                } else if (words[0] == "fatal_right") {
                    obj.fatal_right();
                } else if (words[0] == "fatal_top") {
                    obj.fatal_top();
                } else if (words[0] == "fatal_bottom") {
                    obj.fatal_bottom();
                } else if (words[0] == "supercrewmateon") {
                    game.supercrewmate = true;
                } else if (words[0] == "supercrewmateoff") {
                    game.supercrewmate = false;
                } else if (words[0] == "supercrewmateroom") {
                    game.scmprogress = game.roomx - 41;
#define ENTITYDATA \
                    X(active); \
                    X(invis); \
                    X(type); \
                    X(size); \
                    X(tile); \
                    X(rule); \
                    X(state); \
                    X(statedelay); \
                    X(behave); \
                    X(animate); \
                    X(para); \
                    X(life); \
                    X(colour); \
                    X(oldxp); \
                    X(oldyp); \
                    X(ax); \
                    X(ay); \
                    X(vx); \
                    X(vy); \
                    X(cx); \
                    X(cy); \
                    X(w); \
                    X(h); \
                    X(newxp); \
                    X(newyp); \
                    X(isplatform); \
                    X(x1); \
                    X(y1); \
                    X(x2); \
                    X(y2); \
                    X(onentity); \
                    X(harmful); \
                    X(onwall); \
                    X(onxwall); \
                    X(onywall); \
                    X(jumping); \
                    X(gravity); \
                    X(onground); \
                    X(onroof); \
                    X(jumpframe); \
                    X(framedelay); \
                    X(drawframe); \
                    X(walkingframe); \
                    X(dir); \
                    X(actionframe); \
                    X(yp); \
                    X(xp);
                } else if (words[0] == "setentitydata") {
#define X(FIELD) if (words[2] == #FIELD) obj.entities[ss_toi(words[1])].FIELD = parse<decltype(obj.entities[ss_toi(words[1])].FIELD)>(words[3]);
                    ENTITYDATA
#undef X
                } else if (words[0] == "getentitydata") {
#define X(FIELD) if (words[2] == #FIELD) script.setvar(words[3], stringify(obj.entities[ss_toi(words[1])].FIELD));
                    ENTITYDATA
#undef X
                } else if (words[0] == "createcrewman") {
                    if (words[3] == "cyan") {
                        script.r = 0;
                    } else if (words[3] == "red") {
                        script.r = 15;
                    } else if (words[3] == "green") {
                        script.r = 13;
                    } else if (words[3] == "yellow") {
                        script.r = 14;
                    } else if (words[3] == "blue") {
                        script.r = 16;
                    } else if (words[3] == "purple") {
                        script.r = 20;
                    } else if (strspn(words[3].c_str(), "-.0123456789") ==
                                words[3].size() &&
                            words[3].size() != 0) {
                        script.r = ss_toi(words[3]);
                    } else {
                        script.r = 19;
                    }

                    // convert the command to the right index
                    if (words[5] == "followplayer") words[5] = "10";
                    if (words[5] == "followpurple") words[5] = "11";
                    if (words[5] == "followyellow") words[5] = "12";
                    if (words[5] == "followred") words[5] = "13";
                    if (words[5] == "followgreen") words[5] = "14";
                    if (words[5] == "followblue") words[5] = "15";

                    if (words[5] == "followposition") words[5] = "16";
                    if (words[5] == "faceleft") {
                        words[5] = "17";
                        words[6] = "0";
                    }
                    if (words[5] == "faceright") {
                        words[5] = "17";
                        words[6] = "1";
                    }
                    if (words[5] == "faceplayer") {
                        words[5] = "18";
                        words[6] = "0";
                    }
                    if (words[5] == "panic") {
                        words[5] = "20";
                        words[6] = "0";
                    }

                    int ent = 18;
                    if (words[7] == "flip") {
                        ent = 57;
                    }

                    int id;

                    if (ss_toi(words[5]) >= 16) {
                        id = obj.createentity(
                            game, ss_toi(words[1]), ss_toi(words[2]), ent, script.r,
                            ss_toi(words[4]), ss_toi(words[5]), ss_toi(words[6]));
                    } else {
                        id = obj.createentity(game, ss_toi(words[1]),
                                            ss_toi(words[2]), ent, script.r,
                                            ss_toi(words[4]), ss_toi(words[5]));
                    }

                    if (words[7] == "flip") {
                        if (words[8] != "") obj.named_crewmen[words[8]] = id;
                    } else if (words[7] != "") {
                        obj.named_crewmen[words[7]] = id;
                    }
                } else if (words[0] == "createroomtext") {
                    map.roomtexton = true;
                    position++;
                    map.roomtext.push_back(Roomtext{
                        .x = ss_toi(words[1]) / 8,
                        .y = ss_toi(words[2]) / 8,
                        .subx = ss_toi(words[1]) % 8,
                        .suby = ss_toi(words[2]) % 8,
                        .text = commands[position],
                    });
                } else if (words[0] == "createscriptbox") {
                    // Ok, first figure out the first available script box slot
                    int lastslot = 0;
                    for (int bsi = 0; bsi < obj.nblocks; bsi++)
                        if (obj.blocks[bsi].trigger > lastslot)
                            lastslot = obj.blocks[bsi].trigger;
                    int usethisslot;
                    if (lastslot == 0)
                        usethisslot = 300;
                    else
                        usethisslot = lastslot + 1;

                    obj.createblock(TRIGGER, ss_toi(words[1]), ss_toi(words[2]),
                                    ss_toi(words[3]), ss_toi(words[4]),
                                    usethisslot);
                    game.customscript[usethisslot - 300] = words[5];
                } else if (words[0] == "customactivityzone") {
                    if (words[7] != "" && words[8] != "") {
                        // RGB color, 3 color arguments
                        position++;
                        obj.customprompt = script.processvars(commands[position]);
                        obj.customscript = words[8];
                        obj.customr = ss_toi(words[5]);
                        obj.customg = ss_toi(words[6]);
                        obj.customb = ss_toi(words[7]);

                        obj.createblock(ACTIVITY, ss_toi(words[1]),
                                        ss_toi(words[2]), ss_toi(words[3]),
                                        ss_toi(words[4]), 101);
                    } else {
                        // predefined color, 1 color argument
                        position++;
                        obj.customprompt = script.processvars(commands[position]);
                        obj.customscript = words[6];
                        obj.customcolour = words[5];

                        obj.createblock(ACTIVITY, ss_toi(words[1]),
                                        ss_toi(words[2]), ss_toi(words[3]),
                                        ss_toi(words[4]), 100);
                    }
                } else if (words[0] == "changemood") {
                    i = obj.getcrewman(words[1]);

                    if (ss_toi(words[2]) == 0) {
                        obj.entities[i].tile = 0;
                    } else {
                        obj.entities[i].tile = 144;
                    }
                } else if (words[0] == "changecustommood") {
                    i = obj.getcrewman(words[1]);

                    if (ss_toi(words[2]) == 0) {
                        obj.entities[i].tile = 0;
                    } else {
                        obj.entities[i].tile = 144;
                    }
                } else if (words[0] == "changetile") {
                    i = obj.getcrewman(words[1]);

                    obj.entities[i].tile = ss_toi(words[2]);
                } else if (words[0] == "flipgravity") {
                    // not something I'll use a lot, I think. Doesn't need to be
                    // very robust!
                    if (words[1] == "player") {
                        game.gravitycontrol = !game.gravitycontrol;
                    } else {
                        i = obj.getcrewman(words[1]);

                        if (obj.entities[i].rule == 6) {
                            obj.entities[i].rule = 7;
                            obj.entities[i].tile = 6;
                        } else if (obj.entities[i].rule == 7) {
                            obj.entities[i].rule = 6;
                            obj.entities[i].tile = 0;
                        }
                    }
                } else if (words[0] == "changegravity") {
                    // not something I'll use a lot, I think. Doesn't need to be
                    // very robust!
                    i = obj.getcrewman(words[1]);

                    obj.entities[i].tile += 12;
                } else if (words[0] == "changedir") {
                    i = obj.getcrewman(words[1]);

                    if (ss_toi(words[2]) == 0) {
                        obj.entities[i].dir = 0;
                    } else {
                        obj.entities[i].dir = 1;
                    }
                } else if (words[0] == "alarmon") {
                    game.alarmon = true;
                    game.alarmdelay = 0;
                } else if (words[0] == "alarmoff") {
                    game.alarmon = false;
                } else if (words[0] == "changeai") {
                    i = obj.getcrewman(words[1]);

                    if (words[2] == "followplayer") words[2] = "10";
                    if (words[2] == "followpurple") words[2] = "11";
                    if (words[2] == "followyellow") words[2] = "12";
                    if (words[2] == "followred") words[2] = "13";
                    if (words[2] == "followgreen") words[2] = "14";
                    if (words[2] == "followblue") words[2] = "15";

                    if (words[2] == "followposition") words[2] = "16";
                    if (words[2] == "faceleft") {
                        words[2] = "17";
                        words[3] = "0";
                    }
                    if (words[2] == "faceright") {
                        words[2] = "17";
                        words[3] = "1";
                    }

                    obj.entities[i].state = ss_toi(words[2]);
                    if (obj.entities[i].state == 16) {
                        if (words[1] == "player") {
                            // Just do a walk instead
                            int walkframes =
                                (ss_toi(words[3]) - obj.entities[i].xp) / 6;
                            scriptdelay = std::abs(walkframes);
                            if (walkframes > 0) {
                                game.press_left = false;
                                game.press_right = true;
                            } else if (walkframes < 0) {
                                game.press_left = true;
                                game.press_right = false;
                            }
                        } else {
                            obj.entities[i].para = ss_toi(words[3]);
                        }
                    } else if (obj.entities[i].state == 17) {
                        obj.entities[i].dir = ss_toi(words[3]);
                    }
                } else if (words[0] == "alarmon") {
                    game.alarmon = true;
                    game.alarmdelay = 0;
                } else if (words[0] == "alarmoff") {
                    game.alarmon = false;
                } else if (words[0] == "activateteleporter") {
                    i = obj.getteleporter();
                    obj.entities[i].tile = 6;
                    obj.entities[i].colour = 102;
                } else if (words[0] == "changecolour" ||
                        words[0] == "changecolor") {
                    i = obj.getcrewman(words[1]);

                    if (words[2] == "cyan") {
                        obj.entities[i].colour = 0;
                    } else if (words[2] == "red") {
                        obj.entities[i].colour = 15;
                    } else if (words[2] == "green") {
                        obj.entities[i].colour = 13;
                    } else if (words[2] == "yellow") {
                        obj.entities[i].colour = 14;
                    } else if (words[2] == "blue") {
                        obj.entities[i].colour = 16;
                    } else if (words[2] == "purple") {
                        obj.entities[i].colour = 20;
                    } else if (words[2] == "teleporter") {
                        obj.entities[i].colour = 102;
                    } else {
                        obj.entities[i].colour = ss_toi(words[2]);
                    }
                } else if (words[0] == "squeak") {
                    if (words[1] == "player") {
                        music.playef(11, 10);
                    } else if (words[1] == "cyan") {
                        music.playef(11, 10);
                    } else if (words[1] == "red") {
                        music.playef(16, 10);
                    } else if (words[1] == "green") {
                        music.playef(12, 10);
                    } else if (words[1] == "yellow") {
                        music.playef(14, 10);
                    } else if (words[1] == "blue") {
                        music.playef(13, 10);
                    } else if (words[1] == "purple") {
                        music.playef(15, 10);
                    } else if (words[1] == "cry") {
                        music.playef(2, 10);
                    } else if (words[1] == "terminal") {
                        music.playef(20, 10);
                    }
                } else if (words[0] == "blackout") {
                    game.blackout = true;
                } else if (words[0] == "blackon") {
                    game.blackout = false;
                } else if (words[0] == "setcheckpoint") {
                    i = obj.getplayer();
                    game.savepoint = 0;
                    game.savex = obj.entities[i].xp;
                    game.savey = obj.entities[i].yp;
                    game.savegc = game.gravitycontrol;
                    game.saverx = game.roomx;
                    game.savery = game.roomy;
                    game.savedir = obj.entities[i].dir;
                } else if (words[0] == "gotocheckpoint") {
                    i = obj.getplayer();
                    obj.entities[i].xp = game.savex;
                    obj.entities[i].yp = game.savey;
                    game.gravitycontrol = game.savegc;
                    game.roomx = game.saverx;
                    game.roomy = game.savery;
                    obj.entities[i].dir = game.savedir;
                } else if (words[0] == "gamestate") {
                    game.state = ss_toi(words[1]);
                    game.statedelay = 0;
                } else if (words[0] == "textboxactive") {
                    dwgfx.textboxactive();
                } else if (words[0] == "gamemode") {
                    if (words[1] == "teleporter") {
                        // TODO this draw the teleporter screen. This is a problem.
                        // :(
                        game.gamestate = 5;
                        dwgfx.menuoffset =
                            240;  // actually this should count the roomname
                        if (map.extrarow) dwgfx.menuoffset -= 10;
                        // dwgfx.menubuffer.copyPixels(dwgfx.screenbuffer,
                        // dwgfx.screenbuffer.rect, dwgfx.tl, null, null, false);

                        dwgfx.resumegamemode = false;

                        game.useteleporter =
                            false;  // good heavens don't actually use it
                    } else if (words[1] == "game") {
                        dwgfx.resumegamemode = true;
                    }
                } else if (words[0] == "ifexplored") {
                    if (map.explored[ss_toi(words[1]) +
                                    (ed.maxwidth * ss_toi(words[2]))] == 1) {
                        call(words[3]);
                        position--;
                    }
                } else if (words[0] == "iflast") {
                    if (game.lastsaved == ss_toi(words[1])) {
                        call(words[2]);
                        position--;
                    }
                } else if (words[0] == "ifskip") {
                    if (game.nocutscenes) {
                        call(words[1]);
                        position--;
                    }
                } else if (words[0] == "ifflag") {
                    if (obj.flags[ss_toi(words[1])] == 1) {
                        call(words[2]);
                        position--;
                    }
                } else if (words[0] == "ifnotflag") {
                    if (obj.flags[ss_toi(words[1])] != 1) {
                        call("custom_" + words[2]);
                        position--;
                    }
                } else if (words[0] == "ifcrewlost") {
                    if (game.crewstats[ss_toi(words[1])] == false) {
                        call(words[2]);
                        position--;
                    }
                } else if (words[0] == "iftrinkets") {
                    if (game.trinkets >= ss_toi(words[1])) {
                        call(words[2]);
                        position--;
                    }
                } else if (words[0] == "ifcrewmates") {
                    if (game.crewmates >= ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        position--;
                    }
                } else if (words[0] == "ifcoins") {
                    if (game.coins >= ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        position--;
                    }
                } else if (words[0] == "iftrinketsless") {
                    if (game.stat_trinkets < ss_toi(words[1])) {
                        call(words[2]);
                        position--;
                    }
                } else if (words[0] == "ifcrewmatesless") {
                    if (game.crewmates < ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        position--;
                    }
                } else if (words[0] == "ifcoinsless") {
                    if (game.coins < ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        position--;
                    }
                } else if (words[0] == "ifrand") {
                    int den = ss_toi(words[1]);
                    if (fRandom() < 1.0f / den) {
                        call("custom_" + words[2]);
                        position--;
                    }
                } else if (words[0] == "ifvce") {
                    call("custom_" + words[1]);
                    position--;
                } else if (words[0] == "ifmod") {
                    if (words[1] == "mmmmmm" && music.mmmmmm) {
                        call("custom_" + words[2]);
                        position--;
                    } else if ((words[1] == "mmmmmm_on" ||
                                words[1] == "mmmmmm_enabled") &&
                            music.mmmmmm && music.usingmmmmmm) {
                        call("custom_" + words[2]);
                        position--;
                    } else if ((words[1] == "mmmmmm_off" ||
                                words[1] == "mmmmmm_disabled") &&
                            music.mmmmmm && !music.usingmmmmmm) {
                        call("custom_" + words[2]);
                        position--;
                    } else if (words[1] == "unifont" && dwgfx.grphx.im_unifont &&
                            dwgfx.grphx.im_wideunifont) {
                        call("custom_" + words[2]);
                        position--;
                    }
                } else if (words[0] == "hidecoordinates") {
                    map.explored[ss_toi(words[1]) +
                                (ed.maxwidth * ss_toi(words[2]))] = 0;
                } else if (words[0] == "showcoordinates") {
                    map.explored[ss_toi(words[1]) +
                                (ed.maxwidth * ss_toi(words[2]))] = 1;
                } else if (words[0] == "hideship") {
                    map.hideship();
                } else if (words[0] == "showship") {
                    map.showship();
                } else if (words[0] == "showsecretlab") {
                    map.explored[16 + (ed.maxwidth * 5)] = 1;
                    map.explored[17 + (ed.maxwidth * 5)] = 1;
                    map.explored[18 + (ed.maxwidth * 5)] = 1;
                    map.explored[17 + (ed.maxwidth * 6)] = 1;
                    map.explored[18 + (ed.maxwidth * 6)] = 1;
                    map.explored[19 + (ed.maxwidth * 6)] = 1;
                    map.explored[19 + (ed.maxwidth * 7)] = 1;
                    map.explored[19 + (ed.maxwidth * 8)] = 1;
                } else if (words[0] == "hidesecretlab") {
                    map.explored[16 + (ed.maxwidth * 5)] = 0;
                    map.explored[17 + (ed.maxwidth * 5)] = 0;
                    map.explored[18 + (ed.maxwidth * 5)] = 0;
                    map.explored[17 + (ed.maxwidth * 6)] = 0;
                    map.explored[18 + (ed.maxwidth * 6)] = 0;
                    map.explored[19 + (ed.maxwidth * 6)] = 0;
                    map.explored[19 + (ed.maxwidth * 7)] = 0;
                    map.explored[19 + (ed.maxwidth * 8)] = 0;
                } else if (words[0] == "showteleporters") {
                    map.showteleporters = true;
                } else if (words[0] == "showtargets") {
                    map.showtargets = true;
                } else if (words[0] == "showtrinkets") {
                    map.showtrinkets = true;
                } else if (words[0] == "hideteleporters") {
                    map.showteleporters = false;
                } else if (words[0] == "hidetargets") {
                    map.showtargets = false;
                } else if (words[0] == "hidetrinkets") {
                    map.showtrinkets = false;
                } else if (words[0] == "hideplayer") {
                    obj.entities[obj.getplayer()].invis = true;
                } else if (words[0] == "showplayer") {
                    obj.entities[obj.getplayer()].invis = false;
                } else if (words[0] == "killplayer") {
                    if (!map.invincibility && game.deathseq <= 0)
                        game.deathseq = 30;
                } else if (words[0] == "hidecoincounter") {
                    game.nocoincounter = true;
                } else if (words[0] == "showcoincounter") {
                    game.nocoincounter = true;
                } else if (words[0] == "teleportscript") {
                    game.teleportscript = words[1];
                } else if (words[0] == "clearteleportscript") {
                    game.teleportscript = "";
                } else if (words[0] == "nocontrol") {
                    game.hascontrol = false;
                } else if (words[0] == "hascontrol") {
                    game.hascontrol = true;
                } else if (words[0] == "companion") {
                    game.companion = ss_toi(words[1]);
                } else if (words[0] == "befadein") {
                    dwgfx.fadeamount = 0;
                    dwgfx.fademode = 0;
                } else if (words[0] == "befadeout") {
                    dwgfx.fademode = 1;
                } else if (words[0] == "fadein") {
                    dwgfx.fademode = 4;
                } else if (words[0] == "fadeout") {
                    dwgfx.fademode = 2;
                } else if (words[0] == "untilfade" || words[0] == "puntilfade") {
                    if (dwgfx.fademode > 1) {
                        scriptdelay = 1;
                        if (words[0] == "puntilfade") passive = true;
                        position--;
                    }
                } else if (words[0] == "untilmusic" || words[0] == "puntilmusic") {
                    if (Mix_FadingMusic() != MIX_NO_FADING) {
                        scriptdelay = 1;
                        if (words[0] == "puntilmusic") passive = true;
                        position--;
                    }
                } else if (words[0] == "entersecretlab") {
                    game.unlock[8] = true;
                    game.insecretlab = true;
                } else if (words[0] == "leavesecretlab") {
                    game.insecretlab = false;
                } else if (words[0] == "resetgame") {
                    map.resetnames();
                    map.resetmap();
                    map.resetplayer(dwgfx, game, obj, music);
                    map.tdrawback = true;

                    obj.resetallflags();
                    i = obj.getplayer();
                    obj.entities[i].tile = 0;

                    game.trinkets = 0;
                    game.crewmates = 0;
                    for (i = 0; i < 100; i++) {
                        obj.collect[i] = 0;
                        obj.customcollect[i] = 0;
                    }

                    obj.coincollect.clear();
                    obj.coincollect.resize(100);
                    game.deathcounts = 0;
                    game.advancetext = false;
                    game.hascontrol = true;
                    game.frames = 0;
                    game.seconds = 0;
                    game.minutes = 0;
                    game.hours = 0;
                    game.gravitycontrol = 0;
                    game.teleport = false;
                    game.companion = 0;
                    game.roomchange = false;
                    game.teleport_to_new_area = false;
                    game.teleport_to_x = 0;
                    game.teleport_to_y = 0;

                    game.teleportscript = "";

                    // get out of final level mode!
                    map.finalmode = false;
                    map.final_colormode = false;
                    map.final_mapcol = 0;
                    map.final_colorframe = 0;
                    map.finalstretch = false;
                } else if (words[0] == "return") {
                    if (callstack.empty()) {
                        throw script_exception("Returned with empty callstack!", *this);
                    }
                    auto frame = callstack.back();
                    load(frame.script);
                    position = frame.line;
                    callstack.pop_back();
                } else if (words[0] == "loadscript") {
                    call(words[1]);
                    position--;
                } else if (words[0] == "load") {
                    call("custom_" + words[1]);
                    position--;
                } else if (words[0] == "rollcredits") {
                    game.gamestate = 6;
                    dwgfx.fademode = 4;
                    game.creditposition = 0;
                } else if (words[0] == "finalmode") {
                    map.finalmode = true;
                    map.finalx = ss_toi(words[1]);
                    map.finaly = ss_toi(words[2]);
                    game.roomx = map.finalx;
                    game.roomy = map.finaly;
                    map.gotoroom(game.roomx, game.roomy, dwgfx, game, obj, music);
                } else if (words[0] == "rescued") {
                    if (words[1] == "red") {
                        game.crewstats[3] = true;
                    } else if (words[1] == "green") {
                        game.crewstats[4] = true;
                    } else if (words[1] == "yellow") {
                        game.crewstats[2] = true;
                    } else if (words[1] == "blue") {
                        game.crewstats[5] = true;
                    } else if (words[1] == "purple") {
                        game.crewstats[1] = true;
                    } else if (words[1] == "player") {
                        game.crewstats[0] = true;
                    } else if (words[1] == "cyan") {
                        game.crewstats[0] = true;
                    }
                } else if (words[0] == "missing") {
                    if (words[1] == "red") {
                        game.crewstats[3] = false;
                    } else if (words[1] == "green") {
                        game.crewstats[4] = false;
                    } else if (words[1] == "yellow") {
                        game.crewstats[2] = false;
                    } else if (words[1] == "blue") {
                        game.crewstats[5] = false;
                    } else if (words[1] == "purple") {
                        game.crewstats[1] = false;
                    } else if (words[1] == "player") {
                        game.crewstats[0] = false;
                    } else if (words[1] == "cyan") {
                        game.crewstats[0] = false;
                    }
                } else if (words[0] == "face") {
                    i = obj.getcrewman(words[1]);
                    j = obj.getcrewman(words[2]);

                    if (obj.entities[j].xp > obj.entities[i].xp + 5) {
                        obj.entities[i].dir = 1;
                    } else if (obj.entities[j].xp < obj.entities[i].xp - 5) {
                        obj.entities[i].dir = 0;
                    }
                } else if (words[0] == "jukebox") {
                    for (j = 0; j < obj.nentity; j++) {
                        if (obj.entities[j].type == 13 && obj.entities[j].active) {
                            obj.entities[j].colour = 4;
                        }
                    }
                    if (ss_toi(words[1]) == 1) {
                        obj.createblock(5, 88 - 4, 80, 20, 16, 25);
                        for (j = 0; j < obj.nentity; j++) {
                            if (obj.entities[j].xp == 88 &&
                                obj.entities[j].yp == 80) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 2) {
                        obj.createblock(5, 128 - 4, 80, 20, 16, 26);
                        for (j = 0; j < obj.nentity; j++) {
                            if (obj.entities[j].xp == 128 &&
                                obj.entities[j].yp == 80) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 3) {
                        obj.createblock(5, 176 - 4, 80, 20, 16, 27);
                        for (j = 0; j < obj.nentity; j++) {
                            if (obj.entities[j].xp == 176 &&
                                obj.entities[j].yp == 80) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 4) {
                        obj.createblock(5, 216 - 4, 80, 20, 16, 28);
                        for (j = 0; j < obj.nentity; j++) {
                            if (obj.entities[j].xp == 216 &&
                                obj.entities[j].yp == 80) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 5) {
                        obj.createblock(5, 88 - 4, 128, 20, 16, 29);
                        for (j = 0; j < obj.nentity; j++) {
                            if (obj.entities[j].xp == 88 &&
                                obj.entities[j].yp == 128) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 6) {
                        obj.createblock(5, 176 - 4, 128, 20, 16, 30);
                        for (j = 0; j < obj.nentity; j++) {
                            if (obj.entities[j].xp == 176 &&
                                obj.entities[j].yp == 128) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 7) {
                        obj.createblock(5, 40 - 4, 40, 20, 16, 31);
                        for (j = 0; j < obj.nentity; j++) {
                            if (obj.entities[j].xp == 40 &&
                                obj.entities[j].yp == 40) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 8) {
                        obj.createblock(5, 216 - 4, 128, 20, 16, 32);
                        for (j = 0; j < obj.nentity; j++) {
                            if (obj.entities[j].xp == 216 &&
                                obj.entities[j].yp == 128) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 9) {
                        obj.createblock(5, 128 - 4, 128, 20, 16, 33);
                        for (j = 0; j < obj.nentity; j++) {
                            if (obj.entities[j].xp == 128 &&
                                obj.entities[j].yp == 128) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 10) {
                        obj.createblock(5, 264 - 4, 40, 20, 16, 34);
                        for (j = 0; j < obj.nentity; j++) {
                            if (obj.entities[j].xp == 264 &&
                                obj.entities[j].yp == 40) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    }
                } else if (words[0] == "createactivityzone") {
                    // WARNING: No one, and I mean NO ONE get any bright ideas to
                    // simplify this `if words[1] == "red" else if words[1] ==
                    // "green"` etc. stuff ESPECIALLY if that bright idea involves
                    // using the version of obj.getcrewman that takes in a string
                    // instead of an int YOU RISK BREAKING BACKWARDS COMPATIBILITY
                    // WITH ALREADY-EXISTING VANILLA LEVELS IF YOU DO SO!
                    if (words[1] == "red") {
                        i = 3;
                    } else if (words[1] == "green") {
                        i = 4;
                    } else if (words[1] == "yellow") {
                        i = 2;
                    } else if (words[1] == "blue") {
                        i = 5;
                    } else if (words[1] == "purple") {
                        i = 1;
                    }

                    if (i == 4) {
                        obj.createblock(5, obj.entities[obj.getcrewman(i)].xp - 32,
                                        obj.entities[obj.getcrewman(i)].yp - 20, 96,
                                        60, i);
                    } else {
                        obj.createblock(5, obj.entities[obj.getcrewman(i)].xp - 32,
                                        0, 96, 240, i);
                    }
                } else if (words[0] == "createrescuedcrew") {
                    // special for final level cutscene
                    // starting at 180, create the rescued crewmembers (ingoring
                    // violet, who's at 155)
                    i = 215;
                    if (game.crewstats[2] && game.lastsaved != 2) {
                        obj.createentity(game, i, 153, 18, 14, 0, 17, 0);
                        i += 25;
                    }
                    if (game.crewstats[3] && game.lastsaved != 3) {
                        obj.createentity(game, i, 153, 18, 15, 0, 17, 0);
                        i += 25;
                    }
                    if (game.crewstats[4] && game.lastsaved != 4) {
                        obj.createentity(game, i, 153, 18, 13, 0, 17, 0);
                        i += 25;
                    }
                    if (game.crewstats[5] && game.lastsaved != 5) {
                        obj.createentity(game, i, 153, 18, 16, 0, 17, 0);
                        i += 25;
                    }
                } else if (words[0] == "restoreplayercolour" ||
                        words[0] == "restoreplayercolor") {
                    i = obj.getplayer();
                    obj.entities[i].colour = 0;
                } else if (words[0] == "changeplayercolour" ||
                        words[0] == "changeplayercolour") {
                    i = obj.getplayer();

                    if (words[1] == "cyan") {
                        obj.entities[i].colour = 0;
                    } else if (words[1] == "red") {
                        obj.entities[i].colour = 15;
                    } else if (words[1] == "green") {
                        obj.entities[i].colour = 13;
                    } else if (words[1] == "yellow") {
                        obj.entities[i].colour = 14;
                    } else if (words[1] == "blue") {
                        obj.entities[i].colour = 16;
                    } else if (words[1] == "purple") {
                        obj.entities[i].colour = 20;
                    } else if (words[1] == "teleporter") {
                        obj.entities[i].colour = 102;
                    }
                } else if (words[0] == "altstates") {
                    obj.altstates = ss_toi(words[1]);
                } else if (words[0] == "activeteleporter") {
                    i = obj.getteleporter();
                    obj.entities[i].colour = 101;
                } else if (words[0] == "foundtrinket") {
                    // music.silencedasmusik();
                    music.haltdasmusik();
                    music.playef(3, 10);

                    game.trinkets++;
                    obj.collect[ss_toi(words[1])] = 1;

                    dwgfx.textboxremovefast();

                    dwgfx.createtextbox("        Congratulations!       ", 50, 85,
                                        174, 174, 174);
                    dwgfx.addline("");
                    dwgfx.addline("You have found a shiny trinket!");
                    dwgfx.textboxcenterx();

                    std::string usethisnum;
                    if (map.custommode) {
                        usethisnum = help.number(map.customtrinkets);
                    } else {
                        usethisnum = "Twenty";
                    }
                    dwgfx.createtextbox(" " + help.number(game.trinkets) +
                                            " out of " + usethisnum + " ",
                                        50, 135, 174, 174, 174);
                    dwgfx.textboxcenterx();

                    if (!game.backgroundtext) {
                        game.advancetext = true;
                        game.hascontrol = false;
                        game.pausescript = true;
                        if (key.isDown(90) || key.isDown(32) || key.isDown(86) ||
                            key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN))
                            game.jumpheld = true;
                    }
                    game.backgroundtext = false;
                } else if (words[0] == "foundlab") {
                    music.playef(3, 10);

                    dwgfx.textboxremovefast();

                    dwgfx.createtextbox("        Congratulations!       ", 50, 85,
                                        174, 174, 174);
                    dwgfx.addline("");
                    dwgfx.addline("You have found the secret lab!");
                    dwgfx.textboxcenterx();
                    dwgfx.textboxcentery();

                    if (!game.backgroundtext) {
                        game.advancetext = true;
                        game.hascontrol = false;
                        game.pausescript = true;
                        if (key.isDown(90) || key.isDown(32) || key.isDown(86) ||
                            key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN))
                            game.jumpheld = true;
                    }
                    game.backgroundtext = false;
                } else if (words[0] == "foundlab2") {
                    dwgfx.textboxremovefast();

                    dwgfx.createtextbox("The secret lab is separate from", 50, 85,
                                        174, 174, 174);
                    dwgfx.addline("the rest of the game. You can");
                    dwgfx.addline("now come back here at any time");
                    dwgfx.addline("by selecting the new SECRET LAB");
                    dwgfx.addline("option in the play menu.");
                    dwgfx.textboxcenterx();
                    dwgfx.textboxcentery();

                    if (!game.backgroundtext) {
                        game.advancetext = true;
                        game.hascontrol = false;
                        game.pausescript = true;
                        if (key.isDown(90) || key.isDown(32) || key.isDown(86) ||
                            key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN))
                            game.jumpheld = true;
                    }
                    game.backgroundtext = false;
                } else if (words[0] == "everybodysad") {
                    for (i = 0; i < obj.nentity; i++) {
                        if (obj.entities[i].rule == 6 ||
                            obj.entities[i].rule == 0) {
                            obj.entities[i].tile = 144;
                        }
                    }
                } else if (words[0] == "startintermission2") {
                    map.finalmode = true;  // Enable final level mode
                    map.finalx = 46;
                    map.finaly = 54;  // Current

                    game.savex = 228;
                    game.savey = 129;
                    game.saverx = 53;
                    game.savery = 49;
                    game.savegc = 0;
                    game.savedir = 0;  // Intermission level 2
                    game.savepoint = 0;
                    game.gravitycontrol = 0;

                    map.gotoroom(46, 54, dwgfx, game, obj, music);
                } else if (words[0] == "telesave") {
                    if (!game.intimetrial && !game.nodeathmode &&
                        !game.inintermission)
                        game.savetele(map, obj, music);
                } else if (words[0] == "customquicksave") {
                    if (!map.custommode || map.custommodeforreal)
                        if (!game.intimetrial && !game.nodeathmode)
                            game.customsavequick(
                                ed.ListOfMetaData[game.playcustomlevel].filename,
                                map, obj, music, dwgfx);
                } else if (words[0] == "createlastrescued") {
                    if (game.lastsaved == 2) {
                        script.r = 14;
                    } else if (game.lastsaved == 3) {
                        script.r = 15;
                    } else if (game.lastsaved == 4) {
                        script.r = 13;
                    } else if (game.lastsaved == 5) {
                        script.r = 16;
                    } else {
                        script.r = 19;
                    }

                    obj.createentity(game, 200, 153, 18, script.r, 0, 19, 30);
                    i = obj.getcrewman(game.lastsaved);
                    obj.entities[i].dir = 1;
                } else if (words[0] == "specialline") {
                    switch (ss_toi(words[1])) {
                        case 1:
                            script.txtnumlines = 1;

                            txt[0] = "I'm worried about " + game.unrescued() +
                                    ", Doctor!";
                            break;
                        case 2:
                            script.txtnumlines = 3;

                            if (game.crewrescued() < 5) {
                                txt[1] = "to helping you find the";
                                txt[2] = "rest of the crew!";
                            } else {
                                script.txtnumlines = 2;
                                txt[1] =
                                    "to helping you find " + game.unrescued() + "!";
                            }
                            break;
                    }
                } else if (words[0] == "trinketbluecontrol") {
                    if (game.trinkets == 20 && obj.flags[67] == 1) {
                        call("talkblue_trinket6");
                        position--;
                    } else if (game.trinkets >= 19 && obj.flags[67] == 0) {
                        call("talkblue_trinket5");
                        position--;
                    } else {
                        call("talkblue_trinket4");
                        position--;
                    }
                } else if (words[0] == "trinketyellowcontrol") {
                    if (game.trinkets >= 19) {
                        call("talkyellow_trinket3");
                        position--;
                    } else {
                        call("talkyellow_trinket2");
                        position--;
                    }
                } else if (words[0] == "redcontrol") {
                    if (game.insecretlab) {
                        call("talkred_14");
                        position--;
                    } else if (game.roomx != 104) {
                        if (game.roomx == 100) {
                            call("talkred_10");
                            position--;
                        } else if (game.roomx == 107) {
                            call("talkred_11");
                            position--;
                        } else if (game.roomx == 114) {
                            call("talkred_12");
                            position--;
                        }
                    } else if (obj.flags[67] == 1) {
                        // game complete
                        call("talkred_13");
                        position--;
                    } else if (obj.flags[35] == 1 && obj.flags[52] == 0) {
                        // Intermission level
                        obj.flags[52] = 1;
                        call("talkred_9");
                        position--;
                    } else if (obj.flags[51] == 0) {
                        // We're back home!
                        obj.flags[51] = 1;
                        call("talkred_5");
                        position--;
                    } else if (obj.flags[48] == 0 && game.crewstats[5]) {
                        // Victoria's back
                        obj.flags[48] = 1;
                        call("talkred_6");
                        position--;
                    } else if (obj.flags[49] == 0 && game.crewstats[4]) {
                        // Verdigris' back
                        obj.flags[49] = 1;
                        call("talkred_7");
                        position--;
                    } else if (obj.flags[50] == 0 && game.crewstats[2]) {
                        // Vitellary's back
                        obj.flags[50] = 1;
                        call("talkred_8");
                        position--;
                    } else if (obj.flags[45] == 0 && !game.crewstats[5]) {
                        obj.flags[45] = 1;
                        call("talkred_2");
                        position--;
                    } else if (obj.flags[46] == 0 && !game.crewstats[4]) {
                        obj.flags[46] = 1;
                        call("talkred_3");
                        position--;
                    } else if (obj.flags[47] == 0 && !game.crewstats[2]) {
                        obj.flags[47] = 1;
                        call("talkred_4");
                        position--;
                    } else {
                        obj.flags[45] = 0;
                        obj.flags[46] = 0;
                        obj.flags[47] = 0;
                        call("talkred_1");
                        position--;
                    }
                }
                // TODO: Non Urgent fix compiler nesting errors without adding
                // complexity
                if (words[0] == "greencontrol") {
                    if (game.insecretlab) {
                        call("talkgreen_11");
                        position--;
                    } else if (game.roomx == 103 && game.roomy == 109) {
                        call("talkgreen_8");
                        position--;
                    } else if (game.roomx == 101 && game.roomy == 109) {
                        call("talkgreen_9");
                        position--;
                    } else if (obj.flags[67] == 1) {
                        // game complete
                        call("talkgreen_10");
                        position--;
                    } else if (obj.flags[34] == 1 && obj.flags[57] == 0) {
                        // Intermission level
                        obj.flags[57] = 1;
                        call("talkgreen_7");
                        position--;
                    } else if (obj.flags[53] == 0) {
                        // Home!
                        obj.flags[53] = 1;
                        call("talkgreen_6");
                        position--;
                    } else if (obj.flags[54] == 0 && game.crewstats[2]) {
                        obj.flags[54] = 1;
                        call("talkgreen_5");
                        position--;
                    } else if (obj.flags[55] == 0 && game.crewstats[3]) {
                        obj.flags[55] = 1;
                        call("talkgreen_4");
                        position--;
                    } else if (obj.flags[56] == 0 && game.crewstats[5]) {
                        obj.flags[56] = 1;
                        call("talkgreen_3");
                        position--;
                    } else if (obj.flags[58] == 0) {
                        obj.flags[58] = 1;
                        call("talkgreen_2");
                        position--;
                    } else {
                        call("talkgreen_1");
                        position--;
                    }
                } else if (words[0] == "bluecontrol") {
                    if (game.insecretlab) {
                        call("talkblue_9");
                        position--;
                    } else if (obj.flags[67] == 1) {
                        // game complete, everything changes for victoria
                        if (obj.flags[41] == 1 && obj.flags[42] == 0) {
                            // second trinket conversation
                            obj.flags[42] = 1;
                            call("talkblue_trinket2");
                            position--;
                        } else if (obj.flags[41] == 0 && obj.flags[42] == 0) {
                            // Third trinket conversation
                            obj.flags[42] = 1;
                            call("talkblue_trinket3");
                            position--;
                        } else {
                            // Ok, we've already dealt with the trinket thing; so
                            // either you have them all, or you don't. If you do:
                            if (game.trinkets >= 20) {
                                call("startepilogue");
                                position--;
                            } else {
                                call("talkblue_8");
                                position--;
                            }
                        }
                    } else if (obj.flags[33] == 1 && obj.flags[40] == 0) {
                        // Intermission level
                        obj.flags[40] = 1;
                        call("talkblue_7");
                        position--;
                    } else if (obj.flags[36] == 0 && game.crewstats[5]) {
                        // Back on the ship!
                        obj.flags[36] = 1;
                        call("talkblue_3");
                        position--;
                    } else if (obj.flags[41] == 0 && game.crewrescued() <= 4) {
                        // First trinket conversation
                        obj.flags[41] = 1;
                        call("talkblue_trinket1");
                        position--;
                    } else if (obj.flags[41] == 1 && obj.flags[42] == 0 &&
                            game.crewrescued() == 5) {
                        // second trinket conversation
                        obj.flags[42] = 1;
                        call("talkblue_trinket2");
                        position--;
                    } else if (obj.flags[41] == 0 && obj.flags[42] == 0 &&
                            game.crewrescued() == 5) {
                        // Third trinket conversation
                        obj.flags[42] = 1;
                        call("talkblue_trinket3");
                        position--;
                    } else if (obj.flags[37] == 0 && game.crewstats[2]) {
                        obj.flags[37] = 1;
                        call("talkblue_4");
                        position--;
                    } else if (obj.flags[38] == 0 && game.crewstats[3]) {
                        obj.flags[38] = 1;
                        call("talkblue_5");
                        position--;
                    } else if (obj.flags[39] == 0 && game.crewstats[4]) {
                        obj.flags[39] = 1;
                        call("talkblue_6");
                        position--;
                    } else {
                        // if all else fails:
                        // if yellow is found
                        if (game.crewstats[2]) {
                            call("talkblue_2");
                            position--;
                        } else {
                            call("talkblue_1");
                            position--;
                        }
                    }
                } else if (words[0] == "yellowcontrol") {
                    if (game.insecretlab) {
                        call("talkyellow_12");
                        position--;
                    } else if (obj.flags[67] == 1) {
                        // game complete
                        call("talkyellow_11");
                        position--;
                    } else if (obj.flags[32] == 1 && obj.flags[31] == 0) {
                        // Intermission level
                        obj.flags[31] = 1;
                        call("talkyellow_6");
                        position--;
                    } else if (obj.flags[27] == 0 && game.crewstats[2]) {
                        // Back on the ship!
                        obj.flags[27] = 1;
                        call("talkyellow_10");
                        position--;
                    } else if (obj.flags[43] == 0 && game.crewrescued() == 5 &&
                            !game.crewstats[5]) {
                        // If by chance we've rescued everyone except Victoria by
                        // the end, Vitellary provides you with the trinket
                        // information instead.
                        obj.flags[43] = 1;
                        obj.flags[42] = 1;
                        obj.flags[41] = 1;
                        call("talkyellow_trinket1");
                        position--;
                    } else if (obj.flags[24] == 0 && game.crewstats[5]) {
                        obj.flags[24] = 1;
                        call("talkyellow_8");
                        position--;
                    } else if (obj.flags[26] == 0 && game.crewstats[4]) {
                        obj.flags[26] = 1;
                        call("talkyellow_7");
                        position--;
                    } else if (obj.flags[25] == 0 && game.crewstats[3]) {
                        obj.flags[25] = 1;
                        call("talkyellow_9");
                        position--;
                    } else if (obj.flags[28] == 0) {
                        obj.flags[28] = 1;
                        call("talkyellow_3");
                        position--;
                    } else if (obj.flags[29] == 0) {
                        obj.flags[29] = 1;
                        call("talkyellow_4");
                        position--;
                    } else if (obj.flags[30] == 0) {
                        obj.flags[30] = 1;
                        call("talkyellow_5");
                        position--;
                    } else if (obj.flags[23] == 0) {
                        obj.flags[23] = 1;
                        call("talkyellow_2");
                        position--;
                    } else {
                        call("talkyellow_1");
                        position--;
                        obj.flags[23] = 0;
                    }
                } else if (words[0] == "purplecontrol") {
                    // Controls Purple's conversion
                    // Crew rescued:
                    if (game.insecretlab) {
                        call("talkpurple_9");
                        position--;
                    } else if (obj.flags[67] == 1) {
                        // game complete
                        call("talkpurple_8");
                        position--;
                    } else if (obj.flags[17] == 0 && game.crewstats[4]) {
                        obj.flags[17] = 1;
                        call("talkpurple_6");
                        position--;
                    } else if (obj.flags[15] == 0 && game.crewstats[5]) {
                        obj.flags[15] = 1;
                        call("talkpurple_4");
                        position--;
                    } else if (obj.flags[16] == 0 && game.crewstats[3]) {
                        obj.flags[16] = 1;
                        call("talkpurple_5");
                        position--;
                    } else if (obj.flags[18] == 0 && game.crewstats[2]) {
                        obj.flags[18] = 1;
                        call("talkpurple_7");
                        position--;
                    } else if (obj.flags[19] == 1 && obj.flags[20] == 0 &&
                            obj.flags[21] == 0) {
                        // intermission one: if played one / not had first
                        // conversation / not played two [conversation one]
                        obj.flags[21] = 1;
                        call("talkpurple_intermission1");
                        position--;
                    } else if (obj.flags[20] == 1 && obj.flags[21] == 1 &&
                            obj.flags[22] == 0) {
                        // intermission two: if played two / had first conversation
                        // / not had second conversation [conversation two]
                        obj.flags[22] = 1;
                        call("talkpurple_intermission2");
                        position--;
                    } else if (obj.flags[20] == 1 && obj.flags[21] == 0 &&
                            obj.flags[22] == 0) {
                        // intermission two: if played two / not had first
                        // conversation / not had second conversation [conversation
                        // three]
                        obj.flags[22] = 1;
                        call("talkpurple_intermission3");
                        position--;
                    } else if (obj.flags[12] == 0) {
                        // Intro conversation
                        obj.flags[12] = 1;
                        call("talkpurple_intro");
                        position--;
                    } else if (obj.flags[14] == 0) {
                        // Shorter intro conversation
                        obj.flags[14] = 1;
                        call("talkpurple_3");
                        position--;
                    } else {
                        // if all else fails:
                        // if green is found
                        if (game.crewstats[4]) {
                            call("talkpurple_2");
                            position--;
                        } else {
                            call("talkpurple_1");
                            position--;
                        }
                    }
                }

                position++;
            } else {
                callstack.clear();
                running = false;
                script.nointerrupt = false;
            }
        }

        if (scriptdelay > 0) {
            scriptdelay--;
        }
    } catch (const script_exception& ex) {
        handle_exception(ex);
        quit();
    } catch (const std::exception& ex) {
        handle_exception(script_exception(ex, *this));
        quit();
    }
}
