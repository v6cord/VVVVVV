#include <optional>
#include <algorithm>
#include <iostream>
#include <cstring>
#include "Script.h"
#include "Graphics.h"

#include "Entity.h"
#include "Music.h"
#include "KeyPoll.h"
#include "Map.h"

scriptclass::scriptclass()
{
    	//Start SDL

	//Init
	commands.resize(500);
	words.resize(40);
	txt.resize(40);

	position = 0;
	scriptlength = 0;
	scriptdelay = 0;
	running = false;
	passive = false;

	b = 0;
	g = 0;
	i = 0;
	j = 0;
	k = 0;
	loopcount = 0;
	looppoint = 0;
	r = 0;
	textx = 0;
	texty = 0;
	textcenterline = 0;
	txtnumlines = 0;

	labelnames.resize(100);
	labelpositions.resize(100);
	nlabels = 0;

	variablenames.resize(100);
	variablecontents.resize(100);

	scriptname = "";
}

void scriptclass::clearcustom(){
	customscript.clear();
}

void scriptclass::call(std::string script) {
    callstack.push_back(stackframe { .script = scriptname, .line = position });
    load(script);
}

int scriptclass::getvar(std::string n) {
	for(std::size_t i = 0; i < variablenames.size(); i++) {
		if (variablenames[i] == n) {
			return i;
		}
	}
	return -1;
}

std::string scriptclass::processvars(std::string t) {
	tempstring = "";
	tempvar = "";
	readingvar = false;
	foundvar = false;
	for (size_t i = 0; i < t.length(); i++)
	{
		currentletter = t.substr(i, 1);
		if (readingvar) {
			if (currentletter == "%") {
				readingvar = false;
				for(std::size_t i_ = 0; i_ < variablenames.size(); i_++) {
					if (variablenames[i_] == tempvar) {
						foundvar = true;
						tempstring += variablecontents[i_];
						tempvar = "";
						break;
					}
					//printf("var: %s", tempvar.c_str());
				}
				if (foundvar == false) {
					tempstring += "%" + tempvar + "%";
					tempvar = "";
				}
			} else {
				tempvar += currentletter;
			}
		}
		else if (currentletter == "%")
		{
			readingvar = true;
		}
		else
		{
			tempstring += currentletter;
		}
	}
	return tempstring;
}

void scriptclass::tokenize( std::string t )
{
	j = 0;
	tempword = "";
    words.clear();

	t = processvars(t);
    
	for (size_t i = 0; i < t.length(); i++)
	{
		currentletter = t.substr(i, 1);
		if (currentletter == "(" || currentletter == ")" || currentletter == ",")
		{
			words.push_back(tempword);
			std::transform(words[j].begin(), words[j].end(), words[j].begin(), ::tolower);
			tempword = "";
		}
		else if (currentletter == " ")
		{
			//don't do anything - i.e. strip out spaces.
		}
		else
		{
			tempword += currentletter;
		}
	}

        words.push_back(tempword);
        words.push_back("");
}

