#include <algorithm>
#include <chrono>
#include <ctime>
#include <stdint.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include "Utilities.h"

#ifdef __SWITCH__
#include <switch.h>
#elif __ANDROID__
#include <SDL2/SDL.h>
#else
#include <SDL.h>
#endif

uint64_t splitmix64(uint64_t& x) {
    uint64_t z = (x += 0x9e3779b97f4a7c15);
    z = (z ^ (z >> 30)) * 0xbf58476d1ce4e5b9;
    z = (z ^ (z >> 27)) * 0x94d049bb133111eb;
    return z ^ (z >> 31);
}

static inline uint64_t rotl(const uint64_t x, int k) {
    return (x << k) | (x >> (64 - k));
}

static uint64_t s[4];

void seed_xoshiro(uint64_t s1, uint64_t s2, uint64_t s3, uint64_t s4) {
    s[0] = s1;
    s[1] = s2;
    s[2] = s3;
    s[3] = s4;
}

uint64_t xoshiro_next(void) {
    const uint64_t result = s[0] + s[3];

    const uint64_t t = s[1] << 17;

    s[2] ^= s[0];
    s[3] ^= s[1];
    s[1] ^= s[2];
    s[0] ^= s[3];

    s[2] ^= t;

    s[3] = rotl(s[3], 45);

    return result;
}

// source: https://gitlab.freedesktop.org/libbsd/libbsd/blob/7aede6a999ef7b6bd2b82aed55896611331a8eea/src/strlcpy.c
size_t bsd_strlcpy(char *dst, const char *src, size_t dsize) {
    const char *osrc = src;
    size_t nleft = dsize;

    /* Copy as many bytes as will fit. */
    if (nleft != 0) {
        while (--nleft != 0) {
            if ((*dst++ = *src++) == '\0') {
                break;
            }
        }
    }

    /* Not enough room in dst, add NUL and traverse rest of src. */
    if (nleft == 0) {
        if (dsize != 0) {
            *dst = '\0';		/* NUL-terminate dst */
        }
        while (*src++) {}
    }

    return src - osrc - 1;	/* count does not include NUL */
}

void free_delete::operator()(void* x) { free(x); }

#ifdef __SWITCH__
int battery_level() {
    psmInitialize();
    uint32_t level = 100;
    psmGetBatteryChargePercentage(&level);
    return (int) level;
}

bool on_battery() {
    psmInitialize();
    ChargerType type;
    psmGetChargerType(&type);
    return type == ChargerType_None;
}
#else
int battery_level() {
    int percent = 100;
    SDL_GetPowerInfo(nullptr, &percent);
    if (percent == -1) percent = 100;
    return percent;
}

bool on_battery() {
    return SDL_GetPowerInfo(nullptr, nullptr) == SDL_POWERSTATE_ON_BATTERY;
}
#endif

std::string hhmmss_time() {
    std::time_t timestamp = std::time(nullptr);
    std::tm* time = std::localtime(&timestamp);
    std::stringstream stream;
    stream << std::setfill('0') << std::setw(2) << time->tm_hour << ":" << time->tm_min << ":" << time->tm_sec;
    return stream.str();
}

// this is specified as being a unix timestamp, unlike std::time
std::chrono::system_clock::rep unix_time() {
    return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch()).count();
}

std::string dtos(double val) {
    if (val == (int) val) {
        return string_cast((int) val);
    } else {
        int size = snprintf(nullptr, 0, "%f", val);
        std::string str(size, '\0');
        char* data = str.data();
        int written = snprintf(data, size + 1, "%f", val);
        str.resize(written);
        str.erase(str.find_last_not_of(".0") + 1);
        return str;
    }
}
