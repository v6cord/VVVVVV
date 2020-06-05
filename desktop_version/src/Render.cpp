#include "Render.h"

#include "Graphics.h"
#include "UtilityClass.h"
#include "Maths.h"
#include "Entity.h"
#include "Map.h"
#include "Script.h"
#include "FileSystemUtils.h"

#include "MakeAndPlay.h"
#include <cstring>
#include <stdio.h>

extern scriptclass script;

int tr;
int tg;
int tb;

std::vector<std::string> changelog = {
    // [line] is a huge line, have fun with that
    // The text should not be bigger than this line:
    // --------------------------------------
    // (vim users: `:set cc=46`)
    "Version c1.0-pre2",
    "[line]",
    "- enablefog/disablefog are now",
    "  fog(enable/disable)",
    "- enableflip/disableflip are now",
    "  toggleflip(enable/disable)",
    "- enableinfiniflip/disableinfiniflip are",
    "  now infiniflip(enable/disable)",
    "- enablesuicide/disablesuicide are now",
    "  suicide(enable/disable)",
    "- nointerrupt/yesinterrupt are now",
    "  setinterrupt(on/off)",
    "- playmusicfile(file) has been removed",
    "- Instead of using playmusicfile(), just",
    "  use playfile(file[,id]).",
    "  To get much of the additional",
    "  functionality of playmusicfile(),",
    "  use the newly-added callback",
    "  on_custom_sfx_end, which sets %path%.",
    "- Added optional third argument to",
    "  playfile() for loop count",
    "- unloadscriptimages - unload images",
    "  loaded by drawimage",
    "- createdamage(x,y,w,h) - makes a region",
    "  that hurts the player like spikes",
    "- Fixed changeplayercolor alias of",
    "  changeplayercolour not working",
    "- Added being able to use tiles4.png,",
    "  tiles5.png, tiles6.png, etc. and using",
    "  F9 to switch between them in the",
    "  editor",
    "- Fixed one-way stickiness in VCE levels",
    "- Removed createentity's color argument",
    "  (6th argument is now enemy type)",
    "- Fixed ifvar() not working with the",
    "  less-than and greater-than operators,",
    "  or their \"or equal to\" counterparts",
    "- Fixed an infinite loop that could",
    "  happen if you died while a 1x1",
    "  quicksand was in the process of",
    "  crumbling in a vanilla level",
    "- Removed the limit where every 21st",
    "  text box in a row would not show up",
    "  (the removal of this limit only",
    "  applies in VCE levels)",
    "- reloadonetime(script) - allow re-",
    "  triggering of one-time script boxes",
    "  with the given script name",
    "- Mannequins, ZZT centipedes, and Edge",
    "  Games fish now face properly",
    "- Added box and bus enemy types",
    "- drawimagemasked(x,y,filename,",
    "  mask_filename[,mask_x[,mask_y]]) -",
    "  draw an image on the screen for one",
    "  frame that uses the alpha values from",
    "  another image",
    "- togglepause(on/off) - toggle opening",
    "  the map or exiting playtesting through",
    "  enter",
    "- Large coins (10/20/50/100) now exist,",
    "  and they give you a different amount",
    "  of coins depending on which size they",
    "  are. You can place these with Z/X/C/V",
    "- ifvar() is now",
    "  ifvar(expression,script)",
    "- You can now use setcheckpoint() like",
    "  setcheckpoint(roomx,roomy,posx,posy,",
    "  flipped)",
    "- The game no longer crashes if you give",
    "  an invalid filename to a command",
    "- Added being able to use sprites2.png,",
    "  sprites3.png, sprites4.png, etc.",
    "  and using Ctrl+F9 to switch between",
    "  them in the editor",
    "",
    "Version c1.0-pre1",
    "[line]",
    "- Removed having to use a load script",
    "  to use internal scripting",
    "- Added automatic loading of custom",
    "  assets - make a folder with the same",
    "  name as your level file, and put the",
    "  assets in there.",
    "- pdelay(n) - a delay(n) that doesn't",
    "  lock the player's movement",
    "- setroomname() - sets the roomname to",
    "  the next line",
    "- settile(x,y,tile) - place a tile",
    "  temporarily in the room",
    "- You can now use text() like",
    "  text(r,g,b,x,y,lines) - r,g,b is 0-255",
    "  - if r,g,b is 0,0,0 only the text will",
    "  show up and not the text box",
    "- reloadroom() - reloads the current",
    "  room",
    "- toceil() - inverted tofloor()",
    "- playfile(file[,id]) - play a file as",
    "  a sound effect. if you specify an id,",
    "  the file loops",
    "- stopfile(id) - stops playing a looping",
    "  audio file",
    "- ifnotflag(n,script) - an inverted",
    "  version of ifflag(n,script)",
    "- drawtext(x,y,r,g,b,center,type,scale)",
    "  draw text for one frame, the text you",
    "  want to display should be on the next",
    "  line - r,g,b is 0-255, center is 0/1/",
    "  true/false, type should be 0 for",
    "  normal text, 1 for bordered text and",
    "  2 for big text; use an optional eighth",
    "  argument for text scale (default is 2)",
    "- drawrect(x,y,w,h,r,g,b[,a]) - draw a",
    "  rectangle for one frame - r,g,b,a is",
    "  0-255",
    "- drawimage(x,y,filename[,centered",
    "  [,alpha[,background[,blend]]]]) -",
    "  draw an image on the screen for one",
    "  frame (alpha 0-255, background true/",
    "  false, blend none/blend/add/mod)",
    "- loadimage(filename) - add the image",
    "  to the cache without actually drawing",
    "  it",
    "- drawimagepersist(x,y,filename",
    "  [,centered[,alpha[,background[,blend]]",
    "  ]]) - same as drawimage(), but it",
    "  stays on screen for more than a frame",
    "  - it sets %return% to its id, so",
    "  you're able to remove it",
    "- removeimage(id) - remove a persistent",
    "  image",
    "- drawpixel(x,y,r,g,b) - draw a pixel",
    "  on the screen for one frame",
    "- followposition now works for the",
    "  player",
    "- There's now an option to disable only",
    "  the music",
    "- The following limits have been",
    "  removed:",
    "  - 500 scripts",
    "  - 500 script lines",
    "  - 200 entities in one room",
    "  - 100 roomtexts in one room",
    "  - 3000 placeable entities",
    "- destroy(platformsreal) - A version of",
    "  destroy(platforms) that isn't bugged",
    "- destroy(enemies)",
    "- destroy(trinkets)",
    "- destroy(warplines)",
    "- destroy(checkpoints)",
    "- destroy(all)",
    "- destroy(conveyors)",
    "- killplayer()",
    "- customquicksave() - quicksaves the",
    "  level",
    "- niceplay() - use this for better area",
    "  music transitions",
    "- destroy(terminals)",
    "- destroy(scriptboxes)",
    "- destroy(disappearingplatforms)",
    "- destroy(1x1quicksand)",
    "- destroy(coins)",
    "- destroy(gravitytokens)",
    "- destroy(roomtext)",
    "- destroy(crewmates) - destroy non-",
    "  rescuable crewmates",
    "- destroy(customcrewmates) - destroy",
    "  rescuable crewmates",
    "- destroy(teleporter)",
    "- destroy(activityzones)",
    "- inf - like do(n), but an infinite",
    "  amount of times",
    "- Add seventh argument to createcrewman,",
    "  if it is flip spawn a flipped crewmate",
    "- fatal_left() - makes the left side of",
    "  the screen deadly",
    "- fatal_right() - makes the right side",
    "  of the screen deadly",
    "- fatal_top() - makes the top of the",
    "  screen deadly",
    "- fatal_bottom() - makes the bottom of",
    "  the screen deadly",
    "- ifrand(n,script) - has a 1/n chance to",
    "  execute the script",
    "- gotocheckpoint() - teleports the",
    "  player to their last checkpoint",
    "- Added 6th argument to createentity",
    "  that sets the raw color",
    "- Added 7th argument to createentity",
    "  that sets enemy type",
    "- Automatically pause/unpause all audio",
    "  on focus change",
    "- Instead of defaulting to gray, assume",
    "  unknown colors in createcrewman/",
    "  changecolour are internal ones",
    "- Added an argument to createcrewman to",
    "  set a name (instead of pink/blue/etc),",
    "  useful in combination with the above",
    "  or when having multiple crewmen of the",
    "  same color",
    "- Added color aliases for all colour",
    "  functions",
    "- Added optional parameter to play() to",
    "  set the fade-in time in milliseconds",
    "  (default 3000)",
    "- markmap(x,y,tile) to put a tile on any",
    "  minimap coordinate",
    "- unmarkmap(x,y) to undo the above",
    "- mapimage(image.png) to use any map",
    "  image",
    "- automapimage to undo",
    "- enablefog/disablefog to enable/disable",
    "  hiding unexplored rooms (called fog in",
    "  the code)",
    "- All names (including player) now work",
    "  with all functions",
    "- Added the following enemy types:",
    "  - Hourglasses",
    "  - Yes Men",
    "  - Stop signs",
    "  - Linear Colliders",
    "  - Red pills",
    "  - Mannequins",
    "  - ZZT centipedes",
    "  - OBEY",
    "  - Edge Games",
    "  - Trench Warfare",
    "  - Tomb of Mad Carew",
    "  - Brass Sent Us Under The Top",
    "  - Wheel of Fortune",
    "  - TRUTH",
    "  - Skeletons",
    "  - Vertigo",
    "- Fixed enemies in Warp Zone gray not",
    "  being gray",
    "- createroomtext(x,y) - x,y in pixels,",
    "  roomtext on next line",
    "- createscriptbox(x,y,w,h,script) -",
    "  x,y,w,h in pixels",
    "- Allowed placed terminals to use any",
    "  sprite they want",
    "- You can now place flipped terminals",
    "- markers(hide/show) - disable/reenable",
    "  markmap() markers",
    "- setspeed(speed) - set player speed, 3",
    "  by default",
    "- setvelocity(velocity) - push the",
    "  player (affected by inertia)",
    "- pinf - variant of inf to automatically",
    "  pdelay(1) if no delay occurred",
    "- noautobars() - to disable automatic",
    "  cutscene bars from a script",
    "- finalstretch(on/off) - toggle Final",
    "  Level palette swap",
    "- reloadscriptboxes() - reload script",
    "  boxes without affecting entities",
    "- puntilbars() - untilbars() with pdelay",
    "- puntilfade() - untilfade() with pdelay",
    "- You can now use the Warp Zone gray",
    "  tileset in the editor",
    "- disableflip - disables flipping",
    "- enableflip - enables flipping",
    "- enableinfiniflip - enables flipping in",
    "  mid-air",
    "- disableinfiniflip - disables flipping",
    "  in mid-air",
    "- untilmusic() - wait until the current",
    "  track has finished fading in or out",
    "- puntilmusic() - untilmusic with pdelay",
    "- Added a label and goto system - write",
    "  down .label on a line, then to go to",
    "  it, use .label as a script name if",
    "  you're in the same script, or write",
    "  scriptname.label to jump to the label",
    "  from another script",
    "- ifvce(script) - detect if game is",
    "  VVVVVV: Community Edition or not",
    "- ifmod(mod,script) - checks for a mod -",
    "  mod can be mmmmmm (checks if MMMMMM",
    "  is installed), mmmmmm_on/mmmmmm_",
    "  enabled/mmmmmm_off/mmmmmm_disabled",
    "  (checks if MMMMMM is installed AND",
    "  enabled/disabled) or unifont (checks",
    "  if Unifont is installed)",
    "- disablesuicide - disable pressing R",
    "- enablesuicide - enable pressing R",
    "- customactivityzone(x,y,w,h,color,",
    "  script) OR customactivityzone(x,y,w,h,",
    "  r,g,b,script) - x,y,w,h in pixels,",
    "  color is red/orange/yellow/green/cyan",
    "  blue/pink/purple (actually pink)/white",
    "  /gray, if invalid it defaults to gray,",
    "  r,g,b is 0-255, prompt goes on the",
    "  next line",
    "- Fixed the 2-frame delay to execute a",
    "  script when entering a room",
    "- position(centerx,<line>) - ",
    "  horizontally center the text box",
    "  around the line x=<line>",
    "- position(centery,<line>) - vertically",
    "  center the text box around the line y=",
    "  line",
    "- Added the Tower Hallway tileset",
    "- Added altstates - F6 to create, F7 to",
    "  delete, press A to switch between",
    "- Added an \"open level folder\" option to",
    "  the \"player levels\" screen",
    "- reloadcustomactivityzones() - reload",
    "  all non-terminal activity zones",
    "- reloadterminalactivityzones() - reload",
    "  all terminal activity zones",
    "- reloadactivityzones() - reload all",
    "  activity zones, regardless of what",
    "  they are",
    "- speak_fast - remove text box fade-in",
    "- speak_active_fast - remove text box",
    "  fade-in, while also removing all other",
    "  text boxes",
    "- textboxtimer(n) - fades out the text",
    "  box after n frames, use this after a",
    "  speak, speak_active, speak_fast, or",
    "  speak_active_fast",
    "- befadeout() - instantly make the",
    "  screen black",
    "- cutscenefast() - instantly appear",
    "  cutscene bars",
    "- endcutscenefast() - instantly remove",
    "  cutscene bars",
    "- setvar(name[,contents]) - set a",
    "  variable to a given argument, or to",
    "  whatever is on the next line",
    "- addvar(name[,value]) - add or subtract",
    "  a number to a given variable or",
    "  concatenate a string to a given",
    "  variable, using either a given",
    "  argument or whatever is on the next",
    "  line",
    "- Variable assignments (var = contents,",
    "  var++, var += 1, var -= 1, var--)",
    "- Built-in variables (%deaths%,",
    "  %player_x%, %player_y%,",
    "  %gravitycontrol%, %room_x%, %room_y%,",
    "  %player_onroof%, %player_onground%,",
    "  %trinkets%, %crewmates%, %coins%,",
    "  %battery_level%, %on_battery%,",
    "  %unix_time%, %hhmmss_time%)",
    "- Upped the map size limit to 100 by 100",
    "- ifcrewmates(n,script) - go to script",
    "  if the player has rescued at least n",
    "  crewmates",
    "- ifcrewmatesless(n,script) - go to",
    "  script if the player has rescued less",
    "  than n crewmates",
    "- ifcoins(n,script) - go to script if",
    "  the player has collected at least n",
    "  coins",
    "- ifcoinsless(n,script) - go to script",
    "  if the player has collected less",
    "  than n coins",
    "- Coins are now placeable in the editor",
    "  using the ^2 tool (press Shift+2)",
    "- Coins display in the roomname if there's",
    "  any coins in the map",
    "- Coins and trinkets no longer share IDs",
    "- Selecting level music from the editor is",
    "  no longer limited",
    "- ifvar(var,operator[,value],script) -",
    "  go to script if a variable equals/",
    "  isn't equal to/is greater than/is lesser",
    "  than/is greater or equal to/is lesser or",
    "  equal to value - if value not given,",
    "  uses the next line",
    "- stop() - stop the script and remove",
    "  cutscene bars",
    "- Flip tokens are now placeable in the",
    "  editor using the ^1 tool (press",
    "  Shift+1)",
    "- You can place activity zones in the",
    "  editor by holding down Z while placing",
    "  a script box",
    "- You can change the speed of enemies",
    "  you're placing down by using Ctrl+",
    "  Period/Comma. Speeds are 0 - 8, but",
    "  since by default p2 is 0, the speeds",
    "  are saved as -4 to 4",
    "- replacetiles(a,b) - replace all",
    "  instances of tile a with tile b in the",
    "  room",
    "- Added being able to use orange as a",
    "  color in text()",
    "- You can now use one unpatterned Space",
    "  Station color in the editor",
    "- Enemies in the unpatterned Space",
    "  Station color are now gray",
    "- Added sub-tile positioning of",
    "  edentities",
    "- coincounter(hide/show)",
    "- Raised the number of flags to 1000,",
    "  now they go from 0-999",
    "- iftrial([id,]script) - go to script if",
    "  if you're in a trial and trial's id is",
    "  id - if id not given, go to script if",
    "  you're in any trial",
    "- endtrial() - ends a time trial - this",
    "  is functionally identical to",
    "  gamestate(82).",
    "- Added sub-tile dimensions of script",
    "  boxes and activity zones",
    "- Added one-time script boxes - hold X",
    "  when placing down a script box to make",
    "  it run only once",
    "- Flip tokens now play the gravity line",
    "  sound effect when touched in VCE",
    "  levels",
    "- Flip tokens now respawn upon death in",
    "  VCE levels",
    "- 1x1 quicksand now respawn upon death",
    "  in VCE levels",
    "- Terminals' activity zones are aligned",
    "  properly in VCE levels",
    "- nointerrupt() - prevent interrupting a",
    "  script when player moves into a script",
    "  box",
    "- yesinterrupt() - undoes the above",
    "- return() - return to the previous",
    "  script and line number if you jumped",
    "  to a script",
    "- load(script) - load script without",
    "  having to type iftrinkets(0,script)",
    "- You can now use Minecraft-like",
    "  relative tilde syntax in gotoroom()",
    "  and gotoposition()",
    "- sayquiet() and replyquiet() - same as",
    "  normal say() and reply(), but without",
    "  a squeak",
    "- Added dimensions, so you can properly",
    "  have more than one dimension in a",
    "  custom level - they are rectangles of",
    "  rooms that the game will keep you in",
    "  and let you wrap around",
    "- gotodimension(n) - go to dimension n",
    "- ifkey(key,script) - if key is pressed,",
    "  go to script - left, right, up, and",
    "  down count controllers and WASD, so to",
    "  get ONLY the arrow keys, use rleft,",
    "  rright, rup, and rdown",
    "- ifflipmode(script) - go to script if",
    "  the game is in flip mode",
    "- delchar(var,n) - remove n chars from",
    "  the end of the variable var",
    "- getvar(var1[,var2]) - set var1 to",
    "  contents of var2, if var2 is not given",
    "  it uses the next line",
    "- analogue(on/off) - toggle Analogue",
    "  Mode screen filter",
    "- To go to script without, prefix it",
    "  with @",
    "- Added callbacks - on_death_start,",
    "  on_death_end, on_input_flip,",
    "  on_input_flip_up, on_input_flip_down",
    "- setcallback(callback,script) - run",
    "  script when callback is triggered",
    "- One-way tiles are now automatically",
    "  recolored to match the tileset",
    "- setblendmode(mode) - set the blend",
    "  mode used for drawing script images,",
    "  mode is blend/none/add/mod, blend is",
    "  default",
    "- csay([lines[,crewmate]]) - say() but",
    "  it puts the dialogue above",
    "  createcrewman crewmates instead",
    "- csayquiet([lines[,crewmate]]) -",
    "  sayquiet() but it puts the dialogue",
    "  above createcrewman crewmates instead",
    "- gamestatedelay(n) - delay running a",
    "  gamestate for n frames",
    "- randchoose(varname,options...) -",
    "  randomly set varname to any of the",
    "  given arguments afterwards, with equal",
    "  probability for each",
    "- randrange(varname[,start],end) -",
    "  randomly set varname to the range",
    "  between start and end excluding end",
    "  start defaults to 0",
    "- moveplayersafe(x,y) - moveplayer",
    "  without going through walls",
    "- supercrewmate(on/off) - toggle whether",
    "  the Intermission 1 crewmate collides",
    "  with hazards or not",
    "- supercrewmateroom() - set the progress",
    "  of the super crewmate to the current",
    "  room",
    "- moveplayer() is instant in VCE levels",
    "- You can place teleporters using the ^3",
    "  tool (press Shift+3)",
    "- Added towers - press F8 to toggle",
    "  tower mode - use plus and minus to go",
    "  up and down",
    "- setentitydata(id,attribute,value) -",
    "  set attribute of entity to value - id",
    "  is slot number in array of entities",
    "  and createentity() will set %return%",
    "  to ID of newly-created entity,",
    "  attribute is internal VVVVVV attribute",
    "  name",
    "- getentitydata(id,attribute,varname) -",
    "  set varname to value of an entity's",
    "  attribute - id is slot number in array",
    "  of entities and createentity() will",
    "  set %return% to ID of newly-created",
    "  entity, attribute is internal VVVVVV",
    "  attribute name",

};

