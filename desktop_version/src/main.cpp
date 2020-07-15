#include <SDL.h>
#include <ctime>
#include <chrono>
#include "SoundSystem.h"

#include "UtilityClass.h"
#include "Utilities.h"
#include "Game.h"
#include "Graphics.h"
#include "KeyPoll.h"
#include "Render.h"

#include "Tower.h"
#include "WarpClass.h"
#include "Labclass.h"
#include "Finalclass.h"
#include "Map.h"

#include "Screen.h"

#include "Script.h"

#include "Logic.h"

#include "Input.h"
#include "editor.h"
#include "preloader.h"

#include "FileSystemUtils.h"
#include "Network.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__)
#include <unistd.h>
#endif

#include "Maths.h"
#include <physfs.h>

#ifdef __MINGW32__
#include <mingw.thread.h>
#include <mingw.condition_variable.h>
#include <mingw.mutex.h>
#elif !defined(__APPLE__)
#include <thread>
#include <condition_variable>
#include <mutex>
#endif

#include <exception>

using namespace std::literals::chrono_literals;

scriptclass script;
#if !defined(NO_CUSTOM_LEVELS)
std::vector<edentities> edentity;
editorclass ed;
#endif

bool startinplaytest = false;
bool savefileplaytest = false;
int savex = 0;
int savey = 0;
int saverx = 0;
int savery = 0;
int savegc = 0;
int savemusic = 0;

std::string playtestname;

UtilityClass help;
Graphics graphics;
musicclass music;
Game game;
KeyPoll key;
mapclass map;
entityclass obj;
Screen gameScreen;

#ifdef __SWITCH__
FILE* logger;
#endif

extern const char* git_rev;
bool headless = false;

