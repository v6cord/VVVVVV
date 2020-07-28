#include <algorithm>
#include <chrono>
#include <ctime>
#include <stdint.h>
#include <stdio.h>
#include <sstream>
#include <iomanip>
#include "Utilities.h"
#include "Script.h"

extern scriptclass script;

#ifdef __SWITCH__
#include <switch.h>
#else
#include <SDL.h>
#endif

#if defined(__linux__) || defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__HAIKU__) || defined(__SWITCH__)
#include <unistd.h>
#ifndef __SWITCH__
#include <sys/types.h>
#include <signal.h>
#endif
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
#ifdef __SWITCH__
static FILE* logger = nullptr;
#elif !defined(_WIN32)
static pid_t logger = 0;
#endif
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

#if !defined(__SWITCH__) && !defined(_WIN32)
pid_t popen2(const char *command, int *infp, int *outfp) {
    int p_stdin[2], p_stdout[2];
    pid_t pid;

    if (pipe(p_stdin) != 0 || pipe(p_stdout) != 0)
        return -1;

    pid = fork();

    if (pid < 0)
        return pid;
    else if (pid == 0) {
        close(p_stdin[1]);
        dup2(p_stdin[0], 0);
        close(p_stdout[0]);
        dup2(p_stdout[1], 1);

        execl("/bin/sh", "sh", "-c", command, NULL);
        perror("execl");
        exit(1);
    }

    if (infp == NULL)
        close(p_stdin[1]);
    else
        *infp = p_stdin[1];

    if (outfp == NULL)
        close(p_stdout[0]);
    else
        *outfp = p_stdout[0];

    return pid;
}
#endif

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
    int logger_fd = fileno(logger);
#else
    int logger_fd;
    logger = popen2("logger", &logger_fd, NULL);
#endif
    if (logger) {
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
    if (logger) kill(logger, SIGTERM);
#endif
}

void handle_exception(const std::exception& ex) {
    SDL_MessageBoxData msg = {};
    msg.flags = SDL_MESSAGEBOX_ERROR;
    msg.title = "Error";
    msg.message = ex.what();
    msg.numbuttons = 3;
    SDL_MessageBoxButtonData quit_game = {};
    quit_game.buttonid = 0;
    quit_game.text = "Quit game";
    SDL_MessageBoxButtonData stop_script = {};
    stop_script.flags = SDL_MESSAGEBOX_BUTTON_ESCAPEKEY_DEFAULT;
    stop_script.buttonid = 1;
    stop_script.text = "Stop script";
    SDL_MessageBoxButtonData exit_level = {};
    exit_level.flags = SDL_MESSAGEBOX_BUTTON_RETURNKEY_DEFAULT;
    exit_level.buttonid = 2;
    exit_level.text = "Exit level";
    SDL_MessageBoxButtonData buttons[3] = {quit_game, stop_script, exit_level};
    msg.buttons = buttons;
    int pressed = 0;
    SDL_ShowMessageBox(&msg, &pressed);
    switch (pressed) {
        case 0:
        default:
            std::terminate();
            break;
        case 1:
            script.stop();
            break;
        case 2:
            script.quit();
            break;
    }
}

const char* script_exception::what() const noexcept {
    return message.c_str();
}

script_exception::script_exception(const char* msg, bool raw/* = false*/) : script_exception(std::string(msg), raw) {}

script_exception::script_exception(std::string msg, bool raw/* = false*/) {
    if (raw) {
        message = msg;
    } else {
        message = "Script error ";
        if (script.position < static_cast<int>(script.commands.size())) {
            message += "on line ";
            message += script.scriptname;
            message += ":";
            message += std::to_string(script.position);
            message += " (`";
            message += script.commands[script.position];
            message += "`): ";
        } else {
            message += "in ";
            message += script.scriptname;
            message += ": ";
        }
        message += msg;
    }
}

script_exception::script_exception(const std::exception& ex) : script_exception(ex.what()) {}