void scriptclass::run( KeyPoll& key, Graphics& dwgfx, Game& game, mapclass& map, entityclass& obj, UtilityClass& help, musicclass& music )
{
	if (scriptdelay == 0) {
		passive = false;
	}
	while(running && scriptdelay<=0 && !game.pausescript)
	{
		if (position < scriptlength)
		{
			//Let's split or command in an array of words
			tokenize(commands[position]);

			//For script assisted input
			game.press_left = false;
			game.press_right = false;
			game.press_action = false;
			game.press_map = false;

			//Ok, now we run a command based on that string
			if (words[0] == "moveplayer")
			{
				//USAGE: moveplayer(x offset, y offset)
				int player = obj.getplayer();
				obj.entities[player].xp += ss_toi(words[1]);
				obj.entities[player].yp += ss_toi(words[2]);
				scriptdelay = 1;
			}
			if (words[0] == "warpdir")
			{
        int temprx=ss_toi(words[1])-1;
        int tempry=ss_toi(words[2])-1;
        int curlevel=temprx+(ed.maxwidth*(tempry));
			  ed.level[curlevel].warpdir=ss_toi(words[3]);
			  //If screen warping, then override all that:
        dwgfx.backgrounddrawn = false;

        //Do we update our own room?
        if(game.roomx-100==temprx && game.roomy-100==tempry){
          map.warpx=false; map.warpy=false;
          if(ed.level[curlevel].warpdir==0){
            map.background = 1;
            //Be careful, we could be in a Lab or Warp Zone room...
            if(ed.level[curlevel].tileset==2){
              //Lab
              map.background = 2;
              dwgfx.rcol = ed.level[curlevel].tilecol;
            }else if(ed.level[curlevel].tileset==3){
              //Warp Zone
              map.background = 6;
            }
          }else if(ed.level[curlevel].warpdir==1){
            map.warpx=true;
            map.background=3;
            dwgfx.rcol = ed.getwarpbackground(temprx,tempry);
          }else if(ed.level[curlevel].warpdir==2){
            map.warpy=true;
            map.background=4;
            dwgfx.rcol = ed.getwarpbackground(temprx,tempry);
          }else if(ed.level[curlevel].warpdir==3){
            map.warpx=true; map.warpy=true;
            map.background = 5;
            dwgfx.rcol = ed.getwarpbackground(temprx,tempry);
          }
        }
			}
			if (words[0] == "ifwarp")
			{
				if (ed.level[ss_toi(words[1])-1+(ed.maxwidth*(ss_toi(words[2])-1))].warpdir == ss_toi(words[3]))
				{
					call("custom_"+words[4]);
					position--;
				}
			}
			if (words[0] == "destroy")
			{
				if(words[1]=="gravitylines"){
				  for(int edi=0; edi<obj.nentity; edi++){
				    if(obj.entities[edi].type==9) obj.entities[edi].active=false;
				    if(obj.entities[edi].type==10) obj.entities[edi].active=false;
				  }
				}else if(words[1]=="warptokens"){
				  for(int edi=0; edi<obj.nentity; edi++){
				    if(obj.entities[edi].type==11) obj.entities[edi].active=false;
          }
				} else if (words[1] == "platforms" || words[1] == "platformsreal") {
					// destroy(platforms) is buggy, doesn't remove platforms' blocks
					for (int edi = 0; edi < obj.nentity; edi++)
						if (obj.entities[edi].rule == 2 && obj.entities[edi].animate == 100) {
							obj.entities[edi].active = false;
							// but destroy(platformsreal) is less buggy
							if (words[1] == "platformsreal")
								obj.removeblockat(obj.entities[edi].xp, obj.entities[edi].yp);
						}

					if (words[1] == "platformsreal") {
						obj.horplatforms = false;
						obj.vertplatforms = false;
					}
				} else if (words[1] == "enemies") {
					for (int eni = 0; eni < obj.nentity; eni++)
						if (obj.entities[eni].rule == 1)
							obj.entities[eni].active = false;
				} else if (words[1] == "trinkets") {
					for (int eti = 0; eti < obj.nentity; eti++)
						if (obj.entities[eti].type == 7)
							obj.entities[eti].active = false;
				} else if (words[1] == "warplines") {
					for (int ewi = 0; ewi < obj.nentity; ewi++)
						if (obj.entities[ewi].type >= 51 && obj.entities[ewi].type <= 54)
							obj.entities[ewi].active = false;

					obj.customwarpmode = false;
					obj.customwarpmodevon = false;
					obj.customwarpmodehon = false;

					// If we had a warp background before, warp lines undid it
					switch (ed.level[game.roomx-100 + ed.maxwidth*(game.roomy-100)].warpdir) {
					case 1:
						map.warpx = true;
						break;
					case 2:
						map.warpy = true;
						break;
					case 3:
						map.warpx = true;
						map.warpy = true;
						break;
					}
				} else if (words[1] == "checkpoints") {
					for (int eci = 0; eci < obj.nentity; eci++)
						if (obj.entities[eci].type == 8)
							obj.entities[eci].active = false;
				} else if (words[1] == "all" || words[1] == "everything") {
					// Don't want to use obj.removeallblocks(), it'll remove all spikes and one-ways too
					// And also we need to remove script boxes a certain way so reloadscriptboxes() will work
					for (int bl = 0; bl < obj.nblocks; bl++)
						if (obj.blocks[bl].type == TRIGGER)
							obj.removetrigger(obj.blocks[bl].trigger);
						else if (obj.blocks[bl].type != DAMAGE && obj.blocks[bl].type != DIRECTIONAL)
							obj.blocks[bl].clear();

					// Too bad there's no obj.removeallentities()
					for (int ei = 0; ei < obj.nentity; ei++)
						if (obj.entities[ei].rule != 0) // Destroy everything except the player
							obj.entities[ei].active = false;

					// Copy-pasted from above
					obj.horplatforms = false;
					obj.vertplatforms = false;
					obj.customwarpmode = false;
					obj.customwarpmodevon = false;
					obj.customwarpmodehon = false;
					// If we had a warp background before, warp lines undid it
					switch (ed.level[game.roomx-100 + ed.maxwidth*(game.roomy-100)].warpdir) {
					case 1:
						map.warpx = true;
						break;
					case 2:
						map.warpy = true;
						break;
					case 3:
						map.warpx = true;
						map.warpy = true;
						break;
					}
				} else if (words[1] == "conveyors") {
					for (int edc = 0; edc < obj.nentity; edc++) {
						if (!obj.entities[edc].active || obj.entities[edc].type != 1 ||
						(obj.entities[edc].behave != 8 && obj.entities[edc].behave != 9))
							continue;

						for (int ii = 0; ii < obj.nblocks; ii++)
							if (obj.blocks[ii].xp == obj.entities[edc].xp && obj.blocks[ii].yp == obj.entities[edc].yp)
								obj.blocks[ii].clear();
						obj.entities[edc].active = false;

						// Important: set width and height to 0, or there will still be collision
						obj.entities[edc].w = 0;
						obj.entities[edc].h = 0;
					}
				} else if (words[1] == "terminals") {
					for (int eti = 0; eti < obj.nentity; eti++)
						if (obj.entities[eti].type == 13)
							obj.entities[eti].active = false;

					for (int bti = 0; bti < obj.nblocks; bti++)
						if (obj.blocks[bti].type == ACTIVITY &&
						(obj.blocks[bti].prompt == "Press ENTER to activate terminal" ||
						obj.blocks[bti].prompt == "Press ENTER to activate terminals"))
							obj.blocks[bti].active = false;
				} else if (words[1] == "scriptboxes") {
					for (int bsi = 0; bsi < obj.nblocks; bsi++)
						if (obj.blocks[bsi].type == TRIGGER)
							obj.removetrigger(obj.blocks[bsi].trigger);
				} else if (words[1] == "disappearingplatforms" || words[1] == "quicksand") {
					for (int epi = 0; epi < obj.nentity; epi++)
						if (obj.entities[epi].type == 2) {
							obj.entities[epi].active = false;
							obj.removeblockat(obj.entities[epi].xp, obj.entities[epi].yp);
						}
				} else if (words[1] == "1x1quicksand" || words[1] == "1x1disappearingplatforms") {
					for (int eqi = 0; eqi < obj.nentity; eqi++)
						if (obj.entities[eqi].type == 3) {
							obj.entities[eqi].active = false;
							obj.removeblockat(obj.entities[eqi].xp, obj.entities[eqi].yp);
						}
				} else if (words[1] == "coins") {
					for (int eci = 0; eci < obj.nentity; eci++)
						if (obj.entities[eci].type == 6)
							obj.entities[eci].active = false;
				} else if (words[1] == "gravitytokens" || words[1] == "fliptokens") {
					for (int egi = 0; egi < obj.nentity; egi++)
						if (obj.entities[egi].type == 4)
							obj.entities[egi].active = false;
				} else if (words[1] == "roomtext") {
					map.roomtexton = false;
					map.roomtextnumlines = 0;
				} else if (words[1] == "crewmates") {
					for (int eci = 0; eci < obj.nentity; eci++)
						if (obj.entities[eci].type == 12 || obj.entities[eci].type == 14)
							obj.entities[eci].active = false;
				} else if (words[1] == "customcrewmates") {
					for (int eci = 0; eci < obj.nentity; eci++)
						if (obj.entities[eci].type == 55)
							obj.entities[eci].active = false;
				} else if (words[1] == "teleporter" || words[1] == "teleporters") {
					for (int eti = 0; eti < obj.nentity; eti++)
						if (obj.entities[eti].type == 100)
							obj.entities[eti].active = false;

					game.activetele = false;
				} else if (words[1] == "activityzones") {
					for (int bai = 0; bai < obj.nblocks; bai++)
						if (obj.blocks[bai].type == ACTIVITY)
							obj.blocks[bai].active = false;
				}

				obj.cleanup();
				int n = obj.nblocks - 1;
				while (n >= 0 && !obj.blocks[n].active) {
					obj.nblocks--;
					n--;
				}
			}
			if (words[0] == "customiftrinkets")
			{
				if (game.trinkets >= ss_toi(words[1]))
				{
					call("custom_"+words[2]);
					position--;
				}
			}
			if (words[0] == "customiftrinketsless")
			{
				if (game.trinkets < ss_toi(words[1]))
				{
					call("custom_"+words[2]);
					position--;
				}
			}
      else if (words[0] == "customifflag")
			{
				if (obj.flags[ss_toi(words[1])]==1)
				{
					call("custom_"+words[2]);
					position--;
				}
			}
			if (words[0] == "custommap")
			{
				if(words[1]=="on"){
				  map.customshowmm=true;
				}else if(words[1]=="off"){
				  map.customshowmm=false;
				}
			}
			if (words[0] == "delay")
			{
				//USAGE: delay(frames)
				scriptdelay = ss_toi(words[1]);
                                loopdelay = true;
			}
			if (words[0] == "pdelay")
			{
				//USAGE: pdelay(frames)
				passive = true;
				scriptdelay = ss_toi(words[1]);
                                loopdelay = true;
			}
			if (words[0] == "settile")
			{
				// settile(x,y,tile)
				int x = ss_toi(words[1]);
				int y = ss_toi(words[2]);
				int tile = ss_toi(words[3]);

				if (tile != 10) // There's nothing behind 1x1 quicksand
					map.settile(x, y, tile);
				else
					// Don't change the `tile` var, we createentity by checking `tile` later
					map.settile(x, y, 0);

				dwgfx.foregrounddrawn = false;

				// Kludge to fix spikes, one-ways, and 1x1 quicksand

				// Remove them first, if there is one where we placed the tile

				// The 1x1 quicksand is part entity, part block
				// Remove its entity
				for (int eqi = 0; eqi < obj.nentity; eqi++)
					if (obj.entities[eqi].type == 3 && obj.entities[eqi].xp/8 == x && obj.entities[eqi].yp/8 == y)
						obj.entities[eqi].active = false;

				// The rest of these removals are blocks, which I'll just put in one for-loop
				for (int bi = 0; bi < obj.nblocks; bi++) {
					// Remove the block part of a 1x1 quicksand
					// We check that it's 8x8 in order to not remove 4-wide quicksand / platforms / conveyors
					if (obj.blocks[bi].type == BLOCK && obj.blocks[bi].xp/8 == x && obj.blocks[bi].yp/8 == y && obj.blocks[bi].wp == 8 && obj.blocks[bi].hp == 8)
						obj.blocks[bi].clear();

					// The spiky part of a spike is a block, remove it
					if (obj.blocks[bi].type == DAMAGE &&
					// Important - do the integer division by 8 on the left side of the equality, not multiply by 8 on the right side
					// (Same applies to the one-way conditional below)
					// Also important - for some godawful reason, the real position of spikes' blocks
					// is stored in their 'x'/'y' attributes instead of 'xp'/'yp' like the rest,
					// and also, the 'x'/'y' attributes are floats instead of ints
					(int) obj.blocks[bi].x/8 == x && (int) obj.blocks[bi].y/8 == y)
						obj.blocks[bi].clear();

					// The collision of a one-way tile is a block, also remove it
					if (obj.blocks[bi].type == DIRECTIONAL &&
					// Read comment about integer division from above conditional
					obj.blocks[bi].x/8 == x && obj.blocks[bi].y/8 == y)
						obj.blocks[bi].clear();
				}

				// Clean up before adding any entities/blocks
				obj.cleanup();
				int n = obj.nblocks - 1;
				while (n >= 0 && !obj.blocks[n].active) {
					obj.nblocks--;
					n--;
				}

				// Ok, now if we've added a tile that's an entity and/or block, actually add it now...
				// TODO: This will need to be updated when the Tower tileset rolls around

				// This is all copy-pasted from Map.cpp

				// Spikes
				if (map.tileset == 0) {
					if (tile == 6 || tile == 8)
						// Sticking up
						obj.createblock(DAMAGE, 8*x, 8*y + 4, 8, 4);
					if (tile == 7 || tile == 9)
						// Sticking down
						obj.createblock(DAMAGE, 8*x, 8*y, 8, 4);
					if (tile == 49 || tile == 50)
						// Left or right
						obj.createblock(DAMAGE, 8*x, 8*y + 3, 8, 2);
				} else if (map.tileset == 1) {
					if ((tile >= 63 && tile <= 74) || (tile >= 6 && tile <= 9)) {
						// Correct the {odd, even}-type of the tile, this will be un-corrected later
						if (tile < 10)
							tile++;

						if(tile % 2 == 0)
							// Sticking up
							obj.createblock(DAMAGE, 8*x, 8*y, 8, 4);
						else
							// Sticking down
							obj.createblock(DAMAGE, 8*x, 8*y + 4, 8, 4);

						// Un-correct it now
						if (tile < 11)
							tile--;
					}
					if (tile >= 49 && tile <= 62)
						// Left or right
						obj.createblock(DAMAGE, 8*x, 8*y + 3, 8, 2);
				}

				// 1x1 quicksand
				if (tile == 10)
					obj.createentity(game, 8*x, 8*y, 4);

				// One-way tiles
				if (tile >= 14 && tile <= 17)
					obj.createblock(DIRECTIONAL, 8*x, 8*y, 8, 8, tile-14);
			}
			if (words[0] == "setroomname")
			{
				// setroomname()
                                position++;
				map.roomname = processvars(commands[position]);
			}
			if (words[0] == "setvar")
			{
				// setvar(name, contents)
				// OR
				// setvar(name)
				// <contents>
				
				variablenames.push_back(words[1]);
				if (words[2] == "") {
    	            position++;
					variablecontents.push_back(processvars(commands[position]));
				} else {
					variablecontents.push_back(words[2]);
				}
			}
			if (words[0] == "addvar")
			{
				// addvar(name, add)
				// OR
				// addvar(name)
				// <add>
				
				int varid = getvar(words[1]);
				if (varid != -1) {
					if (words[2] == "") {
    		            position++;
						variablecontents[varid] += processvars(commands[position]);
					} else {
						if (is_number(variablecontents[varid]) && is_number(words[2])) {
							std::string tempcontents = std::to_string(stod(variablecontents[varid]) + stod(words[2]));
							tempcontents.erase ( tempcontents.find_last_not_of('0') + 1, std::string::npos );
							tempcontents.erase ( tempcontents.find_last_not_of('.') + 1, std::string::npos );
							variablecontents[varid] = tempcontents;
						} else {
							variablecontents[varid] += words[2];
						}
					}
				}
			}
			if (words[0] == "drawtext")
			{
				// drawtext(x,y,r,g,b,centered)
				scriptimage temp;
				temp.type   = 0;
				temp.x      = ss_toi(words[1]);
				temp.y      = ss_toi(words[2]);
				temp.r      = ss_toi(words[3]);
				temp.g      = ss_toi(words[4]);
				temp.b      = ss_toi(words[5]);
				temp.center = ss_toi(words[6]);
                position++;
				temp.text = processvars(commands[position]);
				scriptrender.push_back(temp);
			}
      if (words[0] == "flag")
			{
				if(ss_toi(words[1])>=0 && ss_toi(words[1])<100){
					if(words[2]=="on"){
						obj.changeflag(ss_toi(words[1]),1);
					}else if(words[2]=="off"){
						obj.changeflag(ss_toi(words[1]),0);
					}
				}
			}
			if (words[0] == "flash")
			{
				//USAGE: flash(frames)
				game.flashlight = ss_toi(words[1]);
			}
			if (words[0] == "shake")
			{
				//USAGE: shake(frames)
				game.screenshake = ss_toi(words[1]);
			}
			if (words[0] == "walk")
			{
				//USAGE: walk(dir,frames)
				if (words[1] == "left")
				{
					game.press_left = true;
				}
				else if (words[1] == "right")
				{
					game.press_right = true;
				}
				scriptdelay = ss_toi(words[2]);
			}
			if (words[0] == "flip")
			{
				game.press_action = true;
				scriptdelay = 1;
			}
			if (words[0] == "tofloor")
			{
				if(obj.entities[obj.getplayer()].onroof>0)
				{
					game.press_action = true;
					scriptdelay = 1;
				}
			}
			if (words[0] == "toceil")
			{
				if(obj.entities[obj.getplayer()].onground>0)
				{
					game.press_action = true;
					scriptdelay = 1;
				}
			}
                        if (words[0] == "markmap")
                        {
                            game.scriptmarkers.push_back(scriptmarker {
                                .x = ss_toi(words[1]),
                                .y = ss_toi(words[2]),
                                .tile = ss_toi(words[3]),
                                    });
                        }
                        if (words[0] == "unmarkmap")
                        {
                            auto x = ss_toi(words[1]);
                            auto y = ss_toi(words[2]);
                            game.scriptmarkers.erase(std::remove_if(
                                game.scriptmarkers.begin(),
                                game.scriptmarkers.end(),
                                [=](const scriptmarker& marker) {
                                    return marker.x == x && marker.y == y;
                                }
                            ));
                        }
                        if (words[0] == "hidemarkers")
                        {
                            game.hidemarkers = true;
                        }
                        if (words[0] == "showmarkers")
                        {
                            game.hidemarkers = false;
                        }
                        if (words[0] == "mapimage")
                        {
                            SDL_FreeSurface(dwgfx.images[12]);
                            dwgfx.images[12] = LoadImage(words[1].c_str());
                            dwgfx.mapimage = words[1];
                        }
                        if (words[0] == "automapimage")
                        {
                            ed.generatecustomminimap(dwgfx, map);
                            dwgfx.mapimage = std::nullopt;
                        }
                        if (words[0] == "disablefog")
                        {
                            map.nofog = true;
                        }
                        if (words[0] == "enablefog")
                        {
                            map.nofog = false;
                        }
                        if (words[0] == "finalstretchon")
                        {
                            map.finalstretch = true;
                            map.final_colormode = true;
                            map.final_colorframe = 1;
                            map.colsuperstate = 1;
                        }
                        if (words[0] == "finalstretchoff")
                        {
                            map.finalstretch = false;
                            map.final_colormode = false;
                            map.final_mapcol = 0;
                            map.colsuperstate = 0;
                            dwgfx.foregrounddrawn = false;
                        }
                        if (words[0] == "disableflip")
                        {
                            game.noflip = true;
                        }
                        if (words[0] == "enableflip")
                        {
                            game.noflip = false;
                        }
                        if (words[0] == "enableinfiniflip")
                        {
                            game.infiniflip = true;
                        }
                        if (words[0] == "disableinfiniflip")
                        {
                            game.infiniflip = false;
                        }
                        if (words[0] == "disablesuicide")
                        {
                            game.nosuicide = true;
                        }
                        if (words[0] == "enablesuicide")
                        {
                            game.nosuicide = false;
                        }
                        if (words[0] == "setspeed")
                        {
                            game.playerspeed = std::stoi(words[1]);
                        }
                        if (words[0] == "setvelocity")
                        {
                            game.nofriction = true;
                            obj.entities[obj.getplayer()].ax = std::stoi(words[1]);
                        }
			if (words[0] == "playef")
			{
				music.playef(ss_toi(words[1]), ss_toi(words[2]));
			}
			if (words[0] == "playfile")
			{
				music.playfile(words[1].c_str(), words[2]);
			}
			if (words[0] == "stopfile")
			{
				music.stopfile(words[1]);
			}
			if (words[0] == "play")
			{
                                auto fadeintime = 3000;
                                if (words[2] != "") {
                                    fadeintime = ss_toi(words[2]);
                                }
				music.play(ss_toi(words[1]), fadeintime);
			}
			if (words[0] == "niceplay")
			{
				music.niceplay(ss_toi(words[1]));
			}
			if (words[0] == "stopmusic")
			{
				music.haltdasmusik();
			}
			if (words[0] == "resumemusic")
			{
				music.play(music.resumesong);
			}
      if (words[0] == "musicfadeout")
			{
				music.fadeout();
				music.dontquickfade = true;
			}
			if (words[0] == "musicfadein")
			{
				music.musicfadein = 90;
				//if(!game.muted) music.fadeMusicVolumeIn(3000);
			}
			if (words[0] == "trinketscriptmusic")
			{
				music.play(4);
			}
			if (words[0] == "gotoposition")
			{
				//USAGE: gotoposition(x position, y position, gravity position)
				int player = obj.getplayer();
				obj.entities[player].xp = ss_toi(words[1]);
				obj.entities[player].yp = ss_toi(words[2]);
				if (words[3] != "") {
                                    game.gravitycontrol = ss_toi(words[3]);
                                } else {
                                    game.gravitycontrol = 0;
                                }

			}
			if (words[0] == "gotoroom")
			{
				//USAGE: gotoroom(x,y) (manually add 100)
				map.gotoroom(ss_toi(words[1])+100, ss_toi(words[2])+100, dwgfx, game, obj, music);
				game.gotoroomfromscript = true;
			}
			if (words[0] == "reloadroom")
			{
				//USAGE: reloadroom()
				map.gotoroom(game.roomx, game.roomy, dwgfx, game, obj, music);
				game.gotoroomfromscript = true;
			}
			if (words[0] == "movetoroom")
			{
				//USAGE: movetoroom(x,y)
				map.gotoroom(game.roomx + ss_toi(words[1]), game.roomy + ss_toi(words[2]), dwgfx, game, obj, music);
				game.gotoroomfromscript = true;
			}
			if (words[0] == "reloadscriptboxes")
			{
				for (int brs = 0; brs < obj.nresurrectblocks; brs++) {
					obj.createblock(TRIGGER, obj.resurrectblocks[brs].x, obj.resurrectblocks[brs].y, obj.resurrectblocks[brs].wp, obj.resurrectblocks[brs].hp, obj.resurrectblocks[brs].trigger);
					obj.resurrectblocks[brs].clear();
				}
				obj.nresurrectblocks = 0;
			}
			if (words[0] == "cutscene")
			{
				dwgfx.showcutscenebars = true;
			}
			if (words[0] == "endcutscene")
			{
				dwgfx.showcutscenebars = false;
			}
			if (words[0] == "untilbars" || words[0] == "puntilbars")
			{
				if (dwgfx.showcutscenebars)
				{
					if (dwgfx.cutscenebarspos < 360)
					{
						scriptdelay = 1;
						if (words[0] == "puntilbars") passive = true;
						position--;
					}
				}
				else
				{
					if (dwgfx.cutscenebarspos > 0)
					{
						scriptdelay = 1;
						if (words[0] == "puntilbars") passive = true;
						position--;
					}
				}
			}
			else if (words[0] == "text")
			{
				//oh boy
				//first word is the colour.
				if (words[1] == "cyan")
				{
					r = 164;
					g = 164;
					b = 255;
				}
				else if (words[1] == "player")
				{
					r = 164;
					g = 164;
					b = 255;
				}
				else if (words[1] == "red")
				{
					r = 255;
					g = 60;
					b = 60;
				}
				else if (words[1] == "green")
				{
					r = 144;
					g = 255;
					b = 144;
				}
				else if (words[1] == "yellow")
				{
					r = 255;
					g = 255;
					b = 134;
				}
				else if (words[1] == "blue")
				{
					r = 95;
					g = 95;
					b = 255;
				}
				else if (words[1] == "purple")
				{
					r = 255;
					g = 134;
					b = 255;
				}
				else if (words[1] == "gray")
				{
					r = 174;
					g = 174;
					b = 174;
				}
				else
				{
					//use a gray
					r = 174;
					g = 174;
					b = 174;
				}

				//next are the x,y coordinates
				textx = ss_toi(words[2]);
				texty = ss_toi(words[3]);

				//Number of lines for the textbox!
                                if (!words[4].empty()) {
                                    txtnumlines = ss_toi(words[4]);
                                } else {
                                    txtnumlines = 1;
                                }

				for (int i = 0; i < txtnumlines; i++)
				{
					position++;
					txt[i] = processvars(commands[position]);
				}
			}
			else if ((words[0] == "textcolor") || (words[0] == "textcolour"))
			{
				// set the colors
				r = ss_toi(words[1]);
				g = ss_toi(words[2]);
				b = ss_toi(words[3]);

				//next are the x,y coordinates
				textx = ss_toi(words[4]);
				texty = ss_toi(words[5]);

				//Number of lines for the textbox!
				txtnumlines = ss_toi(words[6]);
				for (int i = 0; i < txtnumlines; i++)
				{
					position++;
					txt[i] = processvars(commands[position]);
				}
			}
			else if (words[0] == "position")
			{
				//are we facing left or right? for some objects we don't care, default at 0.
				j = 0;

				//the first word is the object to position relative to
				if (words[1] == "centerx")
				{
					if (words[2] != "")
						textcenterline = ss_toi(words[2]);
					else
						textcenterline = 0;

					words[2] = "donothing";
					j = -1;
					textx = -500;
				}
				else if (words[1] == "centery")
				{
					if (words[2] != "")
						textcenterline = ss_toi(words[2]);
					else
						textcenterline = 0;

					words[2] = "donothing";
					j = -1;
					texty = -500;
				}
				else if (words[1] == "center")
				{
					words[2] = "donothing";
					j = -1;
					textx = -500;
					texty = -500;
				} else {
					i = obj.getcrewman(words[1]);
					j = obj.entities[i].dir;
                                }

				//next is whether to position above or below
				if (words[2] == "above")
				{
					if (j == 1)    //left
					{
						textx = obj.entities[i].xp -10000; //tells the box to be oriented correctly later
						texty = obj.entities[i].yp - 16 - (txtnumlines*8);
					}
					else if (j == 0)     //Right
					{
						textx = obj.entities[i].xp - 16;
						texty = obj.entities[i].yp - 18 - (txtnumlines * 8);
					}
				}
				else
				{
					if (j == 1)    //left
					{
						textx = obj.entities[i].xp -10000; //tells the box to be oriented correctly later
						texty = obj.entities[i].yp + 26;
					}
					else if (j == 0)     //Right
					{
						textx = obj.entities[i].xp - 16;
						texty = obj.entities[i].yp + 26;
					}
				}
			}
			else if (words[0] == "customposition")
			{
				//are we facing left or right? for some objects we don't care, default at 0.
				j = 0;

				//the first word is the object to position relative to
				if (words[1] == "player")
				{
					i = obj.getcustomcrewman(0);
					j = obj.entities[i].dir;
				}
				else if (words[1] == "cyan")
				{
					i = obj.getcustomcrewman(0);
					j = obj.entities[i].dir;
				}
				else if (words[1] == "purple")
				{
					i = obj.getcustomcrewman(1);
					j = obj.entities[i].dir;
				}
				else if (words[1] == "yellow")
				{
					i = obj.getcustomcrewman(2);
					j = obj.entities[i].dir;
				}
				else if (words[1] == "red")
				{
					i = obj.getcustomcrewman(3);
					j = obj.entities[i].dir;
				}
				else if (words[1] == "green")
				{
					i = obj.getcustomcrewman(4);
					j = obj.entities[i].dir;
				}
				else if (words[1] == "blue")
				{
					i = obj.getcustomcrewman(5);
					j = obj.entities[i].dir;
				}
				else if (words[1] == "centerx")
				{
					words[2] = "donothing";
					j = -1;
					textx = -500;
				}
				else if (words[1] == "centery")
				{
					words[2] = "donothing";
					j = -1;
					texty = -500;
				}
				else if (words[1] == "center")
				{
					words[2] = "donothing";
					j = -1;
					textx = -500;
					texty = -500;
				}

				if(i==0 && words[1]!="player" && words[1]!="cyan"){
				  //Requested crewmate is not actually on screen
          words[2] = "donothing";
					j = -1;
					textx = -500;
					texty = -500;
				}

				//next is whether to position above or below
				if (words[2] == "above")
				{
					if (j == 1)    //left
					{
						textx = obj.entities[i].xp -10000; //tells the box to be oriented correctly later
						texty = obj.entities[i].yp - 16 - (txtnumlines*8);
					}
					else if (j == 0)     //Right
					{
						textx = obj.entities[i].xp - 16;
						texty = obj.entities[i].yp - 18 - (txtnumlines * 8);
					}
				}
				else
				{
					if (j == 1)    //left
					{
						textx = obj.entities[i].xp -10000; //tells the box to be oriented correctly later
						texty = obj.entities[i].yp + 26;
					}
					else if (j == 0)     //Right
					{
						textx = obj.entities[i].xp - 16;
						texty = obj.entities[i].yp + 26;
					}
				}
			}
			else if (words[0] == "backgroundtext")
			{
				game.backgroundtext = true;
			}
			else if (words[0] == "flipme")
			{
				if(dwgfx.flipmode) texty += 2*(120 - texty);
			}
			else if (words[0] == "speak_active")
			{
				//Ok, actually display the textbox we've initilised now!
				dwgfx.createtextbox(txt[0], textx, texty, r, g, b);
				if (txtnumlines > 1)
				{
					for (i = 1; i < txtnumlines; i++)
					{
						dwgfx.addline(txt[i]);
					}
				}

				//the textbox cannot be outside the screen. Fix if it is.
				if (textx <= -1000)
				{
					//position to the left of the player
					textx += 10000;
					textx -= dwgfx.textboxwidth();
					textx += 16;
					dwgfx.textboxmoveto(textx);
				}

				if (textx == -500) {
					if (textcenterline != 0)
						dwgfx.textboxcenterx(textcenterline);
					else
						dwgfx.textboxcenterx(160);

					// So it doesn't use the same line but Y instead of X for texty=-500
					textcenterline = 0;
				}

				if (texty == -500) {
					if (textcenterline != 0)
						dwgfx.textboxcentery(textcenterline);
					else
						dwgfx.textboxcentery(120);

					textcenterline = 0;
				}

				textcenterline = 0;

				dwgfx.textboxadjust();
				dwgfx.textboxactive();

				if (!game.backgroundtext)
				{
					game.advancetext = true;
					game.hascontrol = false;
					game.pausescript = true;
					if (key.isDown(90) || key.isDown(32) || key.isDown(86)
						|| key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN)) game.jumpheld = true;
				}
				game.backgroundtext = false;
			}
			else if (words[0] == "speak")
			{
				//Exactly as above, except don't make the textbox active (so we can use multiple textboxes)
				dwgfx.createtextbox(txt[0], textx, texty, r, g, b);
				if (txtnumlines > 1)
				{
					for (i = 1; i < txtnumlines; i++)
					{
						dwgfx.addline(txt[i]);
					}
				}

				//the textbox cannot be outside the screen. Fix if it is.
				if (textx <= -1000)
				{
					//position to the left of the player
					textx += 10000;
					textx -= dwgfx.textboxwidth();
					textx += 16;
					dwgfx.textboxmoveto(textx);
				}

				if (textx == -500) {
					if (textcenterline != 0)
						dwgfx.textboxcenterx(textcenterline);
					else
						dwgfx.textboxcenterx();

					// So it doesn't use the same line but Y instead of X for texty=-500
					textcenterline = 0;
				}

				if (texty == -500) {
					if (textcenterline != 0)
						dwgfx.textboxcentery(textcenterline);
					else
						dwgfx.textboxcentery();

					textcenterline = 0;
				}

				textcenterline = 0;

				dwgfx.textboxadjust();
				//dwgfx.textboxactive();

				if (!game.backgroundtext)
				{
					game.advancetext = true;
					game.hascontrol = false;
					game.pausescript = true;
					if (key.isDown(90) || key.isDown(32) || key.isDown(86)
						|| key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN)) game.jumpheld = true;
				}
				game.backgroundtext = false;
			}
			else if (words[0] == "endtext")
			{
				dwgfx.textboxremove();
				game.hascontrol = true;
				game.advancetext = false;
			}
			else if (words[0] == "endtextfast")
			{
				dwgfx.textboxremovefast();
				game.hascontrol = true;
				game.advancetext = false;
			}
			else if (words[0] == "do")
			{
				//right, loop from this point
				looppoint = position;
				loopcount = ss_toi(words[1]);
			}
			else if (words[0] == "inf")
			{
				//right, loop from this point
				looppoint = position;
				loopcount = -1;
			}
			else if (words[0] == "pinf")
			{
				//right, loop from this point
				looppoint = position;
				loopcount = -2;
			}
			else if (words[0] == "loop")
			{
				//right, loop from this point
				if (loopcount > 1)
				{
                                        loopcount--;
					position = looppoint;
				} else if (loopcount < 0) {
                                        position = looppoint;
                                        if (loopcount == -2 && !loopdelay) {
                                            passive = true;
                                            scriptdelay = 1;
                                        }
                                }
                                loopdelay = false;
			}
			else if (words[0] == "vvvvvvman")
			{
				//Create the super VVVVVV combo!
				i = obj.getplayer();
				obj.entities[i].xp = 30;
				obj.entities[i].yp = 46;
				obj.entities[i].size = 13;
				obj.entities[i].colour = 23;
				obj.entities[i].cx = 36;// 6;
				obj.entities[i].cy = 12+80;// 2;
				obj.entities[i].h = 126-80;// 21;
			}
			else if (words[0] == "undovvvvvvman")
			{
				//Create the super VVVVVV combo!
				i = obj.getplayer();
				obj.entities[i].xp = 100;
				obj.entities[i].size = 0;
				obj.entities[i].colour = 0;
				obj.entities[i].cx = 6;
				obj.entities[i].cy = 2;
				obj.entities[i].h = 21;
			}
			else if (words[0] == "createentity")
			{
				auto k = obj.createentity(game, ss_toi(words[1]), ss_toi(words[2]), ss_toi(words[3]), ss_toi(words[4]), ss_toi(words[5]));
                                if (words[6] != "") {
                                    if (words[7] != "") {
                                        switch(ss_toi(words[7])) {
                                            case 0:  obj.setenemyroom(k, 4+100, 0+100); break;
                                            case 1:  obj.setenemyroom(k, 2+100, 0+100); break;
                                            case 2:  obj.setenemyroom(k, 12+100, 3+100); break;
                                            case 3:  obj.setenemyroom(k, 13+100, 12+100); break;
                                            case 4:  obj.setenemyroom(k, 16+100, 9+100); break;
                                            case 5:  obj.setenemyroom(k, 19+100, 1+100); break;
                                            case 6:  obj.setenemyroom(k, 19+100, 2+100); break;
                                            case 7:  obj.setenemyroom(k, 18+100, 3+100); break;
                                            case 8:  obj.setenemyroom(k, 16+100, 0+100); break;
                                            case 9:  obj.setenemyroom(k, 14+100, 2+100); break;
                                            case 10:  obj.setenemyroom(k, 10+100, 7+100); break;
                                            default: obj.setenemyroom(k, 4+100, 0+100); break;
                                        }
                                    }
                                    obj.entities[k].colour = ss_toi(words[6]);
                                }
			}
                        else if (words[0] == "fatal_left")
			{
				obj.fatal_left();
			}
			else if (words[0] == "fatal_right")
			{
				obj.fatal_right();
			}
			else if (words[0] == "fatal_top")
			{
				obj.fatal_top();
			}
			else if (words[0] == "fatal_bottom")
			{
				obj.fatal_bottom();
			}
			else if (words[0] == "createcrewman")
			{
				if (words[3] == "cyan")
				{
					r=0;
				}
				else if (words[3] == "red")
				{
					r=15;
				}
				else if (words[3] == "green")
				{
					r=13;
				}
				else if (words[3] == "yellow")
				{
					r=14;
				}
				else if (words[3] == "blue")
				{
					r=16;
				}
				else if (words[3] == "purple")
				{
					r=20;
				}
				else if (strspn( words[3].c_str(), "-.0123456789" ) == words[3].size() && words[3].size() != 0)
				{
					r=ss_toi(words[3]);
				}
				else
				{
					r=19;
				}

				//convert the command to the right index
				if (words[5] == "followplayer") words[5] = "10";
				if (words[5] == "followpurple") words[5] = "11";
				if (words[5] == "followyellow") words[5] = "12";
				if (words[5] == "followred") words[5] = "13";
				if (words[5] == "followgreen") words[5] = "14";
				if (words[5] == "followblue") words[5] = "15";

				if (words[5] == "followposition") words[5] = "16";
				if (words[5] == "faceleft")
				{
					words[5] = "17";
					words[6] = "0";
				}
				if (words[5] == "faceright")
				{
					words[5] = "17";
					words[6] = "1";
				}
				if (words[5] == "faceplayer")
				{
					words[5] = "18";
					words[6] = "0";
				}
				if (words[5] == "panic")
				{
					words[5] = "20";
					words[6] = "0";
				}

                                int ent = 18;
                                if (words[7] == "flip") {
                                    ent = 57;
                                }

                                int id;

                                if (ss_toi(words[5]) >= 16)
                                {
                                    id = obj.createentity(game, ss_toi(words[1]), ss_toi(words[2]), ent, r, ss_toi(words[4]), ss_toi(words[5]), ss_toi(words[6]));
                                }
                                else
                                {
                                    id = obj.createentity(game, ss_toi(words[1]), ss_toi(words[2]), ent, r, ss_toi(words[4]), ss_toi(words[5]));
                                }

                                if (words[7] == "flip") {
                                    if (words[8] != "") obj.named_crewmen[words[8]] = id;
                                } else if (words[7] != "") {
                                    obj.named_crewmen[words[7]] = id;
                                }
			}
			else if (words[0] == "createroomtext")
			{
				map.roomtexton = true;
				map.roomtextx[map.roomtextnumlines] = ss_toi(words[1]);
				map.roomtexty[map.roomtextnumlines] = ss_toi(words[2]);
				position++;
				map.roomtext[map.roomtextnumlines] = processvars(commands[position]);
				map.roomtextnumlines++;
			}
			else if (words[0] == "createscriptbox")
			{
				// Ok, first figure out the first available script box slot
				int lastslot = 0;
				for (int bsi = 0; bsi < obj.nblocks; bsi++)
					if (obj.blocks[bsi].trigger > lastslot)
						lastslot = obj.blocks[bsi].trigger;
				int usethisslot;
				if (lastslot == 0)
					usethisslot = 300;
				else
					usethisslot = lastslot + 1;

				obj.createblock(TRIGGER, ss_toi(words[1]), ss_toi(words[2]), ss_toi(words[3]), ss_toi(words[4]), usethisslot);
				game.customscript[usethisslot - 300] = words[5];
			}
			else if (words[0] == "customactivityzone")
			{
				position++;
				obj.customprompt = processvars(commands[position]);
				obj.customscript = words[6];
				obj.customcolour = words[5];

				obj.createblock(ACTIVITY, ss_toi(words[1]), ss_toi(words[2]), ss_toi(words[3]), ss_toi(words[4]), 100);
			}
			else if (words[0] == "customactivityzonergb")
			{
				position++;
				obj.customprompt = processvars(commands[position]);
				obj.customscript = words[8];
				obj.customr = ss_toi(words[5]);
				obj.customg = ss_toi(words[6]);
				obj.customb = ss_toi(words[7]);

				obj.createblock(ACTIVITY, ss_toi(words[1]), ss_toi(words[2]), ss_toi(words[3]), ss_toi(words[4]), 101);
			}
			else if (words[0] == "changemood")
			{
                            i = obj.getcrewman(words[1]);

				if (ss_toi(words[2]) == 0)
				{
					obj.entities[i].tile = 0;
				}
				else
				{
					obj.entities[i].tile = 144;
				}
			}
			else if (words[0] == "changecustommood")
			{
                            i = obj.getcrewman(words[1]);

				if (ss_toi(words[2]) == 0)
				{
					obj.entities[i].tile = 0;
				}
				else
				{
					obj.entities[i].tile = 144;
				}
			}
			else if (words[0] == "changetile")
			{
                            i = obj.getcrewman(words[1]);

				obj.entities[i].tile = ss_toi(words[2]);
			}
			else if (words[0] == "flipgravity")
			{
				//not something I'll use a lot, I think. Doesn't need to be very robust!
				if (words[1] == "player")
				{
					game.gravitycontrol = !game.gravitycontrol;
				}
				else
				{
                                    i = obj.getcrewman(words[1]);

					if (obj.entities[i].rule == 6)
					{
						obj.entities[i].rule = 7;
						obj.entities[i].tile = 6;
					}
					else if (obj.entities[i].rule == 7)
					{
						obj.entities[i].rule = 6;
						obj.entities[i].tile = 0;
					}
				}
			}
			else if (words[0] == "changegravity")
			{
				//not something I'll use a lot, I think. Doesn't need to be very robust!
                            i = obj.getcrewman(words[1]);

				obj.entities[i].tile +=12;
			}
			else if (words[0] == "changedir")
			{
                            i = obj.getcrewman(words[1]);

				if (ss_toi(words[2]) == 0)
				{
					obj.entities[i].dir = 0;
				}
				else
				{
					obj.entities[i].dir = 1;
				}
			}
			else if (words[0] == "alarmon")
			{
				game.alarmon = true;
				game.alarmdelay = 0;
			}
			else if (words[0] == "alarmoff")
			{
				game.alarmon = false;
			}
			else if (words[0] == "changeai")
			{
                            i = obj.getcrewman(words[1]);

				if (words[2] == "followplayer") words[2] = "10";
				if (words[2] == "followpurple") words[2] = "11";
				if (words[2] == "followyellow") words[2] = "12";
				if (words[2] == "followred") words[2] = "13";
				if (words[2] == "followgreen") words[2] = "14";
				if (words[2] == "followblue") words[2] = "15";

				if (words[2] == "followposition") words[2] = "16";
				if (words[2] == "faceleft")
				{
					words[2] = "17";
					words[3] = "0";
				}
				if (words[2] == "faceright")
				{
					words[2] = "17";
					words[3] = "1";
				}


				obj.entities[i].state = ss_toi(words[2]);
				if (obj.entities[i].state == 16)
				{
					if (words[1] == "player")
					{
						// Just do a walk instead
						int walkframes = (ss_toi(words[3]) - obj.entities[i].xp) / 6;
						scriptdelay = std::abs(walkframes);
						if (walkframes > 0)
						{
							game.press_left = false;
							game.press_right = true;
						}
						else if (walkframes < 0)
						{
							game.press_left = true;
							game.press_right = false;
						}
					}
					else
					{
						obj.entities[i].para=ss_toi(words[3]);
					}
				}
				else if (obj.entities[i].state == 17)
				{
					obj.entities[i].dir=ss_toi(words[3]);
				}
			}
			else if (words[0] == "alarmon")
			{
				game.alarmon = true;
				game.alarmdelay = 0;
			}
			else if (words[0] == "alarmoff")
			{
				game.alarmon = false;
			}
			else if (words[0] == "activateteleporter")
			{
				i = obj.getteleporter();
				obj.entities[i].tile = 6;
				obj.entities[i].colour = 102;
			}
			else if (words[0] == "changecolour" || words[0] == "changecolor")
			{
                            i = obj.getcrewman(words[1]);

				if (words[2] == "cyan")
				{
					obj.entities[i].colour = 0;
				}
				else if (words[2] == "red")
				{
					obj.entities[i].colour = 15;
				}
				else if (words[2] == "green")
				{
					obj.entities[i].colour = 13;
				}
				else if (words[2] == "yellow")
				{
					obj.entities[i].colour = 14;
				}
				else if (words[2] == "blue")
				{
					obj.entities[i].colour = 16;
				}
				else if (words[2] == "purple")
				{
					obj.entities[i].colour = 20;
				}
				else if (words[2] == "teleporter")
				{
					obj.entities[i].colour = 102;
				} else {
                                    obj.entities[i].colour = ss_toi(words[2]);
                                }
			}
			else if (words[0] == "squeak")
			{
				if (words[1] == "player")
				{
					music.playef(11, 10);
				}
				else if (words[1] == "cyan")
				{
					music.playef(11, 10);
				}
				else if (words[1] == "red")
				{
					music.playef(16, 10);
				}
				else if (words[1] == "green")
				{
					music.playef(12, 10);
				}
				else if (words[1] == "yellow")
				{
					music.playef(14, 10);
				}
				else if (words[1] == "blue")
				{
					music.playef(13, 10);
				}
				else if (words[1] == "purple")
				{
					music.playef(15, 10);
				}
				else if (words[1] == "cry")
				{
					music.playef(2, 10);
				}
				else if (words[1] == "terminal")
				{
					music.playef(20, 10);
				}
			}
			else if (words[0] == "blackout")
			{
				game.blackout = true;
			}
			else if (words[0] == "blackon")
			{
				game.blackout = false;
			}
			else if (words[0] == "setcheckpoint")
			{
				i = obj.getplayer();
				game.savepoint = 0;
				game.savex = obj.entities[i].xp ;
				game.savey = obj.entities[i].yp;
				game.savegc = game.gravitycontrol;
				game.saverx = game.roomx;
				game.savery = game.roomy;
				game.savedir = obj.entities[i].dir;
			}
			else if (words[0] == "gotocheckpoint")
			{
				i = obj.getplayer();
				obj.entities[i].xp = game.savex;
				obj.entities[i].yp = game.savey;
				game.gravitycontrol = game.savegc;
				game.roomx = game.saverx;
				game.roomy = game.savery;
				obj.entities[i].dir = game.savedir;
			}
			else if (words[0] == "gamestate")
			{
				game.state = ss_toi(words[1]);
				game.statedelay = 0;
			}
			else if (words[0] == "textboxactive")
			{
				dwgfx.textboxactive();
			}
			else if (words[0] == "gamemode")
			{
				if (words[1] == "teleporter")
				{
					//TODO this draw the teleporter screen. This is a problem. :(
					game.gamestate = 5;
					dwgfx.menuoffset = 240; //actually this should count the roomname
					if (map.extrarow) dwgfx.menuoffset -= 10;
					//dwgfx.menubuffer.copyPixels(dwgfx.screenbuffer, dwgfx.screenbuffer.rect, dwgfx.tl, null, null, false);

					dwgfx.resumegamemode = false;

					game.useteleporter = false; //good heavens don't actually use it
				}
				else if (words[1] == "game")
				{
					dwgfx.resumegamemode = true;
				}
			}
			else if (words[0] == "ifexplored")
			{
				if (map.explored[ss_toi(words[1]) + (20 * ss_toi(words[2]))] == 1)
				{
					call(words[3]);
					position--;
				}
			}
			else if (words[0] == "iflast")
			{
				if (game.lastsaved==ss_toi(words[1]))
				{
					call(words[2]);
					position--;
				}
			}
			else if (words[0] == "ifskip")
			{
				if (game.nocutscenes)
				{
					call(words[1]);
					position--;
				}
			}
			else if (words[0] == "ifflag")
			{
				if (obj.flags[ss_toi(words[1])]==1)
				{
					call(words[2]);
					position--;
				}
			}
			else if (words[0] == "ifnotflag")
			{
				if (obj.flags[ss_toi(words[1])]!=1)
				{
					call("custom_"+words[2]);
					position--;
				}
			}
			else if (words[0] == "ifcrewlost")
			{
				if (game.crewstats[ss_toi(words[1])]==false)
				{
					call(words[2]);
					position--;
				}
			}
			else if (words[0] == "iftrinkets")
			{
				if (game.trinkets >= ss_toi(words[1]))
				{
					call(words[2]);
					position--;
				}
			}
			else if (words[0] == "iftrinketsless")
			{
				if (game.stat_trinkets < ss_toi(words[1]))
				{
					call(words[2]);
					position--;
				}
			}
			else if (words[0] == "ifrand")
			{
				int den = ss_toi(words[1]);
				if (fRandom() < 1.0f/den)
				{
					call("custom_"+words[2]);
					position--;
				}
			}
			else if (words[0] == "ifvce")
			{
				call("custom_"+words[1]);
				position--;
			}
			else if (words[0] == "hidecoordinates")
			{
				map.explored[ss_toi(words[1]) + (20 * ss_toi(words[2]))] = 0;
			}
			else if (words[0] == "showcoordinates")
			{
				map.explored[ss_toi(words[1]) + (20 * ss_toi(words[2]))] = 1;
			}
			else if (words[0] == "hideship")
			{
				map.hideship();
			}
			else if (words[0] == "showship")
			{
				map.showship();
			}
			else if (words[0] == "showsecretlab")
			{
				map.explored[16 + (20 * 5)] = 1;
				map.explored[17 + (20 * 5)] = 1;
				map.explored[18 + (20 * 5)] = 1;
				map.explored[17 + (20 * 6)] = 1;
				map.explored[18 + (20 * 6)] = 1;
				map.explored[19 + (20 * 6)] = 1;
				map.explored[19 + (20 * 7)] = 1;
				map.explored[19 + (20 * 8)] = 1;
			}
			else if (words[0] == "hidesecretlab")
			{
				map.explored[16 + (20 * 5)] = 0;
				map.explored[17 + (20 * 5)] = 0;
				map.explored[18 + (20 * 5)] = 0;
				map.explored[17 + (20 * 6)] = 0;
				map.explored[18 + (20 * 6)] = 0;
				map.explored[19 + (20 * 6)] = 0;
				map.explored[19 + (20 * 7)] = 0;
				map.explored[19 + (20 * 8)] = 0;
			}
			else if (words[0] == "showteleporters")
			{
				map.showteleporters = true;
			}
			else if (words[0] == "showtargets")
			{
				map.showtargets = true;
			}
			else if (words[0] == "showtrinkets")
			{
				map.showtrinkets = true;
			}
			else if (words[0] == "hideteleporters")
			{
				map.showteleporters = false;
			}
			else if (words[0] == "hidetargets")
			{
				map.showtargets = false;
			}
			else if (words[0] == "hidetrinkets")
			{
				map.showtrinkets = false;
			}
			else if (words[0] == "hideplayer")
			{
				obj.entities[obj.getplayer()].invis = true;
			}
			else if (words[0] == "showplayer")
			{
				obj.entities[obj.getplayer()].invis = false;
			}
			else if (words[0] == "killplayer")
			{
				if (game.deathseq <= 0)
					game.deathseq = 30;
			}
			else if (words[0] == "teleportscript")
			{
				game.teleportscript = words[1];
			}
			else if (words[0] == "clearteleportscript")
			{
				game.teleportscript = "";
			}
			else if (words[0] == "nocontrol")
			{
				game.hascontrol = false;
			}
			else if (words[0] == "hascontrol")
			{
				game.hascontrol = true;
			}
			else if (words[0] == "companion")
			{
				game.companion = ss_toi(words[1]);
			}
			else if (words[0] == "befadein")
			{
				dwgfx.fadeamount = 0;
				dwgfx.fademode= 0;
			}
			else if (words[0] == "fadein")
			{
				dwgfx.fademode = 4;
			}
			else if (words[0] == "fadeout")
			{
				dwgfx.fademode = 2;
			}
			else if (words[0] == "untilfade" || words[0] == "puntilfade")
			{
				if (dwgfx.fademode>1)
				{
					scriptdelay = 1;
					if (words[0] == "puntilfade") passive = true;
					position--;
				}
			}
			else if (words[0] == "untilmusic" || words[0] == "puntilmusic")
			{
				if (Mix_FadingMusic() != MIX_NO_FADING)
				{
					scriptdelay = 1;
					if (words[0] == "puntilmusic") passive = true;
					position--;
				}
			}
			else if (words[0] == "entersecretlab")
			{
				game.unlock[8] = true;
				game.insecretlab = true;
			}
			else if (words[0] == "leavesecretlab")
			{
				game.insecretlab = false;
			}
			else if (words[0] == "resetgame")
			{
				map.resetnames();
				map.resetmap();
				map.resetplayer(dwgfx, game, obj, music);
				map.tdrawback = true;

				obj.resetallflags();
				i = obj.getplayer();
				obj.entities[i].tile = 0;

				game.trinkets = 0;
				game.crewmates=0;
				for (i = 0; i < 100; i++)
				{
					obj.collect[i] = 0;
					obj.customcollect[i] = 0;
				}
				game.deathcounts = 0;
				game.advancetext = false;
				game.hascontrol = true;
				game.frames = 0;
				game.seconds = 0;
				game.minutes = 0;
				game.hours = 0;
				game.gravitycontrol = 0;
				game.teleport = false;
				game.companion = 0;
				game.roomchange = false;
				game.teleport_to_new_area = false;
				game.teleport_to_x = 0;
				game.teleport_to_y = 0;

				game.teleportscript = "";

				//get out of final level mode!
				map.finalmode = false;
				map.final_colormode = false;
				map.final_mapcol = 0;
				map.final_colorframe = 0;
				map.finalstretch = false;
			}
			else if (words[0] == "returnscript")
                        {
                            auto frame = callstack.back();
                            load(frame.script);
                            position = frame.line;
                            callstack.pop_back();
                        }
			else if (words[0] == "loadscript")
			{
				call(words[1]);
				position--;
			}
			else if (words[0] == "rollcredits")
			{
				game.gamestate = 6;
				dwgfx.fademode = 4;
				game.creditposition = 0;
			}
			else if (words[0] == "finalmode")
			{
				map.finalmode = true;
				map.finalx = ss_toi(words[1]);
				map.finaly = ss_toi(words[2]);
				game.roomx = map.finalx;
				game.roomy = map.finaly;
				map.gotoroom(game.roomx, game.roomy, dwgfx, game, obj, music);
			}
			else if (words[0] == "rescued")
			{
				if (words[1] == "red")
				{
					game.crewstats[3] = true;
				}
				else if (words[1] == "green")
				{
					game.crewstats[4] = true;
				}
				else if (words[1] == "yellow")
				{
					game.crewstats[2] = true;
				}
				else if (words[1] == "blue")
				{
					game.crewstats[5] = true;
				}
				else if (words[1] == "purple")
				{
					game.crewstats[1] = true;
				}
				else if (words[1] == "player")
				{
					game.crewstats[0] = true;
				}
				else if (words[1] == "cyan")
				{
					game.crewstats[0] = true;
				}
			}
			else if (words[0] == "missing")
			{
				if (words[1] == "red")
				{
					game.crewstats[3] = false;
				}
				else if (words[1] == "green")
				{
					game.crewstats[4] = false;
				}
				else if (words[1] == "yellow")
				{
					game.crewstats[2] = false;
				}
				else if (words[1] == "blue")
				{
					game.crewstats[5] = false;
				}
				else if (words[1] == "purple")
				{
					game.crewstats[1] = false;
				}
				else if (words[1] == "player")
				{
					game.crewstats[0] = false;
				}
				else if (words[1] == "cyan")
				{
					game.crewstats[0] = false;
				}
			}
			else if (words[0] == "face")
			{
                            i = obj.getcrewman(words[1]);
                            j = obj.getcrewman(words[2]);

				if (obj.entities[j].xp > obj.entities[i].xp + 5)
				{
					obj.entities[i].dir = 1;
				}
				else if (obj.entities[j].xp < obj.entities[i].xp - 5)
				{
					obj.entities[i].dir = 0;
				}
			}
			else if (words[0] == "jukebox")
			{
				for (j = 0; j < obj.nentity; j++)
				{
					if (obj.entities[j].type == 13 && obj.entities[j].active)
					{
						obj.entities[j].colour = 4;
					}
				}
				if (ss_toi(words[1]) == 1)
				{
					obj.createblock(5, 88 - 4, 80, 20, 16, 25);
					for (j = 0; j < obj.nentity; j++)
					{
						if (obj.entities[j].xp == 88 && obj.entities[j].yp==80)
						{
							obj.entities[j].colour = 5;
						}
					}
				}
				else if (ss_toi(words[1]) == 2)
				{
					obj.createblock(5, 128 - 4, 80, 20, 16, 26);
					for (j = 0; j < obj.nentity; j++)
					{
						if (obj.entities[j].xp == 128 && obj.entities[j].yp==80)
						{
							obj.entities[j].colour = 5;
						}
					}
				}
				else if (ss_toi(words[1]) == 3)
				{
					obj.createblock(5, 176 - 4, 80, 20, 16, 27);
					for (j = 0; j < obj.nentity; j++)
					{
						if (obj.entities[j].xp == 176 && obj.entities[j].yp==80)
						{
							obj.entities[j].colour = 5;
						}
					}
				}
				else if (ss_toi(words[1]) == 4)
				{
					obj.createblock(5, 216 - 4, 80, 20, 16, 28);
					for (j = 0; j < obj.nentity; j++)
					{
						if (obj.entities[j].xp == 216 && obj.entities[j].yp==80)
						{
							obj.entities[j].colour = 5;
						}
					}
				}
				else if (ss_toi(words[1]) == 5)
				{
					obj.createblock(5, 88 - 4, 128, 20, 16, 29);
					for (j = 0; j < obj.nentity; j++)
					{
						if (obj.entities[j].xp == 88 && obj.entities[j].yp==128)
						{
							obj.entities[j].colour = 5;
						}
					}
				}
				else if (ss_toi(words[1]) == 6)
				{
					obj.createblock(5, 176 - 4, 128, 20, 16, 30);
					for (j = 0; j < obj.nentity; j++)
					{
						if (obj.entities[j].xp == 176 && obj.entities[j].yp==128)
						{
							obj.entities[j].colour = 5;
						}
					}
				}
				else if (ss_toi(words[1]) == 7)
				{
					obj.createblock(5, 40 - 4, 40, 20, 16, 31);
					for (j = 0; j < obj.nentity; j++)
					{
						if (obj.entities[j].xp == 40 && obj.entities[j].yp==40)
						{
							obj.entities[j].colour = 5;
						}
					}
				}
				else if (ss_toi(words[1]) == 8)
				{
					obj.createblock(5, 216 - 4, 128, 20, 16, 32);
					for (j = 0; j < obj.nentity; j++)
					{
						if (obj.entities[j].xp == 216 && obj.entities[j].yp==128)
						{
							obj.entities[j].colour = 5;
						}
					}
				}
				else if (ss_toi(words[1]) == 9)
				{
					obj.createblock(5, 128 - 4, 128, 20, 16, 33);
					for (j = 0; j < obj.nentity; j++)
					{
						if (obj.entities[j].xp == 128 && obj.entities[j].yp==128)
						{
							obj.entities[j].colour = 5;
						}
					}
				}
				else if (ss_toi(words[1]) == 10)
				{
					obj.createblock(5, 264 - 4, 40, 20, 16, 34);
					for (j = 0; j < obj.nentity; j++)
					{
						if (obj.entities[j].xp == 264 && obj.entities[j].yp==40)
						{
							obj.entities[j].colour = 5;
						}
					}
				}
			}
			else if (words[0] == "createactivityzone")
			{
				if (words[1] == "green")
				{
					obj.createblock(5, obj.entities[obj.getcrewman(4)].xp - 32, obj.entities[obj.getcrewman(4)].yp-20, 96, 60, i);
				}
				else
				{
					obj.createblock(5, obj.entities[obj.getcrewman(words[1])].xp - 32, 0, 96, 240, i);
				}
			}
			else if (words[0] == "createrescuedcrew")
			{
				//special for final level cutscene
				//starting at 180, create the rescued crewmembers (ingoring violet, who's at 155)
				i = 215;
				if (game.crewstats[2] && game.lastsaved!=2)
				{
					obj.createentity(game, i, 153, 18, 14, 0, 17, 0);
					i += 25;
				}
				if (game.crewstats[3] && game.lastsaved!=3)
				{
					obj.createentity(game, i, 153, 18, 15, 0, 17, 0);
					i += 25;
				}
				if (game.crewstats[4] && game.lastsaved!=4)
				{
					obj.createentity(game, i, 153, 18, 13, 0, 17, 0);
					i += 25;
				}
				if (game.crewstats[5] && game.lastsaved!=5)
				{
					obj.createentity(game, i, 153, 18, 16, 0, 17, 0);
					i += 25;
				}
			}
			else if (words[0] == "restoreplayercolour" || words[0] == "restoreplayercolor")
			{
				i = obj.getplayer();
				obj.entities[i].colour = 0;
			}
			else if (words[0] == "changeplayercolour" || words[0] == "changeplayercolour")
			{
				i = obj.getplayer();

				if (words[1] == "cyan")
				{
					obj.entities[i].colour = 0;
				}
				else if (words[1] == "red")
				{
					obj.entities[i].colour = 15;
				}
				else if (words[1] == "green")
				{
					obj.entities[i].colour = 13;
				}
				else if (words[1] == "yellow")
				{
					obj.entities[i].colour = 14;
				}
				else if (words[1] == "blue")
				{
					obj.entities[i].colour = 16;
				}
				else if (words[1] == "purple")
				{
					obj.entities[i].colour = 20;
				}
				else if (words[1] == "teleporter")
				{
					obj.entities[i].colour = 102;
				}
			}
			else if (words[0] == "altstates")
			{
				obj.altstates = ss_toi(words[1]);
			}
			else if (words[0] == "activeteleporter")
			{
				i = obj.getteleporter();
				obj.entities[i].colour = 101;
			}
			else if (words[0] == "foundtrinket")
			{
				//music.silencedasmusik();
				music.haltdasmusik();
				music.playef(3,10);

				game.trinkets++;
				obj.collect[ss_toi(words[1])] = 1;

				dwgfx.textboxremovefast();

				dwgfx.createtextbox("        Congratulations!       ", 50, 85, 174, 174, 174);
				dwgfx.addline("");
				dwgfx.addline("You have found a shiny trinket!");
				dwgfx.textboxcenterx();

				dwgfx.createtextbox(" " + help.number(game.trinkets) + " out of Twenty ", 50, 135, 174, 174, 174);
				dwgfx.textboxcenterx();

				if (!game.backgroundtext)
				{
					game.advancetext = true;
					game.hascontrol = false;
					game.pausescript = true;
					if (key.isDown(90) || key.isDown(32) || key.isDown(86)
						|| key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN)) game.jumpheld = true;
				}
				game.backgroundtext = false;
			}
			else if (words[0] == "foundlab")
			{
				music.playef(3,10);

				dwgfx.textboxremovefast();

				dwgfx.createtextbox("        Congratulations!       ", 50, 85, 174, 174, 174);
				dwgfx.addline("");
				dwgfx.addline("You have found the secret lab!");
				dwgfx.textboxcenterx();
				dwgfx.textboxcentery();

				if (!game.backgroundtext)
				{
					game.advancetext = true;
					game.hascontrol = false;
					game.pausescript = true;
					if (key.isDown(90) || key.isDown(32) || key.isDown(86)
						|| key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN)) game.jumpheld = true;
				}
				game.backgroundtext = false;
			}
			else if (words[0] == "foundlab2")
			{
				dwgfx.textboxremovefast();

				dwgfx.createtextbox("The secret lab is separate from", 50, 85, 174, 174, 174);
				dwgfx.addline("the rest of the game. You can");
				dwgfx.addline("now come back here at any time");
				dwgfx.addline("by selecting the new SECRET LAB");
				dwgfx.addline("option in the play menu.");
				dwgfx.textboxcenterx();
				dwgfx.textboxcentery();

				if (!game.backgroundtext)
				{
					game.advancetext = true;
					game.hascontrol = false;
					game.pausescript = true;
					if (key.isDown(90) || key.isDown(32) || key.isDown(86)
						|| key.isDown(KEYBOARD_UP) || key.isDown(KEYBOARD_DOWN)) game.jumpheld = true;
				}
				game.backgroundtext = false;
			}
			else if (words[0] == "everybodysad")
			{
				for (i = 0; i < obj.nentity; i++)
				{
					if (obj.entities[i].rule == 6 || obj.entities[i].rule == 0)
					{
						obj.entities[i].tile = 144;
					}
				}
			}
			else if (words[0] == "startintermission2")
			{
				map.finalmode = true; //Enable final level mode
				map.finalx = 46;
				map.finaly = 54; //Current

				game.savex = 228;
				game.savey = 129;
				game.saverx = 53;
				game.savery = 49;
				game.savegc = 0;
				game.savedir = 0; //Intermission level 2
				game.savepoint = 0;
				game.gravitycontrol = 0;

				map.gotoroom(46, 54, dwgfx, game, obj, music);
			}
			else if (words[0] == "telesave")
			{
				if (!game.intimetrial && !game.nodeathmode && !game.inintermission) game.savetele(map, obj, music);
			}
			else if (words[0] == "customquicksave")
			{
				if (!map.custommode || map.custommodeforreal)
					game.customsavequick(ed.ListOfMetaData[game.playcustomlevel].filename, map, obj, music, dwgfx);
			}
			else if (words[0] == "createlastrescued")
			{
				if (game.lastsaved==2)
				{
					r=14;
				}
				else if (game.lastsaved==3)
				{
					r=15;
				}
				else if (game.lastsaved==4)
				{
					r=13;
				}
				else if (game.lastsaved==5)
				{
					r=16;
				}
				else
				{
					r = 19;
				}

				obj.createentity(game, 200, 153, 18, r, 0, 19, 30);
				i = obj.getcrewman(game.lastsaved);
				obj.entities[i].dir = 1;
			}
			else if (words[0] == "specialline")
			{
				switch(ss_toi(words[1]))
				{
				case 1:
					txtnumlines = 1;

					txt[0] = "I'm worried about " + game.unrescued() + ", Doctor!";
					break;
				case 2:
					txtnumlines = 3;

					if (game.crewrescued() < 5)
					{
						txt[1] = "to helping you find the";
						txt[2] = "rest of the crew!";
					}
					else
					{
						txtnumlines = 2;
						txt[1] = "to helping you find " + game.unrescued() + "!";
					}
					break;
				}
			}
			else if (words[0] == "trinketbluecontrol")
			{
				if (game.trinkets == 20 && obj.flags[67] == 1)
				{
					call("talkblue_trinket6");
					position--;
				}
				else if (game.trinkets >= 19 && obj.flags[67] == 0)
				{
					call("talkblue_trinket5");
					position--;
				}
				else
				{
					call("talkblue_trinket4");
					position--;
				}
			}
			else if (words[0] == "trinketyellowcontrol")
			{
				if (game.trinkets >= 19)
				{
					call("talkyellow_trinket3");
					position--;
				}
				else
				{
					call("talkyellow_trinket2");
					position--;
				}
			}
			else if (words[0] == "redcontrol")
			{
				if (game.insecretlab)
				{
					call("talkred_14");
					position--;
				}
				else	if (game.roomx != 104)
				{
					if (game.roomx == 100)
					{
						call("talkred_10");
						position--;
					}
					else if (game.roomx == 107)
					{
						call("talkred_11");
						position--;
					}
					else if (game.roomx == 114)
					{
						call("talkred_12");
						position--;
					}
				}
				else if (obj.flags[67] == 1)
				{
					//game complete
					call("talkred_13");
					position--;
				}
				else if (obj.flags[35] == 1 && obj.flags[52] == 0)
				{
					//Intermission level
					obj.flags[52] = 1;
					call("talkred_9");
					position--;
				}
				else if (obj.flags[51] == 0)
				{
					//We're back home!
					obj.flags[51] = 1;
					call("talkred_5");
					position--;
				}
				else if (obj.flags[48] == 0 && game.crewstats[5])
				{
					//Victoria's back
					obj.flags[48] = 1;
					call("talkred_6");
					position--;
				}
				else if (obj.flags[49] == 0 && game.crewstats[4])
				{
					//Verdigris' back
					obj.flags[49] = 1;
					call("talkred_7");
					position--;
				}
				else if (obj.flags[50] == 0 && game.crewstats[2])
				{
					//Vitellary's back
					obj.flags[50] = 1;
					call("talkred_8");
					position--;
				}
				else if (obj.flags[45] == 0 && !game.crewstats[5])
				{
					obj.flags[45] = 1;
					call("talkred_2");
					position--;
				}
				else if (obj.flags[46] == 0 && !game.crewstats[4])
				{
					obj.flags[46] = 1;
					call("talkred_3");
					position--;
				}
				else if (obj.flags[47] == 0 && !game.crewstats[2])
				{
					obj.flags[47] = 1;
					call("talkred_4");
					position--;
				}
				else
				{
					obj.flags[45] = 0;
					obj.flags[46] = 0;
					obj.flags[47] = 0;
					call("talkred_1");
					position--;
				}
			}
			//TODO: Non Urgent fix compiler nesting errors without adding complexity
			if (words[0] == "greencontrol")
			{
				if (game.insecretlab)
				{
					call("talkgreen_11");
					position--;
				}
				else	if (game.roomx == 103 && game.roomy == 109)
				{
					call("talkgreen_8");
					position--;
				}
				else if (game.roomx == 101 && game.roomy == 109)
				{
					call("talkgreen_9");
					position--;
				}
				else if (obj.flags[67] == 1)
				{
					//game complete
					call("talkgreen_10");
					position--;
				}
				else if (obj.flags[34] == 1 && obj.flags[57] == 0)
				{
					//Intermission level
					obj.flags[57] = 1;
					call("talkgreen_7");
					position--;
				}
				else if (obj.flags[53] == 0)
				{
					//Home!
					obj.flags[53] = 1;
					call("talkgreen_6");
					position--;
				}
				else if (obj.flags[54] == 0 && game.crewstats[2])
				{
					obj.flags[54] = 1;
					call("talkgreen_5");
					position--;
				}
				else if (obj.flags[55] == 0 && game.crewstats[3])
				{
					obj.flags[55] = 1;
					call("talkgreen_4");
					position--;
				}
				else if (obj.flags[56] == 0 && game.crewstats[5])
				{
					obj.flags[56] = 1;
					call("talkgreen_3");
					position--;
				}
				else if (obj.flags[58] == 0)
				{
					obj.flags[58] = 1;
					call("talkgreen_2");
					position--;
				}
				else
				{
					call("talkgreen_1");
					position--;
				}
			}
			else if (words[0] == "bluecontrol")
			{
				if (game.insecretlab)
				{
					call("talkblue_9");
					position--;
				}
				else	if (obj.flags[67] == 1)
				{
					//game complete, everything changes for victoria
					if (obj.flags[41] == 1 && obj.flags[42] == 0)
					{
						//second trinket conversation
						obj.flags[42] = 1;
						call("talkblue_trinket2");
						position--;
					}
					else if (obj.flags[41] == 0 && obj.flags[42] == 0)
					{
						//Third trinket conversation
						obj.flags[42] = 1;
						call("talkblue_trinket3");
						position--;
					}
					else
					{
						//Ok, we've already dealt with the trinket thing; so either you have them all, or you don't. If you do:
						if (game.trinkets >= 20)
						{
							call("startepilogue");
							position--;
						}
						else
						{
							call("talkblue_8");
							position--;
						}
					}
				}
				else if (obj.flags[33] == 1 && obj.flags[40] == 0)
				{
					//Intermission level
					obj.flags[40] = 1;
					call("talkblue_7");
					position--;
				}
				else if (obj.flags[36] == 0 && game.crewstats[5])
				{
					//Back on the ship!
					obj.flags[36] = 1;
					call("talkblue_3");
					position--;
				}
				else if (obj.flags[41] == 0 && game.crewrescued() <= 4)
				{
					//First trinket conversation
					obj.flags[41] = 1;
					call("talkblue_trinket1");
					position--;
				}
				else if (obj.flags[41] == 1 && obj.flags[42] == 0 && game.crewrescued() == 5)
				{
					//second trinket conversation
					obj.flags[42] = 1;
					call("talkblue_trinket2");
					position--;
				}
				else if (obj.flags[41] == 0 && obj.flags[42] == 0 && game.crewrescued() == 5)
				{
					//Third trinket conversation
					obj.flags[42] = 1;
					call("talkblue_trinket3");
					position--;
				}
				else if (obj.flags[37] == 0 && game.crewstats[2])
				{
					obj.flags[37] = 1;
					call("talkblue_4");
					position--;
				}
				else if (obj.flags[38] == 0 && game.crewstats[3])
				{
					obj.flags[38] = 1;
					call("talkblue_5");
					position--;
				}
				else if (obj.flags[39] == 0 && game.crewstats[4])
				{
					obj.flags[39] = 1;
					call("talkblue_6");
					position--;
				}
				else
				{
					//if all else fails:
					//if yellow is found
					if (game.crewstats[2])
					{
						call("talkblue_2");
						position--;
					}
					else
					{
						call("talkblue_1");
						position--;
					}
				}
			}
			else if (words[0] == "yellowcontrol")
			{
				if (game.insecretlab)
				{
					call("talkyellow_12");
					position--;
				}
				else	if (obj.flags[67] == 1)
				{
					//game complete
					call("talkyellow_11");
					position--;
				}
				else if (obj.flags[32] == 1 && obj.flags[31] == 0)
				{
					//Intermission level
					obj.flags[31] = 1;
					call("talkyellow_6");
					position--;
				}
				else if (obj.flags[27] == 0 && game.crewstats[2])
				{
					//Back on the ship!
					obj.flags[27] = 1;
					call("talkyellow_10");
					position--;
				}
				else if (obj.flags[43] == 0 && game.crewrescued() == 5 && !game.crewstats[5])
				{
					//If by chance we've rescued everyone except Victoria by the end, Vitellary provides you with
					//the trinket information instead.
					obj.flags[43] = 1;
					obj.flags[42] = 1;
					obj.flags[41] = 1;
					call("talkyellow_trinket1");
					position--;
				}
				else if (obj.flags[24] == 0 && game.crewstats[5])
				{
					obj.flags[24] = 1;
					call("talkyellow_8");
					position--;
				}
				else if (obj.flags[26] == 0 && game.crewstats[4])
				{
					obj.flags[26] = 1;
					call("talkyellow_7");
					position--;
				}
				else if (obj.flags[25] == 0 && game.crewstats[3])
				{
					obj.flags[25] = 1;
					call("talkyellow_9");
					position--;
				}
				else if (obj.flags[28] == 0)
				{
					obj.flags[28] = 1;
					call("talkyellow_3");
					position--;
				}
				else if (obj.flags[29] == 0)
				{
					obj.flags[29] = 1;
					call("talkyellow_4");
					position--;
				}
				else if (obj.flags[30] == 0)
				{
					obj.flags[30] = 1;
					call("talkyellow_5");
					position--;
				}
				else if (obj.flags[23] == 0)
				{
					obj.flags[23] = 1;
					call("talkyellow_2");
					position--;
				}
				else
				{
					call("talkyellow_1");
					position--;
					obj.flags[23] = 0;
				}
			}
			else if (words[0] == "purplecontrol")
			{
				//Controls Purple's conversion
				//Crew rescued:
				if (game.insecretlab)
				{
					call("talkpurple_9");
					position--;
				}
				else	if (obj.flags[67] == 1)
				{
					//game complete
					call("talkpurple_8");
					position--;
				}
				else if (obj.flags[17] == 0 && game.crewstats[4])
				{
					obj.flags[17] = 1;
					call("talkpurple_6");
					position--;
				}
				else if (obj.flags[15] == 0 && game.crewstats[5])
				{
					obj.flags[15] = 1;
					call("talkpurple_4");
					position--;
				}
				else if (obj.flags[16] == 0 && game.crewstats[3])
				{
					obj.flags[16] = 1;
					call("talkpurple_5");
					position--;
				}
				else if (obj.flags[18] == 0 && game.crewstats[2])
				{
					obj.flags[18] = 1;
					call("talkpurple_7");
					position--;
				}
				else if (obj.flags[19] == 1 && obj.flags[20] == 0 && obj.flags[21] == 0)
				{
					//intermission one: if played one / not had first conversation / not played two [conversation one]
					obj.flags[21] = 1;
					call("talkpurple_intermission1");
					position--;
				}
				else if (obj.flags[20] == 1 && obj.flags[21] == 1 && obj.flags[22] == 0)
				{
					//intermission two: if played two / had first conversation / not had second conversation [conversation two]
					obj.flags[22] = 1;
					call("talkpurple_intermission2");
					position--;
				}
				else if (obj.flags[20] == 1 && obj.flags[21] == 0 && obj.flags[22] == 0)
				{
					//intermission two: if played two / not had first conversation / not had second conversation [conversation three]
					obj.flags[22] = 1;
					call("talkpurple_intermission3");
					position--;
				}
				else if (obj.flags[12] == 0)
				{
					//Intro conversation
					obj.flags[12] = 1;
					call("talkpurple_intro");
					position--;
				}
				else if (obj.flags[14] == 0)
				{
					//Shorter intro conversation
					obj.flags[14] = 1;
					call("talkpurple_3");
					position--;
				}
				else
				{
					//if all else fails:
					//if green is found
					if (game.crewstats[4])
					{
						call("talkpurple_2");
						position--;
					}
					else
					{
						call("talkpurple_1");
						position--;
					}
				}
			}

			position++;
		}
		else
		{
                        if (!callstack.empty()) callstack.pop_back();
			running = false;
		}
	}

	if(scriptdelay>0)
	{
		scriptdelay--;
	}
}

