#ifndef KEYPOLL_H
#define KEYPOLL_H

#include <string>
#include <vector>
#include "Game.h"
#include <map> // FIXME: I should feel very bad for using C++ -flibit

#include "SDL.h"

enum Kybrd
{
	KEYBOARD_UP = SDLK_UP,
	KEYBOARD_DOWN = SDLK_DOWN,
	KEYBOARD_LEFT = SDLK_LEFT,
	KEYBOARD_RIGHT = SDLK_RIGHT,
	KEYBOARD_ENTER = SDLK_RETURN,
	KEYBOARD_SPACE = SDLK_SPACE,

	KEYBOARD_w = SDLK_w,
	KEYBOARD_s = SDLK_s,
	KEYBOARD_a = SDLK_a,
	KEYBOARD_d = SDLK_d,
	KEYBOARD_m = SDLK_m,

	KEYBOARD_v = SDLK_v,
	KEYBOARD_z = SDLK_z,

	KEYBOARD_BACKSPACE = SDLK_BACKSPACE
};

class KeyPoll
{
public:
	std::map<SDL_Keycode, bool> keymap;

	bool isActive = false;

	bool resetWindow = false;

	bool escapeWasPressedPreviously = false;
	bool quitProgram = false;
	bool toggleFullscreen = false;

	int sensitivity = 0;

	void setSensitivity(int _value);

	KeyPoll();

	void enabletextentry();

	void disabletextentry();

	void Poll();

	bool isDown(SDL_Keycode key);

	bool isUp(SDL_Keycode key);

	bool isDown(growing_vector<SDL_GameControllerButton> buttons);
	bool isDown(SDL_GameControllerButton button);
	bool controllerButtonDown();
	bool controllerWantsLeft(bool includeVert);
	bool controllerWantsRight(bool includeVert);

	int leftbutton, rightbutton, middlebutton = 0;
	int mx, my = 0;

	bool textentrymode = false;
	int keyentered, keybufferlen = 0;
	bool pressedbackspace = false;
	std::string keybuffer;

	bool linealreadyemptykludge;

private:
	std::map<SDL_JoystickID, SDL_GameController*> controllers;
	std::map<SDL_GameControllerButton, bool> buttonmap;
	int xVel, yVel = 0;
	bool useFullscreenSpaces = false;
	Uint32 wasFullscreen = 0;
};


#endif /* KEYPOLL_H */
