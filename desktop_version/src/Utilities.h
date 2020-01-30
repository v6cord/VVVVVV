#ifndef UTILITIES_H
#define UTILITIES_H

#include <algorithm>
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

size_t strlcpy(char *dst, const char *src, size_t dsize);

// source: https://randomascii.wordpress.com/2013/04/03/stop-using-strncpy-already/
template <size_t charCount>
void strcpy_safe(char (&output)[charCount], const char* pSrc) {
    strlcpy(output, pSrc, charCount);
}

struct free_delete {
    void operator()(void* x);
};

#endif
