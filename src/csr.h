#ifndef CSR_H
#define CSR_H

/* For now, use the length of integer register as CSR */
#include "register.h"
typedef rvi_register rv_csr;
typedef uint16_t rv_csr_addr;

#define RV_CSR_BIT_LENGTH 12
#define RV_NUM_CSRS (1 << RV_CSR_BIT_LENGTH)

#define RV_CSR_IS_READONLY(address) (address & (0xC00))
#define RV_CSR_PRIVILEGE(address) (address & RV_CSR_PRIVILEGE_MASK)

#define RV_CSR_ADDR_MASK 0xFFF

struct rv_cpu_state;
rv_csr rv_csr_read(rv_csr_addr address, struct rv_cpu_state *state);
void rv_csr_write(rv_csr_addr address, struct rv_cpu_state *state,
        rv_csr value);
void rv_csr_set_bits(rv_csr_addr address, struct rv_cpu_state *state,
        rv_csr mask);
void rv_csr_clear_bits(rv_csr_addr address, struct rv_cpu_state *state,
        rv_csr mask);

extern const unsigned char rv_csr_validity_table[RV_NUM_CSRS];
extern const char *const rv_csr_names[RV_NUM_CSRS];

typedef enum rv_csr_priviledge {
    RV_CSR_USER             = 0x000,
    RV_CSR_SUPERVISOR       = 0x100,
    RV_CSR_HYPERVISOR       = 0x200,
    RV_CSR_MACHINE          = 0x300,
    RV_CSR_PRIVILEGE_MASK   = 0x300,
} rv_csr_priviledge;

typedef enum rv_csr_validity {
    RV_CSR_INVALID          = 0,
    RV_CSR_VALID            = 1,
    RV_CSR_RV32_ONLY        = 2,
    RV_CSR_HSXLEN32_ONLY    = 3,
} rv_csr_validity;

#define  RV_CSR_MSTATUS_SIE_MASK    (1 << 1)
#define  RV_CSR_MSTATUS_MIE_MASK    (1 << 3)
#define  RV_CSR_MSTATUS_SPIE_MASK   (1 << 5)
#define  RV_CSR_MSTATUS_UBE_MASK    (1 << 6)
#define  RV_CSR_MSTATUS_MPIE_MASK   (1 << 7)
#define  RV_CSR_MSTATUS_SPP_MASK    (1 << 8)
#define  RV_CSR_MSTATUS_VS_MASK     (3 << 9)
#define  RV_CSR_MSTATUS_MPP_MASK    (3 << 11)
#define  RV_CSR_MSTATUS_FS_MASK     (3 << 13)
#define  RV_CSR_MSTATUS_XS_MASK     (3 << 15)
#define  RV_CSR_MSTATUS_MPRV_MASK   (1 << 17)
#define  RV_CSR_MSTATUS_SUM_MASK    (1 << 18)
#define  RV_CSR_MSTATUS_MXR_MASK    (1 << 19)
#define  RV_CSR_MSTATUS_TVM_MASK    (1 << 20)
#define  RV_CSR_MSTATUS_TW_MASK     (1 << 21)
#define  RV_CSR_MSTATUS_TSR_MASK    (1 << 22)
#define  RV_CSR_MSTATUS_UXL_MASK    (3 << 32)
#define  RV_CSR_MSTATUS_SXL_MASK    (3 << 34)
#define  RV_CSR_MSTATUS_SBE_MASK    (1 << 36)
#define  RV_CSR_MSTATUS_MBE_MASK    (1 << 37)
#define  RV_CSR_MSTATUS_SD_MASK     (1 << 63)

#define RV_CSR_MSTATUS_GET(x, field) \
    ((x) & (RV_CSR_MSTATUS_ ## field ## _MASK))

#define RV_CSR_MSTATUS_SET(x, field, value) \
    (((x) & (~RV_CSR_MSTATUS_ ## field ## _MASK)) | (value))

typedef struct rv_csr_state {
    /* Unprivileged CSR addresses */
    rv_csr fflags;
    rv_csr frm;
    rv_csr fcsr;
    rv_csr cycle;
    rv_csr time;
    rv_csr instret;
    rv_csr hpmcounter[32-3 /* 3 -> 32 */];
#if 0
    rv_csr cycleh;
    rv_csr timeh;
    rv_csr instreth;
    rv_csr hpmcounterh[32-3 /* first = 3, last = 31*/];
#endif
    
    /* Supervisor CSR addresses */
    rv_csr sstatus;
    rv_csr sie;
    rv_csr stvec;
    rv_csr scounteren;
    rv_csr senvcfg;
    rv_csr sscratch;
    rv_csr sepc;
    rv_csr scause;
    rv_csr stval;
    rv_csr sip;
    rv_csr satp;
    rv_csr scontext;

    /* Hypervisor CSR addresses */
    rv_csr hstatus;
    rv_csr hedeleg;
    rv_csr hideleg;
    rv_csr hie;
    rv_csr hcounteren;
    rv_csr hgeie;
    rv_csr henvcfg;
//    rv_csr henvcfgh;
    rv_csr htval;
    rv_csr hip;
    rv_csr hvip;
    rv_csr htinst;
    rv_csr hgeip;
    rv_csr hcontext;
    rv_csr htimedelta;
//    rv_csr htimedeltah;
    rv_csr vsstatus;
    rv_csr vsie;
    rv_csr vstvec;
    rv_csr vsscratch;
    rv_csr vsepc;
    rv_csr vscause;
    rv_csr vstval;
    rv_csr vsip;
    rv_csr vsatp;

    /* Machine CSR addresses */
    rv_csr mvendorid;
    rv_csr marchid;
    rv_csr mimpid;
    rv_csr mhartid;
    rv_csr mconfigptr;
    rv_csr mstatus;
    rv_csr misa;
    rv_csr medelg;
    rv_csr mideleg;
    rv_csr mie;
    rv_csr mtvec;
    rv_csr mcounteren;
//    rv_csr mstatush;
    rv_csr mscratch;
    rv_csr mepc;
    rv_csr mcause;
    rv_csr mtval;
    rv_csr mip;
    rv_csr mtinst;
    rv_csr mtval2;
    rv_csr menvcfg;
//    rv_csr menvcfgh;
    rv_csr mseccfg;
//    rv_csr mseccfgh;
    rv_csr pmpcfg[16];
    rv_csr pmpaddr[64];
    rv_csr mcycle;
    rv_csr minstret;
    rv_csr mhpmcounter[32-3];
#if 0
    rv_csr mcycleh;
    rv_csr minstreth;
    rv_csr mhpmcounterh[32-3];
#endif
    rv_csr mcountinhibit;
    rv_csr mhpmevent[32-3];
    rv_csr tselect;
    rv_csr tdata1;
    rv_csr tdata2;
    rv_csr tdata3;
    rv_csr mcontext;
    rv_csr dcsr;
    rv_csr dpc;
    rv_csr dscratch0;
    rv_csr dscratch1;
} rv_csr_state;

#include "csr_addrs.h"

#endif
