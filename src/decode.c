#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <inttypes.h>

#include "decode.h"
#include "opcodes.h"
#include "cpu.h"
#include "csr.h"
#include "utils.h"

#define UNSUPPORTED(op) do { \
    printf("Unsupported instruction opcode 0x%02X\n", op); \
    exit(EXIT_FAILURE); \
} while (0)

#define BIT_SET(x, bit) (!!((uint32_t)(x) & (uint32_t)(1 << bit)))

rvi_register
rv_i_type_immediate(rv_instruction inst)
{
    uint32_t value = (inst.value >> 20) & 0xFFF;
    if (BIT_SET(inst.value, 31)) {
        value |= 0xFFFFF000;
    }
    return (rvi_register)((int32_t)value);
}

rvi_register
rv_s_type_immediate(rv_instruction inst)
{
    uint32_t value = ((inst.value >> 7)  & 0x1F) |
                     (((inst.value >> 25) & 0x7F) << 5);
    if (BIT_SET(inst.value, 31)) {
        value |= 0xFFFFF000;
    }
    return (rvi_register)((int32_t)value);
}

rvi_register
rv_b_type_immediate(rv_instruction inst)
{
    uint32_t imm12 = ((inst.value & (1U << 31))) >> (31-12);
    uint32_t imm10_5 = (inst.value & (0x3F << 25)) >> (25 - 5);
    uint32_t imm4_1 = (inst.value & (0xF << 8)) >> (8 - 1);
    uint32_t imm11 = (inst.value & (0x1 << 7)) << (11 - 7);
    uint32_t value = imm12 | imm11 | imm10_5 | imm4_1;

    if (BIT_SET(inst.value, 31)) {
        value |= 0xFFFFE000;
    }
    return (rvi_register)((int32_t)value);
}

rvi_register
rv_u_type_immediate(rv_instruction inst)
{
    uint32_t value = (inst.value & 0xFFFFF000);
    return (rvi_register)((int32_t)value);
}

rvi_register
rv_j_type_immediate(rv_instruction inst)
{
    uint32_t immediate = inst.value & 0xFFFFF000;
    uint32_t imm20 = (immediate & 0x80000000) >> 31;
    uint32_t imm10_1 = (immediate & 0x7FE00000) >> 21;
    uint32_t imm11 = (immediate & 0x00100000) >> 20;
    uint32_t imm19_12 = (immediate & 0x000FF000) >> 12;
    uint32_t value = (imm20 << 20) | (imm19_12 << 12) |
                     (imm11 << 11) | (imm10_1 << 1);
    if (BIT_SET(inst.value, 31)) {
        value |= 0xFFE00000;
    }
    return (rvi_register)((int32_t)value);
}

rvi_register
rv_get_immediate(rv_decoded_instruction inst)
{
    switch (rv_op_format_table[inst.op]) {
        case RV_R_TYPE:
            assert(0 && "R-type instruction has no immediate");
            break;
        case RV_I_TYPE:
            return rv_i_type_immediate(inst.format);
        case RV_S_TYPE:
            return rv_s_type_immediate(inst.format);
        case RV_B_TYPE:
            return rv_b_type_immediate(inst.format);
            break;
        case RV_U_TYPE:
            return rv_u_type_immediate(inst.format);
            break;
        case RV_J_TYPE:
            return rv_j_type_immediate(inst.format);
            break;
        default:
            assert(0 && "Invalid instruction type");
            break;     
    }
    return 0;
}

static rv_operation
rv_decode_branch_instruction(rv_instruction inst)
{
    static const rv_operation branch_op_table[8] = {
        [0] = RV32I_BEQ,
        [1] = RV32I_BNE,
        [4] = RV32I_BLT,
        [5] = RV32I_BGE,
        [6] = RV32I_BLTU,
        [7] = RV32I_BGEU
    };

    return branch_op_table[RV_INSTRUCTION_FUNCT3(inst)];
}

static rv_operation
rv_decode_load_instruction(rv_instruction inst)
{
    static const rv_operation load_op_table[8] = {
        [0] = RV32I_LB,
        [1] = RV32I_LH,
        [2] = RV32I_LW,
        [3] = RV64I_LD,
        [4] = RV32I_LBU,
        [5] = RV32I_LHU,
        [6] = RV64I_LWU,
    };

    return load_op_table[RV_INSTRUCTION_FUNCT3(inst)];
}

static rv_operation
rv_decode_store_instruction(rv_instruction inst)
{
    static const rv_operation store_op_table[8] = {
        [0] = RV32I_SB,
        [1] = RV32I_SH,
        [2] = RV32I_SW,
        [3] = RV64I_SD,
    };
    
    return store_op_table[RV_INSTRUCTION_FUNCT3(inst)];
}

