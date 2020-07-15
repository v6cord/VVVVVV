#include "Script.h"
#include <shunting-yard.h>
#include <builtin-features.inc>
#include <cstdlib>
#include <cstring>
#include <cmath>
#include <iostream>
#include <optional>
#include <stdexcept>
#include "Graphics.h"

#include "Entity.h"
#include "FileSystemUtils.h"
#include "KeyPoll.h"
#include "Map.h"
#include "Music.h"
#include "Utilities.h"
#include "Maths.h"
#include "LuaScript.h"

extern bool headless;

scriptclass::scriptclass() {
    // Init
    words.resize(40);
    txt.resize(40);

    position = 0;
    scriptdelay = 0;
    running = false;
    passive = false;

    b = 0;
    g = 0;
    i = 0;
    j = 0;
    k = 0;
    loopcount = 0;
    looppoint = 0;
    r = 0;
    textx = 0;
    texty = 0;
    textcenterline = 0;

    labels.clear();
    variables.clear();

    scriptname = "";

    // I really hate this file, by the way
}

void scriptclass::clearcustom() { customscripts.clear(); }

void scriptclass::call(std::string script) {
    if (script.rfind("custom_@", 0) == 0) {
        script = script.erase(7, 1);
    } else if (script[0] == '@') {
        script = script.substr(1);
    } else {
        callstack.push_back(stackframe{scriptname, position});
    }
    load(script);
}

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
    for (auto variable : variables) {
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

int scriptclass::getimage(std::string n) {
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
    if (c == "true") c = "1";
    if (c == "false") c = "0";
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

void scriptclass::tokenize(std::string t) {
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

    t = processvars(t);

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

void scriptclass::quit() {
    stop();
    if(!map.custommode || map.custommodeforreal) {
        graphics.flipmode = false;
        game.gamestate = TITLEMODE;
        FILESYSTEM_unmountassets();
        graphics.fademode = 4;
        music.niceplay(6);
        graphics.backgrounddrawn = true;
        map.tdrawback = true;
        game.createmenu(Menu::levellist);
        game.state = 0;
    } else {
        game.gamestate = EDITORMODE;
        graphics.backgrounddrawn=false;
        if(!game.muted && ed.levmusic>0) music.fadeMusicVolumeIn(3000);
        if(ed.levmusic>0) music.fadeout();
    }
}

void scriptclass::stop() {
    lua_scripts.clear();
    running = false;
}

void scriptclass::renderimages(enum Layer::LayerName layer) {
    for (auto& current : scriptrender) {
        if (layer != current.layer) continue;
        switch (current.type) {
        case 0:
            switch (current.bord) {
            case 0:
                graphics.Print(current.x,current.y,current.text,current.r,current.g,current.b, current.center);
                break;
            case 1:
                graphics.bprint(current.x,current.y,current.text,current.r,current.g,current.b, current.center);
                break;
            case 2:
                graphics.bigprint(current.x,current.y,current.text,current.r,current.g,current.b, current.center, current.sc);
                break;
            }
            break;
        case 1: {
            auto pixels = (uint8_t*) graphics.backBuffer->pixels;
            auto row = pixels + graphics.backBuffer->pitch * current.y;
            auto pixel = ((uint32_t*) row) + current.x;
            *pixel = graphics.getRGB(current.r, current.g, current.b);
            break;
        }
        case 2: {
            SDL_Rect temprect;
            temprect.x = current.x;
            temprect.y = current.y;
            temprect.w = current.w;
            temprect.h = current.h;
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
            auto s = SDL_CreateRGBSurface(0, current.w, current.h, 32, rmask, gmask, bmask, amask);
            SDL_FillRect(s, nullptr, SDL_MapRGBA(s->format, current.r, current.b, current.g, current.alpha));
            SDL_BlitSurface(s, nullptr, graphics.backBuffer, &temprect);
            SDL_FreeSurface(s);
            break;
        }
        case 3:
            graphics.drawscriptimage( game, current.index, current.x, current.y, current.center, current.alpha, current.blend );
            break;
        case 4:
            graphics.drawscriptimagemasked( game, current.index, current.x, current.y, current.mask_index, current.mask_x, current.mask_y );
            break;
        }
    }
}

void scriptclass::run() {
    try {
        for (auto it = lua_scripts.begin(); it != lua_scripts.end();) {
            bool cont = it->run();
            if (lua_scripts.empty()) {
                return;
            } else if (cont) {
                ++it;
            } else {
                it = lua_scripts.erase(it);
            }
        }

        if (scriptdelay == 0) {
            passive = false;
        }
        while (running && scriptdelay <= 0 && !game.pausescript) {
            if (position < (int) commands.size()) {
                // Let's split or command in an array of words

                updatevars();

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
                    if (player > -1) {
                        obj.entities[player].xp += ss_toi(words[1]);
                        obj.entities[player].yp += ss_toi(words[2]);
                    }
                    if (!IS_VCE_LEVEL) scriptdelay = 1;
                }
                if (words[0] == "moveplayersafe") {
                    // USAGE: moveplayersafe(x offset, y offset)
                    int player = obj.getplayer();
                    if (player > -1) {
                        int px;
                        double dpy;

                        int tx = ss_toi(words[1]);
                        double ty = ss_toi(words[2]);
                        bool swapped = std::abs(tx) < std::abs(ty);
                        if (swapped) {
                            tx = ss_toi(words[2]);
                            ty = ss_toi(words[1]);
                            px = obj.entities[player].yp;
                            dpy = obj.entities[player].xp;
                        } else {
                            px = obj.entities[player].xp;
                            dpy = obj.entities[player].yp;
                        }

                        int target;
                        int offset;
                        if (tx < 0) {
                            target = -tx;
                            offset = -1;
                        } else {
                            target = tx;
                            offset = 1;
                        }

                        double slope = ty / std::abs(tx);

                        int py = dpy;

                        int cx = obj.entities[player].cx;
                        int cy = obj.entities[player].cy;
                        int w = obj.entities[player].w;
                        int h = obj.entities[player].h;

                        for (int i = 0; i < target; ++i) {
                            int npx = px + offset;
                            double ndpy = dpy + slope;
                            int npy = std::lround(ndpy);
                            if (swapped) {
                                obj.rectset(npy + cx, npx + cy, w, h);
                            } else {
                                obj.rectset(npx + cx, npy + cy, w, h);
                            }
                            if (obj.checkwall()) break;
                            px = npx;
                            dpy = ndpy;
                            py = npy;
                        }

                        if (swapped) {
                            obj.entities[player].xp = py;
                            obj.entities[player].yp = px;
                        } else {
                            obj.entities[player].xp = px;
                            obj.entities[player].yp = py;
                        }
                    }
                }
#if !defined(NO_CUSTOM_LEVELS)
                if (words[0] == "warpdir") {
                    int temprx = ss_toi(words[1]) - 1;
                    int tempry = ss_toi(words[2]) - 1;
                    int curlevel = temprx + (ed.maxwidth * (tempry));
                    bool inbounds = curlevel >= 0 && curlevel < (int) ed.level.size();
                    if (inbounds) {
                        ed.level[curlevel].warpdir = ss_toi(words[3]);
                    }

                    // Do we update our own room?
                    if (inbounds && game.roomx - 100 == temprx && game.roomy - 100 == tempry) {
                        // If screen warping, then override all that:
                        graphics.backgrounddrawn = false;
                        map.warpx = false;
                        map.warpy = false;
                        if (ed.level[curlevel].warpdir == 0) {
                            map.background = 1;
                            // Be careful, we could be in a Lab or Warp Zone or
                            // Tower room...
                            if (ed.level[curlevel].tileset == 2) {
                                // Lab
                                map.background = 2;
                                graphics.rcol = ed.level[curlevel].tilecol;
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
                            graphics.rcol = ed.getwarpbackground(temprx, tempry);
                        } else if (ed.level[curlevel].warpdir == 2) {
                            map.warpy = true;
                            map.background = 4;
                            graphics.rcol = ed.getwarpbackground(temprx, tempry);
                        } else if (ed.level[curlevel].warpdir == 3) {
                            map.warpx = true;
                            map.warpy = true;
                            map.background = 5;
                            graphics.rcol = ed.getwarpbackground(temprx, tempry);
                        }
                    }
                }
                if (words[0] == "ifwarp") {
                    int room = ss_toi(words[1]) - 1 + (ed.maxwidth * (ss_toi(words[2]) - 1));
                    if (room >= 0 && room < (int) ed.level.size() && ed.level[room].warpdir == ss_toi(words[3])) {
                        call("custom_" + words[4]);
                        position--;
                    }
                }
                if (words[0] == "destroy") {
                    if (words[1] == "gravitylines") {
                        for (size_t edi = 0; edi < obj.entities.size(); edi++) {
                            if (obj.entities[edi].type == 9)
                                removeentity_iter(edi);
                            if (obj.entities[edi].type == 10)
                                removeentity_iter(edi);
                        }
                    } else if (words[1] == "warptokens") {
                        for (size_t edi = 0; edi < obj.entities.size(); edi++) {
                            if (obj.entities[edi].type == 11)
                                removeentity_iter(edi);
                        }
                    } else if (words[1] == "platforms" ||
                            words[1] == "platformsreal") {
                        // destroy(platforms) is buggy, doesn't remove platforms'
                        // blocks
                        for (size_t edi = 0; edi < obj.entities.size(); edi++)
                            if (obj.entities[edi].rule == 2 &&
                                obj.entities[edi].animate == 100) {
                                removeentity_iter(edi);
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
                        for (size_t eni = 0; eni < obj.entities.size(); eni++)
                            if (obj.entities[eni].rule == 1)
                                removeentity_iter(eni);
                    } else if (words[1] == "trinkets") {
                        for (size_t eti = 0; eti < obj.entities.size(); eti++)
                            if (obj.entities[eti].type == 7)
                                removeentity_iter(eti);
                    } else if (words[1] == "warplines") {
                        for (size_t ewi = 0; ewi < obj.entities.size(); ewi++)
                            if (obj.entities[ewi].type >= 51 &&
                                obj.entities[ewi].type <= 54)
                                removeentity_iter(ewi);

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
                        for (size_t eci = 0; eci < obj.entities.size(); eci++)
                            if (obj.entities[eci].type == 8)
                                removeentity_iter(eci);
                    } else if (words[1] == "all" || words[1] == "everything") {
                        // Don't want to use obj.removeallblocks(), it'll remove all
                        // spikes and one-ways too
                        for (size_t bl = 0; bl < obj.blocks.size(); bl++)
                            if (obj.blocks[bl].type != DAMAGE &&
                                obj.blocks[bl].type != DIRECTIONAL)
                                removeblock_iter(bl);

                        // Too bad there's no obj.removeallentities()
                        // (Wouldn't want to use it anyway, we need to take care of
                        // the conveyors' tile 1s)
                        for (size_t ei = 0; ei < obj.entities.size(); ei++) {
                            if (obj.entities[ei].rule ==
                                0)  // Destroy everything except the player
                                continue;

                            // Store a copy of the entity to check its attributes later
                            entclass deletedentity = entclass(obj.entities[ei]);
                            removeentity_iter(ei);

                            // Actually hold up, maybe this is an edentity conveyor,
                            // we want to remove all the tile 1s under it before
                            // deactivating it
                            // Of course this could be a createentity conveyor
                            // and someone placed tile 1s under it manually,
                            // but I don't care
                            // Also I don't care if there's not actually any
                            // tile 1s under it
                            if (deletedentity.type != 1 ||
                                (deletedentity.behave != 8 &&
                                deletedentity.behave != 9))
                                continue;

                            // Ok, we've found a conveyor, is it aligned with the
                            // grid?
                            if (deletedentity.xp % 8 != 0 ||
                                deletedentity.yp % 8 != 0)
                                continue;

                            // Is its top-left corner outside the map?
                            if (deletedentity.xp < 0 ||
                                deletedentity.xp >= 320 ||
                                deletedentity.yp < 0 ||
                                deletedentity.yp >= 240)
                                continue;

                            // Very well then, we might have an edentity conveyor...

                            int thisxp = deletedentity.xp / 8;
                            int thisyp = deletedentity.yp / 8;

                            int usethislength;
                            // Map.cpp uses this exact check to place 8 tiles down
                            // instead of 4, hope this conveyor's width didn't
                            // change in the meantime
                            if (deletedentity.w == 64)
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
                            graphics.foregrounddrawn = false;
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
                        for (size_t edc = 0; edc < obj.entities.size(); edc++) {
                            if (obj.entities[edc].type != 1 ||
                                (obj.entities[edc].behave != 8 &&
                                obj.entities[edc].behave != 9))
                                continue;

                            for (size_t ii = 0; ii < obj.blocks.size(); ii++)
                                if (obj.blocks[ii].xp == obj.entities[edc].xp &&
                                    obj.blocks[ii].yp == obj.entities[edc].yp)
                                    removeblock_iter(ii);
                            // Store a copy of the entity to check its attributes later
                            entclass deletedentity = entclass(obj.entities[edc]);
                            removeentity_iter(edc);

                            // Actually hold up, maybe this is an edentity
                            // conveyor, we want to remove all the tile 1s
                            // under it before deactivating it
                            // Of course this could be a createentity conveyor
                            // and someone placed tile 1s under it manually,
                            // but I don't care
                            // Also I don't care if there's not actually any
                            // tile 1s under it, even if it's a spike/one-way
                            // that's now invisible and can be touched by the
                            // player

                            // Ok, is it aligned with the grid?
                            if (deletedentity.xp % 8 != 0 ||
                                deletedentity.yp % 8 != 0)
                                continue;

                            // Is its top-left corner outside the map?
                            if (deletedentity.xp < 0 ||
                                deletedentity.xp >= 320 ||
                                deletedentity.yp < 0 ||
                                deletedentity.yp >= 240)
                                continue;

                            // Very well then, we might have an edentity conveyor...

                            int thisxp = deletedentity.xp / 8;
                            int thisyp = deletedentity.yp / 8;

                            int usethislength;
                            // Map.cpp uses this exact check to place 8 tiles down
                            // instead of 4, hope this conveyor's width didn't
                            // change in the meantime
                            if (deletedentity.w == 64)
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
                            graphics.foregrounddrawn = false;
                        }
                    } else if (words[1] == "terminals") {
                        for (size_t eti = 0; eti < obj.entities.size(); eti++)
                            if (obj.entities[eti].type == 13)
                                removeentity_iter(eti);

                        for (size_t bti = 0; bti < obj.blocks.size(); bti++)
                            if (obj.blocks[bti].type == ACTIVITY &&
                                (obj.blocks[bti].prompt ==
                                    "Press ENTER to activate terminal" ||
                                obj.blocks[bti].prompt ==
                                    "Press ENTER to activate terminals"))
                                removeblock_iter(bti);
                    } else if (words[1] == "scriptboxes") {
                        for (size_t bsi = 0; bsi < obj.blocks.size(); bsi++)
                            if (obj.blocks[bsi].type == TRIGGER) {
                                obj.removetrigger(obj.blocks[bsi].trigger);
                                bsi--;
                            }
                    } else if (words[1] == "disappearingplatforms" ||
                            words[1] == "quicksand") {
                        for (size_t epi = 0; epi < obj.entities.size(); epi++)
                            if (obj.entities[epi].type == 2) {
                                obj.removeblockat(obj.entities[epi].xp,
                                                obj.entities[epi].yp);
                                removeentity_iter(epi);
                            }
                    } else if (words[1] == "1x1quicksand" ||
                            words[1] == "1x1disappearingplatforms") {
                        for (size_t eqi = 0; eqi < obj.entities.size(); eqi++)
                            if (obj.entities[eqi].type == 3) {
                                obj.removeblockat(obj.entities[eqi].xp,
                                                obj.entities[eqi].yp);
                                removeentity_iter(eqi);
                            }
                    } else if (words[1] == "coins") {
                        for (size_t eci = 0; eci < obj.entities.size(); eci++)
                            if (obj.entities[eci].type == 6)
                                removeentity_iter(eci);
                    } else if (words[1] == "gravitytokens" ||
                            words[1] == "fliptokens") {
                        for (size_t egi = 0; egi < obj.entities.size(); egi++)
                            if (obj.entities[egi].type == 4)
                                removeentity_iter(egi);
                    } else if (words[1] == "roomtext") {
                        map.roomtexton = false;
                        map.roomtext.clear();
                    } else if (words[1] == "crewmates") {
                        for (size_t eci = 0; eci < obj.entities.size(); eci++)
                            if (obj.entities[eci].type == 12 ||
                                obj.entities[eci].type == 14)
                                removeentity_iter(eci);
                    } else if (words[1] == "customcrewmates") {
                        for (size_t eci = 0; eci < obj.entities.size(); eci++)
                            if (obj.entities[eci].type == 55)
                                removeentity_iter(eci);
                    } else if (words[1] == "teleporter" ||
                            words[1] == "teleporters") {
                        for (size_t eti = 0; eti < obj.entities.size(); eti++)
                            if (obj.entities[eti].type == 100)
                                removeentity_iter(eti);

                        game.activetele = false;
                    } else if (words[1] == "activityzones") {
                        for (size_t bai = 0; bai < obj.blocks.size(); bai++)
                            if (obj.blocks[bai].type == ACTIVITY)
                                removeblock_iter(bai);
                    }
                }
#endif
                if (words[0] == "customiftrinkets") {
                    if (game.trinkets() >= ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        position--;
                    }
                }
                if (words[0] == "customiftrinketsless") {
                    if (game.trinkets() < ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        position--;
                    }
                } else if (words[0] == "customifflag") {
                    int flag = ss_toi(words[1]);
                    if (flag >= 0 && flag < (int) obj.flags.size() && obj.flags[flag]) {
                        call("custom_" + words[2]);
                        position--;
                    }
                }
                if (words[0] == "ifflipmode") {
                    if (graphics.setflipmode) {
                        call("custom_" + words[1]);
                        continue;
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
                if (words[0] == "setinterrupt") {
                    nointerrupt = !parsebool(words[1]);
                }
                if (words[0] == "createdamage") {
                    int x = ss_toi(words[1]);
                    int y = ss_toi(words[2]);
                    int w = ss_toi(words[3]);
                    int h = ss_toi(words[4]);
                    obj.createblock(DAMAGE, x, y, w, h);
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
                                continue;
                            }
                        } else {
                            call("custom_" + words[1]);
                            continue;
                        }
                    }
                }
                if (words[0] == "setroomname") {
                    // setroomname()
                    position++;
                    if (position < (int) commands.size())
                        map.roomname = processvars(commands[position]);
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
                        continue;
                    } else {
                        const Uint8* state = SDL_GetKeyboardState(NULL);
                        if (words[1] == "rleft") words[1] = "left";
                        if (words[1] == "rright") words[1] = "right";
                        if (words[1] == "rup") words[1] = "up";
                        if (words[1] == "rdown") words[1] = "down";
                        SDL_Keycode key = SDL_GetKeyFromName(words[1].c_str());
                        if (state[SDL_GetScancodeFromKey(key)]) {
                            call("custom_" + words[2]);
                            continue;
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
                        setvar(words[1], processvars(commands[position]));
                    } else {
                        setvar(words[1], words[2]);
                    }
                }
                if (words[0] == "getvar") {
                    // like setvar, but interprets content as a variable name
                    if (words[2] == "") {
                        position++;
                        setvar(words[1], processvars(processvars(
                                            "%" + commands[position] + "%")));
                    } else {
                        setvar(words[1], processvars("%" + words[2] + "%"));
                    }
                }
                if (words[0] == "addvar") {
                    // addvar(name, add)
                    // OR
                    // addvar(name)
                    // <add>

                    std::string var = words[1];
                    std::string tempcontents;
                    if (variables.find(var) != variables.end()) {
                        if (words[2] == "") {
                            position++;
                            tempcontents = variables[var];
                            tempcontents += processvars(commands[position]);
                        } else {
                            if (is_number(variables[var]) && is_number(words[2])) {
                                tempcontents = std::to_string(stod(variables[var]) +
                                                            stod(words[2]));
                                tempcontents.erase(
                                    tempcontents.find_last_not_of('0') + 1,
                                    std::string::npos);
                                tempcontents.erase(
                                    tempcontents.find_last_not_of('.') + 1,
                                    std::string::npos);
                            } else {
                                tempcontents = variables[var] + words[2];
                            }
                        }
                        setvar(words[1], tempcontents);
                    }
                }
                if (words[0] == "randchoose") {
                    std::vector<std::string> options;
                    for (size_t ti = 2; ti < words.size(); ti++) {
                        if (words[ti].empty())
                            break;

                        options.push_back(words[ti]);
                    }

                    int chosen = fRandom() * options.size();
                    setvar(words[1], options[chosen]);
                }
                if (words[0] == "randrange") {
                    int start, end;
                    if (words[3] == "") {
                        start = 0;
                        end = ss_toi(words[2]);
                    } else {
                        start = ss_toi(words[2]);
                        end = ss_toi(words[3]);
                    }

                    int length = end - start;
                    int offset = start;

                    int chosenzero = fRandom() * length;
                    int chosenoffset = chosenzero + offset;

                    setvar(words[1], help.String(chosenoffset));
                }
                if (words[0] == "delchar") {
                    std::string var = words[1];
                    if (variables.find(var) != variables.end() &&
                        is_number(words[2]) && !is_number(variables[var]) &&
                        variables[var].length() + 1 > stod(words[2])) {
                        variables[var].erase(variables[var].end() - stod(words[2]),
                                            variables[var].end());
                        setvar(words[1], variables[var]);
                    }
                }
                if ((words[0] == "ifvar") || (words[0] == "if")) {
                    if (evalvar(words[1]) == "1") {
                        call("custom_" + words[2]);
                        continue;
                    }
                }
                if (words[0] == "setcallback") {
                    callbacks[words[1]] = words[2];
                }
                if (words[0] == "stop") {
                    graphics.showcutscenebars = false;
                    call("stop");
                }
                if (words[0] == "clearon") {
                    graphics.noclear = false;
                }
                if (words[0] == "clearoff") {
                    graphics.noclear = true;
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

                    auto s = SDL_CreateRGBSurface(0, graphics.backBuffer->w,
                                                graphics.backBuffer->h, 32, rmask,
                                                gmask, bmask, amask);
                    SDL_FillRect(s, nullptr,
                                SDL_MapRGBA(s->format, 0, 0, 0, alpha));
                    SDL_BlitSurface(s, nullptr, graphics.backBuffer, nullptr);
                    SDL_FreeSurface(s);
                }
                if (words[0] == "debuggetpixel" && headless) {
                    getpixelx = ss_toi(words[1]);
                    getpixely = ss_toi(words[2]);
                }
                if (words[0] == "debugprint" && headless) {
                    position++;
                    std::cerr << processvars(commands[position]) << std::endl;
                }
                if (words[0] == "debugexit" && headless) {
                    auto code = ss_toi(words[1]);
                    std::exit(code);
                }
                if (words[0] == "debugsetglow" && headless) {
                    auto glow = ss_toi(words[1]);
                    help.freezeglow = true;
                    help.glow = glow;
                }
                if (words[0] == "debugseedrng" && headless) {
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
                    if (words[8] != "") {
                        if (words[7] == "1") {
                            handle_exception(script_exception("Large text can't have a border!"));
                            call("stop");
                            continue;
                        }
                        words[7] = "2";
                        temp.sc = ss_toi(words[8]);
                    } else {
                        temp.sc = 2;
                    }
                    temp.center = parsebool(words[6]);
                    temp.bord = ss_toi(words[7]);
                    position++;
                    if (position < (int) commands.size())
                        temp.text = processvars(commands[position]);
                    scriptrender.push_back(temp);
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
                    scriptrender.push_back(temp);
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
                    scriptrender.push_back(temp);
                }
                if (words[0] == "loadimage") {
                    SDL_Surface* image = LoadImage(words[1].c_str());
                    if (image != nullptr) {
                        game.script_images.push_back(image);
                        game.script_image_names.push_back(words[1]);
                    }
                }
                if (words[0] == "unloadscriptimages") {
                    game.script_images.clear();
                    game.script_image_names.clear();
                }
                if ((words[0] == "drawimage") || (words[0] == "drawimagepersist")) {
                    // drawimage(x,y,name[, centered])
                    int tempindex = getimage(words[3]);
                    if (tempindex == -1) {
                        SDL_Surface* image = LoadImage(words[3].c_str());
                        if (image == nullptr) {
                            position++;
                            continue;
                        }
                        game.script_images.push_back(image);
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
                            if (layername_to_enum.find(words[6]) != layername_to_enum.end())
                                temp.layer = layername_to_enum.at(words[6]);
                            else
                                temp.layer = Layer::top;
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
                        if (words[0] == "drawimagepersist") setvar("return", std::to_string((int)scriptrender.size()));
                        scriptrender.push_back(temp);
                    }
                }
                if (words[0] == "drawimagemasked") {
                    // drawimage(x,y,name, maskname)
                    int tempindex = getimage(words[3]);
                    if (tempindex == -1) {
                        SDL_Surface* image = LoadImage(words[3].c_str());
                        if (image == nullptr) {
                            position++;
                            continue;
                        }
                        game.script_images.push_back(image);
                        game.script_image_names.push_back(words[3]);
                        tempindex = (int)game.script_images.size() - 1;
                    }
                    int tempindex2 = getimage(words[4]);
                    if (tempindex2 == -1) {
                        SDL_Surface* image = LoadImage(words[4].c_str());
                        if (image == nullptr) {
                            position++;
                            continue;
                        }
                        game.script_images.push_back(image);
                        game.script_image_names.push_back(words[4]);
                        tempindex2 = (int)game.script_images.size() - 1;
                    }
                    scriptimage temp;
                    temp.type = 4;
                    temp.x = ss_toi(words[1]);
                    temp.y = ss_toi(words[2]);
                    temp.index = tempindex;
                    temp.mask_index = tempindex2;
                    temp.mask_x = ss_toi(words[5]);
                    temp.mask_y = ss_toi(words[6]);
                    scriptrender.push_back(temp);
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
                    graphics.blendmode = blend;
                }
                if (words[0] == "removeimage") {
                    // removeimage(id), to be used with drawimagepersist
                    scriptrender.erase(scriptrender.begin() + ss_toi(words[1]));
                }
                if (words[0] == "flag") {
                    if (ss_toi(words[1]) >= 0 && ss_toi(words[1]) < 1000) {
                        if (words[2] == "on") {
                            obj.flags[ss_toi(words[1])] = true;
                        } else if (words[2] == "off") {
                            obj.flags[ss_toi(words[1])] = false;
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
                        graphics.screenbuffer->badSignalEffect = true;
                    else
                        graphics.screenbuffer->badSignalEffect =
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
                    int player = obj.getplayer();
                    if (player > -1 && obj.entities[player].onroof > 0) {
                        game.press_action = true;
                        scriptdelay = 1;
                    }
                }
                if (words[0] == "toceil") {
                    int player = obj.getplayer();
                    if (player > -1 && obj.entities[player].onground > 0) {
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
                    if (i > -1) {
                        obj.entities.erase(obj.entities.begin() + i);
                        game.hascontrol = false;
                        music.fadeout();
                        music.playfile("pop.wav", "", 0);
                        SDL_SetWindowTitle(graphics.screenbuffer->m_window, "");
                        graphics.showcutscenebars = false;
                        running = false;
                        nointerrupt = false;
                        killedviridian = true;
                        killtimer = 120;
                    }
                }
                if (words[0] == "markmap") {
                    game.scriptmarkers.push_back(scriptmarker{
                        ss_toi(words[1]),
                        ss_toi(words[2]),
                        ss_toi(words[3]),
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
                if (words[0] == "markers") {
                    game.hidemarkers = !parsebool(words[1]);
                }
                if (words[0] == "mapimage") {
                    SDL_FreeSurface(graphics.images[12]);
                    SDL_Surface* image = LoadImage(words[1].c_str());
                    if (image == nullptr) {
                        position++;
                        continue;
                    }
                    graphics.images[12] = image;
                    graphics.mapimage = words[1];
                }
                if (words[0] == "automapimage") {
                    ed.generatecustomminimap();
                    graphics.mapimage = std::nullopt;
                }
                if (words[0] == "fog") {
                    map.nofog = !parsebool(words[1]);
                }
                if (words[0] == "finalstretch") {
                    if (parsebool(words[1])) {
                        map.finalstretch = true;
                        map.final_colormode = true;
                        map.final_colorframe = 1;
                        map.final_colorframedelay = 0;
                        map.colsuperstate = 1;
                    } else {
                        map.finalstretch = false;
                        map.final_colormode = false;
                        map.final_mapcol = 0;
                        map.colsuperstate = 0;
                        graphics.foregrounddrawn = false;
                    }
                }
                if (words[0] == "toggleflip") {
                    game.noflip = !parsebool(words[1]);
                }
                if (words[0] == "togglepause") {
                    game.noenter = !parsebool(words[1]);
                }
                if (words[0] == "infiniflip") {
                    game.infiniflip = parsebool(words[1]);
                }
                if (words[0] == "suicide") {
                    game.nosuicide = !parsebool(words[1]);
                }
                if (words[0] == "setspeed") {
                    game.playerspeed = std::stoi(words[1]);
                }
                if (words[0] == "setvelocity") {
                    game.nofriction = true;
                    int player = obj.getplayer();
                    if (player > -1) {
                        obj.entities[player].ax = std::stoi(words[1]);
                    }
                }
                if (words[0] == "playef") {
                    music.playef(ss_toi(words[1]));
                }
                if (words[0] == "playfile") {
                    int loops;
                    if (words[3] != "") {
                        loops = ss_toi(words[3]);
                    } else if (words[2] != "") {
                        loops = -1;
                    } else {
                        loops = 1;
                    }
                    music.playfile(words[1].c_str(), words[2], loops);
                }
                const char* word;
                if (strcmp((word = words[0].c_str()), "playfile")) {
                    music.playfile(++word, word, -1, true);
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
                    if (player > -1) {
                        relativepos(&obj.entities[player].xp, words[1]);
                        relativepos(&obj.entities[player].yp, words[2]);
                    }
                    if (words[3] != "") {
                        relativebool((bool*) &game.gravitycontrol, words[3]);
                    } else {
                        game.gravitycontrol = 0;
                    }
                }
                if (words[0] == "gotodimension") {
                    relativepos(&map.dimension, words[1]);
                    ed.generatecustomminimap();
                }
                if (words[0] == "gotoroom") {
                    // USAGE: gotoroom(x,y) (manually add 100)
                    map.gotoroom(relativepos(game.roomx - 100, words[1]) + 100,
                                relativepos(game.roomy - 100, words[2]) + 100);
                }
                if (words[0] == "reloadroom") {
                    // USAGE: reloadroom()
                    map.gotoroom(game.roomx, game.roomy);
                }
                if (words[0] == "reloadonetime") {
                    game.onetimescripts.erase(std::remove(game.onetimescripts.begin(), game.onetimescripts.end(), words[1]), game.onetimescripts.end());
                }
                if (words[0] == "reloadscriptboxes") {
                    for (size_t brs = 0; brs < obj.resurrectblocks.size(); brs++)
                        if (obj.resurrectblocks[brs].type == TRIGGER) {
                            obj.blocks.push_back(blockclass(obj.resurrectblocks[brs]));
                            obj.resurrectblocks.erase(obj.resurrectblocks.begin() + brs);
                            brs--;
                        }
                    obj.cleanupresurrectblocks();
                }
                if (words[0] == "reloadcustomactivityzones") {
                    // "Custom" here being defined as activity zones whose prompts
                    // are NOT terminals' prompts, e.g. "Press ENTER to activate
                    // terminal" or "Press ENTER to activate terminals"
                    for (size_t brz = 0; brz < obj.resurrectblocks.size(); brz++)
                        if (obj.resurrectblocks[brz].type == ACTIVITY &&
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
                                obj.resurrectblocks.erase(obj.resurrectblocks.begin() + brz);
                                brz--;
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
                            obj.resurrectblocks.erase(obj.resurrectblocks.begin() + brz);
                            brz--;
                        }
                    obj.cleanupresurrectblocks();
                }
                if (words[0] == "reloadterminalactivityzones") {
                    // Copied and pasted from the above, with some slight tweaks
                    // "Terminal" here being defined as activity zones whose prompts
                    // are terminals' prompts, e.g. "Press ENTER to activate
                    // terminal" or "Press ENTER to activate terminals"
                    for (size_t brt = 0; brt < obj.resurrectblocks.size(); brt++)
                        if (obj.resurrectblocks[brt].type == ACTIVITY &&
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
                                obj.resurrectblocks.erase(obj.resurrectblocks.begin() + brt);
                                brt--;
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
                            obj.resurrectblocks.erase(obj.resurrectblocks.begin() + brt);
                            brt--;
                        }
                    obj.cleanupresurrectblocks();
                }
                if (words[0] == "reloadactivityzones") {
                    // Copied and pasted from the above, with some slight tweaks
                    // (again)
                    for (size_t brz = 0; brz < obj.resurrectblocks.size(); brz++)
                        if (obj.resurrectblocks[brz].type == ACTIVITY) {
                            obj.customprompt = obj.resurrectblocks[brz].prompt;
                            if (obj.resurrectblocks[brz].script.length() < 7 ||
                                obj.resurrectblocks[brz].script.substr(0, 7) !=
                                    "custom_") {
                                // It's a main game activity zone, we won't reload
                                // it
                                obj.resurrectblocks.erase(obj.resurrectblocks.begin() + brz);
                                brz--;
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
                            obj.resurrectblocks.erase(obj.resurrectblocks.begin() + brz);
                            brz--;
                        }
                    obj.cleanupresurrectblocks();
                }
                if (words[0] == "cutscene") {
                    graphics.showcutscenebars = true;
                }
                if (words[0] == "endcutscene") {
                    graphics.showcutscenebars = false;
                }
                if (words[0] == "cutscenefast") {
                    graphics.showcutscenebars = true;
                    graphics.cutscenebarspos = 360;
                    graphics.oldcutscenebarspos = 360;
                }
                if (words[0] == "endcutscenefast") {
                    graphics.showcutscenebars = false;
                    graphics.cutscenebarspos = 0;
                    graphics.oldcutscenebarspos = 360;
                }
                if (words[0] == "untilbars" || words[0] == "puntilbars") {
                    if (graphics.showcutscenebars) {
                        if (graphics.cutscenebarspos < 360) {
                            scriptdelay = 1;
                            if (words[0] == "puntilbars") passive = true;
                            position--;
                        }
                    } else {
                        if (graphics.cutscenebarspos > 0) {
                            scriptdelay = 1;
                            if (words[0] == "puntilbars") passive = true;
                            position--;
                        }
                    }
                } else if (words[0] == "text") {
                    // oh boy
                    // first word is the colour.
                    if (words[1] == "cyan") {
                        r = 164;
                        g = 164;
                        b = 255;
                    } else if (words[1] == "player") {
                        r = 164;
                        g = 164;
                        b = 255;
                    } else if (words[1] == "red") {
                        r = 255;
                        g = 60;
                        b = 60;
                    } else if (words[1] == "green") {
                        r = 144;
                        g = 255;
                        b = 144;
                    } else if (words[1] == "yellow") {
                        r = 255;
                        g = 255;
                        b = 134;
                    } else if (words[1] == "blue") {
                        r = 95;
                        g = 95;
                        b = 255;
                    } else if (words[1] == "purple") {
                        r = 255;
                        g = 134;
                        b = 255;
                    } else if (words[1] == "orange") {
                        r = 255;
                        g = 130;
                        b = 20;
                    } else if (words[1] == "gray") {
                        r = 174;
                        g = 174;
                        b = 174;
                    } else {
                        // use a gray
                        r = 174;
                        g = 174;
                        b = 174;
                    }

                    // Time to abuse the assignment-is-an-expression trick
                    int a = 1;
                    if (words[5] != "" && words[6] != "") {
                        a = 0;  // Off-by-one

                        // We have 2 extra args, so there must be 3 color args
                        // instead of 1! 3 color args for R, G, and B
                        r = ss_toi(words[++a]);
                        g = ss_toi(words[++a]);
                        b = ss_toi(words[++a]);
                    }

                    // next are the x,y coordinates
                    textx = ss_toi(words[++a]);
                    texty = ss_toi(words[++a]);

                    // Number of lines for the textbox!
                    int n;
                    if (!words[++a].empty())
                        n = ss_toi(words[a]);
                    else
                        n = 1;

                    txt.clear();
                    for (int i = 0; i < n; i++) {
                        position++;
                        if (position < (int) commands.size()) {
                            txt.push_back(processvars(commands[position]));
                        }
                    }
                } else if (words[0] == "position") {
                    // are we facing left or right? for some objects we don't care,
                    // default at 0.
                    j = 0;

                    // the first word is the object to position relative to
                    if (words[1] == "centerx") {
                        if (words[2] != "")
                            textcenterline = ss_toi(words[2]);
                        else
                            textcenterline = 0;

                        words[2] = "donothing";
                        j = -1;
                        textx = -500;
                    } else if (words[1] == "centery") {
                        if (words[2] != "")
                            textcenterline = ss_toi(words[2]);
                        else
                            textcenterline = 0;

                        words[2] = "donothing";
                        j = -1;
                        texty = -500;
                    } else if (words[1] == "center") {
                        words[2] = "donothing";
                        j = -1;
                        textx = -500;
                        texty = -500;
                    } else {
                        i = obj.getcrewman(words[1]);
                        j = obj.entities[i].dir;
                    }

                    // next is whether to position above or below
                    if (words[2] == "above") {
                        if (j == 1)  // left
                        {
                            textx = obj.entities[i].xp -
                                    10000;  // tells the box to be oriented
                                            // correctly later
                            texty = obj.entities[i].yp - 16 - (txt.size() * 8);
                        } else if (j == 0)  // Right
                        {
                            textx = obj.entities[i].xp - 16;
                            texty = obj.entities[i].yp - 18 - (txt.size() * 8);
                        }
                    } else {
                        if (j == 1)  // left
                        {
                            textx = obj.entities[i].xp -
                                    10000;  // tells the box to be oriented
                                            // correctly later
                            texty = obj.entities[i].yp + 26;
                        } else if (j == 0)  // Right
                        {
                            textx = obj.entities[i].xp - 16;
                            texty = obj.entities[i].yp + 26;
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
                        textx = -500;
                    } else if (words[1] == "centery") {
                        words[2] = "donothing";
                        j = -1;
                        texty = -500;
                    } else if (words[1] == "center") {
                        words[2] = "donothing";
                        j = -1;
                        textx = -500;
                        texty = -500;
                    }

                    if (i == 0 && words[1] != "player" && words[1] != "cyan") {
                        // Requested crewmate is not actually on screen
                        words[2] = "donothing";
                        j = -1;
                        textx = -500;
                        texty = -500;
                    }

                    // next is whether to position above or below
                    if (words[2] == "above") {
                        if (j == 1)  // left
                        {
                            textx = obj.entities[i].xp -
                                    10000;  // tells the box to be oriented
                                            // correctly later
                            texty = obj.entities[i].yp - 16 - (txt.size() * 8);
                        } else if (j == 0)  // Right
                        {
                            textx = obj.entities[i].xp - 16;
                            texty = obj.entities[i].yp - 18 - (txt.size() * 8);
                        }
                    } else {
                        if (j == 1)  // left
                        {
                            textx = obj.entities[i].xp -
                                    10000;  // tells the box to be oriented
                                            // correctly later
                            texty = obj.entities[i].yp + 26;
                        } else if (j == 0)  // Right
                        {
                            textx = obj.entities[i].xp - 16;
                            texty = obj.entities[i].yp + 26;
                        }
                    }
                } else if (words[0] == "backgroundtext") {
                    game.backgroundtext = true;
                } else if (words[0] == "flipme") {
                    if (graphics.flipmode)
                        texty += 2 * (120 - texty) - 8 * (txt.size() + 2);
                } else if (words[0] == "speak" || words[0] == "speak_active" || words[0] == "speak_fast" || words[0] == "speak_active_fast") {
                    // Ok, actually display the textbox we've initilised now!
                    // If using "speak", don't make the textbox active (so we can use multiple textboxes)
                    // If using "speak_fast" or "speak_active_fast", create the textbox immediately
                    if (txt.empty()) {
                        txt.resize(1);
                    }
                    graphics.createtextbox(txt[0], textx, texty, r, g, b);
                    if ((int) txt.size() > 1) {
                        for (i = 1; i < (int) txt.size(); i++) {
                            graphics.addline(txt[i]);
                        }
                    }

                    // the textbox cannot be outside the screen. Fix if it is.
                    if (textx <= -1000) {
                        // position to the left of the player
                        textx += 10000;
                        textx -= graphics.textboxwidth();
                        textx += 16;
                        graphics.textboxmoveto(textx);
                    }

                    if (textx == -500 || textx == -1) {
                        if (textcenterline != 0)
                            graphics.textboxcenterx(textcenterline);
                        else
                            graphics.textboxcenterx();

                        // So it doesn't use the same line but Y instead of X for
                        // texty=-500
                        textcenterline = 0;
                    }

                    if (texty == -500) {
                        if (textcenterline != 0)
                            graphics.textboxcentery(textcenterline);
                        else
                            graphics.textboxcentery();

                        textcenterline = 0;
                    }

                    textcenterline = 0;

                    graphics.textboxadjust();
                    if (words[0] == "speak_active") {
                        graphics.textboxactive();
                    }
                    if (words[0] == "speak_fast" || words[0] == "speak_active_fast") {
                        graphics.textboxcreatefast();
                    }

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
                    graphics.textboxremove();
                    game.hascontrol = true;
                    game.advancetext = false;
                } else if (words[0] == "endtextfast") {
                    graphics.textboxremovefast();
                    game.hascontrol = true;
                    game.advancetext = false;
                } else if (words[0] == "textboxtimer") {
                    graphics.textboxtimer(ss_toi(words[1]));
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
                    if (i > -1) {
                        obj.entities[i].xp = 30;
                        obj.entities[i].yp = 46;
                        obj.entities[i].size = 13;
                        obj.entities[i].colour = 23;
                        obj.entities[i].cx = 36;       // 6;
                        obj.entities[i].cy = 12 + 80;  // 2;
                        obj.entities[i].h = 126 - 80;  // 21;
                    }
                } else if (words[0] == "undovvvvvvman") {
                    // Create the super VVVVVV combo!
                    i = obj.getplayer();
                    if (i > -1) {
                        obj.entities[i].xp = 100;
                        obj.entities[i].size = 0;
                        obj.entities[i].colour = 0;
                        obj.entities[i].cx = 6;
                        obj.entities[i].cy = 2;
                        obj.entities[i].h = 21;
                    }
                } else if (words[0] == "createentity") {
                    auto k = obj.createentity(ss_toi(words[1]),
                                            ss_toi(words[2]), ss_toi(words[3]),
                                            ss_toi(words[4]), ss_toi(words[5]));
                    if (words[6] != "") {
                        switch (ss_toi(words[6])) {
                            case 0: obj.entities[k].setenemyroom(4 + 100, 0 + 100); break;
                            case 1: obj.entities[k].setenemyroom(2 + 100, 0 + 100); break;
                            case 2: obj.entities[k].setenemyroom(12 + 100, 3 + 100); break;
                            case 3: obj.entities[k].setenemyroom(13 + 100, 12 + 100); break;
                            case 4: obj.entities[k].setenemyroom(16 + 100, 9 + 100); break;
                            case 5: obj.entities[k].setenemyroom(19 + 100, 1 + 100); break;
                            case 6: obj.entities[k].setenemyroom(19 + 100, 2 + 100); break;
                            case 7: obj.entities[k].setenemyroom(18 + 100, 3 + 100); break;
                            case 8: obj.entities[k].setenemyroom(16 + 100, 0 + 100); break;
                            case 9: obj.entities[k].setenemyroom(14 + 100, 2 + 100); break;
                            case 10: obj.entities[k].setenemyroom(10 + 100, 7 + 100); break;
                            case 11: obj.entities[k].setenemyroom(12 + 100, 5 + 100); break;  // yes man
                            case 12: obj.entities[k].setenemyroom(15 + 100, 3 + 100); break;  // STOP
                            case 13: obj.entities[k].setenemyroom(13 + 100, 3 + 100); break;  // wave duude
                            case 14: obj.entities[k].setenemyroom(15 + 100, 2 + 100); break;  // numbers
                            case 15: obj.entities[k].setenemyroom(16 + 100, 2 + 100); break;  // the dudes that walk
                            case 16: obj.entities[k].setenemyroom(18 + 100, 2 + 100); break;  // boob
                            case 17: obj.entities[k].setenemyroom(18 + 100, 0 + 100); break;  // OBEY
                            case 18: obj.entities[k].setenemyroom(17 + 100, 3 + 100); break;  // edge games
                            case 19: obj.entities[k].setenemyroom(13 + 100, 6 + 100); break;  // sent over the bottom gottem lmao
                            case 20: obj.entities[k].setenemyroom(16 + 100, 7 + 100); break;  // ghos
                            case 21: obj.entities[k].setenemyroom(17 + 100, 7 + 100); break;  // they b walkin 2.0
                            case 22: obj.entities[k].setenemyroom(14 + 100, 8 + 100); break;  // what the fuck is this
                            case 23: obj.entities[k].setenemyroom(11 + 100, 13 + 100); break;  // TRUTH
                            case 24: obj.entities[k].setenemyroom(14 + 100, 13 + 100); break;  // DABBING SKELETON
                            case 25: obj.entities[k].setenemyroom(44, 51); break;  // enemy in vertigo
                            case 26:
                                obj.entities[k].tile = 24;
                                obj.entities[k].animate = 0;
                                break;
                            case 27: obj.entities[k].setenemyroom(13+100, 7+100); break;  // bus
                            default: obj.entities[k].setenemyroom(4 + 100, 0 + 100); break;
                        }
                    }
                    setvar("return", std::to_string(k));
                } else if (words[0] == "fatal_left") {
                    obj.fatal_left();
                } else if (words[0] == "fatal_right") {
                    obj.fatal_right();
                } else if (words[0] == "fatal_top") {
                    obj.fatal_top();
                } else if (words[0] == "fatal_bottom") {
                    obj.fatal_bottom();
                } else if (words[0] == "supercrewmate") {
                    game.supercrewmate = parsebool(words[1]);
                } else if (words[0] == "supercrewmateroom") {
                    game.scmprogress = game.roomx - 41;
#define ENTITYDATA \
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
#define X(FIELD) if (words[2] == #FIELD) setvar(words[3], stringify(obj.entities[ss_toi(words[1])].FIELD));
                    ENTITYDATA
#undef X
                } else if (words[0] == "createcrewman") {
                    if (words[3] == "cyan") {
                        r = 0;
                    } else if (words[3] == "red") {
                        r = 15;
                    } else if (words[3] == "green") {
                        r = 13;
                    } else if (words[3] == "yellow") {
                        r = 14;
                    } else if (words[3] == "blue") {
                        r = 16;
                    } else if (words[3] == "purple") {
                        r = 20;
                    } else if (strspn(words[3].c_str(), "-.0123456789") ==
                                words[3].size() &&
                            words[3].size() != 0) {
                        r = ss_toi(words[3]);
                    } else {
                        r = 19;
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
                            ss_toi(words[1]), ss_toi(words[2]), ent, r,
                            ss_toi(words[4]), ss_toi(words[5]), ss_toi(words[6]));
                    } else {
                        id = obj.createentity(ss_toi(words[1]),
                                            ss_toi(words[2]), ent, r,
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
                    if (position < (int) commands.size())
                        map.roomtext.push_back(Roomtext{
                            ss_toi(words[1]) / 8,
                            ss_toi(words[2]) / 8,
                            ss_toi(words[1]) % 8,
                            ss_toi(words[2]) % 8,
                            commands[position],
                        });
                } else if (words[0] == "createscriptbox") {
                    // Ok, first figure out the first available script box slot
                    int lastslot = 0;
                    for (size_t bsi = 0; bsi < obj.blocks.size(); bsi++)
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
                        if (position < (int) commands.size())
                            obj.customprompt = processvars(commands[position]);
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
                        if (position < (int) commands.size())
                            obj.customprompt = processvars(commands[position]);
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
                    if (i > -1) {
                        obj.entities[i].tile = 6;
                        obj.entities[i].colour = 102;
                    }
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
                        music.playef(11);
                    } else if (words[1] == "cyan") {
                        music.playef(11);
                    } else if (words[1] == "red") {
                        music.playef(16);
                    } else if (words[1] == "green") {
                        music.playef(12);
                    } else if (words[1] == "yellow") {
                        music.playef(14);
                    } else if (words[1] == "blue") {
                        music.playef(13);
                    } else if (words[1] == "purple") {
                        music.playef(15);
                    } else if (words[1] == "cry") {
                        music.playef(2);
                    } else if (words[1] == "terminal") {
                        music.playef(20);
                    }
                } else if (words[0] == "blackout") {
                    game.blackout = true;
                } else if (words[0] == "blackon") {
                    game.blackout = false;
                } else if (words[0] == "setcheckpoint") {
                    i = obj.getplayer();
                    game.savepoint = 0;
                    if (words[3] != "")
                        relativepos(&game.savex, words[3]);
                    else if (i > -1)
                        game.savex = obj.entities[i].xp;
                    if (words[4] != "")
                        relativepos(&game.savey, words[4]);
                    else if (i > -1)
                        game.savey = obj.entities[i].yp;
                    if (words[5] != "")
                        relativebool((bool*) &game.savegc, words[5]);
                    else
                        game.savegc = game.gravitycontrol;
                    if (words[1] != "")
                        relativepos(&game.saverx, words[1]);
                    else
                        game.saverx = game.roomx;
                    if (words[2] != "")
                        relativepos(&game.savery, words[2]);
                    else
                        game.savery = game.roomy;
                    if (i > -1)
                        game.savedir = obj.entities[i].dir;
                } else if (words[0] == "gotocheckpoint") {
                    i = obj.getplayer();
                    if (i > -1) {
                        obj.entities[i].xp = game.savex;
                        obj.entities[i].yp = game.savey;
                    }
                    game.gravitycontrol = game.savegc;
                    game.roomx = game.saverx;
                    game.roomy = game.savery;
                    obj.entities[i].dir = game.savedir;
                } else if (words[0] == "gamestate") {
                    game.state = ss_toi(words[1]);
                    game.statedelay = 0;
                } else if (words[0] == "gamestatedelay") {
                    relativepos(&game.statedelay, words[1]);
                } else if (words[0] == "textboxactive") {
                    graphics.textboxactive();
                } else if (words[0] == "gamemode") {
                    if (words[1] == "teleporter") {
                        // TODO this draw the teleporter screen. This is a problem.
                        // :(
                        game.gamestate = TELEPORTERMODE;
                        graphics.menuoffset =
                            240;  // actually this should count the roomname
                        graphics.oldmenuoffset = 240;
                        if (map.extrarow) {
                            graphics.menuoffset -= 10;
                            graphics.oldmenuoffset -= 10;
                        }

                        graphics.resumegamemode = false;

                        game.useteleporter =
                            false;  // good heavens don't actually use it
                    } else if (words[1] == "game") {
                        graphics.resumegamemode = true;
                    }
                } else if (words[0] == "ifexplored") {
                    int room = ss_toi(words[1]) + (ed.maxwidth * ss_toi(words[2]));
                    if (room >= 0 && room < (int) map.explored.size() && map.explored[room] == 1) {
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
                    int flag = ss_toi(words[1]);
                    if (flag >= 0 && flag < (int) obj.flags.size() && obj.flags[flag]) {
                        call(words[2]);
                        position--;
                    }
                } else if (words[0] == "ifnotflag") {
                    int flag = ss_toi(words[1]);
                    if (flag >= 0 && flag < (int) obj.flags.size() && !obj.flags[flag]) {
                        call("custom_" + words[2]);
                        continue;
                    }
                } else if (words[0] == "ifcrewlost") {
                    int crewmate = ss_toi(words[1]);
                    if (crewmate >= 0 && crewmate < (int) game.crewstats.size() && game.crewstats[crewmate] == false) {
                        call(words[2]);
                        position--;
                    }
                } else if (words[0] == "iftrinkets") {
                    if (game.trinkets() >= ss_toi(words[1])) {
                        call(words[2]);
                        position--;
                    }
                } else if (words[0] == "ifcrewmates") {
                    if (game.crewmates() >= ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        continue;
                    }
                } else if (words[0] == "ifcoins") {
                    if (game.coins >= ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        continue;
                    }
                } else if (words[0] == "iftrinketsless") {
                    if (game.stat_trinkets < ss_toi(words[1])) {
                        call(words[2]);
                        position--;
                    }
                } else if (words[0] == "ifcrewmatesless") {
                    if (game.crewmates() < ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        continue;
                    }
                } else if (words[0] == "ifcoinsless") {
                    if (game.coins < ss_toi(words[1])) {
                        call("custom_" + words[2]);
                        continue;
                    }
                } else if (words[0] == "ifrand") {
                    int den = ss_toi(words[1]);
                    if (fRandom() < 1.0f / den) {
                        call("custom_" + words[2]);
                        continue;
                    }
                } else if (words[0] == "ifvce") {
                    call("custom_" + words[1]);
                    continue;
                } else if (words[0] == "ifmod") {
                    if (words[1] == "mmmmmm" && music.mmmmmm) {
                        call("custom_" + words[2]);
                        continue;
                    } else if ((words[1] == "mmmmmm_on" ||
                                words[1] == "mmmmmm_enabled") &&
                            music.mmmmmm && music.usingmmmmmm) {
                        call("custom_" + words[2]);
                        continue;
                    } else if ((words[1] == "mmmmmm_off" ||
                                words[1] == "mmmmmm_disabled") &&
                            music.mmmmmm && !music.usingmmmmmm) {
                        call("custom_" + words[2]);
                        continue;
                    } else if (words[1] == "unifont" && graphics.grphx.im_unifont &&
                            graphics.grphx.im_wideunifont) {
                        call("custom_" + words[2]);
                        continue;
                    }
                } else if (words[0] == "hidecoordinates") {
                    int room = ss_toi(words[1]) + (ed.maxwidth * ss_toi(words[2]));
                    if (room >= 0 && room < (int) map.explored.size()) {
                        map.explored[room] = 0;
                    }
                } else if (words[0] == "showcoordinates") {
                    int room = ss_toi(words[1]) + (ed.maxwidth * ss_toi(words[2]));
                    if (room >= 0 && room < (int) map.explored.size()) {
                        map.explored[room] = 1;
                    }
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
                    int player = obj.getplayer();
                    if (player > -1) {
                        obj.entities[player].invis = true;
                    }
                } else if (words[0] == "showplayer") {
                    int player = obj.getplayer();
                    if (player > -1) {
                        obj.entities[player].invis = false;
                    }
                } else if (words[0] == "killplayer") {
                    if (!map.invincibility && game.deathseq <= 0)
                        game.deathseq = 30;
                } else if (words[0] == "coincounter") {
                    game.nocoincounter = !parsebool(words[1]);
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
                    graphics.fadeamount = 0;
                    graphics.fademode = 0;
                } else if (words[0] == "befadeout") {
                    graphics.fademode = 1;
                } else if (words[0] == "fadein") {
                    graphics.fademode = 4;
                } else if (words[0] == "fadeout") {
                    graphics.fademode = 2;
                } else if (words[0] == "untilfade" || words[0] == "puntilfade") {
                    if (graphics.fademode > 1) {
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
                    map.resetplayer();
                    map.tdrawback = true;

                    obj.resetallflags();
                    i = obj.getplayer();
                    if (i > -1) {
                        obj.entities[i].tile = 0;
                    }

                    for (i = 0; i < 100; i++) {
                        obj.collect[i] = false;
                        obj.customcollect[i] = false;
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
                        throw script_exception("Returned with empty callstack!");
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
                    continue;
                } else if (words[0] == "rollcredits") {
                    game.gamestate = GAMECOMPLETE;
                    graphics.fademode = 4;
                    game.creditposition = 0;
                } else if (words[0] == "finalmode") {
                    map.finalmode = true;
                    map.finalx = ss_toi(words[1]);
                    map.finaly = ss_toi(words[2]);
                    game.roomx = map.finalx;
                    game.roomy = map.finaly;
                    map.gotoroom(game.roomx, game.roomy);
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
                    for (j = 0; j < (int) obj.entities.size(); j++) {
                        if (obj.entities[j].type == 13) {
                            obj.entities[j].colour = 4;
                        }
                    }
                    if (ss_toi(words[1]) == 1) {
                        obj.createblock(5, 88 - 4, 80, 20, 16, 25);
                        for (j = 0; j < (int) obj.entities.size(); j++) {
                            if (obj.entities[j].xp == 88 &&
                                obj.entities[j].yp == 80) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 2) {
                        obj.createblock(5, 128 - 4, 80, 20, 16, 26);
                        for (j = 0; j < (int) obj.entities.size(); j++) {
                            if (obj.entities[j].xp == 128 &&
                                obj.entities[j].yp == 80) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 3) {
                        obj.createblock(5, 176 - 4, 80, 20, 16, 27);
                        for (j = 0; j < (int) obj.entities.size(); j++) {
                            if (obj.entities[j].xp == 176 &&
                                obj.entities[j].yp == 80) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 4) {
                        obj.createblock(5, 216 - 4, 80, 20, 16, 28);
                        for (j = 0; j < (int) obj.entities.size(); j++) {
                            if (obj.entities[j].xp == 216 &&
                                obj.entities[j].yp == 80) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 5) {
                        obj.createblock(5, 88 - 4, 128, 20, 16, 29);
                        for (j = 0; j < (int) obj.entities.size(); j++) {
                            if (obj.entities[j].xp == 88 &&
                                obj.entities[j].yp == 128) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 6) {
                        obj.createblock(5, 176 - 4, 128, 20, 16, 30);
                        for (j = 0; j < (int) obj.entities.size(); j++) {
                            if (obj.entities[j].xp == 176 &&
                                obj.entities[j].yp == 128) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 7) {
                        obj.createblock(5, 40 - 4, 40, 20, 16, 31);
                        for (j = 0; j < (int) obj.entities.size(); j++) {
                            if (obj.entities[j].xp == 40 &&
                                obj.entities[j].yp == 40) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 8) {
                        obj.createblock(5, 216 - 4, 128, 20, 16, 32);
                        for (j = 0; j < (int) obj.entities.size(); j++) {
                            if (obj.entities[j].xp == 216 &&
                                obj.entities[j].yp == 128) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 9) {
                        obj.createblock(5, 128 - 4, 128, 20, 16, 33);
                        for (j = 0; j < (int) obj.entities.size(); j++) {
                            if (obj.entities[j].xp == 128 &&
                                obj.entities[j].yp == 128) {
                                obj.entities[j].colour = 5;
                            }
                        }
                    } else if (ss_toi(words[1]) == 10) {
                        obj.createblock(5, 264 - 4, 40, 20, 16, 34);
                        for (j = 0; j < (int) obj.entities.size(); j++) {
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
                        obj.createentity(i, 153, 18, 14, 0, 17, 0);
                        i += 25;
                    }
                    if (game.crewstats[3] && game.lastsaved != 3) {
                        obj.createentity(i, 153, 18, 15, 0, 17, 0);
                        i += 25;
                    }
                    if (game.crewstats[4] && game.lastsaved != 4) {
                        obj.createentity(i, 153, 18, 13, 0, 17, 0);
                        i += 25;
                    }
                    if (game.crewstats[5] && game.lastsaved != 5) {
                        obj.createentity(i, 153, 18, 16, 0, 17, 0);
                        i += 25;
                    }
                } else if (words[0] == "keepcolor") {
                    keepcolor = parsebool(words[1]);
                } else if (words[0] == "restoreplayercolour" ||
                        words[0] == "restoreplayercolor") {
                    i = obj.getplayer();
                    if (i > -1) {
                        obj.entities[i].colour = 0;
                    }
                    game.playercolour = 0;
                } else if (words[0] == "changeplayercolour" ||
                        words[0] == "changeplayercolor") {
                    i = obj.getplayer();

                    if (i > -1) {
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
                    }
                } else if (words[0] == "altstates") {
                    obj.altstates = ss_toi(words[1]);
                } else if (words[0] == "activeteleporter") {
                    i = obj.getteleporter();
                    if (i > -1) {
                        obj.entities[i].colour = 101;
                    }
                } else if (words[0] == "foundtrinket") {
                    // music.silencedasmusik();
                    music.haltdasmusik();
                    music.playef(3);

				int trinket = ss_toi(words[1]);
				if (trinket >= 0 && trinket < (int) obj.collect.size())
				{
					obj.collect[trinket] = true;
				}

                    graphics.textboxremovefast();

                    graphics.createtextbox("        Congratulations!       ", 50, 85,
                                        174, 174, 174);
                    graphics.addline("");
                    graphics.addline("You have found a shiny trinket!");
                    graphics.textboxcenterx();

                    std::string usethisnum;
                    if (map.custommode) {
                        usethisnum = help.number(ed.numtrinkets());
                    } else {
                        usethisnum = "Twenty";
                    }
                    graphics.createtextbox(" " + help.number(game.trinkets()) +
                                            " out of " + usethisnum + " ",
                                        50, 135, 174, 174, 174);
                    graphics.textboxcenterx();

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
                    music.playef(3);

                    graphics.textboxremovefast();

                    graphics.createtextbox("        Congratulations!       ", 50, 85,
                                        174, 174, 174);
                    graphics.addline("");
                    graphics.addline("You have found the secret lab!");
                    graphics.textboxcenterx();
                    graphics.textboxcentery();

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
                    graphics.textboxremovefast();

                    graphics.createtextbox("The secret lab is separate from", 50, 85,
                                        174, 174, 174);
                    graphics.addline("the rest of the game. You can");
                    graphics.addline("now come back here at any time");
                    graphics.addline("by selecting the new SECRET LAB");
                    graphics.addline("option in the play menu.");
                    graphics.textboxcenterx();
                    graphics.textboxcentery();

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
                    for (i = 0; i < (int) obj.entities.size(); i++) {
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

                    map.gotoroom(46, 54);
                } else if (words[0] == "telesave") {
                    if (!game.intimetrial && !game.nodeathmode &&
                        !game.inintermission)
                        game.savetele();
                }
                if (words[0] == "customquicksave") {
                    if (!map.custommode || map.custommodeforreal)
                        if (!game.intimetrial && !game.nodeathmode)
                            game.customsavequick(ed.ListOfMetaData[game.playcustomlevel].filename);
                } else if (words[0] == "createlastrescued") {
                    if (game.lastsaved == 2) {
                        r = 14;
                    } else if (game.lastsaved == 3) {
                        r = 15;
                    } else if (game.lastsaved == 4) {
                        r = 13;
                    } else if (game.lastsaved == 5) {
                        r = 16;
                    } else {
                        r = 19;
                    }

                    obj.createentity(200, 153, 18, r, 0, 19, 30);
                    i = obj.getcrewman(game.lastsaved);
                    obj.entities[i].dir = 1;
                } else if (words[0] == "specialline") {
                    switch (ss_toi(words[1])) {
                        case 1:
                            txt.resize(1);

                            txt[0] = "I'm worried about " + game.unrescued() +
                                    ", Doctor!";
                            break;
                        case 2:
                            txt.resize(3);

                            if (game.crewrescued() < 5) {
                                txt[1] = "to helping you find the";
                                txt[2] = "rest of the crew!";
                            } else {
                                txt.resize(2);
                                txt[1] =
                                    "to helping you find " + game.unrescued() + "!";
                            }
                            break;
                    }
                } else if (words[0] == "trinketbluecontrol") {
                    if (game.trinkets() == 20 && obj.flags[67]) {
                        call("talkblue_trinket6");
                        position--;
                    } else if (game.trinkets() >= 19 && !obj.flags[67]) {
                        call("talkblue_trinket5");
                        position--;
                    } else {
                        call("talkblue_trinket4");
                        position--;
                    }
                } else if (words[0] == "trinketyellowcontrol") {
                    if (game.trinkets() >= 19) {
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
                    } else if (obj.flags[67]) {
                        // game complete
                        call("talkred_13");
                        position--;
                    } else if (obj.flags[35] && !obj.flags[52]) {
                        // Intermission level
                        obj.flags[52] = true;
                        call("talkred_9");
                        position--;
                    } else if (!obj.flags[51]) {
                        // We're back home!
                        obj.flags[51] = true;
                        call("talkred_5");
                        position--;
                    } else if (!obj.flags[48] && game.crewstats[5]) {
                        // Victoria's back
                        obj.flags[48] = true;
                        call("talkred_6");
                        position--;
                    } else if (!obj.flags[49] && game.crewstats[4]) {
                        // Verdigris' back
                        obj.flags[49] = true;
                        call("talkred_7");
                        position--;
                    } else if (!obj.flags[50] && game.crewstats[2]) {
                        // Vitellary's back
                        obj.flags[50] = true;
                        call("talkred_8");
                        position--;
                    } else if (!obj.flags[45] && !game.crewstats[5]) {
                        obj.flags[45] = true;
                        call("talkred_2");
                        position--;
                    } else if (!obj.flags[46] && !game.crewstats[4]) {
                        obj.flags[46] = true;
                        call("talkred_3");
                        position--;
                    } else if (!obj.flags[47] && !game.crewstats[2]) {
                        obj.flags[47] = true;
                        call("talkred_4");
                        position--;
                    } else {
                        obj.flags[45] = false;
                        obj.flags[46] = false;
                        obj.flags[47] = false;
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
                    } else if (obj.flags[67]) {
                        // game complete
                        call("talkgreen_10");
                        position--;
                    } else if (obj.flags[34] && !obj.flags[57]) {
                        // Intermission level
                        obj.flags[57] = true;
                        call("talkgreen_7");
                        position--;
                    } else if (!obj.flags[53]) {
                        // Home!
                        obj.flags[53] = true;
                        call("talkgreen_6");
                        position--;
                    } else if (!obj.flags[54] && game.crewstats[2]) {
                        obj.flags[54] = true;
                        call("talkgreen_5");
                        position--;
                    } else if (!obj.flags[55] && game.crewstats[3]) {
                        obj.flags[55] = true;
                        call("talkgreen_4");
                        position--;
                    } else if (!obj.flags[56] && game.crewstats[5]) {
                        obj.flags[56] = true;
                        call("talkgreen_3");
                        position--;
                    } else if (!obj.flags[58]) {
                        obj.flags[58] = true;
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
                    } else if (obj.flags[67]) {
                        // game complete, everything changes for victoria
                        if (obj.flags[41] && !obj.flags[42]) {
                            // second trinket conversation
                            obj.flags[42] = true;
                            call("talkblue_trinket2");
                            position--;
                        } else if (!obj.flags[41] && !obj.flags[42]) {
                            // Third trinket conversation
                            obj.flags[42] = true;
                            call("talkblue_trinket3");
                            position--;
                        } else {
                            // Ok, we've already dealt with the trinket thing; so
                            // either you have them all, or you don't. If you do:
                            if (game.trinkets() >= 20) {
                                call("startepilogue");
                                position--;
                            } else {
                                call("talkblue_8");
                                position--;
                            }
                        }
                    } else if (obj.flags[33] && !obj.flags[40]) {
                        // Intermission level
                        obj.flags[40] = true;
                        call("talkblue_7");
                        position--;
                    } else if (!obj.flags[36] && game.crewstats[5]) {
                        // Back on the ship!
                        obj.flags[36] = true;
                        call("talkblue_3");
                        position--;
                    } else if (!obj.flags[41] && game.crewrescued() <= 4) {
                        // First trinket conversation
                        obj.flags[41] = true;
                        call("talkblue_trinket1");
                        position--;
                    } else if (obj.flags[41] && !obj.flags[42] &&
                            game.crewrescued() == 5) {
                        // second trinket conversation
                        obj.flags[42] = true;
                        call("talkblue_trinket2");
                        position--;
                    } else if (!obj.flags[41] && !obj.flags[42] &&
                            game.crewrescued() == 5) {
                        // Third trinket conversation
                        obj.flags[42] = true;
                        call("talkblue_trinket3");
                        position--;
                    } else if (!obj.flags[37] && game.crewstats[2]) {
                        obj.flags[37] = true;
                        call("talkblue_4");
                        position--;
                    } else if (!obj.flags[38] && game.crewstats[3]) {
                        obj.flags[38] = true;
                        call("talkblue_5");
                        position--;
                    } else if (!obj.flags[39] && game.crewstats[4]) {
                        obj.flags[39] = true;
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
                    } else if (obj.flags[67]) {
                        // game complete
                        call("talkyellow_11");
                        position--;
                    } else if (obj.flags[32] && !obj.flags[31]) {
                        // Intermission level
                        obj.flags[31] = true;
                        call("talkyellow_6");
                        position--;
                    } else if (!obj.flags[27] && game.crewstats[2]) {
                        // Back on the ship!
                        obj.flags[27] = true;
                        call("talkyellow_10");
                        position--;
                    } else if (!obj.flags[43] && game.crewrescued() == 5 &&
                            !game.crewstats[5]) {
                        // If by chance we've rescued everyone except Victoria by
                        // the end, Vitellary provides you with the trinket
                        // information instead.
                        obj.flags[43] = true;
                        obj.flags[42] = true;
                        obj.flags[41] = true;
                        call("talkyellow_trinket1");
                        position--;
                    } else if (!obj.flags[24] && game.crewstats[5]) {
                        obj.flags[24] = true;
                        call("talkyellow_8");
                        position--;
                    } else if (!obj.flags[26] && game.crewstats[4]) {
                        obj.flags[26] = true;
                        call("talkyellow_7");
                        position--;
                    } else if (!obj.flags[25] && game.crewstats[3]) {
                        obj.flags[25] = true;
                        call("talkyellow_9");
                        position--;
                    } else if (!obj.flags[28]) {
                        obj.flags[28] = true;
                        call("talkyellow_3");
                        position--;
                    } else if (!obj.flags[29]) {
                        obj.flags[29] = true;
                        call("talkyellow_4");
                        position--;
                    } else if (!obj.flags[30]) {
                        obj.flags[30] = true;
                        call("talkyellow_5");
                        position--;
                    } else if (!obj.flags[23]) {
                        obj.flags[23] = true;
                        call("talkyellow_2");
                        position--;
                    } else {
                        call("talkyellow_1");
                        position--;
                        obj.flags[23] = false;
                    }
                } else if (words[0] == "purplecontrol") {
                    // Controls Purple's conversion
                    // Crew rescued:
                    if (game.insecretlab) {
                        call("talkpurple_9");
                        position--;
                    } else if (obj.flags[67]) {
                        // game complete
                        call("talkpurple_8");
                        position--;
                    } else if (!obj.flags[17] && game.crewstats[4]) {
                        obj.flags[17] = true;
                        call("talkpurple_6");
                        position--;
                    } else if (!obj.flags[15] && game.crewstats[5]) {
                        obj.flags[15] = true;
                        call("talkpurple_4");
                        position--;
                    } else if (!obj.flags[16] && game.crewstats[3]) {
                        obj.flags[16] = true;
                        call("talkpurple_5");
                        position--;
                    } else if (!obj.flags[18] && game.crewstats[2]) {
                        obj.flags[18] = true;
                        call("talkpurple_7");
                        position--;
                    } else if (obj.flags[19] && !obj.flags[20] &&
                            !obj.flags[21]) {
                        // intermission one: if played one / not had first
                        // conversation / not played two [conversation one]
                        obj.flags[21] = true;
                        call("talkpurple_intermission1");
                        position--;
                    } else if (!obj.flags[20] && obj.flags[21] &&
                            !obj.flags[22]) {
                        // intermission two: if played two / had first conversation
                        // / not had second conversation [conversation two]
                        obj.flags[22] = true;
                        call("talkpurple_intermission2");
                        position--;
                    } else if (obj.flags[20] && !obj.flags[21] &&
                            !obj.flags[22]) {
                        // intermission two: if played two / not had first
                        // conversation / not had second conversation [conversation
                        // three]
                        obj.flags[22] = true;
                        call("talkpurple_intermission3");
                        position--;
                    } else if (!obj.flags[12]) {
                        // Intro conversation
                        obj.flags[12] = true;
                        call("talkpurple_intro");
                        position--;
                    } else if (!obj.flags[14]) {
                        // Shorter intro conversation
                        obj.flags[14] = true;
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
                nointerrupt = false;
            }
        }

        if (scriptdelay > 0) {
            scriptdelay--;
        }
    } catch (const std::exception& ex) {
        handle_exception(ex);
    }
}

void scriptclass::resetgametomenu() {
    game.gamestate = TITLEMODE;
    FILESYSTEM_unmountassets();
    graphics.flipmode = false;
    graphics.fademode = 4;
    map.tdrawback = true;
    game.createmenu(Menu::gameover);
}

void scriptclass::startgamemode(int t) {
    graphics.noclear = false;
    graphics.mapimage = std::nullopt;
    callstack.clear();

    switch (t) {
        case 0:  // Normal new game
            game.gamestate = GAMEMODE;
            hardreset();
            game.start();
            game.jumpheld = true;
            graphics.showcutscenebars = true;
            graphics.cutscenebarspos = 320;
            graphics.oldcutscenebarspos = 320;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;

            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            call("intro");
            break;
        case 1:
            game.gamestate = GAMEMODE;
            hardreset();
            game.start();
            game.loadtele();
            game.gravitycontrol = game.savegc;
            game.jumpheld = true;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;

            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            graphics.fademode = 4;
            break;
        case 2:  // Load Quicksave
            game.gamestate = GAMEMODE;
            hardreset();
            game.start();
            game.loadquick();
            game.gravitycontrol = game.savegc;
            game.jumpheld = true;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;

            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            // a very special case for here needs to ensure that the tower is
            // set correctly
            if (map.towermode) {
                map.resetplayer();

                i = obj.getplayer();
                if (i > -1) {
                    map.ypos = obj.entities[i].yp - 120;
                }
                map.bypos = map.ypos / 2;
                map.cameramode = 0;
                map.colsuperstate = 0;
            }
            graphics.fademode = 4;
            break;
        case 3:
            // Start Time Trial 1
            hardreset();
            game.nocutscenes = true;
            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 0;
            game.timetrialpar = 75;
            game.timetrialshinytarget = 2;

            music.fadeout();
            game.gamestate = GAMEMODE;
            game.starttrial(game.timetriallevel);
            game.jumpheld = true;

            if (graphics.setflipmode) graphics.flipmode = true;  // set flipmode
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            graphics.fademode = 4;
            break;
        case 4:
            // Start Time Trial 2
            hardreset();
            game.nocutscenes = true;
            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 1;
            game.timetrialpar = 165;
            game.timetrialshinytarget = 4;

            music.fadeout();
            game.gamestate = GAMEMODE;
            game.starttrial(game.timetriallevel);
            game.jumpheld = true;

            if (graphics.setflipmode) graphics.flipmode = true;  // set flipmode
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            graphics.fademode = 4;
            break;
        case 5:
            // Start Time Trial 3 tow
            hardreset();
            game.nocutscenes = true;
            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 2;
            game.timetrialpar = 105;
            game.timetrialshinytarget = 2;

            music.fadeout();
            game.gamestate = GAMEMODE;
            game.starttrial(game.timetriallevel);
            game.jumpheld = true;

            if (graphics.setflipmode) graphics.flipmode = true;  // set flipmode
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            graphics.fademode = 4;
            break;
        case 6:
            // Start Time Trial 4 station
            hardreset();
            game.nocutscenes = true;
            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 3;
            game.timetrialpar = 200;
            game.timetrialshinytarget = 5;

            music.fadeout();
            game.gamestate = GAMEMODE;
            game.starttrial(game.timetriallevel);
            game.jumpheld = true;

            if (graphics.setflipmode) graphics.flipmode = true;  // set flipmode
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            graphics.fademode = 4;
            break;
        case 7:
            // Start Time Trial 5 warp
            hardreset();
            game.nocutscenes = true;
            game.intimetrial = true;
            game.timetrialcountdown = 150;
            game.timetrialparlost = false;
            game.timetriallevel = 4;
            game.timetrialpar = 120;
            game.timetrialshinytarget = 1;

            music.fadeout();
            game.gamestate = GAMEMODE;
            game.starttrial(game.timetriallevel);
            game.jumpheld = true;

            if (graphics.setflipmode) graphics.flipmode = true;  // set flipmode
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            graphics.fademode = 4;
            break;
        case 8:
            // Start Time Trial 6// final level!
            hardreset();
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
            game.starttrial(game.timetriallevel);
            game.jumpheld = true;

            if (graphics.setflipmode) graphics.flipmode = true;  // set flipmode
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            graphics.fademode = 4;
            break;
        case 9:
            game.gamestate = GAMEMODE;
            hardreset();
            game.nodeathmode = true;
            game.start();
            game.jumpheld = true;
            graphics.showcutscenebars = true;
            graphics.cutscenebarspos = 320;
            graphics.oldcutscenebarspos = 320;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;

            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            call("intro");
            break;
        case 10:
            game.gamestate = GAMEMODE;
            hardreset();
            game.nodeathmode = true;
            game.nocutscenes = true;

            game.start();
            game.jumpheld = true;
            graphics.showcutscenebars = true;
            graphics.cutscenebarspos = 320;
            graphics.oldcutscenebarspos = 320;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;

            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            call("intro");
            break;
        case 11:
            game.gamestate = GAMEMODE;
            hardreset();

            game.startspecial(0);
            game.jumpheld = true;

            // Secret lab, so reveal the map, give them all 20 trinkets
            for (int j = 0; j < ed.maxheight; j++)
                for (i = 0; i < ed.maxwidth; i++)
                    map.explored[i + (j * ed.maxwidth)] = 1;

            for (int j = 0; j < 20; j++) obj.collect[j] = true;
            game.insecretlab = true;
            map.showteleporters = true;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;

            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            music.play(11);
            graphics.fademode = 4;
            break;
        case 12:
            game.gamestate = GAMEMODE;
            hardreset();
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
            game.startspecial(1);
            game.jumpheld = true;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            call("intermission_1");
            break;
        case 13:
            game.gamestate = GAMEMODE;
            hardreset();
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
            game.startspecial(1);
            game.jumpheld = true;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            call("intermission_1");
            break;
        case 14:
            game.gamestate = GAMEMODE;
            hardreset();
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
            game.startspecial(1);
            game.jumpheld = true;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            call("intermission_1");
            break;
        case 15:
            game.gamestate = GAMEMODE;
            hardreset();
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
            game.startspecial(1);
            game.jumpheld = true;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            call("intermission_1");
            break;
        case 16:
            game.gamestate = GAMEMODE;
            hardreset();
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
            game.startspecial(1);
            game.jumpheld = true;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            call("intermission_2");
            break;
        case 17:
            game.gamestate = GAMEMODE;
            hardreset();
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
            game.startspecial(1);
            game.jumpheld = true;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            call("intermission_2");
            break;
        case 18:
            game.gamestate = GAMEMODE;
            hardreset();
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
            game.startspecial(1);
            game.jumpheld = true;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            call("intermission_2");
            break;
        case 19:
            game.gamestate = GAMEMODE;
            hardreset();
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
            game.startspecial(1);
            game.jumpheld = true;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            call("intermission_2");
            break;
#if !defined(NO_CUSTOM_LEVELS)
        case 20:
            // Level editor
            hardreset();
            ed.reset();
            music.fadeout();

            map.teleporters.clear();
            game.gamestate = EDITORMODE;
            game.jumpheld = true;

            if (graphics.setflipmode) graphics.flipmode = true;  // set flipmode
            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            ed.generatecustomminimap();
            graphics.fademode = 4;
            break;
        case 21:  // play custom level (in editor)
            game.gamestate = GAMEMODE;
            music.fadeout();
            hardreset();
            // If warpdir() is used during playtesting, we need to set it back
            // after!
            for (int j = 0; j < ed.maxheight; j++) {
                for (int i = 0; i < ed.maxwidth; i++) {
                    ed.kludgewarpdir[i + (j * ed.maxwidth)] =
                        ed.level[i + (j * ed.maxwidth)].warpdir;
                }
            }
            game.customstart();
            game.jumpheld = true;

            ed.vceversion = VCEVERSION;
            ed.ghosts.clear();

            map.custommode = true;
            map.customx = 100;
            map.customy = 100;

            if (graphics.setflipmode) graphics.flipmode = true;

            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            if (map.towermode) {
                // Undo player x/y adjustments and realign camera on checkpoint
                map.resetplayer();
                map.realign_tower();  // resetplayer only realigns if room
                                      // differs
            }
            if (ed.levmusic > 0) {
                music.play(ed.levmusic);
            } else {
                music.currentsong = -1;
            }
            break;
        case 22: {  // play custom level (in game)
            // Initilise the level
            // First up, find the start point
            std::string filename = std::string(ed.ListOfMetaData[game.playcustomlevel].filename);
            ed.load(filename);
            ed.findstartpoint();

            game.gamestate = GAMEMODE;
            music.fadeout();
            hardreset();
            game.customstart();
            game.jumpheld = true;

            map.custommodeforreal = true;
            map.custommode = true;
            map.customx = 100;
            map.customy = 100;

            if (graphics.setflipmode) graphics.flipmode = true;

            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);

            ed.generatecustomminimap();
            map.customshowmm = true;
            if (ed.levmusic > 0) {
                music.play(ed.levmusic);
            } else {
                music.currentsong = -1;
            }
            graphics.fademode = 4;
            break;
        }
        case 23: {  // Continue in custom level
                  // Initilise the level
            // First up, find the start point
            std::string filename = std::string(ed.ListOfMetaData[game.playcustomlevel].filename);
            ed.load(filename);
            ed.findstartpoint();

            game.gamestate = GAMEMODE;
            music.fadeout();
            hardreset();
            map.custommodeforreal = true;
            map.custommode = true;
            map.customx = 100;
            map.customy = 100;

            game.customstart();
            game.customloadquick(ed.ListOfMetaData[game.playcustomlevel].filename);
            game.jumpheld = true;
            game.gravitycontrol = game.savegc;

            if (graphics.setflipmode) graphics.flipmode = true;

            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            ed.generatecustomminimap();
            graphics.fademode = 4;
            break;
        }
        case 24: {  // Custom level time trial!
            // Load the level first
            game.incustomtrial = true;
            std::string filename = std::string(ed.ListOfMetaData[game.playcustomlevel].filename);
            ed.load(filename);
            // ...then find the start point
            ed.findstartpoint();

            game.gamestate = GAMEMODE;  // Set the gamemode
            music.fadeout();            // Fade out the music
            hardreset();              // Reset everything!!
            map.custommodeforreal = true;  // Yep, it's technically
            map.custommode = true;         // a custom level

            map.customx = 100;
            map.customy = 100;

            game.customstart();  // I honestly have no idea what this does
            // Actually, the level is already loaded! This is just to be safe...
            game.customloadquick(ed.ListOfMetaData[game.playcustomlevel].filename);
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
            game.state = 0;
            game.deathseq = -1;
            game.lifeseq = 0;

            // set flipmode
            if (graphics.setflipmode) graphics.flipmode = true;

            if (obj.entities.empty()) {
                obj.createentity(game.savex, game.savey, 0,
                                 0);  // In this game, constant, never destroyed
            } else {
                map.resetplayer();
            }
            map.gotoroom(game.saverx, game.savery);
            music.currentsong = -1;
            ed.generatecustomminimap();
            graphics.fademode = 4;
            break;
        }

#endif
        case 100:
            game.savestats();

            SDL_Quit();
            exit(0);
            break;
    }
}

void scriptclass::teleport() {
    // er, ok! Teleport to a new area, so!
    // A general rule of thumb: if you teleport with a companion, get rid of
    // them!
    game.companion = 0;

    i = obj.getplayer();  // less likely to have a serious collision error if
                          // the player is centered
    if (i > -1) {
        obj.entities[i].xp = 150;
        obj.entities[i].yp = 110;
        if (!map.custommode)
            if (game.teleport_to_x == 17 && game.teleport_to_y == 17)
                obj.entities[i].xp = 88;  // prevent falling!
    }

    if (game.teleportscript == "levelonecomplete") {
        game.teleport_to_x = 2;
        game.teleport_to_y = 11;
    } else if (game.teleportscript == "gamecomplete") {
        game.teleport_to_x = 2;
        game.teleport_to_y = 11;
    }

    game.gravitycontrol = 0;
    map.gotoroom(100 + game.teleport_to_x, 100 + game.teleport_to_y);
    j = obj.getteleporter();
    if (j > -1) {
        obj.entities[j].state = 2;
    }
    game.teleport_to_new_area = false;

    game.savepoint = obj.entities[j].para;
    game.savex = obj.entities[j].xp + 44;
    game.savey = obj.entities[j].yp + 44;
    game.savegc = 0;

    game.saverx = game.roomx;
    game.savery = game.roomy;
    int player = obj.getplayer();
    if (player > -1) {
        game.savedir = obj.entities[player].dir;
    }

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
        call(game.teleportscript);
        game.teleportscript = "";
    } else {
        // change music based on location
        if (!map.custommode) {
            if (graphics.setflipmode && game.teleport_to_x == 11 &&
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
            if (graphics.flipmode) {
                graphics.createtextbox(gamesavedtext, -1, 202, 174, 174, 174);
                graphics.textboxtimer(25);
            } else {
                graphics.createtextbox(gamesavedtext, -1, 12, 174, 174, 174);
                graphics.textboxtimer(25);
            }
            if (map.custommodeforreal)
                game.customsavequick(ed.ListOfMetaData[game.playcustomlevel].filename);
            else if (!map.custommode)
                game.savetele();
        }
    }
}

void scriptclass::hardreset() {
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

    game.state = 0;
    game.statedelay = 0;

    game.hascontrol = true;
    game.advancetext = false;

    game.pausescript = false;

    game.flashlight = 0;
    game.screenshake = 0;

    game.activeactivity = -1;
    game.act_fade = 5;

    game.noflip = false;

    game.noenter = false;

    game.infiniflip = false;

    game.nosuicide = false;

    game.onetimescripts.clear();

    // dwgraphicsclass
    graphics.backgrounddrawn = false;
    graphics.textboxremovefast();
    graphics.flipmode = false;  // This will be reset if needs be elsewhere
    graphics.showcutscenebars = false;
    graphics.cutscenebarspos = 0;
    graphics.oldcutscenebarspos = 0;
    graphics.screenbuffer->badSignalEffect = game.fullScreenEffect_badSignal;

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
    map.scrolldir = 0;
    map.customshowmm = true;
    map.dimension = -1;

	map.roomdeaths.clear();
	map.roomdeaths.resize(ed.maxwidth * ed.maxheight);
	map.roomdeathsfinal.clear();
	map.roomdeathsfinal.resize(20 * 20);
	map.explored.clear();
	map.explored.resize(ed.maxwidth * ed.maxheight);
	//entityclass
    obj.nearelephant = false;
    obj.upsetmode = false;
    obj.upset = 0;

    obj.trophytext = 0;
    obj.trophytype = 0;
    obj.altstates = 0;

	obj.flags.clear();
	obj.flags.resize(1000);

    for (i = 0; i < 6; i++) {
        obj.customcrewmoods[i] = 1;
    }

	obj.collect.clear();
	obj.collect.resize(100);
	obj.customcollect.clear();
	obj.customcollect.resize(100);
	i = 100; //previously a for-loop iterating over collect/customcollect set this to 100

    obj.coincollect.clear();
    obj.coincollect.resize(100);
    game.nocoincounter = false;

	int theplayer = obj.getplayer();
	if (theplayer > -1){
		obj.entities[theplayer].tile = 0;
	}

	// Remove duplicate player entities
	for (int i = 0; i < (int) obj.entities.size(); i++)
	{
		if (i != theplayer)
		{
			removeentity_iter(i);
			theplayer--; // just in case indice of player is not 0
		}
	}

    obj.kludgeonetimescript = false;

    // Script Stuff
    position = 0;
    commands.clear();
    scriptdelay = 0;
    scriptname = "";
    running = false;
    nointerrupt = false;
    passive = false;
    variables.clear();
    callbacks.clear();

    scriptrender.clear();
    game.script_images.clear();
    game.script_image_names.clear();

    lua_scripts.clear();

    keepcolor = false;

    // WARNING: Don't reset teleporter locations, at this point we've already
    // loaded the level!
}

void scriptclass::callback(std::string name) {
    if (callbacks.find(name) == callbacks.end() || callbacks[name].empty())
        return;

    callstack.clear();
    load("custom_" + callbacks[name]);
}

void scriptclass::loadcustom(std::string t)
{
  //this magic function breaks down the custom script and turns into real scripting!
  std::string cscriptname="";
  if(t.length()>7){
    cscriptname=t.substr(7, std::string::npos);
  }

  std::string thelabel;
  // Is the script name a label, or a script name + label?
  if (cscriptname.substr(0, 1) == ".") {
    // It's fully a label
    thelabel = cscriptname.substr(1, cscriptname.length()-1);

    cscriptname = scriptname.substr(7, std::string::npos);
    t = "custom_" + cscriptname;
  } else if (cscriptname.find(".") != std::string::npos) {
    // It's a script name concatenated with a label
    int period = cscriptname.find(".");
    thelabel = cscriptname.substr(period+1, std::string::npos);

    // Remove the label from the script name!
    t = t.substr(0, t.find("."));
    cscriptname = cscriptname.substr(0, period);
  }

  labels.clear();
  scriptname = t;
  commands.clear();
  position = 0;
  running = true;
  if (passive)
    scriptdelay = 0;

  std::string tstring;

  // can't use `auto` here :(
  Script* scriptptr = nullptr;
  for (auto& scriptelem : customscripts)
      if (scriptelem.name == cscriptname) {
          scriptptr = &scriptelem;
          break;
      }
  if(scriptptr == nullptr)
    return;

  auto& script_ = *scriptptr;
  auto& lines = script_.contents;

  if (script_.lua) {
      try {
          lua_script::load(t, lines);
      } catch (const std::exception& ex) {
          handle_exception(ex);
      }
      return;
  }

  //Ok, we've got the relavent script segment, we do a pass to assess it, then run it!
  int customcutscenemode=0;
  for(auto& line : lines){
    tokenize(line);
    if(words[0] == "say" || words[0] == "sayquiet" || words[0] == "csay" || words[0] == "csayquiet"){
      customcutscenemode=1;
    }else if(words[0] == "reply" || words[0] == "replyquiet"){
      customcutscenemode=1;
    }else if(words[0] == "noautobars"){
      customcutscenemode=0;
      break;
    }
  }

  if(customcutscenemode==1){
    add("cutscene()");
    add("untilbars()");
  }
  int customtextmode=0;
  int speakermode=0; //0, terminal, numbers for crew
  int squeakmode=0;//default on
  //Now run the script
  for(size_t i=0; i<lines.size(); i++){
    words[0]="nothing"; //Default!
    words[1]="unused"; //Default!
    tokenize(lines[i]);
    std::transform(words[0].begin(), words[0].end(), words[0].begin(), ::tolower);
    if (words[0] != "flash" && words[1] == "unused") {
      words[1] = "1";
    }
    if (words[0] == "music"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      if(words[1]=="0"){
        tstring="stopmusic()";
      }else{
        if(words[1]=="11"){ tstring="play(14)";
        }else if(words[1]=="10"){ tstring="play(13)";
        }else if(words[1]=="9"){ tstring="play(12)";
        }else if(words[1]=="8"){ tstring="play(11)";
        }else if(words[1]=="7"){ tstring="play(10)";
        }else if(words[1]=="6"){ tstring="play(8)";
        }else if(words[1]=="5"){ tstring="play(6)";
        }else { tstring="play("+words[1]+")"; }
      }
      add(tstring);
    }else if(words[0] == "playremix"){
      add("play(15)");
    }else if(words[0] == "flash"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      if (words[1] != "unused" && words[1] != "" && words[1] != "0") {
        add("flash("+words[1]+")");
      } else {
        add("flash(5)");
        add("shake(20)");
        add("playef(9)");
      }
    }else if(words[0] == "sad" || words[0] == "cry"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      if(words[1]=="player"){
        add("changemood(player,1)");
      }else if(words[1]=="cyan" || words[1]=="viridian" || words[1]=="1"){
        add("changecustommood(customcyan,1)");
      }else if(words[1]=="purple" || words[1]=="violet" || words[1]=="pink" || words[1]=="2"){
        add("changecustommood(purple,1)");
      }else if(words[1]=="yellow" || words[1]=="vitellary" || words[1]=="3"){
        add("changecustommood(yellow,1)");
      }else if(words[1]=="red" || words[1]=="vermilion" || words[1]=="4"){
        add("changecustommood(red,1)");
      }else if(words[1]=="green" || words[1]=="verdigris" || words[1]=="5"){
        add("changecustommood(green,1)");
      }else if(words[1]=="blue" || words[1]=="victoria" || words[1]=="6"){
        add("changecustommood(blue,1)");
      }else if(words[1]=="all" || words[1]=="everybody" || words[1]=="everyone"){
        add("changemood(player,1)");
        add("changecustommood(customcyan,1)");
        add("changecustommood(purple,1)");
        add("changecustommood(yellow,1)");
        add("changecustommood(red,1)");
        add("changecustommood(green,1)");
        add("changecustommood(blue,1)");
      }else{
        add("changemood(player,1)");
      }
      if(squeakmode==0) add("squeak(cry)");
    }else if(words[0] == "happy"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      if(words[1]=="player"){
        add("changemood(player,0)");
        if(squeakmode==0) add("squeak(player)");
      }else if(words[1]=="cyan" || words[1]=="viridian" || words[1]=="1"){
        add("changecustommood(customcyan,0)");
        if(squeakmode==0) add("squeak(player)");
      }else if(words[1]=="purple" || words[1]=="violet" || words[1]=="pink" || words[1]=="2"){
        add("changecustommood(purple,0)");
        if(squeakmode==0) add("squeak(purple)");
      }else if(words[1]=="yellow" || words[1]=="vitellary" || words[1]=="3"){
        add("changecustommood(yellow,0)");
        if(squeakmode==0) add("squeak(yellow)");
      }else if(words[1]=="red" || words[1]=="vermilion" || words[1]=="4"){
        add("changecustommood(red,0)");
        if(squeakmode==0) add("squeak(red)");
      }else if(words[1]=="green" || words[1]=="verdigris" || words[1]=="5"){
        add("changecustommood(green,0)");
        if(squeakmode==0) add("squeak(green)");
      }else if(words[1]=="blue" || words[1]=="victoria" || words[1]=="6"){
        add("changecustommood(blue,0)");
        if(squeakmode==0) add("squeak(blue)");
      }else if(words[1]=="all" || words[1]=="everybody" || words[1]=="everyone"){
        add("changemood(player,0)");
        add("changecustommood(customcyan,0)");
        add("changecustommood(purple,0)");
        add("changecustommood(yellow,0)");
        add("changecustommood(red,0)");
        add("changecustommood(green,0)");
        add("changecustommood(blue,0)");
      }else{
        add("changemood(player,0)");
        if(squeakmode==0) add("squeak(player)");
      }
    }else if(words[0] == "squeak"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      if(words[1]=="player"){
        add("squeak(player)");
      }else if(words[1]=="cyan" || words[1]=="viridian" || words[1]=="1"){
        add("squeak(player)");
      }else if(words[1]=="purple" || words[1]=="violet" || words[1]=="pink" || words[1]=="2"){
        add("squeak(purple)");
      }else if(words[1]=="yellow" || words[1]=="vitellary" || words[1]=="3"){
        add("squeak(yellow)");
      }else if(words[1]=="red" || words[1]=="vermilion" || words[1]=="4"){
        add("squeak(red)");
      }else if(words[1]=="green" || words[1]=="verdigris" || words[1]=="5"){
        add("squeak(green)");
      }else if(words[1]=="blue" || words[1]=="victoria" || words[1]=="6"){
        add("squeak(blue)");
      }else if(words[1]=="cry" || words[1]=="sad"){
        add("squeak(cry)");
      }else if(words[1]=="on"){
        squeakmode=0;
      }else if(words[1]=="off"){
        squeakmode=1;
      }else{
        add(lines[i]);
      }
    }else if(words[0] == "delay"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      add(lines[i]);
    }else if(words[0] == "flag"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      add(lines[i]);
    }else if(words[0] == "map"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      add("custom"+lines[i]);
    }else if(words[0] == "warpdir"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      add(lines[i]);
    }else if(words[0] == "ifwarp"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      add(lines[i]);
    }else if(words[0] == "iftrinkets"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      add("custom"+lines[i]);
    }else if(words[0] == "ifflag"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      add("custom"+lines[i]);
    }else if(words[0] == "iftrinketsless"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      add("custom"+lines[i]);
    }else if(words[0] == "destroy"){
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      add(lines[i]);
    }else if(words[0] == "speaker"){
      speakermode=0;
      if(words[1]=="gray" || words[1]=="grey" || words[1]=="terminal" || words[1]=="0") speakermode=0;
      if(words[1]=="cyan" || words[1]=="viridian" || words[1]=="player" || words[1]=="1") speakermode=1;
      if(words[1]=="purple" || words[1]=="violet" || words[1]=="pink" || words[1]=="2") speakermode=2;
      if(words[1]=="yellow" || words[1]=="vitellary" || words[1]=="3") speakermode=3;
      if(words[1]=="red" || words[1]=="vermilion" || words[1]=="4") speakermode=4;
      if(words[1]=="green" || words[1]=="verdigris" || words[1]=="5") speakermode=5;
      if(words[1]=="blue" || words[1]=="victoria" || words[1]=="6") speakermode=6;
    }else if(words[0] == "say" || words[0] == "sayquiet" || words[0] == "csay" || words[0] == "csayquiet"){
      //Speakers!
      if(words[2]=="terminal" || words[2]=="gray" || words[2]=="grey" || words[2]=="0") speakermode=0;
      if(words[2]=="cyan" || words[2]=="viridian" || words[2]=="player" || words[2]=="1") speakermode=1;
      if(words[2]=="purple" || words[2]=="violet" || words[2]=="pink" || words[2]=="2") speakermode=2;
      if(words[2]=="yellow" || words[2]=="vitellary" || words[2]=="3") speakermode=3;
      if(words[2]=="red" || words[2]=="vermilion" || words[2]=="4") speakermode=4;
      if(words[2]=="green" || words[2]=="verdigris" || words[2]=="5") speakermode=5;
      if(words[2]=="blue" || words[2]=="victoria" || words[2]=="6") speakermode=6;
      switch(speakermode){
        case 0:
          if(squeakmode==0 && words[0] != "sayquiet" && words[0] != "csayquiet") add("squeak(terminal)");
          add("text(gray,0,114,"+words[1]+")");
        break;
        case 1: //NOT THE PLAYER
          if(squeakmode==0 && words[0] != "sayquiet" && words[0] != "csayquiet") add("squeak(cyan)");
          add("text(cyan,0,0,"+words[1]+")");
        break;
        case 2:
          if(squeakmode==0 && words[0] != "sayquiet" && words[0] != "csayquiet") add("squeak(purple)");
          add("text(purple,0,0,"+words[1]+")");
        break;
        case 3:
          if(squeakmode==0 && words[0] != "sayquiet" && words[0] != "csayquiet") add("squeak(yellow)");
          add("text(yellow,0,0,"+words[1]+")");
        break;
        case 4:
          if(squeakmode==0 && words[0] != "sayquiet" && words[0] != "csayquiet") add("squeak(red)");
          add("text(red,0,0,"+words[1]+")");
        break;
        case 5:
          if(squeakmode==0 && words[0] != "sayquiet" && words[0] != "csayquiet") add("squeak(green)");
          add("text(green,0,0,"+words[1]+")");
        break;
        case 6:
          if(squeakmode==0 && words[0] != "sayquiet" && words[0] != "csayquiet") add("squeak(blue)");
          add("text(blue,0,0,"+words[1]+")");
        break;
      }
      int ti = 1;
      if (!words[1].empty()) {
          ti = atoi(words[1].c_str());
      }
      int nti = ti>=0 && ti<=50 ? ti : 1;
      for(int ti2=0; ti2<nti; ti2++){
        i++;
        if(i < lines.size()){
          add(lines[i]);
        }
      }

      std::string addthis;
      if (words[0] != "csay" && words[0] != "csayquiet")
        addthis += "custom";
      addthis += "position(";
      switch(speakermode){
        case 0: addthis += "center"; break;
        case 1: addthis += "cyan,above"; break;
        case 2: addthis += "purple,above"; break;
        case 3: addthis += "yellow,above"; break;
        case 4: addthis += "red,above"; break;
        case 5: addthis += "green,above"; break;
        case 6: addthis += "blue,above"; break;
      }
      addthis += ")";
      add(addthis);
      add("speak_active");
      customtextmode=1;
    }else if(words[0] == "reply" || words[0] == "replyquiet"){
      //For this version, terminal only
      if(squeakmode==0  && words[0] != "replyquiet") add("squeak(player)");
      add("text(cyan,0,0,"+words[1]+")");

      int ti = 1;
      if (!words[1].empty()) {
          ti = atoi(words[1].c_str());
      }
      int nti = ti>=0 && ti<=50 ? ti : 1;
      for(int ti2=0; ti2<nti; ti2++){
        i++;
        if(i < lines.size()){
          add(lines[i]);
        }
      }
      add("position(player,above)");
      add("speak_active");
      customtextmode=1;
    }else{
      if(customtextmode==1){ add("endtext"); customtextmode=0;}
      if (words[0] == "setroomname"
      || words[0] == "drawtext"
      || words[0] == "createroomtext"
      || words[0] == "customactivityzone"
      || ((words[0] == "addvar" || words[0] == "setvar" || words[0] == "getvar") && words[2] == "")) {
        // Don't parse the next line if it is a textbox-like line
        add(lines[i]);
        i++;
      } else if (words[0] == "text") {
        // Ok, it's actually a text box with potentially more than one line
        int lines_;
        if (words[5] != "" && words[6] != "")
          // RGB version, 3 color args, 6 args
          lines_ = ss_toi(words[6]);
        else
          // Predefined color version, 1 color arg, 4 args
          lines_ = ss_toi(words[4]);
        for (int c = 0; c < lines_; c++) {
          if (i < lines.size())
            add(lines[i]);
          i++;
        }
      }
      if (IS_VCE_LEVEL && i < lines.size()) // Don't call one-command internal scripts twice in vanilla levels
          add(lines[i]);

      // Is this a label?
      if (words[0].length() > 1 && words[0].substr(0, 1) == ".") {
        std::string thislabel = words[0].substr(1, words[0].length()-1);

        // Important - use `commands.size()` instead of `i`
        // The former is the internal script's position which is what we want,
        // and the latter is the simplified script's position
        labels[thislabel] = commands.size();
      }
    }
  }

  if(customtextmode==1){ add("endtext"); customtextmode=0;}
  if(customcutscenemode==1){
    add("endcutscene()");
    add("untilbars()");
  }

  if (!thelabel.empty()) {
    if (labels.find(thelabel) != labels.end())
      position = labels[thelabel];
    else
      running = false;
  }
}

bool scriptclass::is_running() {
    return !lua_scripts.empty() || running;
}
