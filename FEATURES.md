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

- `ifnotflag()` - an inverted version of `ifflag()`. `customifnotflag` is the internal counterpart

- `drawtext(x,y,r,g,b,center)` - draw text for one frame. The text you want to display should be after the command. `center` should be either 0 or 1.

- `followposition` now works for the player

- There's now an option to disable only the music

- All limits have been removed except for the 20x20 map size limit

- `destroy(platformsreal)` - A version of `destroy(platforms)` that isn't bugged

- `destroy(enemies)`

- `destroy(trinkets)`

- `destroy(warplines)`

- `destroy(checkpoints)`

- `destroy(all)`

- `destroy(conveyors)`

- `destroy(terminals)`

- `destroy(scriptboxes)`

- `destroy(disappearingplatforms)`

- `destroy(1x1quicksand)`

- `destroy(coins)`

- `destroy(gravitytokens)`

- `destroy(roomtext)`

- `destroy(crewmates)` - destroy non-rescuable crewmates

- `destroy(customcrewmates)` - destroy rescuable crewmates

- `destroy(teleporter)`

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

- Add 6th argument to `createentity` that sets the color (createentity crewmen numbers)

- Add 7th argument to `createentity` that sets entity 1/56 type (ved numbers)

- Automatically pause/unpause all audio on focus change

- Instead of defaulting to gray, assume unknown colors in createcrewman/changecolor are internal ones

- Add an argument to createcrewman to set a name (instead of pink/blue/etc.), useful in combination with the above or when having multiple crewmen of the same color

- Add color aliases for all colour functions

- Add optional parameter to `play` to set the fade-in time in ms (default 3000)

- `markmap(x, y, tile)` to put a tile on any minimap coordinate

- `unmarkmap(x, y)` to undo

- `mapimage(path/name.png)` to use any map image

- `automapimage` to undo

- `enablefog`/`disablefog` to enable/disable hiding rooms (called fog in the code)

- Automatic CI builds for all 3 OSes

- All names (including `player`) now work with all functions

- Add hourglasses as enemy type 10

- Fixed enemies in Warp Zone gray not being gray