void menurender()
{
    switch (game.currentmenuname) {
    case Menu::mainmenu: {
        graphics.drawsprite((160 - 96) + 0 * 32, 50, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 1 * 32, 50, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 2 * 32, 50, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 3 * 32, 50, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 4 * 32, 50, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 5 * 32, 50, 23, tr, tg, tb);
        //graphics.Print(-1,95,"COMMUNITY EDITION",tr, tg, tb, true);

        std::string cestr = "COMMUNITY EDITION";
        for(std::string::size_type i = 0; i < cestr.size(); ++i) {
            if (((game.gametimer / 2) % 40) == (int)i) {
                graphics.Print(92 + (i * 8), 95, cestr.substr(i, 1), tr/2, tg/2, tb/2);
            } else {
                graphics.Print(92 + (i * 8), 95, cestr.substr(i, 1), tr, tg, tb);
            }
        }

        graphics.Print( 310 - (4*8), 220, "c1.0", tr/2, tg/2, tb/2);
        graphics.Print( 310 - (4*8), 230, "pre1", tr/2, tg/2, tb/2);
        int modoffset = 230;
        graphics.Print( 10, modoffset, "git.io/v6-ce", tr/2, tg/2, tb/2);
        modoffset -= 10;
        if (music.mmmmmm) {
            graphics.Print( 10, modoffset, "[MMMMMM Mod Installed]", tr/2, tg/2, tb/2);
            modoffset -= 10;
        }
        if (graphics.grphx.im_unifont && graphics.grphx.im_wideunifont) {
            graphics.Print( 10, modoffset, "[UniFont Installed]", tr/2, tg/2, tb/2);
            modoffset -= 10;
        }
        break;
    }
    case Menu::loadcustomtrial:
        if (ed.customtrials.size() == 0) {
            graphics.Print( -1, 65, "No time trials...", tr, tg, tb, true);
        } else if (game.currentmenuoption == (int)ed.customtrials.size()) {
            graphics.Print( -1, 65, "Select a time trial to play!", tr, tg, tb, true);
        } else {
            game.timetrialpar = ed.customtrials[game.currentmenuoption].par;
            graphics.bigprint( -1, 30, ed.customtrials[game.currentmenuoption].name, tr, tg, tb, true);
            if ((game.currentmenuoption + 1) > (int)game.customtrialstats.size()) {
                graphics.Print( -1, 75, "Not yet attempted", tr, tg, tb, true);
            } else if (!game.customtrialstats[game.currentmenuoption].attempted) {
                graphics.Print( -1, 75, "Not yet attempted", tr, tg, tb, true);
            } else {
                graphics.Print( 16, 65, "BEST TIME  ", tr, tg, tb);
                graphics.Print( 16, 75, "BEST SHINY ", tr, tg, tb);
                graphics.Print( 16, 85, "BEST LIVES ", tr, tg, tb);
                graphics.Print( 110, 65, game.timetstring(game.customtrialstats[game.currentmenuoption].time), tr, tg, tb);
                graphics.Print( 110, 75, help.String(game.customtrialstats[game.currentmenuoption].trinkets)+"/" + help.String(ed.customtrials[game.currentmenuoption].trinkets), tr, tg, tb);
                graphics.Print( 110, 85, help.String(game.customtrialstats[game.currentmenuoption].lives), tr, tg, tb);
                graphics.Print( 170, 65, "PAR TIME    " + game.partimestring(), tr, tg, tb);
                graphics.Print( 170, 85, "Best Rank", tr, tg, tb);
                switch(game.customtrialstats[game.currentmenuoption].rank)
                {
                case 0:
                    graphics.bigprint( 275, 82, "B", 225, 225, 225);
                    break;
                case 1:
                    graphics.bigprint( 275, 82, "A", 225, 225, 225);
                    break;
                case 2:
                    graphics.bigprint( 275, 82, "S", 225, 225, 225);
                    break;
                case 3:
                    graphics.bigprint( 275, 82, "V", 225, 225, 225);
                    break;
                }
            }
        }
        break;
    case Menu::changelog: {
        graphics.bigprint( -1, 20, "Changelog:", tr, tg, tb, true, 2);


        // Let's clamp the offset--we can't really
        // use std::clamp because it's unsigned
        if (game.changelogoffset + 13 >= changelog.size()) {
            game.changelogoffset = changelog.size() - 14;
        }

        // We can just make more variables instead of ifs
        int temp_tr = tr;
        int temp_tg = tg;
        int temp_tb = tb;

        for(growing_vector<std::string>::size_type i = 0 + game.changelogoffset; i != (14 + game.changelogoffset) && i < changelog.size(); i++) {
            // Huge if, I swear this made sense when I wrote it
            // If there's more lines above the top line displayed, or if
            // there's more lines below the bottom lined displayed, make
            // the color darker.
            if ((i == 0 + game.changelogoffset && (game.changelogoffset != 0)) || (i == 13 + game.changelogoffset && game.changelogoffset != changelog.size() - 14)) {
                temp_tr = tr / 2;
                temp_tg = tg / 2;
                temp_tb = tb / 2;
            } else {
                temp_tr = tr;
                temp_tg = tg;
                temp_tb = tb;
            }
            if (changelog[i] == "[line]") {
                // If the string is literally "[line]", just replace it
                graphics.Print(0, 48 + ((i - game.changelogoffset) * 10), "________________________________________", temp_tr, temp_tg, temp_tb);
            } else {
                graphics.Print(0, 50 + ((i - game.changelogoffset) * 10), changelog[i], temp_tr, temp_tg, temp_tb);
            }
        }
        break;
    }
#if !defined(NO_CUSTOM_LEVELS)
    case Menu::levellist:
    {
      if(ed.ListOfMetaData.size()==0){
      graphics.Print( -1, 100, "ERROR: No levels found.", tr, tg, tb, true);
      }
      int tmp=game.currentmenuoption+(game.levelpage*8);
      if(tmp>=0 && tmp < (int) ed.ListOfMetaData.size()){ // FIXME: size_t/int! -flibit
        //Don't show next page or return to menu options here!
        if(game.menuoptions.size() - game.currentmenuoption<=3){

        }else{
          graphics.bigprint( -1, 15, ed.ListOfMetaData[tmp].title, tr, tg, tb, true);
          graphics.Print( -1, 40, "by " + ed.ListOfMetaData[tmp].creator, tr, tg, tb, true);
          graphics.Print( -1, 50, ed.ListOfMetaData[tmp].website, tr, tg, tb, true);
          graphics.Print( -1, 70, ed.ListOfMetaData[tmp].Desc1, tr, tg, tb, true);
          graphics.Print( -1, 80, ed.ListOfMetaData[tmp].Desc2, tr, tg, tb, true);
          graphics.Print( -1, 90, ed.ListOfMetaData[tmp].Desc3, tr, tg, tb, true);
        }
      }
      break;
    }
#endif
    case Menu::errornostart:
      graphics.Print( -1, 65, "ERROR: This level has", tr, tg, tb, true);
      graphics.Print( -1, 75, "no start point!", tr, tg, tb, true);
      break;
    case Menu::options:

        switch (game.currentmenuoption)
        {
        case 0:
            graphics.bigprint( -1, 30, "Accessibility", tr, tg, tb, true);
            graphics.Print( -1, 65, "Disable screen effects, enable", tr, tg, tb, true);
            graphics.Print( -1, 75, "slowdown modes or invincibility", tr, tg, tb, true);
            break;
        case 1:
#if !defined(MAKEANDPLAY)
            graphics.bigprint( -1, 30, "Unlock Play Modes", tr, tg, tb, true);
            graphics.Print( -1, 65, "Unlock parts of the game normally", tr, tg, tb, true);
            graphics.Print( -1, 75, "unlocked as you progress", tr, tg, tb, true);
#else
            graphics.bigprint( -1, 30, "Flip Mode", tr, tg, tb, true);
            graphics.Print( -1, 65, "Flip the entire game vertically.", tr, tg, tb, true);
            if (graphics.setflipmode)
            {
                graphics.Print( -1, 105, "Currently ENABLED!", tr, tg, tb, true);
            }
            else
            {
                graphics.Print( -1, 105, "Currently Disabled.", tr/2, tg/2, tb/2, true);
            }
            break;
#endif
        case 2:
            graphics.bigprint( -1, 30, "Game Pad Options", tr, tg, tb, true);
            graphics.Print( -1, 65, "Rebind your controller's buttons", tr, tg, tb, true);
            graphics.Print( -1, 75, "and adjust sensitivity", tr, tg, tb, true);
            break;
        case 3:
            graphics.bigprint( -1, 30, "Clear Data", tr, tg, tb, true);
            graphics.Print( -1, 65, "Delete your save data", tr, tg, tb, true);
            graphics.Print( -1, 75, "and unlocked play modes", tr, tg, tb, true);
            break;
        case 4:
            if(music.mmmmmm){
                graphics.bigprint( -1, 30, "Soundtrack", tr, tg, tb, true);
                graphics.Print( -1, 65, "Toggle between MMMMMM and PPPPPP", tr, tg, tb, true);
                if(music.usingmmmmmm){
                    graphics.Print( -1, 85, "Current soundtrack: MMMMMM", tr, tg, tb, true);
                }else{
                    graphics.Print( -1, 85, "Current soundtrack: PPPPPP", tr, tg, tb, true);
                }
            }
            break;
        }
        break;
    case Menu::graphicoptions:
        switch (game.currentmenuoption)
        {
        case 0:
            graphics.bigprint( -1, 30, "Toggle Fullscreen", tr, tg, tb, true);
            graphics.Print( -1, 65, "Change to fullscreen/windowed mode.", tr, tg, tb, true);

            if(game.fullscreen){
              graphics.Print( -1, 85, "Current mode: FULLSCREEN", tr, tg, tb, true);
            }else{
              graphics.Print( -1, 85, "Current mode: WINDOWED", tr, tg, tb, true);
            }
            break;

        case 1:
            graphics.bigprint( -1, 30, "Toggle Letterbox", tr, tg, tb, true);
            graphics.Print( -1, 65, "Choose letterbox/stretch/integer mode.", tr, tg, tb, true);

            if(game.stretchMode == 6){
              graphics.Print( -1, 85, "Current mode: 4X", tr, tg, tb, true);
            }else if (game.stretchMode == 5){
              graphics.Print( -1, 85, "Current mode: 3X", tr, tg, tb, true);
            }else if (game.stretchMode == 4){
              graphics.Print( -1, 85, "Current mode: 2X", tr, tg, tb, true);
            }else if (game.stretchMode == 3){
              graphics.Print( -1, 85, "Current mode: 1X", tr, tg, tb, true);
            }else if (game.stretchMode == 2){
              graphics.Print( -1, 85, "Current mode: INTEGER", tr, tg, tb, true);
            }else if (game.stretchMode == 1){
              graphics.Print( -1, 85, "Current mode: STRETCH", tr, tg, tb, true);
            }else{
              graphics.Print( -1, 85, "Current mode: LETTERBOX", tr, tg, tb, true);
            }
            break;
        case 2:
            graphics.bigprint( -1, 30, "Toggle Filter", tr, tg, tb, true);
            graphics.Print( -1, 65, "Change to nearest/linear filter.", tr, tg, tb, true);

            if(game.useLinearFilter){
              graphics.Print( -1, 85, "Current mode: LINEAR", tr, tg, tb, true);
            }else{
              graphics.Print( -1, 85, "Current mode: NEAREST", tr, tg, tb, true);
            }
            break;

        case 3:
            graphics.bigprint( -1, 30, "Analogue Mode", tr, tg, tb, true);
            graphics.Print( -1, 65, "There is nothing wrong with your", tr, tg, tb, true);
            graphics.Print( -1, 75, "television set. Do not attempt to", tr, tg, tb, true);
            graphics.Print( -1, 85, "adjust the picture.", tr, tg, tb, true);
            break;
        case 4:
            graphics.bigprint(-1, 30, "Toggle Mouse Cursor", tr, tg, tb, true);
            graphics.Print(-1, 65, "Show/hide the system mouse cursor.", tr, tg, tb, true);

            if (graphics.showmousecursor) {
                graphics.Print(-1, 85, "Current mode: SHOW", tr, tg, tb, true);
            }
            else {
                graphics.Print(-1, 85, "Current mode: HIDE", tr/2, tg/2, tb/2, true);
            }
            break;
        }
        break;
    case Menu::credits:
        graphics.Print( -1, 50, "VVVVVV is a game by", tr, tg, tb, true);
        graphics.bigprint( 40, 65, "Terry Cavanagh", tr, tg, tb, true, 2);

        graphics.drawimagecol(7, -1, 86, tr *0.75, tg *0.75, tb *0.75, true);

        graphics.Print( -1, 120, "and features music by", tr, tg, tb, true);
        graphics.bigprint( 40, 135, "Magnus P~lsson", tr, tg, tb, true, 2);
        graphics.drawimagecol(8, -1, 156, tr *0.75, tg *0.75, tb *0.75, true);
        break;
    case Menu::credits_ce: {
        auto base = 30; // x = 240 - 126 - x
        graphics.Print( -1, base, "VVVVVV: Community Edition has", tr, tg, tb, true);
        graphics.Print( -1, base + 15, "accepted contributions from:", tr, tg, tb, true);
        graphics.Print( -1, base + 40,  "Misa", tr, tg, tb, true);
        graphics.Print( -1, base + 55,  "AllyTally",  tr, tg, tb, true);
        graphics.Print( -1, base + 70,  "leo60228",   tr, tg, tb, true);
        graphics.Print( -1, base + 85,  "FIQ",        tr, tg, tb, true);
        graphics.Print( -1, base + 100, "Fußmatte",    tr, tg, tb, true);
        graphics.Print( -1, base + 115, "mothbeanie", tr, tg, tb, true);
        graphics.Print( -1, base + 130, "Allison Fleischer", tr, tg, tb, true);
        graphics.Print( -1, base + 145, "Dav999", tr, tg, tb, true);
        graphics.Print( -1, base + 160, "Joshua", tr, tg, tb, true);
        break;
    }
    case Menu::credits2:
        graphics.Print( -1, 50, "Roomnames are by", tr, tg, tb, true);
        graphics.bigprint( 40, 65, "Bennett Foddy", tr, tg, tb, true);
        graphics.drawimagecol(9, -1, 86, tr*0.75, tg *0.75, tb *0.75, true);
        graphics.Print( -1, 110, "C++ version by", tr, tg, tb, true);
        graphics.bigprint( 40, 125, "Simon Roth", tr, tg, tb, true);
        graphics.bigprint( 40, 145, "Ethan Lee", tr, tg, tb, true);
        break;
    case Menu::credits25:
        graphics.Print( -1, 40, "Beta Testing by", tr, tg, tb, true);
        graphics.bigprint( 40, 55, "Sam Kaplan", tr, tg, tb, true);
        graphics.bigprint( 40, 75, "Pauli Kohberger", tr, tg, tb, true);
        graphics.Print( -1, 130, "Ending Picture by", tr, tg, tb, true);
        graphics.bigprint( 40, 145, "Pauli Kohberger", tr, tg, tb, true);
        break;
    case Menu::credits3:
    {
        graphics.Print( -1, 20, "VVVVVV is supported by", tr, tg, tb, true);
        graphics.Print( 40, 30, "the following patrons", tr, tg, tb, true);

        int startidx = game.current_credits_list_index;
        int endidx = std::min(startidx + 9, (int)game.superpatrons.size());

        int xofs = 80 - 16;
        int yofs = 40 + 20;

        for (int i = startidx; i < endidx; ++i)
        {
            graphics.Print(xofs, yofs, game.superpatrons[i], tr, tg, tb);
            xofs += 4;
            yofs += 14;
        }
        break;
    }
    case Menu::credits4:
    {
        graphics.Print( -1, 20, "and also by", tr, tg, tb, true);

        int startidx = game.current_credits_list_index;
        int endidx = std::min(startidx + 14, (int)game.patrons.size());

        int maxheight = 10 * 14;
        int totalheight = (endidx - startidx) * 10;
        int emptyspace = maxheight - totalheight;

        int yofs = 40 + (emptyspace / 2);

        for (int i = startidx; i < endidx; ++i)
        {
            graphics.Print(80, yofs, game.patrons[i], tr, tg, tb);
            yofs += 10;
        }
        break;
    }
    case Menu::credits5:
    {
        graphics.Print( -1, 20, "With contributions on", tr, tg, tb, true);
        graphics.Print( 40, 30, "GitHub from", tr, tg, tb, true);

        int startidx = game.current_credits_list_index;
        int endidx = std::min(startidx + 9, (int)game.githubfriends.size());

        int maxheight = 14 * 9;
        int totalheight = (endidx - startidx) * 14;
        int emptyspace = maxheight - totalheight;

        int xofs = 80 - 16;
        int yofs = 40 + 20 + (emptyspace / 2);

        for (int i = startidx; i < endidx; ++i)
        {
            graphics.Print(xofs, yofs, game.githubfriends[i], tr, tg, tb);
            xofs += 4;
            yofs += 14;
        }
        break;
    }
    case Menu::credits6:
        graphics.Print( -1, 20, "and thanks also to:", tr, tg, tb, true);

        graphics.bigprint(80, 60, "You!", tr, tg, tb, true);

        graphics.Print( 80, 100, "Your support makes it possible", tr, tg, tb,true);
        graphics.Print( 80, 110,"for me to continue making the", tr, tg, tb,true);
        graphics.Print( 80, 120,"games I want to make, now", tr, tg, tb,true);
        graphics.Print( 80, 130, "and into the future.", tr, tg, tb, true);

        graphics.Print( 80, 150,"Thank you!", tr, tg, tb,true);
        break;
    case Menu::setinvincibility:
        graphics.Print( -1, 100, "Are you sure you want to ", tr, tg, tb, true);
        graphics.Print( -1, 110, "enable invincibility?", tr, tg, tb, true);
        break;
    case Menu::setslowdown:
        graphics.bigprint( -1, 40, "Game Speed", tr, tg, tb, true);
        graphics.Print( -1, 75, "Select a new game speed below.", tr, tg, tb, true);
        switch (game.gameframerate)
        {
        case 34:
            graphics.Print( -1, 105, "Game speed is normal.", tr/2, tg/2, tb/2, true);
            break;
        case 41:
            graphics.Print( -1, 105, "Game speed is at 80%", tr, tg, tb, true);
            break;
        case 55:
            graphics.Print( -1, 105, "Game speed is at 60%", tr, tg, tb, true);
            break;
        case 83:
            graphics.Print( -1, 105, "Game speed is at 40%", tr, tg, tb, true);
            break;
        }
        break;
    case Menu::newgamewarning:
        graphics.Print( -1, 100, "Are you sure? This will", tr, tg, tb, true);
        graphics.Print( -1, 110, "delete your current saves...", tr, tg, tb, true);
        break;
    case Menu::cleardatamenu:
        graphics.Print( -1, 100, "Are you sure you want to", tr, tg, tb, true);
        graphics.Print( -1, 110, "delete all your saved data?", tr, tg, tb, true);
        break;
    case Menu::startnodeathmode:
        graphics.Print( -1, 45, "Good luck!", tr, tg, tb, true);
        graphics.Print( -1, 80, "You cannot save in this mode.", tr, tg, tb, true);
        graphics.Print( -1, 100, "Would you like to disable the", tr, tg, tb, true);
        graphics.Print( -1, 112, "cutscenes during the game?", tr, tg, tb, true);
        break;
    case Menu::controller:
        graphics.bigprint( -1, 30, "Game Pad", tr, tg, tb, true);
        graphics.Print( -1, 55, "Change controller options.", tr, tg, tb, true);
        switch (game.currentmenuoption)
        {
        case 0:
            switch(game.controllerSensitivity)
            {
            case 0:
                graphics.Print( -1, 85, " Low     Medium     High", tr, tg, tb, true);
                graphics.Print( -1, 95, "[]..................", tr, tg, tb, true);
                break;
            case 1:
                graphics.Print( -1, 85, " Low     Medium     High", tr, tg, tb, true);
                graphics.Print( -1, 95, ".....[].............", tr, tg, tb, true);
                break;
            case 2:
                graphics.Print( -1, 85, " Low     Medium     High", tr, tg, tb, true);
                graphics.Print( -1, 95, ".........[].........", tr, tg, tb, true);
                break;
            case 3:
                graphics.Print( -1, 85, " Low     Medium     High", tr, tg, tb, true);
                graphics.Print( -1, 95, ".............[].....", tr, tg, tb, true);
                break;
            case 4:
                graphics.Print( -1, 85, " Low     Medium     High", tr, tg, tb, true);
                graphics.Print( -1, 95, "..................[]", tr, tg, tb, true);
                break;
            }
            break;
        case 1:
        case 2:
        case 3:
            graphics.Print( -1, 85, "Flip is bound to: " + std::string(help.GCString(game.controllerButton_flip)) , tr, tg, tb, true);
            graphics.Print( -1, 95, "Enter is bound to: "  + std::string(help.GCString(game.controllerButton_map)), tr, tg, tb, true);
            graphics.Print( -1, 105, "Menu is bound to: " + std::string(help.GCString(game.controllerButton_esc)) , tr, tg, tb, true);
            break;
        }


        break;
    case Menu::accessibility:
        switch (game.currentmenuoption)
        {
        case 0:
            graphics.bigprint( -1, 40, "Backgrounds", tr, tg, tb, true);
            if (!game.colourblindmode)
            {
                graphics.Print( -1, 75, "Backgrounds are ON.", tr, tg, tb, true);
            }
            else
            {
                graphics.Print( -1, 75, "Backgrounds are OFF.", tr/2, tg/2, tb/2, true);
            }
            break;
        case 1:
            graphics.bigprint( -1, 40, "Screen Effects", tr, tg, tb, true);
            graphics.Print( -1, 75, "Disables screen shakes and flashes.", tr, tg, tb, true);
            if (!game.noflashingmode)
            {
                graphics.Print( -1, 85, "Screen Effects are ON.", tr, tg, tb, true);
            }
            else
            {
                graphics.Print( -1, 85, "Screen Effects are OFF.", tr/2, tg/2, tb/2, true);
            }
            break;
        case 2:
            graphics.bigprint( -1, 40, "Text Outline", tr, tg, tb, true);
            graphics.Print( -1, 75, "Disables outline on game text", tr, tg, tb, true);
            // FIXME: Maybe do an outlined print instead? -flibit
            if (!graphics.notextoutline)
            {
                graphics.Print( -1, 85, "Text outlines are ON.", tr, tg, tb, true);
            }
            else
            {
                graphics.Print( -1, 85, "Text outlines are OFF.", tr/2, tg/2, tb/2, true);
            }
            break;
        case 3:
            graphics.bigprint( -1, 40, "Invincibility", tr, tg, tb, true);
            graphics.Print( -1, 75, "Provided to help disabled gamers", tr, tg, tb, true);
            graphics.Print( -1, 85, "explore the game. Can cause glitches.", tr, tg, tb, true);
            if (map.invincibility)
            {
                graphics.Print( -1, 105, "Invincibility is ON.", tr, tg, tb, true);
            }
            else
            {
                graphics.Print( -1, 105, "Invincibility is off.", tr/2, tg/2, tb/2, true);
            }
            break;
        case 4:
            graphics.bigprint( -1, 40, "Game Speed", tr, tg, tb, true);
            graphics.Print( -1, 75, "May be useful for disabled gamers", tr, tg, tb, true);
            graphics.Print( -1, 85, "using one switch devices.", tr, tg, tb, true);
            if (game.gameframerate==34)
            {
                graphics.Print( -1, 105, "Game speed is normal.", tr/2, tg/2, tb/2, true);
            }
            else if (game.gameframerate==41)
            {
                graphics.Print( -1, 105, "Game speed is at 80%", tr, tg, tb, true);
            }
            else if (game.gameframerate==55)
            {
                graphics.Print( -1, 105, "Game speed is at 60%", tr, tg, tb, true);
            }
            else if (game.gameframerate==83)
            {
                graphics.Print( -1, 105, "Game speed is at 40%", tr, tg, tb, true);
            }
            break;
        case 5:
            graphics.bigprint( -1, 40, "Music", tr, tg, tb, true);
            graphics.Print( -1, 75, "Disables music.", tr, tg, tb, true);
            if (!game.musicmuted)
            {
                graphics.Print( -1, 85, "Music is ON.", tr, tg, tb, true);
            }
            else
            {
                graphics.Print( -1, 85, "Music is OFF.", tr/2, tg/2, tb/2, true);
            }
            break;
        case 6:
            graphics.bigprint(-1, 30, "Room Name BG", tr, tg, tb, true);
            graphics.Print( -1, 75, "Lets you see through what is behind", tr, tg, tb, true);
            graphics.Print( -1, 85, "the name at the bottom of the screen.", tr, tg, tb, true);
            if (graphics.translucentroomname)
                graphics.Print(-1, 105, "Room name background is TRANSLUCENT", tr/2, tg/2, tb/2, true);
            else
                graphics.Print(-1, 105, "Room name background is OPAQUE", tr, tg, tb, true);
            break;
        }
        break;
    case Menu::playint1:
    case Menu::playint2:
        graphics.Print( -1, 65, "Who do you want to play", tr, tg, tb, true);
        graphics.Print( -1, 75, "the level with?", tr, tg, tb, true);
        break;
    case Menu::playmodes:
        switch (game.currentmenuoption)
        {
        case 0:
            graphics.bigprint( -1, 30, "Time Trials", tr, tg, tb, true);
            graphics.Print( -1, 65, "Replay any level in the game in", tr, tg, tb, true);
            graphics.Print( -1, 75, "a competitive time trial mode.", tr, tg, tb, true);

            if (game.gameframerate > 34 || map.invincibility)
            {
                graphics.Print( -1, 105, "Time Trials are not available", tr, tg, tb, true);
                graphics.Print( -1, 115, "with slowdown or invincibility.", tr, tg, tb, true);
            }
            break;
        case 1:
            graphics.bigprint( -1, 30, "Intermissions", tr, tg, tb, true);
            graphics.Print( -1, 65, "Replay the intermission levels.", tr, tg, tb, true);

            if (!game.unlock[15] && !game.unlock[16])
            {
                graphics.Print( -1, 95, "TO UNLOCK: Complete the", tr, tg, tb, true);
                graphics.Print( -1, 105, "intermission levels in-game.", tr, tg, tb, true);
            }
            break;
        case 2:
            graphics.bigprint( -1, 30, "No Death Mode", tr, tg, tb, true);
            graphics.Print( -1, 65, "Play the entire game", tr, tg, tb, true);
            graphics.Print( -1, 75, "without dying once.", tr, tg, tb, true);

            if (game.gameframerate > 34 || map.invincibility)
            {
                graphics.Print( -1, 105, "No Death Mode is not available", tr, tg, tb, true);
                graphics.Print( -1, 115, "with slowdown or invincibility.", tr, tg, tb, true);
            }
            else if (!game.unlock[17])
            {
                graphics.Print( -1, 105, "TO UNLOCK: Achieve an S-rank or", tr, tg, tb, true);
                graphics.Print( -1, 115, "above in at least 4 time trials.", tr, tg, tb, true);
            }
            break;
        case 3:
            graphics.bigprint( -1, 30, "Flip Mode", tr, tg, tb, true);
            graphics.Print( -1, 65, "Flip the entire game vertically.", tr, tg, tb, true);
            graphics.Print( -1, 75, "Compatible with other game modes.", tr, tg, tb, true);

            if (game.unlock[18])
            {
                if (graphics.setflipmode)
                {
                    graphics.Print( -1, 105, "Currently ENABLED!", tr, tg, tb, true);
                }
                else
                {
                    graphics.Print( -1, 105, "Currently Disabled.", tr/2, tg/2, tb/2, true);
                }
            }
            else
            {
                graphics.Print( -1, 105, "TO UNLOCK: Complete the game.", tr, tg, tb, true);
            }
            break;
        }
        break;
    case Menu::youwannaquit:
        graphics.Print( -1, 75, "Are you sure you want to quit?", tr, tg, tb, true);
        break;
    case Menu::continuemenu:
        graphics.crewframedelay--;
        if (graphics.crewframedelay <= 0)
        {
            graphics.crewframedelay = 8;
            graphics.crewframe = (graphics.crewframe + 1) % 2;
        }
        switch (game.currentmenuoption)
        {
        case 0:
            //Show teleporter save info
            graphics.drawpixeltextbox(25, 65-20, 270, 90, 34,12, 65, 185, 207,0,4);

            graphics.bigprint(-1, 20, "Tele Save", tr, tg, tb, true);
            graphics.Print(0, 80-20, game.tele_currentarea, 25, 255 - (help.glow / 2), 255 - (help.glow / 2), true);
            for (int i = 0; i < 6; i++)
            {
                graphics.drawcrewman(169-(3*42)+(i*42), 95-20, i, game.tele_crewstats[i], true);
            }
            graphics.Print(160 - 84, 132-20, game.tele_gametime, 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));
            graphics.Print(160 + 40, 132-20, help.number(game.tele_trinkets), 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));

            graphics.drawspritesetcol(50, 126-20, 50, 18);
            graphics.drawspritesetcol(175, 126-20, 22, 18);
            break;
        case 1:
            //Show quick save info
            graphics.drawpixeltextbox(25, 65-20, 270, 90, 34,12, 65, 185, 207,0,4);

            graphics.bigprint(-1, 20, "Quick Save", tr, tg, tb, true);
            graphics.Print(0, 80-20, game.quick_currentarea, 25, 255 - (help.glow / 2), 255 - (help.glow / 2), true);
            for (int i = 0; i < 6; i++)
            {
                graphics.drawcrewman(169-(3*42)+(i*42), 95-20, i, game.quick_crewstats[i], true);
            }
            graphics.Print(160 - 84, 132-20, game.quick_gametime, 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));
            graphics.Print(160 + 40, 132-20, help.number(game.quick_trinkets), 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));

            graphics.drawspritesetcol(50, 126-20, 50, 18);
            graphics.drawspritesetcol(175, 126-20, 22, 18);
            break;
        }
        break;
    case Menu::gameover:
    case Menu::gameover2:
    {
        graphics.bigprint( -1, 25, "GAME OVER", tr, tg, tb, true, 3);

        graphics.crewframedelay--;
        if (graphics.crewframedelay <= 0)
        {
            graphics.crewframedelay = 8;
            graphics.crewframe = (graphics.crewframe + 1) % 2;
        }
        for (int i = 0; i < 6; i++)
        {
            graphics.drawcrewman(169-(3*42)+(i*42), 68, i, game.crewstats[i], true);
        }
        std::string tempstring;
        tempstring = "You rescued " + help.number(game.crewrescued()) + " crewmates";
        graphics.Print(0, 100, tempstring, tr, tg, tb, true);

        tempstring = "and found " + help.number(game.trinkets()) + " trinkets.";
        graphics.Print(0, 110, tempstring, tr, tg, tb, true);

        tempstring = "You managed to reach:";
        graphics.Print(0, 145, tempstring, tr, tg, tb, true);
        graphics.Print(0, 155, game.hardestroom, tr, tg, tb, true);

        switch (game.crewrescued())
        {
        case 1:
            tempstring = "Keep trying! You'll get there!";
            break;
        case 2:
            tempstring = "Nice one!";
            break;
        case 3:
            tempstring = "Wow! Congratulations!";
            break;
        case 4:
            tempstring = "Incredible!";
            break;
        case 5:
            tempstring = "Unbelievable! Well done!";
            break;
        case 6:
            tempstring = "Er, how did you do that?";
            break;
        }

        graphics.Print(0, 190, tempstring, tr, tg, tb, true);
        break;
    }
    case Menu::nodeathmodecomplete:
    case Menu::nodeathmodecomplete2:
    {
        graphics.bigprint( -1, 8, "WOW", tr, tg, tb, true, 4);

        graphics.crewframedelay--;
        if (graphics.crewframedelay <= 0)
        {
            graphics.crewframedelay = 8;
            graphics.crewframe = (graphics.crewframe + 1) % 2;
        }
        for (int i = 0; i < 6; i++)
        {
            graphics.drawcrewman(169-(3*42)+(i*42), 68, i, game.crewstats[i], true);
        }
        std::string tempstring = "You rescued all the crewmates!";
        graphics.Print(0, 100, tempstring, tr, tg, tb, true);

        tempstring = "And you found " + help.number(game.trinkets()) + " trinkets.";
        graphics.Print(0, 110, tempstring, tr, tg, tb, true);

        graphics.Print(0, 160, "A new trophy has been awarded and", tr, tg, tb, true);
        graphics.Print(0, 170, "placed in the secret lab to", tr, tg, tb, true);
        graphics.Print(0, 180, "acknowledge your achievement!", tr, tg, tb, true);
        break;
    }
    case Menu::timetrialcomplete:
    case Menu::timetrialcomplete2:
    case Menu::timetrialcomplete3:
    {
        graphics.bigprint( -1, 20, "Results", tr, tg, tb, true, 3);

        std::string tempstring = game.resulttimestring() + " / " + game.partimestring();

        graphics.drawspritesetcol(30, 80-15, 50, 22);
        graphics.Print(65, 80-15, "TIME TAKEN:", 255, 255, 255);
        graphics.Print(65, 90-15, tempstring, tr, tg, tb);
        if (game.timetrialresulttime <= game.timetrialpar)
        {
            graphics.Print(220, 85-15, "+1 Rank!", 255, 255, 255);
        }

        tempstring = help.String(game.deathcounts);
        graphics.drawspritesetcol(30-4, 80+20-4, 12, 22);
        graphics.Print(65, 80+20, "NUMBER OF DEATHS:", 255, 255, 255);
        graphics.Print(65, 90+20, tempstring, tr, tg, tb);
        if (game.deathcounts == 0)
        {
            graphics.Print(220, 85+20, "+1 Rank!", 255, 255, 255);
        }

        tempstring = help.String(game.trinkets()) + " of " + help.String(game.timetrialshinytarget);
        graphics.drawspritesetcol(30, 80+55, 22, 22);
        graphics.Print(65, 80+55, "SHINY TRINKETS:", 255, 255, 255);
        graphics.Print(65, 90+55, tempstring, tr, tg, tb);
        if (game.trinkets() >= game.timetrialshinytarget)
        {
            graphics.Print(220, 85+55, "+1 Rank!", 255, 255, 255);
        }

        if (game.currentmenuname == Menu::timetrialcomplete2 || game.currentmenuname == Menu::timetrialcomplete3)
        {
            graphics.bigprint( 100, 175, "Rank:", tr, tg, tb, false, 2);
        }

        if (game.currentmenuname == Menu::timetrialcomplete3)
        {
            switch(game.timetrialrank)
            {
            case 0:
                graphics.bigprint( 195, 165, "B", 255, 255, 255, false, 4);
                break;
            case 1:
                graphics.bigprint( 195, 165, "A", 255, 255, 255, false, 4);
                break;
            case 2:
                graphics.bigprint( 195, 165, "S", 255, 255, 255, false, 4);
                break;
            case 3:
                graphics.bigprint( 195, 165, "V", 255, 255, 255, false, 4);
                break;
            }
        }
        break;
    }
    case Menu::unlockmenutrials:
        graphics.bigprint( -1, 30, "Unlock Time Trials", tr, tg, tb, true);
        graphics.Print( -1, 65, "You can unlock each time", tr, tg, tb, true);
        graphics.Print( -1, 75, "trial separately.", tr, tg, tb, true);
        break;
    case Menu::timetrials:
        switch (game.currentmenuoption)
        {
        case 0:
            if(game.unlock[9])
            {
                graphics.bigprint( -1, 30, "Space Station 1", tr, tg, tb, true);
                if (game.besttimes[0] == -1)
                {
                    graphics.Print( -1, 75, "Not yet attempted", tr, tg, tb, true);
                }
                else
                {
                    graphics.Print( 16, 65, "BEST TIME  ", tr, tg, tb);
                    graphics.Print( 16, 75, "BEST SHINY ", tr, tg, tb);
                    graphics.Print( 16, 85, "BEST LIVES ", tr, tg, tb);
                    graphics.Print( 110, 65, game.timetstring(game.besttimes[0]), tr, tg, tb);
                    graphics.Print( 110, 75, help.String(game.besttrinkets[0])+"/2", tr, tg, tb);
                    graphics.Print( 110, 85,help.String(game.bestlives[0]), tr, tg, tb);


                    graphics.Print( 170, 65, "PAR TIME    1:15", tr, tg, tb);
                    graphics.Print( 170, 85, "Best Rank", tr, tg, tb);
                    switch(game.bestrank[0])
                    {
                    case 0:
                        graphics.bigprint( 275, 82, "B", 225, 225, 225);
                        break;
                    case 1:
                        graphics.bigprint( 275, 82, "A", 225, 225, 225);
                        break;
                    case 2:
                        graphics.bigprint( 275, 82, "S", 225, 225, 225);
                        break;
                    case 3:
                        graphics.bigprint( 275, 82, "V", 225, 225, 225);
                        break;
                    }
                }

            }
            else
            {
                graphics.bigprint( -1, 30, "???", tr, tg, tb, true);
                graphics.Print( -1, 60, "TO UNLOCK:", tr, tg, tb, true);
                graphics.Print( -1, 75, "Rescue Violet", tr, tg, tb, true);
                graphics.Print( -1, 85, "Find three trinkets", tr, tg, tb, true);
            }
            break;
        case 1:
            if(game.unlock[10])
            {
                graphics.bigprint( -1, 30, "The Laboratory", tr, tg, tb, true);
                if (game.besttimes[1] == -1)
                {
                    graphics.Print( -1, 75, "Not yet attempted", tr, tg, tb, true);
                }
                else
                {
                    graphics.Print( 16, 65, "BEST TIME  ", tr, tg, tb);
                    graphics.Print( 16, 75, "BEST SHINY ", tr, tg, tb);
                    graphics.Print( 16, 85, "BEST LIVES ", tr, tg, tb);
                    graphics.Print( 110, 65, game.timetstring(game.besttimes[1]), tr, tg, tb);
                    graphics.Print( 110, 75, help.String(game.besttrinkets[1])+"/4", tr, tg, tb);
                    graphics.Print( 110, 85, help.String(game.bestlives[1]), tr, tg, tb);


                    graphics.Print( 170, 65, "PAR TIME    2:45", tr, tg, tb);
                    graphics.Print( 170, 85, "Best Rank", tr, tg, tb);
                    switch(game.bestrank[1])
                    {
                    case 0:
                        graphics.bigprint( 275, 82, "B", 225, 225, 225);
                        break;
                    case 1:
                        graphics.bigprint( 275, 82, "A", 225, 225, 225);
                        break;
                    case 2:
                        graphics.bigprint( 275, 82, "S", 225, 225, 225);
                        break;
                    case 3:
                        graphics.bigprint( 275, 82, "V", 225, 225, 225);
                        break;
                    }
                }

            }
            else
            {
                graphics.bigprint( -1, 30, "???", tr, tg, tb, true);
                graphics.Print( -1, 60, "TO UNLOCK:", tr, tg, tb, true);
                graphics.Print( -1, 75, "Rescue Victoria", tr, tg, tb, true);
                graphics.Print( -1, 85, "Find six trinkets", tr, tg, tb, true);
            }
            break;
        case 2:
            if(game.unlock[11])
            {
                graphics.bigprint( -1, 30, "The Tower", tr, tg, tb, true);
                if (game.besttimes[2] == -1)
                {
                    graphics.Print( -1, 75, "Not yet attempted", tr, tg, tb, true);
                }
                else
                {
                    graphics.Print( 16, 65, "BEST TIME  ", tr, tg, tb);
                    graphics.Print( 16, 75, "BEST SHINY ", tr, tg, tb);
                    graphics.Print( 16, 85, "BEST LIVES ", tr, tg, tb);
                    graphics.Print( 110, 65, game.timetstring(game.besttimes[2]), tr, tg, tb);
                    graphics.Print( 110, 75, help.String(game.besttrinkets[2])+"/2", tr, tg, tb);
                    graphics.Print( 110, 85, help.String(game.bestlives[2]), tr, tg, tb);


                    graphics.Print( 170, 65, "PAR TIME    1:45", tr, tg, tb);
                    graphics.Print( 170, 85, "Best Rank", tr, tg, tb);
                    switch(game.bestrank[2])
                    {
                    case 0:
                        graphics.bigprint( 275, 82, "B", 225, 225, 225);
                        break;
                    case 1:
                        graphics.bigprint( 275, 82, "A", 225, 225, 225);
                        break;
                    case 2:
                        graphics.bigprint( 275, 82, "S", 225, 225, 225);
                        break;
                    case 3:
                        graphics.bigprint( 275, 82, "V", 225, 225, 225);
                        break;
                    }
                }

            }
            else
            {
                graphics.bigprint( -1, 30, "???", tr, tg, tb, true);
                graphics.Print( -1, 60, "TO UNLOCK:", tr, tg, tb, true);
                graphics.Print( -1, 75, "Rescue Vermilion", tr, tg, tb, true);
                graphics.Print( -1, 85, "Find nine trinkets", tr, tg, tb, true);
            }
            break;
        case 3:
            if(game.unlock[12])
            {
                graphics.bigprint( -1, 30, "Space Station 2", tr, tg, tb, true);
                if (game.besttimes[3] == -1)
                {
                    graphics.Print( -1, 75, "Not yet attempted", tr, tg, tb, true);
                }
                else
                {
                    graphics.Print( 16, 65, "BEST TIME  ", tr, tg, tb);
                    graphics.Print( 16, 75, "BEST SHINY ", tr, tg, tb);
                    graphics.Print( 16, 85, "BEST LIVES ", tr, tg, tb);
                    graphics.Print( 110, 65, game.timetstring(game.besttimes[3]), tr, tg, tb);
                    graphics.Print( 110, 75, help.String(game.besttrinkets[3])+"/5", tr, tg, tb);
                    graphics.Print( 110, 85, help.String(game.bestlives[3]), tr, tg, tb);


                    graphics.Print( 170, 65, "PAR TIME    3:20", tr, tg, tb);
                    graphics.Print( 170, 85, "Best Rank", tr, tg, tb);
                    switch(game.bestrank[3])
                    {
                    case 0:
                        graphics.bigprint( 275, 82, "B", 225, 225, 225);
                        break;
                    case 1:
                        graphics.bigprint( 275, 82, "A", 225, 225, 225);
                        break;
                    case 2:
                        graphics.bigprint( 275, 82, "S", 225, 225, 225);
                        break;
                    case 3:
                        graphics.bigprint( 275, 82, "V", 225, 225, 225);
                        break;
                    }
                }

            }
            else
            {
                graphics.bigprint( -1, 30, "???", tr, tg, tb, true);
                graphics.Print( -1, 60, "TO UNLOCK:", tr, tg, tb, true);
                graphics.Print( -1, 75, "Rescue Vitellary", tr, tg, tb, true);
                graphics.Print( -1, 85, "Find twelve trinkets", tr, tg, tb, true);
            }
            break;
        case 4:
            if(game.unlock[13])
            {
                graphics.bigprint( -1, 30, "The Warp Zone", tr, tg, tb, true);
                if (game.besttimes[4] == -1)
                {
                    graphics.Print( -1, 75, "Not yet attempted", tr, tg, tb, true);
                }
                else
                {
                    graphics.Print( 16, 65, "BEST TIME  ", tr, tg, tb);
                    graphics.Print( 16, 75, "BEST SHINY ", tr, tg, tb);
                    graphics.Print( 16, 85, "BEST LIVES ", tr, tg, tb);
                    graphics.Print( 110, 65, game.timetstring(game.besttimes[4]), tr, tg, tb);
                    graphics.Print( 110, 75, help.String(game.besttrinkets[4])+"/1", tr, tg, tb);
                    graphics.Print( 110, 85, help.String(game.bestlives[4]), tr, tg, tb);


                    graphics.Print( 170, 65, "PAR TIME    2:00", tr, tg, tb);
                    graphics.Print( 170, 85, "Best Rank", tr, tg, tb);
                    switch(game.bestrank[4])
                    {
                    case 0:
                        graphics.bigprint( 275, 82, "B", 225, 225, 225);
                        break;
                    case 1:
                        graphics.bigprint( 275, 82, "A", 225, 225, 225);
                        break;
                    case 2:
                        graphics.bigprint( 275, 82, "S", 225, 225, 225);
                        break;
                    case 3:
                        graphics.bigprint( 275, 82, "V", 225, 225, 225);
                        break;
                    }
                }

            }
            else
            {
                graphics.bigprint( -1, 30, "???", tr, tg, tb, true);
                graphics.Print( -1, 60, "TO UNLOCK:", tr, tg, tb, true);
                graphics.Print( -1, 75, "Rescue Verdigris", tr, tg, tb, true);
                graphics.Print( -1, 85, "Find fifteen trinkets", tr, tg, tb, true);
            }
            break;
        case 5:
            if(game.unlock[14])
            {
                graphics.bigprint( -1, 30, "The Final Level", tr, tg, tb, true);
                if (game.besttimes[5] == -1)
                {
                    graphics.Print( -1, 75, "Not yet attempted", tr, tg, tb, true);
                }
                else
                {
                    graphics.Print( 16, 65, "BEST TIME  ", tr, tg, tb);
                    graphics.Print( 16, 75, "BEST SHINY ", tr, tg, tb);
                    graphics.Print( 16, 85, "BEST LIVES ", tr, tg, tb);
                    graphics.Print( 110, 65, game.timetstring(game.besttimes[5]), tr, tg, tb);
                    graphics.Print( 110, 75, help.String(game.besttrinkets[5])+"/1", tr, tg, tb);
                    graphics.Print( 110, 85, help.String(game.bestlives[5]), tr, tg, tb);


                    graphics.Print( 170, 65, "PAR TIME    2:15", tr, tg, tb);
                    graphics.Print( 170, 85, "Best Rank", tr, tg, tb);
                    switch(game.bestrank[5])
                    {
                    case 0:
                        graphics.bigprint( 275, 82, "B", 225, 225, 225);
                        break;
                    case 1:
                        graphics.bigprint( 275, 82, "A", 225, 225, 225);
                        break;
                    case 2:
                        graphics.bigprint( 275, 82, "S", 225, 225, 225);
                        break;
                    case 3:
                        graphics.bigprint( 275, 82, "V", 225, 225, 225);
                        break;
                    }
                }

            }
            else
            {
                graphics.bigprint( -1, 30, "???", tr, tg, tb, true);
                graphics.Print( -1, 60, "TO UNLOCK:", tr, tg, tb, true);
                graphics.Print( -1, 75, "Complete the game", tr, tg, tb, true);
                graphics.Print( -1, 85, "Find eighteen trinkets", tr, tg, tb, true);
            }
            break;
        }
        break;
    case Menu::gamecompletecontinue:
        graphics.bigprint( -1, 25, "Congratulations!", tr, tg, tb, true, 2);

        graphics.Print( -1, 45, "Your save files have been updated.", tr, tg, tb, true);

        graphics.Print( -1, 110, "If you want to keep exploring", tr, tg, tb, true);
        graphics.Print( -1, 120, "the game, select CONTINUE", tr, tg, tb, true);
        graphics.Print( -1, 130, "from the play menu.", tr, tg, tb, true);
        break;
    case Menu::unlockmenu:
        graphics.bigprint( -1, 25, "Unlock Play Modes", tr, tg, tb, true, 2);

        graphics.Print( -1, 55, "From here, you may unlock parts", tr, tg, tb, true);
        graphics.Print( -1, 65, "of the game that are normally", tr, tg, tb, true);
        graphics.Print( -1, 75, "unlocked as you play.", tr, tg, tb, true);
        break;
    case Menu::unlocktimetrial:
        graphics.bigprint( -1, 45, "Congratulations!", tr, tg, tb, true, 2);

        graphics.Print( -1, 125, "You have unlocked", tr, tg, tb, true);
        graphics.Print( -1, 135, "a new Time Trial.", tr, tg, tb, true);
        break;
    case Menu::unlocktimetrials:
        graphics.bigprint( -1, 45, "Congratulations!", tr, tg, tb, true, 2);

        graphics.Print( -1, 125, "You have unlocked some", tr, tg, tb, true);
        graphics.Print( -1, 135, "new Time Trials.", tr, tg, tb, true);
        break;
    case Menu::unlocknodeathmode:
        graphics.bigprint( -1, 45, "Congratulations!", tr, tg, tb, true, 2);

        graphics.Print( -1, 125, "You have unlocked", tr, tg, tb, true);
        graphics.Print( -1, 135, "No Death Mode.", tr, tg, tb, true);
        break;
    case Menu::unlockflipmode:
        graphics.bigprint( -1, 45, "Congratulations!", tr, tg, tb, true, 2);

        graphics.Print( -1, 125, "You have unlocked", tr, tg, tb, true);
        graphics.Print( -1, 135, "Flip Mode.", tr, tg, tb, true);
        break;
    case Menu::unlockintermission:
        graphics.bigprint( -1, 45, "Congratulations!", tr, tg, tb, true, 2);

        graphics.Print( -1, 125, "You have unlocked", tr, tg, tb, true);
        graphics.Print( -1, 135, "the intermission levels.", tr, tg, tb, true);
        break;
    case Menu::playerworlds:
    {
        std::string tempstring = FILESYSTEM_getUserLevelDirectory();
        if(tempstring.length()>80){
            graphics.Print( -1, 160, "To install new player levels, copy", tr, tg, tb, true);
            graphics.Print( -1, 170, "the .vvvvvv files to this folder:", tr, tg, tb, true);
            graphics.Print( 320-((tempstring.length()-80)*8), 190, tempstring.substr(0,tempstring.length()-80), tr, tg, tb);
            graphics.Print( 0, 200, tempstring.substr(tempstring.length()-80,40), tr, tg, tb);
            graphics.Print( 0, 210, tempstring.substr(tempstring.length()-40,40), tr, tg, tb);
        }else if(tempstring.length()>40){
            graphics.Print( -1, 170, "To install new player levels, copy", tr, tg, tb, true);
            graphics.Print( -1, 180, "the .vvvvvv files to this folder:", tr, tg, tb, true);
            graphics.Print( 320-((tempstring.length()-40)*8), 200, tempstring.substr(0,tempstring.length()-40), tr, tg, tb);
            graphics.Print( 0, 210, tempstring.substr(tempstring.length()-40,40), tr, tg, tb);
        }else{
            graphics.Print( -1, 180, "To install new player levels, copy", tr, tg, tb, true);
            graphics.Print( -1, 190, "the .vvvvvv files to this folder:", tr, tg, tb, true);
            graphics.Print( 320-(tempstring.length()*8), 210, tempstring, tr, tg, tb);
        }
        break;
    }
    default:
        break;
    }
}

