#include <ctype.h>
#include <math.h>
#include <stdarg.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "citron.h"

#include "grapheme_break.c"
/**
 * UTF8Size
 *
 * measures the size of character
 */
int ctr_utf8size(char c)
{
    if ((c & CTR_UTF8_BYTE3) == CTR_UTF8_BYTE3)
        return 4;
    if ((c & CTR_UTF8_BYTE2) == CTR_UTF8_BYTE2)
        return 3;
    if ((c & CTR_UTF8_BYTE1) == CTR_UTF8_BYTE1)
        return 2;
    return 1;
}

/**
 * GetBytesForUTF8String
 */
ctr_size getBytesUtf8(char* strval, long startByte, ctr_size lenUChar)
{
    long i = 0;
    long bytes = 0;
    int s = 0;
    int x = 0;
    long index = 0;
    char c;
    while (x < lenUChar) {
        index = startByte + i;
        c = strval[index];
        s = ctr_utf8size(c);
        bytes = bytes + s;
        i = i + s;
        x++;
    }
    return bytes;
}

/**
 * GetUTF8Length
 *
 * measures the length of an utf8 string in utf8 chars
 */
ctr_size ctr_getutf8len(char* strval, ctr_size max)
{
    ctr_size i;
    ctr_size j = 0;
    ctr_size s = 0;
    for (i = 0; i < max; i++) {
        s = ctr_utf8size(strval[i]);
        j += (s - 1);
    }
    return (i - j);
}

#ifndef withICU
// __attribute__((always_inline))
ctr_size ctr_getutf8clustercount(char* strval, ctr_size max)
{
    // return countGraphemes((wchar_t*)strval, max);
    return ctr_getutf8len(strval, max);
}

#else

#    include <unicode/ubrk.h>
#    include <unicode/uchar.h>
#    include <unicode/ustring.h>
static UBreakIterator* BIter__ = NULL;
static UErrorCode err;
// __attribute__((always_inline))
ctr_size ctr_getutf8clustercount(char* strval, ctr_size max)
{
    if (!BIter__)
        BIter__ = ubrk_open(UBRK_CHARACTER, NULL, NULL, 0, &err);
    ubrk_setText(BIter__, (UChar*)strval, max, &err);
    int start = ubrk_first(BIter__);
    int count = 0;
    // printf("%.*s\n", max, strval);
    for (int end = ubrk_next(BIter__); end != UBRK_DONE;
         start = end, end = ubrk_next(BIter__)) {
        long c = 0;
        for (int i = start; i < end; i++) {
            // printf("%d : %d", start, end);
            c += ((unsigned char*)strval)[i];
        }
        int prop = getGraphemeBreakProperty(c);
        // printf("%d\n", prop);
        if (prop == Control)
            count -= 2;
        // if(prop == Extend) count -= 2;
        // if(prop == Glue_After_Zwj) count -= 1;
        count++;
    }
    // printf("%d\n", count);
    return count;
}
#endif

int ctr_utf8_is_one_cluster(char* strval, ctr_size max)
{
    // TODO: Implement cluster count
    return ctr_getutf8clustercount(strval, max) == 1;
}
