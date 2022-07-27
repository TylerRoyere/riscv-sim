#ifndef OPCODES_H
#define OPCODES_H

typedef enum rvi_base_opcodes {
    RV_LOAD_OPCODE          = 0x03,
    RV_LOAD_FP_OPCODE       = 0x07,
    RV_CUSTOM_0_OPCODE      = 0x0B,
    RV_MISC_MEM_OPCODE      = 0x0F,
    RV_OP_IMM_OPCODE        = 0x13,
    RV_AUIPC_OPCODE         = 0x17,
    RV_OP_IMM_32_OPCODE     = 0x1B,
    RV_48_BIT_1_OPCODE      = 0x1F,
    RV_STORE_OPCODE         = 0x23,
    RV_STORE_FP_OPCODE      = 0x27,
    RV_CUSTOM_1_OPCODE      = 0x2B,
    RV_AMO_OPCODE           = 0x2F,
    RV_OP_OPCODE            = 0x33,
    RV_LUI_OPCODE           = 0x37,
    RV_OP_32_OPCODE         = 0x3B,
    RV_64_BIT_OPCODE        = 0x3F,
    RV_MADD_OPCODE          = 0x43,
    RV_MSUB_OPCODE          = 0x47,
    RV_NMSUB_OPCODE         = 0x4B,
    RV_NMADD_OPCODE         = 0x4F,
    RV_OP_FP_OPCODE         = 0x53,
    /* opcode 0x15 reserved */
    RV_CUSTOM_2_OPCODE      = 0x5B,
    RV_48_BIT_2_OPCODE      = 0x5F,
    RV_BRANCH_OPCODE        = 0x63,
    RV_JALR_OPCODE          = 0x67,
    /* opcode 0x1A reserved */
    RV_JAL_OPCODE           = 0x6F,
    RV_SYSTEM_OPCODE        = 0x73,
    /* opcode 0x1D reserved */
    RV_CUSTOM_3_OPCODE      = 0x7B,
    RV_80_BIT_OPCODE        = 0x7F,
} rvi_base_opcodes;

#endif
