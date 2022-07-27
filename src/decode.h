#ifndef DECODE_H
#define DECODE_H

#include "instructions.h"
#include "register.h"

rvi_register rv_i_type_immediate(rv_instruction inst);
rvi_register rv_s_type_immediate(rv_instruction inst);
rvi_register rv_b_type_immediate(rv_instruction inst);
rvi_register rv_u_type_immediate(rv_instruction inst);
rvi_register rv_j_type_immediate(rv_instruction inst);

rv_operation rv_decode_instruction(rv_instruction inst);
rvi_register rv_get_immediate(rv_decoded_instruction);

void rv_print_decoded_instruction(rv_decoded_instruction inst);

#endif
