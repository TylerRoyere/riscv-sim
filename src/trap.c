#include "trap.h"
#include "utils.h"
#include "csr.h"

static rv_cpu_privilege
rv_pop_machine_privilege_stack(rv_cpu_state *state)
{
    rv_csr mstatus = rv_csr_read(RV_CSR_MSTATUS, state);
    rv_cpu_privilege mpp = RV_CSR_MSTATUS_GET(mstatus, MPP);
    rv_csr mpie = RV_CSR_MSTATUS_GET(mstatus, MPIE);
    mstatus = RV_CSR_MSTATUS_SET(mstatus, MIE, mpie >> 4);
    mstatus = RV_CSR_MSTATUS_SET(mstatus, MPIE, 1 << 8);
    rv_csr_write(RV_CSR_MSTATUS, state, mstatus);
    mstatus = RV_CSR_MSTATUS_SET(mstatus, MPP, RV_USER << 11);
    mstatus = RV_CSR_MSTATUS_SET(mstatus, MPRV, 0);
    rv_csr_write(RV_CSR_MSTATUS, state, mstatus);
    return mpp;
}

static rv_cpu_privilege
rv_pop_supervisor_privilege_stack(rv_cpu_state *state)
{
    rv_csr mstatus = rv_csr_read(RV_CSR_MSTATUS, state);
    rv_cpu_privilege spp = RV_CSR_MSTATUS_GET(mstatus, SPP);
    rv_csr spie = RV_CSR_MSTATUS_GET(mstatus, SPIE);
    mstatus = RV_CSR_MSTATUS_SET(mstatus, SIE, spie >> 4);
    mstatus = RV_CSR_MSTATUS_SET(mstatus, SPIE, 1 << 5);
    rv_csr_write(RV_CSR_MSTATUS, state, mstatus);
    mstatus = RV_CSR_MSTATUS_SET(mstatus, SPP, RV_USER << 8);
    mstatus = RV_CSR_MSTATUS_SET(mstatus, MPRV, 0);
    rv_csr_write(RV_CSR_MSTATUS, state, mstatus);
    return spp;
}

void
rv_do_trap(int is_interrupt, rvi_register code, rv_cpu_state *state)
{
    if (is_interrupt) {
        TODO("Handle interrupts\n");
    }
    TODO("Trap delegation\n");
    rvi_register new_pc = 0;
    switch (code) {
        case RV_TRAP_U_ECALL:
            /* fall through */
        case RV_TRAP_S_ECALL:
            /* fall through */
        case RV_TRAP_M_ECALL:
            new_pc = (rvi_register)rv_csr_read(RV_CSR_MTVEC, state);
            rv_csr_write(RV_CSR_MCAUSE, state, code);
            if (new_pc & 3) {
                TODO("Support trap vector modes\n");
            }
            state->rvi_pc = new_pc;
            state->flags |= RV_TRAP_TAKEN_FLAG;
            break;
        default:
            TODO("Handle this trap code %ld\n", code);
    }
}

void
execute_MRET(rv_instruction inst, rv_cpu_state *state)
{
    (void) inst;
    if (state->privilege != RV_MACHINE) {
        TODO("Executing MRET from lower privilege level "
               "should throw exception\n");
        return;
    }

    rvi_register new_pc = rv_csr_read(RV_CSR_MEPC, state);

    state->privilege = rv_pop_machine_privilege_stack(state); 

    state->rvi_pc = new_pc;
}


void
execute_SRET(rv_instruction inst, rv_cpu_state *state)
{
    (void) inst;
    if (state->privilege < RV_SUPERVISOR) {
        TODO("Executing MRET from lower privilege level "
               "should throw exception\n");
        return;
    }

    rvi_register new_pc = rv_csr_read(RV_CSR_SEPC, state);

    rv_cpu_privilege priv = rv_pop_supervisor_privilege_stack(state);

    if (state->privilege == RV_MACHINE) {
        rv_pop_supervisor_privilege_stack(state);
    }

    state->privilege = priv;

    state->rvi_pc = new_pc;
}