void titlerender()
{

    FillRect(graphics.backBuffer, 0,0,graphics.backBuffer->w, graphics.backBuffer->h, 0x00000000 );

    if (!game.menustart)
    {
        tr = (int)(164 - (help.glow / 2) - int(fRandom() * 4));
        tg = 164 - (help.glow / 2) - int(fRandom() * 4);
        tb = 164 - (help.glow / 2) - int(fRandom() * 4);

        graphics.drawsprite((160 - 96) + 0 * 32, 50, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 1 * 32, 50, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 2 * 32, 50, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 3 * 32, 50, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 4 * 32, 50, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 5 * 32, 50, 23, tr, tg, tb);
        graphics.Print(-1,95,"COMMUNITY EDITION",tr, tg, tb, true);

        graphics.Print(5, 175, "[ Press ACTION to Start ]", tr, tg, tb, true);
        graphics.Print(5, 195, "ACTION = Space, Z, or V", int(tr*0.5f), int(tg*0.5f), int(tb*0.5f), true);
    }
    else
    {
        if(!game.colourblindmode) graphics.drawtowerbackgroundsolo();
        graphics.screenbuffer->badSignalEffect = game.fullScreenEffect_badSignal;

        tr = map.r - (help.glow / 4) - int(fRandom() * 4);
        tg = map.g - (help.glow / 4) - int(fRandom() * 4);
        tb = map.b - (help.glow / 4) - int(fRandom() * 4);

        tr = std::clamp(tr, 0, 255);
        tg = std::clamp(tg, 0, 255);
        tb = std::clamp(tb, 0, 255);

        menurender();

        tr = int(tr * .8f);
        tg = int(tg * .8f);
        tb = int(tb * .8f);
        if (tr < 0) tr = 0;
        if(tr>255) tr=255;
        if (tg < 0) tg = 0;
        if(tg>255) tg=255;
        if (tb < 0) tb = 0;
        if(tb>255) tb=255;
        if (game.currentmenuname == Menu::timetrials || game.currentmenuname == Menu::unlockmenutrials)
        {
            graphics.drawmenu(tr, tg, tb, 15);
        }
        else if (game.currentmenuname == Menu::unlockmenu)
        {
            graphics.drawmenu(tr, tg, tb, 15);
        }
        else if (game.currentmenuname == Menu::playmodes)
        {
            graphics.drawmenu(tr, tg, tb, 20);
        }
        else if (game.currentmenuname == Menu::mainmenu)
        {
            graphics.drawmenu(tr, tg, tb, 15);
        }
        else if (game.currentmenuname == Menu::playerworlds)
        {
            graphics.drawmenu(tr, tg, tb, 15);
        }
        else if (game.currentmenuname == Menu::levellist)
        {
            graphics.drawlevelmenu(tr, tg, tb, 5);
        }
        else
        {
            graphics.drawmenu(tr, tg, tb);
        }
    }

    graphics.drawfade();

    graphics.renderwithscreeneffects();
}

