#ifndef RV32I_H
#define RV32I_H

#include "cpu.h"
#include "instructions.h"

#define RV32I_FN_DECL(name) \
    void execute_ ## name (rv_instruction inst, rv_cpu_state *state)

void execute_LUI(rv_instruction inst, rv_cpu_state *state);
void execute_AUIPC(rv_instruction inst, rv_cpu_state *state);
void execute_JAL(rv_instruction inst, rv_cpu_state *state);
void execute_JALR(rv_instruction inst, rv_cpu_state *state);

void execute_BEQ(rv_instruction inst, rv_cpu_state *state);
void execute_BNE(rv_instruction inst, rv_cpu_state *state);
void execute_BLT(rv_instruction inst, rv_cpu_state *state);
void execute_BGE(rv_instruction inst, rv_cpu_state *state);
void execute_BLTU(rv_instruction inst, rv_cpu_state *state);
void execute_BGEU(rv_instruction inst, rv_cpu_state *state);

void execute_LB(rv_instruction inst, rv_cpu_state *state);
void execute_LH(rv_instruction inst, rv_cpu_state *state);
void execute_LW(rv_instruction inst, rv_cpu_state *state);
void execute_LBU(rv_instruction inst, rv_cpu_state *state);
void execute_LHU(rv_instruction inst, rv_cpu_state *state);
void execute_SB(rv_instruction inst, rv_cpu_state *state);
void execute_SH(rv_instruction inst, rv_cpu_state *state);
void execute_SW(rv_instruction inst, rv_cpu_state *state);

void execute_ADDI(rv_instruction inst, rv_cpu_state *state);
void execute_SLTI(rv_instruction inst, rv_cpu_state *state);
void execute_SLTIU(rv_instruction inst, rv_cpu_state *state);
void execute_XORI(rv_instruction inst, rv_cpu_state *state);
void execute_ORI(rv_instruction inst, rv_cpu_state *state);
void execute_ANDI(rv_instruction inst, rv_cpu_state *state);
#if 0
void execute_SLLI(rv_instruction inst, rv_cpu_state *state);
void execute_SRLI(rv_instruction inst, rv_cpu_state *state);
void execute_SRAI(rv_instruction inst, rv_cpu_state *state);
#endif

void execute_ADD(rv_instruction inst, rv_cpu_state *state);
void execute_SUB(rv_instruction inst, rv_cpu_state *state);
void execute_SLL(rv_instruction inst, rv_cpu_state *state);
void execute_SLT(rv_instruction inst, rv_cpu_state *state);
void execute_SLTU(rv_instruction inst, rv_cpu_state *state);
void execute_XOR(rv_instruction inst, rv_cpu_state *state);
void execute_SRL(rv_instruction inst, rv_cpu_state *state);
void execute_SRA(rv_instruction inst, rv_cpu_state *state);
void execute_OR(rv_instruction inst, rv_cpu_state *state);
void execute_AND(rv_instruction inst, rv_cpu_state *state);

void execute_FENCE(rv_instruction inst, rv_cpu_state *state);
void execute_FENCEI(rv_instruction inst, rv_cpu_state *state);

void execute_ECALL(rv_instruction inst, rv_cpu_state *state);
void execute_EBREAK(rv_instruction inst, rv_cpu_state *state);

void execute_CSRRW(rv_instruction inst, rv_cpu_state *state);
void execute_CSRRS(rv_instruction inst, rv_cpu_state *state);
void execute_CSRRC(rv_instruction inst, rv_cpu_state *state);
void execute_CSRRWI(rv_instruction inst, rv_cpu_state *state);
void execute_CSRRSI(rv_instruction inst, rv_cpu_state *state);
void execute_CSRRCI(rv_instruction inst, rv_cpu_state *state);


#endif
