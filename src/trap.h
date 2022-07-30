#ifndef TRAP_H
#define TRAP_H

#include "instructions.h"
#include "cpu.h"

#define RV_TRAP_U_ECALL 8
#define RV_TRAP_S_ECALL 9
#define RV_TRAP_M_ECALL 11

void rv_do_trap(int is_interrupt, rvi_register code, rv_cpu_state *state);

void execute_MRET(rv_instruction inst, rv_cpu_state *state);
void execute_SRET(rv_instruction inst, rv_cpu_state *state);

#endif
