#include <string.h>
#include <stdlib.h>

#include "cpu.h"

void
rv_load_simple_program(rv_cpu_state *state, rv_cpu_program program)
{
    printf("Loading program, length = %lu, offset = 0x%016lX\n",
            program.length, program.vaddr_offset);
    if (program.length > sizeof(rv_memory)) {
        printf("Program too large to load\n");
        printf("Program size = %lu, Memory size = %lu\n",
                program.length, sizeof(rv_memory));
        printf("Either reduce program size or increase memory size\n");
    }
    memcpy(rv_memory, program.bytes, program.length);
    memory_vaddr_offset = program.vaddr_offset;
    state->rvi_pc = (rvi_register)program.entry_address;
}

void
rv_program_free(rv_cpu_program program)
{
    free(program.bytes);
}

void
rv_print_regs(rv_cpu_state *state)
{
    for (int ii = 0; ii < RV_NUM_REGS; ii++) {
        printf("x%d = 0x%08x (%d)\n", ii,
                (uint32_t)state->rvi_regs[ii], (int)state->rvi_regs[ii]);
    }
}


const char *const register_names[RV_NUM_REGS] = {
    [0]     = "zero",
    [1]     = "ra",
    [2]     = "sp",
    [3]     = "gp",
    [4]     = "tp",
    [5]     = "t0",
    [6]     = "t1",
    [7]     = "t2",
    [8]     = "fp",
    [9]     = "s1",
    [10]    = "a0",
    [11]    = "a1",
    [12]    = "a2",
    [13]    = "a3",
    [14]    = "a4",
    [15]    = "a5",
    [16]    = "a6",
    [17]    = "a7",
    [18]    = "s2",
    [19]    = "s3",
    [20]    = "s4",
    [21]    = "s5",
    [22]    = "s6",
    [23]    = "s7",
    [24]    = "s8",
    [25]    = "s9",
    [26]    = "s10",
    [27]    = "s11",
    [28]    = "t3",
    [29]    = "t4",
    [30]    = "t5",
    [31]    = "t6",
};
