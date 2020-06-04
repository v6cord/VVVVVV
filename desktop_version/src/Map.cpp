#include "Map.h"

#include "MakeAndPlay.h"

#if !defined(NO_CUSTOM_LEVELS)
	extern editorclass ed;
#endif

mapclass::mapclass()
{
	//Start here!
	r = 196;
	g = 196;
	b = 196;
	colstate = 0;
	colstatedelay = 0;
	colsuperstate = 0;
	spikeleveltop = 0;
	spikelevelbottom = 0;
	warpx = false;
	warpy = false;
	extrarow = 0;

	showteleporters = false;
	showtargets = false;
	showtrinkets = false;

	finalmode = false;
	finalstretch = false;
	finalx = 50;
	finaly = 50;

	cursorstate = 0;
	cursordelay = 0;

	final_colormode = false;
	final_colorframe = 0;
	final_colorframedelay = 0;
	final_mapcol = 0;
	final_aniframe = 0;
	final_aniframedelay = 0;

	custommode=false;
	custommodeforreal=false;
	customx=0; customy=0;
	customwidth=20; customheight=20;
	custommmxoff=0; custommmyoff=0; custommmxsize=0; custommmysize=0;
	customzoom=0;
	customshowmm=true;

	rcol = 0;

	//This needs to be in map instead!
	invincibility = false;

	//We init the lookup table:
	for (int i = 0; i < 30; i++)
	{
		vmult.push_back(int(i * 40));
	}
	//We create a blank map
	for (int j = 0; j < 30; j++)
	{
		for (int i = 0; i < 40; i++)
		{
			contents.push_back(0);
		}
	}

	areamap.resize(20 * 20);
	roomdeaths.resize(ed.maxwidth * ed.maxheight);
	roomdeathsfinal.push_back(20 * 20);
	explored.push_back(ed.maxwidth * ed.maxheight);

	tileset = 0;
	initmapdata();

	for (int i = 0; i < 8; i++)
	{
		specialnames.push_back(std::string());
	}
	resetnames();

	//Areamap starts at 100,100 and extends 20x20
	growing_vector<std::string> tmap;
	tmap.push_back("1,2,2,2,2,2,2,2,0,3,0,0,0,4,4,4,4,4,4,4");
	tmap.push_back("1,2,2,2,2,2,2,0,0,3,0,0,0,0,4,4,4,4,4,4");
	tmap.push_back("0,1,0,0,2,0,0,0,0,3,0,0,0,0,4,4,4,4,4,4");
	tmap.push_back("0,0,0,0,2,0,0,0,0,3,0,0,5,5,5,5,4,4,4,4");
	tmap.push_back("0,0,2,2,2,0,0,0,0,3,11,11,5,5,5,5,0,0,0,0");
	tmap.push_back("0,0,0,0,0,0,0,0,0,3,5,5,5,5,5,5,0,0,0,0");
	tmap.push_back("0,0,0,0,0,0,0,0,0,3,5,5,5,5,5,5,5,0,0,0");
	tmap.push_back("0,0,0,0,0,0,0,0,0,3,5,5,5,5,5,5,5,5,5,0");
	tmap.push_back("0,0,0,0,0,0,0,0,0,3,0,0,0,5,5,5,5,5,5,0");
	tmap.push_back("0,0,0,0,0,0,0,0,11,3,0,0,0,5,5,5,5,5,5,0");
	tmap.push_back("0,0,0,0,0,0,0,0,0,3,0,0,0,5,5,5,5,5,5,0");
	tmap.push_back("0,0,0,0,0,0,0,0,0,3,0,5,5,5,5,5,5,5,5,0");
	tmap.push_back("0,0,0,0,0,0,0,0,0,3,0,5,5,5,5,5,5,0,5,0");
	tmap.push_back("0,0,0,0,0,0,0,0,0,3,0,5,5,5,5,5,5,0,5,0");
	tmap.push_back("0,0,0,0,0,0,0,0,0,3,0,5,5,0,0,0,0,0,5,0");
	tmap.push_back("0,0,0,0,0,0,0,2,0,3,0,0,0,0,0,0,0,0,0,0");
	tmap.push_back("0,0,2,2,2,2,2,2,0,3,0,0,0,0,0,0,0,0,0,0");
	tmap.push_back("0,2,2,2,2,2,2,2,0,3,0,0,0,0,0,0,0,0,0,0");
	tmap.push_back("2,2,2,2,2,0,0,2,0,3,0,0,0,0,0,0,0,0,0,0");
	tmap.push_back("2,2,2,2,2,0,0,2,0,3,0,0,0,0,0,0,0,0,0,0");
	fillareamap(tmap);
}

int mapclass::RGB(int red,int green,int blue)
{
	return (blue | (green << 8) | (red << 16));
}

int mapclass::intpol(int a, int b, float c)
{
	return static_cast<int>(a + ((b - a) * c));
}

void mapclass::setteleporter(int x, int y)
{
	point temp;
	temp.x = x;
	temp.y = y;
	teleporters.push_back(temp);
}

void mapclass::remteleporter(int x, int y)
{
	for (size_t i = 0; i < teleporters.size(); i++) {
		if (teleporters[i].x == x && teleporters[i].y == y) {
			teleporters.erase(teleporters.begin() + i);
			break;
		}
	}
}

void mapclass::settrinket(int x, int y)
{
	point temp;
	temp.x = x;
	temp.y = y;
	shinytrinkets.push_back(temp);
}

void mapclass::resetmap()
{
	//clear the explored area of the map
	for (int j = 0; j < ed.maxheight; j++)
	{
		for (int i = 0; i < ed.maxwidth; i++)
		{
			explored[i + (j * ed.maxwidth)] = 0;
		}
	}
}

void mapclass::resetnames()
{
	//Reset all the special names
	specialnames[0] = "Rear Window";
	specialnames[1] = "On the Waterfront";
	specialnames[2] = "The Untouchables";
	specialnames[3] = "Television Newsveel";
	specialnames[4] = "Vwitched";
	specialnames[5] = "Gvnsmoke";
	specialnames[6] = "Please enjoy these repeats";
	specialnames[7] = "Try Jiggling the Antenna";

	glitchmode = 0;
	glitchdelay = 0;
}

void mapclass::transformname(int t)
{
	//transform special names into new ones, one step at a time

	glitchdelay--;
	if(glitchdelay<=0)
	{
		switch(t)
		{
		case 3:
			//Television Newsveel -> The 9 O'Clock News
			if (specialnames[3] == "Television Newsveel")
			{
				specialnames[3] = "Television Newsvel";
			}
			else if (specialnames[3] == "Television Newsvel")
			{
				specialnames[3] = "TelevisvonvNewsvel";
			}
			else if (specialnames[3] == "TelevisvonvNewsvel")
			{
				specialnames[3] = "TvlvvvsvonvNevsvel";
			}
			else if (specialnames[3] == "TvlvvvsvonvNevsvel")
			{
				specialnames[3] = "vvvvvvsvovvNe svel";
			}
			else if (specialnames[3] == "vvvvvvsvovvNe svel")
			{
				specialnames[3] = "vhv vvv'vvovv vevl";
			}
			else if (specialnames[3] == "vhv vvv'vvovv vevl")
			{
				specialnames[3] = "vhv V v'Cvovv vewv";
			}
			else if (specialnames[3] == "vhv V v'Cvovv vewv")
			{
				specialnames[3] = "vhe 9 v'Cvovv vewv";
			}
			else if (specialnames[3] == "vhe 9 v'Cvovv vewv")
			{
				specialnames[3] = "vhe 9 v'Cvovv Newv";
			}
			else if (specialnames[3] == "vhe 9 v'Cvovv Newv")
			{
				specialnames[3] = "The 9 O'Cvovk Newv";
			}
			else if (specialnames[3] == "The 9 O'Cvovk Newv")
			{
				specialnames[3] = "The 9 O'Clock News";
			}
			break;
		case 4:
			//Vwitched -> Dial M for Murder
			if (specialnames[4] == "Vwitched")
			{
				specialnames[4] = "Vwitvhed";
			}
			else if (specialnames[4] == "Vwitvhed")
			{
				specialnames[4] = "vVwivcvedv";
			}
			else if (specialnames[4] == "vVwivcvedv")
			{
				specialnames[4] = "vvvwMvcvMdvv";
			}
			else if (specialnames[4] == "vvvwMvcvMdvv")
			{
				specialnames[4] = "DvvvwMvfvvMdvvv";
			}
			else if (specialnames[4] == "DvvvwMvfvvMdvvv")
			{
				specialnames[4] = "Dvav Mvfvr Mdvvvv";
			}
			else if (specialnames[4] == "Dvav Mvfvr Mdvvvv")
			{
				specialnames[4] = "Diav M for Mdrver";
			}
			else if (specialnames[4] == "Diav M for Mdrver")
			{
				specialnames[4] = "Dial M for Murder";
			}
			break;
		case 5:
			//Gvnsmoke -> Gunsmoke 1966
			if (specialnames[5] == "Gvnsmoke")
			{
				specialnames[5] = "Gvnsmove";
			}
			else if (specialnames[5] == "Gvnsmove")
			{
				specialnames[5] = "Gvnvmovevv";
			}
			else if (specialnames[5] == "Gvnvmovevv")
			{
				specialnames[5] = "Gunvmove1vv6";
			}
			else if (specialnames[5] == "Gunvmove1vv6")
			{
				specialnames[5] = "Vunsmoke 19v6";
			}
			else if (specialnames[5] == "Vunsmoke 19v6")
			{
				specialnames[5] = "Gunsmoke 1966";
			}
			break;
		case 6:
			//Please enjoy these repeats -> In the Margins
			if (specialnames[6] == "Please enjoy these repeats")
			{
				specialnames[6] = "Please envoy theve repeats";
			}
			else if (specialnames[6] == "Please envoy theve repeats")
			{
				specialnames[6] = "Plse envoy tse rvpvas";
			}
			else if (specialnames[6] == "Plase envoy these rvpeas")
			{
				specialnames[6] = "Plse envoy tse rvpvas";
			}
			else if (specialnames[6] == "Plse envoy tse rvpvas")
			{
				specialnames[6] = "Vl envoy te rvevs";
			}
			else if (specialnames[6] == "Vl envoy te rvevs")
			{
				specialnames[6] = "Vv evo tv vevs";
			}
			else if (specialnames[6] == "Vv evo tv vevs")
			{
				specialnames[6] = "Iv vhv Mvrvivs";
			}
			else if (specialnames[6] == "Iv vhv Mvrvivs")
			{
				specialnames[6] = "In the Margins";
			}
			break;
		case 7:
			//Try Jiggling the Antenna -> Heaven's Gate
			if (specialnames[7] == "Try Jiggling the Antenna")
			{
				specialnames[7] = "Try Viggling the Antenna";
			}
			else if (specialnames[7] == "Try Viggling the Antenna")
			{
				specialnames[7] = "TryJivglvng theAvtevna";
			}
			else if (specialnames[7] == "TryJivglvng theAvtevna")
			{
				specialnames[7] = "Tvvivglvng thAvtvvv";
			}
			else if (specialnames[7] == "Tvvivglvng thAvtvvv")
			{
				specialnames[7] = "Vvvgglvnv tvnvva";
			}
			else if (specialnames[7] == "Vvvgglvnv tvnvva")
			{
				specialnames[7] = "Vvavvnvs vvtv";
			}
			else if (specialnames[7] == "Vvavvnvs vvtv")
			{
				specialnames[7] = "Veavvn's Gvte";
			}
			else if (specialnames[7] == "Veavvn's Gvte")
			{
				specialnames[7] = "Heaven's Gate";
			}
			break;

		}
		glitchdelay = 5;
	}
	else
	{
		glitchdelay--;
	}
}

