#include "UtilityClass.h"

#include <SDL.h>

#include <sstream>

#include <algorithm>

/* Used by UtilityClass::GCString to generate a button list */
const char *GCChar(SDL_GameControllerButton button)
{
	if (button == SDL_CONTROLLER_BUTTON_A)
	{
		return "A";
	}
	else if (button == SDL_CONTROLLER_BUTTON_B)
	{
		return "B";
	}
	else if (button == SDL_CONTROLLER_BUTTON_X)
	{
		return "X";
	}
	else if (button == SDL_CONTROLLER_BUTTON_Y)
	{
		return "Y";
	}
	else if (button == SDL_CONTROLLER_BUTTON_BACK)
	{
		return "BACK";
	}
	else if (button == SDL_CONTROLLER_BUTTON_GUIDE)
	{
		return "GUIDE";
	}
	else if (button == SDL_CONTROLLER_BUTTON_START)
	{
		return "START";
	}
	else if (button == SDL_CONTROLLER_BUTTON_LEFTSTICK)
	{
		return "L3";
	}
	else if (button == SDL_CONTROLLER_BUTTON_RIGHTSTICK)
	{
		return "R3";
	}
	else if (button == SDL_CONTROLLER_BUTTON_LEFTSHOULDER)
	{
		return "LB";
	}
	else if (button == SDL_CONTROLLER_BUTTON_RIGHTSHOULDER)
	{
		return "RB";
	}
	SDL_assert(0 && "Unhandled button!");
	return NULL;
}

bool is_number(const std::string& str)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (!std::isdigit(static_cast<unsigned char>(str[i])) && !(i == 0 && str[0] == '-'))
		{
			return false;
		}
	}
	return true;
}

int ss_toi( std::string _s )
{
	std::istringstream i(_s);
	int x = 0;
	i >> x;
	return x;
}

std::vector<std::string> split( const std::string &s, char delim, std::vector<std::string> &elems )
{
	std::stringstream ss(s);
	std::string item;
	while(std::getline(ss, item, delim))
	{
		elems.push_back(item);
	}
	return elems;
}

std::vector<std::string> split( const std::string &s, char delim )
{
	std::vector<std::string> elems;
	return split(s, delim, elems);
}

UtilityClass::UtilityClass() :
glow(0),
	glowdir(0)
{
	for (int i = 0; i < 30; i++)
	{
		splitseconds.push_back(int((i * 100) / 30));
	}

	slowsine = 0;
	globaltemp = 0;
	temp = 0;
	temp2 = 0;
}

std::string UtilityClass::String( int _v )
{
	std::ostringstream os;
	os << _v;
	return(os.str());
}

std::string UtilityClass::GCString(std::vector<SDL_GameControllerButton> buttons)
{
	std::string retval = "";
	for (size_t i = 0; i < buttons.size(); i += 1)
	{
		retval += GCChar(buttons[i]);
		if ((i + 1) < buttons.size())
		{
			retval += ",";
		}
	}
	return retval;
}

std::string UtilityClass::twodigits( int t )
{
	if (t < 10)
	{
		return "0" + String(t);
	}
	if (t >= 100)
	{
		return "??";
	}
	return String(t);
}

std::string UtilityClass::timestring( int t )
{
	//given a time t in frames, return a time in seconds
	std::string tempstring = "";
	temp = (t - (t % 30)) / 30;
	if (temp < 60)   //less than one minute
	{
		t = t % 30;
		tempstring = String(temp) + ":" + twodigits(splitseconds[t]);
	}
	else
	{
		temp2 = (temp - (temp % 60)) / 60;
		temp = temp % 60;
		t = t % 30;
		tempstring = String(temp2) + ":" + twodigits(temp) + ":" + twodigits(splitseconds[t]);
	}
	return tempstring;
}

