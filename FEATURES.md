A list of all the new features added to VVVVVV: Community Edition. This list is updated with each major release.

VVVVVV: Community Edition has accepted contributions from Misa, AllyTally, leo60228, FIQ, Stelpjo, mothbeanie, Allison Fleischer, and Dav999.

## Version c1.0
- Added UTF-8 support

- Removed having to use a load script to use internal scripting

- Added automatic loading of custom assets - make a folder with the same name as your level file, and put the assets in there.

- `pdelay(n)` - a `delay(n)` that doesn't lock the players movement

- `setroomname()` - sets the roomname to the next line

- `settile(x,y,tile)` - place a tile temporarily in the room

- `textcolo(u)r(r,g,b,x,y,lines)` - `text()` but you can set the color directly - r,g,b is 0-255 - if r,g,b is 0,0,0 only the text will show up and not the text box

- `reloadroom()` - reloads the current room

- `toceil()` - inverted `tofloor()`

- `playfile(file[, id])` - play a file as either music or a sound effect. if you specify an id, the file loops

- `stopfile(id)` - stops playing a looping audio file

- `ifnotflag()` - an inverted version of `ifflag()`

- `drawtext(x,y,r,g,b,center,type)` - draw text for one frame, the text you want to display should be on the next line - r,g,b is 0-255, `center` is `0`/`1`/`true`/`false`, `type` can be 0 for normal text, 1 for bordered text and 2 for big text; use an optional eighth argument for text scale (default is 2)

- `drawrect(x,y,w,h,r,g,b)` - draw a rectangle for one frame - r,g,b is 0-255

- `drawimage(x,y,filename[, centered])` - draw an image on the screen for one frame

- `loadimage(filename)` - add the image to the cache without actually drawing it

- `drawpixel(x,y,r,g,b)` - draw a pixel on the screen for one frame

- `followposition` now works for the player

- There's now an option to disable only the music

- All limits have been removed

- `destroy(crewmates)` - destroy non-rescuable crewmates

- `destroy(customcrewmates)` - destroy rescuable crewmates

- `destroy(platformsreal)` - A version of `destroy(platforms)` that isn't bugged

- `destroy(enemies|trinkets|warplines|checkpoints|all|conveyors|terminals|scriptboxes|disappearingplatforms|1x1quicksand|coins|gravitytokens|roomtext|teleporter|activityzones)`

- `killplayer()`

- `customquicksave()`

- `niceplay()` - use this for better area music transitions

- `inf` - like `do(x)`, but an infinite amount of times

- Added a seventh argument to `createcrewman`, if it is `flip` it spawns a flipped crewmate

- `fatal_left()` - makes the left side of the screen deadly

- `fatal_right()` - makes the right side of the screen deadly

- `fatal_top()` - makes the top of the screen deadly

- `fatal_bottom()` - makes the bottom of the screen deadly

- `ifrand(n,script)` - has a 1 in *n* chance to execute the script

- `gotocheckpoint()` - teleports the player to their last checkpoint

- Added 6th argument to `createentity` that sets the raw color

- Added 7th argument to `createentity` that sets entity 1/56 type (Ved numbers)

- Automatically pause/unpause all audio on focus change

- Instead of defaulting to gray, assume unknown colors in createcrewman/changecolor are internal ones

- Added an argument to createcrewman to set a name (instead of pink/blue/etc.), useful in combination with the above or when having multiple crewmen of the same color

- Added color aliases for all colour functions

- Added optional parameter to `play` to set the fade-in time in ms (default 3000)

- `markmap(x,y,tile)` to put a tile on any minimap coordinate

- `unmarkmap(x,y)` to undo

- `mapimage(path/name.png)` to use any map image

- `automapimage` to undo

- `enablefog`/`disablefog` to enable/disable hiding unexplored rooms (called fog in the code)

- All names (including `player`) now work with all functions

- Added hourglasses as enemy type 10

- Fixed enemies in Warp Zone gray not being gray

- `createroomtext(x,y)` - x,y in tiles, roomtext on next line

- `createscriptbox(x,y,w,h,script)` - x,y,w,h in pixels

- Allowed placed terminals to use any sprite they want

- You can now place flipped terminals

- Add showmarkers/hidemarkers to temporarily disable markmap markers

- `setspeed(x)` for player speed, 3 by default

- `setvelocity(x)` to push the player (affected by inertia)

- `pinf` - variant of `inf` to automatically `pdelay(1)` if no delay occurred

