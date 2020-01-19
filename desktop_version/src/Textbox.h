#ifndef TEXTBOX_H
#define TEXTBOX_H

#include "SDL.h"
#include <string>
#include <vector>
#include "Game.h"

class textboxclass
{
public:
    textboxclass();

    void firstcreate();

    void clear();

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
public:
    //Fundamentals
    growing_vector<std::string> line;
    int xp, yp, lw, w, h, numlines = 0;
    int x,y = 0;
    int r,g,b = 0;
    int tr,tg,tb = 0;
    SDL_Rect textrect = {0};
    bool active = false;
    int timer = 0;

    float tl = 0.0;
    int tm = 0;

    int max = 0;

};

#endif /* TEXTBOX_H */