void scriptclass::resetgametomenu( Graphics& dwgfx, Game& game,mapclass& map, entityclass& obj, UtilityClass& help, musicclass& music )
{
	game.gamestate = TITLEMODE;
	dwgfx.flipmode = false;
	obj.nentity = 0;
	dwgfx.fademode = 4;
	game.createmenu("gameover");
}

void scriptclass::startgamemode( int t, KeyPoll& key, Graphics& dwgfx, Game& game, mapclass& map, entityclass& obj, UtilityClass& help, musicclass& music )
{
    dwgfx.mapimage = std::nullopt;
	switch(t)
	{
	case 0:  //Normal new game
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.start(obj, music);
		game.jumpheld = true;
		dwgfx.showcutscenebars = true;
		dwgfx.cutscenebarspos = 320;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;

		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

		call("intro");
		break;
	case 1:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.start(obj, music);
		game.loadtele(map, obj, music);
		game.gravitycontrol = game.savegc;
		game.jumpheld = true;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;

		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
		dwgfx.fademode = 4;
		break;
	case 2: //Load Quicksave
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.start(obj, music);
		game.loadquick(map, obj, music);
		game.gravitycontrol = game.savegc;
		game.jumpheld = true;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;

		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
		//a very special case for here needs to ensure that the tower is set correctly
		if (map.towermode)
		{
			map.resetplayer(dwgfx, game, obj, music);

			i = obj.getplayer();
			map.ypos = obj.entities[i].yp - 120;
			map.bypos = map.ypos / 2;
			map.cameramode = 0;
			map.colsuperstate = 0;
		}
		dwgfx.fademode = 4;
		break;
	case 3:
		//Start Time Trial 1
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.nocutscenes = true;
		game.intimetrial = true;
		game.timetrialcountdown = 150;
		game.timetrialparlost = false;
		game.timetriallevel = 0;
		game.timetrialpar = 75;
		game.timetrialshinytarget = 2;

		music.fadeout();
		game.gamestate = GAMEMODE;
		game.starttrial(game.timetriallevel, obj, music);
		game.jumpheld = true;

		if (dwgfx.setflipmode) dwgfx.flipmode = true;//set flipmode
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
		dwgfx.fademode = 4;
		break;
	case 4:
		//Start Time Trial 2
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.nocutscenes = true;
		game.intimetrial = true;
		game.timetrialcountdown = 150;
		game.timetrialparlost = false;
		game.timetriallevel = 1;
		game.timetrialpar = 165;
		game.timetrialshinytarget = 4;

		music.fadeout();
		game.gamestate = GAMEMODE;
		game.starttrial(game.timetriallevel, obj, music);
		game.jumpheld = true;

		if (dwgfx.setflipmode) dwgfx.flipmode = true;//set flipmode
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
		dwgfx.fademode = 4;
		break;
	case 5:
		//Start Time Trial 3 tow
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.nocutscenes = true;
		game.intimetrial = true;
		game.timetrialcountdown = 150;
		game.timetrialparlost = false;
		game.timetriallevel = 2;
		game.timetrialpar = 105;
		game.timetrialshinytarget = 2;

		music.fadeout();
		game.gamestate = GAMEMODE;
		game.starttrial(game.timetriallevel, obj, music);
		game.jumpheld = true;

		if (dwgfx.setflipmode) dwgfx.flipmode = true;//set flipmode
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
		dwgfx.fademode = 4;
		break;
	case 6:
		//Start Time Trial 4 station
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.nocutscenes = true;
		game.intimetrial = true;
		game.timetrialcountdown = 150;
		game.timetrialparlost = false;
		game.timetriallevel = 3;
		game.timetrialpar = 200;
		game.timetrialshinytarget = 5;

		music.fadeout();
		game.gamestate = GAMEMODE;
		game.starttrial(game.timetriallevel, obj, music);
		game.jumpheld = true;

		if (dwgfx.setflipmode) dwgfx.flipmode = true;//set flipmode
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
		dwgfx.fademode = 4;
		break;
	case 7:
		//Start Time Trial 5 warp
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.nocutscenes = true;
		game.intimetrial = true;
		game.timetrialcountdown = 150;
		game.timetrialparlost = false;
		game.timetriallevel = 4;
		game.timetrialpar = 120;
		game.timetrialshinytarget = 1;

		music.fadeout();
		game.gamestate = GAMEMODE;
		game.starttrial(game.timetriallevel, obj, music);
		game.jumpheld = true;

		if (dwgfx.setflipmode) dwgfx.flipmode = true;//set flipmode
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
		dwgfx.fademode = 4;
		break;
	case 8:
		//Start Time Trial 6// final level!
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.nocutscenes = true;
		game.intimetrial = true;
		game.timetrialcountdown = 150;
		game.timetrialparlost = false;
		game.timetriallevel = 5;
		game.timetrialpar = 135;
		game.timetrialshinytarget = 1;

		music.fadeout();
		map.finalmode = true; //Enable final level mode
		map.finalx = 46;
		map.finaly = 54; //Current
		map.final_colormode = false;
		map.final_mapcol = 0;
		map.final_colorframe = 0;
		game.gamestate = GAMEMODE;
		game.starttrial(game.timetriallevel, obj, music);
		game.jumpheld = true;

		if (dwgfx.setflipmode) dwgfx.flipmode = true;//set flipmode
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
		dwgfx.fademode = 4;
		break;
	case 9:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.nodeathmode = true;
		game.start(obj, music);
		game.jumpheld = true;
		dwgfx.showcutscenebars = true;
		dwgfx.cutscenebarspos = 320;
		//game.starttest(obj, music);
		//music.play(4);

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;

		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);


		call("intro");
		break;
	case 10:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.nodeathmode = true;
		game.nocutscenes = true;

		game.start(obj, music);
		game.jumpheld = true;
		dwgfx.showcutscenebars = true;
		dwgfx.cutscenebarspos = 320;
		//game.starttest(obj, music);
		//music.play(4);

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;

		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);


		call("intro");
		break;
	case 11:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);

		game.startspecial(0, obj, music);
		game.jumpheld = true;

		//Secret lab, so reveal the map, give them all 20 trinkets
		for (int j = 0; j < 20; j++)
		{
			obj.collect[j] = true;
			for (i = 0; i < 20; i++)
			{
				map.explored[i + (j * 20)] = 1;
			}
		}
		game.trinkets = 20;
		game.insecretlab = true;
		map.showteleporters = true;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;

		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
		music.play(11);
		dwgfx.fademode = 4;
		break;
	case 12:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		music.fadeout();

		game.lastsaved = 2;

		game.crewstats[game.lastsaved] = true;
		game.inintermission = true;
		game.companion = 11;
		game.supercrewmate = true;
		game.scmprogress = 0;
		map.finalmode = true;
		map.finalx = 41;
		map.finaly = 56;
		map.final_colormode = false;
		map.final_mapcol = 0;
		map.final_colorframe = 0;
		game.startspecial(1, obj, music);
		game.jumpheld = true;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

		call("intermission_1");
		break;
	case 13:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		music.fadeout();

		game.lastsaved = 3;

		game.crewstats[game.lastsaved] = true;
		game.inintermission = true;
		game.companion = 11;
		game.supercrewmate = true;
		game.scmprogress = 0;
		map.finalmode = true;
		map.finalx = 41;
		map.finaly = 56;
		map.final_colormode = false;
		map.final_mapcol = 0;
		map.final_colorframe = 0;
		game.startspecial(1, obj, music);
		game.jumpheld = true;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

		call("intermission_1");
		break;
	case 14:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		music.fadeout();

		game.lastsaved = 4;

		game.crewstats[game.lastsaved] = true;
		game.inintermission = true;
		game.companion = 11;
		game.supercrewmate = true;
		game.scmprogress = 0;
		map.finalmode = true;
		map.finalx = 41;
		map.finaly = 56;
		map.final_colormode = false;
		map.final_mapcol = 0;
		map.final_colorframe = 0;
		game.startspecial(1, obj, music);
		game.jumpheld = true;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

		call("intermission_1");
		break;
	case 15:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		music.fadeout();

		game.lastsaved = 5;

		game.crewstats[game.lastsaved] = true;
		game.inintermission = true;
		game.companion = 11;
		game.supercrewmate = true;
		game.scmprogress = 0;
		map.finalmode = true;
		map.finalx = 41;
		map.finaly = 56;
		map.final_colormode = false;
		map.final_mapcol = 0;
		map.final_colorframe = 0;
		game.startspecial(1, obj, music);
		game.jumpheld = true;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

		call("intermission_1");
		break;
	case 16:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		music.fadeout();

		game.lastsaved = 2;

		game.crewstats[game.lastsaved] = true;
		game.inintermission = true;
		map.finalmode = true;
		map.finalx = 41;
		map.finaly = 56;
		map.final_colormode = false;
		map.final_mapcol = 0;
		map.final_colorframe = 0;
		game.startspecial(1, obj, music);
		game.jumpheld = true;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

		call("intermission_2");
		break;
	case 17:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		music.fadeout();

		game.lastsaved = 3;

		game.crewstats[game.lastsaved] = true;
		game.inintermission = true;
		map.finalmode = true;
		map.finalx = 41;
		map.finaly = 56;
		map.final_colormode = false;
		map.final_mapcol = 0;
		map.final_colorframe = 0;
		game.startspecial(1, obj, music);
		game.jumpheld = true;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

		call("intermission_2");
		break;
	case 18:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		music.fadeout();

		game.lastsaved = 4;

		game.crewstats[game.lastsaved] = true;
		game.inintermission = true;
		map.finalmode = true;
		map.finalx = 41;
		map.finaly = 56;
		map.final_colormode = false;
		map.final_mapcol = 0;
		map.final_colorframe = 0;
		game.startspecial(1, obj, music);
		game.jumpheld = true;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

		call("intermission_2");
		break;
	case 19:
		game.gamestate = GAMEMODE;
		hardreset(key, dwgfx, game, map, obj, help, music);
		music.fadeout();

		game.lastsaved = 5;

		game.crewstats[game.lastsaved] = true;
		game.inintermission = true;
		map.finalmode = true;
		map.finalx = 41;
		map.finaly = 56;
		map.final_colormode = false;
		map.final_mapcol = 0;
		map.final_colorframe = 0;
		game.startspecial(1, obj, music);
		game.jumpheld = true;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

		call("intermission_2");
		break;
	case 20:
		//Level editor
		hardreset(key, dwgfx, game, map, obj, help, music);
		ed.reset();
		music.fadeout();

		game.gamestate = EDITORMODE;
		game.jumpheld = true;

		if (dwgfx.setflipmode) dwgfx.flipmode = true;//set flipmode
		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
		dwgfx.fademode = 4;
		break;
	case 21:  //play custom level (in editor)
		game.gamestate = GAMEMODE;
		music.fadeout();
		hardreset(key, dwgfx, game, map, obj, help, music);
		game.customstart(obj, music);
		game.jumpheld = true;


		map.custommode = true;
		map.customx = 100;
		map.customy = 100;

		//dwgfx.showcutscenebars = true;
		//dwgfx.cutscenebarspos = 320;

		//set flipmode
		if (dwgfx.setflipmode) dwgfx.flipmode = true;

		if(obj.nentity==0)
		{
			obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
		}
		else
		{
			map.resetplayer(dwgfx, game, obj, music);
		}
		map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
		if(ed.levmusic>0){
		  music.play(ed.levmusic);
		}else{
		  music.currentsong=-1;
		}
		//call("intro");
		break;
  case 22:  //play custom level (in game)
    //Initilise the level
    //First up, find the start point
    ed.weirdloadthing(ed.ListOfMetaData[game.playcustomlevel].filename,dwgfx);
    ed.findstartpoint(game);

    game.gamestate = GAMEMODE;
    music.fadeout();
    hardreset(key, dwgfx, game, map, obj, help, music);
    game.customstart(obj, music);
    game.jumpheld = true;

		map.custommodeforreal = true;
    map.custommode = true;
    map.customx = 100;
    map.customy = 100;

    //dwgfx.showcutscenebars = true;
    //dwgfx.cutscenebarspos = 320;

    //set flipmode
    if (dwgfx.setflipmode) dwgfx.flipmode = true;

    if(obj.nentity==0)
    {
      obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
    }
    else
    {
      map.resetplayer(dwgfx, game, obj, music);
    }
    map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);

		ed.generatecustomminimap(dwgfx, map);
		map.customshowmm=true;
    if(ed.levmusic>0){
      music.play(ed.levmusic);
    }else{
      music.currentsong=-1;
		}
		dwgfx.fademode = 4;
    //call("intro");
  break;
  case 23: //Continue in custom level
      //Initilise the level
    //First up, find the start point
    ed.weirdloadthing(ed.ListOfMetaData[game.playcustomlevel].filename, dwgfx);
    ed.findstartpoint(game);

    game.gamestate = GAMEMODE;
    music.fadeout();
    hardreset(key, dwgfx, game, map, obj, help, music);
		map.custommodeforreal = true;
    map.custommode = true;
    map.customx = 100;
    map.customy = 100;

    game.customstart(obj, music);
		game.customloadquick(ed.ListOfMetaData[game.playcustomlevel].filename, map, obj, music, dwgfx);
    game.jumpheld = true;
    game.gravitycontrol = game.savegc;


    //dwgfx.showcutscenebars = true;
    //dwgfx.cutscenebarspos = 320;

    //set flipmode
    if (dwgfx.setflipmode) dwgfx.flipmode = true;

    if(obj.nentity==0)
    {
      obj.createentity(game, game.savex, game.savey, 0, 0); //In this game, constant, never destroyed
    }
    else
    {
      map.resetplayer(dwgfx, game, obj, music);
    }
    map.gotoroom(game.saverx, game.savery, dwgfx, game, obj, music);
    /* Handled by load
    if(ed.levmusic>0){
      music.play(ed.levmusic);
    }else{
      music.currentsong=-1;
		}
		*/
                ed.generatecustomminimap(dwgfx, map);
		dwgfx.fademode = 4;
    //call("intro");
  break;
	case 100:
		game.savestats(map, dwgfx, music);

		SDL_Quit();
		exit(0);
		break;
	}
}

