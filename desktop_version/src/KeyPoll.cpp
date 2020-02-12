#include "KeyPoll.h"
#include "Enums.h"
#include "Graphics.h"
#include <stdio.h>
#include <string.h>
#include <utf8/checked.h>
#ifdef __SWITCH__
#include <switch.h>
#endif

void KeyPoll::setSensitivity(int _value)
{
	switch (_value)
	{
		case 0:
			sensitivity = 28000;
			break;
		case 1:
			sensitivity = 16000;
			break;
		case 2:
			sensitivity = 8000;
			break;
		case 3:
			sensitivity = 4000;
			break;
		case 4:
			sensitivity = 2000;
			break;
	}

}

KeyPoll::KeyPoll()
{
	xVel = 0;
	yVel = 0;
	setSensitivity(2);

	quitProgram = 0;
	textentrymode=false;
	keybuffer="";
	leftbutton=0; realleftbutton=0; rightbutton=0; middlebutton=0;
	mx=0; my=0;
	resetWindow = 0;
	toggleFullscreen = false;
	pressedbackspace=false;

	useFullscreenSpaces = false;
	if (strcmp(SDL_GetPlatform(), "Mac OS X") == 0)
	{
		useFullscreenSpaces = true;
		const char *hint = SDL_GetHint(SDL_HINT_VIDEO_MAC_FULLSCREEN_SPACES);
		if (hint != NULL)
		{
			useFullscreenSpaces = (strcmp(hint, "1") == 0);
		}
	}

	linealreadyemptykludge = false;
}

void KeyPoll::enabletextentry()
{
	keybuffer="";
	textentrymode = true;
	SDL_StartTextInput();
        wantsOSKClose = SDL_IsScreenKeyboardShown(graphics.screenbuffer->m_window);
}

void KeyPoll::disabletextentry()
{
	textentrymode = false;
	SDL_StopTextInput();
        wantsOSKClose = false;
}

