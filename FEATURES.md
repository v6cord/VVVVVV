A list of all the new features added to VVVVVV: Community Edition. This list is updated with each major release.

VVVVVV: Community Edition has accepted contributions from Info Teddy, AllyTally, leo60228, FIQ, and Stelpjo.

## Version c1.0
- Added UTF-8 support

- Removed having to use a load script to use internal scripting

- `pdelay(n)` - a `delay(n)` that doesn't lock the players movement

- `setroomname()` - sets the roomname to the next line

- `settile(x,y,tile)` - place a tile temporarily in the room

- `textcolo(u)r(r,g,b,x,y,lines)` - `text()` but you can set the color directly

- `reloadroom()` - reloads the current room

- `toceil()` - inverted `tofloor()`

- `movetoroom(x,y)` - a relative `gotoroom()`

- `playfile(file[, id])` - play a file as either music or a sound effect. if you specify an id, the file loops

- `stopfile(id)` - stops playing a looping audio file

- `ifnotflag()` - an inverted version of `ifflag()`

- `drawtext(x,y,r,g,b,center)` - draw text for one frame. The text you want to display should be after the command. `center` should be either 0 or 1.

- `followposition` now works for the player

- There's now an option to disable only the music

- All limits have been removed except for the 20x20 map size limit

- `destroy(crewmates)` - destroy non-rescuable crewmates

- `destroy(customcrewmates)` - destroy rescuable crewmates

- `destroy(platformsreal)` - A version of `destroy(platforms)` that isn't bugged

- `destroy(enemies|trinkets|warplines|checkpoints|all|conveyors|terminals|scriptboxes|disappearingplatforms|1x1quicksand|coins|gravitytokens|roomtext|teleporter|activityzones)`

- `killplayer()`

- `customquicksave()`

- `niceplay()` - use this for better area music transitions

- `inf` - like `do(x)`, but an infinite amount of times

- Added a seventh argument to `createcrewman`, if it is `flip` it spawns a flipped crewmate

- `fatal_left()` - Makes the left side of the screen deadly

- `fatal_right()` - Makes the right side of the screen deadly

- `fatal_top()` - Makes the top of the screen deadly

- `fatal_bottom()` - Makes the bottom of the screen deadly

- `ifrand(n,script)` - Has a 1 in *n* chance to execute the script

- `gotocheckpoint()` - Teleports the player to their last checkpoint

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

- Edentity conveyors no longer have tile 1s underneath them

- Added a label and goto system - write down `$label$` on a line, then to go to it, use `$label$` as a script name if you're in the same script, or write `scriptname$label$` to jump to the label from another script

- `ifvce()` - detect if game is VVVVVV: Community Edition or not

- `disablesuicide` - Disable pressing R

- `enablesuicide` - Enable pressing R

- `customactivityzone(x,y,w,h,color,script)` - x,y,w,h in pixels, color is `red`/`orange`/`yellow`/`green`/`cyan`/`blue`/`pink`/`purple` (actually pink)/`white`/`gray`, if invalid it defaults to gray, prompt goes on the next line

- `customactivityzonergb(x,y,w,h,r,g,b,script)` - x,y,w,h in pixels, prompt goes on the next line

- Fixed the 2-frame delay to execute a script when entering a room

- `position(centerx,<line>)` - Horizontally center the text box around the line x=\<line>

- `position(centery,<line>)` - Vertically center the text box around the line y=\<line>
