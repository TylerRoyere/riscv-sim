#include "csr.h"
#include "cpu.h"
#include "utils.h"

#define SUFFICIENT_PRIVILEGE(address, state) \
    (RV_CSR_PRIVILEGE(address) >= (state)->privilege)

rv_csr
rv_csr_read(rv_csr_addr address, rv_cpu_state *state)
{
    if (!SUFFICIENT_PRIVILEGE(address, state)) {
        TODO("This illegal csr access should throw an exception\n");
        return 0;
    }

    switch (address) {
        case RV_CSR_MTVEC:
            return state->csrs.mtvec;
        case RV_CSR_MEPC:
            return state->csrs.mepc;
        case RV_CSR_MCAUSE:
            return state->csrs.mcause;
        default:
            TODO("rv_csr_read(%s, %p) = 0\n",
                rv_csr_names[address], (void*)state);
            return 0;
    }
}

void
rv_csr_write(rv_csr_addr address, rv_cpu_state *state,
        rvi_register value)
{
    if (RV_CSR_IS_READONLY(address)) {
        TODO("This illegal csr write should throw an exception\n");
        return;
    }
    if (!SUFFICIENT_PRIVILEGE(address, state)) {
        TODO("This illegal csr access should throw an exception\n");
        return;
    }

    switch (address) {
        case RV_CSR_MTVEC:
            state->csrs.mtvec = value;
            break;
        case RV_CSR_MEPC:
            state->csrs.mepc = value;
            break;
        case RV_CSR_MCAUSE:
            TODO("Support only legal excpetions for writing");
            state->csrs.mcause = value;
            break;
        default:
            TODO("rv_csr_write(%s, %p, %ld)\n",
                rv_csr_names[address], (void*)state, value);
            break;
    }

}

void
rv_csr_set_bits(rv_csr_addr address, rv_cpu_state *state,
        rvi_register mask)
{
    rv_csr temp = rv_csr_read(address, state);
    temp |= mask;
    rv_csr_write(address, state, temp);
}

void
rv_csr_clear_bits(rv_csr_addr address, rv_cpu_state *state,
        rvi_register mask)
{
    
    rv_csr temp = rv_csr_read(address, state);
    temp &= ~mask;
    rv_csr_write(address, state, temp);
}
