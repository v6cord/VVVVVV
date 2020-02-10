#include <algorithm>
#include <fstream>
#include <string>
#include <optional>
#include <stdint.h>
#include <string.h>
#include "Utilities.h"
#include <stdio.h>

#ifdef __linux__
#include <dirent.h>
#include <limits.h>
#include "FileSystemUtils.h"
#elif __SWITCH__
#include <switch.h>
#elif _WIN32
#include <windows.h>
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

#ifdef __linux__
static std::optional<std::string> battery_path() {
    auto dir = opendir("/sys/class/power_supply");
    if (!dir) {
        perror("opendir");
        return {};
    }
    struct dirent* entry;
    while ((entry = readdir(dir)) != nullptr) {
        if (strncmp(entry->d_name, "BAT", 3) == 0) {
            std::string path = "/sys/class/power_supply/";
            path += entry->d_name;
            return path;
        }
    }
    return {};
}

int battery_level() {
    if (auto bat = battery_path()) {
        *bat += "/capacity";
        std::ifstream stream(*bat);
        int level = 100;
        stream >> level;
        return level;
    } else {
        return 100;
    }
}

bool on_battery() {
    if (auto bat = battery_path()) {
        *bat += "/status";
        std::ifstream stream(*bat);
        std::string status;
        stream >> status;
        return status != "Charging";
    } else {
        return false;
    }
}
#elif defined(__SWITCH__)
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
#elif defined(_WIN32)
int battery_level() {
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status)) return 100;
    return (int) status.BatteryLifePercent;
}

bool on_battery() {
    SYSTEM_POWER_STATUS status;
    if (!GetSystemPowerStatus(&status)) return false;
    return status.ACLineStatus == 0;
}
#elif !defined(__APPLE__)
int battery_level() {
    return 100;
}

bool on_battery() {
    return false;
}
#endif