static void ctrl_click(KeyPoll* key, SDL_Event* evt, bool* was) {
    bool ctrl = key->keymap[SDLK_LCTRL] || key->keymap[SDLK_RCTRL] || key->isDown(SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
    bool left = key->realleftbutton;
    if (evt) {
        auto type = evt->type;
        if (type == SDL_KEYDOWN || type == SDL_KEYUP) {
            auto sym = evt->key.keysym.sym;
            if (sym == SDLK_LCTRL || sym == SDLK_RCTRL) {
                ctrl = type == SDL_KEYDOWN;
            }
        } else if (type == SDL_MOUSEBUTTONDOWN || type == SDL_MOUSEBUTTONUP) {
            if (evt->button.button == SDL_BUTTON_LEFT) left = type == SDL_MOUSEBUTTONDOWN;
        } else if (type == SDL_FINGERDOWN) {
            left = true;
        } else if (type == SDL_FINGERUP) {
            left = false;
        }
    }
    key->realleftbutton = left;
    if (ctrl && left) {
        key->rightbutton = true;
        *was = true;
    } else if (ctrl || left) {
        key->rightbutton = false;
        if (left) key->leftbutton = true;
        *was = false;
    }
}

void KeyPoll::Poll()
{
        if (fakekeytimer == 0) {
            keymap[fakekey] = 0;
            fakekeytimer = -1;
        } else if (fakekeytimer > 0) {
            keymap[fakekey] = 1;
            --fakekeytimer;
        } else if (fakekeytimer == -2) {
            keymap[fakekey] = 1;
        }

        if (delayed_left_time == 0) {
            delayed_left_time = -1;
            keymap[SDLK_LEFT] = 1;
        } else if (delayed_left_time > -10) {
            --delayed_left_time;
        }

        if (delayed_right_time == 0) {
            delayed_right_time = -1;
            keymap[SDLK_RIGHT] = 1;
        } else if (delayed_right_time > -10) {
            --delayed_right_time;
        }

	SDL_Event evt;
        bool was_ctrl_click = false;
	while (SDL_PollEvent(&evt))
	{
                ctrl_click(this, &evt, &was_ctrl_click);
		/* Keyboard Input */
		if (evt.type == SDL_KEYDOWN)
		{
			keymap[evt.key.keysym.sym] = true;
			if (evt.key.keysym.sym == SDLK_BACKSPACE)
			{
				pressedbackspace = true;
			}
			else if (	(	evt.key.keysym.sym == SDLK_RETURN ||
						evt.key.keysym.sym == SDLK_f	) &&
#ifdef __APPLE__ /* OSX prefers the command key over the alt keys. -flibit */
					keymap[SDLK_LGUI]	)
#else
					(	keymap[SDLK_LALT] ||
						keymap[SDLK_RALT]	)	)
#endif
			{
				toggleFullscreen = true;
			}

			if (textentrymode)
			{
				if (evt.key.keysym.sym == SDLK_BACKSPACE && keybuffer.size() > 0)
				{
					bool kbemptybefore = keybuffer.empty();
					std::string::iterator iter = keybuffer.end();
					utf8::prior(iter, keybuffer.begin());
					keybuffer = keybuffer.substr(0, iter - keybuffer.begin());
					if (!kbemptybefore && keybuffer.empty())
					{
						linealreadyemptykludge = true;
					}
				}
				else if (	evt.key.keysym.sym == SDLK_v &&
						keymap[SDLK_LCTRL]	)
				{
					keybuffer += SDL_GetClipboardText();
				}
			}
		}
		else if (evt.type == SDL_KEYUP)
		{
			keymap[evt.key.keysym.sym] = false;
			if (evt.key.keysym.sym == SDLK_BACKSPACE)
			{
				pressedbackspace = false;
			}
		}
		else if (evt.type == SDL_TEXTINPUT)
		{
			keybuffer += evt.text.text;
		}

		/* Mouse Input */
		else if (evt.type == SDL_MOUSEMOTION)
		{
			mx = evt.motion.x;
			my = evt.motion.y;
		}
		else if (!was_ctrl_click && evt.type == SDL_MOUSEBUTTONDOWN)
		{
			if (evt.button.button == SDL_BUTTON_LEFT)
			{
				mx = evt.button.x;
				my = evt.button.y;
				leftbutton = 1;
                                realleftbutton = 1;
			}
			else if (evt.button.button == SDL_BUTTON_RIGHT)
			{
				mx = evt.button.x;
				my = evt.button.y;
				rightbutton = 1;
			}
			else if (evt.button.button == SDL_BUTTON_MIDDLE)
			{
				mx = evt.button.x;
				my = evt.button.y;
				middlebutton = 1;
			}
		}
		else if (!was_ctrl_click && evt.type == SDL_MOUSEBUTTONUP)
		{
			if (evt.button.button == SDL_BUTTON_LEFT)
			{
				mx = evt.button.x;
				my = evt.button.y;
				leftbutton = 0;
                                realleftbutton = 0;
			}
			else if (evt.button.button == SDL_BUTTON_RIGHT)
			{
				mx = evt.button.x;
				my = evt.button.y;
				rightbutton=0;
			}
			else if (evt.button.button == SDL_BUTTON_MIDDLE)
			{
				mx = evt.button.x;
				my = evt.button.y;
				middlebutton=0;
			}
		}
                else if(game.gamestate == EDITORMODE && !was_ctrl_click && evt.type == SDL_FINGERDOWN)
                {
                    leftbutton = 1;
                    realleftbutton = 1;
                    mx = evt.tfinger.x * 320;
                    my = evt.tfinger.y * 240;
                }
                else if(game.gamestate == EDITORMODE && evt.type == SDL_FINGERMOTION)
                {
                    mx = evt.tfinger.x * 320;
                    my = evt.tfinger.y * 240;
                }
                else if(game.gamestate == EDITORMODE && !was_ctrl_click && evt.type == SDL_FINGERUP)
                {
                    leftbutton = 0;
                    realleftbutton = 0;
                    mx = evt.tfinger.x * 320;
                    my = evt.tfinger.y * 240;
                }
                else if(evt.type == SDL_FINGERDOWN)
                {
                    if (evt.tfinger.x < 0.5) {
                        if (keymap[SDLK_RIGHT] || delayed_right_time > -3) {
                            keymap[SDLK_v] = 1;
                            finger_buttons[evt.tfinger.fingerId] = SDLK_v;
                            if (delayed_right_time > -3) {
                                keymap[SDLK_RIGHT] = 0;
                            }
                            delayed_right_time = 0;
                        } else {
                            delayed_left_time = 0;
                            finger_buttons[evt.tfinger.fingerId] = SDLK_LEFT;
                        }
                    } else {
                        if (keymap[SDLK_LEFT] || delayed_left_time > -3) {
                            keymap[SDLK_v] = 1;
                            finger_buttons[evt.tfinger.fingerId] = SDLK_v;
                            if (delayed_left_time > -3) {
                                keymap[SDLK_LEFT] = 0;
                            }
                            delayed_left_time = 0;
                        } else {
                            delayed_right_time = 0;
                            finger_buttons[evt.tfinger.fingerId] = SDLK_RIGHT;
                        }
                    }
                }
                else if(evt.type == SDL_FINGERUP)
                {
                    auto iter = finger_buttons.find(evt.tfinger.fingerId);
                    if (iter != finger_buttons.end()) {
                        keymap[iter->second] = 0;
                        if (iter->second == SDLK_LEFT) {
                            delayed_left_time = -10;
                        } else if (iter->second == SDLK_RIGHT) {
                            delayed_right_time = -10;
                        }
                        finger_buttons.erase(iter);
                    }
                }

		/* Controller Input */
		else if (evt.type == SDL_CONTROLLERBUTTONDOWN)
		{
			buttonmap[(SDL_GameControllerButton) evt.cbutton.button] = true;
		}
		else if (evt.type == SDL_CONTROLLERBUTTONUP)
		{
			buttonmap[(SDL_GameControllerButton) evt.cbutton.button] = false;
		}
		else if (evt.type == SDL_CONTROLLERAXISMOTION)
		{
			if (evt.caxis.axis == SDL_CONTROLLER_AXIS_LEFTX)
			{
				if (	evt.caxis.value > -sensitivity &&
					evt.caxis.value < sensitivity	)
				{
					xVel = 0;
				}
				else
				{
					xVel = (evt.caxis.value > 0) ? 1 : -1;
				}
			}
			if (evt.caxis.axis == SDL_CONTROLLER_AXIS_LEFTY)
			{
				if (	evt.caxis.value > -sensitivity &&
					evt.caxis.value < sensitivity	)
				{
					yVel = 0;
				}
				else
				{
					yVel = (evt.caxis.value > 0) ? 1 : -1;
				}
			}
			if (evt.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTX)
			{
				if (	evt.caxis.value > -sensitivity &&
					evt.caxis.value < sensitivity	)
				{
					rxVel = 0;
				}
				else
				{
					rxVel = (evt.caxis.value > 0) ? 1 : -1;
				}
			}
			if (evt.caxis.axis == SDL_CONTROLLER_AXIS_RIGHTY)
			{
				if (	evt.caxis.value > -sensitivity &&
					evt.caxis.value < sensitivity	)
				{
					ryVel = 0;
				}
				else
				{
					ryVel = (evt.caxis.value > 0) ? 1 : -1;
				}
			}
		}
		else if (evt.type == SDL_CONTROLLERDEVICEADDED)
		{
			SDL_GameController *toOpen = SDL_GameControllerOpen(evt.cdevice.which);
			printf(
				"Opened SDL_GameController ID #%i, %s\n",
				evt.cdevice.which,
				SDL_GameControllerName(toOpen)
			);
			controllers[SDL_JoystickInstanceID(SDL_GameControllerGetJoystick(toOpen))] = toOpen;
		}
		else if (evt.type == SDL_CONTROLLERDEVICEREMOVED)
		{
			SDL_GameController *toClose = controllers[evt.cdevice.which];
			controllers.erase(evt.cdevice.which);
			printf("Closing %s\n", SDL_GameControllerName(toClose));
			SDL_GameControllerClose(toClose);
		}

		/* Window Events */
		else if (evt.type == SDL_WINDOWEVENT)
		{
			/* Window Resize */
			if (evt.window.event == SDL_WINDOWEVENT_RESIZED)
			{
				resetWindow = true;
			}

			/* Window Focus */
			else if (evt.window.event == SDL_WINDOWEVENT_FOCUS_GAINED)
			{
				isActive = true;
				if (!useFullscreenSpaces)
				{
					SDL_Window *window = SDL_GetWindowFromID(evt.window.windowID);
					wasFullscreen = SDL_GetWindowFlags(window);
					SDL_SetWindowFullscreen(window, 0);
				}
				SDL_DisableScreenSaver();
			}
			else if (evt.window.event == SDL_WINDOWEVENT_FOCUS_LOST)
			{
				isActive = false;
				if (!useFullscreenSpaces)
				{
					SDL_SetWindowFullscreen(
						SDL_GetWindowFromID(evt.window.windowID),
						wasFullscreen
					);
				}
				SDL_EnableScreenSaver();
			}

			/* Mouse Focus */
			else if (evt.window.event == SDL_WINDOWEVENT_ENTER)
			{
				SDL_DisableScreenSaver();
			}
			else if (evt.window.event == SDL_WINDOWEVENT_LEAVE)
			{
				SDL_EnableScreenSaver();
			}
		}

		/* Quit Event */
		else if (evt.type == SDL_QUIT)
		{
			quitProgram = true;
		}
	}
        if (textentrymode)
        {
#ifdef __SWITCH__
            char buf[512] = {0};
            SwkbdConfig conf;
            swkbdCreate(&conf, 0);
            swkbdConfigMakePresetDefault(&conf);
            swkbdConfigSetInitialText(&conf, keybuffer.c_str());
            swkbdShow(&conf, buf, sizeof(buf));
            swkbdClose(&conf);
            keybuffer = buf;
            if (fakekeytimer > 0) {
                keymap[fakekey] = 0;
            }
            fakekey = SDLK_RETURN;
            fakekeytimer = 6;
            textentrymode = 0;
#else
            //if (wantsOSKClose && !SDL_IsScreenKeyboardShown(graphics.screenbuffer->m_window)) {
            if (SDL_HasScreenKeyboardSupport() && !SDL_IsScreenKeyboardShown(graphics.screenbuffer->m_window)) {
                if (fakekeytimer > 0) {
                    keymap[fakekey] = 0;
                }
                fakekey = SDLK_RETURN;
                fakekeytimer = 6;
                textentrymode = 0;
            }
#endif
        }
}

