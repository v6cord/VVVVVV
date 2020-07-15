#ifndef TEXTBOX_H
#define TEXTBOX_H

#include <SDL.h>
#include <string>
#include <vector>
#include "Game.h"

class textboxclass
{
public:
    textboxclass();

    void centerx(int centerline = 160);

    void centery(int centerline = 120);

    void adjust();

    void initcol(int rr, int gg, int bb);

    void setcol(int rr, int gg, int bb);

    void update();

    void remove();

    void removefast();

    void resize();

    void addline(std::string t);

    void createfast();
public:
    //Fundamentals
    std::vector<std::string> line;
    int xp, yp, lw, w, h = 0;
    int x,y = 0;
    int r,g,b = 0;
    int tr,tg,tb = 0;
    SDL_Rect textrect = {0};
    int timer = 0;

    float tl = 0.0;
    float prev_tl = 0.0;
    int tm = 0;

    int max = 0;

};

#endif /* TEXTBOX_H */