std::string mapclass::getglitchname(int x, int y)
{
	//Returns the name in the final area.
	if (roomname == "glitch")
	{
		//8 Cases!
		//First, the three "glitches"
		glitchdelay--;
		if (glitchdelay <= -5)
		{
			glitchmode = (glitchmode + 1) % 2;
			glitchdelay = 0;
			if (glitchmode == 0) glitchdelay = 20 +int(fRandom() * 10);
		}

		if (x == 42 && y == 51)
		{
			if (glitchmode == 0)
			{
				return specialnames[0];
			}
			else return "Rear Vindow";
		}
		else if (x == 48 && y == 51)
		{
			if (glitchmode == 0)
			{
				return specialnames[1];
			}
			else return "On the Vaterfront";
		}
		else if (x == 49 && y == 51)
		{
			if (glitchmode == 0)
			{
				return specialnames[2];
			}
			else return "The Untouchavles";
		}
	}
	else if (roomname == "change")
	{
		if (finalstretch)
		{
			if (x == 45 && y == 51) transformname(3);
			if (x == 46 && y == 51) transformname(4);
			if (x == 47 && y == 51) transformname(5);
			if (x == 50 && y == 53) transformname(6);
			if (x == 50 && y == 54) transformname(7);
		}

		if (x == 45 && y == 51) return specialnames[3];
		if (x == 46 && y == 51) return specialnames[4];
		if (x == 47 && y == 51) return specialnames[5];
		if (x == 50 && y == 53) return specialnames[6];
		if (x == 50 && y == 54) return specialnames[7];
		return roomname;
	}
	else
	{
		return roomname;
	}
	return roomname;
}

void mapclass::initmapdata()
{
	//Set up static map information like teleporters and shiny trinkets.
	setteleporter(0, 0);
	setteleporter(0, 16);
	setteleporter(2, 4);
	setteleporter(2, 11);
	setteleporter(7, 9);
	setteleporter(7, 15);
	setteleporter(8, 11);
	setteleporter(10, 5);
	setteleporter(11, 4);
	setteleporter(13, 2);
	setteleporter(13, 8);
	setteleporter(14, 19);
	setteleporter(15, 0);
	setteleporter(17, 12);
	setteleporter(17, 17);
	setteleporter(18, 1);
	setteleporter(18, 7);

	settrinket(14, 4);
	settrinket(13, 6);
	settrinket(11, 12);
	settrinket(15, 12);
	settrinket(14, 11);
	settrinket(18, 14);
	settrinket(11, 7);
	settrinket(9, 2);
	settrinket(9, 16);
	settrinket(2, 18);
	settrinket(7, 18);
	settrinket(6, 1);
	settrinket(17, 3);
	settrinket(10, 19);
	settrinket(5, 15);
	settrinket(1, 10);
	settrinket(3, 2);
	settrinket(10, 8);
}

int mapclass::finalat(int x, int y)
{
	//return the tile index of the final stretch tiles offset by the colour difference
	if (contents[x + vmult[y]] == 740)
	{
		//Special case: animated tiles
		if (final_mapcol == 1)
		{
			// Windows hits fRandom() == 1 frequently! For fuck sake! -flibit
			return 737 + (std::min(int(fRandom() * 12), 11) * 40);
		}
		else
		{
			return contents[x + vmult[y]] - (final_mapcol * 3) + (final_aniframe * 40);
		}
	}
	else if (contents[x + vmult[y]] >= 80)
	{
		return contents[x + vmult[y]] - (final_mapcol * 3);
	}
	else
	{
		return contents[x + vmult[y]];
	}
	return 0;
}

int mapclass::maptiletoenemycol(int t)
{
	//returns the colour index for enemies that matches the map colour t
	switch(t)
	{
	case 0:
		return 11;
		break;
	case 1:
		return 6;
		break;
	case 2:
		return 8;
		break;
	case 3:
		return 12;
		break;
	case 4:
		return 9;
		break;
	case 5:
		return 7;
		break;
	case 6:
		return 18;
		break;
	}
	return 11;
}

void mapclass::changefinalcol(int t)
{
	//change the map to colour t - for the game's final stretch.
	//First up, the tiles. This is just a setting:
	final_mapcol = t;
	temp = 6 - t;
	//Next, entities
	for (size_t i = 0; i < obj.entities.size(); i++)
	{
		if (obj.entities[i].type == 1) //something with a movement behavior
		{
			if (obj.entities[i].animate == 10 || obj.entities[i].animate == 11) //treadmill
			{
				if (custommode) {
					obj.entities[i].tile = 568 + (temp * 12);
				} else {
					if(temp<3)
					{
						obj.entities[i].tile = 907 + (temp * 80);
					}
					else
					{
						obj.entities[i].tile = 911 + ((temp-3) * 80);
					}
				}
				if (custommode) {
					if(obj.entities[i].animate == 10)	obj.entities[i].tile += 4;
				} else {
					if(obj.entities[i].animate == 10)	obj.entities[i].tile += 40;
				}
			}
			else if (obj.entities[i].isplatform)
			{
				if (custommode)
					obj.entities[i].tile = 564+(temp*12);
				else
					obj.entities[i].tile = 915+(temp*40);
			}
			else	//just an enemy
			{
				if (obj.entities[i].colour == 102)
					; // If flashy, do nothing!
				else
					obj.entities[i].colour = maptiletoenemycol(temp);
			}
		}
		else if (obj.entities[i].type == 2)	//disappearing platforms
		{
			if (custommode) {
				if (obj.entities[i].state == 3)
					// It's disappeared, so its tile is offset, so we have to correct for that offset
					obj.entities[i].tile = 568+(temp*12);
				else
					// Normal
					obj.entities[i].tile = 564+(temp*12);

				if (obj.entities[i].state == 5)
					// Extra kludge for when it respawns
					obj.entities[i].tile += 3 - obj.entities[i].life/3;
			} else {
				obj.entities[i].tile = 915+(temp*40);
			}
		}
	}
}

void mapclass::setcol(const int r1, const int g1, const int b1 , const int r2, const int g2, const int b2, const int c)
{
	r = intpol(r1, r2, c / 5);
	g = intpol(g1, g2, c / 5);
	b = intpol(b1, b2, c / 5);
}

void mapclass::updatetowerglow()
{
	if (colstatedelay <= 0 || colsuperstate > 0)
	{
		if (colsuperstate > 0) colstate--;
		colstate++;
		if (colstate >= 30) colstate = 0;
		check = colstate % 5; //current state of phase
		cmode = (colstate - check) / 5; // current colour transition

		switch(cmode)
		{
		case 0:
			setcol(255, 93, 107, 255, 255, 93, check);
			break;
		case 1:
			setcol(255, 255, 93, 159, 255, 93, check);
			break;
		case 2:
			setcol(159, 255, 93, 93, 245, 255, check);
			break;
		case 3:
			setcol(93, 245, 255, 177, 93, 255, check);
			break;
		case 4:
			setcol(177, 93, 255, 255, 93, 255, check);
			break;
		case 5:
			setcol(255, 93, 255, 255, 93, 107, check);
			break;
		}

		updatetowerentcol(cmode);

		if (check == 0)
		{
			colstatedelay = 45;
		}
		else
		{
			colstatedelay = 0;
		}
		if (colsuperstate > 0) colstatedelay = 0;

		tdrawback = true;
		towercol = RGB(r*0.04f, g*0.04f, b*0.04f);
	}
	else
	{
		colstatedelay--;
	}
}

