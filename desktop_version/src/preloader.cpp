#include "preloader.h"

#include "Enums.h"
#include <ctime>
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

std::atomic_int pre_fakepercent;
std::atomic_bool pre_quickend;
int pre_transition=30;
bool pre_startgame=false;
int pre_darkcol=0, pre_lightcol=0, pre_curcol=0, pre_coltimer=0, pre_offset=0;

int pre_frontrectx=30, pre_frontrecty=20, pre_frontrectw=260, pre_frontrecth=200;
int pre_temprectx=0, pre_temprecty=0, pre_temprectw=320, pre_temprecth=240;

#ifdef VCE_DEBUG
#include "stdio.h"

int last_percent = 0;
#endif

void preloaderlogic()
{
#ifdef VCE_DEBUG
  if (pre_fakepercent.load() != last_percent) {
    last_percent = pre_fakepercent.load();
    printf("%i%%\n", last_percent);
  }
#endif

  if (pre_transition < 30) pre_transition--;
  if(pre_transition>=30){
    if (pre_fakepercent.load() >= 100) {
      pre_fakepercent.store(100);
      pre_startgame = true;
    }

    pre_offset = (pre_offset + 4 + int(fRandom() * 5.0f))%32;
    pre_coltimer--;
    if (pre_coltimer <= 0) {
      pre_curcol = (pre_curcol + int(fRandom() * 5.0f)) % 6;
      pre_coltimer = 8;
    }
  }
}

void preloaderrender()
{
  if(pre_transition>=30){
    switch(pre_curcol) {
    case 0:
      pre_lightcol = graphics.RGBflip(0xBF,0x59,0x6F);
      pre_darkcol = graphics.RGBflip(0x88,0x3E,0x53);
      break;
    case 1:
      pre_lightcol = graphics.RGBflip(0x6C,0xBC,0x5C);
      pre_darkcol = graphics.RGBflip(0x50,0x86,0x40);
      break;
    case 2:
      pre_lightcol = graphics.RGBflip(0x5D,0x57,0xAA);
      pre_darkcol = graphics.RGBflip(0x2F,0x2F,0x6C);
      break;
    case 3:
      pre_lightcol = graphics.RGBflip(0xB7,0xBA,0x5E);
      pre_darkcol = graphics.RGBflip(0x84,0x83,0x42);
      break;
    case 4:
      pre_lightcol = graphics.RGBflip(0x57,0x90,0xAA);
      pre_darkcol = graphics.RGBflip(0x2F,0x5B,0x6C);
      break;
    case 5:
      pre_lightcol = graphics.RGBflip(0x90,0x61,0xB1);
      pre_darkcol = graphics.RGBflip(0x58,0x3D,0x71);
      break;
    default:
      pre_lightcol = graphics.RGBflip(0x00,0x00,0x00);
      pre_darkcol = graphics.RGBflip(0x08,0x00,0x00);
      break;
    }

    for (int i = 0; i < 18; i++) {
      pre_temprecty = (i * 16)- pre_offset;
      if (i % 2 == 0)
      {
        FillRect(graphics.backBuffer, pre_temprectx, pre_temprecty, pre_temprectw,pre_temprecth, pre_lightcol);
      }
      else
      {
        FillRect(graphics.backBuffer, pre_temprectx, pre_temprecty, pre_temprectw,pre_temprecth, pre_darkcol);
      }
    }

    FillRect(graphics.backBuffer, pre_frontrectx, pre_frontrecty, pre_frontrectw,pre_frontrecth, graphics.getBGR(0x3E,0x31,0xA2));

    if(pre_fakepercent.load() == 100){
      graphics.Print(282-(15*8), 204, "LOADING... " + help.String(int(pre_fakepercent.load()))+"%", 124, 112, 218, false);
    }else{
      graphics.Print(282-(14*8), 204, "LOADING... " + help.String(int(pre_fakepercent.load()))+"%", 124, 112, 218, false);
    }

    //Render
    if (pre_startgame) {
      pre_transition = 29;
    }
  }else if (pre_transition <= -10) {
    game.gamestate=TITLEMODE;
  }else if (pre_transition < 5) {
    FillRect(graphics.backBuffer, 0, 0, 320,240, graphics.getBGR(0,0,0));
  }else if (pre_transition < 20) {
    pre_temprecty = 0;
    pre_temprecth = 240;
    FillRect(graphics.backBuffer, pre_temprectx, pre_temprecty, pre_temprectw,pre_temprecth, 0x000000);
    FillRect(graphics.backBuffer, pre_frontrectx, pre_frontrecty, pre_frontrectw,pre_frontrecth, graphics.getBGR(0x3E,0x31,0xA2));

    graphics.Print(282-(15*8), 204, "LOADING... 100%", 124, 112, 218, false);
  }

  graphics.drawfade();

  graphics.render();
}

