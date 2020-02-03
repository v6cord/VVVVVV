#include "Textbox.h"
#include "Graphics.h"
#include <utf8/checked.h>

textboxclass::textboxclass()
{
    firstcreate();
}

void textboxclass::firstcreate()
{
    //Like clear, only it creates the actual arrays, etc
    for (int iter = 0; iter < 10; iter++)
    {
        std::string t;
        t = "";
        line.push_back(t);
    }
    x = 0;
    y = 0;
    w = 0;
    h = 0;
    numlines = 0;
    lw = 0;
    tl = 0;
    tm = 0;
    active = false;
    timer = 0;
}

void textboxclass::clear()
{
    //Set all values to a default, required for creating a new entity
    for (size_t iter = 0; iter < line.size(); iter++)
    {
        line[iter]="";
    }
    xp = 0;
    yp = 0;
    w = 0;
    h = 0;
    numlines = 1;
    lw = 0;
    tl = 0;
    tm = 0;
    active = true;
    timer = 0;
}

void textboxclass::centerx(int centerline)
{
    resize();
    xp = centerline - (w / 2);
    resize();
}
void textboxclass::centery(int centerline)
{
    resize();
    yp = centerline - (h / 2);
    resize();
}

void textboxclass::adjust()
{
    resize();
    if (xp < 10) xp = 10;
    if (yp < 10) yp = 10;
    if (xp + w > 310) xp = 310 - w;
    if (yp + h > 230) yp = 230 - h;
    resize();
}

void textboxclass::initcol(int rr, int gg, int bb)
{
    tr = rr;
    tg = gg;
    tb = bb;
    r = 0;
    g = 0;
    b = 0;
    tl = 0.5;
}

void textboxclass::setcol(int rr, int gg, int bb)
{
    r = rr;
    g = gg;
    b = bb;
}

void textboxclass::update()
{
    if (tm == 0)
    {
        tl += .1f;
        if (tl >= 1)
        {
            tl = 1;
            tm = 1;
        }
        setcol(int(tr * tl), int(tg * tl), int(tb * tl));
    }
    else if (tm == 2)
    {
        tl -= .1f;
        if (tl <= 0.5)
        {
            tl = 0.5;
            active = false;
        }
        setcol(int(tr * tl), int(tg * tl), int(tb * tl));
    }
    if (timer > 0)
    {
        timer--;
        if (timer == 0) tm = 2;
    }
}

void textboxclass::remove()
{
    tm = 2;
    tl = 1.0f; //Remove mode
}

void textboxclass::removefast()
{
    tm = 2;
    tl = 0.4f; //Remove mode
}

void textboxclass::resize()
{
    //Set the width and height to the correct sizes
    max = 0;
    for (int iter = 0; iter < numlines; iter++)
    {
        auto utf = line[iter].begin();
        unsigned int len = 0;
        while (utf != line[iter].end()) {
            auto ch = utf8::next(utf, line[iter].end());
            len += graphics.bfontlen(ch);
        }
        if (len > (unsigned int)max) max = len;
    }

    lw = max / 8;
    w = max + 16;
    h = (numlines + 2) * 8;
    textrect.x = xp;
    textrect.y = yp;
    textrect.w = w;
    textrect.h = h;
}

void textboxclass::addline(std::string t)
{
    auto max = 8;
    auto utf = t.begin();
    while (utf != t.end()) {
        auto ch = utf8::next(utf, t.end());
        auto len = graphics.bfont[graphics.font_idx(ch)]->h;
        if (len > max) max = len;
    }

    line[numlines] = t;
    numlines += max / 8;
    resize();
    if (numlines >= 12) numlines = 0;
}

void textboxclass::createfast()
{
    tm = 1;
    tl = 1.0f;
    setcol(int(tr * tl), int(tg * tl), int(tb * tl));
}