- `nobars()` - to disable automatic cutscene bars from a script

- `finalstretchon` - turn on Final Level palette swap

- `finalstretchoff` - turn off Final Level palette swap

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

- Added a label and goto system - write down `$label$` on a line, then to go to it, use `$label$` as a script name if you're in the same script, or write `scriptname$label$` to jump to the label from another script

- `ifvce()` - detect if game is VVVVVV: Community Edition or not

- `ifmod(mod,script)` - Checks for a mod. `mod` can be `mmmmmm` (checks if MMMMMM is installed), `mmmmmm_on`/`mmmmmm_enabled`/`mmmmmm_off`/`mmmmmm_disabled` (checks if MMMMMM is installed AND enabled/disabled) or `unifont` (checks if Unifont is installed). Jump to `script` if so.

- `disablesuicide` - disable pressing R

- `enablesuicide` - enable pressing R

- `customactivityzone(x,y,w,h,color,script)` - x,y,w,h in pixels, color is `red`/`orange`/`yellow`/`green`/`cyan`/`blue`/`pink`/`purple` (actually pink)/`white`/`gray`, if invalid it defaults to gray, prompt goes on the next line

- `customactivityzonergb(x,y,w,h,r,g,b,script)` - x,y,w,h in pixels, r,g,b is 0-255, prompt goes on the next line

- Fixed the 2-frame delay to execute a script when entering a room

- `position(centerx,<line>)` - horizontally center the text box around the line x=\<line>

- `position(centery,<line>)` - vertically center the text box around the line y=\<line>

- Added the Tower tileset

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

- `setvar(name [, contents])` - set a variable to a given argument, or to whatever is on the next line

- `addvar(name [, value])` - add or subtract a number to a given variable, or concatenate a string to a given variable, using either a given argument or whatever is on the next line

- Upped the trinkets/crewmates limit to 100

- Upped the map size limit to 100 by 100

- Variable assignments (`var = contents`, `var++`, `var += 1`, `var -= 1`, `var--`)

- Built-in variables (`%deaths%`, `%player_x%`, `%player_y%`, `%trinkets%`, `%coins%`)

- `ifcoins(x)` - Run a script if the player has collected at least `x` coins

- `ifcoinsless(x)` - Run a script if the player has less than `x` coins

- Coins are now placeable in the editor using the G tool

- Coins display in the roomname if there's any coins in the map

- Coins and trinkets no longer share IDs

- Large enemies are now enemy types you can select in the editor

- Selecting level music from the editor is no longer limited

- `ifvar(var, operator[, value], script)` - Run a script if a variable equals/isn't equal to/is greater than/is lesser than/is greater or equal to/is lesser or equal to `value`. If the `value` argument isn't given, it reads the text from the next line.

- Flip tokens are now placeable in the editor using the F tool

- `stop()` - A command for convenience: stops the script, also runs `endcutscene()`

- You can place activity zones in the editor by holding down `Z` while placing a script box

- You can change the speed of enemies you're placing down by using Ctrl+Period/Comma. Speeds are 0 - 8, but since by default `p2` is `0`, the speeds are saved as `-4` to `4`

- `replacetiles(a,b)` - Replace all instances of tile `a` with tile `b` in the room 

- Added being able to use orange as a color in `text()`

- You can now use one unpatterned Space Station color in the editor

- Enemies in the unpatterned Space Station color are now gray

- Added sub-tile positioning of edentities

- `hidecoincounter()`

- `showcoincounter()`

- Raised the number of flags to 1000, now they go from 0-999

- Time trials now exist in custom levels! Your time trials stats are saved as `saves/<levelname>.vvvvvv.trial`, and you can add time trials to your level through the editor.

- `iftrial([id, ]script)` - Run a custom script if you're currently in a trial and the trial's id matches up with the `id` argument. If the `id` argument isn't passed, the script is ran if you're in any trial.

- `endtrial()` - Ends a time trial! This is functionally identical as `gamestate(82)`.

- Added sub-tile dimensions of script boxes and activity zones

- Added one-time script boxes - hold X when placing down a script box to make it run only once

- Flip tokens now respawn upon death

- 1x1 quicksand now respawn upon death

- `nointerrupt()` - prevent interrupting a script when player moves into a script box

- `yesinterrupt()` - undoes the above

- `return()` - return to the previous script and line number if you jumped to a script

- `load()` - load a custom script without having to type iftrinkets()

- You can now use Minecraft-like relative tilde syntax in `gotoroom()` and `gotoposition()`