int main(int argc, char *argv[])
{
    seed_xoshiro_64(std::time(nullptr));

    bool syslog = log_default();

    char* assetsPath = NULL;
    char* baseDir = NULL;

    for (int i = 1; i < argc; ++i) {
        if (strcmp(argv[i], "--quiet") == 0) {
            game.quiet = true;
        }
        if (strcmp(argv[i], "--version") == 0) {
            puts("VVVVVV-CE");
            puts("Version c1.0-pre1");
            printf("Built from commit %s\n", git_rev);
            return 0;
        }
        if (strcmp(argv[i], "--headless") == 0) {
            headless = true;
        }
        if (strcmp(argv[i], "--syslog") == 0) {
            syslog = true;
        }
        if (strcmp(argv[i], "--no-syslog") == 0) {
            syslog = false;
        }
        if ((std::string(argv[i]) == "-playing") || (std::string(argv[i]) == "-p")) {
            if (i + 1 < argc) {
                startinplaytest = true;
                i++;
                playtestname = std::string("levels/");
                playtestname.append(argv[i]);
                playtestname.append(std::string(".vvvvvv"));
            } else {
                printf("-playing option requires one argument.\n");
                return 1;
            }
        }
        if (strcmp(argv[i], "-playx") == 0 ||
                strcmp(argv[i], "-playy") == 0 ||
                strcmp(argv[i], "-playrx") == 0 ||
                strcmp(argv[i], "-playry") == 0 ||
                strcmp(argv[i], "-playgc") == 0 ||
                strcmp(argv[i], "-playmusic") == 0) {
            if (i + 1 < argc) {
                savefileplaytest = true;
                auto v = std::atoi(argv[i+1]);
                if (strcmp(argv[i], "-playx") == 0) savex = v;
                else if (strcmp(argv[i], "-playy") == 0) savey = v;
                else if (strcmp(argv[i], "-playrx") == 0) saverx = v;
                else if (strcmp(argv[i], "-playry") == 0) savery = v;
                else if (strcmp(argv[i], "-playgc") == 0) savegc = v;
                else if (strcmp(argv[i], "-playmusic") == 0) savemusic = v;
                i++;
            } else {
                printf("-playing option requires one argument.\n");
                return 1;
            }
        }
        if (std::string(argv[i]) == "-renderer") {
            i++;
            SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, argv[i], SDL_HINT_OVERRIDE);
        } else if (strcmp(argv[i], "-basedir") == 0) {
            ++i;
            baseDir = argv[i];
        } else if (strcmp(argv[i], "-assets") == 0) {
            ++i;
            assetsPath = argv[i];
        }
    }

    if (syslog) {
        log_init();
    }

    if (!game.quiet) {
        printf("\t\t\n");
        printf("\t\t\n");
        printf("\t\t       VVVVVV\n");
        printf("\t\t\n");
        printf("\t\t\n");
        printf("\t\t  8888888888888888  \n");
        printf("\t\t88888888888888888888\n");
        printf("\t\t888888    8888    88\n");
        printf("\t\t888888    8888    88\n");
        printf("\t\t88888888888888888888\n");
        printf("\t\t88888888888888888888\n");
        printf("\t\t888888            88\n");
        printf("\t\t88888888        8888\n");
        printf("\t\t  8888888888888888  \n");
        printf("\t\t      88888888      \n");
        printf("\t\t  8888888888888888  \n");
        printf("\t\t88888888888888888888\n");
        printf("\t\t88888888888888888888\n");
        printf("\t\t88888888888888888888\n");
        printf("\t\t8888  88888888  8888\n");
        printf("\t\t8888  88888888  8888\n");
        printf("\t\t    888888888888    \n");
        printf("\t\t    8888    8888    \n");
        printf("\t\t  888888    888888  \n");
        printf("\t\t  888888    888888  \n");
        printf("\t\t  888888    888888  \n");
        printf("\t\t\n");
        printf("\t\t\n");
    }

    if(!FILESYSTEM_initCore(argv[0], baseDir, assetsPath))
    {
        return 1;
    }

    SDL_SetMainReady();
    if (SDL_Init(
            SDL_INIT_VIDEO |
            SDL_INIT_AUDIO |
            SDL_INIT_JOYSTICK |
            SDL_INIT_GAMECONTROLLER
        ) < 0) {
        fprintf(stderr, "SDL_Init failed: %s\n", SDL_GetError());
    }

    game.init();
    graphics.init();
    gameScreen.init();
    graphics.screenbuffer = &gameScreen;

    game.loadstats();

    gameScreen.headless = headless;

    const SDL_PixelFormat* fmt = gameScreen.GetFormat();
    graphics.backBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_SetSurfaceBlendMode(graphics.backBuffer, SDL_BLENDMODE_NONE);
    graphics.footerbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 10, fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_SetSurfaceBlendMode(graphics.footerbuffer, SDL_BLENDMODE_BLEND);
    SDL_SetSurfaceAlphaMod(graphics.footerbuffer, 127);
    FillRect(graphics.footerbuffer, SDL_MapRGB(fmt, 0, 0, 0));

    graphics.ghostbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 240, fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_SetSurfaceBlendMode(graphics.ghostbuffer, SDL_BLENDMODE_BLEND);
    SDL_SetSurfaceAlphaMod(graphics.ghostbuffer, 127);

    graphics.Makebfont();

    graphics.foregroundBuffer =  SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask  );
    SDL_SetSurfaceBlendMode(graphics.foregroundBuffer, SDL_BLENDMODE_NONE);

    graphics.menubuffer = SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask );
    SDL_SetSurfaceBlendMode(graphics.menubuffer, SDL_BLENDMODE_NONE);

    graphics.towerbuffer =  SDL_CreateRGBSurface(SDL_SWSURFACE ,320+16 ,240+16 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask  );
    SDL_SetSurfaceBlendMode(graphics.towerbuffer, SDL_BLENDMODE_NONE);
    graphics.towerbuffer_lerp = SDL_CreateRGBSurface(SDL_SWSURFACE, 320+16, 240+16, fmt->BitsPerPixel, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_SetSurfaceBlendMode(graphics.towerbuffer, SDL_BLENDMODE_NONE);

    graphics.tempBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask  );
    SDL_SetSurfaceBlendMode(graphics.tempBuffer, SDL_BLENDMODE_NONE);

    game.infocus = true;
    key.isActive = true;
    game.gametimer = 0;
    obj.init();
#if !defined(__APPLE__)
    std::condition_variable timeout;
    std::mutex mutex;
    std::thread init([&]() {
        auto start = std::chrono::steady_clock::now();
#endif
        if(!FILESYSTEM_init(argv[0], baseDir, assetsPath)) {
            exit(1);
        }
        pre_fakepercent.store(50);
        music.init();
        pre_fakepercent.store(80);
        graphics.reloadresources(true);
        pre_fakepercent.store(100);
#if !defined(__APPLE__)
        auto end = std::chrono::steady_clock::now();
        if (end - start < 1s) {
            pre_quickend.store(true);
        }
        std::unique_lock<std::mutex> lock(mutex);
        lock.unlock();
        timeout.notify_all();
    });

    std::unique_lock<std::mutex> uniq(mutex);
    timeout.wait_for(uniq, 1s);
    uniq.unlock();
    preloaderloop();
    init.join();
