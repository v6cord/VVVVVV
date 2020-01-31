#ifndef EDITOR_H
#define EDITOR_H

#include <vector>
#include "Game.h"
#include <string>
#include <string_view>
#include "Script.h"

class KeyPoll; class Graphics; class Game; class mapclass; class entityclass; class UtilityClass;

enum tiletyp {
    TILE_NONE,
    TILE_BACKGROUND,
    TILE_SPIKE,
    TILE_FOREGROUND,
};

// Text entry field type
enum textmode {
    TEXT_NONE,

    // In-editor text fields
    TEXT_LOAD,
    TEXT_SAVE,
    TEXT_ROOMNAME,
    TEXT_SCRIPT,
    TEXT_ROOMTEXT,
    TEXT_ACTIVITYZONE,
    TEXT_GOTOROOM,
    LAST_EDTEXT = TEXT_GOTOROOM,

    // Settings-mode text fields
    TEXT_TITLE,
    TEXT_DESC,
    TEXT_WEBSITE,
    TEXT_CREATOR,
    NUM_TEXTMODES,

    // Text modes with an entity
    FIRST_ENTTEXT = TEXT_SCRIPT,
    LAST_ENTTEXT = TEXT_ACTIVITYZONE,
};

std::string find_title(std::string_view buf);
std::string find_desc1(std::string_view buf);
std::string find_desc2(std::string_view buf);
std::string find_desc3(std::string_view buf);
std::string find_creator(std::string_view buf);
std::string find_website(std::string_view buf);
std::string find_created(std::string_view buf);
std::string find_modified(std::string_view buf);
std::string find_modifiers(std::string_view buf);

class edentities{
public:
    int x, y, t = 0;
    int subx, suby = 0;
    //parameters
    int p1, p2, p3, p4, p5, p6 = 0;
    int state = 0;
    int intower = 0;
    std::string scriptname;
    std::string activityname;
    std::string activitycolor;
    bool onetime = false;
};


class edlevelclass{
public:
    edlevelclass();
    int tileset, tilecol = 0;
    std::string roomname;
    int warpdir = 0;
    int platx1 = 0;
    int platy1 = 0;
    int platx2 = 0;
    int platy2 = 0;
    int platv = 0;
    int enemyv = 0;
    int enemyx1, enemyy1, enemyx2, enemyy2, enemytype = 0;
    int directmode = 0;
    int tower = 0;
    int tower_row = 0;
};

class edaltstate {
public:
    edaltstate();
    int x, y = -1; // -1 means not set
    int state = -1;
    growing_vector<int> tiles;

    void reset();
};

class edtower {
public:
    edtower();
    int size = 40; // minimum size
    int scroll = 0; // scroll direction (0=The Tower, 1=Panic Room)
    growing_vector<int> tiles;

    void reset(void);
};

struct LevelMetaData
{
    std::string title;
    std::string creator;
    std::string Desc1;
    std::string Desc2;
    std::string Desc3;
    std::string website;
    std::string filename;

    std::string modifier;
    std::string timeCreated;
    std::string timeModified;

    int version = 0;
};


extern growing_vector<edentities> edentity;
extern scriptclass script;

class EditorData
{
    public:

    static EditorData& GetInstance()
    {
        static EditorData  instance; // Guaranteed to be destroyed.
        // Instantiated on first use.
        return instance;
    }


    int numedentities;
    std::string title;
    std::string creator;

    std::string modifier;
    std::string timeCreated;
    std::string timeModified;

private:


    EditorData():
    numedentities(0)
    {
    }

};


class editorclass {
    //Special class to handle ALL editor variables locally
public:
    editorclass();

    std::string Desc1;
    std::string Desc2;
    std::string Desc3;
    std::string website;

    growing_vector<std::string> directoryList;
    growing_vector<LevelMetaData> ListOfMetaData;

    void getDirectoryData();
    bool getLevelMetaData(std::string& filename, LevelMetaData& _data );

    void saveconvertor();
    void reset();
    void getlin(KeyPoll& key, enum textmode mode, std::string prompt, std::string *ptr);
    void loadlevel(int rxi, int ryi, int altstate);

    void placetile(int x, int y, int t);

    void placetilelocal(int x, int y, int t);

    int gettilelocal(int x, int y);
    void settilelocal(int x, int y, int tile);

    int getenemyframe(int t);
    int base(int x, int y);

    int backbase(int x, int y);

    enum tiletyp gettiletyp(int room, int tile);
    enum tiletyp gettiletyplocal(int x, int y);
    enum tiletyp getabstiletyp(int x, int y);

    int absat(int *x, int *y);
    int at(int x, int y);

    int freewrap(int x, int y);

    int backonlyfree(int x, int y);

    int backfree(int x, int y);

    int towerspikefree(int x, int y);
    int spikefree(int x, int y);
    int towerfree(int x, int y);
    int free(int x, int y);
    int getfree(enum tiletyp tile);
    int absfree(int x, int y);

    int match(int x, int y);
    int warpzonematch(int x, int y);
    int outsidematch(int x, int y);

    int backmatch(int x, int y);

    void switch_tileset(bool reversed);
    void switch_tileset_tiles(int from, int to);
    void switch_tilecol(bool reversed);
    void clamp_tilecol(int levx, int levy, bool wrap);

    void enable_tower(void);
    void snap_tower_entry(int rx, int ry);
    void upsize_tower(int tower, int y);
    void downsize_tower(int tower);
    void resize_tower_tiles(int tower);
    void shift_tower(int tower, int y);
    int get_tower(int rx, int ry);
    int tower_size(int tower);
    int tower_scroll(int tower);
    bool intower(void);
    int tower_row(int rx, int ry);

