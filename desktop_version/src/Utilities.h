#ifndef UTILITIES_H
#define UTILITIES_H

#include <algorithm>
#include <chrono>
#include <string>
#include <sstream>
#include <stdint.h>

uint64_t splitmix64(uint64_t& x);
void seed_xoshiro(uint64_t s1, uint64_t s2, uint64_t s3, uint64_t s4);
inline void seed_xoshiro_64(uint64_t s) {
    auto s1 = splitmix64(s);
    auto s2 = splitmix64(s);
    auto s3 = splitmix64(s);
    auto s4 = splitmix64(s);
    seed_xoshiro(s1, s2, s3, s4);
}
uint64_t xoshiro_next(void);

size_t bsd_strlcpy(char *dst, const char *src, size_t dsize);

int battery_level();
bool on_battery();

// source: https://randomascii.wordpress.com/2013/04/03/stop-using-strncpy-already/
template <size_t charCount>
void strcpy_safe(char (&output)[charCount], const char* pSrc) {
    bsd_strlcpy(output, pSrc, charCount);
}

struct free_delete {
    void operator()(void* x);
};

std::string hhmmss_time();
std::chrono::system_clock::rep unix_time();

template<class T = std::string, class U>
T string_cast(U val) {
    std::stringstream stream;
    T ret;
    stream << val;
    stream >> ret;
    return ret;
}

std::string dtos(double val);

#endif