void mapclass::nexttowercolour()
{
	colstate+=5;
	if (colstate >= 30) colstate = 0;
	check = colstate % 5; //current state of phase
	cmode = (colstate - check) / 5; // current colour transition

	switch(cmode)
	{
	case 0:
		setcol(255, 93, 107, 255, 255, 93, check);
		break;
	case 1:
		setcol(255, 255, 93, 159, 255, 93, check);
		break;
	case 2:
		setcol(159, 255, 93, 93, 245, 255, check);
		break;
	case 3:
		setcol(93, 245, 255, 177, 93, 255, check);
		break;
	case 4:
		setcol(177, 93, 255, 255, 93, 255, check);
		break;
	case 5:
		setcol(255, 93, 255, 255, 93, 107, check);
		break;
	}

	tdrawback = true;
	towercol = RGB(r*0.04, g*0.04, b*0.04);
}

void mapclass::settowercolour(int t)
{
	colstate=t*5;
	if (colstate >= 30) colstate = 0;
	check = colstate % 5; //current state of phase
	cmode = (colstate - check) / 5; // current colour transition

	switch(cmode)
	{
	case 0:
		setcol(255, 93, 107, 255, 255, 93, check);
		break;
	case 1:
		setcol(255, 255, 93, 159, 255, 93, check);
		break;
	case 2:
		setcol(159, 255, 93, 93, 245, 255, check);
		break;
	case 3:
		setcol(93, 245, 255, 177, 93, 255, check);
		break;
	case 4:
		setcol(177, 93, 255, 255, 93, 255, check);
		break;
	case 5:
		setcol(255, 93, 255, 255, 93, 107, check);
		break;
	}

	tdrawback = true;
	towercol = RGB(r*0.04, g*0.04, b*0.04);
}

bool mapclass::spikecollide(int x, int y)
{
	if (invincibility) return false;
	if (tower.at(x,y,0)>= 6 && tower.at(x,y,0) <= 11) return true;
	return false;
}

bool mapclass::collide(int x, int y)
{
	if (towermode)
	{
		if (tower.at(x, y, 0) >= 12 && tower.at(x, y, 0) <= 27) return true;
		if (invincibility)
		{
			if (tower.at(x, y, 0) >= 6 && tower.at(x, y, 0) <= 11) return true;
		}
	}
	else if (tileset == 2)
	{
		if (y == -1) return collide(x, y + 1);
		if (y == 29+extrarow) return collide(x, y - 1);
		if (x == -1) return collide(x + 1, y);
		if (x == 40) return collide(x - 1, y);
		if (x < 0 || y < 0 || x >= 40 || y >= 29 + extrarow) return false;
		int tile = (contents[x + vmult[y]]) % 30;
		if (tile >= 12 && tile <= 27) return true;
		if (invincibility)
		{
			if (tile >= 6 && tile <= 11) return true;
		}
		if (collidefloor && contents[x + vmult[y]] == 14) return true;
		if (collideroof && contents[x + vmult[y]] == 15) return true;
	}
	else
	{
		if (y == -1) return collide(x, y + 1);
		if (y == 29+extrarow) return collide(x, y - 1);
		if (x == -1) return collide(x + 1, y);
		if (x == 40) return collide(x - 1, y);
		if (x < 0 || y < 0 || x >= 40 || y >= 29+extrarow) return false;
		if (contents[x + vmult[y]] == 1) return true;
		if (tileset==0 && contents[x + vmult[y]] == 59) return true;
		if (contents[x + vmult[y]]>= 80 && contents[x + vmult[y]] < 680) return true;
		if (contents[x + vmult[y]] == 740 && tileset==1) return true;
		if (collidefloor && contents[x + vmult[y]] == 14) return true;
		if (collideroof && contents[x + vmult[y]] == 15) return true;
		if (invincibility)
		{
			if (contents[x + vmult[y]]>= 6 && contents[x + vmult[y]] <= 9) return true;
			if (contents[x + vmult[y]]>= 49 && contents[x + vmult[y]] <= 50) return true;
			if (tileset == 1)
			{
				if (contents[x + vmult[y]]>= 49 && contents[x + vmult[y]] < 80) return true;
			}
		}
	}
	return false;
}

void mapclass::fillareamap(growing_vector<std::string>& tmap)
{

	for (j = 0; j < 20; j++)
	{
		growing_vector<std::string> maprow = split(tmap[j], ',');
		for (int i = 0; i < 20; i++)
		{
			areamap[i + (j * 20)] = atoi(maprow[i].c_str());
		}
	}
}

void mapclass::settile(int xp, int yp, int t)
{
	if (towermode && minitowermode) {
		if (xp >= 0 && xp < 40 && yp >= 0 && yp < minitowersize)
			tower.minitower[xp + yp*40] = t;
	} else {
		if (xp >= 0 && xp < 40 && yp >= 0 && yp < 29+extrarow)
			contents[xp + vmult[yp]] = t;
	}
}

void mapclass::settile_special(int x, int y, int tile) {
	if (towermode || tileset == 2 || tile != 10) // There's nothing behind 1x1 quicksand
		settile(x, y, tile);
	else
		// Don't change the `tile` var, we createentity by checking `tile` later
		settile(x, y, 0);
	graphics.foregrounddrawn = false;

	// Towers don't use blocks for spikes, and you can't place one-ways / 1x1 quicksand with tiles
	// TODO: Update this if you CAN place one-ways / 1x1 quicksand in towers with tiles
	if (towermode)
		return;

	// Kludge to fix spikes, one-ways, and 1x1 quicksand
	// Remove them first, if there is one where we placed the tile
	// The 1x1 quicksand is part entity, part block
	// Remove its entity
	for (size_t eqi = 0; eqi < obj.entities.size(); eqi++)
		if (obj.entities[eqi].type == 3 && obj.entities[eqi].xp/8 == x && obj.entities[eqi].yp/8 == y)
			obj.removeentity(eqi);
	// The rest of these removals are blocks, which I'll just put in one for-loop
	for (size_t bi = 0; bi < obj.blocks.size(); bi++) {
		// Remove the block part of a 1x1 quicksand
		// We check that it's 8x8 in order to not remove 4-wide quicksand / platforms / conveyors
		if (obj.blocks[bi].type == BLOCK && obj.blocks[bi].xp/8 == x && obj.blocks[bi].yp/8 == y && obj.blocks[bi].wp == 8 && obj.blocks[bi].hp == 8)
			removeblock_iter(bi);

		// The spiky part of a spike is a block, remove it
		if (obj.blocks[bi].type == DAMAGE &&
			// Important - do the integer division by 8 on the left side of the equality, not multiply by 8 on the right side
			// (Same applies to the one-way conditional below)
			// Also important - for some godawful reason, the real position of spikes' blocks
			// is stored in their 'x'/'y' attributes instead of 'xp'/'yp' like the rest,
			// and also, the 'x'/'y' attributes are floats instead of ints
			(int) obj.blocks[bi].x/8 == x && (int) obj.blocks[bi].y/8 == y)
				removeblock_iter(bi)

		// The collision of a one-way tile is a block, also remove it
		if (obj.blocks[bi].type == DIRECTIONAL &&
		// Read comment about integer division from above conditional
		obj.blocks[bi].x/8 == x && obj.blocks[bi].y/8 == y)
			removeblock_iter(bi);
	}

	// Ok, now if we've added a tile that's an entity and/or block, actually add it now...

	// This is all copy-pasted from Map.cpp

	// Spikes
	if (tileset == 0) {
		if (tile == 6 || tile == 8)
			// Sticking up
			obj.createblock(DAMAGE, 8*x, 8*y + 4, 8, 4);
		if (tile == 7 || tile == 9)
			// Sticking down
			obj.createblock(DAMAGE, 8*x, 8*y, 8, 4);
		if (tile == 49 || tile == 50)
			// Left or right
			obj.createblock(DAMAGE, 8*x, 8*y + 3, 8, 2);
	} else if (tileset == 1) {
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
	} else if (tileset == 2) {
		int thetile = tile % 30;
		if (thetile == 6 || thetile == 8)
			// Sticking up
			obj.createblock(DAMAGE, 8*x, 8*y + 4, 8, 4);
		if (thetile == 7 || thetile == 9)
			// Sticking down
			obj.createblock(DAMAGE, 8*x, 8*y, 8, 4);
		if (thetile == 10 || thetile == 11)
			// Left or right
			obj.createblock(DAMAGE, 8*x, 8*y + 3, 8, 2);
	}

	// 1x1 quicksand
	if (tileset != 2 && tile == 10)
		obj.createentity(8*x, 8*y, 4);

	// One-way tiles
	if (tileset != 2 && tile >= 14 && tile <= 17)
		obj.createblock(DIRECTIONAL, 8*x, 8*y, 8, 8, tile-14);
}

void mapclass::fillcontent(growing_vector<std::string>& tmap)
{

	for (j = 0; j < 29+extrarow; j++)
	{
		growing_vector<std::string> maprow = split(tmap[j], ',');

		for(int i = 0; i < 40; i++)
		{
			contents[i + vmult[j]] = atoi(maprow[i].c_str());
		}
	}
}


int mapclass::area(int _rx, int _ry)
{
	//THIS IS THE BUG
	if (finalmode)
	{
		return 6;
	}
	else
	{
		int lookup = (_rx - 100) + ((_ry - 100) * 20);
		//lookup = std::max(0,lookup);
		if(_rx-100>=0 && _rx-100<20 && _ry-100>=0 && _ry-100<20){
			return areamap[lookup];
		}
		else
		{
			return 6;
		}
	}
}

void mapclass::exploretower()
{
	for (int i = 0; i < 20; i++)
	{
		explored[9 + (i * ed.maxwidth)] = 1;
	}
}

void mapclass::hideship()
{
	//remove the ship from the explored areas
	explored[2 + (10 * ed.maxwidth)] = 0;
	explored[3 + (10 * ed.maxwidth)] = 0;
	explored[4 + (10 * ed.maxwidth)] = 0;
	explored[2 + (11 * ed.maxwidth)] = 0;
	explored[3 + (11 * ed.maxwidth)] = 0;
	explored[4 + (11 * ed.maxwidth)] = 0;
}

