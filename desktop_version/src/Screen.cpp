#include "lodepng.h"
#include "FileSystemUtils.h"
#include "GraphicsUtil.h"
#include "Screen.h"

//Global instance
static Screen* s_pScreenInstance = nullptr;

//Screen

//TODO: Error checking
Screen::Screen()
	:m_pWindow(nullptr),
	m_pRenderer(nullptr),
	m_pScreenTexture(nullptr),
	m_pScreen(nullptr),
	m_bWindowed(true),
	m_bFiltered(false),
	m_bBadSignalEffect(false),
	m_bGlScreen(false),
	m_eStretchMode(StretchMode(0)),
	m_sFilterSubrect({1, 1, 318, 238})
{
	std::vector<uint8_t> buffer;
	auto fs = FSUtils::getInstance();

	//Init SDL
	SDL_Init(
		SDL_INIT_VIDEO |
		SDL_INIT_AUDIO |
		SDL_INIT_JOYSTICK |
		SDL_INIT_GAMECONTROLLER);

	SDL_ShowCursor(SDL_DISABLE);

    SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "nearest");

	//Uncomment this next line when you need to debug -flibit
	//SDL_SetHintWithPriority(SDL_HINT_RENDER_DRIVER, "software", SDL_HINT_OVERRIDE);

	//Create window
	SDL_CreateWindowAndRenderer(
		640,
		480,
		SDL_WINDOW_HIDDEN | SDL_WINDOW_RESIZABLE,
		&m_pWindow,
		&m_pRenderer);

	//Set title
	SDL_SetWindowTitle(m_pWindow, "VVVVVV");

	//Set icon
	if (fs->loadFile("VVVVVV.png", buffer))
	{
		std::vector<uint8_t> data;
		uint32_t width, height;

		lodepng::decode(data, width, height, buffer);

		auto icon = SDL_CreateRGBSurfaceFrom(
			reinterpret_cast<void*>(data.data()),
			width, height, 24, width * 3,
			0x000000FF,
			0x0000FF00,
			0x00FF0000,
			0x00000000);

		if (icon)
		{
			SDL_SetWindowIcon(m_pWindow, icon);
			SDL_FreeSurface(icon);
		}
	}

	// FIXME: This surface should be the actual backbuffer! -flibit
	m_pScreen = SDL_CreateRGBSurface(
		0, 320, 240, 32,
		0x00FF0000,
		0x0000FF00,
		0x000000FF,
		0xFF000000);

	m_pScreenTexture = SDL_CreateTexture(
		m_pRenderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		320,
		240);
}

Screen::~Screen()
{
	if (m_pScreen)
		SDL_FreeSurface(m_pScreen);

	if (m_pScreenTexture)
		SDL_DestroyTexture(m_pScreenTexture);

	if (m_pRenderer)
		SDL_DestroyRenderer(m_pRenderer);

	if (m_pWindow)
		SDL_DestroyWindow(m_pWindow);

	SDL_Quit();
}

Screen* Screen::create()
{
	auto p = new Screen();

	if (p)
	{
		s_pScreenInstance = p;
		return p;
	}

	return nullptr;
}

void Screen::destroy()
{
	if (s_pScreenInstance)
	{
		delete s_pScreenInstance;
		s_pScreenInstance = nullptr;
	}
}

Screen* Screen::getInstance()
{
	return s_pScreenInstance;
}

void Screen::ResizeScreen(int x, int y)
{
	static int resX = 320;
	static int resY = 240;

	if (x != -1 && y != -1)
	{
		// This is a user resize!
		resX = x;
		resY = y;
	}

	if (!m_bWindowed)
		SDL_SetWindowFullscreen(
			m_pWindow, SDL_WINDOW_FULLSCREEN_DESKTOP);
	else
	{
		SDL_SetWindowFullscreen(m_pWindow, 0);

		if (x != -1 && y != -1)
		{
			SDL_SetWindowSize(m_pWindow, resX, resY);
			SDL_SetWindowPosition(
				m_pWindow,
				SDL_WINDOWPOS_CENTERED,
				SDL_WINDOWPOS_CENTERED);
		}
	}

	if (m_eStretchMode == LEVEL1)
	{
		int winX, winY;
		SDL_GetWindowSize(m_pWindow, &winX, &winY);
		SDL_RenderSetLogicalSize(m_pRenderer, winX, winY);
		SDL_RenderSetIntegerScale(m_pRenderer, SDL_FALSE);
	}
	else
	{
		SDL_RenderSetLogicalSize(m_pRenderer, 320, 240);
		SDL_RenderSetIntegerScale(m_pRenderer,
			SDL_bool(m_eStretchMode == LEVEL2));
	}

	SDL_ShowWindow(m_pWindow);
}

void Screen::GetWindowSize(int& x, int& y)
{
	SDL_GetWindowSize(m_pWindow, &x, &y);
}

void Screen::UpdateScreen(SDL_Surface* buffer, SDL_Rect* rect )
{
	if ((buffer == NULL) && (m_pScreen == NULL))
		return;

    if(m_bBadSignalEffect)
        buffer = ApplyFilter(buffer);


    FillRect(m_pScreen, 0x000);
    BlitSurfaceStandard(buffer, nullptr, m_pScreen, rect);

    if (m_bBadSignalEffect)
        SDL_FreeSurface(buffer);
}

const SDL_PixelFormat* Screen::GetFormat()
{
    return m_pScreen->format;
}

void Screen::FlipScreen()
{
	SDL_UpdateTexture(
		m_pScreenTexture,
		nullptr,
		m_pScreen->pixels,
		m_pScreen->pitch
	);

	SDL_RenderCopy(
		m_pRenderer,
		m_pScreenTexture,
		m_bFiltered ? &m_sFilterSubrect : nullptr,
		nullptr);

	SDL_RenderPresent(m_pRenderer);
	SDL_RenderClear(m_pRenderer);
	SDL_FillRect(m_pScreen, NULL, 0x00000000);
}

void Screen::toggleFullScreen()
{
	m_bWindowed = !m_bWindowed;
	this->ResizeScreen(-1, -1);
}

void Screen::toggleStretchMode()
{
	m_eStretchMode = StretchMode((m_eStretchMode + 1) % 3);
	this->ResizeScreen(-1, -1);
}

void Screen::toggleLinearFilter()
{
	m_bFiltered = !m_bFiltered;

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY,
		m_bFiltered ? "linear" : "nearest");

	SDL_DestroyTexture(m_pScreenTexture);

	m_pScreenTexture = SDL_CreateTexture(
		m_pRenderer,
		SDL_PIXELFORMAT_ARGB8888,
		SDL_TEXTUREACCESS_STREAMING,
		320,
		240);
}

void Screen::ClearScreen(int colour)
{
    //FillRect(m_screen, colour) ;
}
