#ifndef MAPGAME_H
#define MAPGAME_H

#include "Tower.h"
#include "WarpClass.h"
#include "Finalclass.h"
#include "Labclass.h"
#include "Spacestation2.h"
#include "Otherlevel.h"
#include "Entity.h"
#include "Graphics.h"
#include <vector>
#include "Game.h"
#include "Music.h"
#include "editor.h"

class mapclass
{
public:
    mapclass();

    int RGB(int red,int green,int blue);

    int intpol(int a, int b, float c);

    void setteleporter(int x, int y);

    void remteleporter(int x, int y);

    void settrinket(int x, int y);

    void resetmap();

    void resetnames();

    void transformname(int t);

    std::string getglitchname(int x, int y);

    void initmapdata();
    void initcustommapdata();

    int finalat(int x, int y);

    int maptiletoenemycol(int t);

    void changefinalcol(int t);

    void setcol(const int r1, const int g1, const int b1 , const int r2, const  int g2, const int b2, const int c);

    void updatetowerglow();

    void nexttowercolour();

    void settowercolour(int t);

    bool spikecollide(int x, int y);

    bool collide(int x, int y);

    void settile(int xp, int yp, int t);
    void settile_special(int x, int y, int tile);



    int area(int _rx, int _ry);

    void exploretower();

    void hideship();

    void showship();

    void realign_tower();

    void resetplayer();

    int tower_row(int rx, int ry);
    int get_tower_offset(int tower, int ix, int iy, int *rx, int *ry, int ypos);
    int tower_connection(int *rx, int *ry, int ypos);
    int get_tower(int rx, int ry);
    int entering_tower(int rx, int ry, int *entry);
    bool leaving_tower(int *rx, int *ry);
    void custom_warpto(int ent);
    void warpto(int rx, int ry , int t, int tx, int ty);

    void gotodimroom(int rx, int ry);
    void gotoroom(int rx, int ry);

    std::string currentarea(int t);

    void loadlevel(int rx, int ry);

    void twoframedelayfix();

    Dimension* getdimension(int index);
    Dimension* getdimension();
    void dimensionwraparound(int* rx, int* ry);


    int roomdeaths[ed.maxwidth * ed.maxheight];
    int roomdeathsfinal[20 * 20];
    std::vector <int> areamap;
    std::vector <int> contents;
    bool explored[ed.maxwidth * ed.maxheight];
    std::vector <int> vmult;

    int background = 0;
    int rcol = 0;
    int tileset = 0;
    bool warpx = false;
    bool warpy = false;


    std::string roomname;
    std::string hiddenname;

    //Special tower stuff
    bool towermode = false;
    float ypos = 0.0;
    float oldypos = 0.0;
    int bypos = 0;
    int cameramode = 0;
    int cameraseek, cameraseekframe = 0;
    int resumedelay = 0;
    bool minitowermode = false;
    int minitowersize = 0;
    int scrolldir = 0;

    //This is the old colour cycle
    int r, g,b = 0;
    int check, cmode = 0;
    int towercol = 0;
    int colstate, colstatedelay = 0;
    int colsuperstate = 0;
    int spikeleveltop, spikelevelbottom = 0;
    int oldspikeleveltop, oldspikelevelbottom = 0;
    bool tdrawback = false;
    int bscroll = 0;
    //final level navigation
    int finalx = 0;
    int finaly = 0;
    bool finalmode = false;
    bool finalstretch = false;

    //Variables for playing custom levels
    bool custommode = false;
    bool custommodeforreal = false;
    int customx, customy = 0;
    int customwidth, customheight = 0;
    int custommmstartx, custommmstarty = 0;
    int custommmxoff, custommmyoff, custommmxsize, custommmysize = 0;
    int customzoom = 0;
    bool customshowmm = false;

    std::vector<std::string> specialnames;
    int glitchmode = 0;
    int glitchdelay = 0;
    std::string glitchname;

    //final level colour cycling stuff
    bool final_colormode = false;
    int final_mapcol = 0;
    int final_aniframe = 0;
    int final_aniframedelay = 0;
    int final_colorframe, final_colorframedelay = 0;

    //Teleporters and Trinkets on the map
    std::vector<point> teleporters;
    std::vector<point> shinytrinkets;

    bool showteleporters, showtargets, showtrinkets = false;

    //Roomtext
    bool roomtexton = false;
    std::vector<Roomtext> roomtext;

    //Levels
    otherlevelclass otherlevel;
    spacestation2class spacestation2;
    labclass lablevel;
    finalclass finallevel;
    warpclass warplevel;
    towerclass tower;
    int extrarow = 0;

    //Accessibility options
    bool invincibility = false;

    //Map cursor
    int cursorstate, cursordelay = 0;

    int kludge_bypos = 0;
    int kludge_colstate = 0;
    int kludge_scrolldir = 0;
    void inline bg_to_kludge()
    {
        kludge_bypos = bypos;
        kludge_colstate = colstate;
        kludge_scrolldir = scrolldir;
    }
    void inline kludge_to_bg()
    {
        bypos = kludge_bypos;
        colstate = kludge_colstate;
        scrolldir = kludge_scrolldir;
    }

    bool nofog = false;

    void updatetowerentcol(int col);

    int dimension = -1;

    bool collidefloor = false;
    bool collideroof = false;
};

extern mapclass map;

#endif /* MAPGAME_H */