void gamecompleterender()
{
    FillRect(graphics.backBuffer, 0x000000);

    if(!game.colourblindmode) graphics.drawtowerbackgroundsolo();

    tr = map.r - (help.glow / 4) - fRandom() * 4;
    tg = map.g - (help.glow / 4) - fRandom() * 4;
    tb = map.b - (help.glow / 4) - fRandom() * 4;
    if (tr < 0) tr = 0;
    if(tr>255) tr=255;
    if (tg < 0) tg = 0;
    if(tg>255) tg=255;
    if (tb < 0) tb = 0;
    if(tb>255) tb=255;


    //rendering starts... here!

    if (graphics.onscreen(220 + game.creditposition))
    {
        int temp = 220 + game.creditposition;
        graphics.drawsprite((160 - 96) + 0 * 32, temp, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 1 * 32, temp, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 2 * 32, temp, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 3 * 32, temp, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 4 * 32, temp, 23, tr, tg, tb);
        graphics.drawsprite((160 - 96) + 5 * 32, temp, 23, tr, tg, tb);
    }

    if (graphics.onscreen(290 + game.creditposition)) graphics.bigprint( -1, 290 + game.creditposition, "Starring", tr, tg, tb, true, 2);

    if (graphics.onscreen(320 + game.creditposition))
    {
        graphics.drawcrewman(70, 320 + game.creditposition, 0, true);
        graphics.Print(100, 330 + game.creditposition, "Captain Viridian", tr, tg, tb);
    }
    if (graphics.onscreen(350 + game.creditposition))
    {
        graphics.drawcrewman(70, 350 + game.creditposition, 1, true);
        graphics.Print(100, 360 + game.creditposition, "Doctor Violet", tr, tg, tb);
    }
    if (graphics.onscreen(380 + game.creditposition))
    {
        graphics.drawcrewman(70, 380 + game.creditposition, 2, true);
        graphics.Print(100, 390 + game.creditposition, "Professor Vitellary", tr, tg, tb);
    }
    if (graphics.onscreen(410 + game.creditposition))
    {
        graphics.drawcrewman(70, 410 + game.creditposition, 3, true);
        graphics.Print(100, 420 + game.creditposition, "Officer Vermilion", tr, tg, tb);
    }
    if (graphics.onscreen(440 + game.creditposition))
    {
        graphics.drawcrewman(70, 440 + game.creditposition, 4, true);
        graphics.Print(100, 450 + game.creditposition, "Chief Verdigris", tr, tg, tb);
    }
    if (graphics.onscreen(470 + game.creditposition))
    {
        graphics.drawcrewman(70, 470 + game.creditposition, 5, true);
        graphics.Print(100, 480 + game.creditposition, "Doctor Victoria", tr, tg, tb);
    }

    if (graphics.onscreen(520 + game.creditposition)) graphics.bigprint( -1, 520 + game.creditposition, "Credits", tr, tg, tb, true, 3);

    if (graphics.onscreen(560 + game.creditposition))
    {
        graphics.Print(40, 560 + game.creditposition, "Created by", tr, tg, tb);
        graphics.bigprint(60, 570 + game.creditposition, "Terry Cavanagh", tr, tg, tb);
    }

    if (graphics.onscreen(600 + game.creditposition))
    {
        graphics.Print(40, 600 + game.creditposition, "With Music by", tr, tg, tb);
        graphics.bigprint(60, 610 + game.creditposition, "Magnus P~lsson", tr, tg, tb);
    }

    if (graphics.onscreen(640 + game.creditposition))
    {
        graphics.Print(40, 640 + game.creditposition, "Rooms Named by", tr, tg, tb);
        graphics.bigprint(60, 650 + game.creditposition, "Bennett Foddy", tr, tg, tb);
    }

    if (graphics.onscreen(680 + game.creditposition))
    {
        graphics.Print(40, 680 + game.creditposition, "C++ Port by", tr, tg, tb);
        graphics.bigprint(60, 690 + game.creditposition, "Simon Roth", tr, tg, tb);
        graphics.bigprint(60, 710 + game.creditposition, "Ethan Lee", tr, tg, tb);
    }


    if (graphics.onscreen(740 + game.creditposition))
    {
        graphics.Print(40, 740 + game.creditposition, "Beta Testing by", tr, tg, tb);
        graphics.bigprint(60, 750 + game.creditposition, "Sam Kaplan", tr, tg, tb);
        graphics.bigprint(60, 770 + game.creditposition, "Pauli Kohberger", tr, tg, tb);
    }

    if (graphics.onscreen(800 + game.creditposition))
    {
        graphics.Print(40, 800 + game.creditposition, "Ending Picture by", tr, tg, tb);
        graphics.bigprint(60, 810 + game.creditposition, "Pauli Kohberger", tr, tg, tb);
    }

    if (graphics.onscreen(890 + game.creditposition)) graphics.bigprint( -1, 870 + game.creditposition, "Patrons", tr, tg, tb, true, 3);

    int creditOffset = 930;

    for (size_t i = 0; i < game.superpatrons.size(); i += 1)
    {
        if (graphics.onscreen(creditOffset + game.creditposition))
        {
            graphics.Print(-1, creditOffset + game.creditposition, game.superpatrons[i], tr, tg, tb, true);
        }
        creditOffset += 10;
    }

    creditOffset += 10;
    if (graphics.onscreen(creditOffset + game.creditposition)) graphics.Print( -1, creditOffset + game.creditposition, "and", tr, tg, tb, true);
    creditOffset += 20;

    for (size_t i = 0; i < game.patrons.size(); i += 1)
    {
        if (graphics.onscreen(creditOffset + game.creditposition))
        {
            graphics.Print(-1, creditOffset + game.creditposition, game.patrons[i], tr, tg, tb, true);
        }
        creditOffset += 10;
    }

    creditOffset += 20;
    if (graphics.onscreen(creditOffset + game.creditposition)) graphics.bigprint(40, creditOffset + game.creditposition, "GitHub Contributors", tr, tg, tb, true);
    creditOffset += 30;

    for (size_t i = 0; i < game.githubfriends.size(); i += 1)
    {
        if (graphics.onscreen(creditOffset + game.creditposition))
        {
            graphics.Print(-1, creditOffset + game.creditposition, game.githubfriends[i], tr, tg, tb, true);
        }
        creditOffset += 10;
    }

    creditOffset += 140;
    if (graphics.onscreen(creditOffset + game.creditposition)) graphics.bigprint( -1, creditOffset + game.creditposition, "Thanks for playing!", tr, tg, tb, true, 2);

    graphics.drawfade();

    graphics.render();
}

