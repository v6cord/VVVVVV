A list of all the new features added to VVVVVV: Community Edition.

VVVVVV: Community Edition has accepted contributions from Misa, AllyTally, leo60228, FIQ, Stelpjo, mothbeanie, Allison Fleischer, Dav999, and Joshua.

## Version c1.0-pre2 (upcoming)

- `enablefog`/`disablefog` are now `fog(enable/disable)`

- `enableflip`/`disableflip` are now `toggleflip(enable/disable)`

- `enableinfiniflip`/`disableinfiniflip` are now `infiniflip(enable/disable)`

- `enablesuicide`/`disablesuicide` are now `suicide(enable/disable)`

- `nointerrupt`/`yesinterrupt` are now `setinterrupt(on/off)`

- `playmusicfile(file)` has been removed

- Instead of using `playmusicfile()`, simply use `playfile(file[,id])`. To get much of the additional functionality of `playmusicfile()`, use the newly-added callback `on_custom_sfx_end`, which sets `%path%`.

- Added optional third argument to `playfile()` for loop count

- `unloadscriptimages` - unload images loaded by `drawimage`

- `createdamage(x,y,w,h)` - makes a region that hurts the player like spikes

- Fixed `changeplayercolor` alias of `changeplayercolour` not working

- Added being able to use `tiles4.png`, `tiles5.png`, `tiles6.png`, etc. and using F9 to switch between them in the editor

- Fixed one-way stickiness in VCE levels

- Removed `createentity`'s color argument (6th argument is now enemy type)

- Fixed `ifvar()` not working with the less-than and greater-than operators, or their "or equal to" counterparts

- Fixed an infinite loop that could happen if you died while a 1x1 quicksand was in the process of crumbling in a vanilla level

- Removed the limit where every 21st text box in a row would not show up (the removal of this limit only applies in VCE levels)

- `reloadonetime(script)` - allow re-triggering of one-time script boxes with the given script name

- Mannequins, ZZT centipedes, and Edge Games fish now face properly

- Added box and bus enemy types

- `drawimagemasked(x,y,filename,mask_filename[,mask_x[,mask_y]])` - draw an image on the screen for one frame that uses the alpha values from another image

- `togglepause(on/off)` - toggle opening the map or exiting playtesting through enter

- Large coins (10/20/50/100) now exist, and they give you a different amount of coins depending on which size they are. You can place these with Z/X/C/V

- `ifvar()` is now `ifvar(expression,script)`

- You can now use `setcheckpoint()` like `setcheckpoint(roomx,roomy,posx,posy,flipped)`

- The game no longer crashes if you give an invalid filename to a command

- Added being able to use `sprites2.png`, `sprites3.png`, `sprites4.png`, etc. and using Ctrl+F9 to switch between them in the editor

## Version c1.0-pre1

- Removed having to use a load script to use internal scripting

- Added automatic loading of custom assets - make a folder with the same name as your level file, and put the assets in there.

- `pdelay(n)` - a `delay(n)` that doesn't lock the player's movement

- `setroomname()` - sets the roomname to the next line

- `settile(x,y,tile)` - place a tile temporarily in the room

- You can now use `text()` like `text(r,g,b,x,y,lines)` - r,g,b is 0-255 - if r,g,b is 0,0,0 only the text will show up and not the text box

- `reloadroom()` - reloads the current room

- `toceil()` - inverted `tofloor()`

- `playfile(file[,id])` - play a file as either music or a sound effect. if you specify an id, the file loops

- `stopfile(id)` - stops playing a looping audio file

- `ifnotflag(n,script)` - an inverted version of `ifflag(n,script)`

- `drawtext(x,y,r,g,b,center,type)` - draw text for one frame, the text you want to display should be on the next line - r,g,b is 0-255, *center* is `0`/`1`/`true`/`false`, *type* can be 0 for normal text, 1 for bordered text and 2 for big text; use an optional eighth argument for text scale (default is 2)

- `drawrect(x,y,w,h,r,g,b[,a])` - draw a rectangle for one frame - r,g,b,a is 0-255

- `drawimage(x,y,filename[,centered[,alpha[,background[,blend]]]])` - draw an image on the screen for one frame (*alpha* 0-255, *background* `true`/`false`, *blend* `none`/`blend`/`add`/`mod`)