#endif

    if (!game.quiet) NETWORK_init(); // FIXME: this is probably bad

    game.infocus = true;
    game.gamestate = TITLEMODE;

    game.menustart = false;
    game.mainmenu = 0;

    map.ypos = (700-29) * 8;
    map.bypos = map.ypos / 2;

    //Moved screensetting init here from main menu V2.1
    graphics.processVsync();

    if (game.skipfakeload)
        game.gamestate = TITLEMODE;
                if(game.usingmmmmmm==0) music.usingmmmmmm=false;
                if(game.usingmmmmmm==1) music.usingmmmmmm=true;
    if (game.slowdown == 0) game.slowdown = 30;

    switch(game.slowdown){
    case 30: game.gameframerate=34; break;
    case 24: game.gameframerate=41; break;
    case 18: game.gameframerate=55; break;
    case 12: game.gameframerate=83; break;
    default: game.gameframerate=34; break;
    }

                //Check to see if you've already unlocked some achievements here from before the update
                if (game.swnbestrank > 0){
                if(game.swnbestrank >= 1) NETWORK_unlockAchievement("vvvvvvsupgrav5");
                        if(game.swnbestrank >= 2) NETWORK_unlockAchievement("vvvvvvsupgrav10");
                        if(game.swnbestrank >= 3) NETWORK_unlockAchievement("vvvvvvsupgrav15");
                        if(game.swnbestrank >= 4) NETWORK_unlockAchievement("vvvvvvsupgrav20");
                        if(game.swnbestrank >= 5) NETWORK_unlockAchievement("vvvvvvsupgrav30");
                        if(game.swnbestrank >= 6) NETWORK_unlockAchievement("vvvvvvsupgrav60");
                }

                if(game.unlock[5]) NETWORK_unlockAchievement("vvvvvvgamecomplete");
                if(game.unlock[19]) NETWORK_unlockAchievement("vvvvvvgamecompleteflip");
                if(game.unlock[20]) NETWORK_unlockAchievement("vvvvvvmaster");

                if (game.bestgamedeaths > -1) {
                        if (game.bestgamedeaths <= 500) {
                                NETWORK_unlockAchievement("vvvvvvcomplete500");
                        }
                        if (game.bestgamedeaths <= 250) {
                                NETWORK_unlockAchievement("vvvvvvcomplete250");
                        }
                        if (game.bestgamedeaths <= 100) {
                                NETWORK_unlockAchievement("vvvvvvcomplete100");
                        }
                        if (game.bestgamedeaths <= 50) {
                                NETWORK_unlockAchievement("vvvvvvcomplete50");
                        }
                }

                if(game.bestrank[0]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_station1_fixed");
                if(game.bestrank[1]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_lab_fixed");
                if(game.bestrank[2]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_tower_fixed");
                if(game.bestrank[3]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_station2_fixed");
                if(game.bestrank[4]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_warp_fixed");
                if(game.bestrank[5]>=3) NETWORK_unlockAchievement("vvvvvvtimetrial_final_fixed");

#if !defined(NO_CUSTOM_LEVELS)
    if (startinplaytest) {
        game.levelpage = 0;
        game.playcustomlevel = 0;

        ed.directoryList = { playtestname };

        LevelMetaData meta;
        if (ed.getLevelMetaData(playtestname, meta)) {
            ed.ListOfMetaData = { meta };
        } else {
            ed.loadZips();

            ed.directoryList = { playtestname };
            if (ed.getLevelMetaData(playtestname, meta)) {
                ed.ListOfMetaData = { meta };
            } else {
                printf("Level not found\n");
                return 1;
            }
        }

        game.loadcustomlevelstats();

        game.customleveltitle=ed.ListOfMetaData[game.playcustomlevel].title;
        game.customlevelfilename=ed.ListOfMetaData[game.playcustomlevel].filename;
        if (savefileplaytest) {
            game.playx = savex;
            game.playy = savey;
            game.playrx = saverx;
            game.playry = savery;
            game.playgc = savegc;
            game.cliplaytest = true;
            music.play(savemusic);
            script.startgamemode(23);
        } else {
            script.startgamemode(22);
        }
        graphics.fademode = 0;

    }

    volatile Uint32 time = 0;
    volatile Uint32 timePrev = 0;
    volatile Uint32 accumulator = 0;
    volatile Uint32 f_time = 0;
    volatile Uint32 f_timePrev = 0;
    volatile Uint32 f_accumulator = 0;

#ifdef VCE_DEBUG
    auto last_gamestate = game.gamestate;
#endif

    while(!key.quitProgram)
    {
#ifdef VCE_DEBUG
        if (last_gamestate != game.gamestate) {
            printf("gamestate %i -> %i\n", last_gamestate, game.gamestate);
            last_gamestate = game.gamestate;
        }
#endif

        f_timePrev = f_time;
        f_time = SDL_GetTicks();
        const float f_rawdeltatime = static_cast<float>(f_time - f_timePrev);
        if (!game.over30mode)
        {
            f_accumulator += f_rawdeltatime;
        }

        while ((game.over30mode || f_accumulator >= 34) && !key.quitProgram)
        {
            if (game.over30mode)
            {
                f_accumulator = 0;
            }
            else
            {
                f_accumulator = fmodf(f_accumulator, 34);
            }

            timePrev = time;
            time = SDL_GetTicks();

            game.infocus = key.isActive;

            // Update network per frame.
            NETWORK_update();

            //timestep limit to 30
            const float rawdeltatime = static_cast<float>(time - timePrev);
            accumulator += rawdeltatime;

            Uint32 timesteplimit;
            if (game.gamestate == EDITORMODE)
            {
                timesteplimit = 24;
            }
            else if (game.gamestate == GAMEMODE || game.gamestate == MAPMODE || game.gamestate == TELEPORTERMODE)
            {
                timesteplimit = game.gameframerate;
            }
            else
            {
                timesteplimit = 34;
            }

            while (accumulator >= timesteplimit)
            {
                accumulator = fmodf(accumulator, timesteplimit);

                key.Poll();
                if(key.toggleFullscreen)
                {
                    if(!gameScreen.isWindowed)
                    {
                        SDL_ShowCursor(SDL_DISABLE);
                        SDL_ShowCursor(SDL_ENABLE);
                    }
                    else
                    {
                        SDL_ShowCursor(SDL_ENABLE);
                    }


                    if(game.gamestate == EDITORMODE)
                    {
                        SDL_ShowCursor(SDL_ENABLE);
                    }

                    gameScreen.toggleFullScreen();
                    game.fullscreen = !game.fullscreen;
                    key.toggleFullscreen = false;

                    key.keymap.clear(); //we lost the input due to a new window.
                    game.press_left = false;
                    game.press_right = false;
                    game.press_action = true;
                    game.press_map = false;
                }

                if(!game.infocus)
                {
                    Mix_Pause(-1);
                    Mix_PauseMusic();
                    if (!game.blackout)
                    {
                        FillRect(graphics.backBuffer, 0x00000000);
                        graphics.bprint(5, 110, "Game paused", 196 - help.glow, 255 - help.glow, 196 - help.glow, true);
                        graphics.bprint(5, 120, "[click to resume]", 196 - help.glow, 255 - help.glow, 196 - help.glow, true);
                        graphics.bprint(5, 220, "Press M to mute in game", 164 - help.glow, 196 - help.glow, 164 - help.glow, true);
                        graphics.bprint(5, 230, "Press N to mute music only", 164 - help.glow, 196 - help.glow, 164 - help.glow, true);
                    }
                    graphics.render();
                    gameScreen.FlipScreen();
                    //We are minimised, so lets put a bit of a delay to save CPU
                    SDL_Delay(100);
                }
                else
                {
                    Mix_Resume(-1);
                    Mix_ResumeMusic();
                    game.gametimer++;
                    graphics.cutscenebarstimer();

                    switch(game.gamestate)
                    {
                    case PRELOADER:
                        preloaderlogic();
                        break;
#if !defined(NO_CUSTOM_LEVELS)
                    case EDITORMODE:
                        graphics.flipmode = false;
                        //Input
                        editorinput();
                        ////Logic
                        editorlogic();
                        break;
#endif
                    case TITLEMODE:
                        //Input
                        changeloginput();
                        titleinput();
                        ////Logic
                        titlelogic();
                        break;
                    case GAMEMODE:
                        if (script.is_running()) {
                            script.run();
                        }

                        //Update old positions of entities - has to be done BEFORE gameinput!
                        for (size_t i = 0; i < obj.entities.size(); i++)
                        {
                            obj.entities[i].oldxp = obj.entities[i].xp;
                            obj.entities[i].oldyp = obj.entities[i].yp;
                        }

                        gameinput();
                        gamelogic();
                        break;
                    case MAPMODE:
                        mapinput();
                        maplogic();
                        break;
                    case TELEPORTERMODE:
                        if(game.useteleporter)
                        {
                            teleporterinput();
                        }
                        else
                        {
                            if (script.is_running()) {
                                script.run();
                            }
                            gameinput();
                        }
                        maplogic();
                        break;
                    case GAMECOMPLETE:
                        //Input
                        gamecompleteinput();
                        //Logic
                        gamecompletelogic();
                        break;
                    case GAMECOMPLETE2:
                        //Input
                        gamecompleteinput2();
                        //Logic
                        gamecompletelogic2();
                        break;
                    case CLICKTOSTART:
                        break;
                    default:
                        break;
                    }

                }

                //Screen effects timers
                if (game.infocus && game.flashlight > 0)
                {
                    game.flashlight--;
                }
                if (game.infocus && game.screenshake > 0)
                {
                    game.screenshake--;
                    graphics.updatescreenshake();
                }

                if (graphics.screenbuffer->badSignalEffect)
                {
                    UpdateFilter();
                }

                //We did editorinput, now it's safe to turn this off
                key.linealreadyemptykludge = false;

                if (game.savemystats)
                {
                    game.savemystats = false;
                    game.savestats();
                }

                //Mute button
#if !defined(NO_CUSTOM_LEVELS)
                bool inEditor = ed.textentry || ed.textmod || ed.scripthelppage == 1;
#else
                bool inEditor = false;
#endif
                if (key.isDown(KEYBOARD_m) && game.mutebutton<=0 && !inEditor)
                {
                    game.mutebutton = 8;
                    if (game.muted)
                    {
                        game.muted = false;
                    }
                    else
                    {
                        game.muted = true;
                    }
                }
                if(game.mutebutton>0)
                {
                    game.mutebutton--;
                }

                if (key.isDown(KEYBOARD_n) && game.musicmutebutton <= 0 && !inEditor)
                {
                    game.musicmutebutton = 8;
                    game.musicmuted = !game.musicmuted;
                }
                if (game.musicmutebutton > 0)
                {
                    game.musicmutebutton--;
                }

                if (game.muted)
                {
                    Mix_VolumeMusic(0) ;
                    Mix_Volume(-1,0);
                }
                else
                {
                    Mix_Volume(-1,MIX_MAX_VOLUME);

                    if (game.musicmuted || game.completestop)
                    {
                        Mix_VolumeMusic(0);
                    }
                    else
                    {
                        Mix_VolumeMusic(MIX_MAX_VOLUME);
                    }
                }

                if (key.resetWindow)
                {
                    key.resetWindow = false;
                    gameScreen.ResizeScreen(-1, -1);
                }

                music.processmusic();
                graphics.processfade();
                game.gameclock();
            }
            const float alpha = static_cast<float>(accumulator) / timesteplimit;
            graphics.alpha = alpha;

            if (game.infocus)
            {
                switch (game.gamestate)
                {
                case PRELOADER:
                    preloaderrender();
                    break;
                case EDITORMODE:
                    graphics.flipmode = false;
                    editorrender();
                    break;
                case TITLEMODE:
                    titlerender();
                    break;
                case GAMEMODE:
                    gamerender();
                    break;
                case MAPMODE:
                    maprender();
                    break;
                case TELEPORTERMODE:
                    teleporterrender();
                    break;
                case GAMECOMPLETE:
                    gamecompleterender();
                    break;
                case GAMECOMPLETE2:
                    gamecompleterender2();
                    break;
                case CLICKTOSTART:
                    help.updateglow();
                    break;
                }
                gameScreen.FlipScreen();
            }
        }
    }



    log_close();

    //Quit SDL
    game.savestats();
    NETWORK_shutdown();
    SDL_Quit();
    FILESYSTEM_deinit();
#endif

    return 0;
}
