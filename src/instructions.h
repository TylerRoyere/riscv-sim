#ifndef INSTRUCTIONS_H
#define INSTRUCTIONS_H

#include <stdint.h>

typedef enum rv_instruction_format {
    RV_R_TYPE = 0,
    RV_I_TYPE,
    RV_S_TYPE,
    RV_B_TYPE,
    RV_U_TYPE,
    RV_J_TYPE,
} rv_instruction_format;

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

typedef union rv_instruction {
    uint32_t value;
} rv_instruction;

typedef enum rv_operation {
    RV_UNSUPPORTED = 0,
    /* RV32I instructions */
    RV32I_LUI,
    RV32I_AUIPC,
    RV32I_JAL,
    RV32I_JALR,
    RV32I_BEQ,
    RV32I_BNE,
    RV32I_BLT,
    RV32I_BGE,
    RV32I_BLTU,
    RV32I_BGEU,
    RV32I_LB,
    RV32I_LH,
    RV32I_LW,
    RV32I_LBU,
    RV32I_LHU,
    RV32I_SB,
    RV32I_SH,
    RV32I_SW,
    RV32I_ADDI,
    RV32I_SLTI,
    RV32I_SLTIU,
    RV32I_XORI,
    RV32I_ORI,
    RV32I_ANDI,
    RV32I_SLLI,
    RV32I_SRLI,
    RV32I_SRAI,
    RV32I_ADD,
    RV32I_SUB,
    RV32I_SLL,
    RV32I_SLT,
    RV32I_SLTU,
    RV32I_XOR,
    RV32I_SRL,
    RV32I_SRA,
    RV32I_OR,
    RV32I_AND,
    RV32I_FENCE,
    RV32I_FENCEI,
    RV32I_ECALL,
    RV32I_EBREAK,
    RV32I_CSRRW,
    RV32I_CSRRS,
    RV32I_CSRRC,
    RV32I_CSRRWI,
    RV32I_CSRRSI,
    RV32I_CSRRCI,

    /* RV64I instruction */
    RV64I_MUL,
    RV64I_LD,
    RV64I_SD,
    RV64I_SLLI,
    RV64I_SRLI,
    RV64I_SRAI,
    RV64I_ADDIW,
    RV64I_SLLIW,
    RV64I_SRLIW,
    RV64I_SRAIW,
    RV64I_ADDW,
    RV64I_SUBW,
    RV64I_SLLW,
    RV64I_SRLW,
    RV64I_SRAW,
} rv_operation;

typedef struct rv_decoded_instruction {
    rv_operation    op;
    rv_instruction  format;
} rv_decoded_instruction;

static const rv_instruction_format rv_op_format_table[] = {
    /* RV32I instructions */
    [RV32I_LUI]     = RV_U_TYPE,
    [RV32I_AUIPC]   = RV_U_TYPE,
    [RV32I_JAL]     = RV_J_TYPE,
    [RV32I_JALR]    = RV_I_TYPE,
    [RV32I_BEQ]     = RV_B_TYPE,
    [RV32I_BNE]     = RV_B_TYPE,
    [RV32I_BLT]     = RV_B_TYPE,
    [RV32I_BGE]     = RV_B_TYPE,
    [RV32I_BLTU]    = RV_B_TYPE,
    [RV32I_BGEU]    = RV_B_TYPE,
    [RV32I_LB]      = RV_I_TYPE,
    [RV32I_LH]      = RV_I_TYPE,
    [RV32I_LW]      = RV_I_TYPE,
    [RV32I_LBU]     = RV_I_TYPE,
    [RV32I_LHU]     = RV_I_TYPE,
    [RV32I_SB]      = RV_S_TYPE,
    [RV32I_SH]      = RV_S_TYPE,
    [RV32I_SW]      = RV_S_TYPE,
    [RV32I_ADDI]    = RV_I_TYPE,
    [RV32I_SLTI]    = RV_I_TYPE,
    [RV32I_SLTIU]   = RV_I_TYPE,
    [RV32I_XORI]    = RV_I_TYPE,
    [RV32I_ORI]     = RV_I_TYPE,
    [RV32I_ANDI]    = RV_I_TYPE,
    [RV32I_SLLI]    = RV_R_TYPE,
    [RV32I_SRLI]    = RV_R_TYPE,
    [RV32I_SRAI]    = RV_R_TYPE,
    [RV32I_ADD]     = RV_R_TYPE,
    [RV32I_SUB]     = RV_R_TYPE,
    [RV32I_SLL]     = RV_R_TYPE,
    [RV32I_SLT]     = RV_R_TYPE,
    [RV32I_SLTU]    = RV_R_TYPE,
    [RV32I_XOR]     = RV_R_TYPE,
    [RV32I_SRL]     = RV_R_TYPE,
    [RV32I_SRA]     = RV_R_TYPE,
    [RV32I_OR]      = RV_R_TYPE,
    [RV32I_AND]     = RV_R_TYPE,
    /* It get's more dicey after this point */
    [RV32I_FENCE]   = RV_I_TYPE,
    [RV32I_FENCEI]  = RV_I_TYPE,
    [RV32I_ECALL]   = RV_I_TYPE,
    [RV32I_EBREAK]  = RV_I_TYPE,
    [RV32I_CSRRW]   = RV_I_TYPE,
    [RV32I_CSRRS]   = RV_I_TYPE,
    [RV32I_CSRRC]   = RV_I_TYPE,
    [RV32I_CSRRWI]  = RV_I_TYPE,
    [RV32I_CSRRSI]  = RV_I_TYPE,
    [RV32I_CSRRCI]  = RV_I_TYPE,

    /* RV64I instructions */
    [RV64I_MUL]     = RV_I_TYPE,
    [RV64I_LD]      = RV_I_TYPE,
    [RV64I_SD]      = RV_S_TYPE,
    [RV64I_SLLI]    = RV_R_TYPE,
    [RV64I_SRLI]    = RV_R_TYPE,
    [RV64I_SRAI]    = RV_R_TYPE,
    [RV64I_ADDIW]   = RV_I_TYPE,
    [RV64I_SLLIW]   = RV_R_TYPE,
    [RV64I_SRLIW]   = RV_R_TYPE,
    [RV64I_SRAIW]   = RV_R_TYPE,
    [RV64I_ADDW]    = RV_R_TYPE,
    [RV64I_SUBW]    = RV_R_TYPE,
    [RV64I_SLLW]    = RV_R_TYPE,
    [RV64I_SRLW]    = RV_R_TYPE,
    [RV64I_SRAW]    = RV_R_TYPE,
};