static rv_operation
rv_decode_op_immediate_instruction(rv_instruction inst)
{
    static const rv_operation op_immediate_table[8] = {
        [0] = RV32I_ADDI,
        [2] = RV32I_SLTI,
        [3] = RV32I_SLTIU,
        [4] = RV32I_XORI,
        [6] = RV32I_ORI,
        [7] = RV32I_ANDI,
        [1] = RV32I_SLLI,
        [5] = RV32I_SRLI,
    };

    rv_operation op = op_immediate_table[RV_INSTRUCTION_FUNCT3(inst)];

    /* Handle special case for distinguishing SRLI and SRAI */
    switch (op) {
        case RV32I_SRLI:
            if (BIT_SET(inst.value, 30)) return RV32I_SRAI;
            /* fallthrough */
        case RV32I_SLLI:
            /* This is a hack to account for the shamt being one bit larger */
            if (RV_INSTRUCTION_FUNCT7(inst) >> 1) return RV_UNSUPPORTED;
            break;
        default:
            break;
    }

    return op;
}

static rv_operation
rv_decode_op_immediate_32_instruction(rv_instruction inst)
{
    static const rv_operation op_table[8] = {
        [0] = RV64I_ADDIW,
        [1] = RV64I_SLLIW,
        [5] = RV64I_SRLIW,
    };

    rv_operation op = op_table[RV_INSTRUCTION_FUNCT3(inst)];

    if (op == RV64I_SRLIW) {
        if (RV_INSTRUCTION_FUNCT7(inst) == 0x20) {
            op = RV64I_SRAIW;
        }
        else if (RV_INSTRUCTION_FUNCT7(inst) != 0x20) {
            op = RV_UNSUPPORTED;
        }
    }

    return op;
}

static rv_operation
rv_decode_op_instruction(rv_instruction inst)
{
    static const rv_operation op_table[8] = {
        [0] = RV32I_ADD,
        [1] = RV32I_SLL,
        [2] = RV32I_SLT,
        [3] = RV32I_SLTU,
        [4] = RV32I_XOR,
        [5] = RV32I_SRL,
        [6] = RV32I_OR,
        [7] = RV32I_AND,
    };

    rv_operation op = op_table[RV_INSTRUCTION_FUNCT3(inst)];

    /* Handle special case for ADD/SUB and SRL/SRA */
    switch (op) {
        case RV32I_ADD:
            if (BIT_SET(inst.value, 30)) return RV32I_SUB;
            break;
        case RV32I_SRL:
            if (BIT_SET(inst.value, 30)) return RV32I_SRA;
            break;
        default:
            break;
    }
    
    if (RV_INSTRUCTION_FUNCT7(inst)) return RV_UNSUPPORTED;

    return op;
}

static rv_operation
rv_decode_op_32_instruction(rv_instruction inst)
{
    static const rv_operation op_table[8] = {
        [0] = RV64I_ADDW,
        [1] = RV64I_SLLW,
        [5] = RV64I_SRLW,
    };

    rv_operation op = op_table[RV_INSTRUCTION_FUNCT3(inst)];

    switch (op) {
        case RV64I_ADDW:
            if (BIT_SET(inst.value, 30)) return RV64I_SUBW;
            break;
        case RV64I_SRLW:
            if (BIT_SET(inst.value, 30)) return RV64I_SRAW;
            break;
        default:
            break;
    }

    return op;
}

static rv_operation
rv_decode_misc_mem_instruction(rv_instruction inst)
{
    static const rv_operation misc_mem_op_table[8] = {
        [0] = RV32I_FENCE,
        [1] = RV32I_FENCEI,
    };

    rv_operation op = misc_mem_op_table[RV_INSTRUCTION_FUNCT3(inst)];

    if (op == RV32I_FENCE) {
        if (inst.value & 0xF00FFFF0 ) {
            return RV_UNSUPPORTED;
        }
    }
    else if (op == RV32I_FENCEI) {
        if (inst.value & 0xFFFFEFF0) {
            return RV_UNSUPPORTED;
        }
    }

    return op;
}

static rv_operation
rv_decode_system_instruction(rv_instruction inst)
{
    static const rv_operation system_op_table[8] = {
        [0] = RV32I_ECALL,
        [1] = RV32I_CSRRW,
        [2] = RV32I_CSRRS,
        [3] = RV32I_CSRRC,
        [5] = RV32I_CSRRWI,
        [6] = RV32I_CSRRSI,
        [7] = RV32I_CSRRCI,
    };

    rv_operation op = system_op_table[RV_INSTRUCTION_FUNCT3(inst)];

    if (op == RV32I_ECALL) {
        switch (inst.value) {
            case 0x00000073:
                return RV32I_ECALL;
            case 0x00100073:
                return RV32I_EBREAK;
            case 0x10200073:
                return RV_SRET;
            case 0x30200073:
                return RV_MRET;
            default:
                return RV_UNSUPPORTED;
        }
    }

    return op;
}

