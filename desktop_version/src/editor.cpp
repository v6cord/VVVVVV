#include "editor.h"

#include "Game.h"
#include "Graphics.h"
#include "Entity.h"
#include "Music.h"
#include "KeyPoll.h"
#include "Map.h"
#include "Script.h"
#include "UtilityClass.h"
#include "time.h"
#include "Utilities.h"

#include "tinyxml.h"

#include "Enums.h"

#include "FileSystemUtils.h"

#include <string>
#include <string_view>
#include <memory>
#include <stdexcept>
#include <utf8/checked.h>
#include <physfs.h>
#include <iterator>

edlevelclass::edlevelclass()
{
    tileset=0;
    tilecol=0;
    roomname="";
    warpdir=0;
    platx1=0;
    platy1=0;
    platx2=320;
    platy2=240;
    platv=4;
    enemyv=4;
    enemyx1=0;
    enemyy1=0;
    enemyx2=320;
    enemyy2=240;
    enemytype=0;
    directmode=0;
    tower=0;
    tower_row=0;
}

edaltstate::edaltstate()
{
    reset();
}

void edaltstate::reset()
{
    x = -1;
    y = -1;
    state = -1;
    tiles.resize(40 * 30);
}

edtower::edtower() {
    reset();
}

void edtower::reset(void) {
    size = 40;
    scroll = 0;
    tiles.resize(40 * size);
    int x, y;
    for (x = 0; x < 40; x++)
        for (y = 0; y < size; y++)
            tiles[x + y*40] = 0;
}

editorclass::editorclass()
{
    //We create a blank map
    for (int j = 0; j < 30 * maxwidth; j++)
    {
        for (int i = 0; i < 40 * maxheight; i++)
        {
            contents.push_back(0);
        }
    }

    for (int j = 0; j < 30; j++)
    {
        for (int i = 0; i < 40; i++)
        {
            swapmap.push_back(0);
        }
    }

    for (int i = 0; i < 30 * maxheight; i++)
    {
        vmult.push_back(int(i * 40 * maxwidth));
    }

    altstates.resize(500);
    towers.resize(400);
    level.resize(maxwidth * maxheight);
    kludgewarpdir.resize(maxwidth * maxheight);

    entspeed = 0;

    reset();
}

// comparison, not case sensitive.
bool compare_nocase (std::string first, std::string second)
{
    unsigned int i=0;
    while ( (i<first.length()) && (i<second.length()) )
    {
        if (tolower(first[i])<tolower(second[i]))
            return true;
        else if (tolower(first[i])>tolower(second[i]))
            return false;
        ++i;
    }
    if (first.length()<second.length())
        return true;
    else
        return false;
}

void editorclass::getDirectoryData()
{

    ListOfMetaData.clear();
    directoryList.clear();

    directoryList = FILESYSTEM_getLevelDirFileNames();

    for(size_t i = 0; i < directoryList.size(); i++)
    {
        LevelMetaData temp;
        if (getLevelMetaData( directoryList[i], temp))
        {
            ListOfMetaData.push_back(temp);
        }
    }

    for(size_t i = 0; i < ListOfMetaData.size(); i++)
    {
        for(size_t k = 0; k < ListOfMetaData.size(); k++)
        {
            if(compare_nocase(ListOfMetaData[i].title, ListOfMetaData[k].title ))
            {
                std::swap(ListOfMetaData[i] , ListOfMetaData[k]);
                std::swap(directoryList[i], directoryList[k]);
            }
        }
    }

}
bool editorclass::getLevelMetaData(std::string& _path, LevelMetaData& _data )
{
    unsigned char *uMem = NULL;
    FILESYSTEM_loadFileToMemory(_path.c_str(), &uMem, NULL, true);

    if (uMem == NULL)
    {
        printf("Level %s not found :(\n", _path.c_str());
        return false;
    }

    std::unique_ptr<char[], free_delete> mem((char*) uMem);

    try {
        _data.timeCreated = find_created(mem.get());
        _data.creator = find_creator(mem.get());
        _data.title = find_title(mem.get());
        _data.timeModified = find_modified(mem.get());
        _data.modifier = find_modifiers(mem.get());
        _data.Desc1 = find_desc1(mem.get());
        _data.Desc2 = find_desc2(mem.get());
        _data.Desc3 = find_desc3(mem.get());
        _data.website = find_website(mem.get());
        _data.filename = _path;
        return true;
    } catch (const std::out_of_range& ex) {
        return false;
    }
}

void editorclass::reset()
{
    version=2; //New smaller format change is 2

    mapwidth=5;
    mapheight=5;

    EditorData::GetInstance().title="Untitled Level";
    EditorData::GetInstance().creator="Unknown";
    Desc1="";
    Desc2="";
    Desc3="";
    website="";

    roomnamehide=0;
    zmod=false;
    xmod=false;
    spacemod=false;
    spacemenu=0;
    shiftmenu=false;
    shiftkey=false;
    saveandquit=false;
    note="";
    notedelay=0;
    textentry=false;
    deletekeyheld=false;
    textmod = TEXT_NONE;

    entcycle = 0;
    lastentcycle = 0;

    trialnamemod=false;
    titlemod=false;
    creatormod=false;
    desc1mod=false;
    desc2mod=false;
    desc3mod=false;
    websitemod=false;
    settingsmod=false;
    trialmod=false;
    warpmod=false; //Two step process
    warpent=-1;

    boundarymod=0;
    boundarytype=0;
    boundx1=0;
    boundx2=0;
    boundy1=0;
    boundy2=0;

    drawmode=0;
    dmtile=0;
    dmtileeditor=0;
    entcol=0;

    tilex=0;
    tiley=0;
    levx=0;
    levy=0;
    levaltstate=0;
    keydelay=0;
    lclickdelay=0;
    savekey=false;
    loadkey=false;
    updatetiles=true;
    changeroom=true;
    levmusic=0;

    trialstartpoint = false;

    entframe=0;
    entframedelay=0;

    numtrinkets=0;
    numcoins=0;
    numcrewmates=0;
    EditorData::GetInstance().numedentities=0;
    levmusic=0;

    for (int j = 0; j < maxheight; j++)
    {
        for (int i = 0; i < maxwidth; i++)
        {
            level[i+(j*maxwidth)].tileset=0;
            level[i+(j*maxwidth)].tilecol=(i+j)%32;
            level[i+(j*maxwidth)].roomname="";
            level[i+(j*maxwidth)].warpdir=0;
            level[i+(j*maxwidth)].platx1=0;
            level[i+(j*maxwidth)].platy1=0;
            level[i+(j*maxwidth)].platx2=320;
            level[i+(j*maxwidth)].platy2=240;
            level[i+(j*maxwidth)].platv=4;
            level[i+(j*maxwidth)].enemyv=4;
            level[i+(j*maxwidth)].enemyx1=0;
            level[i+(j*maxwidth)].enemyy1=0;
            level[i+(j*maxwidth)].enemyx2=320;
            level[i+(j*maxwidth)].enemyy2=240;
            level[i+(j*maxwidth)].enemytype=0;
            level[i+(j*maxwidth)].directmode=0;
            level[i+(j*maxwidth)].tower=0;
            level[i+(j*maxwidth)].tower_row=0;
            kludgewarpdir[i+(j*maxwidth)]=0;
        }
    }

    for (int j = 0; j < 30 * maxheight; j++)
    {
        for (int i = 0; i < 40 * maxwidth; i++)
        {
            contents[i+(j*40*maxwidth)]=0;
        }
    }

    if(numhooks>0)
    {
        for(int i=0; i<numhooks; i++)
        {
            removehook(hooklist[i]);
        }
    }

    for (int i = 0; i < 500; i++)
    {
        sb[i]="";
    }
    for (int i = 0; i < 500; i++)
    {
        hooklist[i]="";
    }

    clearscriptbuffer();
    sblength=1;
    sbx=0;
    sby=0;
    pagey=0;
    scripteditmod=false;
    sbscript="null";
    scripthelppage=0;
    scripthelppagedelay=0;

    hookmenupage=0;
    hookmenu=0;
    numhooks=0;
    script.customscript.clear();

    grayenemieskludge = false;

    for (size_t i = 0; i < altstates.size(); i++)
        altstates[i].reset();
    for (size_t i = 0; i < towers.size(); i++)
        towers[i].reset();

    edentity.clear();
    edentity.resize(3000);
}

void editorclass::weirdloadthing(std::string t, Graphics& dwgfx, mapclass& map, Game& game)
{
    //Stupid pointless function because I hate C++ and everything to do with it
    //It's even stupider now that I don't need to append .vvvvvv anymore! bah, whatever
    //t=t+".vvvvvv";
    load(t, dwgfx, map, game);
}

void editorclass::gethooks()
{
    //Scan through the script and create a hooks list based on it
    numhooks=0;
    std::string tstring;
    std::string tstring2;
    for(size_t i=0; i<script.customscript.size(); i++)
    {
        tstring=script.customscript[i];
        if((int) tstring.length() - 1 >= 0) // FIXME: This is sketchy. -flibit
        {
            tstring=tstring[tstring.length()-1];
        }
        else
        {
            tstring="";
        }
        if(tstring==":")
        {
            tstring2="";
            tstring=script.customscript[i];
            for(size_t j=0; j<tstring.length()-1; j++)
            {
                tstring2+=tstring[j];
            }
            hooklist[numhooks]=tstring2;
            numhooks++;
        }
    }
}

void editorclass::loadhookineditor(std::string t)
{
    //Find hook t in the scriptclass, then load it into the editor
    clearscriptbuffer();

    std::string tstring;

    bool removemode=false;
    for(size_t i=0; i<script.customscript.size(); i++)
    {
        if(script.customscript[i]==t+":")
        {
            removemode=true;
        }
        else if(removemode)
        {
            tstring=script.customscript[i];
            if(tstring != "")
            {
                tstring = tstring[tstring.length()-1];
            }
            if(tstring==":")
            {
                //this is a hook
                removemode=false;
            }
            else
            {
                //load in this line
                sb[sblength-1]=script.customscript[i];
                sblength++;
            }
        }
    }
    if(sblength>1) sblength--;
}

void editorclass::addhooktoscript(std::string t)
{
    //Adds hook+the scriptbuffer to the end of the scriptclass
    removehookfromscript(t);
    script.customscript.push_back(t+":");
    if(sblength>=1)
    {
        for(int i=0; i<sblength; i++)
        {
            script.customscript.push_back(sb[i]);
        }
    }
}

void editorclass::removehookfromscript(std::string t)
{
    //Find hook t in the scriptclass, then removes it (and any other code with it)
    std::string tstring;
    bool removemode=false;
    for(size_t i=0; i<script.customscript.size(); i++)
    {
        if(script.customscript[i]==t+":")
        {
            removemode=true;
            //Remove this line
            for(size_t j=i; j<script.customscript.size()-1; j++)
            {
                script.customscript[j]=script.customscript[j+1];
            }
            script.customscript.pop_back();

            i--;
        }
        else if(removemode)
        {
            //If this line is not the start of a new hook, remove it!
            tstring=script.customscript[i];
            tstring=tstring[tstring.length()-1];
            if(tstring==":")
            {
                //this is a hook
                removemode=false;
            }
            else
            {
                //Remove this line
                for(size_t j=i; j<script.customscript.size()-1; j++)
                {
                    script.customscript[j]=script.customscript[j+1];
                }
                script.customscript.pop_back();

                i--;
            }
        }
    }
}

void editorclass::removehook(std::string t)
{
    //Check the hooklist for the hook t. If it's there, remove it from here and the script
    for(int i=0; i<numhooks; i++)
    {
        if(hooklist[i]==t)
        {
            removehookfromscript(t);
            for(int j=i; j<numhooks; j++)
            {
                hooklist[j]=hooklist[j+1];
            }
            hooklist[numhooks]="";
            numhooks--;
            i--;
        }
    }
}

void editorclass::addhook(std::string t)
{
    //Add an empty function to the list in both editor and script
    removehook(t);
    hooklist[numhooks]=t;
    numhooks++;
    addhooktoscript(t);
}

bool editorclass::checkhook(std::string t)
{
    //returns true if hook t already is in the list
    for(int i=0; i<numhooks; i++)
    {
        if(hooklist[i]==t) return true;
    }
    return false;
}


void editorclass::clearscriptbuffer()
{
    for(int i=0; i<sblength+1; i++)
    {
        sb[i]="";
    }
    sblength=1;
}

void editorclass::removeline(int t)
{
    //Remove line t from the script
    if(sblength>0)
    {
        if(sblength==t)
        {
            sblength--;
        }
        else
        {
            for(int i=t; i<sblength; i++)
            {
                sb[i]=sb[i+1];
            }
            sb[sblength]="";
            sblength--;
        }
    }
}

void editorclass::insertline(int t)
{
    //insert a blank line into script at line t
    for(int i=sblength; i>=t; i--)
    {
        sb[i+1]=sb[i];
    }
    sb[t]="";
    sblength++;
}

void editorclass::getlin(KeyPoll& key, enum textmode mode, std::string prompt,
                         std::string *ptr) {
    ed.textmod = mode;
    ed.textptr = ptr;
    ed.textdesc = prompt;
    key.enabletextentry();
    if (ptr)
        key.keybuffer = *ptr;
    else {
        key.keybuffer = "";
        ed.textptr = &(key.keybuffer);
    }

    ed.oldenttext = key.keybuffer;
}

void editorclass::loadlevel( int rxi, int ryi, int altstate )
{
    //Set up our buffer array to be picked up by mapclass
    rxi -= 100;
    ryi -= 100;
    if (rxi < 0) rxi += mapwidth;
    if (ryi < 0) ryi += mapheight;
    if (rxi >= mapwidth) rxi -= mapwidth;
    if (ryi >= mapheight) ryi -= mapheight;

    int tower = get_tower(rxi, ryi);

    if (tower) {
        int ymax = tower_size(tower);
        for (int y = 0; y < ymax; y++)
            for (int x = 0; x < 40; x++)
                swapmap[x + y*40] = towers[tower-1].tiles[x + y*40];

        return;
    }

    int thisstate = -1;
    if (altstate != 0)
        thisstate = getedaltstatenum(rxi, ryi, altstate);

    if (thisstate == -1) { // Didn't find the alt state, or not using one
        for (int j = 0; j < 30; j++)
            for (int i = 0; i < 40; i++)
                swapmap[i+(j*40)]=contents[i+(rxi*40)+vmult[j+(ryi*30)]];
    } else {
        for (int j = 0; j < 30; j++)
            for (int i = 0; i < 40; i++)
                swapmap[i + j*40] = altstates[thisstate].tiles[i + j*40];
    }
}

int editorclass::getlevelcol(int t)
{
    if(level[t].tileset==0)  //Station
    {
        if (level[t].tilecol == -1)
            // Fix gray enemies
            grayenemieskludge = true;
        return level[t].tilecol;
    }
    else if(level[t].tileset==1)   //Outside
    {
        return 32+level[t].tilecol;
    }
    else if(level[t].tileset==2)   //Lab
    {
        return 40+level[t].tilecol;
    }
    else if(level[t].tileset==3)   //Warp Zone
    {
        if (level[t].tilecol == 6)
            // Fix gray enemies
            grayenemieskludge = true;
        return 46+level[t].tilecol;
    }
    else if(level[t].tileset==4)   //Ship
    {
        return 52+level[t].tilecol;
    }
    else if (level[t].tileset==5)   //Tower
    {
        return 58 + level[t].tilecol/5;
    }
    return 0;
}

int editorclass::getenemycol(int t)
{
    switch(t)
    {
        //RED
    case 3:
    case 7:
    case 12:
    case 23:
    case 28:
    case 34:
    case 42:
    case 48:
    case 58:
    case 59:
        return 6;
        break;
        //GREEN
    case 5:
    case 9:
    case 22:
    case 25:
    case 29:
    case 31:
    case 38:
    case 46:
    case 52:
    case 53:
    case 61:
        return 7;
        break;
        //BLUE
    case 1:
    case 6:
    case 14:
    case 27:
    case 33:
    case 44:
    case 50:
    case 57:
        return 12;
        break;
        //YELLOW
    case 4:
    case 17:
    case 24:
    case 30:
    case 37:
    case 45:
    case 51:
    case 55:
    case 60:
        return 9;
        break;
        //PURPLE
    case 2:
    case 11:
    case 15:
    case 19:
    case 32:
    case 36:
    case 49:
    case 63:
        return 20;
        break;
        //CYAN
    case 8:
    case 10:
    case 13:
    case 18:
    case 26:
    case 35:
    case 41:
    case 47:
    case 54:
    case 62:
        return 11;
        break;
        //PINK
    case 16:
    case 20:
    case 39:
    case 43:
    case 56:
    case 64:
        return 8;
        break;
        //ORANGE
    case 21:
    case 40:
        return 17;
        break;
    default:
        return 6;
        break;
    }
    return 0;
}

int editorclass::getwarpbackground(int rx, int ry)
{
    int tmp=rx+(maxwidth*ry);
    switch(level[tmp].tileset)
    {
    case 0: //Space Station
        switch(level[tmp].tilecol)
        {
        case 0:
            return 3;
            break;
        case 1:
            return 2;
            break;
        case 2:
            return 1;
            break;
        case 3:
            return 4;
            break;
        case 4:
            return 5;
            break;
        case 5:
            return 3;
            break;
        case 6:
            return 1;
            break;
        case 7:
            return 0;
            break;
        case 8:
            return 5;
            break;
        case 9:
            return 0;
            break;
        case 10:
            return 2;
            break;
        case 11:
            return 1;
            break;
        case 12:
            return 5;
            break;
        case 13:
            return 0;
            break;
        case 14:
            return 3;
            break;
        case 15:
            return 2;
            break;
        case 16:
            return 4;
            break;
        case 17:
            return 0;
            break;
        case 18:
            return 3;
            break;
        case 19:
            return 1;
            break;
        case 20:
            return 4;
            break;
        case 21:
            return 5;
            break;
        case 22:
            return 1;
            break;
        case 23:
            return 4;
            break;
        case 24:
            return 5;
            break;
        case 25:
            return 0;
            break;
        case 26:
            return 3;
            break;
        case 27:
            return 1;
            break;
        case 28:
            return 5;
            break;
        case 29:
            return 4;
            break;
        case 30:
            return 5;
            break;
        case 31:
            return 2;
            break;
        default:
            return 6;
            break;
        }
        break;
    case 1: //Outside
        switch(level[tmp].tilecol)
        {
        case 0:
            return 3;
            break;
        case 1:
            return 1;
            break;
        case 2:
            return 0;
            break;
        case 3:
            return 2;
            break;
        case 4:
            return 4;
            break;
        case 5:
            return 5;
            break;
        case 6:
            return 2;
            break;
        case 7:
            return 4;
            break;
        default:
            return 6;
            break;
        }
        break;
    case 2: //Lab
        switch(level[tmp].tilecol)
        {
        case 0:
            return 0;
            break;
        case 1:
            return 1;
            break;
        case 2:
            return 2;
            break;
        case 3:
            return 3;
            break;
        case 4:
            return 4;
            break;
        case 5:
            return 5;
            break;
        case 6:
            return 6;
            break;
        default:
            return 6;
            break;
        }
        break;
    case 3: //Warp Zone
        switch(level[tmp].tilecol)
        {
        case 0:
            return 0;
            break;
        case 1:
            return 1;
            break;
        case 2:
            return 2;
            break;
        case 3:
            return 3;
            break;
        case 4:
            return 4;
            break;
        case 5:
            return 5;
            break;
        case 6:
            return 6;
            break;
        default:
            return 6;
            break;
        }
        break;
    case 4: //Ship
        switch(level[tmp].tilecol)
        {
        case 0:
            return 5;
            break;
        case 1:
            return 0;
            break;
        case 2:
            return 4;
            break;
        case 3:
            return 2;
            break;
        case 4:
            return 3;
            break;
        case 5:
            return 1;
            break;
        case 6:
            return 6;
            break;
        default:
            return 6;
            break;
        }
        break;
    case 5: //Tower
        temp = (level[tmp].tilecol) / 5;
        switch(temp)
        {
        case 0:
            return 1;
            break;
        case 1:
            return 4;
            break;
        case 2:
            return 5;
            break;
        case 3:
            return 0;
            break;
        case 4:
            return 3;
            break;
        case 5:
            return 2;
            break;
        default:
            return 6;
            break;
        }
        break;
    default:
        return 6;
        break;
    }
}

int editorclass::getenemyframe(int t)
{
    switch(t)
    {
    case 0:
        return 78;
        break;
    case 1:
        return 88;
        break;
    case 2:
        return 36;
        break;
    case 3:
        return 164;
        break;
    case 4:
        return 68;
        break;
    case 5:
        return 48;
        break;
    case 6:
        return 176;
        break;
    case 7:
        return 168;
        break;
    case 8:
        return 112;
        break;
    case 9:
        return 114;
        break;
    case 10:
        return 92;
        break;
    case 11:
        return 40;
        break;
    case 12:
        return 28;
        break;
    case 13:
        return 32;
        break;
    case 14:
        return 100;
        break;
    case 15:
        return 52;
        break;
    case 16:
        return 54;
        break;
    case 17:
        return 51;
        break;
    case 18:
        return 156;
        break;
    case 19:
        return 44;
        break;
    case 20:
        return 106;
        break;
    case 21:
        return 82;
        break;
    case 22:
        return 116;
        break;
    case 23:
        return 64;
        break;
    case 24:
        return 56;
        break;
    default:
        return 78;
        break;
    }
    return 78;
}


void editorclass::placetile( int x, int y, int t )
{
    // Unused, no need to add altstates support to this function
    if(x>=0 && y>=0 && x<mapwidth*40 && y<mapheight*30)
    {
        contents[x+(levx*40)+vmult[y+(levy*30)]]=t;
    }
}

void editorclass::placetilelocal( int x, int y, int t )
{
    if(x>=0 && y>=0 && x<40 && y<30)
        settilelocal(x, y, t);
    updatetiles=true;
}

int editorclass::gettilelocal(int x, int y)
{
    int tower = get_tower(levx, levy);
    if (tower) {
        y += ypos;

        // Show spikes beyond the tower boundaries
        if (y < 0)
            return 159;
        if (y >= tower_size(tower))
            return 158;

        // Mark tower entry point for current screen with green
        int tile = towers[tower-1].tiles[x + y*40];
        int entrypos = level[levx + levy*maxwidth].tower_row;
        if (y >= entrypos && y <= (entrypos + 29) && tile)
            tile += 300;

        return tile;
    }

    if (levaltstate == 0)
        return contents[x + levx*40 + vmult[y + levy*30]];
    else
        return altstates[getedaltstatenum(levx, levy, levaltstate)].tiles[x + y*40];
}

void editorclass::settilelocal(int x, int y, int tile)
{
    int tower = get_tower(levx, levy);
    if (tower) {
        y += ypos;

        upsize_tower(tower, y);
        if (y < 0)
            y = 0;

        towers[tower-1].tiles[x + y*40] = tile % 30;
        downsize_tower(tower);
    } else if (levaltstate == 0)
        contents[x + levx*40 + vmult[y + levy*30]] = tile;
    else
        altstates[getedaltstatenum(levx, levy, levaltstate)].tiles[x + y*40] = tile;
}

int editorclass::base( int x, int y )
{
    //Return the base tile for the given tileset and colour
    temp=x+(y*maxwidth);
    if(level[temp].tileset==0)  //Space Station
    {
        if(level[temp].tilecol>=22)
        {
            return 483 + ((level[temp].tilecol-22)*3);
        }
        else if(level[temp].tilecol>=11)
        {
            return 283 + ((level[temp].tilecol-11)*3);
        }
        else
        {
            return 83 + (level[temp].tilecol*3);
        }
    }
    else if(level[temp].tileset==1)   //Outside
    {
        return 480 + (level[temp].tilecol*3);
    }
    else if(level[temp].tileset==2)   //Lab
    {
        return 280 + (level[temp].tilecol*3);
    }
    else if(level[temp].tileset==3)   //Warp Zone/Intermission
    {
        return 80 + (level[temp].tilecol*3);
    }
    else if(level[temp].tileset==4)   //SHIP
    {
        return 101 + (level[temp].tilecol*3);
    }
    else if(level[temp].tileset==5)   //Tower
    {
        return 12 + (level[temp].tilecol*30);
    }
    return 0;
}

int editorclass::backbase( int x, int y )
{
    //Return the base tile for the background of the given tileset and colour
    temp=x+(y*maxwidth);
    if(level[temp].tileset==0)  //Space Station
    {
        //Pick depending on tilecol
        switch(level[temp].tilecol)
        {
        case 0:
        case 5:
        case 26:
            return 680; //Blue
            break;
        case 3:
        case 16:
        case 23:
            return 683; //Yellow
            break;
        case 9:
        case 12:
        case 21:
            return 686; //Greeny Cyan
            break;
        case 4:
        case 8:
        case 24:
        case 28:
        case 30:
            return 689; //Green
            break;
        case 20:
        case 29:
            return 692; //Orange
            break;
        case 2:
        case 6:
        case 11:
        case 22:
        case 27:
            return 695; //Red
            break;
        case 1:
        case 10:
        case 15:
        case 19:
        case 31:
            return 698; //Pink
            break;
        case 14:
        case 18:
            return 701; //Dark Blue
            break;
        case 7:
        case 13:
        case 17:
        case 25:
            return 704; //Cyan
            break;
        default:
            return 680;
            break;
        }

    }
    else if(level[temp].tileset==1)   //outside
    {
        return 680 + (level[temp].tilecol*3);
    }
    else if(level[temp].tileset==2)   //Lab
    {
        return 0;
    }
    else if(level[temp].tileset==3)   //Warp Zone/Intermission
    {
        return 120 + (level[temp].tilecol*3);
    }
    else if(level[temp].tileset==4)   //SHIP
    {
        return 741 + (level[temp].tilecol*3);
    }
    else if(level[temp].tileset==5)   //Tower
    {
        return 28 + (level[temp].tilecol*30);
    }
    return 0;
}

enum tiletyp
editorclass::gettiletyplocal(int x, int y)
{
    return gettiletyp(level[levx + levy*maxwidth].tileset, at(x, y));
}

enum tiletyp
editorclass::getabstiletyp(int x, int y)
{
    int tile = absat(&x, &y);
    int room = x / 40 + ((y / 30)*maxwidth);

    return gettiletyp(level[room].tileset, tile);
}

enum tiletyp
editorclass::gettiletyp(int tileset, int tile)
{
    if (tile == 0)
        return TILE_NONE;

    if (tileset == 5) {
        tile = tile % 30;
        if (tile >= 6 && tile <= 11)
            return TILE_SPIKE;
        if (tile >= 12 && tile <= 27)
            return TILE_FOREGROUND;
        return TILE_BACKGROUND;
    }

    // non-space station has more spikes
    int lastspike = 50;
    if (tileset != 0)
        lastspike = 74;

    if ((tile >= 6 && tile <= 9) || (tile >= 49 && tile <= lastspike))
        return TILE_SPIKE;
    if (tile == 1 || (tile >= 80 && tile <= 679))
        return TILE_FOREGROUND;
    return TILE_BACKGROUND;
}

int editorclass::at( int x, int y )
{
    if(x<0) return at(0,y);
    if(y<0) return at(x,0);
    if(x>=40) return at(39,y);
    if(y>=30) return at(x,29);

    return gettilelocal(x, y);
}

int
editorclass::absat(int *x, int *y)
{
    if (*x < 0) *x = (*x) +mapwidth*40;
    if (*y < 0) *y = (*y) +mapheight*30;
    if (*x >= (mapwidth*40)) *x = (*x) - mapwidth*40;
    if (*y >= (mapheight*30)) *y = (*y) - mapheight*30;
    return contents[(*x) + vmult[*y]];
}
int editorclass::freewrap( int x, int y )
{
    temp = getabstiletyp(x, y);
    if (temp != TILE_FOREGROUND) return 0;
    return 1;
}