void mapclass::showship()
{
	//remove the ship from the explored areas
	explored[2 + (10 * ed.maxwidth)] = 1;
	explored[3 + (10 * ed.maxwidth)] = 1;
	explored[4 + (10 * ed.maxwidth)] = 1;
	explored[2 + (11 * ed.maxwidth)] = 1;
	explored[3 + (11 * ed.maxwidth)] = 1;
	explored[4 + (11 * ed.maxwidth)] = 1;
}

// Centers the tower camera on the player position
void mapclass::realign_tower() {
	int i = obj.getplayer();
	ypos = obj.entities[i].yp - 120;

	if (ypos < 0)
		ypos = 0;
	bypos = ypos / 2;
}

void mapclass::resetplayer() {
	// Possibly warp if we died in a different room than our checkpoint is
	bool room_different = true;
	int old_tower = get_tower(game.roomx, game.roomy);
	int new_tower = get_tower(game.saverx, game.savery);

	// Towers use more than one screen, so compare tower ID
	if (towermode && old_tower == new_tower)
		room_different = false;

	// Obviously, identical death room and checkpoint room isn't different
	if (game.roomx == game.saverx && game.roomy == game.savery)
		room_different = false;

	if (room_different) {
		gotoroom(game.saverx, game.savery);
		// If in finalstretch, update the colors of entities immediately
		if (custommode && finalstretch)
			changefinalcol(final_mapcol);
	}

	game.deathseq = -1;
	int i = obj.getplayer();
	if(i>-1)
	{
		obj.entities[i].vx = 0;
		obj.entities[i].vy = 0;
		obj.entities[i].ax = 0;
		obj.entities[i].ay = 0;
		obj.entities[i].xp = game.savex;
		obj.entities[i].yp = game.savey;
		obj.entities[i].dir = game.savedir;
		obj.entities[i].colour = 0;
		game.lifeseq = 10;
		obj.entities[i].invis = true;

		if (towermode && old_tower != new_tower)
			realign_tower();
	}

	game.scmhurt = false; //Just in case the supercrewmate is fucking this up!
	if (game.supercrewmate)
	{
		if (game.roomx == game.scmprogress + 41)
		{
			game.scmprogress = game.roomx - 41;
		}
		else
		{
			game.scmprogress = game.roomx - 40;
		}
		if (game.scmprogress != 0)
		{
			game.scmmoveme = true;
		}
		else
		{
			game.scmmoveme = false;
		}
	}
}

// Returns tower row for given room, or -1 if invalid
int mapclass::tower_row(int rx, int ry) {
	if (custommode)
		return ed.tower_row(rx - 100, ry - 100);

	int tower = get_tower(rx, ry);
	if (tower == 1 && ry == 109)
		return 671;
	else if ((tower == 2 && ry == 53) || (tower == 3 && ry == 54))
		return 71;
	else if (!tower || (tower == 1 && ry != 104))
		return -1;
	return 0;
}

int mapclass::get_tower_offset(int tower, int ix, int iy, int *rx, int *ry, int ypos) {
	if (tower != get_tower(ix, iy))
		return -1;

	int rpos = tower_row(ix, iy) * 8;
	if (rpos >= 0 && ypos >= rpos && ypos < rpos + 240) {
		*rx += (ix - (*rx));
		*ry += (iy - (*ry));
		return ypos - rpos;
	}

	return -1;
}

// Returns y offset upon tower entry/exit. ypos is negative if entering.
// Returns -1 for an invalid boundary.
int mapclass::tower_connection(int *rx, int *ry, int ypos) {
	/* Figure out the location of tower connections */
	int ix, iy, rix, riy, rpos;
	rix = (*rx);
	riy = (*ry);
	ix = rix;
	iy = riy;

	if (ypos < 0) // entering
		return tower_row(ix, iy) * 8;

	/* Iterate all rooms connected to this tower and figure out our exit
	   position. */
	int ymin = 100;
	int ymax = 100 + ed.maxheight - 1;
	int xmin = 100;
	int xmax = 100 + ed.maxwidth - 1;
	int tower = get_tower(ix, iy);
	if (!custommode) {
		ymax = 119;
		if (rix < 100) {
			ymin = 52;
			ymax = 54;
		}
		xmin = rix;
		xmax = rix;
	}

	// Scan x from 0..max, but scan y based on closeness to current y
	for (ix = xmin; ix <= xmax; ix++) {
		for (iy = riy; iy >= ymin; iy--)
			if ((rpos = get_tower_offset(tower, ix, iy, rx, ry, ypos)) >= 0)
				return rpos;

		for (iy = riy + 1; iy <= ymax; iy++)
			if ((rpos = get_tower_offset(tower, ix, iy, rx, ry, ypos)) >= 0)
				return rpos;
	}

	// We failed to find an exit boundary!
	return -1;
}

/* Returns custom tower ID or for the maingame 1 (The Tower), 2 (Panic Room),
   3 (The Final Challenge) */
int mapclass::get_tower(int rx, int ry) {
	if (custommode)
		return ed.get_tower(rx - 100, ry - 100);

#if defined(MAKEANDPLAY)
	return 0;
#endif

	if (rx == 109)
		return 1; // The Tower

	if (rx == 49 && ry >= 52 && ry <= 53)
		return 2; // Panic Room

	if (rx == 51 && ry >= 53 && ry <= 54)
		return 3; // The Final Challenge

	return 0; // Not a tower
}

// Returns tower ID like get_tower() and updates entry with entry row
int mapclass::entering_tower(int rx, int ry, int *entry) {
	int tower = get_tower(rx, ry);
	if (!tower)
		return 0;

	*entry = tower_connection(&rx, &ry, -1);
	return tower;
}

// Moves player y appropriate and possibly change destination screen.
bool mapclass::leaving_tower(int *rx, int *ry, entityclass &obj) {
	int i = obj.getplayer();

	// Check if we're inside a valid exit boundary
	int yp = tower_connection(rx, ry, obj.entities[i].yp);

	if (yp < 0)
		return false;

	// Fix x position
	if (obj.entities[i].xp < -14) {
		(*rx)--;
		obj.entities[i].xp += 320;
	} else {
		if (obj.entities[i].xp >= 308)
			obj.entities[i].xp -= 320;
		(*rx)++;
	}

	obj.entities[i].yp = yp;
	return true;
}

void mapclass::custom_warpto(int ent)
{
	int ex = obj.entities[ent].behave;
	int ey = obj.entities[ent].para;
	int tower = obj.entities[ent].life;
	int rx, ry;
	if (!tower || !ed.find_tower(tower, rx, ry)) {
		rx = ex / 40;
		ry = ey / 30;
		ex -= (rx * 40);
		ey -= (ry * 30);
	}

	// Warp token destination offset
	ey += 2;
	rx += 100;
	ry += 100;

	warpto(rx, ry, obj.getplayer(), ex, ey);
	game.teleport = false;
	game.flashlight = 6;
	game.screenshake = 25;
}

void mapclass::warpto(int rx, int ry , int t, int tx, int ty)
{
	gotoroom(rx, ry);
	game.teleport = false;
	obj.entities[t].xp = tx * 8;
	obj.entities[t].yp = (ty * 8) - obj.entities[t].h;
	game.gravitycontrol = 0;
	if (towermode)
		realign_tower();
}

// Set map dimension to one containing this room if current doesn't
// Set to -1 if no dimension holds this room.
void mapclass::gotodimroom(int rx, int ry) {
#ifdef NO_CUSTOM_LEVELS
	return; // do nothing
#else
	if (!custommode || dimension < 0)
		return;

	// Dimensions doesn't use 100-indexing
	int ox = rx - 100;
	int oy = ry - 100;

	// First, check if the dimension we were in last is still valid
	int ix = ox;
	int iy = oy;
	dimensionwraparound(&ix, &iy);
	if (ix == ox && iy == oy)
		return;

	// Otherwise, find one that is
	Dimension *dim;
	for (int i = 0; (dim = getdimension(i)); i++) {
		dimension = i;
		ix = ox;
		iy = oy;
		dimensionwraparound(&ix, &iy);
		if (ix == ox && iy == oy) {
		ed.generatecustomminimap();
		return;
		}
	}

	dimension = -1;

	ed.generatecustomminimap();
#endif
}

