#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#if defined(__SWITCH__) || defined(__ANDROID__)
    #include <SDL2/SDL.h>
#else
    #include <SDL.h>
#endif
#include "Maths.h"
#include "UtilityClass.h"
#include "GraphicsUtil.h"

struct scriptmarker {
    int x = 0;
    int y = 0;
    int tile = 0;
};

struct customtrial {
    int roomx = 0;
    int roomy = 0;
    int startx = 0;
    int starty = 0;
    int startf = 0;
    int par = 0;
    int trinkets = 0;
    int music = 0;
    std::string name = "";
};

struct customtrialrecord {
    int time = 0;
    int trinkets = 0;
    int lives = 0;
    int rank = 0;
    int attempted = false;
};

class Game
{
public:
    void init(void);
    ~Game(void);


    void setGlobalSoundVol(const float _vol)
    {
        m_globalVol = _vol;
    }
    float getGlobalSoundVol()
    {
        return m_globalVol;
    }


    int crewrescued();

    std::string unrescued();

    void resetgameclock();

    void customsavequick(std::string savfile);
    void savequick();

    void gameclock();

    std::string giventimestring(int hrs, int min, int sec);

    std::string  timestring();

    std::string partimestring();

    std::string resulttimestring();

    std::string timetstring(int t);

    void  createmenu(std::string t);

    void lifesequence();

    void gethardestroom();

    void updatestate();

    void unlocknum(int t);

    void loadstats();

    void  savestats();

    void deletestats();

    void deletequick();

    void savetele();

    void loadtele();

    void deletetele();

    void customstart();

    void start();

    void startspecial(int t);

    void starttrial(int t);

    void swnpenalty();

    void deathsequence();

    void customloadtrialsave(std::string savfile);
    void customsavetrialsave(std::string savfile);
    void customloadquick(std::string savfile);
    void loadquick();

    void loadsummary();

    void initteleportermode();

    std::string saveFilePath;


    int door_left = 0;
    int door_right = 0;
    int door_up = 0;
    int door_down = 0;
    int roomx, roomy, roomchangedir, roomchangevdir = 0;
    int prevroomx, prevroomy = 0;
    int temp, j, k = 0;

    int savex, savey, saverx, savery = 0;
    int savegc, savedir = 0;

    //Added for port
    int edsavex, edsavey, edsaverx, edsavery = 0;
    int edsavegc, edsavedir = 0;

    //State logic stuff
    int state, statedelay = 0;

    bool glitchrunkludge = false;

    int usingmmmmmm = 0;

    int gamestate = 0;
    bool hascontrol, jumpheld = false;
    int jumppressed = 0;
    int gravitycontrol = 0;

    bool infocus = false;
    bool muted = false;
    int mutebutton = 0;
private:
    float m_globalVol = 0.0;

public:

    int tapleft, tapright = 0;

    //Menu interaction stuff
    bool mapheld = false;
    int menupage = 0;
    //public var crewstats:Array = new Array();
    int lastsaved = 0;
    int deathcounts = 0;
    int timerStartTime = 0;

    int frames, seconds, minutes, hours = 0;
    bool gamesaved = false;
    std::string savetime;
    std::string savearea;
    int savetrinkets = 0;
    bool startscript = false;
    std::string newscript;

    int mainmenu = 0;
    bool menustart = false;

    unsigned changelogoffset = 0;
    int changelogkeydelay = 0;

    //Teleporting
    bool teleport_to_new_area = false;
    int teleport_to_x, teleport_to_y = 0;
    std::string teleportscript;
    bool useteleporter = false;
    int teleport_to_teleporter = 0;

    //Main Menu Variables
    growing_vector<std::string> menuoptions;
    growing_vector<int> menuoptionsactive;
    int nummenuoptions, currentmenuoption = 0;
    std::string menuselection, currentmenuname, previousmenuname;
    int current_credits_list_index = 0;
    int menuxoff, menuyoff = 0;

    int menucountdown = 0;
    std::string menudest;

    int creditposx, creditposy, creditposdelay = 0;


    //Sine Wave Ninja Minigame
    bool swnmode = false;
    int swngame, swnstate, swnstate2, swnstate3, swnstate4, swndelay, swndeaths = 0;
    int swntimer, swncolstate, swncoldelay = 0;
    int  swnrecord, swnbestrank, swnrank, swnmessage = 0;

    //SuperCrewMate Stuff
    bool supercrewmate, scmhurt, scmmoveme = false;
    int scmprogress = 0;

    //Accessibility Options
    bool  colourblindmode = false;
    bool noflashingmode = false;
    int slowdown = 0;
    Uint32 gameframerate = 0;