int editorclass::backonlyfree( int x, int y )
{
    //Returns 1 if tile is a background tile, 0 otherwise
    temp = gettiletyplocal(x, y);
    if (temp == TILE_BACKGROUND)
        return 1;
    return 0;
}

int editorclass::backfree( int x, int y )
{
    //Returns 1 if tile is nonzero
    if (gettiletyplocal(x, y) == TILE_NONE)
        return 0;
    return 1;
}

int editorclass::towerspikefree(int x, int y) {
    // Uses absolute y in tower mode
    int tower = get_tower(levx, levy);
    int size = tower_size(tower);
    if (!intower())
        return spikefree(x, y);

    if (x == -1) return 1;
    if (x == 40) return 1;
    if (y == -1) return 1;
    if (y >= size) return 1;

    int tile = towers[tower-1].tiles[x + y*40];
    temp = gettiletyp(level[levx + levy * maxwidth].tileset, tile);
    if (temp == TILE_FOREGROUND || temp == TILE_SPIKE)
        return 1;
    return 0;
}

int editorclass::spikefree(int x, int y) {
    //Returns 0 if tile is not a block or spike, 1 otherwise
    if(x==-1) return 1;
    if(y==-1) return 1;
    if(x==40) return 1;
    if(y==30) return 1;

    temp = gettiletyplocal(x, y);
    if (temp == TILE_FOREGROUND || temp == TILE_SPIKE)
        return 1;
    return 0;
}

int editorclass::getfree(enum tiletyp thistiletyp)
{
    //Returns 0 if tile is not a block, 1 otherwise
    if (thistiletyp != TILE_FOREGROUND)
        return 0;
    return 1;
}

int editorclass::towerfree(int x, int y) {
    // Uses absolute y in tower mode
    int tower = get_tower(levx, levy);
    int size = tower_size(tower);
    if (!intower())
        return free(x, y);

    if (x == -1) return 1;
    if (x == 40) return 1;
    if (y == -1) return 1;
    if (y >= size) return 1;

    int tile = towers[tower-1].tiles[x + y*40];
    return getfree(gettiletyp(level[levx + levy * maxwidth].tileset,
                              tile));
}

int editorclass::free(int x, int y) {
    //Returns 0 if tile is not a block, 1 otherwise
    if(x==-1) return 1;
    if(y==-1) return 1;
    if(x==40) return 1;
    if(y==30) return 1;

    return getfree(gettiletyplocal(x, y));
}

int editorclass::absfree( int x, int y )
{
    //Returns 0 if tile is not a block, 1 otherwise, abs on grid
    if(x>=0 && y>=0 && x<mapwidth*40 && y<mapheight*30)
        return getfree(getabstiletyp(x, y));
    return 1;
}

int editorclass::match( int x, int y )
{
    if (intower())
        y += ypos;

    if(towerfree(x-1,y)==0 && towerfree(x,y-1)==0 &&
       towerfree(x+1,y)==0 && towerfree(x,y+1)==0) return 0;

    if(towerfree(x-1,y)==0 && towerfree(x,y-1)==0) return 10;
    if(towerfree(x+1,y)==0 && towerfree(x,y-1)==0) return 11;
    if(towerfree(x-1,y)==0 && towerfree(x,y+1)==0) return 12;
    if(towerfree(x+1,y)==0 && towerfree(x,y+1)==0) return 13;

    if(towerfree(x,y-1)==0) return 1;
    if(towerfree(x-1,y)==0) return 2;
    if(towerfree(x,y+1)==0) return 3;
    if(towerfree(x+1,y)==0) return 4;
    if(towerfree(x-1,y-1)==0) return 5;
    if(towerfree(x+1,y-1)==0) return 6;
    if(towerfree(x-1,y+1)==0) return 7;
    if(towerfree(x+1,y+1)==0) return 8;

    return 0;
}

int editorclass::warpzonematch( int x, int y )
{
    if(free(x-1,y)==0 && free(x,y-1)==0 && free(x+1,y)==0 && free(x,y+1)==0) return 0;

    if(free(x-1,y)==0 && free(x,y-1)==0) return 10;
    if(free(x+1,y)==0 && free(x,y-1)==0) return 11;
    if(free(x-1,y)==0 && free(x,y+1)==0) return 12;
    if(free(x+1,y)==0 && free(x,y+1)==0) return 13;

    if(free(x,y-1)==0) return 1;
    if(free(x-1,y)==0) return 2;
    if(free(x,y+1)==0) return 3;
    if(free(x+1,y)==0) return 4;
    if(free(x-1,y-1)==0) return 5;
    if(free(x+1,y-1)==0) return 6;
    if(free(x-1,y+1)==0) return 7;
    if(free(x+1,y+1)==0) return 8;

    return 0;
}

int editorclass::outsidematch( int x, int y )
{

    if(backonlyfree(x-1,y)==0 && backonlyfree(x+1,y)==0) return 2;
    if(backonlyfree(x,y-1)==0 && backonlyfree(x,y+1)==0) return 1;

    return 0;
}

int editorclass::backmatch( int x, int y )
{
    //Returns the first position match for a border
    // 5 1 6
    // 2 X 4
    // 7 3 8
    /*
    if(at(x-1,y)>=80 && at(x,y-1)>=80) return 10;
    if(at(x+1,y)>=80 && at(x,y-1)>=80) return 11;
    if(at(x-1,y)>=80 && at(x,y+1)>=80) return 12;
    if(at(x+1,y)>=80 && at(x,y+1)>=80) return 13;

    if(at(x,y-1)>=80) return 1;
    if(at(x-1,y)>=80) return 2;
    if(at(x,y+1)>=80) return 3;
    if(at(x+1,y)>=80) return 4;
    if(at(x-1,y-1)>=80) return 5;
    if(at(x+1,y-1)>=80) return 6;
    if(at(x-1,y+1)>=80) return 7;
    if(at(x+1,y+1)>=80) return 8;
    */
    if(backfree(x-1,y)==0 && backfree(x,y-1)==0 && backfree(x+1,y)==0 && backfree(x,y+1)==0) return 0;

    if(backfree(x-1,y)==0 && backfree(x,y-1)==0) return 10;
    if(backfree(x+1,y)==0 && backfree(x,y-1)==0) return 11;
    if(backfree(x-1,y)==0 && backfree(x,y+1)==0) return 12;
    if(backfree(x+1,y)==0 && backfree(x,y+1)==0) return 13;

    if(backfree(x,y-1)==0) return 1;
    if(backfree(x-1,y)==0) return 2;
    if(backfree(x,y+1)==0) return 3;
    if(backfree(x+1,y)==0) return 4;
    if(backfree(x-1,y-1)==0) return 5;
    if(backfree(x+1,y-1)==0) return 6;
    if(backfree(x-1,y+1)==0) return 7;
    if(backfree(x+1,y+1)==0) return 8;

    return 0;
}

int editorclass::toweredgetile(int x, int y)
{
    switch(match(x,y))
    {
    case 14: // true center
        return 0;
        break;
    case 10: // top left
        return 5;
        break;
    case 11: // top right
        return 7;
        break;
    case 12: // bottom left
        return 10;
        break;
    case 13: // bottom right
        return 12;
        break;
    case 1: // top center
        return 6;
        break;
    case 2: // center left
        return 8;
        break;
    case 3: // bottom center
        return 11;
        break;
    case 4: // center right
        return 9;
        break;
    case 5: // reversed bottom right edge
        return 4;
        break;
    case 6: // reversed bottom left edge
        return 3;
        break;
    case 7: // reversed top right edge
        return 2;
        break;
    case 8: // reversed top left edge
        return 1;
        break;
    case 0:
    default:
        return 0;
        break;
    }
    return 0;
}
int editorclass::edgetile( int x, int y )
{
    switch(match(x,y))
    {
    case 14: // true center
        return 0;
        break;
    case 10: // top left
        return 80;
        break;
    case 11: // top right
        return 82;
        break;
    case 12: // bottom left
        return 160;
        break;
    case 13: // bottom right
        return 162;
        break;
    case 1: // top center
        return 81;
        break;
    case 2: // center left
        return 120;
        break;
    case 3: // bottom center
        return 161;
        break;
    case 4: // center right
        return 122;
        break;
    case 5: // reversed bottom right edge
        return 42;
        break;
    case 6: // reversed bottom left edge
        return 41;
        break;
    case 7: // reversed top right edge
        return 2;
        break;
    case 8: // reversed top left edge
        return 1;
        break;
    case 0:
    default:
        return 0;
        break;
    }
    return 0;
}

int editorclass::spikebase(int x, int y)
{
    temp=x+(y*maxwidth);
    if (level[temp].tileset==5) {
        return level[temp].tilecol * 30;
    }
    return 0;
}

int editorclass::warpzoneedgetile( int x, int y )
{
    switch(backmatch(x,y))
    {
    case 14:
        return 0;
        break;
    case 10:
        return 80;
        break;
    case 11:
        return 82;
        break;
    case 12:
        return 160;
        break;
    case 13:
        return 162;
        break;
    case 1:
        return 81;
        break;
    case 2:
        return 120;
        break;
    case 3:
        return 161;
        break;
    case 4:
        return 122;
        break;
    case 5:
        return 42;
        break;
    case 6:
        return 41;
        break;
    case 7:
        return 2;
        break;
    case 8:
        return 1;
        break;
    case 0:
    default:
        return 0;
        break;
    }
    return 0;
}

int editorclass::outsideedgetile( int x, int y )
{
    switch(outsidematch(x,y))
    {
    case 2:
        return 0;
        break;
    case 1:
        return 1;
        break;
    case 0:
    default:
        return 2;
        break;
    }
    return 2;
}


int editorclass::backedgetile( int x, int y )
{
    switch(backmatch(x,y))
    {
    case 14:
        return 0;
        break;
    case 10:
        return 80;
        break;
    case 11:
        return 82;
        break;
    case 12:
        return 160;
        break;
    case 13:
        return 162;
        break;
    case 1:
        return 81;
        break;
    case 2:
        return 120;
        break;
    case 3:
        return 161;
        break;
    case 4:
        return 122;
        break;
    case 5:
        return 42;
        break;
    case 6:
        return 41;
        break;
    case 7:
        return 2;
        break;
    case 8:
        return 1;
        break;
    case 0:
    default:
        return 0;
        break;
    }
    return 0;
}

int editorclass::labspikedir( int x, int y, int t )
{
    // a slightly more tricky case
    if(free(x,y+1)==1) return 63 + (t*2);
    if(free(x,y-1)==1) return 64 + (t*2);
    if(free(x-1,y)==1) return 51 + (t*2);
    if(free(x+1,y)==1) return 52 + (t*2);
    return 63 + (t*2);
}

int editorclass::spikedir( int x, int y )
{
    if(free(x,y+1)==1) return 8;
    if(free(x,y-1)==1) return 9;
    if(free(x-1,y)==1) return 49;
    if(free(x+1,y)==1) return 50;
    return 8;
}

int editorclass::towerspikedir(int x, int y) {
    if (intower())
        y += ypos;

    if(towerfree(x,y+1) == 1) return 8;
    if(towerfree(x,y-1) == 1) return 9;
    if(towerfree(x-1,y) == 1) return 10;
    if(towerfree(x+1,y) == 1) return 11;
    return 8;
}

void editorclass::findstartpoint(Game& game)
{
    //Ok! Scan the room for the closest checkpoint
    int testeditor=-1;
    //First up; is there a start point on this screen?
    for(int i=0; i<EditorData::GetInstance().numedentities; i++)
    {
        //if() on screen
        if(edentity[i].t==16 && testeditor==-1)
        {
            testeditor=i;
        }
    }

    if(testeditor==-1)
    {
        game.edsavex = 160;
        game.edsavey = 120;
        game.edsaverx = 100;
        game.edsavery = 100;
        game.edsavegc = 0;
        game.edsavey--;
        game.edsavedir=1;
    }
    else
    {
        //Start point spawn
        int tx=(edentity[testeditor].x-(edentity[testeditor].x%40))/40;
        int ty=(edentity[testeditor].y-(edentity[testeditor].y%30))/30;
        game.edsavex = ((edentity[testeditor].x%40)*8)-4;
        game.edsavey = (edentity[testeditor].y%30)*8;
        game.edsavex += edentity[testeditor].subx;
        game.edsavey += edentity[testeditor].suby;
        game.edsaverx = 100+tx;
        game.edsavery = 100+ty;
        game.edsavegc = 0;
        game.edsavey--;
        game.edsavedir=1-edentity[testeditor].p1;
    }
}

void editorclass::saveconvertor()
{
    // Unused, no need to add altstates support to this function

    //In the case of resizing breaking a level, this function can fix it
    maxwidth=20;
    maxheight=20;
    int oldwidth=10, oldheight=10;

    growing_vector <int> tempcontents;
    for (int j = 0; j < 30 * oldwidth; j++)
    {
        for (int i = 0; i < 40 * oldheight; i++)
        {
            tempcontents.push_back(contents[i+(j*40*oldwidth)]);
        }
    }

    contents.clear();
    for (int j = 0; j < 30 * maxheight; j++)
    {
        for (int i = 0; i < 40 * maxwidth; i++)
        {
            contents.push_back(0);
        }
    }

    for (int j = 0; j < 30 * oldheight; j++)
    {
        for (int i = 0; i < 40 * oldwidth; i++)
        {
            contents[i+(j*40*oldwidth)]=tempcontents[i+(j*40*oldwidth)];
        }
    }

    tempcontents.clear();

    for (int i = 0; i < 30 * maxheight; i++)
    {
        vmult.push_back(int(i * 40 * maxwidth));
    }

    for (int j = 0; j < maxheight; j++)
    {
        for (int i = 0; i < maxwidth; i++)
        {
            level[i+(j*maxwidth)].tilecol=(i+j)%6;
        }
    }
    contents.clear();

}

int editorclass::findtrinket(int t)
{
    int ttrinket=0;
    for(int i=0; i<EditorData::GetInstance().numedentities; i++)
    {
        if(i==t) return ttrinket;
        if(edentity[i].t==9) ttrinket++;
    }
    return 0;
}

int editorclass::findcoin(int t)
{
    int tcoin=0;
    for(int i=0; i<EditorData::GetInstance().numedentities; i++)
    {
        if(i==t) return tcoin;
        if(edentity[i].t==8) tcoin++;
    }
    return 0;
}

int editorclass::findcrewmate(int t)
{
    int ttrinket=0;
    for(int i=0; i<EditorData::GetInstance().numedentities; i++)
    {
        if(i==t) return ttrinket;
        if(edentity[i].t==15) ttrinket++;
    }
    return 0;
}

int editorclass::findwarptoken(int t)
{
    int ttrinket=0;
    for(int i=0; i<EditorData::GetInstance().numedentities; i++)
    {
        if(i==t) return ttrinket;
        if(edentity[i].t==13) ttrinket++;
    }
    return 0;
}

void editorclass::countstuff()
{
    numtrinkets=0;
    numcoins=0;
    numcrewmates=0;
    for(int i=0; i<EditorData::GetInstance().numedentities; i++)
    {
        if(edentity[i].t==9) numtrinkets++;
        if(edentity[i].t==8) numcoins++;
        if(edentity[i].t==15) numcrewmates++;
    }
}

// Switches tileset
void editorclass::switch_tileset(bool reversed) {
    std::string tilesets[6] =
        {"Space Station", "Outside", "Lab", "Warp Zone", "Ship", "Tower"};
    int tiles = level[levx + levy*maxwidth].tileset;
    int oldtiles = tiles;
    if (reversed)
        tiles--;
    else
        tiles++;

    tiles = mod(tiles, 6);
    level[levx + levy*maxwidth].tileset = tiles;
    int newtiles = tiles;

    clamp_tilecol(levx, levy, false);

    switch_tileset_tiles(oldtiles, newtiles);
    notedelay = 45;
    ed.note = "Now using "+tilesets[tiles]+" Tileset";
    updatetiles = true;
}

// Gracefully switches to and from Tower Tileset if autotilig is on
void editorclass::switch_tileset_tiles(int from, int to) {
    // Do nothing in Direct Mode
    if (level[levx + levy*maxwidth].directmode)
        return;

    // Otherwise, set tiles naively to one of the correct type.
    // Autotiling will fix them automatically later.
    int tile;
    enum tiletyp typ;
    int newfg = 80;
    int newbg = 680;
    int newspike = 6;

    if (to == 5) {
        newfg = 12;
        newbg = 28;
    }

    for (int x = 0; x < 40; x++) {
        for (int y = 0; y < 30; y++) {
            tile = gettilelocal(x, y);
            typ = gettiletyp(from, tile);
            if (typ == TILE_FOREGROUND)
                settilelocal(x, y, newfg);
            else if (typ == TILE_BACKGROUND)
                settilelocal(x, y, newbg);
            else if (typ == TILE_SPIKE)
                settilelocal(x, y, newspike);
        }
    }
}

// Switches tileset color
void editorclass::switch_tilecol(bool reversed) {
    if (reversed)
        level[levx + levy*maxwidth].tilecol--;
    else
        level[levx + levy*maxwidth].tilecol++;

    clamp_tilecol(levx, levy, true);

    notedelay = 45;
    ed.note = "Tileset Colour Changed";
    updatetiles = true;
}

void editorclass::clamp_tilecol(int levx, int levy, bool wrap) {
    int tileset = level[levx + levy*maxwidth].tileset;
    int tilecol = level[levx + levy*maxwidth].tilecol;

    int mincol = -1;
    int maxcol = 5;

    // Only Space Station allows tileset -1
    if (tileset != 0)
        mincol = 0;

    if (tileset == 0)
        maxcol = 31;
    else if (tileset == 1)
        maxcol = 7;
    else if (tileset == 3)
        maxcol = 6;
    else if (tileset == 5)
        maxcol = 29;

    // If wrap is true, wrap-around, otherwise just cap
    if (tilecol > maxcol)
        tilecol = (wrap ? mincol : maxcol);
    if (tilecol < mincol)
        tilecol = (wrap ? maxcol : mincol);
    level[levx + levy*maxwidth].tilecol = tilecol;
}

// Performs tasks needed when enabling Tower Mode
void editorclass::enable_tower(void) {
    int room = levx + levy*maxwidth;

    // Set Tower Tileset and color 0
    level[room].tileset = 5;
    level[room].tilecol = 0;

    /* Place the player at the level's tower destination.
       Defaults to zero, but might be something else if we've
       had tower mode enabled in this room previously. */
    ypos = level[room].tower_row;

    // If we have an adjacant tower room, reuse its tower
    int rx = levx;
    int ry = levy;
    int tower = 0;
    if (get_tower(rx, ry - 1))
        tower = get_tower(rx, ry - 1);
    else if (get_tower(rx, ry + 1))
        tower = get_tower(rx, ry + 1);

    if (!tower) {
        // Find an unused tower ID
        int i;
        bool unused = false;
        for (i = 1; i <= maxwidth * maxheight; i++) {
            unused = true;

            for (rx = 0; rx < maxwidth && unused; rx++)
                for (ry = 0; ry < maxheight && unused; ry++)
                    if (get_tower(rx, ry) == i)
                        unused = false;

            if (unused)
                break;
        }

        tower = i;
    }

    level[room].tower = tower;
    snap_tower_entry(levx, levy);
}

// Move tower entry and editor position within tower boundaries
void editorclass::snap_tower_entry(int rx, int ry) {
    int tower = get_tower(rx, ry);
    int size = tower_size(tower);

    // Snap editor position to the whole tower bottom or top with a 10 offset
    if (ypos > size - 20)
        ypos = size - 20;

    if (ypos < -10)
        ypos = -10;

    // Snap entry row to the bottom row.
    // Useful to avoid using the room as exit point.
    if (level[rx + ry*maxwidth].tower_row >= size)
        level[rx + ry*maxwidth].tower_row = size - 1;
}

// Enlarge a tower, downwards to y or shifting down if y is negative
void editorclass::upsize_tower(int tower, int y)
{
    if (!y || !tower)
        return;

    // Check if we actually need to upsize it
    if (y > 0 && towers[tower-1].size > y)
        return;

    if (y > 0) {
        towers[tower-1].size = y + 1;
        resize_tower_tiles(tower);
        return;
    }

    towers[tower-1].size = towers[tower-1].size - y;
    resize_tower_tiles(tower);
    shift_tower(tower, -y);
}

// Remove vertical edges lacking tiles (down to a minimum of 40)
void editorclass::downsize_tower(int tower) {
    if (!tower)
        return;

    int ty, by, size;
    size = tower_size(tower);

    // Check unused topmost edges
    for (ty = 0; ty < size * 40; ty++)
        if (towers[tower-1].tiles[ty] != 0)
            break;
    ty /= 40;

    // Don't resize below 40
    if (ty > (size - 40))
        ty = size - 40;
    if (ty > 0) {
        shift_tower(tower, -ty);
        towers[tower-1].size -= ty;
        resize_tower_tiles(tower);
    }

    // Check unused bottom edges
    size = tower_size(tower);
    for (by = size * 40 - 1; by; by--)
        if (towers[tower-1].tiles[by] != 0)
            break;
    by = size * 40 - 1 - by;
    by /= 40;

    if (by > (size - 40))
        by = size - 40;
    if (by > 0) {
        towers[tower-1].size -= by;
        resize_tower_tiles(tower);
    }
}

// Resizes the tower tile size. If enlarged, zerofill the new tiles
void editorclass::resize_tower_tiles(int tower) {
    if (!tower)
        return;

    int oldsize = towers[tower-1].tiles.size() / 40 + 1;
    int newsize = tower_size(tower);
    towers[tower-1].tiles.resize(40 * newsize);

    // Zerofill new rows
    for (; oldsize < newsize; oldsize++)
        for (int x = 0; x < 40; x++)
            towers[tower-1].tiles[x + oldsize*40] = 0;
}

// Shift tower downwards (positive y) or upwards (negative y).
// Also shifts tower entry position
void editorclass::shift_tower(int tower, int y) {
    if (!tower || !y)
        return;

    int x, ny, size;
    size = tower_size(tower);

    // Shift entry points
    for (int rx = 0; rx < maxwidth; rx++) {
        for (int ry = 0; ry < maxheight; ry++) {
            if (tower == get_tower(rx, ry)) {
                level[rx + ry*maxwidth].tower_row += y;
                if (level[rx + ry*maxwidth].tower_row < 0)
                    level[rx + ry*maxwidth].tower_row = 0;
                if (level[rx + ry*maxwidth].tower_row >= size)
                    level[rx + ry*maxwidth].tower_row = size - 1;
            }
        }
    }

    // Shift entities
    for (int i = 0; i < EditorData::GetInstance().numedentities; i++)
        if (tower == edentity[i].intower)
            edentity[i].y += y;

    // Shift editor scroll position
    ypos += y;

    // Shift tower downwards
    if (y > 0) {
        for (ny = size - 1; ny >= 0; ny--) {
            for (x = 0; x < 40; x++) {
                if (ny >= y)
                    towers[tower-1].tiles[x + ny*40] =
                        towers[tower-1].tiles[x + (ny - y)*40];
                else
                    towers[tower-1].tiles[x + ny*40] = 0;
            }
        }

        return;
    }

    // Shift tower upwards
    for (ny = 0; ny < size + y; ny++)
        for (x = 0; x < 40; x++)
            towers[tower-1].tiles[x + ny*40] =
                towers[tower-1].tiles[x + (ny - y)*40];
}

int editorclass::get_tower(int rx, int ry) {
    /* Returns the tower of this room */

    int room = rx + ry * maxwidth;
    if (ry < 0 || rx < 0 || rx >= maxwidth || ry >= maxheight)
        return 0;

    return level[room].tower;
}

int editorclass::tower_size(int tower) {
    if (!tower)
        return 0;

    return towers[tower-1].size;
}
int editorclass::tower_scroll(int tower) {
    return towers[tower-1].scroll;
}

bool editorclass::intower(void) {
    return !!get_tower(levx, levy);
}

int editorclass::tower_row(int rx, int ry) {
    int tower = get_tower(rx, ry);
    if (!tower)
        return -1;

    int room = rx + ry * maxwidth;
    return level[room].tower_row;
}