void mapclass::gotoroom(int rx, int ry)
{
	if (graphics.noclear)
		FillRect(graphics.backBuffer,0x00000);

	//First, destroy the current room
	obj.removeallblocks();
	obj.removeallresurrectblocks();
	game.activetele = false;
	game.readytotele = 0;

	//Ok, let's save the position of all lines on the screen
	obj.linecrosskludge.clear();
	for (size_t i = 0; i < obj.entities.size(); i++)
	{
		if (obj.entities[i].type == 9)
		{
			//It's a horizontal line
			if (obj.entities[i].xp <= 0 || (obj.entities[i].xp + obj.entities[i].w) >= 312)
			{
				//it's on a screen edge
				obj.copylinecross(i);
			}
		}
	}

	int theplayer = obj.getplayer();
	for (int i = 0; i < (int) obj.entities.size(); i++)
	{
		if (i != theplayer)
		{
			removeentity_iter(i);
			theplayer--; //just in case indice of player is not 0
		}
	}

	game.door_up = rx + ((ry - 1) * 100);
	game.door_down = rx + ((ry + 1) * 100);
	game.door_right = rx + 1 + (ry * 100);
	game.door_left = rx -1 + (ry * 100);

	if (rx < game.roomx)
	{
		game.roomchangedir = 0;
	}
	else
	{
		game.roomchangedir = 1;
	}
	if (ry < game.roomy)
	{
		game.roomchangevdir = 0;
	}
	else
	{
		game.roomchangevdir = 1;
	}

	if (finalmode)
	{
		//Ok, what way are we moving?
		finalx = rx;
		finaly = ry;
		game.roomx = finalx;
		game.roomy = finaly;
		game.roomchange = true;
		rx = finalx;
		ry = finaly;

		if (game.roomy < 10)
		{
			game.roomy = 11;
			finaly = 11;
		}

		if(game.roomx>=41 && game.roomy>=48 && game.roomx<61 && game.roomy<68 )
		{
			game.currentroomdeaths = roomdeathsfinal[game.roomx - 41 + (20 * (game.roomy - 48))];
		}
		else
		{
			game.currentroomdeaths = 0;
		}

		//Final level for time trial
		if (game.intimetrial)
		{
			if (game.roomx == 46 && game.roomy == 54) music.niceplay(15); //Final level remix
		}
	}
#if !defined(NO_CUSTOM_LEVELS)
	else if (custommode)
	{
		// Get a positive modulo
		int ix = rx - 100;
		int iy = ry - 100;
		// Are we in a dimension?
		// Do this before we convert it back to 100-indexing
		if (dimension >= 0)
			dimensionwraparound(&ix, &iy);
		int ih = ed.mapheight;
		int iw = ed.mapwidth;
		ix = mod(ix, iw);
		iy = mod(iy, ih);
		game.roomx = ix + 100;
		game.roomy = iy + 100;
		game.roomchange = true;
	}
#endif
	else
	{
	game.roomx = rx;
	game.roomy = ry;
	game.roomchange = true;
	if (game.roomx < 100) game.roomx = 119;
	if (game.roomy < 100) game.roomy = 119;
	if (game.roomx > 119) game.roomx = 100;
	if (game.roomy > 119) game.roomy = 100;

	game.currentroomdeaths = roomdeaths[game.roomx - 100 + (ed.maxwidth * (game.roomy - 100))];

	//Alright, change music depending on where we are:
	//Tower
	if (game.roomx == 107 && game.roomy == 106) music.niceplay(4);
	if (game.roomx == 107 && game.roomy == 107) music.niceplay(4);
	if (game.roomx == 107 && game.roomy == 108) music.niceplay(4);
	if (game.roomx == 107 && game.roomy == 109) music.niceplay(4);
	if (game.roomx == 108 && game.roomy == 109)
	{
		if (graphics.setflipmode)
		{
			music.niceplay(9);
		}
		else
		{
			music.niceplay(2);
		}
	}
	if (game.roomx == 109)
	{
		if (graphics.setflipmode)
		{
			music.niceplay(9);
		}
		else
		{
			music.niceplay(2);
		}
	}
	//Warp Zone
	if (game.roomx == 112 && game.roomy == 101) music.niceplay(4);
	if (game.roomx == 113 && game.roomy == 101) music.niceplay(4);
	if (game.roomx == 113 && game.roomy == 102) music.niceplay(4);
	if (game.roomx == 114 && game.roomy == 101) music.niceplay(12);
	if (game.roomx == 115 && game.roomy == 101) music.niceplay(12);
	if (game.roomx == 115 && game.roomy == 102) music.niceplay(12);
	//Lab
	if (game.roomx == 101 && game.roomy == 115) music.niceplay(4);
	if (game.roomx == 100 && game.roomy == 115) music.niceplay(4);
	if (game.roomx == 101 && game.roomy == 116) music.niceplay(4);
	if (game.roomx == 100 && game.roomy == 116) music.niceplay(4);
	if (game.roomx == 102 && game.roomy == 116) music.niceplay(3);
	if (game.roomx == 102 && game.roomy == 117) music.niceplay(3);
	if (game.roomx == 101 && game.roomy == 117) music.niceplay(3);
	//Space Station
	if (game.intimetrial)
	{
		if (game.roomx == 111 && game.roomy == 112) music.niceplay(1);
		if (game.roomx == 111 && game.roomy == 113) music.niceplay(1);
		if (game.roomx == 112 && game.roomy == 114) music.niceplay(1);
		if (game.roomx == 112 && game.roomy == 115) music.niceplay(1);
	}
	else
	{
		if (game.roomx == 111 && game.roomy == 112) music.niceplay(1);
		if (game.roomx == 111 && game.roomy == 113) music.niceplay(1);
		if (game.roomx == 112 && game.roomy == 114) music.niceplay(4);
		if (game.roomx == 112 && game.roomy == 115) music.niceplay(4);
	}
	//Leaving the Ship
	if (game.roomx == 104 && game.roomy == 112) music.niceplay(4);
	}
	temp = rx + (ry * 100);
	loadlevel(game.roomx, game.roomy);


	//Do we need to reload the background?
	bool redrawbg = game.roomx != game.prevroomx || game.roomy != game.prevroomy;

	if(redrawbg)
	{
		graphics.backgrounddrawn = false; //Used for background caching speedup
	}
	graphics.foregrounddrawn = false; //Used for background caching speedup

	game.prevroomx = game.roomx;
	game.prevroomy = game.roomy;

	//a very special case: if entering the communication room, room 13,4 before tag 5 is set, set the game state to a background
	//textbox thingy. if tag five is not set when changing room, reset the game state. (tag 5 is set when you get back to the ship)
	if(!game.intimetrial && !custommode)
	{
		if (!obj.flags[5] && !finalmode)
		{
			game.state = 0;
			if (game.roomx == 113 && game.roomy == 104)
			{
				game.state = 50;
			}
		}
	}

	//Ok, kludge to fix lines in crossing rooms - if we're intersecting a gravity line right now, let's
	//set it to an inactive state.

	//Alright! So, let's look at our lines from the previous rooms, and determine if any of them are actually
	//continuations!

	temp = obj.getplayer();
	if(temp>-1)
	{
		obj.entities[temp].oldxp = obj.entities[temp].xp;
		obj.entities[temp].oldyp = obj.entities[temp].yp;
	}

	for (size_t i = 0; i < obj.entities.size(); i++)
	{
		if (obj.entities[i].type == 9)
		{
			//It's a horizontal line
			if (obj.entities[i].xp <= 0 || obj.entities[i].xp + obj.entities[i].w >= 312)
			{
				//it's on a screen edge
				for (j = 0; j < (int) obj.linecrosskludge.size(); j++)
				{
					if (obj.entities[i].yp == obj.linecrosskludge[j].yp)
					{
						//y's match, how about x's?
						//we're moving left:
						if (game.roomchangedir == 0)
						{
							if (obj.entities[i].xp + obj.entities[i].w >= 312 && obj.linecrosskludge[j].xp <= 0)
							{
								obj.revertlinecross(i, j);
							}
						}
						else
						{
							if (obj.entities[i].xp <= 0 && obj.linecrosskludge[j].xp + obj.linecrosskludge[j].w >= 312)
							{
								obj.revertlinecross(i, j);
							}
						}
					}
				}
			}
		}
	}
}

std::string mapclass::currentarea(int t)
{
	switch(t)
	{
	case 0:
		return "Dimension VVVVVV";
		break;
	case 1:
		return "Dimension VVVVVV";
		break;
	case 2:
		return "Laboratory";
		break;
	case 3:
		return "The Tower";
		break;
	case 4:
		return "Warp Zone";
		break;
	case 5:
		return "Space Station";
		break;
	case 6:
		return "Outside Dimension VVVVVV";
		break;
	case 7:
		return "Outside Dimension VVVVVV";
		break;
	case 8:
		return "Outside Dimension VVVVVV";
		break;
	case 9:
		return "Outside Dimension VVVVVV";
		break;
	case 10:
		return "Outside Dimension VVVVVV";
		break;
	case 11:
		return "The Tower";
		break;
	}
	return "???";
}