void gamecompleterender2()
{
    FillRect(graphics.backBuffer, 0x000000);

    graphics.drawimage(10, 0, 0);

    for (int j = 0; j < 30; j++)
    {
        for (int i = 0; i < 40; i++)
        {
            if (j == game.creditposy)
            {
                if (i > game.creditposx)
                {
                    FillRect(graphics.backBuffer, i * 8, j * 8, 8, 8, 0, 0, 0);
                }
            }

            if (j > game.creditposy)
            {
                FillRect(graphics.backBuffer, i * 8, j * 8, 8, 8, 0, 0, 0);
            }
        }
    }

    graphics.drawfade();

    graphics.render();
}

void gamerender()
{



    if(!game.blackout)
    {

        if (map.towermode)
        {
            if (!game.colourblindmode)
            {
                if (!graphics.noclear) graphics.drawtowerbackground();
                graphics.drawtowermap();
            }
            else
            {
                graphics.drawtowermap_nobackground();
            }
        }
        else
        {
            if (!graphics.noclear) {
                if(!game.colourblindmode)
                {
                    graphics.drawbackground(map.background);
                }
                else
                {
                    FillRect(graphics.backBuffer,0x00000);
                }
            }
            if (map.final_colormode)
            {
                graphics.drawfinalmap();
            }
            else
            {
                graphics.drawmap();
            }
        }

        for(growing_vector<std::string>::size_type i = 0; i < script.scriptrender.size(); i++) {
            scriptimage current = script.scriptrender[i];
            if (current.type == 3 && current.background) {
                graphics.drawscriptimage( game, current.index, current.x, current.y, current.center, current.alpha, current.blend );
            }
        }

        if(!game.completestop)
        {
            for (size_t i = 0; i < obj.entities.size(); i++)
            {
                //Is this entity on the ground? (needed for jumping)
                if (obj.entitycollidefloor(i))
                {
                    obj.entities[i].onground = 2;
                }
                else
                {
                    obj.entities[i].onground--;
                }

                if (obj.entitycollideroof(i))
                {
                    obj.entities[i].onroof = 2;
                }
                else
                {
                    obj.entities[i].onroof--;
                }

                //Animate the entities
                obj.animateentities(i);
            }
        }

        graphics.drawentities();
        if (map.towermode)
        {
            graphics.drawtowerspikes();
        }

        if (map.custommode && !map.custommodeforreal) {
            if (game.gametimer % 3 == 0) {
                int i = obj.getplayer();
                GhostInfo ghost;
                ghost.rx = game.roomx-100;
                ghost.ry = game.roomy-100;
                ghost.x = obj.entities[i].xp;
                ghost.y = obj.entities[i].yp;
                ghost.col = obj.entities[i].colour;
                ghost.frame = obj.entities[i].drawframe;
                ed.ghosts.push_back(ghost);
            }
            if (ed.ghosts.size() > 100) {
                ed.ghosts.erase(ed.ghosts.begin());
            }
        }
    }

    if(map.extrarow==0 || (map.custommode && map.roomname!=""))
    {
        graphics.footerrect.y = 230;
        if (graphics.translucentroomname)
        {
            SDL_BlitSurface(graphics.footerbuffer, NULL, graphics.backBuffer, &graphics.footerrect);
        }
        else
        {
            FillRect(graphics.backBuffer, graphics.footerrect, 0);
        }

        if (map.finalmode)
        {
            map.glitchname = map.getglitchname(game.roomx, game.roomy);
            graphics.bprint(5, 231, map.glitchname, 196, 196, 255 - help.glow, true);
        }else{

            graphics.bprint(5, 231, map.roomname, 196, 196, 255 - help.glow, true);
        }
    }

    if (map.roomtexton)
    {
        //Draw room text!
        for (size_t i = 0; i < map.roomtext.size(); i++)
        {
            graphics.Print(map.roomtext[i].x*8 + map.roomtext[i].subx, (map.roomtext[i].y*8) + map.roomtext[i].suby, map.roomtext[i].text, 196, 196, 255 - help.glow);
        }
    }

    if (ed.numcoins() > 0 && !game.nocoincounter) {
        std::string coinstring = std::to_string(game.coins);
        if (game.coins >= ed.numcoins()) {
            graphics.bprint(304 - coinstring.length() * 8, 231,coinstring, 255 - help.glow/2, 255 - help.glow/2, 96);
        } else {
            graphics.bprint(304 - coinstring.length() * 8, 231,coinstring, 255 - help.glow/2, 255 - help.glow/2, 196);
        }
        graphics.drawhuetile(311, 230, 48, 1);
    }

    // scriptrender

    for(growing_vector<std::string>::size_type i = 0; i < script.scriptrender.size(); i++) {
        scriptimage current = script.scriptrender[i];
        if (current.type == 0) {
            if (current.bord == 0)
                graphics.Print(current.x,current.y,current.text,current.r,current.g,current.b, current.center);
            else if (current.bord == 1)
                graphics.bprint(current.x,current.y,current.text,current.r,current.g,current.b, current.center);
            else if (current.bord == 2)
                graphics.bigprint(current.x,current.y,current.text,current.r,current.g,current.b, current.center, current.sc);
        } else if (current.type == 1) {
            auto pixels = (uint8_t*) graphics.backBuffer->pixels;
            auto row = pixels + graphics.backBuffer->pitch * current.y;
            auto pixel = ((uint32_t*) row) + current.x;
            *pixel = graphics.getRGB(current.r, current.g, current.b);
        } else if (current.type == 2) {
            SDL_Rect temprect;
            temprect.x = current.x;
            temprect.y = current.y;
            temprect.w = current.w;
            temprect.h = current.h;
#if SDL_BYTEORDER == SDL_BIG_ENDIAN
            auto rmask = 0xff000000;
            auto gmask = 0x00ff0000;
            auto bmask = 0x0000ff00;
            auto amask = 0x000000ff;
#else
            auto rmask = 0x000000ff;
            auto gmask = 0x0000ff00;
            auto bmask = 0x00ff0000;
            auto amask = 0xff000000;
#endif
            auto s = SDL_CreateRGBSurface(0, current.w, current.h, 32, rmask, gmask, bmask, amask);
            SDL_FillRect(s, nullptr, SDL_MapRGBA(s->format, current.r, current.b, current.g, current.alpha));
            SDL_BlitSurface(s, nullptr, graphics.backBuffer, &temprect);
            SDL_FreeSurface(s);
        } else if (current.type == 3 && !current.background) {
            graphics.drawscriptimage( game, current.index, current.x, current.y, current.center, current.alpha, current.blend );
        } else if (current.type == 4) {
            graphics.drawscriptimagemasked( game, current.index, current.x, current.y, current.mask_index, current.mask_x, current.mask_y );
        }
    }

    // Now we have to clear the vector
    if (script.scriptrender.size() > 0) {
        for(int i = (int)script.scriptrender.size() - 1; i >= 0; i--) {
            if (!script.scriptrender[i].persistent) {
                script.scriptrender.erase(script.scriptrender.begin() + i);
            }
        }
    }

#if !defined(NO_CUSTOM_LEVELS)
    if(map.custommode && !map.custommodeforreal && !game.advancetext){
        //Return to level editor
        graphics.bprintalpha(5, 5, "[Press ENTER to return to editor]", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), ed.returneditoralpha, false);
        if (ed.returneditoralpha > 0) {
            ed.returneditoralpha -= 15;
        }
    }
#endif


    graphics.cutscenebars();
    graphics.drawfade();
    BlitSurfaceStandard(graphics.backBuffer, NULL, graphics.tempBuffer, NULL);

    graphics.drawgui();
    if (graphics.flipmode)
    {
        if (game.advancetext) graphics.bprint(5, 228, "- Press ACTION to advance text -", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
    }
    else
    {
        if (game.advancetext) graphics.bprint(5, 5, "- Press ACTION to advance text -", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
    }

    if (game.readytotele > 100 && !game.advancetext && game.hascontrol && (!script.running || (script.running && script.passive)) && !game.intimetrial)
    {
        if(graphics.flipmode)
        {
            graphics.bprint(5, 20, "- Press ENTER to Teleport -", game.readytotele - 20 - (help.glow / 2), game.readytotele - 20 - (help.glow / 2), game.readytotele, true);
        }
        else
        {
            graphics.bprint(5, 210, "- Press ENTER to Teleport -", game.readytotele - 20 - (help.glow / 2), game.readytotele - 20 - (help.glow / 2), game.readytotele, true);
        }
    }

    if (game.swnmode)
    {
        if (game.swngame == 0)
        {
            std::string tempstring = help.timestring(game.swntimer);
            graphics.bigprint( -1, 20, tempstring, 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 2);
        }
        else if (game.swngame == 1)
        {
            if (game.swnmessage == 0)
            {
                std::string tempstring = help.timestring(game.swntimer);
                graphics.Print( 10, 10, "Current Time", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), false);
                graphics.bigprint( 25, 24, tempstring, 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), false, 2);
                tempstring = help.timestring(game.swnrecord);
                graphics.Print( 240, 10, "Best Time", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), false);
                graphics.bigrprint( 300, 24, tempstring, 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), false, 2);

                switch(game.swnbestrank)
                {
                case 0:
                    graphics.Print( -1, 204, "Next Trophy at 5 seconds", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
                    break;
                case 1:
                    graphics.Print( -1, 204, "Next Trophy at 10 seconds", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
                    break;
                case 2:
                    graphics.Print( -1, 204, "Next Trophy at 15 seconds", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
                    break;
                case 3:
                    graphics.Print( -1, 204, "Next Trophy at 20 seconds", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
                    break;
                case 4:
                    graphics.Print( -1, 204, "Next Trophy at 30 seconds", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
                    break;
                case 5:
                    graphics.Print( -1, 204, "Next Trophy at 1 minute", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
                    break;
                case 6:
                    graphics.Print( -1, 204, "All Trophies collected!", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
                    break;
                }
            }
            else if (game.swnmessage == 1)
            {
                std::string tempstring = help.timestring(game.swntimer);
                graphics.Print( 10, 10, "Current Time", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), false);
                graphics.bigprint( 25, 24, tempstring, 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), false, 2);
                tempstring = help.timestring(game.swnrecord);
                if (int(game.deathseq / 5) % 2 == 1)
                {
                    graphics.Print( 240, 10, "Best Time", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), false);
                    graphics.bigrprint( 300, 24, tempstring, 128 - (help.glow), 220 - (help.glow), 128 - (help.glow / 2), false, 2);

                    graphics.bigprint( -1, 200, "New Record!", 128 - (help.glow), 220 - (help.glow), 128 - (help.glow / 2), true, 2);
                }
            }
            else if (game.swnmessage >= 2)
            {
                game.swnmessage--;
                if (game.swnmessage == 2) game.swnmessage = 0;
                std::string tempstring = help.timestring(game.swntimer);
                graphics.Print( 10, 10, "Current Time", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), false);
                graphics.bigprint( 25, 24, tempstring, 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), false, 2);
                tempstring = help.timestring(game.swnrecord);
                graphics.Print( 240, 10, "Best Time", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), false);
                graphics.bigrprint( 300, 24, tempstring, 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), false, 2);

                if (int(game.swnmessage / 5) % 2 == 1)
                {
                    graphics.bigprint( -1, 200, "New Trophy!", 220 - (help.glow), 128 - (help.glow), 128 - (help.glow / 2), true, 2);
                }
            }

            graphics.Print( 20, 228, "[Press ENTER to stop]", 160 - (help.glow/2), 160 - (help.glow/2), 160 - (help.glow/2), true);
        }
        else if(game.swngame==2)
        {
            if (int(game.swndelay / 15) % 2 == 1 || game.swndelay >= 120)
            {
                if (graphics.flipmode)
                {
                    graphics.bigprint( -1, 30, "Survive for", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 2);
                    graphics.bigprint( -1, 10, "60 seconds!", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 2);
                }
                else
                {
                    graphics.bigprint( -1, 10, "Survive for", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 2);
                    graphics.bigprint( -1, 30, "60 seconds!", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 2);
                }
            }
        }
        else if(game.swngame==7)
        {
            if (game.swndelay >= 60)
            {
                graphics.bigprint( -1, 20, "SUPER GRAVITRON", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 2);

                std::string tempstring = help.timestring(game.swnrecord);
                graphics.Print( 240, 190, "Best Time", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
                graphics.bigrprint( 300, 205, tempstring, 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 2);
            }
            else	if (int(game.swndelay / 10) % 2 == 1)
            {
                graphics.bigprint( -1, 20, "SUPER GRAVITRON", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 2);
                graphics.bigprint( -1, 200, "GO!", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 3);
            }
        }
    }

    if (game.intimetrial && graphics.fademode==0)
    {
        //Draw countdown!
        if (game.timetrialcountdown > 0)
        {
            if (game.timetrialcountdown < 30)
            {
                game.resetgameclock();
                if (int(game.timetrialcountdown / 4) % 2 == 0) graphics.bigprint( -1, 100, "Go!", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 4);
            }
            else if (game.timetrialcountdown < 60)
            {
                graphics.bigprint( -1, 100, "1", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 4);
            }
            else if (game.timetrialcountdown < 90)
            {
                graphics.bigprint( -1, 100, "2", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 4);
            }
            else if (game.timetrialcountdown < 120)
            {
                graphics.bigprint( -1, 100, "3", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true, 4);
            }
        }
        else
        {
            //Draw OSD stuff
            graphics.bprint(6, 18, "TIME :",  255,255,255);
            graphics.bprint(6, 30, "DEATH:",  255, 255, 255);
            graphics.bprint(6, 42, "SHINY:",  255,255,255);

            if(game.timetrialparlost)
            {
                graphics.bprint(56, 18, game.timestring(),  196, 80, 80);
            }
            else
            {
                graphics.bprint(56, 18, game.timestring(),  196, 196, 196);
            }
            if(game.deathcounts>0)
            {
                graphics.bprint(56, 30,help.String(game.deathcounts),  196, 80, 80);
            }
            else
            {
                graphics.bprint(56, 30,help.String(game.deathcounts),  196, 196, 196);
            }
            if(game.trinkets()<game.timetrialshinytarget)
            {
                graphics.bprint(56, 42,help.String(game.trinkets()) + " of " +help.String(game.timetrialshinytarget),  196, 80, 80);
            }
            else
            {
                graphics.bprint(56, 42,help.String(game.trinkets()) + " of " +help.String(game.timetrialshinytarget),  196, 196, 196);
            }

            if(game.timetrialparlost)
            {
                graphics.bprint(195, 214, "PAR TIME:",  80, 80, 80);
                graphics.bprint(275, 214, game.partimestring(),  80, 80, 80);
            }
            else
            {
                graphics.bprint(195, 214, "PAR TIME:",  255, 255, 255);
                graphics.bprint(275, 214, game.partimestring(),  196, 196, 196);
            }
        }
    }

    if (game.activeactivity > -1)
    {
        game.activity_lastprompt = obj.blocks[game.activeactivity].prompt;
        game.activity_r = obj.blocks[game.activeactivity].r;
        game.activity_g = obj.blocks[game.activeactivity].g;
        game.activity_b = obj.blocks[game.activeactivity].b;
        if(game.act_fade<5) game.act_fade=5;
        if(game.act_fade<10)
        {
            game.act_fade++;
        }
        graphics.drawtextbox(16, 4, 36, 3, game.activity_r*(game.act_fade/10.0f), game.activity_g*(game.act_fade/10.0f), game.activity_b*(game.act_fade/10.0f));
        graphics.Print(5, 12, game.activity_lastprompt, game.activity_r*(game.act_fade/10.0f), game.activity_g*(game.act_fade/10.0f), game.activity_b*(game.act_fade/10.0f), true);
    }
    else
    {
        if(game.act_fade>5)
        {
            graphics.drawtextbox(16, 4, 36, 3, game.activity_r*(game.act_fade/10.0f), game.activity_g*(game.act_fade/10.0f), game.activity_b*(game.act_fade/10.0f));
            graphics.Print(5, 12, game.activity_lastprompt, game.activity_r*(game.act_fade/10.0f), game.activity_g*(game.act_fade/10.0f), game.activity_b*(game.act_fade/10.0f), true);
            game.act_fade--;
        }
    }

    if (obj.trophytext > 0)
    {
        graphics.drawtrophytext();
        obj.trophytext--;
    }


    if (script.getpixelx != -1) {
        auto x = script.getpixelx;
        auto y = script.getpixely;
        auto pixels = (char*) graphics.backBuffer->pixels;
        auto pixel_ptr = pixels + (x * graphics.backBuffer->format->BytesPerPixel) + (y * graphics.backBuffer->pitch);
        uint32_t pixel;
        std::memcpy(&pixel, pixel_ptr, sizeof(uint32_t));
        uint8_t r;
        uint8_t g;
        uint8_t b;
        SDL_GetRGB(pixel, graphics.backBuffer->format, &r, &g, &b);
        script.setvar("r", std::to_string(r));
        script.setvar("g", std::to_string(g));
        script.setvar("b", std::to_string(b));
        script.getpixelx = -1;
        script.getpixely = -1;
    }

    graphics.renderwithscreeneffects();
}

