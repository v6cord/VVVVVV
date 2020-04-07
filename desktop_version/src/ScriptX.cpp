#include <optional>
#include <algorithm>
#include <iostream>
#include <cstring>
#include <cstdlib>
#include <vector>
#include "ScriptX.h"
#include "Graphics.h"

#include "Entity.h"
#include "Music.h"
#include "KeyPoll.h"
#include "Map.h"
#include "FileSystemUtils.h"
#include "Utilities.h"
#include "Game.h"
#include "Enums.h"

scriptx::scriptx()
{
	async = false;
	running = true;
	commands.clear();
	position = 0;
	scriptdelay = 0;
}

growing_vector<std::string> scriptx::tokenize(std::string command) {

	growing_vector<std::string> temp;
	std::string tempword = "";
    
	for (size_t i = 0; i < command.length(); i++)
	{
		std::string currentletter = command.substr(i, 1);
		if (currentletter == "(" || currentletter == ")" || currentletter == ",")
		{
			std::transform(tempword.begin(), tempword.end(), tempword.begin(), ::tolower);
			temp.push_back(tempword);
			tempword = "";
		}
		else if (currentletter != " ")
		{
			tempword += currentletter;
		}
	}

	temp.push_back(tempword);
    temp.push_back("");
	return temp;
}


void scriptx::update()
{
	while(running && scriptdelay == 0) {
		if (position < (int)commands.size()) {
			// Let's fill the words vector

			growing_vector<std::string> words;

			words = tokenize(commands[position]);

			if (words[0] == "test") {
				music.playef(11);
			}

			position++;
		} else {
			running = false;
		}
	}

	if(scriptdelay>0) scriptdelay--;
}