void mapclass::loadlevel(int rx, int ry)
{
	int t;
	if (!finalmode)
	{
		explored[rx - 100 + ((ry - 100) * ed.maxwidth)] = 1;
		if (rx == 109 && !custommode)
		{
			exploretower();
		}
	}


	roomtexton = false;
	roomtext.clear();

	obj.platformtile = 0;
	obj.customplatformtile=0;
	obj.vertplatforms = false;
	obj.horplatforms = false;
	roomname = "";
	background = 1;
	warpx = false;
	warpy = false;

	towermode = false;
	ypos = 0;
	extrarow = 0;

	//Custom stuff for warplines
	obj.customwarpmode=false;
	obj.customwarpmodevon=false;
	obj.customwarpmodehon=false;

	growing_vector<std::string> tmap;

	int tower_entry = 0;
	int newtower = entering_tower(rx, ry, &tower_entry);
	if (newtower) {
		// Set up baseline tower settings
		tdrawback = true;
		minitowermode = false;
		tower.minitowermode = false;
		minitowersize = 100;
		bscroll = 0;
		scrolldir = 0;

		tileset = 1;
		background = 3;
		towermode = true;

		cameramode = 0;
		colstate = 0;
		colsuperstate = 0;

		int i = obj.getplayer();
		obj.entities[i].yp += tower_entry;

		ypos = tower_entry;
		bypos = ypos/2;
	}

	if (finalmode)
	{
		t = 6;
		//check if we're in the towers
		if (rx == 49 && ry == 52)
		{
			//entered tower 1
			t = 7;
		}
		else if (rx == 49 && ry == 53)
		{
			//re entered tower 1
			t = 8;
		}
		else if (rx == 51 && ry == 54)
		{
			//entered tower 2
			t = 9;
		}
		else if (rx == 51 && ry == 53)
		{
			//re entered tower 2
			t = 10;
		}
	}
	else if (custommode)
	{
		t= 12;
	}
	else
	{
		t = area(rx, ry);

		if (t < 2) //on the world map, want to test if we're in the secret lab
		{
			if (rx >= 116)
			{
				if (ry >= 105)
				{
					if (ry <= 107)
					{
						if (rx == 119 && ry == 105)
						{
							//Ah, this is just a normal area
						}
						else
						{
							//in the secret lab! Crazy background!
							background = 2;
							if (rx == 116 && ry == 105) graphics.rcol = 1;
							if (rx == 117 && ry == 105) graphics.rcol = 5;
							if (rx == 118 && ry == 105) graphics.rcol = 4;
							if (rx == 117 && ry == 106) graphics.rcol = 2;
							if (rx == 118 && ry == 106) graphics.rcol = 0;
							if (rx == 119 && ry == 106) graphics.rcol = 3;
							if (rx == 119 && ry == 107) graphics.rcol = 1;
						}
					}
				}
			}
		}
	}

	if (rx == 119 && ry == 108 && !custommode)
	{
		background = 5;
		graphics.rcol = 3;
		warpx = true;
		warpy = true;
	}

	switch(t)
	{
#if !defined(MAKEANDPLAY)
	case 0:
	case 1: //World Map
		tileset = 1;
		extrarow = 1;
		tmap = otherlevel.loadlevel(rx, ry);
		fillcontent(tmap);
		roomname = otherlevel.roomname;
		tileset = otherlevel.roomtileset;
		//do the appear/remove roomname here

		if (otherlevel.roomtexton)
		{
			roomtexton = true;
			roomtext = std::vector<Roomtext>(otherlevel.roomtext);
		}
		break;
	case 2: //The Lab
		tmap = lablevel.loadlevel(rx, ry);
		fillcontent(tmap);
		roomname = lablevel.roomname;
		tileset = 1;
		background = 2;
		graphics.rcol = lablevel.rcol;
		break;
	case 3: //The Tower
		roomname = "The Tower";

		//All the entities for here are just loaded here; it's essentially one room after all


		obj.createentity(48, 5456, 10, 1, 505007); // (savepoint)
		obj.createentity(224, 4528, 10, 1, 505017); // (savepoint)
		obj.createentity(232, 4168, 10, 0, 505027); // (savepoint)
		obj.createentity(280, 3816, 10, 1, 505037); // (savepoint)
		obj.createentity(152, 3552, 10, 1, 505047); // (savepoint)
		obj.createentity(216, 3280, 10, 0, 505057); // (savepoint)
		obj.createentity(216, 4808, 10, 1, 505067); // (savepoint)
		obj.createentity(72, 3096, 10, 0, 505077); // (savepoint)
		obj.createentity(176, 2600, 10, 0, 505087); // (savepoint)
		obj.createentity(216, 2392, 10, 0, 505097); // (savepoint)
		obj.createentity(152, 1184, 10, 1, 505107); // (savepoint)
		obj.createentity(152, 912, 10, 1, 505117); // (savepoint)
		obj.createentity(152, 536, 10, 1, 505127); // (savepoint)
		obj.createentity(120, 5136, 10, 0, 505137); // (savepoint)
		obj.createentity(144, 1824, 10, 0, 505147); // (savepoint)
		obj.createentity(72, 2904, 10, 0, 505157); // (savepoint)
		obj.createentity(224, 1648, 10, 1, 505167); // (savepoint)
		obj.createentity(112, 5280, 10, 1, 50517); // (savepoint)

		obj.createentity(24, 4216, 9, 7); // (shiny trinket)
		obj.createentity(280, 3216, 9, 8); // (shiny trinket)
		break;
	case 4: //The Warpzone
		tmap = warplevel.loadlevel(rx, ry);
		fillcontent(tmap);
		roomname = warplevel.roomname;
		tileset = 1;
		background = 3;
		graphics.rcol = warplevel.rcol;
		graphics.backgrounddrawn = false;

		warpx = warplevel.warpx;
		warpy = warplevel.warpy;
		background = 5;
		if (warpy) background = 4;
		if (warpx) background = 3;
		if (warpx && warpy) background = 5;
		break;
	case 5: //Space station
		tmap = spacestation2.loadlevel(rx, ry);
		fillcontent(tmap);
		roomname = spacestation2.roomname;
		tileset = 0;
		break;
	case 6: //final level
		tmap = finallevel.loadlevel(finalx, finaly);
		fillcontent(tmap);
		roomname = finallevel.roomname;
		tileset = 1;
		background = 3;
		graphics.rcol = finallevel.rcol;
		graphics.backgrounddrawn = false;

		if (finalstretch)
		{
			background = 6;
		}
		else
		{
			warpx = finallevel.warpx;
			warpy = finallevel.warpy;
			background = 5;
			if (warpy) background = 4;
			if (warpx) background = 3;
			if (warpx && warpy) background = 5;
		}

		graphics.rcol = 6;
		changefinalcol(final_mapcol);
		break;
	case 7: //Final Level, Tower 1
	case 8:
		roomname = "Panic Room";
		minitowermode = true;
		tower.minitowermode = true;
		scrolldir = 1;

		tower.loadminitower1();
		break;
	case 9: //Final Level, Tower 2
	case 10:
		roomname = "The Final Challenge";
		minitowermode = true;
		tower.minitowermode = true;

		tower.loadminitower2();

		obj.createentity(56, 556, 11, 136); // (horizontal gravity line)
		obj.createentity(184, 592, 10, 0, 50500); // (savepoint)
		obj.createentity(184, 644, 11, 88); // (horizontal gravity line)
		obj.createentity(56, 460, 11, 136); // (horizontal gravity line)
		obj.createentity(216, 440, 10, 0, 50501); // (savepoint)
		obj.createentity(104, 508, 11, 168); // (horizontal gravity line)
		obj.createentity(219, 264, 12, 56); // (vertical gravity line)
		obj.createentity(120, 332, 11, 96); // (horizontal gravity line)
		obj.createentity(219, 344, 12, 56); // (vertical gravity line)
		obj.createentity(224, 332, 11, 48); // (horizontal gravity line)
		obj.createentity(56, 212, 11, 144); // (horizontal gravity line)
		obj.createentity(32, 20, 11, 96); // (horizontal gravity line)
		obj.createentity(72, 156, 11, 200); // (horizontal gravity line)
		break;
	case 11: //Tower Hallways //Content is held in final level routine
	{
		tmap = finallevel.loadlevel(rx, ry);
		fillcontent(tmap);
		roomname = finallevel.roomname;
		tileset = 2;
		if (rx == 108)
		{
			background = 7;
			rcol = 15;
		}
		if (rx == 110)
		{
			background = 8;
			rcol = 10;
		}
		if (rx == 111)
		{
			background = 9;
			rcol = 0;
		}
	}
		break;
					#endif
#if !defined(NO_CUSTOM_LEVELS)
	case 12: //Custom level
		int curlevel=(rx-100)+((ry-100)*ed.maxwidth);
		game.customcol=ed.getlevelcol(curlevel)+1;
		obj.customplatformtile=game.customcol*12;

		if (newtower) {
			minitowermode = true;
			tower.minitowermode = true;
			minitowersize = ed.tower_size(newtower);
			scrolldir = ed.tower_scroll(newtower);

			extrarow = 0;

			int ix = rx - 100;
			int iy;
			for (iy = 0; iy < ed.maxheight; iy++)
				if (ed.get_tower(ix, iy) == newtower)
					explored[ix + iy*ed.maxwidth] = 1;
		} else {
			switch(ed.level[curlevel].tileset){
			case 0: //Space Station
				tileset = 0;
				background = 1;
				break;
			case 1: //Outside
				tileset = 1;
				background = 1;
				break;
			case 2: //Lab
				tileset = 1;
				background = 2;
				graphics.rcol = ed.level[curlevel].tilecol;
				break;
			case 3: //Warp Zone/intermission
				tileset = 1;
				background = 6;
				break;
			case 4://Ship
				tileset = 1;
				background = 1;
				break;
			case 5://Tower
				tileset = 2;
				background = 10;
				graphics.rcol = ed.level[curlevel].tilecol;
				break;
			default:
				tileset = 1;
				background = 1;
				break;
			}

			//If screen warping, then override all that:
			graphics.backgrounddrawn = false;

			if (ed.level[curlevel].warpdir > 0) {
				if(ed.level[curlevel].warpdir==1){
					warpx=true;
					background=3;
					graphics.rcol = ed.getwarpbackground(rx-100,ry-100);
				}else if(ed.level[curlevel].warpdir==2){
					warpy=true;
					background=4;
					graphics.rcol = ed.getwarpbackground(rx-100,ry-100);
				}else if(ed.level[curlevel].warpdir==3){
					warpx=true; warpy=true;
					background = 5;
					graphics.rcol = ed.getwarpbackground(rx-100,ry-100);
				}
			}

			extrarow = 1;
		}

		ed.loadlevel(rx, ry, obj.altstates);
		int ymax = 30;
		if (newtower) {
			ymax = minitowersize;
			tower.loadcustomtower(ed.swapmap, ymax);
		} else {
			for (int edj = 0; edj < ymax; edj++) {
				for (int edi = 0; edi < 40; edi++) {
					contents[edi + vmult[edj]] = ed.swapmap[edi + vmult[edj]];
				}
			}
		}

		//If screen warping, then override all that:
		bool redrawbg = game.roomx != game.prevroomx || game.roomy != game.prevroomy;
		if(redrawbg){
			graphics.backgrounddrawn = false;
		}
		if(ed.level[curlevel].warpdir>0){
			if(ed.level[curlevel].warpdir==1){
			warpx=true;
			background=3;
			graphics.rcol = ed.getwarpbackground(rx-100,ry-100);
			}else if(ed.level[curlevel].warpdir==2){
			warpy=true;
			background=4;
			graphics.rcol = ed.getwarpbackground(rx-100,ry-100);
			}else if(ed.level[curlevel].warpdir==3){
			warpx=true; warpy=true;
			background = 5;
			graphics.rcol = ed.getwarpbackground(rx-100,ry-100);
			}
		}

		roomname="";
		if(ed.level[curlevel].roomname!=""){
			roomname=ed.level[curlevel].roomname;
		}

		roomtexton = false;
		roomtext.clear();

		//Entities have to be created HERE, akwardly
		int tempcheckpoints=0;
		int tempscriptbox=0;
		for(size_t edi=0; edi<edentity.size(); edi++){
			// If entity is in this room, create it
			int bx1, by1, bx2, by2;
			int tsx = (edentity[edi].x-(edentity[edi].x%40))/40;
			int tsy = (edentity[edi].y-(edentity[edi].y%30))/30;

			int thestate = -1;
			if (obj.altstates != 0)
				thestate = ed.getedaltstatenum(tsx, tsy, obj.altstates);
			int usethisaltstate;
			if (thestate == -1)
				usethisaltstate = 0;
			else
				usethisaltstate = ed.altstates[thestate].state;

			if (usethisaltstate != edentity[edi].state ||
				newtower != edentity[edi].intower)
				continue;

			int ex = edentity[edi].x * 8;
			int ey = edentity[edi].y * 8;
			if (!newtower) {
				/* Non-tower entity xy is stored with an xy offset denoting
				   the room */
				ex -= tsx * 40 * 8;
				ey -= tsy * 30 * 8;
			}
			ex += edentity[edi].subx;
			ey += edentity[edi].suby;

			/* Enemy or platform boundaries */
			bx1=ed.level[curlevel].enemyx1;
			by1=ed.level[curlevel].enemyy1;
			bx2=ed.level[curlevel].enemyx2;
			by2=ed.level[curlevel].enemyy2;

			if (edentity[edi].t == 2) {
				bx1=ed.level[curlevel].platx1;
				by1=ed.level[curlevel].platy1;
				bx2=ed.level[curlevel].platx2;
				by2=ed.level[curlevel].platy2;
			}

			/* Allow wrap-around if the room wraps */
			if (warpx && bx1 == 0 && bx2 == 320) {
				bx1 -= 100;
				bx2 += 100;
			}

			if (warpy && by1 == 0 && by2 == 240) {
				by1 -= 100;
				by2 += 100;
			}

			if (newtower || (tsx == rx-100 && tsy == ry-100)) {
			switch (edentity[edi].t){
			case 1: // Enemies
				obj.customenemy=ed.level[curlevel].enemytype;
				obj.createentity(ex, ey, 56, edentity[edi].p1, ed.level[curlevel].enemyv + edentity[edi].p2, bx1, by1, bx2, by2);
				break;
			case 2: // Platforms and Threadmills
				/* Conveyors */
				if (edentity[edi].p1 > 4) {
					if (edentity[edi].p1 > 8)
						break;

					obj.createentity(ex, ey, 2, edentity[edi].p1 + 3, 4);
					break;
				}

				/* Just in case */
				if (edentity[edi].p1 < 0)
					break;

				obj.createentity(ex, ey, 2, edentity[edi].p1, ed.level[curlevel].platv + edentity[edi].p2, bx1, by1, bx2, by2);
				break;
			case 3: // Disappearing platforms
				obj.createentity(ex, ey, 3);
				break;
			case 5: // Flip tokens
				obj.createentity(ex, ey, 5, edentity[edi].p1);
				break;
			case 8: // Coins
				obj.createentity(ex, ey, 8, ed.findcoin(edi), edentity[edi].p1);
				break;
			case 9: // Trinkets
				obj.createentity(ex, ey, 9, ed.findtrinket(edi));
				break;
			case 10: // Checkpoints
				obj.createentity(ex, ey, 10, edentity[edi].p1, ((rx+(ry*100))*20)+tempcheckpoints);
				tempcheckpoints++;
				break;
			case 11: // Gravity Lines
				if (edentity[edi].p1==0) //Horizontal
					obj.createentity(edentity[edi].p2 * 8, ey + 4, 11, edentity[edi].p3);
				else // Vertical
					obj.createentity(ex + 3, edentity[edi].p2 * 8, 12, edentity[edi].p3);
				break;
			case 13: // Warp Tokens
				obj.createentity(ex, ey, 13, edentity[edi].p1, edentity[edi].p2, edentity[edi].p3);
				break;
			case 14: // Round teleporter (whyy)
				obj.createentity(ex, ey, 14);
				break;
			case 15: // Collectable crewmate
				obj.createentity(ex, ey, 55, ed.findcrewmate(edi), edentity[edi].p1, edentity[edi].p2);
				break;
			case 17: { // Roomtext!
				roomtexton = true;
				Roomtext text;
				text.x = ex / 8;
				text.y = ey / 8;
				text.subx = edentity[edi].subx;
				text.suby = edentity[edi].suby;
				text.text = edentity[edi].scriptname;
				roomtext.push_back(text);
				break;
			}
			case 18: { // Terminals
				obj.customscript = edentity[edi].scriptname;

				int usethistile = edentity[edi].p1;
				int usethisy = ey;

				// This isn't a boolean: we just swap 0 and 1 around and leave the rest alone
				if (usethistile == 0) {
					usethistile = 1; // Unflipped
				} else if (usethistile == 1) {
					usethistile = 0; // Flipped
					usethisy -= 8;
				}

				obj.createentity(ex, usethisy+8, 20, usethistile);
				int ew = 16;
				if (!IS_VCE_LEVEL) {
					// Misalign the activity zone exactly like it's misaligned in vanilla
					ex -= 8; // only modify ex here because it's also used in the createentity above
					ew += 4;
				}
				obj.createblock(5, ex, usethisy+8, ew, 16, 35);
				break;
			}
			case 19: // Script Box
				game.customscript[tempscriptbox]=edentity[edi].scriptname;
				if (edentity[edi].onetime)
					obj.kludgeonetimescript = true;
				obj.createblock(1, ex, ey, edentity[edi].p1*8 + edentity[edi].p3,
								edentity[edi].p2*8 + edentity[edi].p4, 300+tempscriptbox);
				tempscriptbox++;
				break;
			case 20: // Activity Zone
				obj.customprompt = edentity[edi].activityname;
				obj.customcolour = edentity[edi].activitycolor;
				obj.customscript = edentity[edi].scriptname;
				obj.createblock(ACTIVITY, ex, ey, edentity[edi].p1*8 + edentity[edi].p3,
								edentity[edi].p2*8 + edentity[edi].p4, 100);
				//tempscriptbox++;
				break;
			case 50: // Warp Lines
				obj.customwarpmode=true;
				if (edentity[edi].p1==0) //
					obj.createentity(ex, (edentity[edi].p2*8), 51, edentity[edi].p3);
				else if (edentity[edi].p1==1) //Horizontal, right
					obj.createentity(ex + 4, (edentity[edi].p2*8), 52, edentity[edi].p3);
				else if (edentity[edi].p1==2) //Vertical, top
					obj.createentity((edentity[edi].p2*8), ey + 7, 53, edentity[edi].p3);
				else if (edentity[edi].p1==3)
					obj.createentity((edentity[edi].p2*8), ey, 54, edentity[edi].p3);
				break;
			case 999: // ?
				obj.customenemy=ed.level[curlevel].enemytype;
				int temp;
				temp = obj.createentity(ex, ey, 1, 0, 0, -10000, -10000, 10000, 10000);
				obj.entities[temp].tile = 0;
				obj.entities[temp].colour = 102;
				obj.entities[temp].animate = 0;
				obj.entities[temp].w = 464;
				obj.entities[temp].h = 320;
				obj.entities[temp].size = 11;
				obj.entities[temp].harmful = false;
				break;
			}
			}
		}
		if (ed.grayenemieskludge)
			ed.grayenemieskludge = false;

		//do the appear/remove roomname here
		break;
#endif
	}
	//The room's loaded: now we fill out damage blocks based on the tiles.
	if (towermode)
	{

	}
	else
	{
		for (int j = 0; j < 29 + extrarow; j++)
		{
			for (int i = 0; i < 40; i++)
			{
				//Damage blocks
				if(tileset==0)
				{
					if (contents[i + vmult[j]] == 6 || contents[i + vmult[j]] == 8)
					{
						//sticking up
						obj.createblock(2, (i * 8), (j * 8)+4, 8, 4);
					}
					if (contents[i + vmult[j]] == 7 || contents[i + vmult[j]] == 9)
					{
						//Sticking down
						obj.createblock(2, (i * 8), (j * 8), 8, 4);
					}
					if (contents[i + vmult[j]] == 49 || contents[i + vmult[j]] == 50)
					{
						//left or right
						obj.createblock(2, (i * 8), (j * 8)+3, 8, 2);
					}
				}
				else if(tileset==1)
				{
					//if (contents[i + vmult[j]] >= 6 && contents[i + vmult[j]] <= 9) obj.createblock(2, (i * 8), (j * 8)+1, 8, 6);
					//if (contents[i + vmult[j]] >= 49 && contents[i + vmult[j]] <= 79) obj.createblock(2, (i * 8) + 1, (j * 8) + 1, 6, 6);
					if ((contents[i + vmult[j]] >= 63 && contents[i + vmult[j]] <= 74) ||
							(contents[i + vmult[j]] >= 6 && contents[i + vmult[j]] <= 9))
					{
						//sticking up) {
						if (contents[i + vmult[j]] < 10) contents[i + vmult[j]]++;
						//sticking up
						if(contents[i + vmult[j]]%2==0)
						{
							obj.createblock(2, (i * 8), (j * 8), 8, 4);
						}
						else
						{
							//Sticking down
							obj.createblock(2, (i * 8), (j * 8) + 4, 8, 4);
						}
						if (contents[i + vmult[j]] < 11) contents[i + vmult[j]]--;
					}
					if (contents[i + vmult[j]] >= 49 && contents[i + vmult[j]] <= 62)
					{
						//left or right
						obj.createblock(2, (i * 8), (j * 8)+3, 8, 2);
					}
				}
				else if(tileset==2)
				{
					int tile = (contents[i + vmult[j]]) % 30;
					if (tile == 6 || tile == 8)
					{
						//sticking up
						obj.createblock(2, (i * 8), (j * 8)+4, 8, 4);
					}
					if (tile == 7 || tile == 9)
					{
						//Sticking down
						obj.createblock(2, (i * 8), (j * 8), 8, 4);
					}
					if (tile == 10 || tile == 11)
					{
						//left or right
						obj.createblock(2, (i * 8), (j * 8)+3, 8, 2);
					}
				}
				//Breakable blocks
				if (contents[i + vmult[j]] == 10 && tileset!=2)
				{
					contents[i + vmult[j]] = 0;
					obj.createentity(i * 8, j * 8, 4);
				}
				//Directional blocks
				if (contents[i + vmult[j]] >= 14 && contents[i + vmult[j]] <= 17 && tileset!=2)
				{
					obj.createblock(3, i * 8, j * 8, 8, 8, contents[i + vmult[j]]-14);
				}
			}
		}
	}

	for (size_t i = 0; i < obj.entities.size(); i++)
	{
		if (obj.entities[i].type == 1 && obj.entities[i].behave >= 8 && obj.entities[i].behave < 10)
		{
			int thetile = towermode && minitowermode ? 12 : 1;
			//put a block underneath
			temp = obj.entities[i].xp / 8.0f;
			temp2 = obj.entities[i].yp / 8.0f;
			settile(temp, temp2, thetile);
			settile(temp+1, temp2, thetile);
			settile(temp+2, temp2, thetile);
			settile(temp+3, temp2, thetile);
			if (obj.entities[i].w == 64)
			{
				settile(temp+4, temp2, thetile);
				settile(temp+5, temp2, thetile);
				settile(temp+6, temp2, thetile);
				settile(temp+7, temp2, thetile);
			}
		}
	}

	//Special scripting: Create objects and triggers based on what crewmembers are rescued.
	if (!finalmode)
	{
		//First up: the extra bits:
		//Vermilion's quest:
		if (rx == 100 && ry == 105)  //On path to verdigris
		{
			if (game.crewstats[3] && !game.crewstats[4])
			{
				obj.createentity(87, 105, 18, 15, 0, 18);
				obj.createblock(5, 87-32, 0, 32+32+32, 240, 3);
			}
		}
		else if (rx == 107 && ry == 100)	//victoria
		{
			if (game.crewstats[3] && !game.crewstats[5])
			{
				obj.createentity(140, 137, 18, 15, 0, 18);
				obj.createblock(5, 140-32, 0, 32+32+32, 240, 3);
			}
		}
		else if (rx == 114 && ry == 109)
		{
			if (game.crewstats[3] && !game.crewstats[2])
			{
				obj.createentity(235, 81, 18, 15, 0, 18);
				obj.createblock(5, 235-32, 0, 32+32+32, 240, 3);
			}
		}

		//Verdigris fixing the ship
		if (rx == 101 && ry == 109)
		{
			if (game.crewstats[4])
			{
				if(game.crewrescued()>4 && game.crewrescued()!=6)
				{
					obj.createentity(175, 121, 18, 13, 0, 18);
					obj.createblock(5, 175-32, 0, 32+32+32, 240, 4);
				}
			}
		}
		else if (rx == 103 && ry == 109)
		{
			if (game.crewstats[4])
			{
				if(game.crewrescued()<=4 && game.crewrescued()!=6)
				{
					obj.createentity(53, 161, 18, 13, 1, 18);
					obj.createblock(5, 53-32, 0, 32+32+32, 240, 4);
				}
			}
		}

		if (rx == 104 && ry == 111)
		{
			//Red
			//First: is he rescued?
			if (game.crewstats[3])
			{
				//If so, red will always be at his post
				obj.createentity(107, 121, 18, 15, 0, 18);
				//What script do we use?
				obj.createblock(5, 107-32, 0, 32+32+32, 240, 3);
			}
		}
		else if (rx == 103 && ry == 111)
		{
			//Yellow
			//First: is he rescued?
			if (game.crewstats[2])
			{
				obj.createentity(198, 105, 18, 14, 0, 18);
				//What script do we use?
				obj.createblock(5, 198-32, 0, 32+32+32, 240, 2);
			}
		}
		else if (rx == 103 && ry == 110)
		{
			//Green
			//First: is he rescued?
			if (game.crewstats[4])
			{
				obj.createentity(242, 177, 18, 13, 0, 18);
				//What script do we use?
				obj.createblock(5, 242-32, 177-20, 32+32+32, 40, 4);
			}
		}
		else if (rx == 104 && ry == 110)
		{
			//Purple
			//First: is she rescued?
			if (game.crewstats[1])
			{
				obj.createentity(140, 177, 18, 20, 0, 18);
				//What script do we use?
				obj.createblock(5, 140-32, 0, 32+32+32, 240, 1);
			}
		}
		else if (rx == 102 && ry == 110)
		{
			//Blue
			//First: is she rescued?
			if (game.crewstats[5])
			{
				//A slight varation - she's upside down
				obj.createentity(249, 62, 18, 16, 0, 18);
				j = obj.getcrewman(5);
				obj.entities[j].rule = 7;
				obj.entities[j].tile +=6;
				//What script do we use?
				obj.createblock(5, 249-32, 0, 32+32+32, 240, 5);
			}
		}
	}

	//Make sure our crewmates are facing the player if appliciable
	for (size_t i = 0; i < obj.entities.size(); i++)
	{
		if (obj.entities[i].rule == 6 || obj.entities[i].rule == 7)
		{
			if (obj.entities[i].state == 18)
			{
				//face the player
				j = obj.getplayer();
				if (obj.entities[j].xp > obj.entities[i].xp + 5)
				{
					obj.entities[i].dir = 1;
				}
				else if (obj.entities[j].xp < obj.entities[i].xp - 5)
				{
					obj.entities[i].dir = 0;
				}
			}
		}
	}
}

