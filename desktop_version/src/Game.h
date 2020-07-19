#ifndef GAME_H
#define GAME_H

#include <vector>
#include <string>
#include <SDL.h>
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

struct MenuOption
{
    std::string text;
    bool active;
};

//Menu IDs
namespace Menu
{
    enum MenuName
    {
        mainmenu,
        playerworlds,
        levellist,
        quickloadlevel,
        youwannaquit,
        errornostart,
        graphicoptions,
        ed_settings,
        ed_desc,
        ed_music,
        ed_quit,
        options,
        advancedoptions,
        accessibility,
        controller,
        cleardatamenu,
        setinvincibility,
        setslowdown,
        unlockmenu,
        credits,
        credits2,
        credits25,
        credits3,
        credits4,
        credits5,
        credits6,
        play,
        unlocktimetrial,
        unlocktimetrials,
        unlocknodeathmode,
        unlockintermission,
        unlockflipmode,
        newgamewarning,
        playmodes,
        intermissionmenu,
        playint1,
        playint2,
        continuemenu,
        startnodeathmode,
        gameover,
        gameover2,
        unlockmenutrials,
        timetrials,
        nodeathmodecomplete,
        nodeathmodecomplete2,
        timetrialcomplete,
        timetrialcomplete2,
        timetrialcomplete3,
        gamecompletecontinue,

        loadcustomtrial,
        ed_settings2,
        ed_settings3,
        ed_dimensions,
        ed_trials,
        ed_edit_trial,
        ed_remove_trial,
        changelog,
        credits_ce,
    };
};

struct MenuStackFrame
{
    int option;
    enum Menu::MenuName name;
};

struct CustomLevelStat
{
    std::string name;
    int score; //0 - not played, 1 - finished, 2 - all trinkets, 3 - finished, all trinkets
};


class Game
{
public:
    void init(void);
    ~Game(void);


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

    void returnmenu();
    void returntomenu(enum Menu::MenuName t);
    void  createmenu(enum Menu::MenuName t, bool samemenu = false);

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

    bool muted = false;
    int mutebutton = 0;
    bool musicmuted = false;
    int musicmutebutton = 0;

    int tapleft, tapright = 0;

    //Menu interaction stuff
    bool mapheld = false;
    int menupage = 0;
    int lastsaved = 0;
    int deathcounts = 0;

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
    std::vector<MenuOption> menuoptions;
    int currentmenuoption = 0;
    enum Menu::MenuName currentmenuname;
    enum Menu::MenuName kludge_ingametemp = Menu::mainmenu;
    int current_credits_list_index = 0;
    int menuxoff, menuyoff = 0;
    int menuspacing = 0;
    std::vector<MenuStackFrame> menustack;

    void inline option(std::string text, bool active = true)
    {
        MenuOption menuoption;
        menuoption.text = text;
        menuoption.active = active;
        menuoptions.push_back(menuoption);
    }

    int menucountdown = 0;
    enum Menu::MenuName menudest;

    int creditposx, creditposy, creditposdelay = 0;
    int oldcreditposx = 0;


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
    int timetrialpar, timetrialresulttime, timetrialresultframes, timetrialrank = 0;

    int creditposition = 0;
    int oldcreditposition = 0;
    int creditmaxposition = 0;
    bool insecretlab = false;

    bool inintermission = false;

    static const int numcrew = 6;
    bool crewstats[numcrew];

    bool alarmon = false;
    int alarmdelay = 0;
    bool blackout = false;

    bool tele_crewstats[numcrew];

    bool quick_crewstats[numcrew];

    static const int numunlock = 25;
    bool unlock[numunlock];
    bool unlocknotify[numunlock];
    bool anything_unlocked();
    int stat_trinkets = 0;
    bool fullscreen = false;
    int bestgamedeaths = 0;

    static const int numtrials = 6;
    int besttimes[numtrials];
    int bestframes[numtrials];
    int besttrinkets[numtrials];
    int bestlives[numtrials];
    int bestrank[numtrials];

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

    int coins = 0;
    int trinkets();
    int crewmates();
    int savepoint, teleportxpos = 0;
    bool teleport = false;
    int edteleportent = 0;
    bool completestop = false;

    float inertia = 0.0;

    int companion = 0;
    bool roomchange = false;
    SDL_Rect teleblock = {0};
    bool activetele = false;
    int readytotele = 0;
    int oldreadytotele = 0;
    int activity_r, activity_g, activity_b = 0;
    std::string activity_lastprompt;

    std::string telesummary, quicksummary, customquicksummary;
    bool save_exists();

    bool backgroundtext = false;

    int activeactivity, act_fade = 0;
    int prev_act_fade = 0;

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

    bool quickrestartkludge = false;

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

    std::vector<CustomLevelStat> customlevelstats;
    bool customlevelstatsloaded = false;


    std::vector<SDL_GameControllerButton> controllerButton_map;
    std::vector<SDL_GameControllerButton> controllerButton_flip;
    std::vector<SDL_GameControllerButton> controllerButton_esc;
    std::vector<scriptmarker> scriptmarkers;

    bool hidemarkers = false;
    bool skipfakeload = false;
    bool ghostsenabled = false;

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
    std::string playassets;

    std::vector<SDL_Surface*> script_images;
    std::vector<std::string> script_image_names;

    bool incustomtrial = false;
    int currenttrial = 0;

    std::vector<customtrialrecord> customtrialstats;
    bool nocoincounter = false;


    std::vector<std::string> onetimescripts;

    bool cutemode = false;
    bool allymode = false;
    bool misamode = false;

    void quittomenu();
    void returntolab();
    bool fadetomenu;
    int fadetomenudelay;
    bool fadetolab;
    int fadetolabdelay;

#if !defined(NO_CUSTOM_LEVELS)
    void returntoeditor();
    bool shouldreturntoeditor;
#endif

    int playercolour = 0;

    bool inline inspecial()
    {
        return inintermission || insecretlab || intimetrial || nodeathmode;
    }

    bool over30mode;
    bool glitchrunnermode; // Have fun speedrunners! <3 Misa

    bool ingame_titlemode;

    bool shouldreturntopausemenu;
    void returntopausemenu();

    bool disablepause;
};

extern Game game;

#endif /* GAME_H */