- `loadimage(filename)` - add the image to the cache without actually drawing it

- `drawimagepersist(x,y,filename[,centered[,alpha[,background[,blend]]]])` - same as `drawimage()`, but it stays on screen for more than a frame - it sets `%return%` to its id, so you're able to remove it

- `removeimage(id)` - remove a persistent image

- `drawpixel(x,y,r,g,b)` - draw a pixel on the screen for one frame

- `followposition` now works for the player

- There's now an option to disable only the music

- The following limits have been removed:
  - 500 scripts
  - 500 script lines
  - 200 entities in one room
  - 100 roomtexts in one room
  - 3000 placeable entities

- `destroy(crewmates)` - destroy non-rescuable crewmates

- `destroy(customcrewmates)` - destroy rescuable crewmates

- `destroy(platformsreal)` - A version of `destroy(platforms)` that isn't bugged

- `destroy()` any of the following:
  - `enemies`
  - `trinkets`
  - `warplines`
  - `checkpoints`
  - `all`
  - `conveyors`
  - `terminals`
  - `scriptboxes`
  - `disappearingplatforms`
  - `1x1quicksand`
  - `coins`
  - `gravitytokens`
  - `roomtext`
  - `teleporter`
  - `activityzones`

- `killplayer()`

- `customquicksave()` - quicksaves the level

- `niceplay()` - use this for better area music transitions

- `inf` - like `do(n)`, but an infinite amount of times

- Added a seventh argument to `createcrewman`, if it is `flip` it spawns a flipped crewmate

- `fatal_left()` - makes the left side of the screen deadly

- `fatal_right()` - makes the right side of the screen deadly

- `fatal_top()` - makes the top of the screen deadly

- `fatal_bottom()` - makes the bottom of the screen deadly

- `ifrand(n,script)` - has a 1 in *n* chance to execute the script

- `gotocheckpoint()` - teleports the player to their last checkpoint

- Added 6th argument to `createentity` that sets the raw color

- Added 7th argument to `createentity` that sets enemy type

- Automatically pause/unpause all audio on focus change

- Instead of defaulting to gray, assume unknown colors in createcrewman/changecolor are internal ones

- Added an argument to createcrewman to set a name (instead of pink/blue/etc.), useful in combination with the above or when having multiple crewmen of the same color

- Added color aliases for all colour functions

- Added optional parameter to `play` to set the fade-in time in ms (default 3000)

- `markmap(x,y,tile)` to put a tile on any minimap coordinate

- `unmarkmap(x,y)` to undo

- `mapimage(image.png)` to use any map image

- `automapimage` to undo

- `enablefog`/`disablefog` to enable/disable hiding unexplored rooms (called fog in the code)

- All names (including `player`) now work with all functions

- Added the following enemy types:
  - Hourglasses
  - Yes Men
  - Stop signs
  - Linear Colliders
  - Red pills
  - Mannequins
  - ZZT centipedes
  - OBEY
  - Edge Games
  - Trench Warfare
  - Tomb of Mad Carew
  - Brass Sent Us Under The Top
  - Wheel of Fortune
  - TRUTH
  - Skeletons
  - Vertigo

- Fixed enemies in Warp Zone gray not being gray

- `createroomtext(x,y)` - x,y in pixels, roomtext on next line

- `createscriptbox(x,y,w,h,script)` - x,y,w,h in pixels

- Allowed placed terminals to use any sprite they want

- You can now place flipped terminals

- `markers(hide/show)` - disable/reenable `markmap()` markers

- `setspeed(speed)` - set player speed, 3 by default

- `setvelocity(velocity)` - push the player (affected by inertia)

- `pinf` - variant of `inf` to automatically `pdelay(1)` if no delay occurred

- `noautobars()` - to disable automatic cutscene bars from a script

- `finalstretch(on/off)` - toggle Final Level palette swap

- `reloadscriptboxes()` - reload script boxes without affecting entities

- `puntilbars()` - `untilbars()` with `pdelay()`

- `puntilfade()` - `untilfade()` with `pdelay()`

- You can now use the Warp Zone gray tileset in the editor

- `disableflip` - disables flipping

- `enableflip` - enables flipping