void mapclass::updatetowerentcol(int col)
{
	// Update all sprite-based entities in the tower on "stable" colors
	// TODO: Interpolate between each stable color during transitions between them

	// WARNING: `59 + col` is duplicated from editorclass::getlevelcol()!
	int entcol = ed.getenemycol(59 + col);
	int plattile = ed.gettowerplattile(col * 5);

	// I don't want to deal with main game stuff
	if (!custommode)
		return;

	// Basically copied from mapclass::changefinalcol()
	for (size_t i = 0; i < obj.entities.size(); i++) {
		if (obj.entities[i].type == 1) { // Something with a movement behavior
			if (obj.entities[i].animate == 10 || obj.entities[i].animate == 11) { // Conveyor
				obj.entities[i].tile = 4 + plattile*12;
				if (obj.entities[i].animate == 10)
					obj.entities[i].tile += 4;
			} else if (obj.entities[i].isplatform) { // Moving platform
				obj.entities[i].tile = plattile*12;
			} else { // Just an enemy
				if (obj.entities[i].colour == 102)
					; // If flashy, do nothing!
				else
					obj.entities[i].colour = entcol;
			}
		} else if (obj.entities[i].type == 2) { // Disappearing platform
			if (obj.entities[i].state == 3)
				// It's disappeared, so its tile is offset, so we have to correct for that offset
				obj.entities[i].tile = 4 + plattile*12;
			else
				// Normal
				obj.entities[i].tile = plattile * 12;

			if (obj.entities[i].state == 5)
				// Extra kludge for when it respawns
				obj.entities[i].tile += 3 - obj.entities[i].life/3;
		}
	}
}

