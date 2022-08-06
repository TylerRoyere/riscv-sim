#ifndef FETCH_H
#define FETCH_H

#include "instructions.h"
#include "cpu.h"

void rv_print_pc(rv_cpu_state *state);
rv_instruction rv_fetch_instruction(rv_cpu_state *state);
void rv_pc_increment(rv_cpu_state *state, rv_operation last_op);

#endif
