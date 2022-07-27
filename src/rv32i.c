#include "rv32i.h"
#include "decode.h"
#include "register.h"

#define UNPACK_R_TYPE(inst, rs1, rs2, rd) \
    uint8_t rs1 = RV_INSTRUCTION_RS1(inst); \
    uint8_t rs2 = RV_INSTRUCTION_RS2(inst); \
    uint8_t rd = RV_INSTRUCTION_RD(inst)

#define UNPACK_U_TYPE(inst, imm, rd) \
    rvi_register imm = rv_u_type_immediate(inst); \
    uint8_t rd = RV_INSTRUCTION_RD(inst)

#define UNPACK_I_TYPE(inst, imm, rs, rd) \
    rvi_register imm = rv_i_type_immediate(inst); \
    uint8_t rs = RV_INSTRUCTION_RS1(inst); \
    uint8_t rd = RV_INSTRUCTION_RD(inst)

#define UNPACK_J_TYPE(inst, imm, rd) \
    rvi_register imm = rv_j_type_immediate(inst); \
    uint8_t rd = RV_INSTRUCTION_RD(inst)

#define UNPACK_S_TYPE(inst, offset, src, base) \
    rvi_register offset = rv_s_type_immediate(inst); \
    uint8_t src = RV_INSTRUCTION_RS2(inst); \
    uint8_t base = RV_INSTRUCTION_RS1(inst)

#define UNPACK_B_TYPE(inst, offset, rs1, rs2) \
    rvi_register offset = rv_b_type_immediate(inst); \
    uint8_t rs1 = RV_INSTRUCTION_RS1(inst); \
    uint8_t rs2 = RV_INSTRUCTION_RS2(inst)

#define GET_SHAMT(immediate) \
    (immediate & 0x1F)

static inline rvi_register
rv32i_jump(rvi_register target, rv_cpu_state *state)
{
    rvi_register next_pc = state->rvi_pc + 4;
    state->rvi_pc = target;
    rv_cpu_set_flag(state, RV_JUMP_TAKEN_FLAG);
    return next_pc;
}

static inline void
rv32i_take_branch(rvi_register offset, rv_cpu_state *state)
{
    state->rvi_pc += offset;
    rv_cpu_set_flag(state, RV_BRANCH_TAKEN_FLAG);
}



void
execute_LUI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_U_TYPE(inst, immediate, rd);
    rvi_reg_write(state, rd, immediate);
}

void
execute_AUIPC(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_U_TYPE(inst, immediate, rd);
    rvi_reg_write(state, rd, state->rvi_pc + immediate);
}

void
execute_JAL(rv_instruction inst, rv_cpu_state *state)
{
    /* TODO: Check for aligned target address */
    UNPACK_J_TYPE(inst, immediate, rd);
    rvi_register target = state->rvi_pc + immediate;
    rvi_reg_write(state, rd, rv32i_jump(target, state));
}

void
execute_JALR(rv_instruction inst, rv_cpu_state *state)
{
    /* TODO: Check for aligned target address */
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register target = rvi_reg_read(state, rs) + immediate;
    target &= (~1);
    rvi_reg_write(state, rd, rv32i_jump(target, state));
}

void
execute_BEQ(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_B_TYPE(inst, offset, rs1, rs2);
    if (rvi_reg_read(state, rs1) == rvi_reg_read(state, rs2)) {
        rv32i_take_branch(offset, state);
    }
}

void
execute_BNE(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_B_TYPE(inst, offset, rs1, rs2);
    if (rvi_reg_read(state, rs1) != rvi_reg_read(state, rs2)) {
        rv32i_take_branch(offset, state);
    }
}

void
execute_BLT(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_B_TYPE(inst, offset, rs1, rs2);
    if (rvi_reg_read(state, rs1) < rvi_reg_read(state, rs2)) {
        rv32i_take_branch(offset, state);
    }
}

void
execute_BGE(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_B_TYPE(inst, offset, rs1, rs2);
    if (rvi_reg_read(state, rs1) >= rvi_reg_read(state, rs2)) {
        rv32i_take_branch(offset, state);
    }
}

void
execute_BLTU(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_B_TYPE(inst, offset, rs1, rs2);
    rvi_register rs1_val = rvi_reg_read(state, rs1);
    rvi_register rs2_val = rvi_reg_read(state, rs2);
    if ((rvi_register_unsigned)rs1_val < (rvi_register_unsigned)rs2_val) {
        rv32i_take_branch(offset, state);
    }
}

void
execute_BGEU(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_B_TYPE(inst, offset, rs1, rs2);
    rvi_register_unsigned rs1_val = rvi_reg_read_unsigned(state, rs1);
    rvi_register_unsigned rs2_val = rvi_reg_read_unsigned(state, rs2);
    if (rs1_val >= rs2_val) {
        rv32i_take_branch(offset, state);
    }
}

void
execute_LB(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, offset, rs, rd);
    rvi_register address = rvi_reg_read(state, rs) + offset;
    int8_t value = (int8_t)rv_memory_read8(address);
    rvi_reg_write(state, rd, (rvi_register)value);
}