    bool nodeathmode = false;
    int gameoverdelay = 0;
    bool nocutscenes = false;

    //Time Trials
    bool intimetrial, timetrialparlost = false;
    int timetrialcountdown, timetrialshinytarget, timetriallevel = 0;
    int timetrialpar, timetrialresulttime, timetrialrank = 0;

    int creditposition = 0;
    int creditmaxposition = 0;
    std::vector<const char*> superpatrons;
    std::vector<const char*> patrons;
    std::vector<const char*> githubfriends;
    bool insecretlab = false;

    bool inintermission = false;

    growing_vector<int> crewstats;

    bool alarmon = false;
    int alarmdelay = 0;
    bool blackout = false;

    growing_vector<int> tele_crewstats;

    growing_vector<int> quick_crewstats;

    growing_vector<int> unlock;
    growing_vector<int> unlocknotify;
    growing_vector<int> temp_unlock;
    growing_vector<int> temp_unlocknotify;
    int stat_trinkets = 0;
    bool fullscreen = false;
    int bestgamedeaths = 0;

    bool stat_screenshakes = false;
    bool stat_backgrounds = false;
    bool stat_flipmode = false;
    bool stat_invincibility = false;
    int stat_slowdown = 0;


    growing_vector<int>besttimes;
    growing_vector<int>besttrinkets;
    growing_vector<int>bestlives;
    growing_vector<int> bestrank;

    std::string tele_gametime;
    int tele_trinkets = 0;
    std::string tele_currentarea;
    std::string quick_gametime;
    int quick_trinkets = 0;
    std::string quick_currentarea;

    int mx, my = 0;
    int screenshake, flashlight = 0;
    bool test = false;
    bool advancetext, pausescript = false;

    int deathseq, lifeseq = 0;

    int coins, crewmates, trinkencollect = 0;
    int trinkets();
    int savepoint, teleport, teleportxpos = 0;
    int edteleportent = 0;
    bool completestop = false;

    float inertia = 0.0;

    int companion = 0;
    bool roomchange = false;
    SDL_Rect teleblock = {0};
    bool activetele = false;
    int readytotele = 0;
    int activity_r, activity_g, activity_b = 0;
    std::string activity_lastprompt;

    std::string telesummary, quicksummary, customquicksummary;

    bool backgroundtext = false;

    int activeactivity, act_fade = 0;

    bool press_left, press_right, press_action, press_map = false;

    //Some stats:
    int totalflips = 0;
    std::string hardestroom;
    int hardestroomdeaths, currentroomdeaths = 0;

    bool savemystats = false;


    bool advanced_mode = false;
    bool fullScreenEffect_badSignal = false;
    bool useLinearFilter = false;
    int stretchMode = 0;
    int controllerSensitivity = 0;

    bool menukludge = false;
    bool quickrestartkludge = false;

    bool paused = false;
    int globalsound = 0;

    //Custom stuff
    std::string customscript[50];
    int customcol = 0;
    int levelpage = 0;
    int playcustomlevel = 0;
    std::string customleveltitle;
    std::string customlevelfilename;

    void clearcustomlevelstats();
    void loadcustomlevelstats();
    void savecustomlevelstats();
    void updatecustomlevelstats(std::string clevel, int cscore);

    std::string customlevelstats[200]; //string array containing level filenames
    int customlevelscore[200] = {0};//0 - not played, 1 - finished, 2 - all trinkets, 3 - finished, all trinkets
    int numcustomlevelstats = 0;
    bool customlevelstatsloaded = false;


    growing_vector<SDL_GameControllerButton> controllerButton_map;
    growing_vector<SDL_GameControllerButton> controllerButton_flip;
    growing_vector<SDL_GameControllerButton> controllerButton_esc;
    growing_vector<scriptmarker> scriptmarkers;

    bool hidemarkers = false;
    bool skipfakeload = false;

    int playerspeed = 3;
    bool nofriction = false;

    bool noflip = false;
    bool noenter = false;
    bool infiniflip = false;

    bool nosuicide = false;

    int gametimer = 0;

    bool cliplaytest = false;
    int playx = 0;
    int playy = 0;
    int playrx = 0;
    int playry = 0;
    int playgc = 0;

    growing_vector<SDL_Surface*> script_images;
    growing_vector<std::string> script_image_names;

    bool incustomtrial = false;
    int currenttrial = 0;

    growing_vector<customtrialrecord> customtrialstats;
    bool nocoincounter = false;


    bool quiet = false;

    growing_vector<std::string> onetimescripts;

    bool cutemode = false;
    bool allymode = false;
    bool misamode = false;
};

extern Game game;

#endif /* GAME_H */