Dimension* mapclass::getdimension(int index)
{
	// Return a pointer to the given dimension
	// Does important error checking to make sure both the index and the dimension are valid!
	// Make sure to check for NULL
	//
	// See below for version that automatically uses map.dimension
	if (index < 0 || index >= (int) ed.dimensions.size())
		return NULL;

	Dimension* dim = &ed.dimensions[index];

	// Dimensions cannot overlap themselves
	// and they have to have positive dimensions
	if (dim->w > ed.mapwidth || dim->h > ed.mapheight
	|| dim->w <= 0 || dim->h <= 0)
		return NULL;

	return dim;
}

// This is the above, but it automatically uses map.dimension
Dimension* mapclass::getdimension()
{
	return getdimension(dimension);
}

void mapclass::dimensionwraparound(int* rx, int* ry)
{
	// If rx/ry is outside the current dimension, wrap it around!
	// rx/ry here is 0-indexed
	// NOTE: Depends on game.roomchangedir and game.roomchangevdir

	Dimension* dim = getdimension();
	if (dim == NULL)
		return;

	// If we're negative from the dimension's point of view, correct for it
	if (*rx < dim->x)
		*rx += ed.mapwidth;
	if (*ry < dim->y)
		*ry += ed.mapheight;

	// Correct for the space in between the ends of the dimension depending on which direction we're going
	if (*rx > dim->w && game.roomchangedir == 0)
		*rx -= ed.mapwidth - dim->w;
	if (*ry > dim->h && game.roomchangevdir == 0)
		*ry -= ed.mapheight - dim->h;

	// Translate to dimension's coordinate space
	*rx -= dim->x;
	*ry -= dim->y;

	// Wrap around
	*rx = (*rx) % dim->w;
	*ry = (*ry) % dim->h;

	// Translate back to normal coordinate space
	*rx += dim->x;
	*ry += dim->y;
}

void twoframedelayfix()
{
	// Kludge to remove 2-frame-delay when loading init scripts for a room
	if (IS_VCE_LEVEL && game.deathseq == -1 && obj.checktrigger() > -1 && obj.activetrigger >= 300 && !script.nointerrupt) {
		game.newscript = "custom_" + game.customscript[obj.activetrigger - 300];
		obj.kludgeonetimescript = true;
		obj.removetrigger(obj.activetrigger);
		game.state = 0;
		script.callstack.clear();
		script.load(game.newscript);
	}
}
