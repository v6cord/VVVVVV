#ifndef SCREEN_H
#define SCREEN_H

#include "SDL.h"

typedef enum
{
	NONE,
	LEVEL1,
	LEVEL2
} StretchMode;

class Screen
{
protected:
	SDL_Window* m_pWindow;
	SDL_Renderer* m_pRenderer;
	SDL_Texture* m_pScreenTexture;
	SDL_Surface* m_pScreen;

	bool m_bWindowed;
	bool m_bFiltered;
	bool m_bBadSignalEffect;
	bool m_bGlScreen;
	StretchMode m_eStretchMode;

	SDL_Rect m_sFilterSubrect;

	Screen();
	~Screen();
public:
	static Screen* create();
	static void destroy();
	static Screen* getInstance();

	bool isWindowed() { return m_bWindowed; }

	bool isFiltered() { return m_bFiltered; }

	bool badSignal() { return m_bBadSignalEffect; }
	void setBadSignal(bool b) { m_bBadSignalEffect = b; }

	StretchMode getStretchMode() { return m_eStretchMode; }

	void ResizeScreen(int x, int y);
	void GetWindowSize(int& x, int& y);

	void UpdateScreen(SDL_Surface* buffer, SDL_Rect* rect);
	void ClearScreen(int colour);
	void FlipScreen();

	const SDL_PixelFormat* GetFormat();

	void toggleFullScreen();
	void toggleStretchMode();
	void toggleLinearFilter();
};

#endif /* SCREEN_H */