void preloaderloop() {
    volatile Uint32 time, timePrev = 0;
    auto gameScreen = graphics.screenbuffer;
    while(!key.quitProgram)
    {

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
            if (timetaken < game.gameframerate)
            {
                volatile Uint32 delay = game.gameframerate - timetaken;
                SDL_Delay( delay );
                time = SDL_GetTicks();
            }
            timePrev = time;

        }


        key.Poll();
        if(key.toggleFullscreen)
        {
            if(!gameScreen->isWindowed)
            {
                //SDL_WM_GrabInput(SDL_GRAB_ON);
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

            gameScreen->toggleFullScreen();
            game.fullscreen = !game.fullscreen;
            key.toggleFullscreen = false;

            key.keymap.clear(); //we lost the input due to a new window.
            game.press_left = false;
            game.press_right = false;
            game.press_action = true;
            game.press_map = false;
            printf("Error: failed: %s\n", SDL_GetError());




        }
        /*if(key.quitProgram)
        {
            music.playef(2);
        }*/

        if(!key.isActive)
        {
            Mix_Pause(-1);
            Mix_PauseMusic();
            FillRect(graphics.backBuffer, 0x00000000);
            graphics.bprint(5, 110, "Game paused", 196 - help.glow, 255 - help.glow, 196 - help.glow, true);
            graphics.bprint(5, 120, "[click to resume]", 196 - help.glow, 255 - help.glow, 196 - help.glow, true);
            graphics.bprint(5, 220, "Press M to mute in game", 164 - help.glow, 196 - help.glow, 164 - help.glow, true);
            graphics.bprint(5, 230, "Press N to mute music only", 164 - help.glow, 196 - help.glow, 164 - help.glow, true);
            graphics.render();
            //We are minimised, so lets put a bit of a delay to save CPU
            SDL_Delay(100);
        }
        else
        {
            Mix_Resume(-1);
            Mix_ResumeMusic();
            game.gametimer++;
            //Render
            preloaderrender();
        }

        //We did editorinput, now it's safe to turn this off
        key.linealreadyemptykludge = false;

        if (game.savemystats)
        {
            game.savemystats = false;
            game.savestats();
        }

        //Mute button
        if (key.isDown(KEYBOARD_m) && game.mutebutton<=0 && !ed.textentry &&
            !ed.textmod && ed.scripthelppage != 1)
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

        if (game.muted)
        {
            //if (game.globalsound == 1)
            //{
                Mix_VolumeMusic(0) ;
                Mix_Volume(-1,0);
            //}
        }

        if (!game.muted)
        {
            Mix_VolumeMusic(MIX_MAX_VOLUME) ;
            Mix_Volume(-1,MIX_MAX_VOLUME);
        }

        if(key.resetWindow)
        {
            key.resetWindow = false;
            gameScreen->ResizeScreen(-1, -1);
        }

        music.processmusic();
        graphics.processfade();
        game.gameclock();
        gameScreen->FlipScreen();

        //SDL_FillRect( SDL_GetVideoSurface(), NULL, 0 );

        if (pre_transition <= -10 || pre_quickend.load()) {
            break;
        }
    }
}