void editorclass::load(std::string& _path, Graphics& dwgfx, mapclass& map, Game& game)
{
    reset();
    map.teleporters.clear();
    game.customtrials.clear();

    static const char *levelDir = "levels/";
    if (_path.compare(0, strlen(levelDir), levelDir) != 0)
    {
        _path = levelDir + _path;
    }

    char** path = PHYSFS_getSearchPath();
    char** i = path;
    int len = 0;
    while (*i != nullptr) {
        i++;
        len++;
    }

    //printf("Unmounting %s\n", dwgfx.assetdir.c_str());
    //PHYSFS_unmount(dwgfx.assetdir.c_str());
    //dwgfx.assetdir = "";
    //dwgfx.reloadresources();

    FILESYSTEM_unmountassets(dwgfx);

    std::string dirpath = "levels/" + _path.substr(7,_path.size()-14) + "/";
    if (FILESYSTEM_directoryExists(dirpath.c_str())) {
        if (!game.quiet) printf("Custom asset directory exists at %s\n",dirpath.c_str());
        FILESYSTEM_mount(dirpath.c_str(), dwgfx);
        dwgfx.reloadresources();
        music.init();
    } else if (!game.quiet) {
        printf("Custom asset directory does not exist\n");
    }

    TiXmlDocument doc;
    if (!FILESYSTEM_loadTiXmlDocument(_path.c_str(), &doc))
    {
        printf("No level %s to load :(\n", _path.c_str());
        return;
    }


    TiXmlHandle hDoc(&doc);
    TiXmlElement* pElem;
    TiXmlHandle hRoot(0);
    version = 0;

    {
        pElem=hDoc.FirstChildElement().Element();
        // should always have a valid root but handle gracefully if it does
        if (!pElem)
        {
            printf("No valid root! Corrupt level file?\n");
        }

        pElem->QueryIntAttribute("version", &version);
        // save this for later
        hRoot=TiXmlHandle(pElem);
    }

    for( pElem = hRoot.FirstChild( "Data" ).FirstChild().Element(); pElem; pElem=pElem->NextSiblingElement())
    {
        std::string pKey(pElem->Value());
        const char* pText = pElem->GetText() ;
        if(pText == NULL)
        {
            pText = "";
        }

        if (pKey == "MetaData")
        {

            for( TiXmlElement* subElem = pElem->FirstChildElement(); subElem; subElem= subElem->NextSiblingElement())
            {
                std::string pKey(subElem->Value());
                const char* pText = subElem->GetText() ;
                if(pText == NULL)
                {
                    pText = "";
                }

                if(pKey == "Creator")
                {
                    EditorData::GetInstance().creator = pText;
                }

                if(pKey == "Title")
                {
                    EditorData::GetInstance().title = pText;
                }

                if(pKey == "Desc1")
                {
                    Desc1 = pText;
                }

                if(pKey == "Desc2")
                {
                    Desc2 = pText;
                }

                if(pKey == "Desc3")
                {
                    Desc3 = pText;
                }

                if(pKey == "website")
                {
                    website = pText;
                }
            }
        }

        if (pKey == "mapwidth")
        {
            mapwidth = atoi(pText);
        }
        if (pKey == "mapheight")
        {
            mapheight = atoi(pText);
        }
        if (pKey == "levmusic")
        {
            levmusic = atoi(pText);
        }

        if (pKey == "timetrials")
        {
            for( TiXmlElement* trialEl = pElem->FirstChildElement(); trialEl; trialEl=trialEl->NextSiblingElement())
            {
                customtrial temp;
                trialEl->QueryIntAttribute("roomx",    &temp.roomx    );
                trialEl->QueryIntAttribute("roomy",    &temp.roomy    );
                trialEl->QueryIntAttribute("startx",   &temp.startx   );
                trialEl->QueryIntAttribute("starty",   &temp.starty   );
                trialEl->QueryIntAttribute("startf",   &temp.startf   );
                trialEl->QueryIntAttribute("par",      &temp.par      );
                trialEl->QueryIntAttribute("trinkets", &temp.trinkets );
                trialEl->QueryIntAttribute("music",    &temp.music );
                if(trialEl->GetText() != NULL)
                {
                    temp.name = std::string(trialEl->GetText()) ;
                } else {
                    temp.name = "???";
                }
                game.customtrials.push_back(temp);

            }

        }


        if (pKey == "contents")
        {
            std::string TextString = (pText);
            if(TextString.length())
            {
                growing_vector<std::string> values = split(TextString,',');
                //contents.clear();
                for(size_t i = 0; i < contents.size(); i++)
                {
                    contents[i] =0;
                }
                int x =0;
                int y =0;
                for(size_t i = 0; i < values.size(); i++)
                {
                    contents[x + (maxwidth*40*y)] = atoi(values[i].c_str());
                    x++;
                    if(x == mapwidth*40)
                    {
                        x=0;
                        y++;
                    }

                }
            }
        }

        if (pKey == "altstates") {
            int i = 0;
            for (TiXmlElement* edAltstateEl = pElem->FirstChildElement(); edAltstateEl; edAltstateEl = edAltstateEl->NextSiblingElement()) {
                std::string pKey(edAltstateEl->Value());
                const char* pText = edAltstateEl->GetText();

                if (pText == NULL)
                    pText = "";

                // Do we NEED the parentheses around `pText`? Whatever
                std::string TextString = (pText);

                if (TextString.length()) {
                    edAltstateEl->QueryIntAttribute("x", &altstates[i].x);
                    edAltstateEl->QueryIntAttribute("y", &altstates[i].y);
                    edAltstateEl->QueryIntAttribute("state", &altstates[i].state);

                    growing_vector<std::string> values = split(TextString, ',');

                    for (size_t t = 0; t < values.size(); t++)
                        altstates[i].tiles[t] = atoi(values[t].c_str());

                    i++;
                }
            }
        }

        if (pKey == "towers") {
            int i = 0;
            for (TiXmlElement *edTowerEl = pElem->FirstChildElement();
                 edTowerEl; edTowerEl = edTowerEl->NextSiblingElement()) {
                std::string pKey(edTowerEl->Value());
                const char* pText = edTowerEl->GetText();

                if (pText == NULL)
                    pText = "";

                std::string TextString = pText;

                if (TextString.length()) {
                    edTowerEl->QueryIntAttribute("size", &towers[i].size);
                    edTowerEl->QueryIntAttribute("scroll", &towers[i].scroll);

                    growing_vector<std::string> values = split(TextString, ',');

                    for (size_t t = 0; t < values.size(); t++)
                        towers[i].tiles[t] = atoi(values[t].c_str());

                    i++;
                }
            }
        }

        /*else if(version==1){
          if (pKey == "contents")
          {
            std::string TextString = (pText);
            if(TextString.length())
            {
              growing_vector<std::string> values = split(TextString,',');
              contents.clear();
              for(int i = 0; i < values.size(); i++)
              {
                contents.push_back(atoi(values[i].c_str()));
              }
            }
          }
        //}
        */

        if (pKey == "teleporters")
        {
            for( TiXmlElement* teleporterEl = pElem->FirstChildElement(); teleporterEl; teleporterEl=teleporterEl->NextSiblingElement())
            {
                point temp;
                teleporterEl->QueryIntAttribute("x", &temp.x);
                teleporterEl->QueryIntAttribute("y", &temp.y);

                map.setteleporter(temp.x,temp.y);

            }

        }

        if (pKey == "edEntities")
        {
            int i = 0;
            for( TiXmlElement* edEntityEl = pElem->FirstChildElement(); edEntityEl; edEntityEl=edEntityEl->NextSiblingElement())
            {
                std::string pKey(edEntityEl->Value());
                //const char* pText = edEntityEl->GetText() ;
                if(edEntityEl->GetText() != NULL)
                {
                    edentity[i].scriptname = std::string(edEntityEl->GetText()) ;
                }

                if (edEntityEl->Attribute("activityname")) {
                    edentity[i].activityname = edEntityEl->Attribute("activityname");
                } else {
                    edentity[i].activityname = "";
                }

                if (edEntityEl->Attribute("activitycolor")) {
                    edentity[i].activitycolor = edEntityEl->Attribute("activitycolor");
                } else {
                    edentity[i].activitycolor = "";
                }

                edEntityEl->QueryIntAttribute("x", &edentity[i].x);
                edEntityEl->QueryIntAttribute("y", &edentity[i].y);
                edEntityEl->QueryIntAttribute("subx", &edentity[i].subx);
                edEntityEl->QueryIntAttribute("suby", &edentity[i].suby);
                edEntityEl->QueryIntAttribute("t", &edentity[i].t);

                edEntityEl->QueryIntAttribute("p1", &edentity[i].p1);
                edEntityEl->QueryIntAttribute("p2", &edentity[i].p2);
                edEntityEl->QueryIntAttribute("p3", &edentity[i].p3);
                edEntityEl->QueryIntAttribute("p4", &edentity[i].p4);
                edEntityEl->QueryIntAttribute("p5", &edentity[i].p5);
                edEntityEl->QueryIntAttribute("p6", &edentity[i].p6);

                edEntityEl->QueryIntAttribute("state", &edentity[i].state);
                edEntityEl->QueryIntAttribute("intower", &edentity[i].intower);

                edEntityEl->QueryIntAttribute("onetime", (int*) &edentity[i].onetime);

                i++;

            }

            EditorData::GetInstance().numedentities = i;
        }

        if (pKey == "levelMetaData")
        {
            int i = 0;
            int rowwidth = 0;
            int maxrowwidth = std::max(mapwidth, 20);
            for( TiXmlElement* edLevelClassElement = pElem->FirstChildElement(); edLevelClassElement; edLevelClassElement=edLevelClassElement->NextSiblingElement())
            {
                std::string pKey(edLevelClassElement->Value());
                if(edLevelClassElement->GetText() != NULL)
                {
                    level[i].roomname = std::string(edLevelClassElement->GetText()) ;
                }

                edLevelClassElement->QueryIntAttribute("tileset", &level[i].tileset);
                edLevelClassElement->QueryIntAttribute("tilecol", &level[i].tilecol);
                edLevelClassElement->QueryIntAttribute("platx1", &level[i].platx1);
                edLevelClassElement->QueryIntAttribute("platy1", &level[i].platy1);
                edLevelClassElement->QueryIntAttribute("platx2", &level[i].platx2);
                edLevelClassElement->QueryIntAttribute("platy2", &level[i].platy2);
                edLevelClassElement->QueryIntAttribute("platv", &level[i].platv);
                if (edLevelClassElement->Attribute("enemyv")) {
                    edLevelClassElement->QueryIntAttribute("enemyv", &level[i].enemyv);
                } else {
                    level[i].enemyv = 4;
                }
                edLevelClassElement->QueryIntAttribute("enemyx1", &level[i].enemyx1);
                edLevelClassElement->QueryIntAttribute("enemyy1", &level[i].enemyy1);
                edLevelClassElement->QueryIntAttribute("enemyx2", &level[i].enemyx2);
                edLevelClassElement->QueryIntAttribute("enemyy2", &level[i].enemyy2);
                edLevelClassElement->QueryIntAttribute("enemytype", &level[i].enemytype);
                edLevelClassElement->QueryIntAttribute("directmode", &level[i].directmode);
                edLevelClassElement->QueryIntAttribute("tower", &level[i].tower);
                edLevelClassElement->QueryIntAttribute("tower_row", &level[i].tower_row);
                edLevelClassElement->QueryIntAttribute("warpdir", &level[i].warpdir);

                i++;

                rowwidth++;
                if (rowwidth == maxrowwidth) {
                    rowwidth = 0;
                    i += maxwidth - maxrowwidth;
                }
            }
        }

        if (pKey == "script")
        {
            std::string TextString = (pText);
            if(TextString.length())
            {
                growing_vector<std::string> values = split(TextString,'|');
                script.clearcustom();
                for(size_t i = 0; i < values.size(); i++)
                {
                    script.customscript.push_back(values[i]);
                }

            }
        }

    }

    gethooks();
    countstuff();
    version=2;
    //saveconvertor();
}

void editorclass::save(std::string& _path, mapclass& map, Game& game)
{
    TiXmlDocument doc;
    TiXmlElement* msg;
    TiXmlDeclaration* decl = new TiXmlDeclaration( "1.0", "", "" );
    doc.LinkEndChild( decl );

    TiXmlElement * root = new TiXmlElement( "MapData" );
    root->SetAttribute("version",version);
    root->SetAttribute("vceversion",1);
    doc.LinkEndChild( root );

    TiXmlComment * comment = new TiXmlComment();
    comment->SetValue(" Save file " );
    root->LinkEndChild( comment );

    TiXmlElement * data = new TiXmlElement( "Data" );
    root->LinkEndChild( data );

    msg = new TiXmlElement( "MetaData" );

    time_t rawtime;
    struct tm * timeinfo;

    time ( &rawtime );
    timeinfo = localtime ( &rawtime );

    std::string timeAndDate = asctime (timeinfo);
    //timeAndDate += dateStr;

    EditorData::GetInstance().timeModified =  timeAndDate;
    if(EditorData::GetInstance().timeModified == "")
    {
        EditorData::GetInstance().timeCreated =  timeAndDate;
    }

    //getUser
    TiXmlElement* meta = new TiXmlElement( "Creator" );
    meta->LinkEndChild( new TiXmlText( EditorData::GetInstance().creator.c_str() ));
    msg->LinkEndChild( meta );

    meta = new TiXmlElement( "Title" );
    meta->LinkEndChild( new TiXmlText( EditorData::GetInstance().title.c_str() ));
    msg->LinkEndChild( meta );

    meta = new TiXmlElement( "Created" );
    meta->LinkEndChild( new TiXmlText( UtilityClass::String(version).c_str() ));
    msg->LinkEndChild( meta );

    meta = new TiXmlElement( "Modified" );
    meta->LinkEndChild( new TiXmlText( EditorData::GetInstance().modifier.c_str() ) );
    msg->LinkEndChild( meta );

    meta = new TiXmlElement( "Modifiers" );
    meta->LinkEndChild( new TiXmlText( UtilityClass::String(version).c_str() ));
    msg->LinkEndChild( meta );

    meta = new TiXmlElement( "Desc1" );
    meta->LinkEndChild( new TiXmlText( Desc1.c_str() ));
    msg->LinkEndChild( meta );

    meta = new TiXmlElement( "Desc2" );
    meta->LinkEndChild( new TiXmlText( Desc2.c_str() ));
    msg->LinkEndChild( meta );

    meta = new TiXmlElement( "Desc3" );
    meta->LinkEndChild( new TiXmlText( Desc3.c_str() ));
    msg->LinkEndChild( meta );

    meta = new TiXmlElement( "website" );
    meta->LinkEndChild( new TiXmlText( website.c_str() ));
    msg->LinkEndChild( meta );

    data->LinkEndChild( msg );

    msg = new TiXmlElement( "mapwidth" );
    msg->LinkEndChild( new TiXmlText( UtilityClass::String(mapwidth).c_str() ));
    data->LinkEndChild( msg );

    msg = new TiXmlElement( "mapheight" );
    msg->LinkEndChild( new TiXmlText( UtilityClass::String(mapheight).c_str() ));
    data->LinkEndChild( msg );

    msg = new TiXmlElement( "levmusic" );
    msg->LinkEndChild( new TiXmlText( UtilityClass::String(levmusic).c_str() ));
    data->LinkEndChild( msg );

    //New save format
    std::string contentsString="";
    for(int y = 0; y < mapheight*30; y++ )
    {
        for(int x = 0; x < mapwidth*40; x++ )
        {
            contentsString += UtilityClass::String(contents[x + (maxwidth*40*y)]) + ",";
        }
    }
    msg = new TiXmlElement( "contents" );
    msg->LinkEndChild( new TiXmlText( contentsString.c_str() ));
    data->LinkEndChild( msg );

    msg = new TiXmlElement("altstates");

    // Iterate through all the altstates. Nonexistent altstates are ones at -1,-1
    TiXmlElement* alt;
    for (size_t a = 0; a < altstates.size(); a++) {
        if (altstates[a].x == -1 or altstates[a].y == -1)
            continue;

        std::string tiles = "";
        for (int y = 0; y < 30; y++)
            for (int x = 0; x < 40; x++)
                tiles += UtilityClass::String(altstates[a].tiles[x + y*40]) + ",";

        alt = new TiXmlElement("altstate");
        alt->SetAttribute("x", altstates[a].x);
        alt->SetAttribute("y", altstates[a].y);
        alt->SetAttribute("state", altstates[a].state);
        alt->LinkEndChild(new TiXmlText(tiles.c_str()));
        msg->LinkEndChild(alt);

        a++;
    }
    data->LinkEndChild(msg);

    msg = new TiXmlElement("towers");

    // Figure out amount of towers used
    int twx, twy;
    int max_tower = 0;
    for (twx = 0; twx < maxwidth; twx++)
        for (twy = 0; twy < maxheight; twy++)
            if (max_tower < get_tower(twx, twy))
                max_tower = get_tower(twx, twy);

    TiXmlElement* tw;
    for (int t = 0; t < max_tower; t++) {
        // Don't save unused towers
        bool found = false;
        for (twx = 0; twx < maxwidth && !found; twx++)
            for (twy = 0; twy < maxheight && !found; twy++)
                if ((t + 1) == get_tower(twx, twy))
                    found = true;

        if (!found) {
            for (int u = (t + 1); u < max_tower; u++) {
                towers[u - 1].size = towers[u].size;
                towers[u - 1].scroll = towers[u].scroll;
                towers[u - 1].tiles.resize(40 * towers[u - 1].size);
                for (int i = 0; i < 40 * towers[u - 1].size; i++)
                    towers[u - 1].tiles[i] = towers[u].tiles[i];
            }

            for (twx = 0; twx < maxwidth; twx++)
                for (twy = 0; twy < maxheight; twy++)
                    if (level[twx + twy * maxwidth].tower > t)
                        level[twx + twy * maxwidth].tower--;

            t--;
            max_tower--;
            continue;
        }

        std::string tiles = "";
        for (int y = 0; y < towers[t].size; y++)
            for (int x = 0; x < 40; x++)
                tiles += UtilityClass::String(towers[t].tiles[x + y*40]) + ",";

        tw = new TiXmlElement("tower");
        tw->SetAttribute("size", towers[t].size);
        tw->SetAttribute("scroll", towers[t].scroll);
        tw->LinkEndChild(new TiXmlText(tiles.c_str()));
        msg->LinkEndChild(tw);
    }
    data->LinkEndChild(msg);

    //Old save format
    /*
    std::string contentsString;
    for(int i = 0; i < contents.size(); i++ )
    {
    	contentsString += UtilityClass::String(contents[i]) + ",";
    }
    msg = new TiXmlElement( "contents" );
    msg->LinkEndChild( new TiXmlText( contentsString.c_str() ));
    data->LinkEndChild( msg );
    */

    msg = new TiXmlElement( "teleporters" );
    for(size_t i = 0; i < map.teleporters.size(); i++)
    {
        TiXmlElement *teleporterElement = new TiXmlElement( "teleporter" );
        teleporterElement->SetAttribute( "x", map.teleporters[i].x);
        teleporterElement->SetAttribute( "y", map.teleporters[i].y);
        msg->LinkEndChild( teleporterElement );
    }

    data->LinkEndChild( msg );

    msg = new TiXmlElement( "timetrials" );
    for(int i = 0; i < (int)game.customtrials.size(); i++) {
        TiXmlElement *trialElement = new TiXmlElement( "trial" );
        trialElement->SetAttribute( "roomx",    game.customtrials[i].roomx   );
        trialElement->SetAttribute( "roomy",    game.customtrials[i].roomy   );
        trialElement->SetAttribute( "startx",   game.customtrials[i].startx  );
        trialElement->SetAttribute( "starty",   game.customtrials[i].starty  );
        trialElement->SetAttribute( "startf",   game.customtrials[i].startf  );
        trialElement->SetAttribute( "par",      game.customtrials[i].par     );
        trialElement->SetAttribute( "trinkets", game.customtrials[i].trinkets);
        trialElement->SetAttribute( "music",    game.customtrials[i].music   );
        trialElement->LinkEndChild( new TiXmlText( game.customtrials[i].name.c_str() )) ;
        msg->LinkEndChild( trialElement );
    }

    data->LinkEndChild( msg );

    msg = new TiXmlElement( "edEntities" );
    for(int i = 0; i < EditorData::GetInstance().numedentities; i++)
    {
        TiXmlElement *edentityElement = new TiXmlElement( "edentity" );
        edentityElement->SetAttribute( "x", edentity[i].x);
        edentityElement->SetAttribute(  "y", edentity[i].y);
        edentityElement->SetAttribute( "subx", edentity[i].subx);
        edentityElement->SetAttribute(  "suby", edentity[i].suby);
        edentityElement->SetAttribute(  "t", edentity[i].t);
        edentityElement->SetAttribute(  "p1", edentity[i].p1);
        edentityElement->SetAttribute(  "p2", edentity[i].p2);
        edentityElement->SetAttribute(  "p3", edentity[i].p3);
        edentityElement->SetAttribute( "p4", edentity[i].p4);
        edentityElement->SetAttribute( "p5", edentity[i].p5);
        edentityElement->SetAttribute(  "p6", edentity[i].p6);
        if (edentity[i].state != 0)
                edentityElement->SetAttribute("state", edentity[i].state);
        edentityElement->SetAttribute("intower", edentity[i].intower);
        if (edentity[i].activityname != "") {
            edentityElement->SetAttribute(  "activityname", edentity[i].activityname.c_str());
        }
        if (edentity[i].activitycolor != "") {
            edentityElement->SetAttribute(  "activitycolor", edentity[i].activitycolor.c_str());
        }
        if (edentity[i].onetime)
            edentityElement->SetAttribute("onetime", help.String((int) edentity[i].onetime).c_str());
        edentityElement->LinkEndChild( new TiXmlText( edentity[i].scriptname.c_str() )) ;
        msg->LinkEndChild( edentityElement );
    }

    data->LinkEndChild( msg );

    msg = new TiXmlElement( "levelMetaData" );
    int rowwidth = 0;
    int maxrowwidth = std::max(mapwidth, 20);
    int rows = 0;
    int maxrows = mapwidth <= 20 && mapheight <= 20 ? 20 : mapheight;
    for (int i = 0; i < maxwidth * maxheight; i++) {
        TiXmlElement *edlevelclassElement = new TiXmlElement( "edLevelClass" );
        edlevelclassElement->SetAttribute( "tileset", level[i].tileset);
        edlevelclassElement->SetAttribute(  "tilecol", level[i].tilecol);
        edlevelclassElement->SetAttribute(  "platx1", level[i].platx1);
        edlevelclassElement->SetAttribute(  "platy1", level[i].platy1);
        edlevelclassElement->SetAttribute(  "platx2", level[i].platx2);
        edlevelclassElement->SetAttribute( "platy2", level[i].platy2);
        edlevelclassElement->SetAttribute( "platv", level[i].platv);
        edlevelclassElement->SetAttribute( "enemyv", level[i].enemyv);
        edlevelclassElement->SetAttribute(  "enemyx1", level[i].enemyx1);
        edlevelclassElement->SetAttribute(  "enemyy1", level[i].enemyy1);
        edlevelclassElement->SetAttribute(  "enemyx2", level[i].enemyx2);
        edlevelclassElement->SetAttribute(  "enemyy2", level[i].enemyy2);
        edlevelclassElement->SetAttribute(  "enemytype", level[i].enemytype);
        edlevelclassElement->SetAttribute(  "directmode", level[i].directmode);
        edlevelclassElement->SetAttribute(  "tower", level[i].tower);
        edlevelclassElement->SetAttribute(  "tower_row", level[i].tower_row);
        edlevelclassElement->SetAttribute(  "warpdir", level[i].warpdir);

        edlevelclassElement->LinkEndChild( new TiXmlText( level[i].roomname.c_str() )) ;
        msg->LinkEndChild( edlevelclassElement );

        rowwidth++;
        if (rowwidth == maxrowwidth) {
            rowwidth = 0;
            i += maxwidth - maxrowwidth;
            rows++;
            if (rows == maxrows)
                break;
        }
    }
    data->LinkEndChild( msg );

    std::string scriptString;
    for(size_t i = 0; i < script.customscript.size(); i++ )
    {
        scriptString += script.customscript[i] + "|";
    }
    msg = new TiXmlElement( "script" );
    msg->LinkEndChild( new TiXmlText( scriptString.c_str() ));
    data->LinkEndChild( msg );

    FILESYSTEM_saveTiXmlDocument(("levels/" + _path).c_str(), &doc);
}


void addedentity( int xp, int yp, int tp, int p1/*=0*/, int p2/*=0*/, int p3/*=0*/, int p4/*=0*/, int p5/*=320*/, int p6/*=240*/)
{
    int tower = ed.get_tower(ed.levx, ed.levy);
    edentity[EditorData::GetInstance().numedentities].x=xp;
    edentity[EditorData::GetInstance().numedentities].y=yp;
    edentity[EditorData::GetInstance().numedentities].subx=0;
    edentity[EditorData::GetInstance().numedentities].suby=0;
    edentity[EditorData::GetInstance().numedentities].t=tp;
    edentity[EditorData::GetInstance().numedentities].p1=p1;
    edentity[EditorData::GetInstance().numedentities].p2=p2;
    edentity[EditorData::GetInstance().numedentities].p3=p3;
    edentity[EditorData::GetInstance().numedentities].p4=p4;
    edentity[EditorData::GetInstance().numedentities].p5=p5;
    edentity[EditorData::GetInstance().numedentities].p6=p6;
    edentity[EditorData::GetInstance().numedentities].state=ed.levaltstate;
    edentity[EditorData::GetInstance().numedentities].intower=tower;
    edentity[EditorData::GetInstance().numedentities].scriptname="";
    edentity[EditorData::GetInstance().numedentities].activityname="";
    edentity[EditorData::GetInstance().numedentities].activitycolor="";
    edentity[EditorData::GetInstance().numedentities].onetime = false;

    EditorData::GetInstance().numedentities++;
}

void naddedentity( int xp, int yp, int tp, int p1/*=0*/, int p2/*=0*/, int p3/*=0*/, int p4/*=0*/, int p5/*=320*/, int p6/*=240*/)
{
    int tower = ed.get_tower(ed.levx, ed.levy);
    edentity[EditorData::GetInstance().numedentities].x=xp;
    edentity[EditorData::GetInstance().numedentities].y=yp;
    edentity[EditorData::GetInstance().numedentities].subx=0;
    edentity[EditorData::GetInstance().numedentities].suby=0;
    edentity[EditorData::GetInstance().numedentities].t=tp;
    edentity[EditorData::GetInstance().numedentities].p1=p1;
    edentity[EditorData::GetInstance().numedentities].p2=p2;
    edentity[EditorData::GetInstance().numedentities].p3=p3;
    edentity[EditorData::GetInstance().numedentities].p4=p4;
    edentity[EditorData::GetInstance().numedentities].p5=p5;
    edentity[EditorData::GetInstance().numedentities].p6=p6;
    edentity[EditorData::GetInstance().numedentities].state=ed.levaltstate;
    edentity[EditorData::GetInstance().numedentities].intower=tower;
    edentity[EditorData::GetInstance().numedentities].scriptname="";
    edentity[EditorData::GetInstance().numedentities].activityname="";
    edentity[EditorData::GetInstance().numedentities].activitycolor="";
    edentity[EditorData::GetInstance().numedentities].onetime = false;
}

void copyedentity( int a, int b )
{
    edentity[a].x=edentity[b].x;
    edentity[a].y=edentity[b].y;
    edentity[a].subx=edentity[b].subx;
    edentity[a].suby=edentity[b].suby;
    edentity[a].t=edentity[b].t;
    edentity[a].p1=edentity[b].p1;
    edentity[a].p2=edentity[b].p2;
    edentity[a].p3=edentity[b].p3;
    edentity[a].p4=edentity[b].p4;
    edentity[a].p5=edentity[b].p5;
    edentity[a].p6=edentity[b].p6;
    edentity[a].state=edentity[b].state;
    edentity[a].intower=edentity[b].intower;
    edentity[a].scriptname=edentity[b].scriptname;
    edentity[a].activityname=edentity[b].activityname;
    edentity[a].activitycolor=edentity[b].activitycolor;
    edentity[a].onetime = edentity[b].onetime;
}

void removeedentity( int t )
{
    if(t==EditorData::GetInstance().numedentities-1)
    {
        EditorData::GetInstance().numedentities--;
    }
    else
    {
        for(int m=t; m<EditorData::GetInstance().numedentities; m++) copyedentity(m,m+1);
        EditorData::GetInstance().numedentities--;
    }
}

int edentat(int x, int y, int state, int tower) {
    for(int i=0; i<EditorData::GetInstance().numedentities; i++)
        if (edentity[i].x==x && edentity[i].y==y &&
            edentity[i].state==state && edentity[i].intower==tower)
            return i;
    return -1;
}

bool edentclear(int x, int y, int state, int tower) {
    if (edentat(x, y, state, tower) >= 0)
        return false;
    return true;
}

void fillbox( Graphics& dwgfx, int x, int y, int x2, int y2, int c )
{
    FillRect(dwgfx.backBuffer, x, y, x2-x, 1, c);
    FillRect(dwgfx.backBuffer, x, y2-1, x2-x, 1, c);
    FillRect(dwgfx.backBuffer, x, y, 1, y2-y, c);
    FillRect(dwgfx.backBuffer, x2-1, y, 1, y2-y, c);
}

void fillboxabs( Graphics& dwgfx, int x, int y, int x2, int y2, int c )
{
    FillRect(dwgfx.backBuffer, x, y, x2, 1, c);
    FillRect(dwgfx.backBuffer, x, y+y2-1, x2, 1, c);
    FillRect(dwgfx.backBuffer, x, y, 1, y2, c);
    FillRect(dwgfx.backBuffer, x+x2-1, y, 1, y2, c);
}


extern editorclass ed;
extern growing_vector<edentities> edentity;

extern int temp;

extern scriptclass script;

