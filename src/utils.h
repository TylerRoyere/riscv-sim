#ifndef UTILS_H
#define UTILS_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <limits.h>
#include "decode.h"
#include "register.h"

#ifdef __GNUC__
#define UNUSED __attribute__((unused))
#else
#define UNUSED
#endif

#define TODO(...) \
    printf("%s %s:%d  ", __FILE__, __func__, __LINE__);   \
    printf("TODO: " __VA_ARGS__)

#define BSWAP16(x)  \
    ((uint16_t)                         \
    ( ((((uint16_t)x) & 0x00FF) << 8) | \
      ((((uint16_t)x) & 0xFF00) >> 8)  ))

#define BSWAP32(x)  \
    ((uint32_t)                               \
    ( ((((uint32_t)x) & 0x000000FF) << 24) |  \
      ((((uint32_t)x) & 0x0000FF00) << 8)  |  \
      ((((uint32_t)x) & 0x00FF0000) >> 8)  |  \
      ((((uint32_t)x) & 0xFF000000) >> 24)   ))

#define BSWAP64(x)  \
    ((uint64_t)                                       \
    ( ((((uint64_t)x) & 0x00000000000000FF) << 56) |  \
      ((((uint64_t)x) & 0x000000000000FF00) << 40) |  \
      ((((uint64_t)x) & 0x0000000000FF0000) << 24) |  \
      ((((uint64_t)x) & 0x00000000FF000000) << 8 ) |  \
      ((((uint64_t)x) & 0x000000FF00000000) >> 8 ) |  \
      ((((uint64_t)x) & 0x0000FF0000000000) >> 24) |  \
      ((((uint64_t)x) & 0x00FF000000000000) >> 40) |  \
      ((((uint64_t)x) & 0xFF00000000000000) >> 56)   ))

#define ZERO_EXTEND(val, type) \
    (type)(                                                 \
    sizeof(int8_t)   == sizeof(val) ? ((uint8_t)(val))   :  \
    sizeof(int16_t)  == sizeof(val) ? ((uint16_t)(val))  :  \
    sizeof(int32_t)  == sizeof(val) ? ((uint32_t)(val))  :  \
    sizeof(int64_t)  == sizeof(val) ? ((uint64_t)(val))  :  \
    sizeof(intptr_t) == sizeof(val) ? ((uintptr_t)(val)) :  \
    sizeof(intmax_t) == sizeof(val) ? ((uintmax_t)(val)) :  \
    (printf("Failed to sign extend\n"), exit(1), 0))

#define SIGN_EXTEND(val, type) \
    (type)(                                                 \
    sizeof(uint8_t)   == sizeof(val) ? ((int8_t)(val))   :  \
    sizeof(uint16_t)  == sizeof(val) ? ((int16_t)(val))  :  \
    sizeof(uint32_t)  == sizeof(val) ? ((int32_t)(val))  :  \
    sizeof(uint64_t)  == sizeof(val) ? ((int64_t)(val))  :  \
    sizeof(uintptr_t) == sizeof(val) ? ((intptr_t)(val)) :  \
    sizeof(uintmax_t) == sizeof(val) ? ((intmax_t)(val)) :  \
    (printf("Failed to sign extend\n"), exit(1), 0))

#define RV_INSTRUCTION_FUNCT7(inst) \
    (((inst).value >> 25) & 0x7F)

#define RV_INSTRUCTION_RS2(inst) \
    (((inst).value >> 20) & 0x1F)

#define RV_INSTRUCTION_RS1(inst) \
    (((inst).value >> 15) & 0x1F)

#define RV_INSTRUCTION_RD(inst) \
    (((inst).value >> 7) & 0x1F)

#define RV_INSTRUCTION_FUNCT3(inst) \
    (((inst).value >> 12) & 0x7)

#define RV_INSTRUCTION_OPCODE(inst) \
    (((inst.value) >> 0) & 0x7F)

#define UNPACK_R_TYPE(inst, rs1, rs2, rd) \
    uint8_t rs1 = RV_INSTRUCTION_RS1(inst); \
    uint8_t rs2 = RV_INSTRUCTION_RS2(inst); \
    uint8_t rd = RV_INSTRUCTION_RD(inst)

#define UNPACK_U_TYPE(inst, imm, rd) \
    rvi_register imm = rv_u_type_immediate(inst); \
    uint8_t rd = RV_INSTRUCTION_RD(inst)

#define UNPACK_I_TYPE(inst, imm, rs, rd) \
    rvi_register imm = rv_i_type_immediate(inst); \
    uint8_t rs = RV_INSTRUCTION_RS1(inst); \
    uint8_t rd = RV_INSTRUCTION_RD(inst)

#define UNPACK_J_TYPE(inst, imm, rd) \
    rvi_register imm = rv_j_type_immediate(inst); \
    uint8_t rd = RV_INSTRUCTION_RD(inst)

#define UNPACK_S_TYPE(inst, offset, src, base) \
    rvi_register offset = rv_s_type_immediate(inst); \
    uint8_t src = RV_INSTRUCTION_RS2(inst); \
    uint8_t base = RV_INSTRUCTION_RS1(inst)

#define UNPACK_B_TYPE(inst, offset, rs1, rs2) \
    rvi_register offset = rv_b_type_immediate(inst); \
    uint8_t rs1 = RV_INSTRUCTION_RS1(inst); \
    uint8_t rs2 = RV_INSTRUCTION_RS2(inst)

#define GET_SHAMT(immediate) \
    ((immediate) & 0x1F)

#define GET_SHAMT6(immediate) \
    ((immediate) & 0x3F)

static inline void
print_binary(uintmax_t val, unsigned int length)
{
    uintmax_t mask = (UINTMAX_MAX) & ~(UINTMAX_MAX >> 1);
    uintmax_t temp = val;
    val <<= (CHAR_BIT * sizeof(uintmax_t)) - length;
    for (unsigned int ii = 0; ii < length; ii++) {
        if (val & mask)
            putchar('1');
        else
            putchar('0');
        val <<= 1;
    }
    printf(" (%"PRIuMAX")", temp);
}


#endif
