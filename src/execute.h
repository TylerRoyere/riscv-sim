#ifndef EXECUTE_H
#define EXECUTE_H

#include "instructions.h"
#include "cpu.h"

void rv_execute_instruction(rv_decoded_instruction inst, rv_cpu_state *state);

#endif