std::string UtilityClass::number( int _t )
{
	const std::string ones_place[] = {"One", "Two", "Three", "Four", "Five", "Six", "Seven", "Eight", "Nine"};
	const std::string tens_place[] = {"Ten", "Twenty", "Thirty", "Forty", "Fifty", "Sixty", "Seventy", "Eighty", "Ninety"};
	const std::string teens[] = {"Eleven", "Twelve", "Thirteen", "Fourteen", "Fifteen", "Sixteen", "Seventeen", "Eighteen", "Nineteen"};

	if (_t < 0)
	{
		return "???";
	}
	else if (_t > 100)
	{
		return "Lots";
	}
	else if (_t == 0)
	{
		return "Zero";
	}
	else if (_t == 100)
	{
		return "One Hundred";
	}
	else if (_t >= 1 && _t <= 9)
	{
		return ones_place[_t-1];
	}
	else if (_t >= 11 && _t <= 19)
	{
		return teens[_t-11];
	}
	else if (_t % 10 == 0)
	{
		return tens_place[(_t/10)-1];
	}
	else
	{
		return tens_place[(_t/10)-1] + " " + ones_place[(_t%10)-1];
	}
}

bool UtilityClass::intersects( SDL_Rect A, SDL_Rect B )
{
	return (SDL_HasIntersection(&A, &B) == SDL_TRUE);
}

void UtilityClass::updateglow()
{
        if (freezeglow) return;
	slowsine++;
	if (slowsine >= 64) slowsine = 0;

	if (glowdir == 0) {
		glow+=2;
		if (glow >= 62) glowdir = 1;
	}else {
		glow-=2;
		if (glow < 2) glowdir = 0;
	}
}

bool is_positive_num(const std::string& str, bool hex)
{
	for (size_t i = 0; i < str.length(); i++)
	{
		if (hex)
		{
			if (!std::isxdigit(static_cast<unsigned char>(str[i])))
			{
				return false;
			}
		}
		else
		{
			if (!std::isdigit(static_cast<unsigned char>(str[i])))
			{
				return false;
			}
		}
	}
	return true;
}

bool endsWith(const std::string& str, const std::string& suffix)
{
	if (str.size() < suffix.size())
	{
		return false;
	}
	return str.compare(
		str.size() - suffix.size(),
		suffix.size(),
		suffix
	) == 0;
}

std::string UtilityClass::getmusicname(int num) {
	std::string names[16] = {
		"Nothing",
		"Pushing Onwards",
		"Positive Force",
		"Potential for Anything",
		"Passion For Exploring",
		"Pause",
		"Presenting VVVVVV",
		"Plenary",
		"Predestined Fate",
        "Positive Force Reversed",
        "Popular Potpourri",
        "Pipe Dream",
        "Pressure Cooker",
        "Paced Energy",
        "Piercing the Sky",
        "Predestined Fate Remix"
	};
	return names[num];
}

// Parses a tilde-syntax string number and returns the new absolute number
int relativepos(int original, std::string parsethis)
{
    bool relative = parsethis.substr(0, 1) == "~";
    if (relative)
        parsethis = parsethis.substr(1, std::string::npos);

    int num = ss_toi(parsethis);

    if (!relative)
        return num;

    return original + num;
}

// Use this if you want to mutate a number instead
void relativepos(int* original, std::string parsethis)
{
    *original = relativepos(*original, parsethis);
}

bool parsebool(std::string parsethis)
{
    if (parsethis == "true"
    || parsethis == "yes"
    || parsethis == "y"
    || parsethis == "on"
    || parsethis == "enable"
    || parsethis == "show")
        return true;
    else if (parsethis == "false"
    || parsethis == "no"
    || parsethis == "n"
    || parsethis == "off"
    || parsethis == "disable"
    || parsethis == "hide")
        return false;

    return ss_toi(parsethis);
}

// Like relativepos, but for bools
bool relativebool(bool original, std::string parsethis)
{
	if (parsethis == "~" || (parsethis.substr(0, 1) == "~" && ss_toi(parsethis.substr(1, std::string::npos)) == 0))
		// Keep the current bool
		return original;
	else if (parsethis.substr(0, 1) == "~")
		// Invert the bool
		return !original;
	else
		return ss_toi(parsethis);
}

// Use this if you want to mutate a bool instead
void relativebool(bool* original, std::string parsethis)
{
	*original = relativebool(*original, parsethis);
}
