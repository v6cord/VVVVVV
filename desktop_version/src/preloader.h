#ifndef PRELOADER_H
#define PRELOADER_H

#include "Graphics.h"
#include "Game.h"
#include "UtilityClass.h"
#include <atomic>

void preloaderrender();
void preloaderloop();
extern std::atomic_int pre_fakepercent;
extern std::atomic_bool pre_quickend;

void preloaderlogic();

#endif /* PRELOADER_H */
