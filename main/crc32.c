#ifndef CRC32_H
#define CRC32_H
#include "crc32.h"

#include <stdio.h>
#include <string.h>

uint32_t calc_crc32(uint8_t *data, uint32_t size)
{
    uint32_t crc = CRC32_INITIAL_VALUE;
    uint32_t i;

    for (i = 0; i < size; i++)
    {
        crc = crc32_table[(crc ^ data[i]) & 0xFF] ^ (crc >> 8);
    }

    return ~crc;
}
int checkCrc32(uint32_t dato, char *str)
{
    char ext[8];
    uint8_t i = 0;
    snprintf(ext, 9, "%08lX", dato);
    // 5A120000000000B2 "3613A097" = 8 ASCII
    while (i < 8)
    {
        if (ext[i] != str[i + 16])
            return 0;
        else
            i++;
    }
    return 1;
}

#endif