void scriptclass::teleport( Graphics& dwgfx, Game& game, mapclass& map, entityclass& obj, UtilityClass& help, musicclass& music )
{
	//er, ok! Teleport to a new area, so!
	//A general rule of thumb: if you teleport with a companion, get rid of them!
	game.companion = 0;

	i = obj.getplayer(); //less likely to have a serious collision error if the player is centered
	obj.entities[i].xp = 150;
	obj.entities[i].yp = 110;
	if(game.teleport_to_x==17 && game.teleport_to_y==17) obj.entities[i].xp = 88; //prevent falling!

	if (game.teleportscript == "levelonecomplete")
	{
		game.teleport_to_x = 2;
		game.teleport_to_y = 11;
	}
	else if (game.teleportscript == "gamecomplete")
	{
		game.teleport_to_x = 2;
		game.teleport_to_y = 11;
	}

	game.gravitycontrol = 0;
	map.gotoroom(100+game.teleport_to_x, 100+game.teleport_to_y, dwgfx, game, obj, music);
	j = obj.getteleporter();
	obj.entities[j].state = 2;
	game.teleport_to_new_area = false;

	game.savepoint = obj.entities[j].para;
	game.savex = obj.entities[j].xp + 44;
	game.savey = obj.entities[j].yp + 44;
	game.savegc = 0;

	game.saverx = game.roomx;
	game.savery = game.roomy;
	game.savedir = obj.entities[obj.getplayer()].dir;

	if(game.teleport_to_x==0 && game.teleport_to_y==0)
	{
		game.state = 4020;
	}
	else if(game.teleport_to_x==0 && game.teleport_to_y==16)
	{
		game.state = 4030;
	}
	else if(game.teleport_to_x==7 && game.teleport_to_y==9)
	{
		game.state = 4040;
	}
	else if(game.teleport_to_x==8 && game.teleport_to_y==11)
	{
		game.state = 4050;
	}
	else if(game.teleport_to_x==14 && game.teleport_to_y==19)
	{
		game.state = 4030;
	}
	else if(game.teleport_to_x==17 && game.teleport_to_y==12)
	{
		game.state = 4020;
	}
	else if(game.teleport_to_x==17 && game.teleport_to_y==17)
	{
		game.state = 4020;
	}
	else if(game.teleport_to_x==18 && game.teleport_to_y==7)
	{
		game.state = 4060;
	}
	else
	{
		game.state = 4010;
	}

	if (game.teleportscript != "")
	{
		game.state = 0;
		call(game.teleportscript);
		game.teleportscript = "";
	}
	else
	{
		//change music based on location
		if (dwgfx.setflipmode && game.teleport_to_x == 11 && game.teleport_to_y == 4)
		{
			music.niceplay(9);
		}
		else
		{
			music.changemusicarea(game.teleport_to_x, game.teleport_to_y);
		}
		if (!game.intimetrial && !game.nodeathmode && !game.inintermission)
		{
			if (dwgfx.flipmode)
			{
				dwgfx.createtextbox("    Game Saved    ", -1, 202, 174, 174, 174);
				dwgfx.textboxtimer(25);
			}
			else
			{
				dwgfx.createtextbox("    Game Saved    ", -1, 12, 174, 174, 174);
				dwgfx.textboxtimer(25);
			}
			game.savetele(map, obj, music);
		}
	}
}