- `enableinfiniflip` - enables flipping in mid-air

- `disableinfiniflip` - disables flipping in mid-air

- `untilmusic()` - wait until the current track has finished fading in or out

- `puntilmusic()` - `untilmusic()` with `pdelay()`

- Added a label and goto system - write down `.label` on a line, then to go to it, use `.label` as a script name if you're in the same script, or write `scriptname.label` to jump to the label from another script

- `ifvce(script)` - detect if game is VVVVVV: Community Edition or not

- `ifmod(mod,script)` - checks for a mod - *mod* can be `mmmmmm` (checks if MMMMMM is installed), `mmmmmm_on`/`mmmmmm_enabled`/`mmmmmm_off`/`mmmmmm_disabled` (checks if MMMMMM is installed AND enabled/disabled) or `unifont` (checks if Unifont is installed)

- `disablesuicide` - disable pressing R

- `enablesuicide` - enable pressing R

- `customactivityzone(x,y,w,h,color,script)` OR `customactivityzone(x,y,w,h,r,g,b,script)` - x,y,w,h in pixels, color is `red`/`orange`/`yellow`/`green`/`cyan`/`blue`/`pink`/`purple` (actually pink)/`white`/`gray`, if invalid it defaults to gray, r,g,b is 0-255, prompt goes on the next line

- Fixed the 2-frame delay to execute a script when entering a room

- `position(centerx,<line>)` - horizontally center the text box around the line x=\<line>

- `position(centery,<line>)` - vertically center the text box around the line y=\<line>

- Added the Tower Hallway tileset

- Added altstates - F6 to create, F7 to delete, press A to switch between

- Added an "open level folder" option to the "player levels" screen

- `reloadcustomactivityzones()` - reload all non-terminal activity zones

- `reloadterminalactivityzones()` - reload all terminal activity zones

- `reloadactivityzones()` - reload all activity zones, regardless of what they are

- `speak_fast` - remove text box fade-in

- `speak_active_fast` - remove text box fade-in, while also removing all other text boxes

- `textboxtimer(n)` - fades out the text box after *n* frames, use this after a `speak`, `speak_active`, `speak_fast`, or `speak_active_fast`

- `befadeout()` - instantly make the screen black

- `cutscenefast()` - instantly appear cutscene bars

- `endcutscenefast()` - instantly remove cutscene bars

- `setvar(name[,contents])` - set a variable to a given argument, or to whatever is on the next line

- `addvar(name[,value])` - add or subtract a number to a given variable, or concatenate a string to a given variable, using either a given argument or whatever is on the next line

- Variable assignments (`var = contents`, `var++`, `var += 1`, `var -= 1`, `var--`)

- Built-in variables (`%deaths%`, `%player_x%`, `%player_y%`, `%gravitycontrol%`, `%room_x%`, `%room_y%`, `%player_onroof%`, `%player_onground%`, `%trinkets%`, `%crewmates%`, `%coins%`, `%battery_level%`, `%on_battery%`, `%unix_time%`, `%hhmmss_time%`)

- Upped the map size limit to 100 by 100

- `ifcrewmates(n,script)` - go to *script* if the player has rescued at least *n* crewmates

- `ifcrewmatesless(n,script)` - go to *script* if the player has rescued less than *n* crewmates

- `ifcoins(n,script)` - go to *script* if the player has collected at least *n* coins

- `ifcoinsless(n,script)` - go to *script* if the player has collected less than *n* coins

- Coins are now placeable in the editor using the ^2 tool (press Shift+2)

- Coins display in the roomname if there's any coins in the map

- Coins and trinkets no longer share IDs

- Selecting level music from the editor is no longer limited

- `ifvar(var,operator[,value],script)` - go to *script* if a variable equals/isn't equal to/is greater than/is lesser than/is greater or equal to/is lesser or equal to *value* - if *value* not given, uses the next line

- Flip tokens are now placeable in the editor using the ^1 tool (press Shift+1)

- `stop()` - stop the script and remove cutscene bars

- You can place activity zones in the editor by holding down Z while placing a script box

- You can change the speed of enemies you're placing down by using Ctrl+Period/Comma. Speeds are 0-8, but since by default *p2* is 0, the speeds are saved as -4 to 4

