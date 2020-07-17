#ifndef SCREEN_H
#define SCREEN_H

#include <SDL.h>

class Screen
{
public:
	void init();

	void ResizeScreen(int x, int y);
	void ResizeToNearestMultiple();
	void GetWindowSize(int* x, int* y);

	void UpdateScreen(SDL_Surface* buffer, SDL_Rect* rect);
	void FlipScreen();

	const SDL_PixelFormat* GetFormat();

	void toggleFullScreen();
	void toggleStretchMode();
	void toggleLinearFilter();
	void resetRendererWorkaround();

	bool isWindowed = false;
	bool isFiltered = false;
	bool badSignalEffect = false;
	bool initialized = false;
	int stretchMode = 0;
	bool vsync = false;

	SDL_Window *m_window;
	SDL_Renderer *m_renderer;
	SDL_Texture *m_screenTexture;
	SDL_Surface* m_screen;

	SDL_Rect filterSubrect;
};

#endif /* SCREEN_H */