void maprender()
{
    graphics.drawgui();

    //draw screen alliteration
    //Roomname:
    int temp = map.area(game.roomx, game.roomy);
    if (temp < 2 && !map.custommode && graphics.fademode==0)
    {
        if (game.roomx >= 102 && game.roomx <= 104 && game.roomy >= 110 && game.roomy <= 111)
        {
            graphics.Print(5, 2, "The Ship", 196, 196, 255 - help.glow, true);
        }
        else
        {
            graphics.Print(5, 2, "Dimension VVVVVV", 196, 196, 255 - help.glow, true);
        }
    }
    else
    {
      if (map.finalmode){
        map.glitchname = map.getglitchname(game.roomx, game.roomy);
        graphics.Print(5, 2, map.glitchname, 196, 196, 255 - help.glow, true);
      }else{
        std::string usethisname;
        Dimension* dim = map.getdimension();
        if (!map.roomname.length() && dim != NULL)
            usethisname = dim->name;
        else
            usethisname = map.roomname;
        graphics.Print(5, 2, usethisname, 196, 196, 255 - help.glow, true);
      }
    }

    //Background color
    FillRect(graphics.backBuffer,0, 12, 320, 240, 10, 24, 26 );

    graphics.crewframedelay--;
    if (graphics.crewframedelay <= 0)
    {
        graphics.crewframedelay = 8;
        graphics.crewframe = (graphics.crewframe + 1) % 2;
    }



    //Menubar:
    graphics.drawtextbox( -10, 212, 42, 3, 65, 185, 207);
    switch(game.menupage)
    {
    case 0:
        graphics.Print(30 - 8, 220, "[MAP]", 196, 196, 255 - help.glow);
        if (game.insecretlab)
        {
            graphics.Print(103, 220, "GRAV", 64, 64, 64);
        }
        else if (obj.flags[67] && !map.custommode)
        {
            graphics.Print(103, 220, "SHIP", 64,64,64);
        }
        else
        {
            graphics.Print(103, 220, "CREW", 64,64,64);
        }
        graphics.Print(185-4, 220, "STATS", 64,64,64);
        graphics.Print(258, 220, "SAVE", 64,64,64);

        if (map.finalmode || (map.custommode&&!map.customshowmm))
        {
            //draw the map image
            graphics.drawpixeltextbox(35, 16, 250, 190, 32,24, 65, 185, 207,4,0);
            graphics.drawimage(1, 40, 21, false);
            for (int j = 0; j < 20; j++)
            {
                for (int i = 0; i < 20; i++)
                {
                    graphics.drawimage(2, 40 + (i * 12), 21 + (j * 9), false);
                }
            }
            graphics.Print(-1, 105, "NO SIGNAL", 245, 245, 245, true);
        }
        else if(map.custommode)
        {
          //draw the map image
          graphics.drawcustompixeltextbox(35+map.custommmxoff, 16+map.custommmyoff, map.custommmxsize+10, map.custommmysize+10, (map.custommmxsize+10)/8, (map.custommmysize+10)/8, 65, 185, 207,4,0);
          graphics.drawpartimage(12, 40+map.custommmxoff, 21+map.custommmyoff, map.custommmxsize,map.custommmysize);

          //Black out here
          if (!map.nofog) {
            if(map.customzoom==4){
                for (int j = 0; j < map.customheight; j++){
                for (int i = 0; i < map.customwidth; i++){
                    int i2 = i + map.custommmstartx;
                    int j2 = j + map.custommmstarty;
                    if(map.explored[i2+(j2*ed.maxwidth)]==0){
                    //Draw the fog of war on the map
                    graphics.drawimage(2, map.custommmxoff+40 + (i * 48), map.custommmyoff+21 + (j * 36), false);
                    graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48), map.custommmyoff+21 + (j * 36), false);
                    graphics.drawimage(2, map.custommmxoff+40 + (i * 48), map.custommmyoff+21 + 9 + (j * 36), false);
                    graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48), map.custommmyoff+21 + 9+ (j * 36), false);

                    graphics.drawimage(2, map.custommmxoff+40 + (i * 48) + 24, map.custommmyoff+21 + (j * 36), false);
                    graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48) + 24, map.custommmyoff+21 + (j * 36), false);
                    graphics.drawimage(2, map.custommmxoff+40 + (i * 48) + 24, map.custommmyoff+ 21 + 9 + (j * 36), false);
                    graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48) + 24, map.custommmyoff+21 + 9+ (j * 36), false);

                    graphics.drawimage(2, map.custommmxoff+40 + (i * 48), map.custommmyoff+21 + (j * 36)+18, false);
                    graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48), map.custommmyoff+21 + (j * 36)+18, false);
                    graphics.drawimage(2, map.custommmxoff+40 + (i * 48), map.custommmyoff+21 + 9 + (j * 36)+18, false);
                    graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48), map.custommmyoff+21 + 9+ (j * 36)+18, false);

                    graphics.drawimage(2, map.custommmxoff+40 + (i * 48) + 24, map.custommmyoff+21 + (j * 36)+18, false);
                    graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48) + 24, map.custommmyoff+21 + (j * 36)+18, false);
                    graphics.drawimage(2, map.custommmxoff+40 + (i * 48) + 24, map.custommmyoff+21 + 9 + (j * 36)+18, false);
                    graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48) + 24, map.custommmyoff+21 + 9+ (j * 36)+18, false);
                    }
                }
                }
            }else if(map.customzoom==2){
                for (int j = 0; j < map.customheight; j++){
                for (int i = 0; i < map.customwidth; i++){
                    int i2 = i + map.custommmstartx;
                    int j2 = j + map.custommmstarty;
                    if(map.explored[i2+(j2*ed.maxwidth)]==0){
                    //Draw the fog of war on the map
                    graphics.drawimage(2, map.custommmxoff+40 + (i * 24), map.custommmyoff+21 + (j * 18), false);
                    graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 24), map.custommmyoff+21 + (j * 18), false);
                    graphics.drawimage(2, map.custommmxoff+40 + (i * 24), map.custommmyoff+21 + 9 + (j * 18), false);
                    graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 24), map.custommmyoff+21 + 9+ (j * 18), false);
                    }
                }
                }
            }else{
                for (int j = 0; j < map.customheight; j++){
                for (int i = 0; i < map.customwidth; i++){
                    int i2 = i + map.custommmstartx;
                    int j2 = j + map.custommmstarty;
                    if(map.explored[i2+(j2*ed.maxwidth)]==0){
                    //Draw the fog of war on the map
                    graphics.drawimage(2, map.custommmxoff+40 + (i * 12), map.custommmyoff+21 + (j * 9), false);
                    }
                }
                }
            }
          }

          if (map.cursorstate == 0){
            map.cursordelay++;
            if (map.cursordelay > 10){
              map.cursorstate = 1;
              map.cursordelay = 0;
            }
          }else if (map.cursorstate == 1){
            map.cursordelay++;
            if (map.cursordelay > 30) map.cursorstate = 2;
          }else if (map.cursorstate == 2){
            map.cursordelay++;
          }

          //normal size maps
          if(map.customzoom==4){
            if(map.cursorstate==1){
              if (int(map.cursordelay / 4) % 2 == 0){
                graphics.drawrect(40 + ((game.roomx - 100 - map.custommmstartx) * 48) +map.custommmxoff, 21 + ((game.roomy - 100 - map.custommmstarty) * 36)+map.custommmyoff , 48 , 36 , 255,255,255);
                graphics.drawrect(40 + ((game.roomx - 100 - map.custommmstartx) * 48) + 2+map.custommmxoff, 21 + ((game.roomy - 100 - map.custommmstarty) * 36) + 2+map.custommmyoff, 48 - 4, 36 - 4, 255,255,255);
              }
            }else if (map.cursorstate == 2){
              if (int(map.cursordelay / 15) % 2 == 0){
                graphics.drawrect(40 + ((game.roomx - 100 - map.custommmstartx) * 48) + 2+map.custommmxoff, 21 + ((game.roomy - 100 - map.custommmstarty) * 36) + 2+map.custommmyoff, 48 - 4, 36 - 4, 16, 245 - (help.glow), 245 - (help.glow));
              }
            }
          }else if(map.customzoom==2){
            if(map.cursorstate==1){
              if (int(map.cursordelay / 4) % 2 == 0){
                graphics.drawrect(40 + ((game.roomx - 100 - map.custommmstartx) * 24)+map.custommmxoff , 21 + ((game.roomy - 100 - map.custommmstarty) * 18)+map.custommmyoff , 24 , 18 , 255,255,255);
                graphics.drawrect(40 + ((game.roomx - 100 - map.custommmstartx) * 24) + 2+map.custommmxoff, 21 + ((game.roomy - 100 - map.custommmstarty) * 18) + 2+map.custommmyoff, 24 - 4, 18 - 4, 255,255,255);
              }
            }else if (map.cursorstate == 2){
              if (int(map.cursordelay / 15) % 2 == 0){
                graphics.drawrect(40 + ((game.roomx - 100 - map.custommmstartx) * 24) + 2+map.custommmxoff, 21 + ((game.roomy - 100 - map.custommmstarty) * 18) + 2+map.custommmyoff, 24 - 4, 18 - 4, 16, 245 - (help.glow), 245 - (help.glow));
              }
            }
          }else{
            if(map.cursorstate==1){
              if (int(map.cursordelay / 4) % 2 == 0){
                graphics.drawrect(40 + ((game.roomx - 100 - map.custommmstartx) * 12)+map.custommmxoff , 21 + ((game.roomy - 100 - map.custommmstarty) * 9)+map.custommmyoff , 12 , 9 , 255,255,255);
                graphics.drawrect(40 + ((game.roomx - 100 - map.custommmstartx) * 12) + 2+map.custommmxoff, 21 + ((game.roomy - 100 - map.custommmstarty) * 9) + 2+map.custommmyoff, 12 - 4, 9 - 4, 255,255,255);
              }
            }else if (map.cursorstate == 2){
              if (int(map.cursordelay / 15) % 2 == 0){
                graphics.drawrect(40 + ((game.roomx - 100 - map.custommmstartx) * 12) + 2+map.custommmxoff, 21 + ((game.roomy - 100 - map.custommmstarty) * 9) + 2+map.custommmyoff, 12 - 4, 9 - 4, 16, 245 - (help.glow), 245 - (help.glow));
              }
            }
          }
            for (auto marker : game.scriptmarkers)
            {
                if (!game.hidemarkers) graphics.drawtile(43 + (marker.x * 12), 22 + (marker.y * 9), marker.tile);
            }
        }
        else
        {
            //draw the map image
            graphics.drawpixeltextbox(35, 16, 250, 190, 32,24, 65, 185, 207,4,0);
            graphics.drawimage(1, 40, 21, false);

            //black out areas we can't see yet
            for (int j = 0; j < 20; j++)
            {
                for (int i = 0; i < 20; i++)
                {
                    if(map.explored[i+(j*ed.maxwidth)]==0)
                    {
                        //Draw the fog of war on the map
                        graphics.drawimage(2, 40 + (i * 12), 21 + (j * 9), false);
                    }
                }
            }
            //draw the coordinates
            if (game.roomx == 109)
            {
                //tower!instead of room y, scale map.ypos
                if (map.cursorstate == 0)
                {
                    map.cursordelay++;
                    if (map.cursordelay > 10)
                    {
                        map.cursorstate = 1;
                        map.cursordelay = 0;
                    }
                }
                else if (map.cursorstate == 1)
                {
                    map.cursordelay++;
                    if (int(map.cursordelay / 4) % 2 == 0)
                    {
                        graphics.drawrect(40 + ((game.roomx - 100) * 12) , 21 , 12, 180, 255,255,255);
                        graphics.drawrect(40 + ((game.roomx - 100) * 12) + 2 , 21  + 2, 12 - 4, 180 - 4, 255,255,255);
                    }
                    if (map.cursordelay > 30) map.cursorstate = 2;
                }
                else if (map.cursorstate == 2)
                {
                    map.cursordelay++;
                    if (int(map.cursordelay / 15) % 2 == 0)
                    {
                        graphics.drawrect(40 + ((game.roomx - 100) * 12) + 2 , 21  + 2, 12 - 4, 180 - 4,16, 245 - (help.glow), 245 - (help.glow));
                    }
                }
            }
            else
            {
                if (map.cursorstate == 0)
                {
                    map.cursordelay++;
                    if (map.cursordelay > 10)
                    {
                        map.cursorstate = 1;
                        map.cursordelay = 0;
                    }
                }
                else if (map.cursorstate == 1)
                {
                    map.cursordelay++;
                    if (int(map.cursordelay / 4) % 2 == 0)
                    {
                        graphics.drawrect(40 + ((game.roomx - 100) * 12) , 21 + ((game.roomy - 100) * 9) , 12 , 9 , 255,255,255);
                        graphics.drawrect(40 + ((game.roomx - 100) * 12) + 2, 21 + ((game.roomy - 100) * 9) + 2, 12 - 4, 9 - 4, 255,255,255);
                    }
                    if (map.cursordelay > 30) map.cursorstate = 2;
                }
                else if (map.cursorstate == 2)
                {
                    map.cursordelay++;
                    if (int(map.cursordelay / 15) % 2 == 0)
                    {
                        graphics.drawrect(40 + ((game.roomx - 100) * 12) + 2, 21 + ((game.roomy - 100) * 9) + 2, 12 - 4, 9 - 4, 16, 245 - (help.glow), 245 - (help.glow));
                    }
                }
            }

            //draw legend details
            for (size_t i = 0; i < map.teleporters.size(); i++)
            {
                if (map.showteleporters && map.explored[map.teleporters[i].x + (ed.maxwidth * map.teleporters[i].y)] > 0)
                {
                    int temp = 1126 + map.explored[map.teleporters[i].x + (ed.maxwidth * map.teleporters[i].y)];
                    if (graphics.flipmode) temp += 3;
                    graphics.drawtile(40 + 3 + (map.teleporters[i].x * 12), 22 + (map.teleporters[i].y * 9), temp);
                }
                else if(map.showtargets && map.explored[map.teleporters[i].x+(ed.maxwidth*map.teleporters[i].y)]==0)
                {
                    int temp = 1126 + map.explored[map.teleporters[i].x + (ed.maxwidth * map.teleporters[i].y)];
                    if (graphics.flipmode) temp += 3;
                    graphics.drawtile(40 + 3 + (map.teleporters[i].x * 12), 22 + (map.teleporters[i].y * 9), temp);
                }
            }

            if (map.showtrinkets)
            {
                for (size_t i = 0; i < map.shinytrinkets.size(); i++)
                {
                    if (!obj.collect[i])
                    {
                        int temp = 1086;
                        if (graphics.flipmode) temp += 3;
                        graphics.drawtile(40 + 3 + (map.shinytrinkets[i].x * 12), 22 + (map.shinytrinkets[i].y * 9),	temp);
                    }
                }
            }
        }
        break;
    case 1:
        if (game.insecretlab)
        {
            graphics.Print(30, 220, "MAP", 64,64,64);
            graphics.Print(103-8, 220, "[GRAV]", 196, 196, 255 - help.glow);
            graphics.Print(185-4, 220, "STATS", 64,64,64);
            graphics.Print(258, 220, "SAVE", 64, 64, 64);

            if (graphics.flipmode)
            {
                graphics.Print(0, 174, "SUPER GRAVITRON HIGHSCORE", 196, 196, 255 - help.glow, true);

                std::string tempstring = help.timestring(game.swnrecord);
                graphics.Print( 240, 124, "Best Time", 196, 196, 255 - help.glow, true);
                graphics.bigrprint( 300, 94, tempstring, 196, 196, 255 - help.glow, true, 2);

                switch(game.swnbestrank)
                {
                case 0:
                    graphics.Print( -1, 40, "Next Trophy at 5 seconds", 196, 196, 255 - help.glow, true);
                    break;
                case 1:
                    graphics.Print( -1, 40, "Next Trophy at 10 seconds", 196, 196, 255 - help.glow, true);
                    break;
                case 2:
                    graphics.Print( -1, 40, "Next Trophy at 15 seconds", 196, 196, 255 - help.glow, true);
                    break;
                case 3:
                    graphics.Print( -1, 40, "Next Trophy at 20 seconds", 196, 196, 255 - help.glow, true);
                    break;
                case 4:
                    graphics.Print( -1, 40, "Next Trophy at 30 seconds", 196, 196, 255 - help.glow, true);
                    break;
                case 5:
                    graphics.Print( -1, 40, "Next Trophy at 1 minute", 196, 196, 255 - help.glow, true);
                    break;
                case 6:
                    graphics.Print( -1, 40, "All Trophies collected!", 196, 196, 255 - help.glow, true);
                    break;
                }
            }
            else
            {
                graphics.Print(0, 40, "SUPER GRAVITRON HIGHSCORE", 196, 196, 255 - help.glow, true);

                std::string tempstring = help.timestring(game.swnrecord);
                graphics.Print( 240, 90, "Best Time", 196, 196, 255 - help.glow, true);
                graphics.bigrprint( 300, 104, tempstring, 196, 196, 255 - help.glow, true, 2);

                switch(game.swnbestrank)
                {
                case 0:
                    graphics.Print( -1, 174, "Next Trophy at 5 seconds", 196, 196, 255 - help.glow, true);
                    break;
                case 1:
                    graphics.Print( -1, 174, "Next Trophy at 10 seconds", 196, 196, 255 - help.glow, true);
                    break;
                case 2:
                    graphics.Print( -1, 174, "Next Trophy at 15 seconds", 196, 196, 255 - help.glow, true);
                    break;
                case 3:
                    graphics.Print( -1, 174, "Next Trophy at 20 seconds", 196, 196, 255 - help.glow, true);
                    break;
                case 4:
                    graphics.Print( -1, 174, "Next Trophy at 30 seconds", 196, 196, 255 - help.glow, true);
                    break;
                case 5:
                    graphics.Print( -1, 174, "Next Trophy at 1 minute", 196, 196, 255 - help.glow, true);
                    break;
                case 6:
                    graphics.Print( -1, 174, "All Trophies collected!", 196, 196, 255 - help.glow, true);
                    break;
                }
            }
        }
        else if (obj.flags[67] && !map.custommode)
        {
            graphics.Print(30, 220, "MAP", 64,64,64);
            graphics.Print(103-8, 220, "[SHIP]", 196, 196, 255 - help.glow);
            graphics.Print(185-4, 220, "STATS", 64,64,64);
            graphics.Print(258, 220, "SAVE", 64, 64, 64);

            graphics.Print(0, 105, "Press ACTION to warp to the ship.", 196, 196, 255 - help.glow, true);
        }
