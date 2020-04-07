#ifndef SCREEN_H
#define SCREEN_H

#if defined(__SWITCH__) || defined(__ANDROID__)
	#include <SDL2/SDL.h>
#else
	#include <SDL.h>
#endif

class Screen
{
public:
	Screen();

	void ResizeScreen(int x, int y);
	void GetWindowSize(int* x, int* y);

	void UpdateScreen(SDL_Surface* buffer, SDL_Rect* rect);
	void FlipScreen();

	const SDL_PixelFormat* GetFormat();

	void toggleFullScreen();
	void toggleStretchMode();
	void toggleLinearFilter();

	bool isWindowed = false;
	bool isFiltered = false;
	bool badSignalEffect = false;
	bool glScreen = false;
        bool headless = false;
	int stretchMode = 0;

	SDL_Window *m_window;
	SDL_Renderer *m_renderer;
	SDL_Texture *m_screenTexture;
	SDL_Surface* m_screen;

	SDL_Rect filterSubrect;
};



#endif /* SCREEN_H */