void scriptclass::hardreset( KeyPoll& key, Graphics& dwgfx, Game& game,mapclass& map, entityclass& obj, UtilityClass& help, musicclass& music )
{
	//Game:
	game.hascontrol = true;
	game.gravitycontrol = 0;
	game.teleport = false;
	game.companion = 0;
	game.roomchange = false;
	game.teleport_to_new_area = false;
	game.teleport_to_x = 0;
	game.teleport_to_y = 0;
	game.teleportscript = "";

	game.tapleft = 0;
	game.tapright = 0;
	game.startscript = false;
	game.newscript = "";
	game.alarmon = false;
	game.alarmdelay = 0;
	game.blackout = false;
	game.useteleporter = false;
	game.teleport_to_teleporter = 0;

	game.nodeathmode = false;
	game.nocutscenes = false;

	for (i = 0; i < 6; i++)
	{
		game.crewstats[i] = false;
	}
	game.crewstats[0] = true;
	game.lastsaved = 0;

	game.deathcounts = 0;
	game.gameoverdelay = 0;
	game.frames = 0;
	game.seconds = 0;
	game.minutes = 0;
	game.hours = 0;
	game.gamesaved = false;
	game.savetime = "00:00";
	game.savearea = "nowhere";
	game.savetrinkets = 0;

	game.intimetrial = false;
	game.timetrialcountdown = 0;
	game.timetrialshinytarget = 0;
	game.timetrialparlost = false;
	game.timetrialpar = 0;
	game.timetrialresulttime = 0;

	game.totalflips = 0;
	game.hardestroom = "Welcome Aboard";
	game.hardestroomdeaths = 0;
	game.currentroomdeaths=0;

	game.swnmode = false;
	game.swntimer = 0;
	game.swngame = 0;//Not playing sine wave ninja!
	game.swnstate = 0;
	game.swnstate2 = 0;
	game.swnstate3 = 0;
	game.swnstate4 = 0;
	game.swndelay = 0;
	game.swndeaths = 0;
	game.supercrewmate = false;
	game.scmhurt = false;
	game.scmprogress = 0;
	game.scmmoveme = false;
	game.swncolstate = 0;
	game.swncoldelay = 0;
	game.swnrank = 0;
	game.swnmessage = 0;
	game.creditposx = 0;
	game.creditposy = 0;
	game.creditposdelay = 0;

	game.inintermission = false;
	game.insecretlab = false;

	game.crewmates=0;

	game.state = 0;
	game.statedelay = 0;

  game.hascontrol = true;
  game.advancetext = false;

	game.noflip = false;
	game.infiniflip = false;

	game.nosuicide = false;

	//dwgraphicsclass
	dwgfx.backgrounddrawn = false;
	dwgfx.textboxremovefast();
	dwgfx.flipmode = false; //This will be reset if needs be elsewhere
	dwgfx.showcutscenebars = false;

  //mapclass
	map.warpx = false;
	map.warpy = false;
	map.showteleporters = false;
	map.showtargets = false;
	map.showtrinkets = false;
	map.finalmode = false;
	map.finalstretch = false;
	map.finalx = 50;
	map.finaly = 50;
	map.final_colormode = false;
	map.final_colorframe = 0;
	map.final_colorframedelay = 0;
	map.final_mapcol = 0;
	map.final_aniframe = 0;
	map.final_aniframedelay = 0;
	map.rcol = 0;
	map.resetnames();
	map.custommode=false;
	map.custommodeforreal=false;

	map.customshowmm=true;

	for (j = 0; j < 20; j++)
	{
		for (i = 0; i < 20; i++)
		{
			map.roomdeaths[i + (j * 20)] = 0;
			map.roomdeathsfinal[i + (j * 20)] = 0;
			map.explored[i + (j * 20)] = 0;
		}
	}
	//entityclass
	obj.nearelephant = false;
	obj.upsetmode = false;
	obj.upset = 0;

	obj.trophytext = 0 ;
	obj.trophytype = 0;
	obj.altstates = 0;

	for (i = 0; i < 100; i++)
	{
		obj.flags[i] = false;
	}

  for (i = 0; i < 6; i++){
    obj.customcrewmoods[i]=1;
  }

	for (i = 0; i < 100; i++)
	{
		obj.collect[i] = 0;
		obj.customcollect[i] = 0;
	}

	if (obj.getplayer() > -1){
		obj.entities[obj.getplayer()].tile = 0;
	}

	//Script Stuff
	position = 0;
	scriptlength = 0;
	scriptdelay = 0;
	scriptname = "";
	running = false;
	passive = false;
	variablenames.clear();
	variablecontents.clear();
	variablenames.resize(100);
	variablecontents.resize(100);
}

int scriptclass::getlabelnum(std::string thelabel)
{
	for (int n = 0; n < nlabels; n++)
		if (labelnames[n] == thelabel)
			return n;
	return -1;
}