#if !defined(NO_CUSTOM_LEVELS)
        else if(map.custommode){
            graphics.Print(30, 220, "MAP", 64,64,64);
            graphics.Print(103-8, 220, "[CREW]", 196, 196, 255 - help.glow);
            graphics.Print(185-4, 220, "STATS", 64,64,64);
            graphics.Print(258, 220, "SAVE", 64, 64, 64);

            if (graphics.flipmode)
            {
                graphics.bigprint( -1, 220-45, ed.ListOfMetaData[game.playcustomlevel].title, 196, 196, 255 - help.glow, true);
                graphics.Print( -1, 220-70, "by " + ed.ListOfMetaData[game.playcustomlevel].creator, 196, 196, 255 - help.glow, true);
                graphics.Print( -1, 220-80, ed.ListOfMetaData[game.playcustomlevel].website, 196, 196, 255 - help.glow, true);
                graphics.Print( -1, 220-100, ed.ListOfMetaData[game.playcustomlevel].Desc1, 196, 196, 255 - help.glow, true);
                graphics.Print( -1, 220-110, ed.ListOfMetaData[game.playcustomlevel].Desc2, 196, 196, 255 - help.glow, true);
                graphics.Print( -1, 220-120, ed.ListOfMetaData[game.playcustomlevel].Desc3, 196, 196, 255 - help.glow, true);

                if(ed.numcrewmates()-game.crewmates()==1){
                    graphics.Print(1,220-165, help.number(ed.numcrewmates()-game.crewmates())+ " crewmate remains", 196, 196, 255 - help.glow, true);
                }else if(ed.numcrewmates()-game.crewmates()>0){
                    graphics.Print(1,220-165, help.number(ed.numcrewmates()-game.crewmates())+ " crewmates remain", 196, 196, 255 - help.glow, true);
                }
            }
            else
            {
                graphics.bigprint( -1, 45, ed.ListOfMetaData[game.playcustomlevel].title, 196, 196, 255 - help.glow, true);
                graphics.Print( -1, 70, "by " + ed.ListOfMetaData[game.playcustomlevel].creator, 196, 196, 255 - help.glow, true);
                graphics.Print( -1, 80, ed.ListOfMetaData[game.playcustomlevel].website, 196, 196, 255 - help.glow, true);
                graphics.Print( -1, 100, ed.ListOfMetaData[game.playcustomlevel].Desc1, 196, 196, 255 - help.glow, true);
                graphics.Print( -1, 110, ed.ListOfMetaData[game.playcustomlevel].Desc2, 196, 196, 255 - help.glow, true);
                graphics.Print( -1, 120, ed.ListOfMetaData[game.playcustomlevel].Desc3, 196, 196, 255 - help.glow, true);

                if(ed.numcrewmates()-game.crewmates()==1){
                    graphics.Print(1,165, help.number(ed.numcrewmates()-game.crewmates())+ " crewmate remains", 196, 196, 255 - help.glow, true);
                }else if(ed.numcrewmates()-game.crewmates()>0){
                    graphics.Print(1,165, help.number(ed.numcrewmates()-game.crewmates())+ " crewmates remain", 196, 196, 255 - help.glow, true);
                }
            }
        }
#endif
        else
        {
            graphics.Print(30, 220, "MAP", 64,64,64);
            graphics.Print(103-8, 220, "[CREW]", 196, 196, 255 - help.glow);
            graphics.Print(185-4, 220, "STATS", 64,64,64);
            graphics.Print(258, 220, "SAVE", 64, 64, 64);

            if (graphics.flipmode)
            {
                for (int i = 0; i < 3; i++)
                {
                    graphics.drawcrewman(16, 32 + (i * 64), 2-i, game.crewstats[2-i]);
                    if (game.crewstats[(2-i)])
                    {
                        graphics.printcrewname(44, 32 + (i * 64)+4+10, 2-i);
                        graphics.printcrewnamestatus(44, 32 + (i * 64)+4, 2-i);
                    }
                    else
                    {
                        graphics.printcrewnamedark(44, 32 + (i * 64)+4+10, 2-i);
                        graphics.Print(44, 32 + (i * 64) + 4, "Missing...", 64,64,64);
                    }

                    graphics.drawcrewman(16+160, 32 + (i * 64), (2-i)+3, game.crewstats[(2-i)+3]);
                    if (game.crewstats[(2-i)+3])
                    {
                        graphics.printcrewname(44+160, 32 + (i * 64)+4+10, (2-i)+3);
                        graphics.printcrewnamestatus(44+160, 32 + (i * 64)+4, (2-i)+3);
                    }
                    else
                    {
                        graphics.printcrewnamedark(44+160, 32 + (i * 64)+4+10, (2-i)+3);
                        graphics.Print(44+160, 32 + (i * 64) + 4, "Missing...", 64,64,64);
                    }
                }
            }
            else
            {
                for (int i = 0; i < 3; i++)
                {
                    graphics.drawcrewman(16, 32 + (i * 64), i, game.crewstats[i]);
                    if (game.crewstats[i])
                    {
                        graphics.printcrewname(44, 32 + (i * 64)+4, i);
                        graphics.printcrewnamestatus(44, 32 + (i * 64)+4+10, i);
                    }
                    else
                    {
                        graphics.printcrewnamedark(44, 32 + (i * 64)+4, i);
                        graphics.Print(44, 32 + (i * 64) + 4 + 10, "Missing...", 64,64,64);
                    }

                    graphics.drawcrewman(16+160, 32 + (i * 64), i+3, game.crewstats[i+3]);
                    if (game.crewstats[i+3])
                    {
                        graphics.printcrewname(44+160, 32 + (i * 64)+4, i+3);
                        graphics.printcrewnamestatus(44+160, 32 + (i * 64)+4+10, i+3);
                    }
                    else
                    {
                        graphics.printcrewnamedark(44+160, 32 + (i * 64)+4, i+3);
                        graphics.Print(44+160, 32 + (i * 64) + 4 + 10, "Missing...", 64,64,64);
                    }
                }
            }
        }
        break;
    case 2:
        graphics.Print(30, 220, "MAP", 64,64,64);
        if (game.insecretlab)
        {
            graphics.Print(103, 220, "GRAV", 64, 64, 64);
        }
        else if (obj.flags[67] && !map.custommode)
        {
            graphics.Print(103, 220, "SHIP", 64,64,64);
        }
        else
        {
            graphics.Print(103, 220, "CREW", 64,64,64);
        }
        graphics.Print(185-12, 220, "[STATS]", 196, 196, 255 - help.glow);
        graphics.Print(258, 220, "SAVE", 64, 64, 64);

#if !defined(NO_CUSTOM_LEVELS)
        if(map.custommode)
        {
          if (graphics.flipmode)
          {
              graphics.Print(0, 164, "[Trinkets found]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 152, help.number(game.trinkets()) + " out of " + help.number(ed.numtrinkets()), 96,96,96, true);

              graphics.Print(0, 114, "[Number of Deaths]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 102,help.String(game.deathcounts),  96,96,96, true);

              graphics.Print(0, 64, "[Time Taken]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 52, game.timestring(),  96, 96, 96, true);
          }
          else
          {
              graphics.Print(0, 52, "[Trinkets found]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 64, help.number(game.trinkets()) + " out of "+help.number(ed.numtrinkets()), 96,96,96, true);

              graphics.Print(0, 102, "[Number of Deaths]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 114,help.String(game.deathcounts),  96,96,96, true);

              graphics.Print(0, 152, "[Time Taken]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 164, game.timestring(),  96, 96, 96, true);
          }
        }
        else
#endif
        {
          if (graphics.flipmode)
          {
              graphics.Print(0, 164, "[Trinkets found]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 152, help.number(game.trinkets()) + " out of Twenty", 96,96,96, true);

              graphics.Print(0, 114, "[Number of Deaths]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 102,help.String(game.deathcounts),  96,96,96, true);

              graphics.Print(0, 64, "[Time Taken]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 52, game.timestring(),  96, 96, 96, true);
          }
          else
          {
              graphics.Print(0, 52, "[Trinkets found]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 64, help.number(game.trinkets()) + " out of Twenty", 96,96,96, true);

              graphics.Print(0, 102, "[Number of Deaths]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 114,help.String(game.deathcounts),  96,96,96, true);

              graphics.Print(0, 152, "[Time Taken]", 196, 196, 255 - help.glow, true);
              graphics.Print(0, 164, game.timestring(),  96, 96, 96, true);
          }
        }
        break;
    case 3:
        graphics.Print(30, 220, "MAP", 64,64,64);
        if (game.insecretlab)
        {
            graphics.Print(103, 220, "GRAV", 64, 64, 64);
        }
        else if (obj.flags[67] && !map.custommode)
        {
            graphics.Print(103, 220, "SHIP", 64,64,64);
        }
        else
        {
            graphics.Print(103, 220, "CREW", 64,64,64);
        }
        graphics.Print(185-4, 220, "STATS", 64,64,64);
        graphics.Print(258 - 8, 220, "[SAVE]", 196, 196, 255 - help.glow);

        if (game.inintermission)
        {
            graphics.Print(0, 115, "Cannot Save in Level Replay", 146, 146, 180, true);
        }
        else if (game.nodeathmode)
        {
            graphics.Print(0, 115, "Cannot Save in No Death Mode", 146, 146, 180, true);
        }
        else if (game.intimetrial)
        {
            graphics.Print(0, 115, "Cannot Save in Time Trial", 146, 146, 180, true);
        }
        else if (game.insecretlab)
        {
            graphics.Print(0, 115, "Cannot Save in Secret Lab", 146, 146, 180, true);
        }
        else if (map.custommode)
        {
            if (game.gamesaved)
            {
                graphics.Print(0, 36, "Game saved ok!", 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2), true);

                graphics.drawpixeltextbox(25, 65, 270, 90, 34,12, 65, 185, 207,0,4);

                if (graphics.flipmode)
                {
                    graphics.Print(0, 122, game.customleveltitle, 25, 255 - (help.glow / 2), 255 - (help.glow / 2), true);
                    graphics.Print(160 - 84, 78, game.savetime, 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));
                    graphics.Print(160 + 40, 78, help.number(game.savetrinkets), 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));

                    graphics.drawspritesetcol(50, 74, 50, 18);
                    graphics.drawspritesetcol(175, 74, 22, 18);
                }
                else
                {
                    graphics.Print(0, 90, game.customleveltitle, 25, 255 - (help.glow / 2), 255 - (help.glow / 2), true);
                    graphics.Print(160 - 84, 132, game.savetime, 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));
                    graphics.Print(160 + 40, 132, help.number(game.savetrinkets), 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));

                    graphics.drawspritesetcol(50, 126, 50, 18);
                    graphics.drawspritesetcol(175, 126, 22, 18);
                }
            }
            else
            {
                graphics.Print(0, 80, "[Press ACTION to save your game]", 255 - (help.glow * 2), 255 - (help.glow * 2), 255 - help.glow, true);
            }
        }
        else
        {
            if (graphics.flipmode)
            {
                graphics.Print(0, 186, "(Note: The game is autosaved", 146, 146, 180, true);
                graphics.Print(0, 174, "at every teleporter.)", 146, 146, 180, true);
            }
            else
            {
                graphics.Print(0, 174, "(Note: The game is autosaved", 146, 146, 180, true);
                graphics.Print(0, 186, "at every teleporter.)", 146, 146, 180, true);
            }

            if (game.gamesaved)
            {
                graphics.Print(0, 36, "Game saved ok!", 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2), true);

                graphics.drawpixeltextbox(25, 65, 270, 90, 34,12, 65, 185, 207,0,4);

                if (graphics.flipmode)
                {
                    graphics.Print(0, 132, game.savearea, 25, 255 - (help.glow / 2), 255 - (help.glow / 2), true);
                    for (int i = 0; i < 6; i++)
                    {
                        graphics.drawcrewman(169-(3*42)+(i*42), 98, i, game.crewstats[i], true);
                    }
                    graphics.Print(160 - 84, 78, game.savetime, 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));
                    graphics.Print(160 + 40, 78, help.number(game.savetrinkets), 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));

                    graphics.drawspritesetcol(50, 74, 50, 18);
                    graphics.drawspritesetcol(175, 74, 22, 18);
                }
                else
                {
                    graphics.Print(0, 80, game.savearea, 25, 255 - (help.glow / 2), 255 - (help.glow / 2), true);
                    for (int i = 0; i < 6; i++)
                    {
                        graphics.drawcrewman(169-(3*42)+(i*42), 95, i, game.crewstats[i], true);
                    }
                    graphics.Print(160 - 84, 132, game.savetime, 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));
                    graphics.Print(160 + 40, 132, help.number(game.savetrinkets), 255 - (help.glow / 2), 255 - (help.glow / 2), 255 - (help.glow / 2));

                    graphics.drawspritesetcol(50, 126, 50, 18);
                    graphics.drawspritesetcol(175, 126, 22, 18);
                }
            }
            else
            {
                graphics.Print(0, 80, "[Press ACTION to save your game]", 255 - (help.glow * 2), 255 - (help.glow * 2), 255 - help.glow, true);

                if (game.quicksummary != "")
                {
                    if (graphics.flipmode)
                    {
                        graphics.Print(0, 110, "Last Save:", 164 - (help.glow / 4), 164 - (help.glow / 4), 164, true);
                        graphics.Print(0, 100, game.quicksummary, 164  - (help.glow / 4), 164 - (help.glow / 4), 164, true);
                    }
                    else
                    {
                        graphics.Print(0, 100, "Last Save:", 164 - (help.glow / 4), 164 - (help.glow / 4), 164, true);
                        graphics.Print(0, 110, game.quicksummary, 164  - (help.glow / 4), 164 - (help.glow / 4), 164, true);
                    }
                }
            }
        }
        break;
    case 10:
        graphics.Print(128, 220, "[ QUIT ]", 196, 196, 255 - help.glow);

        if (graphics.flipmode)
        {
            if (game.intimetrial || game.insecretlab || game.nodeathmode || game.menukludge)
            {
                graphics.Print(0, 135, "Return to main menu?", 196, 196, 255 - help.glow, true);
            }
            else
            {
                graphics.Print(0, 142, "Do you want to quit? You will", 196, 196, 255 - help.glow, true);
                graphics.Print(0, 130, "lose any unsaved progress.", 196, 196, 255 - help.glow, true);
            }

            graphics.Print(80-16, 88, "[ NO, KEEP PLAYING ]", 196, 196, 255 - help.glow);
            graphics.Print(80 + 32, 76, "yes, quit to menu",  96, 96, 96);
        }
        else
        {

            if (game.intimetrial || game.insecretlab || game.nodeathmode || game.menukludge)
            {
                graphics.Print(0, 80, "Return to main menu?", 196, 196, 255 - help.glow, true);
            }
            else
            {
                graphics.Print(0, 76, "Do you want to quit? You will", 196, 196, 255 - help.glow, true);
                graphics.Print(0, 88, "lose any unsaved progress.", 196, 196, 255 - help.glow, true);
            }

            graphics.Print(80-16, 130, "[ NO, KEEP PLAYING ]", 196, 196, 255 - help.glow);
            graphics.Print(80 + 32, 142, "yes, quit to menu",  96, 96, 96);

        }
        break;
    case 11:
        graphics.Print(128, 220, "[ QUIT ]", 196, 196, 255 - help.glow);

        if (graphics.flipmode)
        {
            if (game.intimetrial || game.insecretlab || game.nodeathmode || game.menukludge)
            {
                graphics.Print(0, 135, "Return to main menu?", 196, 196, 255 - help.glow, true);
            }
            else
            {
                graphics.Print(0, 142, "Do you want to quit? You will", 196, 196, 255 - help.glow, true);
                graphics.Print(0, 130, "lose any unsaved progress.", 196, 196, 255 - help.glow, true);
            }

            graphics.Print(80, 88, "no, keep playing", 96,96,96);
            graphics.Print(80+32-16, 76, "[ YES, QUIT TO MENU ]",  196, 196, 255 - help.glow);
        }
        else
        {
            if (game.intimetrial || game.insecretlab || game.nodeathmode || game.menukludge)
            {
                graphics.Print(0, 80, "Return to main menu?", 196, 196, 255 - help.glow, true);
            }
            else
            {
                graphics.Print(0, 76, "Do you want to quit? You will", 196, 196, 255 - help.glow, true);
                graphics.Print(0, 88, "lose any unsaved progress.", 196, 196, 255 - help.glow, true);
            }

            graphics.Print(80, 130, "no, keep playing", 96,96,96);
            graphics.Print(80+32-16, 142, "[ YES, QUIT TO MENU ]", 196, 196, 255 - help.glow);
        }
        break;
    case 20:
        graphics.Print(128, 220, "[ GRAVITRON ]", 196, 196, 255 - help.glow, true);

        if (graphics.flipmode)
        {
            graphics.Print(0, 76, "the secret laboratory?", 196, 196, 255 - help.glow, true);
            graphics.Print(0, 88, "Do you want to return to", 196, 196, 255 - help.glow, true);
            graphics.Print(80-16, 142, "[ NO, KEEP PLAYING ]", 196, 196, 255 - help.glow);
            graphics.Print(80 + 32, 130, "yes, return",  96, 96, 96);
        }
        else
        {
            graphics.Print(0, 76, "Do you want to return to", 196, 196, 255 - help.glow, true);
            graphics.Print(0, 88, "the secret laboratory?", 196, 196, 255 - help.glow, true);
            graphics.Print(80-16, 130, "[ NO, KEEP PLAYING ]", 196, 196, 255 - help.glow);
            graphics.Print(80 + 32, 142, "yes, return",  96, 96, 96);
        }

        break;
    case 21:
        graphics.Print(128, 220, "[ GRAVITRON ]", 196, 196, 255 - help.glow, true);

        if (graphics.flipmode)
        {
            graphics.Print(0, 76, "the secret laboratory?", 196, 196, 255 - help.glow, true);
            graphics.Print(0, 88, "Do you want to return to", 196, 196, 255 - help.glow, true);
            graphics.Print(80, 142, "no, keep playing", 96, 96, 96);
            graphics.Print(80 + 32-16, 130, "[ YES, RETURN ]",  196, 196, 255 - help.glow);
        }
        else
        {
            graphics.Print(0, 76, "Do you want to return to", 196, 196, 255 - help.glow, true);
            graphics.Print(0, 88, "the secret laboratory?", 196, 196, 255 - help.glow, true);
            graphics.Print(80, 130, "no, keep playing", 96, 96, 96);
            graphics.Print(80 + 32-16, 142, "[ YES, RETURN ]",  196, 196, 255 - help.glow);
        }

    }




    graphics.drawfade();

    if (graphics.resumegamemode)
    {
        graphics.menuoffset += 25;
        if (map.extrarow)
        {
            if (graphics.menuoffset >= 230)
            {
                graphics.menuoffset = 230;
                //go back to gamemode!
                game.mapheld = true;
                game.gamestate = GAMEMODE;
            }
        }
        else
        {
            if (graphics.menuoffset >= 240)
            {
                graphics.menuoffset = 240;
                //go back to gamemode!
                game.mapheld = true;
                game.gamestate = GAMEMODE;
            }
        }
        graphics.menuoffrender();
    }
    else if (graphics.menuoffset > 0)
    {
        graphics.menuoffset -= 25;
        if (graphics.menuoffset < 0) graphics.menuoffset = 0;
        graphics.menuoffrender();
    }
    else
    {
        graphics.render();
    }
}

