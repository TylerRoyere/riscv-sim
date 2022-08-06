#include "rv64i.h"
#include "utils.h"
#include "cpu.h"

void
execute_LWU(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, offset, rs, rd);
    rvi_register address = rvi_reg_read(state, rs) + offset;
    rv_cpu_check_watchpoint(state, address, 4);
    uint32_t value = (uint32_t)rv_memory_read64(address);
    rvi_reg_write(state, rd, ZERO_EXTEND(value, rvi_register));
}

void
execute_LD(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, offset, rs, rd);
    rvi_register address = rvi_reg_read(state, rs) + offset;
    rv_cpu_check_watchpoint(state, address, 8);
    rvi_register value = (rvi_register)rv_memory_read64(address);
    rvi_reg_write(state, rd, value);   
}

void
execute_SD(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_S_TYPE(inst, offset, src, base);
    rvi_register address = rvi_reg_read(state, base) + offset;
    rv_cpu_check_watchpoint(state, address, 8);
    rvi_register value = rvi_reg_read(state, src);
    rv_memory_write64(address, (uint64_t)value);
}

void
execute_ADDIW(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    int32_t result = (int32_t)rvi_reg_read(state, rs) + (int32_t)immediate;
    rvi_reg_write(state, rd, SIGN_EXTEND(result, rvi_register));
}

void
execute_SLLIW(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    uint8_t shamt = GET_SHAMT(immediate);
    uint32_t result = (uint32_t)rvi_reg_read(state, rs) << shamt;
    rvi_reg_write(state, rd, SIGN_EXTEND(result, rvi_register));
}

void
execute_SRLIW(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    uint8_t shamt = GET_SHAMT(immediate);
    uint32_t result = (uint32_t)rvi_reg_read(state, rs) >> shamt;
    rvi_reg_write(state, rd, SIGN_EXTEND(result, rvi_register));
}

void
execute_SRAIW(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    uint8_t shamt = GET_SHAMT(immediate);
    int32_t result = (int32_t)rvi_reg_read(state, rs) >> shamt;
    rvi_reg_write(state, rd, SIGN_EXTEND(result, rvi_register));
}

void
execute_ADDW(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    int32_t rs1_val = (int32_t)rvi_reg_read(state, rs1);
    int32_t rs2_val = (int32_t)rvi_reg_read(state, rs2);
    int32_t result = rs1_val + rs2_val;
    rvi_reg_write(state, rd, SIGN_EXTEND(result, rvi_register));
}

void
execute_SUBW(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    int32_t rs1_val = (int32_t)rvi_reg_read(state, rs1);
    int32_t rs2_val = (int32_t)rvi_reg_read(state, rs2);
    int32_t result = rs1_val - rs2_val;
    rvi_reg_write(state, rd, SIGN_EXTEND(result, rvi_register));
}

void
execute_SLLW(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    uint8_t shamt = (uint8_t)(rvi_reg_read(state, rs2) & 0x1F);
    uint32_t rs1_val = (uint32_t)rvi_reg_read(state, rs1);
    rvi_register result = SIGN_EXTEND(rs1_val << shamt, rvi_register);
    rvi_reg_write(state, rd, result);
}

void
execute_SRLW(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    uint8_t shamt = (uint8_t)(rvi_reg_read(state, rs2) & 0x1F);
    uint32_t rs1_val = (uint32_t)rvi_reg_read(state, rs1);
    rvi_register result = SIGN_EXTEND(rs1_val >> shamt, rvi_register);
    rvi_reg_write(state, rd, (rvi_register)result);
}

void
execute_SRAW(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    uint8_t shamt = (uint8_t)(rvi_reg_read(state, rs2) & 0x1F);
    int32_t rs1_val = (int32_t)rvi_reg_read(state, rs1);
    rvi_register result = SIGN_EXTEND(rs1_val >> shamt, rvi_register);
    rvi_reg_write(state, rd, result);
}

#if !defined(RV32_ONLY)
void
execute_SLLI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register shamt = GET_SHAMT6(immediate);
    rvi_register value = rvi_reg_read(state, rs) << shamt;
    rvi_reg_write(state, rd, value);
}

void
execute_SRLI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register shamt = GET_SHAMT6(immediate);
    rvi_register_unsigned value = rvi_reg_read_unsigned(state, rs) >> shamt;
    rvi_reg_write(state, rd, (rvi_register)value);
}

void
execute_SRAI(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_I_TYPE(inst, immediate, rs, rd);
    rvi_register shamt = GET_SHAMT6(immediate);
    rvi_register value = rvi_reg_read(state, rs) >> shamt;
    rvi_reg_write(state, rd, value);
}


void
execute_SLL(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register shamt = rvi_reg_read(state, rs2) & 0x3F;
    rvi_register result = rvi_reg_read(state, rs1) << shamt;
    rvi_reg_write(state, rd, result);
}

void
execute_SRL(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register shamt = rvi_reg_read(state, rs2) & 0x3F;
    rvi_register_unsigned result = rvi_reg_read_unsigned(state, rs1) >> shamt;
    rvi_reg_write(state, rd, (rvi_register)result);
}

void
execute_SRA(rv_instruction inst, rv_cpu_state *state)
{
    UNPACK_R_TYPE(inst, rs1, rs2, rd);
    rvi_register shamt = rvi_reg_read(state, rs2) & 0x3F;
    rvi_register result = rvi_reg_read(state, rs1) >> shamt;
    rvi_reg_write(state, rd, result);
}
#endif