void editorclass::generatecustomminimap(Graphics& dwgfx, mapclass& map)
{
    map.customwidth=mapwidth;
    map.customheight=mapheight;

    map.customzoom=1;
    if(map.customwidth<=10 && map.customheight<=10) map.customzoom=2;
    if(map.customwidth<=5 && map.customheight<=5) map.customzoom=4;

    //Set minimap offsets
    if(map.customzoom==4)
    {
        map.custommmxoff=24*(5-map.customwidth);
        map.custommmxsize=240-(map.custommmxoff*2);

        map.custommmyoff=18*(5-map.customheight);
        map.custommmysize=180-(map.custommmyoff*2);
    }
    else if(map.customzoom==2)
    {
        map.custommmxoff=12*(10-map.customwidth);
        map.custommmxsize=240-(map.custommmxoff*2);

        map.custommmyoff=9*(10-map.customheight);
        map.custommmysize=180-(map.custommmyoff*2);
    }
    else
    {
        map.custommmxoff=6*(20-map.customwidth);
        map.custommmxsize=240-(map.custommmxoff*2);

        map.custommmyoff=int(4.5*(20-map.customheight));
        map.custommmysize=180-(map.custommmyoff*2);
    }

    if (auto mapimage = dwgfx.mapimage) {
        SDL_FreeSurface(dwgfx.images[12]);
        dwgfx.images[12] = LoadImage(mapimage->c_str());
        return;
    }


    FillRect(dwgfx.images[12], dwgfx.getRGB(0,0,0));

    int tm=0;
    int temp=0;
    //Scan over the map size
    if(mapheight<=5 && mapwidth<=5)
    {
        //4x map
        for(int j2=0; j2<mapheight; j2++)
        {
            for(int i2=0; i2<mapwidth; i2++)
            {
                //Ok, now scan over each square
                tm=196;
                if(level[i2 + (j2*maxwidth)].tileset==1) tm=96;

                for(int j=0; j<36; j++)
                {
                    for(int i=0; i<48; i++)
                    {
                        temp=absfree(int(i*0.83) + (i2*40),int(j*0.83)+(j2*30));
                        if(temp>=1)
                        {
                            //Fill in this pixel
                            FillRect(dwgfx.images[12], (i2*48)+i, (j2*36)+j, 1, 1, dwgfx.getRGB(tm, tm, tm));
                        }
                    }
                }
            }
        }
    }
    else if(mapheight<=10 && mapwidth<=10)
    {
        //2x map
        for(int j2=0; j2<mapheight; j2++)
        {
            for(int i2=0; i2<mapwidth; i2++)
            {
                //Ok, now scan over each square
                tm=196;
                if(level[i2 + (j2*maxwidth)].tileset==1) tm=96;

                for(int j=0; j<18; j++)
                {
                    for(int i=0; i<24; i++)
                    {
                        temp=absfree(int(i*1.6) + (i2*40),int(j*1.6)+(j2*30));
                        if(temp>=1)
                        {
                            //Fill in this pixel
                            FillRect(dwgfx.images[12], (i2*24)+i, (j2*18)+j, 1, 1, dwgfx.getRGB(tm, tm, tm));
                        }
                    }
                }
            }
        }
    }
    else
    {
        for(int j2=0; j2<mapheight; j2++)
        {
            for(int i2=0; i2<mapwidth; i2++)
            {
                //Ok, now scan over each square
                tm=196;
                if(level[i2 + (j2*maxwidth)].tileset==1) tm=96;

                for(int j=0; j<9; j++)
                {
                    for(int i=0; i<12; i++)
                    {
                        temp=absfree(3+(i*3) + (i2*40),(j*3)+(j2*30));
                        if(temp>=1)
                        {
                            //Fill in this pixel
                            FillRect(dwgfx.images[12], (i2*12)+i, (j2*9)+j, 1, 1, dwgfx.getRGB(tm, tm, tm));
                        }
                    }
                }
            }
        }
    }
}

int
dmcap(void)
{
    if (ed.level[ed.levx+(ed.levy*ed.maxwidth)].tileset == 5)
        return 900;
    return 1200;
}

int
dmwidth(void)
{
    if (ed.level[ed.levx+(ed.levy*ed.maxwidth)].tileset == 5)
        return 30;
    return 40;
}

