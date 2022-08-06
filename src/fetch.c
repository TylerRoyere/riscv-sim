#include "fetch.h"

void
rv_print_pc(rv_cpu_state *state)
{
    printf("0x%08lX\n", (uint64_t)state->rvi_pc);
}

rv_instruction
rv_fetch_instruction(rv_cpu_state *state)
{
    rv_instruction inst;
    inst.value = rv_memory_read32(state->rvi_pc);
    return inst;
}

void
rv_pc_increment(rv_cpu_state *state, rv_operation last_op)
{
    (void) last_op;
    if (rv_cpu_flag_is_set(state,
                RV_BRANCH_TAKEN_FLAG | RV_JUMP_TAKEN_FLAG | RV_TRAP_TAKEN_FLAG)) {
        rv_cpu_clear_flag(state, RV_BRANCH_TAKEN_FLAG | RV_JUMP_TAKEN_FLAG | RV_TRAP_TAKEN_FLAG);
    }
    else {
        state->rvi_pc += 4;
    }
}