- `replacetiles(a,b)` - replace all instances of tile *a* with tile *b* in the room 

- Added being able to use orange as a color in `text()`

- You can now use one unpatterned Space Station color in the editor

- Enemies in the unpatterned Space Station color are now gray

- Added sub-tile positioning of edentities

- `coincounter(hide/show)`

- Raised the number of flags to 1000, now they go from 0-999

- Time trials now exist in custom levels! Your time trials stats are saved as `saves/<levelname>.vvvvvv.trial`, and you can add time trials to your level through the editor.

- `iftrial([id,]script)` - go to *script* if you're in a trial and trial's id is *id* - if *id* not given, go to *script* if you're in any trial

- `endtrial()` - ends a time trial - this is functionally identical as `gamestate(82)`.

- Added sub-tile dimensions of script boxes and activity zones

- Added one-time script boxes - hold X when placing down a script box to make it run only once

- Flip tokens now play the gravity line sound effect when touched in VCE levels

- Flip tokens now respawn upon death in VCE levels

- 1x1 quicksand now respawn upon death in VCE levels

- Terminals' activity zones are aligned properly in VCE levels

- `nointerrupt()` - prevent interrupting a script when player moves into a script box

- `yesinterrupt()` - undoes the above

- `return()` - return to the previous script and line number if you jumped to a script

- `load(script)` - load *script* without having to type `iftrinkets(0,script)`

- You can now use Minecraft-like relative tilde syntax in `gotoroom()` and `gotoposition()`

- `sayquiet()` and `replyquiet()` - same as normal `say()` and `reply()`, but without a squeak

- Added dimensions, so you can properly have more than one dimension in a custom level - they are rectangles of rooms that the game will keep you in and let you wrap around

- `gotodimension(n)` - go to dimension n

- `ifkey(key,script)` - if *key* is pressed, go to *script* - `left`, `right`, `up`, and `down` count controllers and WASD, so to get ONLY the arrow keys, use `rleft`, `rright`, `rup`, and `rdown`.

- `ifflipmode(script)` - go to *script* if the game is in flip mode

- `delchar(var,n)` - remove *n* chars from the end of the variable *var*

- `getvar(var1[,var2])` - set *var1* to contents of *var2*, if *var2* is not given it uses the next line

- `analogue(on/off)` - toggle Analogue Mode screen filter

- To go to script without adding a stack frame, prefix it with `@`

- Added callbacks - `on_death_start`, `on_death_end`, `on_input_flip`, `on_input_flip_up`, `on_input_flip_down`

- `setcallback(callback,script)` - run *script* when callback is triggered

- One-way tiles are now automatically recolored to match the tileset

- `setblendmode(mode)` - set the blend mode used for drawing script images, *mode* is `blend`/`none`/`add`/`mod`, `blend` is default

- `csay([lines[,crewmate]])` - `say()` but it puts the dialogue above `createcrewman` crewmates instead

- `csayquiet([lines[,crewmate]])` - `sayquiet()` but it puts the dialogue above `createcrewman` crewmates instead

- `gamestatedelay(n)` - delay running a gamestate for *n* frames

- `randchoose(varname,options...)` - randomly set *varname* to any of the given arguments afterwards, with equal probability for each

- `randrange(varname[,start],end)` - randomly set *varname* to the range between *start* and *end* excluding *end*, *start* defaults to 0

- `moveplayersafe(x,y)` - `moveplayer` without going through walls

- `supercrewmate(on/off)` - toggle whether the Intermission 1 crewmate collides with hazards or not

- `supercrewmateroom()` - set the progress of the super crewmate to the current room

- `moveplayer()` is instant in VCE levels

- You can place teleporters using the ^3 tool (press Shift+3)

- Added towers - press F8 to toggle tower mode - use plus and minus to go up and down

- `setentitydata(id,attribute,value)` - set attribute of entity to *value* - *id* is slot number in array of entities and `createentity()` will set `%return%` to ID of newly-created entity, *attribute* is internal VVVVVV attribute name

- `getentitydata(id,attribute,varname)` - set *varname* to value of an entity's attribute - *id* is slot number in array of entities and `createentity()` will set `%return%` to ID of newly-created entity, *attribute* is internal VVVVVV attribute name