rv_operation
rv_decode_instruction(rv_instruction inst)
{
    uint8_t opcode = RV_INSTRUCTION_OPCODE(inst);

    switch (opcode) {
        case RV_LOAD_OPCODE:
            return rv_decode_load_instruction(inst);
            break;
        case RV_LOAD_FP_OPCODE:
            UNSUPPORTED(opcode);
            break;
        case RV_CUSTOM_0_OPCODE:
            UNSUPPORTED(opcode);
            break;
        case RV_MISC_MEM_OPCODE:
            return rv_decode_misc_mem_instruction(inst);
            break;
        case RV_OP_IMM_OPCODE:
            return rv_decode_op_immediate_instruction(inst);
            break;
        case RV_AUIPC_OPCODE:
            return RV32I_AUIPC;
            break;
        case RV_OP_IMM_32_OPCODE:
            return rv_decode_op_immediate_32_instruction(inst);
            break;
        case RV_STORE_OPCODE:
            return rv_decode_store_instruction(inst);
            break;
        case RV_STORE_FP_OPCODE:
            UNSUPPORTED(opcode);
            break;
        case RV_CUSTOM_1_OPCODE:
            UNSUPPORTED(opcode);
            break;
        case RV_AMO_OPCODE:
            UNSUPPORTED(opcode);
            break;
        case RV_OP_OPCODE:
            return rv_decode_op_instruction(inst);
            break;
        case RV_LUI_OPCODE:
            return RV32I_LUI;
            break;
        case RV_OP_32_OPCODE:
            return rv_decode_op_32_instruction(inst);
            UNSUPPORTED(opcode);
            break;
        case RV_MADD_OPCODE:
            UNSUPPORTED(opcode);
            break;
        case RV_MSUB_OPCODE:
            UNSUPPORTED(opcode);
            break;
        case RV_NMSUB_OPCODE:
            UNSUPPORTED(opcode);
            break;
        case RV_NMADD_OPCODE:
            UNSUPPORTED(opcode);
            break;
        case RV_OP_FP_OPCODE:
            UNSUPPORTED(opcode);
            break;
        case RV_CUSTOM_2_OPCODE:
            UNSUPPORTED(opcode);
            break;
        case RV_BRANCH_OPCODE:
            return rv_decode_branch_instruction(inst);
            break;
        case RV_JALR_OPCODE:
            return RV32I_JALR;
            break;
        case RV_JAL_OPCODE:
            return RV32I_JAL;
            break;
        case RV_SYSTEM_OPCODE:
            return rv_decode_system_instruction(inst);
            break;
        case RV_CUSTOM_3_OPCODE:
            UNSUPPORTED(opcode);
            break;
        default:
            UNSUPPORTED(opcode);
            break;
    }
}

static inline int
rv_is_csr_instruction(rv_decoded_instruction inst)
{
    uint32_t opcode = RV_INSTRUCTION_OPCODE(inst.format);
    uint32_t funct3 = RV_INSTRUCTION_FUNCT3(inst.format);
    return (opcode == 0x73) && (funct3 != 0x0);
}

void
rv_print_decoded_instruction(rv_decoded_instruction inst)
{
    rv_instruction_format format = rv_op_format_table[inst.op];

    printf("[ %08X ]\t", inst.format.value);

    printf("%s\t", rv_op_names[inst.op]);

    switch (format) {
        case RV_R_TYPE:
            printf("%s,%s,%s\n",
                register_names[RV_INSTRUCTION_RD(inst.format)],
                register_names[RV_INSTRUCTION_RS1(inst.format)],
                register_names[RV_INSTRUCTION_RS2(inst.format)]);
            break;
        case RV_I_TYPE:
            if (!rv_is_csr_instruction(inst)) {
                printf("%s,%s,%"PRId64"\n",
                    register_names[RV_INSTRUCTION_RD(inst.format)],
                    register_names[RV_INSTRUCTION_RS1(inst.format)],
                    (int64_t)rv_i_type_immediate(inst.format));
            }
            else {
                uint64_t csr = rv_i_type_immediate(inst.format) & 0xFFF;
                printf("%s,%s,%s\n",
                    register_names[RV_INSTRUCTION_RD(inst.format)],
                    register_names[RV_INSTRUCTION_RS1(inst.format)],
                    rv_csr_names[csr]);
            }
            break;
        case RV_S_TYPE:
            printf("%s,%s,%"PRId64"\n",
                register_names[RV_INSTRUCTION_RS1(inst.format)],
                register_names[RV_INSTRUCTION_RS2(inst.format)],
                (int64_t)rv_s_type_immediate(inst.format));
            break;
        case RV_B_TYPE:
            printf("%s,%s,%"PRId64"\n",
                register_names[RV_INSTRUCTION_RS1(inst.format)],
                register_names[RV_INSTRUCTION_RS2(inst.format)],
                (int64_t)rv_b_type_immediate(inst.format));
            break;
        case RV_U_TYPE:
            printf("%s,%"PRId64"\n",
                register_names[RV_INSTRUCTION_RD(inst.format)],
                (int64_t)rv_u_type_immediate(inst.format));
            break;
        case RV_J_TYPE:
            printf("%s,%"PRId64"\n",
                register_names[RV_INSTRUCTION_RD(inst.format)],
                (int64_t)rv_j_type_immediate(inst.format));
            break;
        default:
            printf("Invalid instruction\n");
            break;
    }
}
