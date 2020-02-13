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

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__) || defined(__SWITCH__)
#include <unistd.h>
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
    stream
        << std::setfill('0') << std::setw(2)
        << time->tm_hour << ":"
        << std::setfill('0') << std::setw(2)
        << time->tm_min << ":"
        << std::setfill('0') << std::setw(2)
        << time->tm_sec;
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

#if !defined(__ANDROID__) && !defined(_WIN32)
static FILE* logger = nullptr;
#endif

#ifdef __ANDROID__
#include <pthread.h>
#include <android/log.h>

static int pfd[2];
static pthread_t thr;
static const char *tag = "VVVVVV-CE";

static void* log_thread(void* ctx) {
    ssize_t rdsz;
    char buf[128];
    while((rdsz = read(pfd[0], buf, sizeof buf - 1)) > 0) {
        if(buf[rdsz - 1] == '\n') --rdsz;
        buf[rdsz] = 0;
        __android_log_write(ANDROID_LOG_DEBUG, tag, buf);
    }
    return 0;
}
#endif

bool log_default() {
#if defined(__SWITCH__) || defined(__APPLE__) || defined(__ANDROID__)
    return true;
#else
    return false;
#endif
}

void log_init() {
#if defined(__ANDROID__)
    setvbuf(stdout, 0, _IOLBF, 0);
    setvbuf(stderr, 0, _IONBF, 0);

    pipe(pfd);
    dup2(pfd[1], 1);
    dup2(pfd[1], 2);

    pthread_create(&thr, 0, log_thread, 0);
    pthread_detach(thr);
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__) || defined(__SWITCH__)
#ifdef __SWITCH__
    logger = fopen("sdmc:/switch/VVVVVV/vvvvvv-ce.log", "a");
#else
    logger = popen("logger", "w");
#endif
    if (logger) {
        auto logger_fd = fileno(logger);
        auto stdout_fd = fileno(stdout);
        auto stderr_fd = fileno(stderr);
        dup2(logger_fd, stdout_fd);
        dup2(logger_fd, stderr_fd);
        setbuf(stdout, nullptr);
    } else {
        puts("Couldn't create logger!");
    }
#endif
}

void log_close() {
#if defined(__SWITCH__)
    if (logger) fclose(logger);
#elif defined(__ANDROID__)
    // doesn't need closing
#elif defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__) || defined(__SWITCH__)
    if (logger) pclose(logger);
#endif
}