void editorrender( KeyPoll& key, Graphics& dwgfx, Game& game, mapclass& map, entityclass& obj, UtilityClass& help )
{
    //TODO
    //dwgfx.backbuffer.lock();

    //Draw grid

    FillRect(dwgfx.backBuffer, 0, 0, 320,240, dwgfx.getRGB(0,0,0));
    for(int j=0; j<30; j++)
    {
        for(int i=0; i<40; i++)
        {
            fillbox(dwgfx, i*8, j*8, (i*8)+7, (j*8)+7, dwgfx.getRGB(8,8,8)); //a simple grid
            if(i%4==0) fillbox(dwgfx, i*8, j*8, (i*8)+7, (j*8)+7, dwgfx.getRGB(16,16,16));
            if(j%4==0) fillbox(dwgfx, i*8, j*8, (i*8)+7, (j*8)+7, dwgfx.getRGB(16,16,16));

            //Minor guides
            if(i==9) fillbox(dwgfx, i*8, j*8, (i*8)+7, (j*8)+7, dwgfx.getRGB(24,24,24));
            if(i==30) fillbox(dwgfx, i*8, j*8, (i*8)+7, (j*8)+7, dwgfx.getRGB(24,24,24));
            if(j==6 || j==7) fillbox(dwgfx, i*8, j*8, (i*8)+7, (j*8)+7, dwgfx.getRGB(24,24,24));
            if(j==21 || j==22) fillbox(dwgfx, i*8, j*8, (i*8)+7, (j*8)+7, dwgfx.getRGB(24,24,24));

            //Major guides
            if(i==20 || i==19) fillbox(dwgfx, i*8, j*8, (i*8)+7, (j*8)+7, dwgfx.getRGB(32,32,32));
            if(j==14) fillbox(dwgfx, i*8, j*8, (i*8)+7, (j*8)+7, dwgfx.getRGB(32,32,32));
        }
    }

    //Or draw background
    //dwgfx.drawbackground(1, map);
    if(!ed.settingsmod)
    {
        switch(ed.level[ed.levx+(ed.levy*ed.maxwidth)].warpdir)
        {
        case 1:
            dwgfx.rcol=ed.getwarpbackground(ed.levx, ed.levy);
            dwgfx.drawbackground(3, map);
            break;
        case 2:
            dwgfx.rcol=ed.getwarpbackground(ed.levx, ed.levy);
            dwgfx.drawbackground(4, map);
            break;
        case 3:
            dwgfx.rcol=ed.getwarpbackground(ed.levx, ed.levy);
            dwgfx.drawbackground(5, map);
            break;
        default:
            break;
        }

        if (ed.level[ed.levx+(ed.levy*ed.maxwidth)].tower)
            dwgfx.drawbackground(9, map);
    }

    //Draw map, in function
    int temp;
    if(ed.level[ed.levx+(ed.maxwidth*ed.levy)].tileset==0 || ed.level[ed.levx+(ed.maxwidth*ed.levy)].tileset==10)
    {
        for (int j = 0; j < 30; j++)
        {
            for (int i = 0; i < 40; i++)
            {
                temp = ed.gettilelocal(i, j);
                if(temp>0) dwgfx.drawtile(i*8,j*8,temp,0,0,0);
            }
        }
    }
    else if(ed.level[ed.levx+(ed.maxwidth*ed.levy)].tileset==5)
    {
        for (int j = 0; j < 30; j++)
        {
            for (int i = 0; i < 40; i++)
            {
                temp = ed.gettilelocal(i, j);
                if(temp>0) dwgfx.drawtile3(i*8,j*8,temp,0,0,0);
            }
        }
    }
    else
    {
        for (int j = 0; j < 30; j++)
        {
            for (int i = 0; i < 40; i++)
            {
                temp = ed.gettilelocal(i, j);
                if(temp>0) dwgfx.drawtile2(i*8,j*8,temp,0,0,0);
            }
        }
    }

    //Edge tile fix

    //Buffer the sides of the new room with tiles from other rooms, to ensure no gap problems.
    for(int j=0; j<30; j++)
    {
        //left edge
        if(ed.freewrap((ed.levx*40)-1,j+(ed.levy*30))==1)
        {
            FillRect(dwgfx.backBuffer, 0,j*8, 2,8, dwgfx.getRGB(255,255,255-help.glow));
        }
        //right edge
        if(ed.freewrap((ed.levx*40)+40,j+(ed.levy*30))==1)
        {
            FillRect(dwgfx.backBuffer, 318,j*8, 2,8, dwgfx.getRGB(255,255,255-help.glow));
        }
    }

    for(int i=0; i<40; i++)
    {
        if(ed.freewrap((ed.levx*40)+i,(ed.levy*30)-1)==1)
        {
            FillRect(dwgfx.backBuffer, i*8,0, 8,2, dwgfx.getRGB(255,255,255-help.glow));
        }

        if(ed.freewrap((ed.levx*40)+i,30+(ed.levy*30))==1)
        {
            FillRect(dwgfx.backBuffer, i*8,238, 8,2, dwgfx.getRGB(255,255,255-help.glow));
        }
    }

    std::string rmstr;
    rmstr = "("+help.String(ed.levx+1)+","+help.String(ed.levy+1)+")";
    int tower = ed.get_tower(ed.levx, ed.levy);
    if (tower)
        rmstr += "T" + help.String(tower) + ":" + help.String(ed.ypos);
    else if (ed.levaltstate != 0)
        rmstr += "@" + help.String(ed.levaltstate);

    int rmstrx = 318 - rmstr.length() * 8;

    //Draw entities
    game.customcol=ed.getlevelcol(ed.levx+(ed.levy*ed.maxwidth))+1;
    ed.entcol=ed.getenemycol(game.customcol);
    if (ed.grayenemieskludge) {
        ed.entcol = 18;
        ed.grayenemieskludge = false;
    }
    obj.customplatformtile=game.customcol*12;

    int tx = ed.tilex;
    int ty = ed.tiley;
    if (!tower) {
        tx += ed.levx * 40;
        ty += ed.levy * 30;
    } else
        ty += ed.ypos;

    ed.temp=edentat(tx, ty, ed.levaltstate, tower);

    // Iterate backwards to make the editor draw in the same order as ingame
    for(int i=EditorData::GetInstance().numedentities - 1; i >= 0; i--) {
        // Entity locations
        int ex = edentity[i].x;
        int ey = edentity[i].y;
        if (!tower) {
            ex -= ed.levx * 40;
            ey -= ed.levy * 30;
        } else
            ey -= ed.ypos;

        ex *= 8;
        ey *= 8;

        // Warp line/gravity line area
        tx = ex / 8;
        ty = ey / 8;
        int tx2 = ex / 8;
        int ty2 = ey / 8;
        if (tower) {
            ty += ed.ypos;
            ty2 += ed.ypos;
        }
        ex += edentity[i].subx;
        ey += edentity[i].suby;

        int len;
        SDL_Rect drawRect;

        // WARNING: Don't get any bright ideas about reducing indentation by negating this conditional and using a `continue`
        if (edentity[i].state == ed.levaltstate &&
            edentity[i].intower == tower &&
            (tower || (ex >= 0 && ex < 320 && ey >= 0 && ey < 240))) {

            switch(edentity[i].t) {
            case 1: // Enemies
                dwgfx.drawspritesetcol(ex, ey, ed.getenemyframe(ed.level[ed.levx+(ed.levy*ed.maxwidth)].enemytype),ed.entcol,help);
                if(edentity[i].p1==0)
                    dwgfx.Print(ex+4,ey+4, "V", 255, 255, 255 - help.glow, false);
                if(edentity[i].p1==1)
                    dwgfx.Print(ex+4,ey+4, "^", 255, 255, 255 - help.glow, false);
                if(edentity[i].p1==2)
                    dwgfx.Print(ex+4,ey+4, "<", 255, 255, 255 - help.glow, false);
                if(edentity[i].p1==3)
                    dwgfx.Print(ex+4,ey+4, ">", 255, 255, 255 - help.glow, false);
                break;
            case 2: // Moving platforms, conveyors
            case 3: // Disappearing platforms
                drawRect = dwgfx.tiles_rect;
                drawRect.x += ex;
                drawRect.y += ey;

                len = 32;
                if (edentity[i].t == 2 && edentity[i].p1 >= 7)
                    len *= 2;
                while (drawRect.x < (ex + len)) {
                    BlitSurfaceStandard(dwgfx.entcolours[obj.customplatformtile],
                                        NULL, dwgfx.backBuffer, &drawRect);
                    drawRect.x += 8;
                }

                fillboxabs(dwgfx, ex, ey, len, 8, dwgfx.getBGR(255, 255, 255));
                if (edentity[i].t == 3) {
                    dwgfx.Print(ex, ey, "////", 255, 255, 255 - help.glow, false);
                    break;
                }

                if (edentity[i].p1 == 5) {
                    dwgfx.Print(ex, ey, ">>>>", 255, 255, 255 - help.glow, false);
                    break;
                }

                if (edentity[i].p1 == 6) {
                    dwgfx.Print(ex, ey, "<<<<", 255, 255, 255 - help.glow, false);
                    break;
                }

                if (edentity[i].p1 == 7) {
                    dwgfx.Print(ex, ey, "> > > >", 255, 255, 255 - help.glow,
                                false);
                    break;
                }

                if (edentity[i].p1 == 8) {
                    dwgfx.Print(ex, ey, "< < < <", 255, 255, 255 - help.glow,
                                false);
                    break;
                }

                if(edentity[i].p1==0)
                    dwgfx.Print(ex+12,ey, "V", 255, 255, 255 - help.glow, false);
                if(edentity[i].p1==1)
                    dwgfx.Print(ex+12,ey, "^", 255, 255, 255 - help.glow, false);
                if(edentity[i].p1==2)
                    dwgfx.Print(ex+12,ey, "<", 255, 255, 255 - help.glow, false);
                if(edentity[i].p1==3)
                    dwgfx.Print(ex+12,ey, ">", 255, 255, 255 - help.glow, false);
                break;
            case 5: // Flip Tokens
                dwgfx.drawspritesetcol(ex, ey, 192, obj.crewcolour(0), help);
                //dwgfx.drawsprite(ex, ty, 16 + !edentity[i].p1, 96, 96, 96);
                fillboxabs(dwgfx, ex, ey, 16, 16, dwgfx.getRGB(164,164,255));
                break;
            case 8: // Coin
                dwgfx.drawhuetile(ex, ey, 48, 8);
                //dwgfx.drawsprite(ex, ey, 22, 196, 196, 196);
                fillboxabs(dwgfx, ex, ey, 8, 8, dwgfx.getRGB(164,164,164));
                break;
            case 9: // Shiny Trinket
                dwgfx.drawsprite(ex, ey, 22, 196, 196, 196);
                fillboxabs(dwgfx, ex, ey, 16, 16, dwgfx.getRGB(164, 164, 255));
                break;
            case 10: // Checkpoints
                if (edentity[i].p1 == 0 || edentity[i].p1 == 1)
                    dwgfx.drawsprite(ex, ey, 20 + edentity[i].p1, 196, 196, 196);
                else
                    dwgfx.drawsprite(ex, ey, 188 + edentity[i].p1, 196, 196, 196);
                fillboxabs(dwgfx, ex, ey, 16, 16, dwgfx.getRGB(164, 164, 255));
                break;
            case 11: // Gravity lines
                fillboxabs(dwgfx, ex, ey, 8, 8, dwgfx.getRGB(164,255,164));
                if(edentity[i].p1 == 0) { //Horizontal
                    while (!ed.spikefree(tx, ey / 8)) tx--;
                    while (!ed.spikefree(tx2, ey / 8)) tx2++;
                    tx++;
                    FillRect(dwgfx.backBuffer, (tx*8), ey+4, (tx2-tx)*8, 1,
                             dwgfx.getRGB(194,194,194));
                    edentity[i].p2 = tx;
                    edentity[i].p3 = (tx2-tx)*8;
                } else { // Vertical
                    while (!ed.towerspikefree(tx, ty)) ty--;
                    while (!ed.towerspikefree(tx, ty2)) ty2++;
                    ty++;
                    FillRect(dwgfx.backBuffer, (tx*8)+3, (ty*8) - (ed.ypos*8), 1,
                             (ty2-ty)*8, dwgfx.getRGB(194,194,194));
                    edentity[i].p2 = ty;
                    edentity[i].p3 = (ty2-ty) * 8;
                }
                break;
            case 13: // Warp tokens
                dwgfx.drawsprite(ex, ey, 18+(ed.entframe%2),196,196,196);
                fillboxabs(dwgfx, ex, ey, 16, 16, dwgfx.getRGB(164,164,255));
                if(ed.temp==i)
                    dwgfx.Print(ex, ey - 8,
                                "("+help.String(((edentity[i].p1-int(edentity[i].p1%40))/40)+1)+","+help.String(((edentity[i].p2-int(edentity[i].p2%30))/30)+1)+")",210,210,255);
                else
                    dwgfx.Print(ex, ey - 8,
                                help.String(ed.findwarptoken(i)),210,210,255);
                break;
            case 14: // Teleporter
                dwgfx.drawtele(ex, ey, 1, 100, help);
                fillboxabs(dwgfx, ex, ey, 8*12, 8*12, dwgfx.getRGB(164,164,255));
                break;
            case 15: // Crewmates
                dwgfx.drawspritesetcol(ex - 4, ey, 144,
                                       obj.crewcolour(edentity[i].p1), help);
                fillboxabs(dwgfx, ex, ey, 16, 24, dwgfx.getRGB(164,164,164));
                break;
            case 16: // Start
                if (edentity[i].p1==0) // Left
                    dwgfx.drawspritesetcol(ex - 4, ey, 0,
                                           obj.crewcolour(0), help);
                else if (edentity[i].p1==1)
                    dwgfx.drawspritesetcol(ex - 4, ey, 3,
                                           obj.crewcolour(0), help);
                fillboxabs(dwgfx, ex, ey, 16, 24, dwgfx.getRGB(164,164,164));
                if(ed.entframe<2)
                    dwgfx.Print(ex - 12, ey - 8, "START", 255, 255, 255);
                else
                    dwgfx.Print(ex - 12, ey - 8, "START", 196, 196, 196);
                break;
            case 17: // Roomtext
                if(edentity[i].scriptname.length()<1) {
                    fillboxabs(dwgfx, ex, ey, 8, 8, dwgfx.getRGB(96, 96, 96));
                } else {
                    auto length = utf8::distance(edentity[i].scriptname.begin(),
                                                 edentity[i].scriptname.end());
                    fillboxabs(dwgfx, ex, ey, length*8, 8, dwgfx.getRGB(96,96,96));
                }
                dwgfx.Print(ex, ey, edentity[i].scriptname,
                            196, 196, 255 - help.glow);
                break;
            case 18: // Terminals
                ty = ey;
                if (!edentity[i].p1) // Unflipped
                    ty += 8;

                dwgfx.drawsprite(ex, ty, 16 + !edentity[i].p1, 96, 96, 96);
                fillboxabs(dwgfx, ex, ey, 16, 24, dwgfx.getRGB(164,164,164));
                if(ed.temp==i)
                    dwgfx.Print(ex, ey - 8, edentity[i].scriptname,210,210,255);
                break;
            case 19: // Script Triggers
                fillboxabs(dwgfx, ex, ey, edentity[i].p1*8 + edentity[i].p3, edentity[i].p2*8 + edentity[i].p4,
                           edentity[i].onetime ? dwgfx.getRGB(255,255,164) : dwgfx.getRGB(255,164,255));
                fillboxabs(dwgfx, ex, ey, 8, 8, dwgfx.getRGB(255,255,255));
                if(ed.temp==i)
                    dwgfx.Print(ex, ey - 8, edentity[i].scriptname,210,210,255);
                break;
            case 20: // Activity Zones
                fillboxabs(dwgfx, ex, ey, edentity[i].p1*8 + edentity[i].p3, edentity[i].p2*8 + edentity[i].p4,
                           dwgfx.getRGB(164,255,164));
                fillboxabs(dwgfx, ex, ey, 8, 8, dwgfx.getRGB(255,255,255));
                if(ed.temp==i)
                    dwgfx.Print(ex, ey - 8, edentity[i].scriptname,210,210,255);
                break;
            case 50: // Warp lines
                fillboxabs(dwgfx, ex, ey, 8, 8, dwgfx.getRGB(164,255,164));
                if (edentity[i].p1>=2) { //Horizontal
                    while (!ed.free(tx, ey / 8)) tx--;
                    while (!ed.free(tx2, ey / 8)) tx2++;
                    tx++;
                    fillboxabs(dwgfx, (tx*8), ey+1, (tx2-tx)*8, 6,
                               dwgfx.getRGB(255,255,194));
                    edentity[i].p2=tx;
                    edentity[i].p3=(tx2-tx)*8;
                } else { // Vertical
                    while (!ed.towerfree(tx, ty)) ty--;
                    while (!ed.towerfree(tx, ty2)) ty2++;
                    ty++;
                    fillboxabs(dwgfx, (tx*8)+1, (ty*8) - (ed.ypos*8), 6,
                               (ty2-ty)*8, dwgfx.getRGB(255,255,194));
                    edentity[i].p2=ty;
                    edentity[i].p3=(ty2-ty)*8;
                }
                break;
            case 999: // ?
                //dwgfx.drawspritesetcol(ex, ey, 3, 102, help);
                dwgfx.setcol(102, help);
                dwgfx.drawimage(3, ex, ey);
                //dwgfx.drawsprite(ex, ty, 16 + !edentity[i].p1, 96, 96, 96);
                fillboxabs(dwgfx, ex, ey, 464, 320, dwgfx.getRGB(164,164,255));
                break;
            }
        }

        //Need to also check warp point destinations
        if(edentity[i].t==13 && ed.warpent!=i)
        {
            int ep1 = edentity[i].p1 - ed.levx*40;
            int ep2 = edentity[i].p2 - ed.levy*30;
            if (tower)
                ep2 -= ed.ypos;
            ep1 *= 8;
            ep2 *= 8;
            if (tower || (ep1 >= 0 && ep1 < 320 && ep2 >= 0 && ep2 < 240))
            {
                dwgfx.drawsprite(ep1, ep2, 18 + ed.entframe%2, 64, 64, 64);
                fillboxabs(dwgfx, ep1, ep2, 16, 16, dwgfx.getRGB(64, 64, 96));
                if (ed.tilex == ep1/8 && ed.tiley == ep2/8)
                {
                    dwgfx.bprint(ep1, ep2 - 8, rmstr, 190, 190, 225);
                }
                else
                {
                    dwgfx.bprint(ep1, ep2 - 8, help.String(ed.findwarptoken(i)), 190, 190, 225);
                }
            }
        }
    }

    if(ed.boundarymod>0)
    {
        if(ed.boundarymod==1)
        {
            fillboxabs(dwgfx, ed.tilex*8, ed.tiley*8, 8,8,dwgfx.getRGB(255-(help.glow/2),191+(help.glow),210+(help.glow/2)));
            fillboxabs(dwgfx, (ed.tilex*8)+2, (ed.tiley*8)+2, 4,4,dwgfx.getRGB(128-(help.glow/4),100+(help.glow/2),105+(help.glow/4)));
        }
        else if(ed.boundarymod==2)
        {
            if((ed.tilex*8)+8<=ed.boundx1 || (ed.tiley*8)+8<=ed.boundy1)
            {
                fillboxabs(dwgfx, ed.boundx1, ed.boundy1, 8, 8,dwgfx.getRGB(255-(help.glow/2),191+(help.glow),210+(help.glow/2)));
                fillboxabs(dwgfx, ed.boundx1+2, ed.boundy1+2, 4, 4,dwgfx.getRGB(128-(help.glow/4),100+(help.glow/2),105+(help.glow/4)));
            }
            else
            {
                fillboxabs(dwgfx, ed.boundx1, ed.boundy1, (ed.tilex*8)+8-ed.boundx1,(ed.tiley*8)+8-ed.boundy1,dwgfx.getRGB(255-(help.glow/2),191+(help.glow),210+(help.glow/2)));
                fillboxabs(dwgfx, ed.boundx1+2, ed.boundy1+2, (ed.tilex*8)+8-ed.boundx1-4,(ed.tiley*8)+8-ed.boundy1-4,dwgfx.getRGB(128-(help.glow/4),100+(help.glow/2),105+(help.glow/4)));
            }
        }
    }
    else
    {
        //Draw boundaries
        int tmp=ed.levx+(ed.levy*ed.maxwidth);
        if(ed.level[tmp].enemyx1!=0 && ed.level[tmp].enemyy1!=0
                && ed.level[tmp].enemyx2!=320 && ed.level[tmp].enemyy2!=240)
        {
            fillboxabs(dwgfx,  ed.level[tmp].enemyx1, ed.level[tmp].enemyy1,
                       ed.level[tmp].enemyx2-ed.level[tmp].enemyx1,
                       ed.level[tmp].enemyy2-ed.level[tmp].enemyy1,
                       dwgfx.getBGR(255-(help.glow/2),64,64));
        }

        if(ed.level[tmp].platx1!=0 && ed.level[tmp].platy1!=0
                && ed.level[tmp].platx2!=320 && ed.level[tmp].platy2!=240)
        {
            fillboxabs(dwgfx,  ed.level[tmp].platx1, ed.level[tmp].platy1,
                       ed.level[tmp].platx2-ed.level[tmp].platx1,
                       ed.level[tmp].platy2-ed.level[tmp].platy1,
                       dwgfx.getBGR(64,64,255-(help.glow/2)));
        }
    }

    //Draw connecting map guidelines
    //TODO

    //Draw Cursor
    if (!ed.trialstartpoint) {
        switch(ed.drawmode)
        {
        case 0:
        case 1:
        case 2:
        case 9:
        case 10:
        case 12:
        case 18: //Single point
            fillboxabs(dwgfx, (ed.tilex*8),(ed.tiley*8),8,8, dwgfx.getRGB(200,32,32));
            break;
        case 3:
        case 4:
        case 8:
        case 13:
        case 17: //2x2
            fillboxabs(dwgfx, (ed.tilex*8),(ed.tiley*8),16,16, dwgfx.getRGB(200,32,32));
            break;
        case 5:
        case 6:
        case 7://Platform
            fillboxabs(dwgfx, (ed.tilex*8),(ed.tiley*8),32,8, dwgfx.getRGB(200,32,32));
            break;
        case 14: //X if not on edge
            if(ed.tilex==0 || ed.tilex==39 || ed.tiley==0 || ed.tiley==29)
            {
                fillboxabs(dwgfx, (ed.tilex*8),(ed.tiley*8),8,8, dwgfx.getRGB(200,32,32));
            }
            else
            {
                dwgfx.Print((ed.tilex*8),(ed.tiley*8),"X",255,0,0);
            }
            break;
        case 11:
        case 15:
        case 16: //2x3
            fillboxabs(dwgfx, (ed.tilex*8),(ed.tiley*8),16,24, dwgfx.getRGB(200,32,32));
            break;
        case 19: //12x12 :))))))
            fillboxabs(dwgfx, (ed.tilex*8),(ed.tiley*8),8*12,8*12, dwgfx.getRGB(200,32,32));
            break;
        case -6: // ...?
            fillboxabs(dwgfx, (ed.tilex*8),(ed.tiley*8),464,320, dwgfx.getRGB(200,32,32));
            break;
        }

        if(ed.drawmode<3)
        {
            if(ed.zmod && ed.drawmode<2)
            {
                fillboxabs(dwgfx, (ed.tilex*8)-8,(ed.tiley*8)-8,24,24, dwgfx.getRGB(200,32,32));
            }
            else if(ed.xmod && ed.drawmode<2)
            {
                fillboxabs(dwgfx, (ed.tilex*8)-16,(ed.tiley*8)-16,24+16,24+16, dwgfx.getRGB(200,32,32));
            }
        }
    } else {
        dwgfx.drawspritesetcol((ed.tilex*8) - 4, (ed.tiley*8), 0, obj.crewcolour(0), help);
        fillboxabs(dwgfx, (ed.tilex*8),(ed.tiley*8),16,24, dwgfx.getRGB(200,32,32));
    }

    //If in directmode, show current directmode tile
    if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].directmode==1)
    {
        //Tile box for direct mode
        int t2=0;
        if(ed.dmtileeditor>0)
        {
            ed.dmtileeditor--;
            if(ed.dmtileeditor<=4)
            {
                t2=(4-ed.dmtileeditor)*12;
            }

            //Draw five lines of the editor
            temp=ed.dmtile-(ed.dmtile%dmwidth());
            temp-=dmwidth()*2;
            FillRect(dwgfx.backBuffer, 0,-t2,320,40, dwgfx.getRGB(0,0,0));
            FillRect(dwgfx.backBuffer, 0,-t2+40,320,2, dwgfx.getRGB(255,255,255));
            if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].tileset==0)
            {
                for(int i=0; i<dmwidth(); i++)
                {
                    dwgfx.drawtile(i*8,0-t2,(temp+dmcap()+i)%dmcap(),0,0,0);
                    dwgfx.drawtile(i*8,8-t2,(temp+dmcap()+dmwidth()*1+i)%dmcap(),0,0,0);
                    dwgfx.drawtile(i*8,16-t2,(temp+dmcap()+dmwidth()*2+i)%dmcap(),0,0,0);
                    dwgfx.drawtile(i*8,24-t2,(temp+dmcap()+dmwidth()*3+i)%dmcap(),0,0,0);
                    dwgfx.drawtile(i*8,32-t2,(temp+dmcap()+dmwidth()*4+i)%dmcap(),0,0,0);
                }
            }
            else if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].tileset==5)
            {
                for(int i=0; i<dmwidth(); i++)
                {
                    dwgfx.drawtile3(i*8,0-t2,(temp+dmcap()+i)%dmcap(),0,0,0);
                    dwgfx.drawtile3(i*8,8-t2,(temp+dmcap()+dmwidth()*1+i)%dmcap(),0,0,0);
                    dwgfx.drawtile3(i*8,16-t2,(temp+dmcap()+dmwidth()*2+i)%dmcap(),0,0,0);
                    dwgfx.drawtile3(i*8,24-t2,(temp+dmcap()+dmwidth()*3+i)%dmcap(),0,0,0);
                    dwgfx.drawtile3(i*8,32-t2,(temp+dmcap()+dmwidth()*4+i)%dmcap(),0,0,0);
                }
            }
            else
            {
                for(int i=0; i<dmwidth(); i++)
                {
                    dwgfx.drawtile2(i*8,0-t2,(temp+dmcap()+i)%dmcap(),0,0,0);
                    dwgfx.drawtile2(i*8,8-t2,(temp+dmcap()+dmwidth()*1+i)%dmcap(),0,0,0);
                    dwgfx.drawtile2(i*8,16-t2,(temp+dmcap()+dmwidth()*2+i)%dmcap(),0,0,0);
                    dwgfx.drawtile2(i*8,24-t2,(temp+dmcap()+dmwidth()*3+i)%dmcap(),0,0,0);
                    dwgfx.drawtile2(i*8,32-t2,(temp+dmcap()+dmwidth()*4+i)%dmcap(),0,0,0);
                }
            }
            //Highlight our little block
            fillboxabs(dwgfx,((ed.dmtile%dmwidth())*8)-2,16-2,12,12,dwgfx.getRGB(196, 196, 255 - help.glow));
            fillboxabs(dwgfx,((ed.dmtile%dmwidth())*8)-1,16-1,10,10,dwgfx.getRGB(0,0,0));
        }

        if(ed.dmtileeditor>0 && t2<=30)
        {
            dwgfx.bprint(2, 45-t2, "Tile:", 196, 196, 255 - help.glow, false);
            dwgfx.bprint(58, 45-t2, help.String(ed.dmtile), 196, 196, 255 - help.glow, false);
            FillRect(dwgfx.backBuffer, 44,44-t2,10,10, dwgfx.getRGB(196, 196, 255 - help.glow));
            FillRect(dwgfx.backBuffer, 45,45-t2,8,8, dwgfx.getRGB(0,0,0));

            if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].tileset==0)
            {
                dwgfx.drawtile(45,45-t2,ed.dmtile,0,0,0);
            }
            else if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].tileset==5)
            {
                dwgfx.drawtile3(45,45-t2,ed.dmtile,0,0,0);
            }
            else
            {
                dwgfx.drawtile2(45,45-t2,ed.dmtile,0,0,0);
            }
        }
        else
        {
            dwgfx.bprint(2, 12, "Tile:", 196, 196, 255 - help.glow, false);
            dwgfx.bprint(58, 12, help.String(ed.dmtile), 196, 196, 255 - help.glow, false);
            FillRect(dwgfx.backBuffer, 44,11,10,10, dwgfx.getRGB(196, 196, 255 - help.glow));
            FillRect(dwgfx.backBuffer, 45,12,8,8, dwgfx.getRGB(0,0,0));

            if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].tileset==0)
            {
                dwgfx.drawtile(45,12,ed.dmtile,0,0,0);
            }
            else if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].tileset==5)
            {
                dwgfx.drawtile3(45,12,ed.dmtile,0,0,0);
            }
            else
            {
                dwgfx.drawtile2(45,12,ed.dmtile,0,0,0);
            }
        }
    }




    //Draw GUI
    if(ed.boundarymod>0)
    {
        if(ed.boundarymod==1)
        {
            FillRect(dwgfx.backBuffer, 0,230,320,240, dwgfx.getRGB(32,32,32));
            FillRect(dwgfx.backBuffer, 0,231,320,240, dwgfx.getRGB(0,0,0));
            switch(ed.boundarytype)
            {
            case 0:
                dwgfx.Print(4, 232, "SCRIPT BOX: Click on top left", 255,255,255, false);
                break;
            case 1:
                dwgfx.Print(4, 232, "ENEMY BOUNDS: Click on top left", 255,255,255, false);
                break;
            case 2:
                dwgfx.Print(4, 232, "PLATFORM BOUNDS: Click on top left", 255,255,255, false);
                break;
            case 3:
                dwgfx.Print(4, 232, "COPY TILES: Click on top left", 255,255,255, false);
                break;
            case 4:
                dwgfx.Print(4, 232, "TOWER ENTRY: Click on top row", 255,255,255, false);
                break;
            default:
                dwgfx.Print(4, 232, "Click on top left", 255,255,255, false);
                break;
            }
        }
        else if(ed.boundarymod==2)
        {
            FillRect(dwgfx.backBuffer, 0,230,320,240, dwgfx.getRGB(32,32,32));
            FillRect(dwgfx.backBuffer, 0,231,320,240, dwgfx.getRGB(0,0,0));
            switch(ed.boundarytype)
            {
            case 0:
                dwgfx.Print(4, 232, "SCRIPT BOX: Click on bottom right", 255,255,255, false);
                break;
            case 1:
                dwgfx.Print(4, 232, "ENEMY BOUNDS: Click on bottom right", 255,255,255, false);
                break;
            case 2:
                dwgfx.Print(4, 232, "PLATFORM BOUNDS: Click on bottom right", 255,255,255, false);
                break;
            case 3:
                dwgfx.Print(4, 232, "COPY TILES: Click on bottom right", 255,255,255, false);
                break;
            case 4:
                dwgfx.Print(4, 232, "ACTIVITY ZONE: Click on top left", 255,255,255, false);
                break;
            default:
                dwgfx.Print(4, 232, "Click on bottom right", 255,255,255, false);
                break;
            }
        }
    }
    else if(ed.scripteditmod)
    {
        //Elaborate C64 BASIC menu goes here!
        FillRect(dwgfx.backBuffer, 0,0,320,240, dwgfx.getBGR(123, 111, 218));
        FillRect(dwgfx.backBuffer, 14,16,292,208, dwgfx.getRGB(162,48,61));
        switch(ed.scripthelppage)
        {
        case 0:
            dwgfx.Print(16,28,"**** VVVVVV SCRIPT EDITOR ****", 123, 111, 218, true);
            dwgfx.Print(16,44,"PRESS ESC TO RETURN TO MENU", 123, 111, 218, true);
            //dwgfx.Print(16,60,"READY.", 123, 111, 218, false);

            if(ed.numhooks>0)
            {
                for(int i=0; i<9; i++)
                {
                    if(ed.hookmenupage+i<ed.numhooks)
                    {
                        if(ed.hookmenupage+i==ed.hookmenu)
                        {
                            std::string tstring="> " + ed.hooklist[(ed.numhooks-1)-(ed.hookmenupage+i)] + " <";
                            std::transform(tstring.begin(), tstring.end(),tstring.begin(), ::toupper);
                            dwgfx.Print(16,68+(i*16),tstring,123, 111, 218, true);
                        }
                        else
                        {
                            dwgfx.Print(16,68+(i*16),ed.hooklist[(ed.numhooks-1)-(ed.hookmenupage+i)],123, 111, 218, true);
                        }
                    }
                }
            }
            else
            {
                dwgfx.Print(16,110,"NO SCRIPT IDS FOUND", 123, 111, 218, true);
                dwgfx.Print(16,130,"CREATE A SCRIPT WITH EITHER", 123, 111, 218, true);
                dwgfx.Print(16,140,"THE TERMINAL OR SCRIPT BOX TOOLS", 123, 111, 218, true);
            }
            break;
        case 1:
            //Current scriptname
            FillRect(dwgfx.backBuffer, 14,226,292,12, dwgfx.getRGB(162,48,61));
            dwgfx.Print(16,228,"CURRENT SCRIPT: " + ed.sbscript, 123, 111, 218, true);
            //Draw text
            for(int i=0; i<25; i++)
            {
                if(i+ed.pagey<500)
                {
                    dwgfx.Print(16,20+(i*8),ed.sb[i+ed.pagey], 123, 111, 218, false);
                }
            }
            //Draw cursor
            if(ed.entframe<2)
            {
                dwgfx.Print(16+(ed.sbx*8),20+(ed.sby*8),"_",123, 111, 218, false);
            }
            break;
        }
    }
    else if (ed.trialstartpoint) {
        FillRect(dwgfx.backBuffer, 0,230,320,240, dwgfx.getRGB(32,32,32));
        FillRect(dwgfx.backBuffer, 0,231,320,240, dwgfx.getRGB(0,0,0));
        dwgfx.Print(4, 232, "TIME TRIALS: Place start point", 255,255,255, false);
    }
    else if(ed.settingsmod)
    {
        if(!game.colourblindmode) dwgfx.drawtowerbackgroundsolo(map);

        int tr = map.r - (help.glow / 4) - int(fRandom() * 4);
        int tg = map.g - (help.glow / 4) - int(fRandom() * 4);
        int tb = map.b - (help.glow / 4) - int(fRandom() * 4);
        if (tr < 0) tr = 0;
        if(tr>255) tr=255;
        if (tg < 0) tg = 0;
        if(tg>255) tg=255;
        if (tb < 0) tb = 0;
        if(tb>255) tb=255;
        if (game.currentmenuname == "ed_settings")
        {
            dwgfx.bigprint( -1, 75, "Map Settings", tr, tg, tb, true);
        }
        else if (game.currentmenuname == "ed_edit_trial") {
            customtrial ctrial = game.customtrials[ed.edtrial];

            if(ed.trialnamemod)
            {
                if(ed.entframe<2)
                {
                    dwgfx.bigprint( -1, 35, key.keybuffer+"_", tr, tg, tb, true);
                }
                else
                {
                    dwgfx.bigprint( -1, 35, key.keybuffer+" ", tr, tg, tb, true);
                }
            } else {
                dwgfx.bigprint( -1, 35, ctrial.name, tr, tg, tb, true);
            }
            game.timetrialpar = ctrial.par;
            dwgfx.Print( 16, 65,  "MUSIC      " + help.getmusicname(ctrial.music), tr, tg, tb);
            if (ed.trialmod && (game.currentmenuoption == 3))
                dwgfx.Print( 16, 75,  "TRINKETS   < " + help.number(ctrial.trinkets) + " >", tr, tg, tb);
            else
                dwgfx.Print( 16, 75,  "TRINKETS   " + help.number(ctrial.trinkets), tr, tg, tb);
            if (ed.trialmod && (game.currentmenuoption == 4))
                dwgfx.Print( 16, 85,  "TIME       < " + game.partimestring(help) + " >", tr, tg, tb);
            else
                dwgfx.Print( 16, 85,  "TIME       " + game.partimestring(help), tr, tg, tb);
        }
        else if (game.currentmenuname == "ed_trials")
        {
            dwgfx.bigprint( -1, 35, "Time Trials", tr, tg, tb, true);
            for (int i = 0; i < (int)game.customtrials.size(); i++) {
                std::string sl = game.customtrials[i].name;
                if (game.currentmenuoption == i) {
                    std::transform(sl.begin(), sl.end(), sl.begin(), ::toupper);
                    sl = "[ " + sl + " ]";
                } else {
                    std::transform(sl.begin(), sl.end(), sl.begin(), ::tolower);
                    sl = "  " + sl + "  ";
                }
                dwgfx.Print(-1, 75 + (i * 16), sl, tr,tg,tb,true);
            }
            if (game.currentmenuoption == (int)game.customtrials.size()) {
                dwgfx.Print(-1, 75 + ((int)game.customtrials.size() * 16), "[ ADD NEW TRIAL ]", tr,tg,tb,true);
            } else {
                dwgfx.Print(-1, 75 + ((int)game.customtrials.size() * 16), "  add new trial  ", tr,tg,tb,true);
            }
            if (game.currentmenuoption == (int)game.customtrials.size() + 1) {
                dwgfx.Print(-1, 75 + (((int)game.customtrials.size() + 1) * 16), "[ BACK TO MENU ]", tr,tg,tb,true);
            } else {
                dwgfx.Print(-1, 75 + (((int)game.customtrials.size() + 1) * 16), "  back to menu  ", tr,tg,tb,true);
            }
        }
        else if (game.currentmenuname=="ed_desc")
        {
            if(ed.titlemod)
            {
                if(ed.entframe<2)
                {
                    dwgfx.bigprint( -1, 35, key.keybuffer+"_", tr, tg, tb, true);
                }
                else
                {
                    dwgfx.bigprint( -1, 35, key.keybuffer+" ", tr, tg, tb, true);
                }
            }
            else
            {
                dwgfx.bigprint( -1, 35, EditorData::GetInstance().title, tr, tg, tb, true);
            }
            if(ed.creatormod)
            {
                if(ed.entframe<2)
                {
                    dwgfx.Print( -1, 60, "by " + key.keybuffer+ "_", tr, tg, tb, true);
                }
                else
                {
                    dwgfx.Print( -1, 60, "by " + key.keybuffer+ " ", tr, tg, tb, true);
                }
            }
            else
            {
                dwgfx.Print( -1, 60, "by " + EditorData::GetInstance().creator, tr, tg, tb, true);
            }
            if(ed.websitemod)
            {
                if(ed.entframe<2)
                {
                    dwgfx.Print( -1, 70, key.keybuffer+"_", tr, tg, tb, true);
                }
                else
                {
                    dwgfx.Print( -1, 70, key.keybuffer+" ", tr, tg, tb, true);
                }
            }
            else
            {
                dwgfx.Print( -1, 70, ed.website, tr, tg, tb, true);
            }
            if(ed.desc1mod)
            {
                if(ed.entframe<2)
                {
                    dwgfx.Print( -1, 90, key.keybuffer+"_", tr, tg, tb, true);
                }
                else
                {
                    dwgfx.Print( -1, 90, key.keybuffer+" ", tr, tg, tb, true);
                }
            }
            else
            {
                dwgfx.Print( -1, 90, ed.Desc1, tr, tg, tb, true);
            }
            if(ed.desc2mod)
            {
                if(ed.entframe<2)
                {
                    dwgfx.Print( -1, 100, key.keybuffer+"_", tr, tg, tb, true);
                }
                else
                {
                    dwgfx.Print( -1, 100, key.keybuffer+" ", tr, tg, tb, true);
                }
            }
            else
            {
                dwgfx.Print( -1, 100, ed.Desc2, tr, tg, tb, true);
            }
            if(ed.desc3mod)
            {
                if(ed.entframe<2)
                {
                    dwgfx.Print( -1, 110, key.keybuffer+"_", tr, tg, tb, true);
                }
                else
                {
                    dwgfx.Print( -1, 110, key.keybuffer+" ", tr, tg, tb, true);
                }
            }
            else
            {
                dwgfx.Print( -1, 110, ed.Desc3, tr, tg, tb, true);
            }
        }
        else if (game.currentmenuname == "ed_music")
        {
            dwgfx.bigprint( -1, 65, "Map Music", tr, tg, tb, true);

            dwgfx.Print( -1, 85, "Current map music:", tr, tg, tb, true);
            switch(ed.levmusic)
            {
            case 0:
                dwgfx.Print( -1, 120, "No background music", tr, tg, tb, true);
                break;
            case 1:
                dwgfx.Print( -1, 120, "1: Pushing Onwards", tr, tg, tb, true);
                break;
            case 2:
                dwgfx.Print( -1, 120, "2: Positive Force", tr, tg, tb, true);
                break;
            case 3:
                dwgfx.Print( -1, 120, "3: Potential for Anything", tr, tg, tb, true);
                break;
            case 4:
                dwgfx.Print( -1, 120, "4: Passion for Exploring", tr, tg, tb, true);
                break;
            case 5:
                dwgfx.Print( -1, 120, "N/A: Pause", tr, tg, tb, true);
                break;
            case 6:
                dwgfx.Print( -1, 120, "5: Presenting VVVVVV", tr, tg, tb, true);
                break;
            case 7:
                dwgfx.Print( -1, 120, "N/A: Plenary", tr, tg, tb, true);
                break;
            case 8:
                dwgfx.Print( -1, 120, "6: Predestined Fate", tr, tg, tb, true);
                break;
            case 9:
                dwgfx.Print( -1, 120, "N/A: Positive Force Reversed", tr, tg, tb, true);
                break;
            case 10:
                dwgfx.Print( -1, 120, "7: Popular Potpourri", tr, tg, tb, true);
                break;
            case 11:
                dwgfx.Print( -1, 120, "8: Pipe Dream", tr, tg, tb, true);
                break;
            case 12:
                dwgfx.Print( -1, 120, "9: Pressure Cooker", tr, tg, tb, true);
                break;
            case 13:
                dwgfx.Print( -1, 120, "10: Paced Energy", tr, tg, tb, true);
                break;
            case 14:
                dwgfx.Print( -1, 120, "11: Piercing the Sky", tr, tg, tb, true);
                break;
            case 15:
                dwgfx.Print( -1, 120, "N/A: Predestined Fate Remix", tr, tg, tb, true);
                break;
            default:
                dwgfx.Print( -1, 120, "?: something else", tr, tg, tb, true);
                break;
            }
        }
        else if (game.currentmenuname == "ed_quit")
        {
            dwgfx.bigprint( -1, 90, "Save before", tr, tg, tb, true);
            dwgfx.bigprint( -1, 110, "quiting?", tr, tg, tb, true);
        }

        dwgfx.drawmenu(game, tr, tg, tb, 15);

        /*
        dwgfx.Print(4, 224, "Enter name to save map as:", 255,255,255, false);
        if(ed.entframe<2){
          dwgfx.Print(4, 232, ed.filename+"_", 196, 196, 255 - help.glow, true);
        }else{
          dwgfx.Print(4, 232, ed.filename+" ", 196, 196, 255 - help.glow, true);
        }
        */
    } else if (ed.textmod) {
        FillRect(dwgfx.backBuffer, 0,221,320,240, dwgfx.getRGB(32,32,32));
        FillRect(dwgfx.backBuffer, 0,222,320,240, dwgfx.getRGB(0,0,0));
        dwgfx.Print(4, 224, ed.textdesc, 255,255,255, false);
        std::string input = key.keybuffer;
        if (ed.entframe < 2)
            input += "_";
        else
            input += " ";
        dwgfx.Print(4, 232, input, 196, 196, 255 - help.glow, true);
    }
    else if(ed.warpmod)
    {
        //placing warp token
        FillRect(dwgfx.backBuffer, 0,221,320,240, dwgfx.getRGB(32,32,32));
        FillRect(dwgfx.backBuffer, 0,222,320,240, dwgfx.getRGB(0,0,0));
        dwgfx.Print(4, 224, "Left click to place warp destination", 196, 196, 255 - help.glow, false);
        dwgfx.Print(4, 232, "Right click to cancel", 196, 196, 255 - help.glow, false);
    }
    else
    {
        if(ed.spacemod)
        {
            FillRect(dwgfx.backBuffer, 0,208,320,240, dwgfx.getRGB(32,32,32));
            FillRect(dwgfx.backBuffer, 0,209,320,240, dwgfx.getRGB(0,0,0));

            //Draw little icons for each thingy
            int tx=6, ty=211, tg=32;

            if(ed.spacemenu==0)
            {
                for(int i=0; i<10; i++)
                {
                    FillRect(dwgfx.backBuffer, 4+(i*tg), 209,20,20,dwgfx.getRGB(32,32,32));
                }
                FillRect(dwgfx.backBuffer, 4+(ed.drawmode*tg), 209,20,20,dwgfx.getRGB(64,64,64));
                //0:
                dwgfx.drawtile(tx,ty,83,0,0,0);
                dwgfx.drawtile(tx+8,ty,83,0,0,0);
                dwgfx.drawtile(tx,ty+8,83,0,0,0);
                dwgfx.drawtile(tx+8,ty+8,83,0,0,0);
                //1:
                tx+=tg;
                dwgfx.drawtile(tx,ty,680,0,0,0);
                dwgfx.drawtile(tx+8,ty,680,0,0,0);
                dwgfx.drawtile(tx,ty+8,680,0,0,0);
                dwgfx.drawtile(tx+8,ty+8,680,0,0,0);
                //2:
                tx+=tg;
                dwgfx.drawtile(tx+4,ty+4,8,0,0,0);
                //3:
                tx+=tg;
                dwgfx.drawsprite(tx,ty,22,196,196,196);
                //4:
                tx+=tg;
                dwgfx.drawsprite(tx,ty,21,196,196,196);
                //5:
                tx+=tg;
                dwgfx.drawtile(tx,ty+4,3,0,0,0);
                dwgfx.drawtile(tx+8,ty+4,4,0,0,0);
                //6:
                tx+=tg;
                dwgfx.drawtile(tx,ty+4,24,0,0,0);
                dwgfx.drawtile(tx+8,ty+4,24,0,0,0);
                //7:
                tx+=tg;
                dwgfx.drawtile(tx,ty+4,1,0,0,0);
                dwgfx.drawtile(tx+8,ty+4,1,0,0,0);
                //8:
                tx+=tg;
                dwgfx.drawsprite(tx,ty,78+ed.entframe,196,196,196);
                //9:
                tx+=tg;
                FillRect(dwgfx.backBuffer, tx+2,ty+8,12,1,dwgfx.getRGB(255,255,255));


                std::string toolkeys [10] = {"1","2","3","4","5","6","7","8","9","0"};
                for (int i = 0; i < 10; i++) {
                    int col = 96;
                    int col2 = 164;
                    if ((ed.drawmode) == i) {
                        col = 200;
                        col2 = 255;
                    }
                    fillboxabs(dwgfx, 4+(i*tg), 209,20,20,dwgfx.getRGB(col,col,col));
                    dwgfx.Print((22+(i*tg)+4) - (toolkeys[i].length() * 8), 225-4, toolkeys[i],col2,col2,col2,false);
                }

                dwgfx.Print(4, 232, "1/2", 196, 196, 255 - help.glow, false);
            } else {
                for(int i=0; i<10; i++) 
                    FillRect(dwgfx.backBuffer, 4+(i*tg), 209,20,20,dwgfx.getRGB(32,32,32));
                FillRect(dwgfx.backBuffer, 4+((ed.drawmode-10)*tg), 209,20,20,dwgfx.getRGB(64,64,64));
                //10:
                dwgfx.Print(tx,ty,"A",196, 196, 255 - help.glow, false);
                dwgfx.Print(tx+8,ty,"B",196, 196, 255 - help.glow, false);
                dwgfx.Print(tx,ty+8,"C",196, 196, 255 - help.glow, false);
                dwgfx.Print(tx+8,ty+8,"D",196, 196, 255 - help.glow, false);
                //11:
                tx+=tg;
                dwgfx.drawsprite(tx,ty,17,196,196,196);
                //12:
                tx+=tg;
                fillboxabs(dwgfx, tx+4,ty+4,8,8,dwgfx.getRGB(96,96,96));
                //13:
                tx+=tg;
                dwgfx.drawsprite(tx,ty,18+(ed.entframe%2),196,196,196);
                //14:
                tx+=tg;
                FillRect(dwgfx.backBuffer, tx+6,ty+2,4,12,dwgfx.getRGB(255,255,255));
                //15:
                tx+=tg;
                dwgfx.drawsprite(tx,ty,186,75, 75, 255- help.glow/4 - (fRandom()*20));
                //16:
                tx+=tg;
                dwgfx.drawsprite(tx,ty,184,160- help.glow/2 - (fRandom()*20), 200- help.glow/2, 220 - help.glow);
                //17:
                tx+=tg;
                dwgfx.drawsprite(tx,ty,192,160- help.glow/2 - (fRandom()*20), 200- help.glow/2, 220 - help.glow);
                //18:
                tx+=tg;
                dwgfx.drawhuetile(tx,   ty,   48, 8);
                dwgfx.drawhuetile(tx+8, ty,   48, 8);
                dwgfx.drawhuetile(tx,   ty+8, 48, 8);
                dwgfx.drawhuetile(tx+8, ty+8, 48, 8);
                //19:
                tx+=tg;
                dwgfx.drawtelepart(tx, ty, 1, 100, help);

                std::string toolkeys [10] = {"R","T","Y","U","I","O","P","^1","^2","^3"};
                for (int i = 0; i < 10; i++) {
                    int col = 96;
                    int col2 = 164;
                    if ((ed.drawmode - 10) == i) {
                        col = 200;
                        col2 = 255;
                    }
                    fillboxabs(dwgfx, 4+(i*tg), 209,20,20,dwgfx.getRGB(col,col,col));
                    dwgfx.Print((22+(i*tg)+4) - (toolkeys[i].length() * 8), 225-4, toolkeys[i],col2,col2,col2,false);
                }

                dwgfx.Print(4, 232, "2/2", 196, 196, 255 - help.glow, false);
            }

            dwgfx.Print(128, 232, "< and > keys change tool", 196, 196, 255 - help.glow, false);

            FillRect(dwgfx.backBuffer, 0,198,120,10, dwgfx.getRGB(32,32,32));
            FillRect(dwgfx.backBuffer, 0,199,119,9, dwgfx.getRGB(0,0,0));
            switch(ed.drawmode)
            {
            case 0:
                dwgfx.bprint(2,199, "1: Walls",196, 196, 255 - help.glow);
                break;
            case 1:
                dwgfx.bprint(2,199, "2: Backing",196, 196, 255 - help.glow);
                break;
            case 2:
                dwgfx.bprint(2,199, "3: Spikes",196, 196, 255 - help.glow);
                break;
            case 3:
                dwgfx.bprint(2,199, "4: Trinkets",196, 196, 255 - help.glow);
                break;
            case 4:
                dwgfx.bprint(2,199, "5: Checkpoint",196, 196, 255 - help.glow);
                break;
            case 5:
                dwgfx.bprint(2,199, "6: Disappear",196, 196, 255 - help.glow);
                break;
            case 6:
                dwgfx.bprint(2,199, "7: Conveyors",196, 196, 255 - help.glow);
                break;
            case 7:
                dwgfx.bprint(2,199, "8: Moving",196, 196, 255 - help.glow);
                break;
            case 8:
                dwgfx.bprint(2,199, "9: Enemies",196, 196, 255 - help.glow);
                break;
            case 9:
                dwgfx.bprint(2,199, "0: Grav Line",196, 196, 255 - help.glow);
                break;
            case 10:
                dwgfx.bprint(2,199, "R: Roomtext",196, 196, 255 - help.glow);
                break;
            case 11:
                dwgfx.bprint(2,199, "T: Terminal",196, 196, 255 - help.glow);
                break;
            case 12:
                dwgfx.bprint(2,199, "Y: Script Box",196, 196, 255 - help.glow);
                break;
            case 13:
                dwgfx.bprint(2,199, "U: Warp Token",196, 196, 255 - help.glow);
                break;
            case 14:
                dwgfx.bprint(2,199, "I: Warp Lines",196, 196, 255 - help.glow);
                break;
            case 15:
                dwgfx.bprint(2,199, "O: Crewmate",196, 196, 255 - help.glow);
                break;
            case 16:
                dwgfx.bprint(2,199, "P: Start Point",196, 196, 255 - help.glow);
                break;
            case 17:
                dwgfx.bprint(2,199, "^1: Flip Token",196, 196, 255 - help.glow);
                break;
            case 18:
                dwgfx.bprint(2,199, "^2: Coin",196, 196, 255 - help.glow);
                break;
            case 19:
                dwgfx.bprint(2,199, "^3: Teleporter",196, 196, 255 - help.glow);
                break;
            default:
                dwgfx.bprint(2,199, "?: ???",196, 196, 255 - help.glow);
                break;
            }

            FillRect(dwgfx.backBuffer, 260-24,198,80+24,10, dwgfx.getRGB(32,32,32));
            FillRect(dwgfx.backBuffer, 261-24,199,80+24,9, dwgfx.getRGB(0,0,0));
            dwgfx.bprint(rmstrx, 199, rmstr, 196, 196, 255 - help.glow, false);
        } else {
            //FillRect(dwgfx.backBuffer, 0,230,72,240, dwgfx.RGB(32,32,32));
            //FillRect(dwgfx.backBuffer, 0,231,71,240, dwgfx.RGB(0,0,0));
            if(ed.level[ed.levx+(ed.maxwidth*ed.levy)].roomname!="")
            {
                if(ed.tiley<28)
                {
                    if(ed.roomnamehide>0) ed.roomnamehide--;
                }
                else
                {
                    if(ed.roomnamehide<12) ed.roomnamehide++;
                }
                if (dwgfx.translucentroomname)
                {
                    dwgfx.footerrect.y = 230+ed.roomnamehide;
                    SDL_BlitSurface(dwgfx.footerbuffer, NULL, dwgfx.backBuffer, &dwgfx.footerrect);
                }
                else
                {
                    FillRect(dwgfx.backBuffer, 0,230+ed.roomnamehide,320,10, dwgfx.getRGB(0,0,0));
                }
                dwgfx.bprint(5,231+ed.roomnamehide,ed.level[ed.levx+(ed.maxwidth*ed.levy)].roomname, 196, 196, 255 - help.glow, true);
                dwgfx.bprint(4, 222, "Ctrl+F1: Help", 196, 196, 255 - help.glow, false);
                dwgfx.bprint(rmstrx, 222, rmstr,196, 196, 255 - help.glow, false);
            }
            else
            {
                dwgfx.bprint(4, 232, "Ctrl+F1: Help", 196, 196, 255 - help.glow, false);
                dwgfx.bprint(rmstrx,232, rmstr,196, 196, 255 - help.glow, false);
            }
        }

        if(ed.shiftmenu)
        {
            fillboxabs(dwgfx, 0, 57,161+8,200,dwgfx.getRGB(64,64,64));
            FillRect(dwgfx.backBuffer, 0,58,160+8,200, dwgfx.getRGB(0,0,0));
            dwgfx.Print(4, 60, "Space: Mode Select", 164, 164, 164, false);
            if (tower) {
                dwgfx.Print(4, 70, "F1: Tower Direction",164,164,164,false);
                dwgfx.Print(4, 80, "F2: Tower Entry",164,164,164,false);
            } else {
                dwgfx.Print(4,  70, "F1: Change Tileset",164,164,164,false);
                dwgfx.Print(4,  80, "F2: Change Colour",164,164,164,false);
            }
            dwgfx.Print(4,  90, "F3: Change Enemies",164,164,164,false);
            dwgfx.Print(4, 100, "F4: Enemy Bounds",164,164,164,false);
            dwgfx.Print(4, 110, "F5: Platform Bounds",164,164,164,false);

            if (tower) {
                dwgfx.Print(4, 130, "F6: Next Tower",164,164,164,false);
                dwgfx.Print(4, 140, "F7: Previous Tower",164,164,164,false);
            } else {
                dwgfx.Print(4, 130, "F6: New Alt State",164,164,164,false);
                dwgfx.Print(4, 140, "F7: Remove Alt State",164,164,164,false);
            }

            dwgfx.Print(4, 160, "F8: Tower Mode",164,164,164,false);

            dwgfx.Print(4, 180, "F10: Direct Mode",164,164,164,false);

            if (tower) {
                dwgfx.Print(4, 200, "+: Scroll Down",164,164,164,false);
                dwgfx.Print(4, 210, "-: Scroll Up",164,164,164,false);
            } else {
                dwgfx.Print(4, 200, "A: Change Alt State",164,164,164,false);
                dwgfx.Print(4, 210, "W: Change Warp Dir",164,164,164,false);
            }
            dwgfx.Print(4, 220, "E: Change Roomname",164,164,164,false);

            fillboxabs(dwgfx, 220, 207,100,60,dwgfx.getRGB(64,64,64));
            FillRect(dwgfx.backBuffer, 221,208,160,60, dwgfx.getRGB(0,0,0));
            dwgfx.Print(224, 210, "S: Save Map",164,164,164,false);
            dwgfx.Print(224, 220, "L: Load Map",164,164,164,false);
        }
    }


    if(!ed.settingsmod && !ed.scripteditmod)
    {
        //Same as above, without borders
        switch(ed.drawmode)
        {
        case 0:
            dwgfx.bprint(2,2, "1: Walls",196, 196, 255 - help.glow);
            break;
        case 1:
            dwgfx.bprint(2,2, "2: Backing",196, 196, 255 - help.glow);
            break;
        case 2:
            dwgfx.bprint(2,2, "3: Spikes",196, 196, 255 - help.glow);
            break;
        case 3:
            dwgfx.bprint(2,2, "4: Trinkets",196, 196, 255 - help.glow);
            break;
        case 4:
            dwgfx.bprint(2,2, "5: Checkpoint",196, 196, 255 - help.glow);
            break;
        case 5:
            dwgfx.bprint(2,2, "6: Disappear",196, 196, 255 - help.glow);
            break;
        case 6:
            dwgfx.bprint(2,2, "7: Conveyors",196, 196, 255 - help.glow);
            break;
        case 7:
            dwgfx.bprint(2,2, "8: Moving, Speed: " + std::to_string(ed.entspeed + ed.level[ed.levx+(ed.maxwidth*ed.levy)].platv),196, 196, 255 - help.glow);
            break;
        case 8:
            dwgfx.bprint(2,2, "9: Enemies, Speed: " + std::to_string(ed.entspeed + ed.level[ed.levx+(ed.maxwidth*ed.levy)].enemyv),196, 196, 255 - help.glow);
            break;
        case 9:
            dwgfx.bprint(2,2, "0: Grav Line",196, 196, 255 - help.glow);
            break;
        case 10:
            dwgfx.bprint(2,2, "R: Roomtext",196, 196, 255 - help.glow);
            break;
        case 11:
            dwgfx.bprint(2,2, "T: Terminal",196, 196, 255 - help.glow);
            break;
        case 12:
            if (ed.zmod) {
                dwgfx.bprint(2,2, "Y+Z: Activity Zone",196, 196, 255 - help.glow);
            } else if (ed.xmod) {
                dwgfx.bprint(2,2, "Y+X: One-Time Script Box",196, 196, 255 - help.glow);
            } else {
                dwgfx.bprint(2,2, "Y: Script Box",196, 196, 255 - help.glow);
            }
            break;
        case 13:
            dwgfx.bprint(2,2, "U: Warp Token",196, 196, 255 - help.glow);
            break;
        case 14:
            dwgfx.bprint(2,2, "I: Warp Lines",196, 196, 255 - help.glow);
            break;
        case 15:
            dwgfx.bprint(2,2, "O: Crewmate",196, 196, 255 - help.glow);
            break;
        case 16:
            dwgfx.bprint(2,2, "P: Start Point",196, 196, 255 - help.glow);
            break;
        case 17:
            dwgfx.bprint(2,2, "^1: Flip Token",196, 196, 255 - help.glow);
            break;
        case 18:
            dwgfx.bprint(2,2, "^2: Coin",196, 196, 255 - help.glow);
            break;
        case 19:
            dwgfx.bprint(2,2, "^3: Teleporter",196, 196, 255 - help.glow);
            break;
        default:
            dwgfx.bprint(2,2, "?: ???",196, 196, 255 - help.glow);
            break;
        }

        //dwgfx.Print(254, 2, "F1: HELP", 196, 196, 255 - help.glow, false);
    }

    /*
    for(int i=0; i<script.customscript.size(); i++){
      dwgfx.Print(0,i*8,script.customscript[i],255,255,255);
    }
    dwgfx.Print(0,8*script.customscript.size(),help.String(script.customscript.size()),255,255,255);

    for(int i=0; i<ed.numhooks; i++){
      dwgfx.Print(260,i*8,ed.hooklist[i],255,255,255);
    }
    dwgfx.Print(260,8*ed.numhooks,help.String(ed.numhooks),255,255,255);
    */

    if(ed.notedelay>0)
    {
        FillRect(dwgfx.backBuffer, 0,115,320,18, dwgfx.getRGB(92,92,92));
        FillRect(dwgfx.backBuffer, 0,116,320,16, dwgfx.getRGB(0,0,0));
        dwgfx.Print(0,121, ed.note,196-((45-ed.notedelay)*4), 196-((45-ed.notedelay)*4), 196-((45-ed.notedelay)*4), true);
    }

    if (game.test)
    {
        dwgfx.bprint(5, 5, game.teststring, 196, 196, 255 - help.glow, false);
    }

    dwgfx.drawfade();

    if (game.flashlight > 0 && !game.noflashingmode)
    {
        game.flashlight--;
        dwgfx.flashlight();
    }

    if (game.screenshake > 0  && !game.noflashingmode)
    {
        game.screenshake--;
        dwgfx.screenshake();
    }
    else
    {
        dwgfx.render();
    }
    //dwgfx.backbuffer.unlock();
}

