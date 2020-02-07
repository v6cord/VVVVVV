#ifndef PRELOADER_H
#define PRELOADER_H

#include "Graphics.h"
#include "Game.h"
#include "UtilityClass.h"
#include <atomic>

void preloaderrender(Graphics& dwgfx, Game& game, UtilityClass& help);
void preloaderloop();
extern std::atomic_int pre_fakepercent;

#endif /* PRELOADER_H */
