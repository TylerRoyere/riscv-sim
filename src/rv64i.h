#ifndef RV64I_H
#define RV64I_H

#include "instructions.h"
#include "cpu.h"

void execute_LWU(rv_instruction inst, rv_cpu_state *state);
void execute_LD(rv_instruction inst, rv_cpu_state *state);
void execute_SD(rv_instruction inst, rv_cpu_state *state);
void execute_SLLI(rv_instruction inst, rv_cpu_state *state);
void execute_SRLI(rv_instruction inst, rv_cpu_state *state);
void execute_SRAI(rv_instruction inst, rv_cpu_state *state);
void execute_ADDIW(rv_instruction inst, rv_cpu_state *state);
void execute_SLLIW(rv_instruction inst, rv_cpu_state *state);
void execute_SRLIW(rv_instruction inst, rv_cpu_state *state);
void execute_SRAIW(rv_instruction inst, rv_cpu_state *state);
void execute_ADDW(rv_instruction inst, rv_cpu_state *state);
void execute_SUBW(rv_instruction inst, rv_cpu_state *state);
void execute_SLLW(rv_instruction inst, rv_cpu_state *state);
void execute_SRLW(rv_instruction inst, rv_cpu_state *state);
void execute_SRAW(rv_instruction inst, rv_cpu_state *state);


#endif
