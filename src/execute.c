#include "utils.h"
#include "execute.h"
#include "rv32i.h"
#include "rv64i.h"
#include "trap.h"

#define RV32I_CASE(op) \
    case RV32I_ ## op:                          \
        execute_ ## op (inst.format, state);    \
        break

#define RV64I_CASE(op) \
    case RV64I_ ## op:                          \
        execute_ ## op (inst.format, state);    \
        break


#define RV_CASE(op) \
    case RV_ ## op:                             \
        execute_ ## op (inst.format, state);    \
        break
void
rv_execute_instruction(rv_decoded_instruction inst, rv_cpu_state *state)
{
    switch (inst.op) {
        RV32I_CASE(LUI);
		RV32I_CASE(AUIPC);
		RV32I_CASE(JAL);
		RV32I_CASE(JALR);
		RV32I_CASE(BEQ);
		RV32I_CASE(BNE);
		RV32I_CASE(BLT);
		RV32I_CASE(BGE);
		RV32I_CASE(BLTU);
		RV32I_CASE(BGEU);
		RV32I_CASE(LB);
		RV32I_CASE(LH);
		RV32I_CASE(LW);
		RV32I_CASE(LBU);
		RV32I_CASE(LHU);
		RV32I_CASE(SB);
		RV32I_CASE(SH);
		RV32I_CASE(SW);
		RV32I_CASE(ADDI);
		RV32I_CASE(SLTI);
		RV32I_CASE(SLTIU);
		RV32I_CASE(XORI);
		RV32I_CASE(ORI);
		RV32I_CASE(ANDI);
/* Please note that these are actually RV64I instructions */
		RV32I_CASE(SLLI);
		RV32I_CASE(SRLI);
		RV32I_CASE(SRAI);
/**********************************************************/

		RV32I_CASE(ADD);
		RV32I_CASE(SUB);
		RV32I_CASE(SLL);
		RV32I_CASE(SLT);
		RV32I_CASE(SLTU);
		RV32I_CASE(XOR);
		RV32I_CASE(SRL);
		RV32I_CASE(SRA);
		RV32I_CASE(OR);
		RV32I_CASE(AND);
		RV32I_CASE(FENCE);
		RV32I_CASE(FENCEI);
		RV32I_CASE(ECALL);
		RV32I_CASE(EBREAK);
		RV32I_CASE(CSRRW);
		RV32I_CASE(CSRRS);
		RV32I_CASE(CSRRC);
		RV32I_CASE(CSRRWI);
		RV32I_CASE(CSRRSI);
		RV32I_CASE(CSRRCI);

		RV64I_CASE(LWU);
		RV64I_CASE(LD);
		RV64I_CASE(SD);
#if 0
		RV64I_CASE(SLLI);
		RV64I_CASE(SRLI);
		RV64I_CASE(SRAI);
#endif
		RV64I_CASE(ADDIW);
		RV64I_CASE(SLLIW);
		RV64I_CASE(SRLIW);
		RV64I_CASE(SRAIW);
		RV64I_CASE(ADDW);
		RV64I_CASE(SUBW);
		RV64I_CASE(SLLW);
		RV64I_CASE(SRLW);
		RV64I_CASE(SRAW);

        RV_CASE(SRET);
        RV_CASE(MRET);

        case RV_UNSUPPORTED:
            rv_print_regs(state);
            //rv_print_mem();
            /* fallthrough */
        default:
            printf("opcode = "); print_binary(RV_INSTRUCTION_OPCODE(inst.format), 7); printf("\n");
            printf("funct3 = "); print_binary(RV_INSTRUCTION_FUNCT3(inst.format), 3); printf("\n");
            printf("funct7 = "); print_binary(RV_INSTRUCTION_FUNCT7(inst.format), 7); printf("\n");
            printf("rs1    = "); print_binary(RV_INSTRUCTION_RS1(inst.format), 5);    printf("\n");
            printf("rs2    = "); print_binary(RV_INSTRUCTION_RS2(inst.format), 5);    printf("\n");
            printf("rd     = "); print_binary(RV_INSTRUCTION_RD(inst.format), 5);     printf("\n");
            assert(0 && "This is an unsupported or invalid instruction");
            break;
    }
}
