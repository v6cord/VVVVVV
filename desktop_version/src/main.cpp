#include <SDL.h>
#include <thread>
#include <atomic>
#include "SoundSystem.h"

#include "UtilityClass.h"
#include "Game.h"
#include "Graphics.h"
#include "KeyPoll.h"
#include "titlerender.h"

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
#include <string.h>

scriptclass script;
growing_vector<edentities> edentity;
editorclass ed;

bool startinplaytest = false;
bool savefileplaytest = false;
int savex = 0;
int savey = 0;
int saverx = 0;
int savery = 0;
int savegc = 0;
int savemusic = 0;

std::string playtestname;

int main(int argc, char *argv[])
{
    if(!FILESYSTEM_init(argv[0]))
    {
        return 1;
    }
    SDL_Init(
        SDL_INIT_VIDEO |
        SDL_INIT_AUDIO |
        SDL_INIT_JOYSTICK |
        SDL_INIT_GAMECONTROLLER
    );

    for (int i = 1; i < argc; ++i) {
        if ((std::string(argv[i]) == "--playing") || (std::string(argv[i]) == "-p")) {
            if (i + 1 < argc) {
                startinplaytest = true;
                i++;
                playtestname = std::string("levels/");
                playtestname.append(argv[i]);
                playtestname.append(std::string(".vvvvvv"));
            } else {
                printf("--playing option requires one argument.\n");
                return 1;
            }
        }
        if (strcmp(argv[i], "--playx") == 0 ||
                strcmp(argv[i], "--playy") == 0 ||
                strcmp(argv[i], "--playrx") == 0 ||
                strcmp(argv[i], "--playry") == 0 ||
                strcmp(argv[i], "--playgc") == 0 ||
                strcmp(argv[i], "--playmusic") == 0) {
            if (i + 1 < argc) {
                savefileplaytest = true;
                auto v = std::atoi(argv[i+1]);
                if (strcmp(argv[i], "--playx") == 0) savex = v;
                else if (strcmp(argv[i], "--playy") == 0) savey = v;
                else if (strcmp(argv[i], "--playrx") == 0) saverx = v;
                else if (strcmp(argv[i], "--playry") == 0) savery = v;
                else if (strcmp(argv[i], "--playgc") == 0) savegc = v;
                else if (strcmp(argv[i], "--playmusic") == 0) savemusic = v;
                i++;
            } else {
                printf("--playing option requires one argument.\n");
                return 1;
            }
        }
        if (std::string(argv[i]) == "-renderer") {
            SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, argv[2], SDL_HINT_OVERRIDE);
        }
    }

    /*if (argc > 2 && strcmp(argv[1], "-renderer") == 0)
    {
        SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, argv[2], SDL_HINT_OVERRIDE);
    }*/

    NETWORK_init();

    Screen gameScreen;

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

    //Set up screen




    UtilityClass help;
    // Load Ini


    Graphics graphics;



    musicclass music;
    Game game;
    game.infocus = true;

    graphics.MakeTileArray();
    graphics.MakeSpriteArray();
    graphics.maketelearray();


    graphics.images.push_back(graphics.grphx.im_image0);
    graphics.images.push_back(graphics.grphx.im_image1);
    graphics.images.push_back(graphics.grphx.im_image2);
    graphics.images.push_back(graphics.grphx.im_image3);
    graphics.images.push_back(graphics.grphx.im_image4);
    graphics.images.push_back(graphics.grphx.im_image5);
    graphics.images.push_back(graphics.grphx.im_image6);

    graphics.images.push_back(graphics.grphx.im_image7);
    graphics.images.push_back(graphics.grphx.im_image8);
    graphics.images.push_back(graphics.grphx.im_image9);
    graphics.images.push_back(graphics.grphx.im_image10);
    graphics.images.push_back(graphics.grphx.im_image11);
    graphics.images.push_back(graphics.grphx.im_image12);

    const SDL_PixelFormat* fmt = gameScreen.GetFormat();
    graphics.backBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,32,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask ) ;
    SDL_SetSurfaceBlendMode(graphics.backBuffer, SDL_BLENDMODE_NONE);
    graphics.footerbuffer = SDL_CreateRGBSurface(SDL_SWSURFACE, 320, 10, 32, fmt->Rmask, fmt->Gmask, fmt->Bmask, fmt->Amask);
    SDL_SetSurfaceBlendMode(graphics.footerbuffer, SDL_BLENDMODE_BLEND);
    SDL_SetSurfaceAlphaMod(graphics.footerbuffer, 127);
    FillRect(graphics.footerbuffer, SDL_MapRGB(fmt, 0, 0, 0));
    graphics.Makebfont();


    graphics.foregroundBuffer =  SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask  );
    SDL_SetSurfaceBlendMode(graphics.foregroundBuffer, SDL_BLENDMODE_NONE);

    graphics.screenbuffer = &gameScreen;

    graphics.menubuffer = SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask );
    SDL_SetSurfaceBlendMode(graphics.menubuffer, SDL_BLENDMODE_NONE);

    graphics.towerbuffer =  SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask  );
    SDL_SetSurfaceBlendMode(graphics.towerbuffer, SDL_BLENDMODE_NONE);

	graphics.tempBuffer = SDL_CreateRGBSurface(SDL_SWSURFACE ,320 ,240 ,fmt->BitsPerPixel,fmt->Rmask,fmt->Gmask,fmt->Bmask,fmt->Amask  );
    SDL_SetSurfaceBlendMode(graphics.tempBuffer, SDL_BLENDMODE_NONE);

    //Make a temporary rectangle to hold the offsets
    // SDL_Rect offset;
    //Give the offsets to the rectangle
    // offset.x = 60;
    // offset.y = 80;

    //game.gamestate = TITLEMODE;
    //game.gamestate=EDITORMODE;
    //game.gamestate = PRELOADER; //Remember to uncomment this later!
    game.gamestate = TITLEMODE;

    game.menustart = false;
    game.mainmenu = 0;

    KeyPoll key;
    mapclass map;

    map.ypos = (700-29) * 8;
    map.bypos = map.ypos / 2;

    //Moved screensetting init here from main menu V2.1
    game.loadstats(map, graphics, music);
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

    entityclass obj;
    obj.init();

    if (startinplaytest) {
        game.levelpage=0;
        ed.getDirectoryData();
        game.loadcustomlevelstats();

        bool found = false;

        // search for the file in the vector
        for(growing_vector<std::string>::size_type i = 0; i < ed.ListOfMetaData.size(); i++) {
            LevelMetaData currentmeta = ed.ListOfMetaData[i];
            if (currentmeta.filename == playtestname) {
                game.playcustomlevel = (int)i;
                found = true;
                break;
            }
        }
        if (!found) {
            printf("Level not found\n");
            return 1;
        }
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
            script.startgamemode(23, key, graphics, game, map, obj, help, music);
        } else {
            script.startgamemode(22, key, graphics, game, map, obj, help, music);
        }
		//dwgfx.fademode = 4;

    }
    //Quick hack to start in final level ---- //Might be useful to leave this commented in for testing
    /*
    //game.gamestate=GAMEMODE;
		//game.start(obj,music);
		//script.startgamemode(8, key, graphics, game, map, obj, help, music);
   // map.finalmode = true; //Enable final level mode
		//map.finalx = 41; map.finaly = 52; //Midpoint
		//map.finalstretch = true;
		//map.final_colormode = true;
		//map.final_mapcol = 0;
		//map.final_colorframe = 0;

		//game.starttest(obj, music);

    game.savex = 5 * 8; game.savey = 15 * 8; game.saverx = 41; game.savery = 52;
    game.savegc = 0; game.savedir = 1;
    game.state = 0; game.deathseq = -1; game.lifeseq = 10;
		//obj.createentity(game, game.savex, game.savey, 0);
		map.gotoroom(game.saverx, game.savery, graphics, game, obj, music);
		//music.play(1);
		*/
    //End hack here ----

    volatile Uint32 time, timePrev = 0;
    game.infocus = true;
    key.isActive = true;

    game.gametimer = 0;

    std::thread logicthread([&](){
        while(!key.quitProgram)
        {
            std::atomic_thread_fence(std::memory_order_seq_cst);

            time = SDL_GetTicks();

            //framerate limit to 30
            Uint32 timetaken = time - timePrev;
            if(game.gamestate==EDITORMODE)
                    {
            if (timetaken < 24)
            {
                volatile Uint32 delay = 24 - timetaken;
                SDL_Delay( delay );
                time = SDL_GetTicks();
            }
            timePrev = time;

            }else{
            unsigned useframerate = game.gameframerate;
            if (game.sfpsmode) useframerate = useframerate / 2;
            if (timetaken < useframerate)
            {
                volatile Uint32 delay = useframerate - timetaken;
                SDL_Delay( delay );
                time = SDL_GetTicks();
            }
            timePrev = time;

            }

            key.Poll();

            if (key.isDown(SDLK_j) && (game.fpskeytimer == 0)) { // DEBUG 60 FPS MODE
                game.sfpsmode = !game.sfpsmode;
                game.fpskeytimer = 16;
                if (game.sfpsmode) game.fpskeytimer = 32;
            }
            if (game.fpskeytimer > 0) game.fpskeytimer--;

            changeloginput(key, graphics, map, game, obj, help, music);
            titleinput(key, graphics, map, game, obj, help, music);
            ////Logic
            titlelogic(graphics, game, obj, help, music, map);

            game.gameclock();

            //SDL_FillRect( SDL_GetVideoSurface(), NULL, 0 );
        }
    });

    while(!key.quitProgram)
    {
        std::atomic_thread_fence(std::memory_order_seq_cst);

        time = SDL_GetTicks();

        //framerate limit to 30
        Uint32 timetaken = time - timePrev;
        if(game.gamestate==EDITORMODE)
		{
          if (timetaken < 24)
          {
              volatile Uint32 delay = 24 - timetaken;
              SDL_Delay( delay );
              time = SDL_GetTicks();
          }
          timePrev = time;

        }else{
          unsigned useframerate = game.gameframerate;
          if (game.sfpsmode) useframerate = useframerate / 2;
          if (timetaken < useframerate)
          {
              volatile Uint32 delay = useframerate - timetaken;
              SDL_Delay( delay );
              time = SDL_GetTicks();
          }
          timePrev = time;

        }

        titlerender(graphics, map, game, obj, help, music);

        music.processmusic();
        graphics.processfade();
        gameScreen.FlipScreen();

        //SDL_FillRect( SDL_GetVideoSurface(), NULL, 0 );
    }


	  //SDL_Delay(300);

    //TODO
    //Free the loaded image
    //SDL_FreeSurface( gameScreen );

    //Quit SDL
    NETWORK_shutdown();
    SDL_Quit();
    FILESYSTEM_deinit();

    return 0;
}
