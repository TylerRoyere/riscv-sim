#ifndef CPU_H
#define CPU_H

#include <stdint.h>
#include <stdio.h>

#include "memory.h"
#include "register.h"
#include "csr.h"

#define RV_NUM_REGS 32

typedef enum rv_cpu_flag {
    RV_BRANCH_TAKEN_FLAG = 0x1,
    RV_JUMP_TAKEN_FLAG = 0x2,
    RV_TRAP_TAKEN_FLAG = 0x4,
} rv_cpu_flag;

typedef enum rv_cpu_privilege {
    RV_USER         = 0x0,
    RV_SUPERVISOR   = 0x1,
    /* 0x2 is Reserved */
    RV_MACHINE      = 0x3,
} rv_cpu_privilege;

struct rv_cpu_state;
typedef void (*watch_action)(struct rv_cpu_state *state);

typedef struct rv_cpu_state {
    rvi_register        rvi_regs[RV_NUM_REGS];
    rvi_register        rvi_pc;
    rv_csr_state        csrs;
    rv_cpu_flag         flags;
    rv_cpu_privilege    privilege;
    rvi_register        watchpoint;
    watch_action        action;
} rv_cpu_state;

typedef struct rv_cpu_program {
    /* Special memory address for testing */
    uint64_t tohost;
    uint64_t entry_address;
    uint64_t vaddr_offset;
    uint64_t length;
    uint8_t *bytes;
} rv_cpu_program;

extern const char *const register_names[RV_NUM_REGS];

int  rv_load_simple_program(rv_cpu_state *state, rv_cpu_program program);
void rv_print_regs(rv_cpu_state *state);
void rv_program_free(rv_cpu_program program);

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
rv_cpu_check_watchpoint(rv_cpu_state *state, rvi_register address, int length)
{
    if (address <= state->watchpoint &&
            (address + length) > state->watchpoint) {
        state->action(state);
    }
}

#endif