void teleporterrender()
{
    int tempx;
    int tempy;
    //draw screen alliteration
    //Roomname:
    int temp = map.area(game.roomx, game.roomy);
    if (temp < 2 && !map.custommode && graphics.fademode==0)
    {
        if (game.roomx >= 102 && game.roomx <= 104 && game.roomy >= 110 && game.roomy <= 111)
        {
            graphics.Print(5, 2, "The Ship", 196, 196, 255 - help.glow, true);
        }
        else
        {
            graphics.Print(5, 2, "Dimension VVVVVV", 196, 196, 255 - help.glow, true);
        }
    }
    else
    {
        graphics.Print(5, 2, map.roomname, 196, 196, 255 - help.glow, true);
    }

    //Background color
    FillRect(graphics.backBuffer, 0, 12, 320, 240, 10, 24, 26);

    //draw the map image
    if (map.custommode) {
        graphics.drawcustompixeltextbox(35+map.custommmxoff, 16+map.custommmyoff, map.custommmxsize+10, map.custommmysize+10, (map.custommmxsize+10)/8, (map.custommmysize+10)/8, 65, 185, 207,4,0);
        graphics.drawpartimage(12, 40+map.custommmxoff, 21+map.custommmyoff, map.custommmxsize, map.custommmysize);
    } else {
        graphics.drawpixeltextbox(35, 16, 250, 190, 32,24, 65, 185, 207,4,0);
        graphics.drawimage(1, 40, 21, false);
    }
    //black out areas we can't see yet
    if (!map.custommode) {
        for (int j = 0; j < 20; j++)
        {
            for (int i = 0; i < 20; i++)
            {
                if(map.explored[i+(j*20)]==0)
                {
                    //graphics.drawfillrect(10 + (i * 12), 21 + (j * 9), 12, 9, 16, 16, 16);
                    graphics.drawimage(2, 40 + (i * 12), 21 + (j * 9), false);
                }
            }
        }
    } else if (!map.nofog) {
        if (map.customzoom==4) {
            for (int j = 0; j < map.customheight; j++){
            for (int i = 0; i < map.customwidth; i++){
                if(map.explored[i + j*ed.maxwidth]==0){
                //Draw the fog of war on the map
                graphics.drawimage(2, map.custommmxoff+40 + (i * 48), map.custommmyoff+21 + (j * 36), false);
                graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48), map.custommmyoff+21 + (j * 36), false);
                graphics.drawimage(2, map.custommmxoff+40 + (i * 48), map.custommmyoff+21 + 9 + (j * 36), false);
                graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48), map.custommmyoff+21 + 9+ (j * 36), false);

                graphics.drawimage(2, map.custommmxoff+40 + (i * 48) + 24, map.custommmyoff+21 + (j * 36), false);
                graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48) + 24, map.custommmyoff+21 + (j * 36), false);
                graphics.drawimage(2, map.custommmxoff+40 + (i * 48) + 24, map.custommmyoff+ 21 + 9 + (j * 36), false);
                graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48) + 24, map.custommmyoff+21 + 9+ (j * 36), false);

                graphics.drawimage(2, map.custommmxoff+40 + (i * 48), map.custommmyoff+21 + (j * 36)+18, false);
                graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48), map.custommmyoff+21 + (j * 36)+18, false);
                graphics.drawimage(2, map.custommmxoff+40 + (i * 48), map.custommmyoff+21 + 9 + (j * 36)+18, false);
                graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48), map.custommmyoff+21 + 9+ (j * 36)+18, false);

                graphics.drawimage(2, map.custommmxoff+40 + (i * 48) + 24, map.custommmyoff+21 + (j * 36)+18, false);
                graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48) + 24, map.custommmyoff+21 + (j * 36)+18, false);
                graphics.drawimage(2, map.custommmxoff+40 + (i * 48) + 24, map.custommmyoff+21 + 9 + (j * 36)+18, false);
                graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 48) + 24, map.custommmyoff+21 + 9+ (j * 36)+18, false);
                }
            }
            }
        }else if(map.customzoom==2){
            for (int j = 0; j < map.customheight; j++){
            for (int i = 0; i < map.customwidth; i++){
                if(map.explored[i + j*ed.maxwidth]==0){
                //Draw the fog of war on the map
                graphics.drawimage(2, map.custommmxoff+40 + (i * 24), map.custommmyoff+21 + (j * 18), false);
                graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 24), map.custommmyoff+21 + (j * 18), false);
                graphics.drawimage(2, map.custommmxoff+40 + (i * 24), map.custommmyoff+21 + 9 + (j * 18), false);
                graphics.drawimage(2, map.custommmxoff+40 + 12 + (i * 24), map.custommmyoff+21 + 9+ (j * 18), false);
                }
            }
            }
        }else{
            for (int j = 0; j < map.customheight; j++){
            for (int i = 0; i < map.customwidth; i++){
                if(map.explored[i + j*ed.maxwidth]==0){
                //Draw the fog of war on the map
                graphics.drawimage(2, map.custommmxoff+40 + (i * 12), map.custommmyoff+21 + (j * 9), false);
                }
            }
            }
        }
    }

    //draw the coordinates //current
    if (game.roomx == 109 && !map.custommode)
    {
        //tower!instead of room y, scale map.ypos
        graphics.drawrect(40 + ((game.roomx - 100) * 12) + 2, 21  + 2, 12 - 4, 180 - 4, 16, 245 - (help.glow * 2), 245 - (help.glow * 2));
    }
    else
    {
        if (map.custommode && map.customzoom == 4)
            graphics.drawrect(map.custommmxoff+40 + ((game.roomx - 100) * 48) + 2, map.custommmyoff+21 + ((game.roomy - 100) * 36) + 2, 48 - 4, 36 - 4, 16, 245 - (help.glow * 2), 245 - (help.glow * 2));
        else if (map.custommode && map.customzoom == 2)
            graphics.drawrect(map.custommmxoff+40 + ((game.roomx - 100) * 24) + 2, map.custommmyoff+21 + ((game.roomy - 100) * 18) + 2, 24 - 4, 18 - 4, 16, 245 - (help.glow * 2), 245 - (help.glow * 2));
        else if (map.custommode)
            graphics.drawrect(map.custommmxoff+40 + ((game.roomx - 100) * 12) + 2, map.custommmyoff+21 + ((game.roomy - 100) * 9) + 2, 12 - 4, 9 - 4, 16, 245 - (help.glow * 2), 245 - (help.glow * 2));
        else
            graphics.drawrect(40 + ((game.roomx - 100) * 12) + 2, 21 + ((game.roomy - 100) * 9) + 2, 12 - 4, 9 - 4, 16, 245 - (help.glow * 2), 245 - (help.glow * 2));
    }

    if (game.useteleporter)
    {
        //Draw the chosen destination coordinate!
        //TODO
        //draw the coordinates //destination
        int tempx = map.teleporters[game.teleport_to_teleporter].x;
        int tempy = map.teleporters[game.teleport_to_teleporter].y;
        if (map.custommode && map.customzoom == 4) {
            graphics.drawrect(map.custommmxoff+40 + (tempx * 48) + 1, map.custommmyoff+21 + (tempy * 36) + 1, 48 - 2, 36 - 2, 245 - (help.glow * 2), 16, 16);
            graphics.drawrect(map.custommmxoff+40 + (tempx * 48) + 3, map.custommmyoff+21 + (tempy * 36) + 3, 48 - 6, 36 - 6, 245 - (help.glow * 2), 16, 16);
        } else if (map.custommode && map.customzoom == 2) {
            graphics.drawrect(map.custommmxoff+40 + (tempx * 24) + 1, map.custommmyoff+21 + (tempy * 18) + 1, 24 - 2, 18 - 2, 245 - (help.glow * 2), 16, 16);
            graphics.drawrect(map.custommmxoff+40 + (tempx * 24) + 3, map.custommmyoff+21 + (tempy * 18) + 3, 24 - 6, 18 - 6, 245 - (help.glow * 2), 16, 16);
        } else if (map.custommode) {
            graphics.drawrect(map.custommmxoff+40 + (tempx * 12) + 1, map.custommmyoff+21 + (tempy * 9) + 1, 12 - 2, 9 - 2, 245 - (help.glow * 2), 16, 16);
            graphics.drawrect(map.custommmxoff+40 + (tempx * 12) + 3, map.custommmyoff+21 + (tempy * 9) + 3, 12 - 6, 9 - 6, 245 - (help.glow * 2), 16, 16);
        } else {
            graphics.drawrect(40 + (tempx * 12) + 1, 21 + (tempy * 9) + 1, 12 - 2, 9 - 2, 245 - (help.glow * 2), 16, 16);
            graphics.drawrect(40 + (tempx * 12) + 3, 21 + (tempy * 9) + 3, 12 - 6, 9 - 6, 245 - (help.glow * 2), 16, 16);
        }
    }

    //draw legend details
    for (size_t i = 0; i < map.teleporters.size(); i++)
    {
        if (map.showteleporters && map.explored[map.teleporters[i].x + (ed.maxwidth * map.teleporters[i].y)] > 0)
        {
            temp = 1126 + map.explored[map.teleporters[i].x + (ed.maxwidth * map.teleporters[i].y)];
            if (graphics.flipmode) temp += 3;
            if (map.custommode && map.customzoom == 4)
                graphics.drawtile(map.custommmxoff+40 + (map.teleporters[i].x * 48) + 21, map.custommmyoff+20 + (map.teleporters[i].y * 36) + 15, temp);
            else if (map.custommode && map.customzoom == 2)
                graphics.drawtile(map.custommmxoff+40 + (map.teleporters[i].x * 24) + 9, map.custommmyoff+20 + (map.teleporters[i].y * 18) + 6, temp);
            else if (map.custommode)
                graphics.drawtile(map.custommmxoff+40 + 3 + (map.teleporters[i].x * 12), map.custommmyoff+22 + (map.teleporters[i].y * 9), temp);
            else
                graphics.drawtile(40 + 3 + (map.teleporters[i].x * 12), 22 + (map.teleporters[i].y * 9), temp);
        }
        else if(map.showtargets && map.explored[map.teleporters[i].x+(ed.maxwidth*map.teleporters[i].y)]==0)
        {
            temp = 1126 + map.explored[map.teleporters[i].x + (ed.maxwidth * map.teleporters[i].y)];
            if (graphics.flipmode) temp += 3;
            if (map.custommode && map.customzoom == 4)
                graphics.drawtile(map.custommmxoff+40 + (map.teleporters[i].x * 48) + 21, map.custommmyoff+20 + (map.teleporters[i].y * 36) + 15, temp);
            else if (map.custommode && map.customzoom == 2)
                graphics.drawtile(map.custommmxoff+40 + (map.teleporters[i].x * 24) + 9, map.custommmyoff+20 + (map.teleporters[i].y * 18) + 6, temp);
            else if (map.custommode)
                graphics.drawtile(map.custommmxoff+40 + 3 + (map.teleporters[i].x * 12), map.custommmyoff+22 + (map.teleporters[i].y * 9), temp);
            else
                graphics.drawtile(40 + 3 + (map.teleporters[i].x * 12), 22 + (map.teleporters[i].y * 9), temp);
        }
    }

    if (map.showtrinkets)
    {
        for (size_t i = 0; i < map.shinytrinkets.size(); i++)
        {
            if (!obj.collect[i])
            {
                temp = 1086;
                if (graphics.flipmode) temp += 3;
                graphics.drawtile(40 + 3 + (map.shinytrinkets[i].x * 12), 22 + (map.shinytrinkets[i].y * 9),	temp);
            }
        }
    }

    tempx = map.teleporters[game.teleport_to_teleporter].x;
    tempy = map.teleporters[game.teleport_to_teleporter].y;
    if (game.useteleporter && ((help.slowsine%16)>8))
    {
        //colour in the legend
        temp = 1128;
        if (graphics.flipmode) temp += 3;
        if (map.custommode && map.customzoom == 4)
            graphics.drawtile(map.custommmxoff+40 + (tempx * 48) + 21, map.custommmyoff+20 + (tempy * 36) + 15, temp);
        else if (map.custommode && map.customzoom == 2)
            graphics.drawtile(map.custommmxoff+40 + (tempx * 24) + 9, map.custommmyoff+20 + (tempy * 18) + 6, temp);
        else if (map.custommode)
            graphics.drawtile(map.custommmxoff+40 + 3 + (tempx * 12), map.custommmyoff+22 + (tempy * 9), temp);
        else
            graphics.drawtile(40 + 3 + (tempx * 12), 22 + (tempy * 9), temp);
    }

    graphics.cutscenebars();


    if (game.useteleporter)
    {
        //Instructions!
        graphics.Print(5, 210, "Press Left/Right to choose a Teleporter", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
        graphics.Print(5, 225, "Press ENTER to Teleport", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
    }

    graphics.drawgui();

    if (graphics.flipmode)
    {
        if (game.advancetext) graphics.bprint(5, 228, "- Press ACTION to advance text -", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
    }
    else
    {
        if (game.advancetext) graphics.bprint(5, 5, "- Press ACTION to advance text -", 220 - (help.glow), 220 - (help.glow), 255 - (help.glow / 2), true);
    }


    if (graphics.resumegamemode)
    {
        graphics.menuoffset += 25;
        if (map.extrarow)
        {
            if (graphics.menuoffset >= 230)
            {
                graphics.menuoffset = 230;
                //go back to gamemode!
                game.mapheld = true;
                game.gamestate = GAMEMODE;
            }
        }
        else
        {
            if (graphics.menuoffset >= 240)
            {
                graphics.menuoffset = 240;
                //go back to gamemode!
                game.mapheld = true;
                game.gamestate = GAMEMODE;
            }
        }
        graphics.menuoffrender();
    }
    else if (graphics.menuoffset > 0)
    {
        graphics.menuoffset -= 25;
        if (graphics.menuoffset < 0) graphics.menuoffset = 0;
        graphics.menuoffrender();
    }
    else
    {
        graphics.render();
    }
}
