#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>

#include "memory.h"
#include "register.h"

#define RV_NUM_REGS 32

typedef enum rv_cpu_flag {
    RV_BRANCH_TAKEN_FLAG = 0x1,
    RV_JUMP_TAKEN_FLAG = 0x2,
} rv_cpu_flag;

typedef struct rv_cpu_state {
    rvi_register rvi_regs[RV_NUM_REGS];
    rvi_register rvi_pc;
    rv_cpu_flag flags;
} rv_cpu_state;

extern const char *const register_names[RV_NUM_REGS];

static inline void
rv_cpu_set_flag(rv_cpu_state *state, rv_cpu_flag flag)
{
    state->flags |= flag;
}

static inline void
rv_cpu_clear_flag(rv_cpu_state *state, rv_cpu_flag flag)
{
    state->flags &= (~flag);
}

static inline int
rv_cpu_flag_is_set(rv_cpu_state *state, rv_cpu_flag flag)
{
    return !!(state->flags & flag);
}


static inline void
rv_load_simple_program(rv_cpu_state *state, const uint8_t *bytes, uint64_t length)
{
    for (uint64_t ii = 0; ii < length; ii++) {
        rv_memory[ii] = bytes[ii];
    }

    state->rvi_pc = 0;
}

static inline rvi_register
rvi_reg_read(rv_cpu_state *state, uint8_t reg)
{
    if (reg && reg < RV_NUM_REGS) {
        return state->rvi_regs[reg];
    }
    return 0;
}

static inline rvi_register_unsigned
rvi_reg_read_unsigned(rv_cpu_state *state, uint8_t reg)
{
    return (rvi_register_unsigned)rvi_reg_read(state, reg);
}

static inline void
rvi_reg_write(rv_cpu_state *state, uint8_t reg, rvi_register value)
{
    if (reg && reg < RV_NUM_REGS) {
        state->rvi_regs[reg] = value;
    }
}

static inline void
rv_print_regs(rv_cpu_state *state)
{
    for (int ii = 0; ii < RV_NUM_REGS; ii++) {
        printf("x%d = 0x%08x (%d)\n", ii,
                (uint32_t)state->rvi_regs[ii], (int)state->rvi_regs[ii]);
    }
}


#endif