void
execute_LH(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, offset, rs, rd);
    rvi_register address = rvi_reg_read(state, rs) + offset;
    int16_t value = (int16_t)rv_memory_read16(address);
    rvi_reg_write(state, rd, (rvi_register)value);
}

void
execute_LW(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, offset, rs, rd);
    rvi_register address = rvi_reg_read(state, rs) + offset;
    rvi_reg_write(state, rd, rv_memory_read32(address));
}

void
execute_LBU(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, offset, rs, rd);
    rvi_register address = rvi_reg_read(state, rs) + offset;
    uint8_t value = rv_memory_read8(address);
    rvi_reg_write(state, rd, (rvi_register)value);
}

void
execute_LHU(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, offset, rs, rd);
    rvi_register address = rvi_reg_read(state, rs) + offset;
    uint16_t value = rv_memory_read16(address);
    rvi_reg_write(state, rd, (rvi_register)value);
}

void
execute_SB(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_S_TYPE(inst, offset, src, base);
    rvi_register address = rvi_reg_read(state, base) + offset;
    uint8_t value = (uint8_t)(rvi_reg_read(state, src) & 0xFF);
    rv_memory_write8(address, value);
}

void
execute_SH(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_S_TYPE(inst, offset, src, base);
    rvi_register address = rvi_reg_read(state, base) + offset;
    uint16_t value = (uint16_t)(rvi_reg_read(state, src) & 0xFFFF);
    rv_memory_write16(address, value);
}

void
execute_SW(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_S_TYPE(inst, offset, src, base);
    rvi_register address = rvi_reg_read(state, base) + offset;
    uint32_t value = (uint32_t)(rvi_reg_read(state, src) & 0xFFFFFFFF);
    rv_memory_write32(address, value);
}

void
execute_ADDI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register result = rvi_reg_read(state, rs) + immediate;
    rvi_reg_write(state, rd, result);
}

void
execute_SLTI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register lt = !!(rvi_reg_read(state, rs) < immediate);
    rvi_reg_write(state, rd, lt);
}

void
execute_SLTIU(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register ltu = !!(rvi_reg_read_unsigned(state, rs) <
            (rvi_register_unsigned)immediate);
    rvi_reg_write(state, rd, ltu);
}

void
execute_XORI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register value = rvi_reg_read(state, rs) ^ immediate;
    rvi_reg_write(state, rd, value);
}

void
execute_ORI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register value = rvi_reg_read(state, rs) | immediate;
    rvi_reg_write(state, rd, value);
}

void
execute_ANDI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register value = rvi_reg_read(state, rs) & immediate;
    rvi_reg_write(state, rd, value);
}

void
execute_SLLI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register shamt = GET_SHAMT(immediate);
    rvi_register value = rvi_reg_read(state, rs) << shamt;
    rvi_reg_write(state, rd, value);
}

void
execute_SRLI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register shamt = GET_SHAMT(immediate);
    rvi_register_unsigned value = rvi_reg_read_unsigned(state, rs) >> shamt;
    rvi_reg_write(state, rd, (rvi_register)value);
}

void
execute_SRAI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register shamt = GET_SHAMT(immediate);
    rvi_register value = rvi_reg_read(state, rs) >> shamt;
    rvi_reg_write(state, rd, value);
}

void
execute_ADD(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register result = rvi_reg_read(state, rs1) + rvi_reg_read(state, rs2);
    rvi_reg_write(state, rd, result);
}

void
execute_SUB(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register result = rvi_reg_read(state, rs1) - rvi_reg_read(state, rs2);
    rvi_reg_write(state, rd, result);
}

void
execute_SLL(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register shamt = rvi_reg_read(state, rs2) & 0x1F;
    rvi_register result = rvi_reg_read(state, rs1) << shamt;
    rvi_reg_write(state, rd, result);
}

void
execute_SLT(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register lt = !!(rvi_reg_read(state, rs1) < rvi_reg_read(state, rs2));
    rvi_reg_write(state, rd, lt);
}

void
execute_SLTU(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register ltu = !!(rvi_reg_read_unsigned(state, rs1) <
            rvi_reg_read_unsigned(state, rs2));
    rvi_reg_write(state, rd, ltu);
}

void
execute_XOR(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register result = rvi_reg_read(state, rs1) ^ rvi_reg_read(state, rs2);
    rvi_reg_write(state, rd, result);
}

void
execute_SRL(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register shamt = rvi_reg_read(state, rs2) & 0x1F;
    rvi_register_unsigned result = rvi_reg_read_unsigned(state, rs1) >> shamt;
    rvi_reg_write(state, rd, (rvi_register)result);
}

void
execute_SRA(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register shamt = rvi_reg_read(state, rs2) & 0x1F;
    rvi_register result = rvi_reg_read(state, rs1) >> shamt;
    rvi_reg_write(state, rd, result);
}

void
execute_OR(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register result = rvi_reg_read(state, rs1) | rvi_reg_read(state, rs2);
    rvi_reg_write(state, rd, result);
}

void
execute_AND(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register result = rvi_reg_read(state, rs1) & rvi_reg_read(state, rs2);
    rvi_reg_write(state, rd, result);
}