bool KeyPoll::isDown(SDL_Keycode key)
{
	return keymap[key];
}

bool KeyPoll::isUp(SDL_Keycode key)
{
	return !keymap[key];
}

bool KeyPoll::isDown(growing_vector<SDL_GameControllerButton> buttons)
{
	for (size_t i = 0; i < buttons.size(); i += 1)
	{
		if (buttonmap[buttons[i]])
		{
			return true;
		}
	}
	return false;
}

bool KeyPoll::isDown(SDL_GameControllerButton button)
{
	return buttonmap[button];
}

bool KeyPoll::controllerButtonDown()
{
	for (
		SDL_GameControllerButton button = SDL_CONTROLLER_BUTTON_A;
		button < SDL_CONTROLLER_BUTTON_DPAD_UP;
		button = (SDL_GameControllerButton) (button + 1)
	) {
		if (isDown(button))
		{
			return true;
		}
	}
	return false;
}

bool KeyPoll::controllerWantsLeft(bool includeVert)
{
	return (	buttonmap[SDL_CONTROLLER_BUTTON_DPAD_LEFT] ||
			xVel < 0 ||
			(	includeVert &&
				(	buttonmap[SDL_CONTROLLER_BUTTON_DPAD_UP] ||
					yVel < 0	)	)	);
}

bool KeyPoll::controllerWantsRight(bool includeVert)
{
	return (	buttonmap[SDL_CONTROLLER_BUTTON_DPAD_RIGHT] ||
			xVel > 0 ||
			(	includeVert &&
				(	buttonmap[SDL_CONTROLLER_BUTTON_DPAD_DOWN] ||
					yVel > 0	)	)	);
}

bool KeyPoll::controllerWantsRLeft(bool includeVert)
{
	return (rxVel < 0 || (includeVert && ryVel < 0));
}

bool KeyPoll::controllerWantsRRight(bool includeVert)
{
	return (rxVel > 0 || (includeVert && ryVel > 0));
}

bool KeyPoll::controllerWantsRUp()
{
	return ryVel < 0;
}

bool KeyPoll::controllerWantsRDown()
{
	return ryVel > 0;
}
