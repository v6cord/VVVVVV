#ifndef UTILITIES_H
#define UTILITIES_H

#include <algorithm>

size_t strlcpy(char *dst, const char *src, size_t dsize);

// source: https://randomascii.wordpress.com/2013/04/03/stop-using-strncpy-already/
template <size_t charCount>
void strcpy_safe(char (&output)[charCount], const char* pSrc) {
    strlcpy(output, pSrc, charCount);
}

#endif