    void load(std::string& _path, Graphics& dwgfx, mapclass& map, Game& game);
    void save(std::string& _path, mapclass& map, Game& game);
    void generatecustomminimap(Graphics& dwgfx, mapclass& map);
    int toweredgetile(int x, int y);
    int edgetile(int x, int y);
    int warpzoneedgetile(int x, int y);
    int outsideedgetile(int x, int y);

    int backedgetile(int x, int y);

    int labspikedir(int x, int y, int t);
    int spikebase(int x, int y);
    int spikedir(int x, int y);
    int towerspikedir(int x, int y);
    int findtrinket(int t);
    int findcoin(int t);
    int findcrewmate(int t);
    int findwarptoken(int t);
    void countstuff();
    void findstartpoint(Game& game);
    void weirdloadthing(std::string t, Graphics& dwgfx, mapclass& map, Game& game);
    int getlevelcol(int t);
    int getenemycol(int t);
    int entcol = 0;

    //Colouring stuff
    int getwarpbackground(int rx, int ry);

    growing_vector<std::string> getLevelDirFileNames( );
    growing_vector <int> swapmap;
    growing_vector <int> contents;
    growing_vector <int> vmult;
    int numtrinkets = 0;
    int numcoins = 0;
    int numcrewmates = 0;
    growing_vector<edlevelclass> level;
    growing_vector<int> kludgewarpdir;

    int temp = 0;
    int notedelay = 0;
    std::string note;
    std::string keybuffer;
    std::string filename;

    int drawmode = 0;
    int tilex, tiley = 0;
    int keydelay, lclickdelay = 0;
    bool savekey, loadkey = false;
    int levx, levy = 0;
    int levaltstate = 0;
    int entframe, entframedelay = 0;

    enum textmode textmod; // In text entry
    std::string *textptr; // Pointer to text we're changing
    std::string textdesc; // Description (for editor mode text fields)
    std::string oldenttext; // Old text content
    int textcount; // Level description row, or activity zone parameter
    int textent; // Entity ID for text prompt

    int lastentcycle;
    int entcycle;

    bool xmod, zmod, spacemod, warpmod, textentry = false;
    bool titlemod = false;
    bool trialnamemod = false;
    bool creatormod = false;
    bool desc1mod = false;
    bool desc2mod = false;
    bool desc3mod = false;
    bool websitemod = false;

    bool trialstartpoint = false;

    int edtrial = 0;

    int roomnamehide = 0;
    bool saveandquit = false;
    bool shiftmenu, shiftkey = false;
    int spacemenu = 0;
    bool settingsmod, settingskey = false;
    bool trialmod = false;
    int warpent = 0;
    bool updatetiles, changeroom = false;
    int deletekeyheld = 0;

    int boundarymod, boundarytype = 0;
    int boundx1, boundx2, boundy1, boundy2 = 0;

    int levmusic = 0;
    int mapwidth, mapheight = 0; //Actual width and height of stage

    //Special; the physical max the engine allows
    int maxwidth = 100;
    int maxheight = 100;

    int version = 0;

    //Script editor stuff
    void removeline(int t);
    void insertline(int t);

    bool scripteditmod = false;
    int scripthelppage, scripthelppagedelay = 0;
    std::string sb[500];
    std::string sbscript;
    int sblength = 0;
    int sbx, sby = 0;
    int pagey = 0;

    std::string author;
    std::string description;
    std::string title;

    //Functions for interfacing with the script:
    void addhook(std::string t);
    void removehook(std::string t);
    void addhooktoscript(std::string t);
    void removehookfromscript(std::string t);
    void loadhookineditor(std::string t);
    void clearscriptbuffer();
    void gethooks();
    bool checkhook(std::string t);
    std::string hooklist[500];
    int numhooks = 0;

    int hookmenupage, hookmenu = 0;

    //Direct Mode variables
    int dmtile = 0;
    int dmtileeditor = 0;

    bool grayenemieskludge = false;

    growing_vector<edaltstate> altstates;
    growing_vector<edtower> towers;

    int ypos; // tower mode y position

    int getedaltstatenum(int rxi, int ryi, int state);
    void addaltstate(int rxi, int ryi, int state);
    void removealtstate(int rxi, int ryi, int state);
    int getnumaltstates(int rxi, int ryi);

    int entspeed = 0;
};

void addedentity(int xp, int yp, int tp, int p1=0, int p2=0, int p3=0, int p4=0, int p5=320, int p6=240);

void naddedentity(int xp, int yp, int tp, int p1=0, int p2=0, int p3=0, int p4=0, int p5=320, int p6=240);

void copyedentity(int a, int b);

void removeedentity(int t);

int edentat(int x, int y, int state = 0, int tower = 0);


bool edentclear(int x, int y, int state = 0, int tower = 0);

void fillbox(Graphics& dwgfx, int x, int y, int x2, int y2, int c);

void fillboxabs(Graphics& dwgfx, int x, int y, int x2, int y2, int c);

int dmcap(void);
int dmwidth(void);

void editorrender(KeyPoll& key, Graphics& dwgfx, Game& game,  mapclass& map, entityclass& obj, UtilityClass& help);

void editorlogic(KeyPoll& key, Graphics& dwgfx, Game& game, entityclass& obj,  musicclass& music, mapclass& map, UtilityClass& help);

void editorinput(KeyPoll& key, Graphics& dwgfx, Game& game, mapclass& map,
                 entityclass& obj, UtilityClass& help, musicclass& music);

#endif /* EDITOR_H */
