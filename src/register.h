#ifndef REGISTER_H
#define REGISTER_H

#include <stdint.h>

#if !defined(XLEN) || defined(XLEN) && XLEN == 64
typedef int64_t rvi_register;
typedef uint64_t rvi_register_unsigned;
#elif defined(XLEN) && XLEN == 32
typedef int32_t rvi_register;
typedef uint32_t rvi_register_unsigned;
#else
#error "Invalid register length (XLEN)"
#endif

#endif