static const char *const rv_op_names[] = {
    [RV_UNSUPPORTED]    = "UNSUPPORTED",
    [RV32I_LUI]         = "LUI",
    [RV32I_AUIPC]       = "AUIPC",
    [RV32I_JAL]         = "JAL",
    [RV32I_JALR]        = "JALR",
    [RV32I_BEQ]         = "BEQ",
    [RV32I_BNE]         = "BNE",
    [RV32I_BLT]         = "BLT",
    [RV32I_BGE]         = "BGE",
    [RV32I_BLTU]        = "BLTU",
    [RV32I_BGEU]        = "BGEU",
    [RV32I_LB]          = "LB",
    [RV32I_LH]          = "LH",
    [RV32I_LW]          = "LW",
    [RV32I_LBU]         = "LBU",
    [RV32I_LHU]         = "LHU",
    [RV32I_SB]          = "SB",
    [RV32I_SH]          = "SH",
    [RV32I_SW]          = "SW",
    [RV32I_ADDI]        = "ADDI",
    [RV32I_SLTI]        = "SLTI",
    [RV32I_SLTIU]       = "SLTIU",
    [RV32I_XORI]        = "XORI",
    [RV32I_ORI]         = "ORI",
    [RV32I_ANDI]        = "ANDI",
    [RV32I_SLLI]        = "SLLI",
    [RV32I_SRLI]        = "SRLI",
    [RV32I_SRAI]        = "SRAI",
    [RV32I_ADD]         = "ADD",
    [RV32I_SUB]         = "SUB",
    [RV32I_SLL]         = "SLL",
    [RV32I_SLT]         = "SLT",
    [RV32I_SLTU]        = "SLTU",
    [RV32I_XOR]         = "XOR",
    [RV32I_SRL]         = "SRL",
    [RV32I_SRA]         = "SRA",
    [RV32I_OR]          = "OR",
    [RV32I_AND]         = "AND",
    [RV32I_FENCE]       = "FENCE",
    [RV32I_FENCEI]      = "FENCEI",
    [RV32I_ECALL]       = "ECALL",
    [RV32I_EBREAK]      = "EBREAK",
    [RV32I_CSRRW]       = "CSRRW",
    [RV32I_CSRRS]       = "CSRRS",
    [RV32I_CSRRC]       = "CSRRC",
    [RV32I_CSRRWI]      = "CSRRWI",
    [RV32I_CSRRSI]      = "CSRRSI",
    [RV32I_CSRRCI]      = "CSRRCI",
    [RV64I_MUL]         = "MUL",
    [RV64I_LD]          = "LD",
    [RV64I_SD]          = "SD",
    [RV64I_SLLI]        = "SLLI",
    [RV64I_SRLI]        = "SRLI",
    [RV64I_SRAI]        = "SRAI",
    [RV64I_ADDIW]       = "ADDIW",
    [RV64I_SLLIW]       = "SLLIW",
    [RV64I_SRLIW]       = "SRLIW",
    [RV64I_SRAIW]       = "SRAIW",
    [RV64I_ADDW]        = "ADDW",
    [RV64I_SUBW]        = "SUBW",
    [RV64I_SLLW]        = "SLLW",
    [RV64I_SRLW]        = "SRLW",
    [RV64I_SRAW]        = "SRAW",
};

#endif
