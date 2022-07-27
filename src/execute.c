#include "execute.h"
#include "rv32i.h"

void
rv_execute_instruction(rv_decoded_instruction inst, rv_cpu_state *state)
{
    switch (inst.op) {
        case RV32I_LUI:
            execute_LUI(inst.format, state);
            break;
        case RV32I_AUIPC:
            execute_AUIPC(inst.format, state);
            break;
        case RV32I_JAL:
            execute_JAL(inst.format, state);
            break;
        case RV32I_JALR:
            execute_JALR(inst.format, state);
            break;
        case RV32I_BEQ:
            execute_BEQ(inst.format, state);
            break;
        case RV32I_BNE:
            execute_BNE(inst.format, state);
            break;
        case RV32I_BLT:
            execute_BLT(inst.format, state);
            break;
        case RV32I_BGE:
            execute_BGE(inst.format, state);
            break;
        case RV32I_BLTU:
            execute_BLTU(inst.format, state);
            break;
        case RV32I_BGEU:
            execute_BGEU(inst.format, state);
            break;
        case RV32I_LW:
            execute_LW(inst.format, state);
            break;
        case RV32I_SW:
            execute_SW(inst.format, state);
            break;
        case RV32I_ADDI:
            execute_ADDI(inst.format, state);
            break;
        case RV32I_ADD:
            execute_ADD(inst.format, state);
            break;
        case RV32I_SUB:
            execute_SUB(inst.format, state);
            break;
        case RV_UNSUPPORTED:
            rv_print_regs(state);
            rv_print_mem();
            /* fallthrough */
        default:
            assert(0 && "This is an unsupported or invalid instruction");
            break;
    }
}