void editorlogic( KeyPoll& key, Graphics& dwgfx, Game& game, entityclass& obj, musicclass& music, mapclass& map, UtilityClass& help )
{
    //Misc
    help.updateglow();

    map.bypos -= 2;
    map.bscroll = -2;

    ed.entframedelay--;
    if(ed.entframedelay<=0)
    {
        ed.entframe=(ed.entframe+1)%4;
        ed.entframedelay=8;
    }

    if(ed.notedelay>0)
    {
        ed.notedelay--;
    }

    if (dwgfx.fademode == 1)
    {
        //Return to game
        map.nexttowercolour();
        map.colstate = 10;
        game.gamestate = 1;
        dwgfx.fademode = 4;
        music.stopmusic();
        music.play(6);
        map.nexttowercolour();
        ed.settingsmod=false;
        ed.trialmod=false;
        dwgfx.backgrounddrawn=false;
        game.createmenu("mainmenu");
    }
}


void editorinput( KeyPoll& key, Graphics& dwgfx, Game& game, mapclass& map, entityclass& obj, UtilityClass& help, musicclass& music )
{
    //TODO Mouse Input!
    game.mx = (float) key.mx;
    game.my = (float) key.my;
    ed.tilex=(game.mx - (game.mx%8))/8;
    ed.tiley=(game.my - (game.my%8))/8;
    if (game.stretchMode == 1) {
        // In this mode specifically, we have to fix the mouse coordinates
        int winwidth, winheight;
        dwgfx.screenbuffer->GetWindowSize(&winwidth, &winheight);
        ed.tilex = ed.tilex * 320 / winwidth;
        ed.tiley = ed.tiley * 240 / winheight;
    }

    game.press_left = false;
    game.press_right = false;
    game.press_action = false;
    game.press_map = false;

    if (key.isDown(KEYBOARD_LEFT) || key.isDown(KEYBOARD_a))
    {
        game.press_left = true;
    }
    if (key.isDown(KEYBOARD_RIGHT) || key.isDown(KEYBOARD_d))
    {
        game.press_right = true;
    }
    if (key.isDown(KEYBOARD_z) || key.isDown(KEYBOARD_SPACE) || key.isDown(KEYBOARD_v))
    {
        // || key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN)
        game.press_action = true;
    };

    if (key.keymap[SDLK_F11] && (ed.keydelay==0)) {
        ed.keydelay = 30;
        ed.note="Reloaded resources";
        ed.notedelay=45;
        dwgfx.reloadresources();
    }

    int tower = ed.get_tower(ed.levx, ed.levy);

    if (key.isDown(KEYBOARD_ENTER)) game.press_map = true;
    if (key.isDown(27) && !ed.settingskey)
    {
        ed.settingskey=true;
        if (ed.textmod) {
            key.disabletextentry();
            if (ed.textmod >= FIRST_ENTTEXT && ed.textmod <= LAST_ENTTEXT) {
                *ed.textptr = ed.oldenttext;
                if (ed.oldenttext == "")
                    removeedentity(ed.textent);
            }

            ed.textmod = TEXT_NONE;

            ed.shiftmenu = false;
            ed.shiftkey = false;
        } else if (ed.textentry) {
            key.disabletextentry();
            ed.textentry=false;
            ed.titlemod=false;
            ed.trialnamemod=false;
            ed.desc1mod=false;
            ed.desc2mod=false;
            ed.desc3mod=false;
            ed.websitemod=false;
            ed.creatormod=false;

            ed.shiftmenu=false;
            ed.shiftkey=false;
        }
        else if(ed.boundarymod>0)
        {
            ed.boundarymod=0;
        }
        else if (ed.trialstartpoint) {
            ed.trialstartpoint = false;
            ed.settingsmod = true;
        }
        else
        {
            ed.settingsmod=!ed.settingsmod;
            ed.trialmod = false;
            dwgfx.backgrounddrawn=false;

            game.createmenu("ed_settings");
            map.nexttowercolour();
        }
    }

    if (!key.isDown(27))
    {
        ed.settingskey=false;
    }

    if(key.keymap[SDLK_LCTRL] || key.keymap[SDLK_RCTRL])
    {
        if(key.leftbutton) key.rightbutton=true;
    }

    if(ed.scripteditmod)
    {
        if(ed.scripthelppage==0)
        {
            //hook select menu
            if(ed.keydelay>0) ed.keydelay--;

            if(key.keymap[SDLK_UP] && ed.keydelay<=0)
            {
                ed.keydelay=6;
                ed.hookmenu--;
            }

            if(key.keymap[SDLK_DOWN] && ed.keydelay<=0)
            {
                ed.keydelay=6;
                ed.hookmenu++;
            }

            if(ed.hookmenu>=ed.numhooks)
            {
                ed.hookmenu=ed.numhooks-1;
            }
            if(ed.hookmenu<0) ed.hookmenu=0;

            if(ed.hookmenu<ed.hookmenupage)
            {
                ed.hookmenupage=ed.hookmenu;
            }

            if(ed.hookmenu>=ed.hookmenupage+9)
            {
                ed.hookmenupage=ed.hookmenu+8;
            }

            if(!key.keymap[SDLK_BACKSPACE]) ed.deletekeyheld=0;

            if(key.keymap[SDLK_BACKSPACE] && ed.deletekeyheld==0)
            {
                ed.deletekeyheld=1;
                music.playef(2);
                ed.removehook(ed.hooklist[(ed.numhooks-1)-ed.hookmenu]);
            }

            if (!game.press_action && !game.press_left && !game.press_right
                    && !key.keymap[SDLK_UP] && !key.keymap[SDLK_DOWN] && !key.isDown(27)) game.jumpheld = false;
            if (!game.jumpheld)
            {
                if (game.press_action || game.press_left || game.press_right || game.press_map
                        || key.keymap[SDLK_UP] || key.keymap[SDLK_DOWN] || key.isDown(27))
                {
                    game.jumpheld = true;
                }
                if ((game.press_action || game.press_map) && ed.numhooks>0)
                {
                    game.mapheld=true;
                    ed.scripthelppage=1;
                    key.keybuffer="";
                    ed.sbscript=ed.hooklist[(ed.numhooks-1)-ed.hookmenu];
                    ed.loadhookineditor(ed.sbscript);

                    ed.sby=ed.sblength-1;
                    ed.pagey=0;
                    while(ed.sby>=20)
                    {
                        ed.pagey++;
                        ed.sby--;
                    }
                    key.keybuffer=ed.sb[ed.pagey+ed.sby];
                    ed.sbx = utf8::distance(ed.sb[ed.pagey+ed.sby].begin(), ed.sb[ed.pagey+ed.sby].end());
                }

                if (key.isDown(27))
                {
                    ed.scripteditmod=false;
                    ed.settingsmod=false;
                    ed.trialmod=false;
                }
            }
        }
        else if(ed.scripthelppage==1)
        {
            //Script editor!
            if (key.isDown(27))
            {
                ed.scripthelppage=0;
                game.jumpheld = true;
                //save the script for use again!
                ed.addhook(ed.sbscript);
            }

            if(ed.keydelay>0) ed.keydelay--;

            if(key.keymap[SDLK_UP] && ed.keydelay<=0)
            {
                ed.keydelay=6;
                ed.sby--;
                if(ed.sby<=5)
                {
                    if(ed.pagey>0)
                    {
                        ed.pagey--;
                        ed.sby++;
                    }
                    else
                    {
                        if(ed.sby<0) ed.sby=0;
                    }
                }
                key.keybuffer=ed.sb[ed.pagey+ed.sby];
            }

            if(key.keymap[SDLK_DOWN] && ed.keydelay<=0)
            {
                ed.keydelay=6;
                if(ed.sby+ed.pagey<ed.sblength-1)
                {
                    ed.sby++;
                    if(ed.sby>=20)
                    {
                        ed.pagey++;
                        ed.sby--;
                    }
                }
                key.keybuffer=ed.sb[ed.pagey+ed.sby];
            }

            if(key.linealreadyemptykludge)
            {
                ed.keydelay=6;
                key.linealreadyemptykludge=false;
            }

            if(key.pressedbackspace && ed.sb[ed.pagey+ed.sby]=="" && ed.keydelay<=0)
            {
                //Remove this line completely
                ed.removeline(ed.pagey+ed.sby);
                ed.sby--;
                if(ed.sby<=5)
                {
                    if(ed.pagey>0)
                    {
                        ed.pagey--;
                        ed.sby++;
                    }
                    else
                    {
                        if(ed.sby<0) ed.sby=0;
                    }
                }
                key.keybuffer=ed.sb[ed.pagey+ed.sby];
                ed.keydelay=6;
            }

            ed.sb[ed.pagey+ed.sby]=key.keybuffer;
            ed.sbx = utf8::distance(ed.sb[ed.pagey+ed.sby].begin(), ed.sb[ed.pagey+ed.sby].end());

            if(!game.press_map && !key.isDown(27)) game.mapheld=false;
            if (!game.mapheld)
            {
                if(game.press_map)
                {
                    game.mapheld=true;
                    //Continue to next line
                    if(ed.sby+ed.pagey>=ed.sblength) //we're on the last line
                    {
                        ed.sby++;
                        if(ed.sby>=20)
                        {
                            ed.pagey++;
                            ed.sby--;
                        }
                        if(ed.sby+ed.pagey>=ed.sblength) ed.sblength=ed.sby+ed.pagey;
                        key.keybuffer=ed.sb[ed.pagey+ed.sby];
                        ed.sbx = utf8::distance(ed.sb[ed.pagey+ed.sby].begin(), ed.sb[ed.pagey+ed.sby].end());
                    }
                    else
                    {
                        //We're not, insert a line instead
                        ed.sby++;
                        if(ed.sby>=20)
                        {
                            ed.pagey++;
                            ed.sby--;
                        }
                        ed.insertline(ed.sby+ed.pagey);
                        key.keybuffer="";
                        ed.sbx = 0;
                    }
                }
            }
        }
    } else if (ed.textmod) {
        *ed.textptr = key.keybuffer;

        if (!game.press_map && !key.isDown(27))
            game.mapheld = false;
        if (!game.mapheld && game.press_map) {
            game.mapheld = true;
            if (!ed.textcount)
                key.disabletextentry();

            growing_vector<std::string> coords;
            std::string filename = ed.filename+".vvvvvv";
            switch (ed.textmod) {
            case TEXT_GOTOROOM:
                coords = split(key.keybuffer, ',');
                if (coords.size() == 2) {
                    ed.levx = (atoi(coords[0].c_str()) - 1) % ed.mapwidth;
                    if (ed.levx < 0)
                        ed.levx = 0;
                    ed.levy = (atoi(coords[1].c_str()) - 1) % ed.mapheight;
                    if (ed.levy < 0)
                        ed.levy = 0;
                }
                break;
            case TEXT_LOAD:
                ed.load(filename, dwgfx, map, game);
                // don't use filename, it has the full path
                ed.note = "[ Loaded map: "+ed.filename+".vvvvvv ]";
                ed.notedelay = 45;
                break;
            case TEXT_SAVE:
                ed.save(filename, map, game);
                ed.note="[ Saved map: " + ed.filename+".vvvvvv ]";
                ed.notedelay=45;

                if(ed.saveandquit)
                    dwgfx.fademode = 2; // quit editor
                break;
            case TEXT_ACTIVITYZONE:
                if (ed.textcount == 2) {
                    ed.textptr = &(edentity[ed.textent].activitycolor);
                    ed.textdesc = "Enter activity zone color:";
                } else if (ed.textcount == 1) {
                    ed.textptr = &(edentity[ed.textent].scriptname);
                    ed.textdesc = "Enter script name:";
                }

                if (ed.textcount) {
                    key.keybuffer = *ed.textptr;
                    ed.oldenttext = key.keybuffer;
                    break;
                }

                // fallthrough
            case TEXT_SCRIPT:
                ed.clearscriptbuffer();
                if (!ed.checkhook(key.keybuffer))
                    ed.addhook(key.keybuffer);
                break;
            default:
                break;
            }

            if (!ed.textcount) {
                ed.shiftmenu = false;
                ed.shiftkey = false;
                ed.textmod = TEXT_NONE;
            } else
                ed.textcount--;
        }
    } else if (ed.textentry) {
        if(ed.titlemod)
        {
            EditorData::GetInstance().title=key.keybuffer;
        }
        else if (ed.trialnamemod) {
            game.customtrials[ed.edtrial].name=key.keybuffer;
        }
        else if(ed.creatormod)
        {
            EditorData::GetInstance().creator=key.keybuffer;
        }
        else if(ed.websitemod)
        {
            ed.website=key.keybuffer;
        }
        else if(ed.desc1mod)
        {
            ed.Desc1=key.keybuffer;
        }
        else if(ed.desc2mod)
        {
            ed.Desc2=key.keybuffer;
        }
        else if(ed.desc3mod)
        {
            ed.Desc3=key.keybuffer;
        }

        if(!game.press_map && !key.isDown(27)) game.mapheld=false;
        if (!game.mapheld)
        {
            if(game.press_map)
            {
                game.mapheld=true;
                if(ed.titlemod)
                {
                    EditorData::GetInstance().title=key.keybuffer;
                    ed.titlemod=false;
                }
                else if (ed.trialnamemod) {
                    game.customtrials[ed.edtrial].name = key.keybuffer;
                    ed.trialnamemod=false;
                }
                else if(ed.creatormod)
                {
                    EditorData::GetInstance().creator=key.keybuffer;
                    ed.creatormod=false;
                }
                else if(ed.websitemod)
                {
                    ed.website=key.keybuffer;
                    ed.websitemod=false;
                }
                else if(ed.desc1mod)
                {
                    ed.Desc1=key.keybuffer;
                }
                else if(ed.desc2mod)
                {
                    ed.Desc2=key.keybuffer;
                }
                else if(ed.desc3mod)
                {
                    ed.Desc3=key.keybuffer;
                    ed.desc3mod=false;
                }

                if(ed.desc1mod)
                {
                    ed.desc1mod=false;

                    ed.textentry=true;
                    ed.desc2mod=true;
                    key.enabletextentry();
                    key.keybuffer=ed.Desc2;
                }
                else if(ed.desc2mod)
                {
                    ed.desc2mod=false;

                    ed.textentry=true;
                    ed.desc3mod=true;
                    key.enabletextentry();
                    key.keybuffer=ed.Desc3;
                }
            }
        }
    }
    else
    {
        if(ed.settingsmod)
        {
            if (!game.press_action && !game.press_left && !game.press_right
                    && !key.keymap[SDLK_UP] && !key.keymap[SDLK_DOWN] && !key.keymap[SDLK_PAGEUP] && !key.keymap[SDLK_PAGEDOWN]) game.jumpheld = false;
            if (!game.jumpheld)
            {
                if (game.press_action || game.press_left || game.press_right || game.press_map
                        || key.keymap[SDLK_UP] || key.keymap[SDLK_DOWN] || key.keymap[SDLK_PAGEUP] || key.keymap[SDLK_PAGEDOWN])
                {
                    game.jumpheld = true;
                }

                if(game.menustart)
                {
                    if (game.press_left || key.keymap[SDLK_UP])
                    {
                        if (!ed.trialmod) game.currentmenuoption--;
                    }
                    else if (game.press_right || key.keymap[SDLK_DOWN])
                    {
                        if (!ed.trialmod) game.currentmenuoption++;
                    }
                }

                if (game.currentmenuname != "ed_trials") {
                    if (game.currentmenuoption < 0) game.currentmenuoption = game.nummenuoptions-1;
                    if (game.currentmenuoption >= game.nummenuoptions ) game.currentmenuoption = 0;
                } else {
                    if (game.currentmenuoption < 0) game.currentmenuoption = (int)game.customtrials.size()+1;
                    if (game.currentmenuoption > (int)game.customtrials.size()+1) game.currentmenuoption = 0;
                }

                if (ed.trialmod) {
                    if (game.press_action) {
                        ed.trialmod = false;
                        game.jumpheld = true;
                        music.playef(11, 10);
                    }
                    if (game.currentmenuoption == 3) {
                        if (game.press_left || key.keymap[SDLK_UP]) game.customtrials[ed.edtrial].trinkets--;
                        if (game.press_right || key.keymap[SDLK_DOWN]) game.customtrials[ed.edtrial].trinkets++;
                        if (game.customtrials[ed.edtrial].trinkets > 99) game.customtrials[ed.edtrial].trinkets = 0;
                        if (game.customtrials[ed.edtrial].trinkets < 0) game.customtrials[ed.edtrial].trinkets = 99;
                    }
                    if (game.currentmenuoption == 4) {
                        if (game.press_left || key.keymap[SDLK_UP]) game.customtrials[ed.edtrial].par--;
                        if (game.press_right || key.keymap[SDLK_DOWN]) game.customtrials[ed.edtrial].par++;
                        if (key.keymap[SDLK_PAGEDOWN]) game.customtrials[ed.edtrial].par += 60;
                        if (key.keymap[SDLK_PAGEUP]) game.customtrials[ed.edtrial].par -= 60;
                        if (game.customtrials[ed.edtrial].par > 600) game.customtrials[ed.edtrial].par = 0;
                        if (game.customtrials[ed.edtrial].par < 0) game.customtrials[ed.edtrial].par = 600;
                    }
                }
                else if (game.press_action)
                {
                    if (game.currentmenuname == "ed_trials")
                    {
                        if (game.currentmenuoption == (int)game.customtrials.size())
                        {
                            customtrial temp;
                            temp.name = "Trial " + std::to_string(game.customtrials.size() + 1);
                            game.customtrials.push_back(temp);
                            ed.edtrial = (int)game.customtrials.size() - 1;
                            music.playef(11, 10);
                            game.createmenu("ed_edit_trial");
                        }
                        else if (game.currentmenuoption == (int)game.customtrials.size()+1)
                        {
                            music.playef(11, 10);
                            game.createmenu("ed_settings");
                            map.nexttowercolour();
                        }
                        else
                        {
                            ed.edtrial = game.currentmenuoption;
                            music.playef(11, 10);
                            game.createmenu("ed_edit_trial");
                        }
                    }
                    else if (game.currentmenuname == "ed_edit_trial")
                    {
                        if (game.currentmenuoption == 0)
                        {
                            ed.textentry=true;
                            ed.trialnamemod=true;
                            key.enabletextentry();
                            key.keybuffer=game.customtrials[ed.edtrial].name;
                        }
                        if (game.currentmenuoption == 1) {
                            ed.trialstartpoint = true;
                            ed.settingsmod = false;
                            music.playef(11, 10);
                        }
                        if (game.currentmenuoption == 2) {
                            music.playef(11, 10);
                            game.customtrials[ed.edtrial].music++;
                            if (game.customtrials[ed.edtrial].music > 15) game.customtrials[ed.edtrial].music = 0;
                        }
                        if (game.currentmenuoption == 3) {
                            music.playef(11, 10);
                            ed.trialmod = true;
                        }
                        if (game.currentmenuoption == 4) {
                            music.playef(11, 10);
                            ed.trialmod = true;
                        }
                        if (game.currentmenuoption == 5) {
                            music.playef(11, 10);
                            game.createmenu("ed_trials");
                            map.nexttowercolour();
                        }
                    }
                    else if (game.currentmenuname == "ed_desc")
                    {
                        if (game.currentmenuoption == 0)
                        {
                            ed.textentry=true;
                            ed.titlemod=true;
                            key.enabletextentry();
                            key.keybuffer=EditorData::GetInstance().title;
                        }
                        else if (game.currentmenuoption == 1)
                        {
                            ed.textentry=true;
                            ed.creatormod=true;
                            key.enabletextentry();
                            key.keybuffer=EditorData::GetInstance().creator;
                        }
                        else if (game.currentmenuoption == 2)
                        {
                            ed.textentry=true;
                            ed.desc1mod=true;
                            key.enabletextentry();
                            key.keybuffer=ed.Desc1;
                        }
                        else if (game.currentmenuoption == 3)
                        {
                            ed.textentry=true;
                            ed.websitemod=true;
                            key.enabletextentry();
                            key.keybuffer=ed.website;
                        }
                        else if (game.currentmenuoption == 4)
                        {
                            music.playef(11, 10);
                            game.createmenu("ed_settings");
                            map.nexttowercolour();
                        }
                    }
                    else if (game.currentmenuname == "ed_settings")
                    {
                        if (game.currentmenuoption == 0)
                        {
                            //Change level description stuff
                            music.playef(11, 10);
                            game.createmenu("ed_desc");
                            map.nexttowercolour();
                        }
                        else if (game.currentmenuoption == 1)
                        {
                            //Enter script editormode
                            music.playef(11, 10);
                            ed.scripteditmod=true;
                            ed.clearscriptbuffer();
                            key.enabletextentry();
                            key.keybuffer="";
                            ed.hookmenupage=0;
                            ed.hookmenu=0;
                            ed.scripthelppage=0;
                            ed.scripthelppagedelay=0;
                            ed.sby=0;
                            ed.sbx=0, ed.pagey=0;
                        }
                        else if (game.currentmenuoption == 2)
                        {
                            music.playef(11, 10);
                            game.createmenu("ed_trials");
                            map.nexttowercolour();
                        }
                        else if (game.currentmenuoption == 3)
                        {
                            music.playef(11, 10);
                            game.createmenu("ed_music");
                            map.nexttowercolour();
                            if(ed.levmusic>0) music.play(ed.levmusic);
                        }
                        else if (game.currentmenuoption == 4)
                        {
                            //Load level
                            ed.settingsmod=false;
                            map.nexttowercolour();

                            ed.keydelay = 6;
                            ed.getlin(key, TEXT_LOAD, "Enter map filename "
                                      "to load:", &(ed.filename));
                            game.mapheld=true;
                            dwgfx.backgrounddrawn=false;
                        }
                        else if (game.currentmenuoption == 5)
                        {
                            //Save level
                            ed.settingsmod=false;
                            map.nexttowercolour();

                            ed.keydelay = 6;
                            ed.getlin(key, TEXT_SAVE, "Enter map filename "
                                      "to save map as:", &(ed.filename));
                            game.mapheld=true;
                            dwgfx.backgrounddrawn=false;
                        }
                        else if (game.currentmenuoption == 6)
                        {
                            music.playef(11, 10);
                            game.createmenu("ed_quit");
                            map.nexttowercolour();
                        }
                    }
                    else if (game.currentmenuname == "ed_music")
                    {
                        if (game.currentmenuoption == 0)
                        {
                            ed.levmusic++;
                            //if(ed.levmusic==5) ed.levmusic=6;
                            //if(ed.levmusic==7) ed.levmusic=8;
                            //if(ed.levmusic==9) ed.levmusic=10;
                            //if(ed.levmusic==15) ed.levmusic=0;
                            if(ed.levmusic==16) ed.levmusic=0;
                            if(ed.levmusic>0)
                            {
                                music.play(ed.levmusic);
                            }
                            else
                            {
                                music.haltdasmusik();
                            }
                            music.playef(11, 10);
                        }
                        else if (game.currentmenuoption == 1)
                        {
                            music.playef(11, 10);
                            music.fadeout();
                            game.createmenu("ed_settings");
                            map.nexttowercolour();
                        }
                    }
                    else if (game.currentmenuname == "ed_quit")
                    {
                        if (game.currentmenuoption == 0)
                        {
                            //Saving and quit
                            ed.saveandquit=true;
                            ed.settingsmod=false;
                            map.nexttowercolour();

                            ed.keydelay = 6;
                            ed.getlin(key, TEXT_SAVE, "Enter map filename "
                                      "to save map as:", &(ed.filename));
                            game.mapheld=true;
                            dwgfx.backgrounddrawn=false;
                        }
                        else if (game.currentmenuoption == 1)
                        {
                            //Quit without saving
                            music.playef(11, 10);
                            music.fadeout();
                            dwgfx.fademode = 2;
                        }
                        else if (game.currentmenuoption == 2)
                        {
                            //Go back to editor
                            music.playef(11, 10);
                            game.createmenu("ed_settings");
                            map.nexttowercolour();
                        }
                    }
                }
            }
        } else if (ed.keydelay) {
            ed.keydelay--;
        } else if (ed.trialstartpoint) {
            // Allow the player to switch rooms
            if (key.keymap[SDLK_UP] || key.keymap[SDLK_DOWN] ||
                key.keymap[SDLK_LEFT] || key.keymap[SDLK_RIGHT]) {
                ed.keydelay = 6;
                if (key.keymap[SDLK_UP])
                    ed.levy--;
                else if (key.keymap[SDLK_DOWN])
                    ed.levy++;
                else if (key.keymap[SDLK_LEFT])
                    ed.levx--;
                else if (key.keymap[SDLK_RIGHT])
                    ed.levx++;
                ed.updatetiles = true;
                ed.changeroom = true;
                dwgfx.backgrounddrawn=false;
                ed.levaltstate = 0;
                ed.levx = (ed.levx + ed.mapwidth) % ed.mapwidth;
                ed.levy = (ed.levy + ed.mapheight) % ed.mapheight;
            }
            if(key.leftbutton) {
                ed.trialstartpoint = false;
                game.customtrials[ed.edtrial].startx = (ed.tilex*8) - 4;
                game.customtrials[ed.edtrial].starty = (ed.tiley*8);
                game.customtrials[ed.edtrial].startf = 0;
                game.customtrials[ed.edtrial].roomx = ed.levx;
                game.customtrials[ed.edtrial].roomy = ed.levy;
                ed.settingsmod = true;
            }
        } else if ((key.keymap[SDLK_LSHIFT] || key.keymap[SDLK_RSHIFT]) &&
                   (key.keymap[SDLK_LCTRL] || key.keymap[SDLK_RCTRL])) {
            // Ctrl+Shift modifiers
            // TODO: Better Direct Mode interface
            ed.dmtileeditor=10;
            if(key.keymap[SDLK_LEFT]) {
                ed.dmtile--;
                ed.keydelay=3;
                if(ed.dmtile<0) ed.dmtile+=dmcap();
            } else if(key.keymap[SDLK_RIGHT]) {
                ed.dmtile++;
                ed.keydelay=3;

                if (ed.dmtile>=dmcap())
                    ed.dmtile-=dmcap();
            }
            if(key.keymap[SDLK_UP]) {
                ed.dmtile-=dmwidth();
                ed.keydelay=3;
                if(ed.dmtile<0) ed.dmtile+=dmcap();
            } else if(key.keymap[SDLK_DOWN]) {
                ed.dmtile+=dmwidth();
                ed.keydelay=3;

                if(ed.dmtile>=dmcap()) ed.dmtile-=dmcap();
            }

            // CONTRIBUTORS: keep this a secret :)
            if (key.keymap[SDLK_6]) {
                ed.drawmode=-6;
                ed.keydelay = 6;
            }
        } else if (key.keymap[SDLK_LCTRL] || key.keymap[SDLK_RCTRL]) {
            // Ctrl modifiers
            if (key.keymap[SDLK_F1]) {
                // Help screen
                ed.shiftmenu = !ed.shiftmenu;
                ed.keydelay = 6;
            }

            if (key.keymap[SDLK_p] || key.keymap[SDLK_o] ||
                key.keymap[SDLK_t]) {
                // Jump to player location, next crewmate or trinket
                int ent = 16; // player
                if (key.keymap[SDLK_o])
                    ent = 15;
                else if (key.keymap[SDLK_t])
                    ent = 9;

                if (ed.lastentcycle != ent) {
                    ed.entcycle = 0;
                    ed.lastentcycle = ent;
                }

                // Find next entity of this kind
                ed.entcycle++;
                int num_ents = 0;
                int i;
                for (i = 0; i < EditorData::GetInstance().numedentities;
                     i++)
                    if (edentity[i].t == ent)
                        num_ents++;

                if (ed.entcycle > num_ents)
                    ed.entcycle = 1;

                num_ents = 0;
                for (i = 0; i < EditorData::GetInstance().numedentities;
                     i++) {
                    if (edentity[i].t == ent) {
                        num_ents++;
                        if (ed.entcycle == num_ents)
                            break;
                    }
                }

                int roomx = ed.levx;
                int roomy = ed.levy;
                if (num_ents && !edentity[i].intower) {
                    roomx = edentity[i].x / 40;
                    roomy = edentity[i].y / 30;
                }

                if (roomx != ed.levx || roomy != ed.levy) {
                    ed.levx = mod(roomx, ed.mapwidth);
                    ed.levy = mod(roomy, ed.mapheight);
                    ed.updatetiles = true;
                    ed.changeroom = true;
                    dwgfx.backgrounddrawn=false;
                    ed.levaltstate = 0;
                    ed.keydelay = 12;
                }
            }
            int speedcap = 16;

            if (key.keymap[SDLK_COMMA] && (ed.keydelay==0)) {
                ed.keydelay = 6;
                ed.entspeed--;
                if (ed.entspeed < -speedcap) ed.entspeed = speedcap;
            }

            if (key.keymap[SDLK_PERIOD] && (ed.keydelay==0)) {
                ed.keydelay = 6;
                ed.entspeed++;
                if (ed.entspeed > speedcap) ed.entspeed = -speedcap;
            }

        } else if (key.keymap[SDLK_LSHIFT] || key.keymap[SDLK_RSHIFT]) {
            // Shift modifiers
            if (key.keymap[SDLK_UP] || key.keymap[SDLK_DOWN] ||
                key.keymap[SDLK_LEFT] || key.keymap[SDLK_RIGHT]) {
                ed.keydelay = 6;
                if (key.keymap[SDLK_UP])
                    ed.mapheight--;
                else if (key.keymap[SDLK_DOWN])
                    ed.mapheight++;
                else if (key.keymap[SDLK_LEFT])
                    ed.mapwidth--;
                else if (key.keymap[SDLK_RIGHT])
                    ed.mapwidth++;

                if (ed.mapheight < 1)
                    ed.mapheight = 1;
                if (ed.mapheight > ed.maxheight)
                    ed.mapheight = ed.maxheight;
                if (ed.mapwidth < 1)
                    ed.mapwidth = 1;
                if (ed.mapwidth > ed.maxwidth)
                    ed.mapwidth = ed.maxwidth;

                ed.note = "Mapsize is now [" + help.String(ed.mapwidth) + "," +
                    help.String(ed.mapheight) + "]";
                ed.notedelay=45;
            }

            if (key.keymap[SDLK_F1] && ed.keydelay==0) {
                ed.switch_tileset(true);
                dwgfx.backgrounddrawn=false;
                ed.keydelay = 6;
            }
            if (key.keymap[SDLK_F2] && ed.keydelay==0) {
                ed.switch_tilecol(true);
                dwgfx.backgrounddrawn=false;
                ed.keydelay = 6;
            }

            if (key.keymap[SDLK_1]) ed.drawmode=17;
            if (key.keymap[SDLK_2]) ed.drawmode=18;
            if (key.keymap[SDLK_3]) ed.drawmode=19;
        } else {
            // No modifiers
            if (key.keymap[SDLK_COMMA] || key.keymap[SDLK_PERIOD]) {
                ed.keydelay = 6;
                if (key.keymap[SDLK_PERIOD])
                    if (ed.drawmode != -6)
                        ed.drawmode++;
                    else
                        ed.drawmode = 0;
                else {
                    ed.drawmode--;
                    if (ed.drawmode < 0 && ed.drawmode != -6)
                        ed.drawmode = 19;
                }
                if (ed.drawmode != -6) ed.drawmode %= 20;

                ed.spacemenu = 0;
                if (ed.drawmode > 9)
                    ed.spacemenu = 1;
            }
            if (tower && (key.keymap[SDLK_w] || key.keymap[SDLK_a])) {
                ed.notedelay=45;
                ed.note="Unavailable in Tower Mode";
                ed.updatetiles=true;
                ed.keydelay=6;
            }
            if (tower && ed.keydelay == 0 && key.keymap[SDLK_F1]) {
                // Change Scroll Direction
                ed.towers[tower-1].scroll = !ed.towers[tower-1].scroll;
                ed.notedelay=45;
                if (ed.towers[tower-1].scroll)
                    ed.note="Tower now Descending";
                else
                    ed.note="Tower now Ascending";
                ed.updatetiles=true;
                ed.keydelay=6;
            }
            if (tower && ed.keydelay == 0 && key.keymap[SDLK_F2]) {
                // Change Tower Entry
                ed.keydelay=6;
                ed.boundarytype=4;
                ed.boundarymod=1;
            }
            if (tower && ed.keydelay == 0 &&
                (key.keymap[SDLK_F6] || key.keymap[SDLK_F7])) {
                // Change Used Tower
                if (key.keymap[SDLK_F7]) {
                    if (ed.level[ed.levx + ed.levy*ed.maxwidth].tower > 1)
                        ed.level[ed.levx + ed.levy*ed.maxwidth].tower--;
                } else if (ed.level[ed.levx + ed.levy*ed.maxwidth].tower < ed.maxwidth * ed.maxheight)
                    ed.level[ed.levx + ed.levy*ed.maxwidth].tower++;

                ed.note = "Tower Changed";
                ed.keydelay = 6;
                ed.notedelay = 45;
                ed.updatetiles = true;
                ed.snap_tower_entry(ed.levx, ed.levy);
            }
            if (tower && ed.keydelay == 0 &&
                (key.keymap[SDLK_PLUS] || key.keymap[SDLK_KP_PLUS] ||
                 key.keymap[SDLK_EQUALS] || key.keymap[SDLK_KP_EQUALS] ||
                 key.keymap[SDLK_MINUS] || key.keymap[SDLK_KP_MINUS] ||
                 key.keymap[SDLK_HOME] || key.keymap[SDLK_END] ||
                 key.keymap[SDLK_PAGEUP] || key.keymap[SDLK_PAGEDOWN])) {
                int modpos = 1;
                if (key.keymap[SDLK_LSHIFT] || key.keymap[SDLK_RSHIFT])
                    modpos = 5;
                if (key.keymap[SDLK_PAGEUP] || key.keymap[SDLK_PAGEDOWN])
                    modpos = 30;
                if (key.keymap[SDLK_HOME] || key.keymap[SDLK_END])
                    modpos = ed.tower_size(tower);
                if (key.keymap[SDLK_MINUS] || key.keymap[SDLK_KP_MINUS] ||
                    key.keymap[SDLK_HOME] || key.keymap[SDLK_PAGEUP])
                    modpos *= -1;
                ed.ypos += modpos;
                ed.snap_tower_entry(ed.levx, ed.levy);
            }
            if(key.keymap[SDLK_F1] && ed.keydelay==0)
            {
                ed.switch_tileset(false);
                dwgfx.backgrounddrawn=false;
                ed.keydelay = 6;
            }
            if(key.keymap[SDLK_F2] && ed.keydelay==0)
            {
                ed.switch_tilecol(false);
                dwgfx.backgrounddrawn=false;
                ed.keydelay = 6;
            }
            if(key.keymap[SDLK_F3] && ed.keydelay==0)
            {
                ed.level[ed.levx+(ed.levy*ed.maxwidth)].enemytype=(ed.level[ed.levx+(ed.levy*ed.maxwidth)].enemytype+1)%25;
                ed.keydelay=6;
                ed.notedelay=45;
                ed.note="Enemy Type Changed";
            }
            if(key.keymap[SDLK_F4] && ed.keydelay==0)
            {
                ed.keydelay=6;
                ed.boundarytype=1;
                ed.boundarymod=1;
            }
            if(key.keymap[SDLK_F5] && ed.keydelay==0)
            {
                ed.keydelay=6;
                ed.boundarytype=2;
                ed.boundarymod=1;
            }
            if (key.keymap[SDLK_F6] && ed.keydelay == 0) {
                int newaltstate = ed.getnumaltstates(ed.levx, ed.levy) + 1;
                ed.addaltstate(ed.levx, ed.levy, newaltstate);
                ed.keydelay = 6;
                ed.notedelay = 45;
                // But did we get a new alt state?
                if (ed.getedaltstatenum(ed.levx, ed.levy, newaltstate) == -1) {
                    // Don't switch to the new alt state, or we'll segfault!
                    ed.note = "ERROR: Couldn't add new alt state";
                } else {
                    ed.note = "Added new alt state " + help.String(newaltstate);
                    ed.levaltstate = newaltstate;
                }
            }
            if (key.keymap[SDLK_F7] && ed.keydelay == 0) {
                if (ed.levaltstate == 0) {
                    ed.note = "Cannot remove main state";
                } else {
                    ed.removealtstate(ed.levx, ed.levy, ed.levaltstate);
                    ed.note = "Removed alt state " + help.String(ed.levaltstate);
                    ed.levaltstate--;
                }
                ed.keydelay = 6;
                ed.notedelay = 45;
            }
            if(key.keymap[SDLK_F8] && ed.keydelay==0) {
                if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].tower) {
                    ed.level[ed.levx+(ed.levy*ed.maxwidth)].tower=0;
                    ed.note="Tower Mode Disabled";
                } else {
                    ed.enable_tower();
                    ed.note="Tower Mode Enabled";
                }
                dwgfx.backgrounddrawn=false;

                ed.notedelay=45;
                ed.updatetiles=true;
                ed.keydelay=6;
            }
            if(key.keymap[SDLK_F10] && ed.keydelay==0)
            {
                if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].directmode==1)
                {
                    ed.level[ed.levx+(ed.levy*ed.maxwidth)].directmode=0;
                    ed.note="Direct Mode Disabled";
                }
                else
                {
                    ed.level[ed.levx+(ed.levy*ed.maxwidth)].directmode=1;
                    ed.note="Direct Mode Enabled";
                }
                dwgfx.backgrounddrawn=false;

                ed.notedelay=45;
                ed.updatetiles=true;
                ed.keydelay=6;
            }
            if(key.keymap[SDLK_1]) ed.drawmode=0;
            if(key.keymap[SDLK_2]) ed.drawmode=1;
            if(key.keymap[SDLK_3]) ed.drawmode=2;
            if(key.keymap[SDLK_4]) ed.drawmode=3;
            if(key.keymap[SDLK_5]) ed.drawmode=4;
            if(key.keymap[SDLK_6]) ed.drawmode=5;
            if(key.keymap[SDLK_7]) ed.drawmode=6;
            if(key.keymap[SDLK_8]) ed.drawmode=7;
            if(key.keymap[SDLK_9]) ed.drawmode=8;
            if(key.keymap[SDLK_0]) ed.drawmode=9;
            if(key.keymap[SDLK_r]) ed.drawmode=10;
            if(key.keymap[SDLK_t]) ed.drawmode=11;
            if(key.keymap[SDLK_y]) ed.drawmode=12;
            if(key.keymap[SDLK_u]) ed.drawmode=13;
            if(key.keymap[SDLK_i]) ed.drawmode=14;
            if(key.keymap[SDLK_o]) ed.drawmode=15;
            if(key.keymap[SDLK_p]) ed.drawmode=16;

            if(key.keymap[SDLK_w] && ed.keydelay==0)
            {
                int j=0, tx=0, ty=0;
                for(int i=0; i<EditorData::GetInstance().numedentities; i++)
                {
                    if(edentity[i].t==50)
                    {
                        tx=(edentity[i].p1-(edentity[i].p1%40))/40;
                        ty=(edentity[i].p2-(edentity[i].p2%30))/30;
                        if(tx==ed.levx && ty==ed.levy &&
                           edentity[i].state==ed.levaltstate &&
                           edentity[i].intower==tower)
                        {
                            j++;
                        }
                    }
                }
                if(j>0)
                {
                    ed.note="ERROR: Cannot have both warp types";
                    ed.notedelay=45;
                }
                else
                {
                    ed.level[ed.levx+(ed.levy*ed.maxwidth)].warpdir=(ed.level[ed.levx+(ed.levy*ed.maxwidth)].warpdir+1)%4;
                    if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].warpdir==0)
                    {
                        ed.note="Room warping disabled";
                        ed.notedelay=45;
                        dwgfx.backgrounddrawn=false;
                    }
                    else if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].warpdir==1)
                    {
                        ed.note="Room warps horizontally";
                        ed.notedelay=45;
                        dwgfx.backgrounddrawn=false;
                    }
                    else if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].warpdir==2)
                    {
                        ed.note="Room warps vertically";
                        ed.notedelay=45;
                        dwgfx.backgrounddrawn=false;
                    }
                    else if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].warpdir==3)
                    {
                        ed.note="Room warps in all directions";
                        ed.notedelay=45;
                        dwgfx.backgrounddrawn=false;
                    }
                }
                ed.keydelay=6;
            }
            if (key.keymap[SDLK_e] && ed.keydelay==0) {
                ed.keydelay = 6;
                ed.getlin(key, TEXT_ROOMNAME, "Enter new room name:",
                          &(ed.level[ed.levx+(ed.levy*ed.maxwidth)].roomname));
                game.mapheld=true;
            }
            if (key.keymap[SDLK_g] && ed.keydelay==0) {
                ed.keydelay = 6;
                ed.getlin(key, TEXT_GOTOROOM, "Enter room coordinates (x,y):",
                          NULL);
                game.mapheld=true;
            }
            if (key.keymap[SDLK_a] && ed.keydelay == 0) {
                if (ed.getedaltstatenum(ed.levx, ed.levy, ed.levaltstate + 1) != -1) {
                    ed.levaltstate++;
                    ed.note = "Switched to alt state " + help.String(ed.levaltstate);
                } else if (ed.levaltstate == 0) {
                    ed.note = "No alt states in this room";
                } else {
                    ed.levaltstate = 0;
                    ed.note = "Switched to main state";
                }
                ed.notedelay = 45;
                ed.keydelay = 6;
            }

            //Save and load
            if(key.keymap[SDLK_s] && ed.keydelay==0)
            {
                ed.keydelay = 6;
                ed.getlin(key, TEXT_SAVE, "Enter map filename to save map as:",
                          &(ed.filename));
                game.mapheld=true;
                dwgfx.backgrounddrawn=false;
            }

            if(key.keymap[SDLK_l] && ed.keydelay==0)
            {
                ed.keydelay = 6;
                ed.getlin(key, TEXT_LOAD, "Enter map filename to load:",
                          &(ed.filename));
                game.mapheld=true;
                dwgfx.backgrounddrawn=false;
            }

            if(!game.press_map) game.mapheld=false;
            if (!game.mapheld)
            {
                if(game.press_map)
                {
                    game.mapheld=true;

                    //Ok! Scan the room for the closest checkpoint
                    int testeditor=-1;
                    int startpoint=0;
                    //First up; is there a start point on this screen?
                    for(int i=0; i<EditorData::GetInstance().numedentities; i++)
                    {
                        //if() on screen
                        if(edentity[i].t==16 && testeditor==-1)
                        {
                            int tx=(edentity[i].x-(edentity[i].x%40))/40;
                            int ty=(edentity[i].y-(edentity[i].y%30))/30;
                            if(tx==ed.levx && ty==ed.levy &&
                               edentity[i].state==ed.levaltstate &&
                               edentity[i].intower==tower)
                            {
                                testeditor=i;
                                startpoint=1;
                            }
                        }
                    }
                    if(testeditor==-1)
                    {
                        //Ok, settle for a check point
                        for(int i=0; i<EditorData::GetInstance().numedentities; i++)
                        {
                            //if() on screen
                            if(edentity[i].t==10 && testeditor==-1)
                            {
                                int tx=(edentity[i].x-(edentity[i].x%40))/40;
                                int ty=(edentity[i].y-(edentity[i].y%30))/30;
                                if(tx==ed.levx && ty==ed.levy &&
                                   edentity[i].state==ed.levaltstate &&
                                   edentity[i].intower==tower)
                                {
                                    testeditor=i;
                                }
                            }
                        }
                    }

                    if(testeditor==-1)
                    {
                        ed.note="ERROR: No checkpoint to spawn at";
                        ed.notedelay=45;
                    }
                    else
                    {
                        if(startpoint==0)
                        {
                            //Checkpoint spawn
                            int tx=(edentity[testeditor].x-(edentity[testeditor].x%40))/40;
                            int ty=(edentity[testeditor].y-(edentity[testeditor].y%30))/30;
                            game.edsavex = (edentity[testeditor].x%40)*8;
                            game.edsavey = (edentity[testeditor].y%30)*8;
                            game.edsavex += edentity[testeditor].subx;
                            game.edsavey += edentity[testeditor].suby;
                            game.edsaverx = 100+tx;
                            game.edsavery = 100+ty;
                            game.edsavegc = edentity[testeditor].p1;
                            if(game.edsavegc==0)
                            {
                                game.edsavey--;
                            }
                            else
                            {
                                game.edsavey-=8;
                            }
                            game.edsavedir = 0;
                        }
                        else
                        {
                            //Start point spawn
                            int tx=(edentity[testeditor].x-(edentity[testeditor].x%40))/40;
                            int ty=(edentity[testeditor].y-(edentity[testeditor].y%30))/30;
                            game.edsavex = ((edentity[testeditor].x%40)*8)-4;
                            game.edsavey = (edentity[testeditor].y%30)*8;
                            game.edsavex += edentity[testeditor].subx;
                            game.edsavey += edentity[testeditor].suby;
                            game.edsaverx = 100+tx;
                            game.edsavery = 100+ty;
                            game.edsavegc = 0;
                            game.edsavey--;
                            game.edsavedir=1-edentity[testeditor].p1;
                        }

                        music.stopmusic();
                        dwgfx.backgrounddrawn=false;
                        script.startgamemode(21, key, dwgfx, game, map, obj, help, music);
                    }
                    //Return to game
                    //game.gamestate=GAMEMODE;
                    /*if(dwgfx.fademode==0)
                    {
                    dwgfx.fademode = 2;
                    music.fadeout();
                    }*/
                }
            }

            if(key.keymap[SDLK_x])
            {
                ed.xmod=true;
            }
            else
            {
                ed.xmod=false;
            }


            if(key.keymap[SDLK_z])
            {
                ed.zmod=true;
            }
            else
            {
                ed.zmod=false;
            }

            if (key.keymap[SDLK_UP] || key.keymap[SDLK_DOWN] ||
                key.keymap[SDLK_LEFT] || key.keymap[SDLK_RIGHT]) {
                ed.keydelay = 6;
                if (key.keymap[SDLK_UP])
                    ed.levy--;
                else if (key.keymap[SDLK_DOWN])
                    ed.levy++;
                else if (key.keymap[SDLK_LEFT])
                    ed.levx--;
                else if (key.keymap[SDLK_RIGHT])
                    ed.levx++;
                ed.updatetiles = true;
                ed.changeroom = true;
                dwgfx.backgrounddrawn=false;
                ed.levaltstate = 0;
                ed.levx = (ed.levx + ed.mapwidth) % ed.mapwidth;
                ed.levy = (ed.levy + ed.mapheight) % ed.mapheight;
            }

            if(key.keymap[SDLK_SPACE]) {
                ed.spacemod = !ed.spacemod;
                ed.keydelay=6;
            }
        }

        if(!ed.settingsmod && !ed.trialstartpoint)
        {
            if(ed.boundarymod>0)
            {
                if(key.leftbutton)
                {
                    if(ed.lclickdelay==0)
                    {
                        if(ed.boundarymod==1)
                        {
                            ed.lclickdelay=1;
                            ed.boundx1=(ed.tilex*8);
                            ed.boundy1=(ed.tiley*8);
                            ed.boundarymod=2;
                            if (ed.boundarytype == 4) {
                                int tmp=ed.levx+(ed.levy*ed.maxwidth);
                                ed.level[tmp].tower_row=ed.tiley + ed.ypos;
                                ed.boundarymod = 0;
                            }
                        }
                        else if(ed.boundarymod==2)
                        {
                            if((ed.tilex*8)+8>=ed.boundx1 || (ed.tiley*8)+8>=ed.boundy1)
                            {
                                ed.boundx2=(ed.tilex*8)+8;
                                ed.boundy2=(ed.tiley*8)+8;
                            }
                            else
                            {
                                ed.boundx2=ed.boundx1+8;
                                ed.boundy2=ed.boundy1+8;
                            }
                            if(ed.boundarytype==0 || ed.boundarytype==5)
                            {
                                //Script trigger
                                ed.lclickdelay=1;
                                ed.textent=EditorData::GetInstance().numedentities;
                                ed.getlin(key, TEXT_SCRIPT, "Enter script name:",
                                          &(edentity[ed.textent].scriptname));
                                addedentity((ed.boundx1/8)+(ed.levx*40),(ed.boundy1/8)+ (ed.levy*30),19,
                                            (ed.boundx2-ed.boundx1)/8, (ed.boundy2-ed.boundy1)/8);
                                if (ed.boundarytype==5)
                                    // Don't forget to subtract 1 from index because addedentity incremented it
                                    edentity[EditorData::GetInstance().numedentities-1].onetime = true;
                            }
                            else if(ed.boundarytype==4)
                            {
                                //Activity zone
                                ed.lclickdelay=1;
                                ed.textent=EditorData::GetInstance().numedentities;
                                ed.textcount = 2;
                                ed.getlin(key, TEXT_ACTIVITYZONE,
                                          "Enter activity zone text:",
                                          &(edentity[ed.textent].activityname));
                                addedentity((ed.boundx1/8)+(ed.levx*40),(ed.boundy1/8)+ (ed.levy*30),20,
                                            (ed.boundx2-ed.boundx1)/8, (ed.boundy2-ed.boundy1)/8);
                            }
                            else if(ed.boundarytype==1)
                            {
                                //Enemy bounds
                                int tmp=ed.levx+(ed.levy*ed.maxwidth);
                                ed.level[tmp].enemyx1=ed.boundx1;
                                ed.level[tmp].enemyy1=ed.boundy1;
                                ed.level[tmp].enemyx2=ed.boundx2;
                                ed.level[tmp].enemyy2=ed.boundy2;
                            }
                            else if(ed.boundarytype==2)
                            {
                                //Platform bounds
                                int tmp=ed.levx+(ed.levy*ed.maxwidth);
                                ed.level[tmp].platx1=ed.boundx1;
                                ed.level[tmp].platy1=ed.boundy1;
                                ed.level[tmp].platx2=ed.boundx2;
                                ed.level[tmp].platy2=ed.boundy2;
                            }
                            else if(ed.boundarytype==3)
                            {
                                //Copy
                            }
                            ed.boundarymod=0;
                            ed.lclickdelay=1;
                        }
                    }
                }
                else
                {
                    ed.lclickdelay=0;
                }
                if(key.rightbutton)
                {
                    ed.boundarymod=0;
                }
            }
            else if(ed.warpmod)
            {
                //Placing warp token
                if(key.leftbutton)
                {
                    if(ed.lclickdelay==0)
                    {
                        if(ed.free(ed.tilex, ed.tiley)==0)
                        {
                            edentity[ed.warpent].p1=ed.tilex+(ed.levx*40);
                            edentity[ed.warpent].p2=ed.tiley+(ed.levy*30);
                            ed.warpmod=false;
                            ed.warpent=-1;
                            ed.lclickdelay=1;
                        }
                    }
                }
                else
                {
                    ed.lclickdelay=0;
                }
                if(key.rightbutton)
                {
                    removeedentity(ed.warpent);
                    ed.warpmod=false;
                    ed.warpent=-1;
                }
            } else {
                int tx = ed.tilex;
                int ty = ed.tiley;
                if (!tower) {
                    tx += (ed.levx * 40);
                    ty += (ed.levy * 30);
                } else
                    ty += ed.ypos;

                //Mouse input
                if(key.leftbutton)
                {
                    if(ed.lclickdelay==0)
                    {
                        //Depending on current mode, place something
                        if(ed.drawmode==0)
                        {
                            //place tiles
                            //Are we in direct mode?
                            if(ed.level[ed.levx+(ed.levy*ed.maxwidth)].directmode>=1)
                            {
                                if(ed.xmod)
                                {
                                    for(int j=-2; j<3; j++)
                                    {
                                        for(int i=-2; i<3; i++)
                                        {
                                            ed.placetilelocal(ed.tilex+i, ed.tiley+j, ed.dmtile);
                                        }
                                    }
                                }
                                else if(ed.zmod)
                                {
                                    for(int j=-1; j<2; j++)
                                    {
                                        for(int i=-1; i<2; i++)
                                        {
                                            ed.placetilelocal(ed.tilex+i, ed.tiley+j, ed.dmtile);
                                        }
                                    }
                                }
                                else
                                {
                                    ed.placetilelocal(ed.tilex, ed.tiley, ed.dmtile);
                                }
                            }
                            else
                            {
                                if(ed.xmod)
                                {
                                    for(int j=-2; j<3; j++)
                                    {
                                        for(int i=-2; i<3; i++)
                                        {
                                            ed.placetilelocal(ed.tilex+i, ed.tiley+j, 80);
                                        }
                                    }
                                }
                                else if(ed.zmod)
                                {
                                    for(int j=-1; j<2; j++)
                                    {
                                        for(int i=-1; i<2; i++)
                                        {
                                            ed.placetilelocal(ed.tilex+i, ed.tiley+j, 80);
                                        }
                                    }
                                }
                                else
                                {
                                    ed.placetilelocal(ed.tilex, ed.tiley, 80);
                                }
                            }
                        }
                        else if(ed.drawmode==1)
                        {
                            //place background tiles
                            if(ed.xmod)
                            {
                                for(int j=-2; j<3; j++)
                                {
                                    for(int i=-2; i<3; i++)
                                    {
                                        ed.placetilelocal(ed.tilex+i, ed.tiley+j, 2);
                                    }
                                }
                            }
                            else if(ed.zmod)
                            {
                                for(int j=-1; j<2; j++)
                                {
                                    for(int i=-1; i<2; i++)
                                    {
                                        ed.placetilelocal(ed.tilex+i, ed.tiley+j, 2);
                                    }
                                }
                            }
                            else
                            {
                                ed.placetilelocal(ed.tilex, ed.tiley, 2);
                            }
                        }
                        else if(ed.drawmode==2)
                        {
                            //place spikes
                            ed.placetilelocal(ed.tilex, ed.tiley, 8);
                        }

                        int tmp=edentat(tx, ty, ed.levaltstate, tower);
                        if(tmp==-1)
                        {
                            //Room text and script triggers can be placed in walls
                            if(ed.drawmode==10)
                            {
                                ed.lclickdelay=1;
                                ed.textent=EditorData::GetInstance().numedentities;
                                ed.getlin(key, TEXT_ROOMTEXT, "Enter roomtext:",
                                          &(edentity[ed.textent].scriptname));
                                dwgfx.backgrounddrawn=false;
                                addedentity(tx, ty, 17);
                            }
                            else if(ed.drawmode==12)   //Script Trigger
                            {
                                if (ed.zmod) {
                                    ed.boundarytype=4;
                                } else if (ed.xmod) {
                                    ed.boundarytype=5;
                                } else {
                                    ed.boundarytype=0;
                                }
                                ed.boundx1=ed.tilex*8;
                                ed.boundy1=ed.tiley*8;
                                ed.boundarymod=2;
                                ed.lclickdelay=1;
                            }
                        }
                        if(tmp==-1 && ed.free(ed.tilex,ed.tiley)==0)
                        {
                            if(ed.drawmode==3)
                            {
                                if(ed.numtrinkets<100)
                                {
                                    addedentity(tx, ty, 9);
                                    ed.lclickdelay=1;
                                    ed.numtrinkets++;
                                }
                                else
                                {
                                    ed.note="ERROR: Max number of trinkets is 100";
                                    ed.notedelay=45;
                                }
                            }
                            else if(ed.drawmode==4)
                            {
                                addedentity(tx, ty, 10, 1);
                                ed.lclickdelay=1;
                            }
                            else if(ed.drawmode==5)
                            {
                                addedentity(tx, ty, 3);
                                ed.lclickdelay=1;
                            }
                            else if(ed.drawmode==6)
                            {
                                addedentity(tx, ty, 2, 5);
                                ed.lclickdelay=1;
                            }
                            else if(ed.drawmode==7)
                            {
                                addedentity(tx, ty, 2, 0, ed.entspeed);
                                ed.lclickdelay=1;
                            }
                            else if(ed.drawmode==8) // Enemies
                            {
                                addedentity(tx, ty, 1, 0, ed.entspeed);
                                ed.lclickdelay=1;
                            }
                            else if(ed.drawmode==9)
                            {
                                addedentity(tx, ty, 11, 0);
                                ed.lclickdelay=1;
                            }
                            else if(ed.drawmode==11)
                            {
                                ed.lclickdelay=1;
                                ed.textent=EditorData::GetInstance().numedentities;
                                ed.getlin(key, TEXT_SCRIPT, "Enter script name",
                                          &(edentity[ed.textent].scriptname));
                                addedentity(tx, ty, 18, 0);
                            }
                            else if(ed.drawmode==13)
                            {
                                ed.warpmod=true;
                                ed.warpent=EditorData::GetInstance().numedentities;
                                addedentity(tx, ty, 13);
                                ed.lclickdelay=1;
                            }
                            else if(ed.drawmode==14)
                            {
                                //Warp lines
                                if(ed.level[ed.levx+(ed.maxwidth*ed.levy)].warpdir==0)
                                {
                                    if (ed.tilex == 0)
                                        addedentity(tx, ty, 50, 0);
                                    else if (ed.tilex == 39)
                                        addedentity(tx, ty, 50, 1);
                                    else if (!tower && ed.tiley == 0)
                                        addedentity(tx, ty, 50, 2);
                                    else if (!tower && ed.tiley == 29)
                                        addedentity(tx, ty, 50, 3);
                                    else if (tower) {
                                        ed.note = "ERROR: Warp lines must be on vertical edges";
                                        ed.notedelay=45;
                                    } else {
                                        ed.note="ERROR: Warp lines must be on edges";
                                        ed.notedelay=45;
                                    }
                                } else {
                                    ed.note="ERROR: Cannot have both warp types";
                                    ed.notedelay=45;
                                }
                                ed.lclickdelay=1;
                            }
                            else if(ed.drawmode==15)  //Crewmate
                            {
                                if(ed.numcrewmates<100)
                                {
                                    addedentity(tx, ty, 15,
                                                1 + int(fRandom() * 5));
                                    ed.lclickdelay=1;
                                    ed.numcrewmates++;
                                } else {
                                    ed.note="ERROR: Max number of crewmates is 100";
                                    ed.notedelay=45;
                                }
                            }
                            else if(ed.drawmode==16)  //Start Point
                            {
                                if (ed.levaltstate != 0) {
                                    ed.note = "ERROR: Start point must be in main state";
                                    ed.notedelay = 45;
                                } else {
                                    //If there is another start point, destroy it
                                    for(int i=0; i<EditorData::GetInstance().numedentities; i++)
                                    {
                                        if(edentity[i].t==16)
                                        {
                                            removeedentity(i);
                                            i--;
                                        }
                                    }
                                    addedentity(tx, ty, 16, 0);
                                }
                                ed.lclickdelay=1;
                            }
                            else if(ed.drawmode==17)  // Flip Tokens
                            {
                                addedentity(tx, ty, 5, 181, 1);
                                ed.lclickdelay=1;
                            }
                            else if(ed.drawmode==18)  // Coins
                            {
                                addedentity(tx, ty, 8, 0);
                                ed.numcoins++;
                                //ed.lclickdelay=1;
                            }
                            else if(ed.drawmode==19)  // Teleporter
                            {
                                addedentity(tx, ty, 14);
                                ed.lclickdelay=1;
                                map.remteleporter(ed.levx, ed.levy);
                                map.setteleporter(ed.levx, ed.levy);
                            }
                            else if(ed.drawmode==-6)  // ??????
                            {
                                addedentity(tx, ty, 999);
                                ed.lclickdelay=1;
                            }
                        }
                        else if(edentity[tmp].t==1)
                        {
                            edentity[tmp].p1=(edentity[tmp].p1+1)%4;
                            ed.lclickdelay=1;
                        }
                        else if(edentity[tmp].t==2)
                        {
                            if(edentity[tmp].p1>=5)
                            {
                                edentity[tmp].p1=(edentity[tmp].p1+1)%9;
                                if(edentity[tmp].p1<5) edentity[tmp].p1=5;
                            }
                            else
                            {
                                edentity[tmp].p1=(edentity[tmp].p1+1)%4;
                            }
                            ed.lclickdelay=1;
                        }
                        else if(edentity[tmp].t==10) //Checkpoint sprite changing
                        {
                                 if (edentity[tmp].p1 == 0)
                                     edentity[tmp].p1 = 2;
                            else if (edentity[tmp].p1 == 2)
                                     edentity[tmp].p1 = 3;
                            else if (edentity[tmp].p1 == 3)
                                     edentity[tmp].p1 = 1;
                            else if (edentity[tmp].p1 == 1)
                                     edentity[tmp].p1 = 0;

                            ed.lclickdelay=1;
                        }
                        else if(edentity[tmp].t==11)
                        {
                            edentity[tmp].p1=(edentity[tmp].p1+1)%2;
                            ed.lclickdelay=1;
                        }
                        else if(edentity[tmp].t==15)
                        {
                            edentity[tmp].p1=(edentity[tmp].p1+1)%6;
                            ed.lclickdelay=1;
                        }
                        else if(edentity[tmp].t==16)
                        {
                            edentity[tmp].p1=(edentity[tmp].p1+1)%2;
                            ed.lclickdelay=1;
                        }
                        else if(edentity[tmp].t==17)
                        {
                            ed.lclickdelay=1;
                            ed.getlin(key, TEXT_ROOMTEXT, "Enter roomtext:",
                                      &(edentity[tmp].scriptname));
                            ed.textent=tmp;
                        }
                        else if(edentity[tmp].t==18)
                        {
                            ed.lclickdelay=1;
                            ed.getlin(key, TEXT_ROOMTEXT, "Enter script name:",
                                      &(edentity[ed.textent].scriptname));
                            ed.textent=tmp;

                            // A bit meh that the easiest way is doing this at the same time you start changing the script name, but oh well
                            if (edentity[tmp].p1 == 0) // Currently not flipped
                                edentity[tmp].p1 = 1; // Flip it, then
                            else if (edentity[tmp].p1 == 1) // Currently is flipped
                                edentity[tmp].p1 = 0; // Unflip it, then
                        }
                    }
                }
                else
                {
                    ed.lclickdelay=0;
                }

                if(key.rightbutton)
                {
                    //place tiles
                    if(ed.xmod)
                    {
                        for(int j=-2; j<3; j++)
                        {
                            for(int i=-2; i<3; i++)
                            {
                                ed.placetilelocal(ed.tilex+i, ed.tiley+j, 0);
                            }
                        }
                    }
                    else if(ed.zmod)
                    {
                        for(int j=-1; j<2; j++)
                        {
                            for(int i=-1; i<2; i++)
                            {
                                ed.placetilelocal(ed.tilex+i, ed.tiley+j, 0);
                            }
                        }
                    }
                    else
                    {
                        ed.placetilelocal(ed.tilex, ed.tiley, 0);
                    }
                    for(int i=0; i<EditorData::GetInstance().numedentities; i++)
                    {
                        if (edentity[i].x==tx && edentity[i].y==ty &&
                            edentity[i].state==ed.levaltstate &&
                            edentity[i].intower==tower) {
                            if (edentity[i].t==9) ed.numtrinkets--;
                            if (edentity[i].t==8) ed.numcoins--;
                            if (edentity[i].t==15) ed.numcrewmates--;
                            if (edentity[i].t==14) {
                                map.remteleporter(ed.levx, ed.levy);
                            }
                            removeedentity(i);
                        }
                    }
                }

                if(key.middlebutton)
                {
                    ed.dmtile=ed.gettilelocal(ed.tilex, ed.tiley);
                }
            }
        }
    }

    if(ed.updatetiles && ed.level[ed.levx + (ed.levy*ed.maxwidth)].directmode==0)
    {
        ed.updatetiles=false;
        //Correctly set the tiles in the current room
        switch(ed.level[ed.levx + (ed.levy*ed.maxwidth)].tileset)
        {
        case 0: //The Space Station
            for(int j=0; j<30; j++)
            {
                for(int i=0; i<40; i++)
                {
                    if(ed.gettilelocal(i, j)>=3 && ed.gettilelocal(i, j)<80)
                    {
                        //Fix spikes
                        ed.settilelocal(i, j, ed.spikedir(i,j));
                    }
                    else if(ed.gettilelocal(i, j)==2 || ed.gettilelocal(i, j)>=680)
                    {
                        //Fix background
                        ed.settilelocal(i, j, ed.backedgetile(i,j)+ed.backbase(ed.levx,ed.levy));
                    }
                    else if(ed.gettilelocal(i, j)>0)
                    {
                        //Fix tiles
                        ed.settilelocal(i, j, ed.edgetile(i,j)+ed.base(ed.levx,ed.levy));
                    }
                }
            }
            break;
        case 1: //Outside
            for(int j=0; j<30; j++)
            {
                for(int i=0; i<40; i++)
                {
                    if(ed.gettilelocal(i, j)>=3 && ed.gettilelocal(i, j)<80)
                    {
                        //Fix spikes
                        ed.settilelocal(i, j, ed.spikedir(i,j));
                    }
                    else if(ed.gettilelocal(i, j)==2 || ed.gettilelocal(i, j)>=680)
                    {
                        //Fix background
                        ed.settilelocal(i, j, ed.outsideedgetile(i,j)+ed.backbase(ed.levx,ed.levy));
                    }
                    else if(ed.gettilelocal(i, j)>0)
                    {
                        //Fix tiles
                        ed.settilelocal(i, j, ed.edgetile(i,j)+ed.base(ed.levx,ed.levy));
                    }
                }
            }
            break;
        case 2: //Lab
            for(int j=0; j<30; j++)
            {
                for(int i=0; i<40; i++)
                {
                    if(ed.gettilelocal(i, j)>=3 && ed.gettilelocal(i, j)<80)
                    {
                        //Fix spikes
                        ed.settilelocal(i, j, ed.labspikedir(i,j, ed.level[ed.levx + (ed.maxwidth*ed.levy)].tilecol));
                    }
                    else if(ed.gettilelocal(i, j)==2 || ed.gettilelocal(i, j)>=680)
                    {
                        //Fix background
                        ed.settilelocal(i, j, 713);
                    }
                    else if(ed.gettilelocal(i, j)>0)
                    {
                        //Fix tiles
                        ed.settilelocal(i, j, ed.edgetile(i,j)+ed.base(ed.levx,ed.levy));
                    }
                }
            }
            break;
        case 3: //Warp Zone/Intermission
            for(int j=0; j<30; j++)
            {
                for(int i=0; i<40; i++)
                {
                    if(ed.gettilelocal(i, j)>=3 && ed.gettilelocal(i, j)<80)
                    {
                        //Fix spikes
                        ed.settilelocal(i, j, ed.spikedir(i,j));
                    }
                    else if(ed.gettilelocal(i, j)==2 || ed.gettilelocal(i, j)>=680)
                    {
                        //Fix background
                        ed.settilelocal(i, j, 713);//ed.backbase(ed.levx,ed.levy);
                    }
                    else if(ed.gettilelocal(i, j)>0)
                    {
                        //Fix tiles
                        //ed.settilelocal(i, j, ed.warpzoneedgetile(i,j)+ed.base(ed.levx,ed.levy));
                        ed.settilelocal(i, j, ed.edgetile(i,j)+ed.base(ed.levx,ed.levy));
                    }
                }
            }
            break;
        case 4: //The ship
            for(int j=0; j<30; j++)
            {
                for(int i=0; i<40; i++)
                {
                    if(ed.gettilelocal(i, j)>=3 && ed.gettilelocal(i, j)<80)
                    {
                        //Fix spikes
                        ed.settilelocal(i, j, ed.spikedir(i,j));
                    }
                    else if(ed.gettilelocal(i, j)==2 || ed.gettilelocal(i, j)>=680)
                    {
                        //Fix background
                        ed.settilelocal(i, j, ed.backedgetile(i,j)+ed.backbase(ed.levx,ed.levy));
                    }
                    else if(ed.gettilelocal(i, j)>0)
                    {
                        //Fix tiles
                        ed.settilelocal(i, j, ed.edgetile(i,j)+ed.base(ed.levx,ed.levy));
                    }
                }
            }
            break;
        case 5: //The Tower
            for(int j=0; j<30; j++) {
                if (ed.intower() &&
                    ((j + ed.ypos) < 0 ||
                     (j + ed.ypos) >= ed.tower_size(ed.get_tower(ed.levx,
                                                                 ed.levy))))
                    continue;

                for(int i=0; i<40; i++)
                {
                    if(ed.gettiletyplocal(i, j) == TILE_SPIKE)
                    {
                        //Fix spikes
                        ed.settilelocal(i, j, ed.towerspikedir(i,j)+ed.spikebase(ed.levx,ed.levy));
                    }
                    else if(ed.gettiletyplocal(i, j) == TILE_BACKGROUND)
                    {
                        //Fix background
                        ed.settilelocal(i, j, ed.backbase(ed.levx,ed.levy));
                    }
                    else if(ed.gettiletyplocal(i, j) == TILE_FOREGROUND)
                    {
                        //Fix tiles
                        ed.settilelocal(i, j, ed.toweredgetile(i,j)+ed.base(ed.levx,ed.levy));
                    }
                }
            }
            break;
        case 6: //Custom Set 1
            break;
        case 7: //Custom Set 2
            break;
        case 8: //Custom Set 3
            break;
        case 9: //Custom Set 4
            break;
        }
    }
}

