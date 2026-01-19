#ifndef __MY_OS__COMMON_H
#define __MY_OS__COMMON_H

#include <common/types.h>

namespace myos
{
    namespace common
    {

        void printf(char *str);

        void printHex(uint8_t key);

        void *memset(void *dst, int value, unsigned int size);
        void *memcpy(void *dst, const void *src, unsigned int size);

    }
}

#endif