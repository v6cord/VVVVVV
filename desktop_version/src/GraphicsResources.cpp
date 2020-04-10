#include "GraphicsResources.h"
#include "FileSystemUtils.h"
#include <stdio.h>
#include <stdlib.h>
#include <png.h>

SDL_Surface* LoadImage(const char *filename, bool noBlend /*= true*/, bool noAlpha /*= false*/, bool optional /*= false*/)
{
	//Temporary storage for the image that's loaded
	SDL_Surface* loadedImage = NULL;
	//The optimized image that will be used
	SDL_Surface* optimizedImage = NULL;

	unsigned char *data;
	unsigned int width, height;
        int err;

	unsigned char *fileIn = NULL;
	size_t length = 0;
	bool ret = FILESYSTEM_loadFileToMemory(filename, &fileIn, &length);
    if (!ret) {
        return nullptr;
    }

        png_image image;
        image.version = PNG_IMAGE_VERSION;
        image.opaque = nullptr;

        err = png_image_begin_read_from_memory(&image, fileIn, length);
        if (!err) {
            FILESYSTEM_freeMemory(&fileIn);
            return nullptr;
        }
        width = image.width;
        height = image.height;

	if (noAlpha)
	{
		image.format = PNG_FORMAT_RGB;
	}
	else
	{
		image.format = PNG_FORMAT_RGBA;
	}

        data = (unsigned char*) malloc(PNG_IMAGE_SIZE(image));
        err = png_image_finish_read(&image, nullptr, data, 0, nullptr);
	FILESYSTEM_freeMemory(&fileIn);
        if (!err) {
            free(data);
            return nullptr;
        }

	loadedImage = SDL_CreateRGBSurfaceFrom(
		data,
		width,
		height,
		noAlpha ? 24 : 32,
		width * (noAlpha ? 3 : 4),
		0x000000FF,
		0x0000FF00,
		0x00FF0000,
		noAlpha ? 0x00000000 : 0xFF000000
	);

	if (loadedImage != NULL && data != NULL)
	{
		optimizedImage = SDL_ConvertSurfaceFormat(
			loadedImage,
			SDL_PIXELFORMAT_ABGR8888, // FIXME: Format? -flibit
			0
		);
		SDL_FreeSurface( loadedImage );
		free(data);
		if (noBlend)
		{
			SDL_SetSurfaceBlendMode(optimizedImage, SDL_BLENDMODE_BLEND);
		}
		return optimizedImage;
	}
	else if (!optional)
	{
		fprintf(stderr,"Image not found: %s\n", filename);
		SDL_assert(0 && "Image not found! See stderr.");
	}
        return NULL;
}

void GraphicsResources::init(void)
{
	im_tiles =		LoadImage("graphics/tiles.png");
	im_tiles2 =		LoadImage("graphics/tiles2.png");
	im_tiles3 =		LoadImage("graphics/tiles3.png");
	im_entcolours =		LoadImage("graphics/entcolours.png");
	im_sprites =		LoadImage("graphics/sprites.png");
	im_flipsprites =	LoadImage("graphics/flipsprites.png");
	im_bfont =		LoadImage("graphics/font.png");
	im_unifont =		LoadImage("graphics/unifont.png", true, false, true);
	im_wideunifont =	LoadImage("graphics/wideunifont.png", true, false, true);
	im_bfontmask =		LoadImage("graphics/fontmask.png");
	im_teleporter =		LoadImage("graphics/teleporter.png");

	im_image0 =		LoadImage("graphics/levelcomplete.png", false);
	im_image1 =		LoadImage("graphics/minimap.png", true, true);
	im_image2 =		LoadImage("graphics/covered.png", true, true);
	im_image3 =		LoadImage("graphics/elephant.png");
	im_image4 =		LoadImage("graphics/gamecomplete.png", false);
	im_image5 =		LoadImage("graphics/fliplevelcomplete.png", false);
	im_image6 =		LoadImage("graphics/flipgamecomplete.png", false);
	im_image7 =		LoadImage("graphics/site.png", false);
	im_image8 =		LoadImage("graphics/site2.png");
	im_image9 =		LoadImage("graphics/site3.png");
	im_image10 =		LoadImage("graphics/ending.png");
	im_image11 =		LoadImage("graphics/site4.png");
	im_image12 =		LoadImage("graphics/minimap.png");

	auto templist = FILESYSTEM_getGraphicsDirFileNames();
	for (auto name : templist) {
		if (name.find("graphics/tiles") != 0 || name.length() < 4 || name.substr(name.length()-4, 4) != ".png")
			continue;

		std::string thenumber = name.substr(14, name.length()-18);

		if (!is_number(thenumber))
			continue;

		int sheetnum = atoi(thenumber.c_str());
		if (sheetnum <= 3)
			// Tilesheet 0 just means "just use the default tiles/tiles2/tiles3"
			// Negative numbers are also equally invalid
			continue;

		im_customtiles[sheetnum] = LoadImage(name.c_str());
	}
}


GraphicsResources::~GraphicsResources(void)
{
	SDL_FreeSurface(im_tiles);
	SDL_FreeSurface(im_tiles2);
	SDL_FreeSurface(im_tiles3);
	SDL_FreeSurface(im_entcolours);
	SDL_FreeSurface(im_sprites);
	SDL_FreeSurface(im_flipsprites);
	SDL_FreeSurface(im_bfont);
	SDL_FreeSurface(im_unifont);
	SDL_FreeSurface(im_wideunifont);
	SDL_FreeSurface(im_bfontmask);
	SDL_FreeSurface(im_teleporter);

	SDL_FreeSurface(im_image0);
	SDL_FreeSurface(im_image1);
	SDL_FreeSurface(im_image2);
	SDL_FreeSurface(im_image3);
	SDL_FreeSurface(im_image4);
	SDL_FreeSurface(im_image5);
	SDL_FreeSurface(im_image6);
	SDL_FreeSurface(im_image7);
	SDL_FreeSurface(im_image8);
	SDL_FreeSurface(im_image9);
	SDL_FreeSurface(im_image10);
	SDL_FreeSurface(im_image11);
	SDL_FreeSurface(im_image12);

	for (auto tilesheet : im_customtiles)
		SDL_FreeSurface(tilesheet.second);
}