void replaceAll(std::string& str, const std::string& from, const std::string& to) {
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos) {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

std::string find_tag(std::string_view buf, std::string_view start, std::string_view end) {
    auto title_tag = buf.find(start);
    auto title_start = title_tag + start.size();
    auto title_close = buf.find(end, title_start);
    auto title_len = title_close - title_start;
    std::string value(buf.substr(title_start, title_len));
    replaceAll(value, "&quot;", "\"");
    replaceAll(value, "&amp;", "&");
    replaceAll(value, "&apos;", "'");
    replaceAll(value, "&lt;", "<");
    replaceAll(value, "&gt;", ">");
    size_t start_pos = 0;
    while ((start_pos = value.find("&#", start_pos)) != std::string::npos) {
        auto end = value.find(';', start_pos);
        int character = std::stoi(value.substr(start_pos + 2, end - start_pos));
        int utf32[] = {character, 0};
        std::string utf8;
        utf8::utf32to8(utf32, utf32 + 1, std::back_inserter(utf8));
        value.replace(start_pos, end - start_pos + 1, utf8);
    }
    return value;
}

int editorclass::getedaltstatenum(int rxi, int ryi, int state)
{
    for (size_t i = 0; i < altstates.size(); i++)
        if (altstates[i].x == rxi && altstates[i].y == ryi && altstates[i].state == state)
            return i;

    return -1;
}

void editorclass::addaltstate(int rxi, int ryi, int state)
{
    for (size_t i = 0; i < altstates.size(); i++)
        if (altstates[i].x == -1 || altstates[i].y == -1) {
            altstates[i].x = rxi;
            altstates[i].y = ryi;
            altstates[i].state = state;

            // Copy the tiles from the main state
            for (int ty = 0; ty < 30; ty++)
                for (int tx = 0; tx < 40; tx++)
                    altstates[i].tiles[tx + ty*40] = contents[tx + rxi*40 + vmult[ty + ryi*30]];

            // Copy the entities from the main state
            // But since we're incrementing numedentities, don't use it as a bounds check!
            int limit = EditorData::GetInstance().numedentities;
            for (int i = 0; i < limit; i++)
                if (edentity[i].x >= rxi*40 && edentity[i].x < (rxi+1)*40
                && edentity[i].y >= ryi*30 && edentity[i].y < (ryi+1)*30
                && edentity[i].state == 0 && edentity[i].intower == 0) {
                    if (edentity[i].t == 9) {
                        // TODO: If removing the 100 trinkets limit, update this
                        if (numtrinkets >= 100)
                            continue;
                        numtrinkets++;
                    } else if (edentity[i].t == 15) {
                        // TODO: If removing the 100 crewmates limit, update this
                        if (numcrewmates >= 100)
                            continue;
                        numcrewmates++;
                    } else if (edentity[i].t == 16) {
                        // Don't copy the start point
                        continue;
                    }
                    // Why does copyedentity() copy from argument #2 to argument #1 instead of 1 to 2??? Makes no sense
                    copyedentity(EditorData::GetInstance().numedentities, i);
                    edentity[EditorData::GetInstance().numedentities].state = state;
                    edentity[EditorData::GetInstance().numedentities].intower = 0;
                    EditorData::GetInstance().numedentities++;
                }

            break;
        }
}

void editorclass::removealtstate(int rxi, int ryi, int state)
{
    int n = getedaltstatenum(rxi, ryi, state);
    if (n == -1) {
        printf("Trying to remove nonexistent altstate (%i,%i)@%i!\n", rxi, ryi, state);
        return;
    }

    altstates[n].x = -1;
    altstates[n].y = -1;

    for (int i = 0; i < EditorData::GetInstance().numedentities; i++)
        if (edentity[i].x >= rxi*40 && edentity[i].x < (rxi+1)*40
        && edentity[i].y >= ryi*30 && edentity[i].y < (ryi+1)*30
        && edentity[i].state == state && edentity[i].intower == 0) {
            removeedentity(i);
            if (edentity[i].t == 9)
                numtrinkets--;
            else if (edentity[i].t == 15)
                numcrewmates--;
        }

    // Ok, now update the rest
    // This looks like it's O(n^2), and, well, it probably is lmao
    int dothisstate = state;
    while (true) {
        dothisstate++;
        int nextstate = getedaltstatenum(rxi, ryi, dothisstate);
        if (nextstate == -1)
            break;
        altstates[nextstate].state--;
    }

    // Don't forget to update entities
    for (int i = 0; i < EditorData::GetInstance().numedentities; i++)
        if (edentity[i].x >= rxi*40 && edentity[i].x < (rxi+1)*40
        && edentity[i].y >= ryi*30 && edentity[i].y < (ryi+1)*30
        && edentity[i].state > state && edentity[i].intower == 0)
            edentity[i].state--;
}

int editorclass::getnumaltstates(int rxi, int ryi)
{
    int num = 0;
    for (size_t i = 0; i < altstates.size(); i++)
        if (altstates[i].x == rxi && altstates[i].y == ryi)
            num++;
    return num;
}

#define TAG_FINDER(NAME, TAG) std::string NAME(std::string_view buf) { return find_tag(buf, "<" TAG ">", "</" TAG ">"); }

TAG_FINDER(find_title, "Title");
TAG_FINDER(find_desc1, "Desc1");
TAG_FINDER(find_desc2, "Desc2");
TAG_FINDER(find_desc3, "Desc3");
TAG_FINDER(find_creator, "Creator");
TAG_FINDER(find_website, "website");
TAG_FINDER(find_created, "Created");
TAG_FINDER(find_modified, "Modified");
TAG_FINDER(find_modifiers, "Modifiers");

#undef TAG_FINDER
