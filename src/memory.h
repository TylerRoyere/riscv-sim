#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>
#include <assert.h>
#include <stdio.h>

#include "register.h"

#ifndef MEMORY_SIZE
#define MEMORY_SIZE (10 * 1024)
#endif

extern uint8_t rv_memory[MEMORY_SIZE];

static inline uint8_t
rv_memory_read8(rvi_register index)
{
    assert(index < MEMORY_SIZE);
    return rv_memory[index];
}

static inline uint16_t
rv_memory_read16(rvi_register index)
{
    return (rv_memory_read8(index + 1) << 8) |
            rv_memory_read8(index);
}

static inline uint32_t
rv_memory_read32(rvi_register index)
{
    return (rv_memory_read16(index + 2) << 16) |
            rv_memory_read16(index);
}

static inline void
rv_memory_write8(rvi_register index, uint8_t value)
{
    //printf("index = %ld\n", index);
    assert(index < MEMORY_SIZE);
    rv_memory[index] = value;
}

static inline void
rv_memory_write16(rvi_register index, uint16_t value)
{
    rv_memory_write8(index + 1, (uint8_t)(value >> 8));
    rv_memory_write8(index, (uint8_t)(value & 0xFF));
}

static inline void
rv_memory_write32(rvi_register index, uint32_t value)
{
    rv_memory_write16(index + 2, (uint16_t)(value >> 16));
    rv_memory_write16(index, (uint16_t)(value & 0xFFFF));
}

static inline void
rv_print_mem(void)
{
    for (int ii = 0; ii < MEMORY_SIZE; ii += 4) {
        printf("0x%08X: ", ii);
        for (int jj = 0; jj < 4; jj++) {
            printf("%02X ", rv_memory[ii + jj]);
        }
        printf("\n");
    }
}

#endif
