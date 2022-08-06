#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "cpu.h"
#include "fetch.h"
#include "decode.h"
#include "execute.h"
#include "elf.h"

#define FAIL() exit(EXIT_FAILURE)\

static rv_cpu_state *running_state = NULL;

void
on_watchpoint(rv_cpu_state *state)
{
    printf("Watchpoint hit!\n");
    //rv_print_regs(state);
    rvi_register testnum = rvi_reg_read(state, 3);
    if (testnum == 1) {
        printf("Test passed!\n");
        exit(EXIT_SUCCESS);
    }
    else {
        printf("Test failed!\n");
        exit(EXIT_FAILURE);
    }
}

void
usage_error(int argc, char *argv[])
{
    (void) argc; (void) argv;
    printf("Usage: %s [file]\n", argv[0]);
    FAIL();
}

rv_cpu_program
read_file(const char *filename)
{
    rv_cpu_program result = {0};
    elf_program prog = elf_load(filename);
    if (prog.error) {
        printf("prog.error = %s\n", prog.error);
        return result;
    }
    //elf_print_program(prog);

    uint64_t tohost = 0;

    elf_symbol tohost_symbol = elf_find_symbol(prog, "tohost");
    tohost = tohost_symbol.symbol.st_value;
    if (!tohost_symbol.found) {
        printf("Unable to find \"tohost\" symbol\n");
    }
    else {
        printf("Found \"tohost\" with value %016lX\n", tohost);
    }

    uint64_t prog_start = 0xFFFFFFFFFFFFFFFF;
    uint64_t prog_end = 0;

    elf_memory_map *curr = prog.head;
    if (curr) while (1) {
        if (curr->memory.vaddr < prog_start) {
            prog_start = curr->memory.vaddr;
        }
        if (curr->memory.vaddr + curr->memory.size > prog_end) {
            prog_end = curr->memory.vaddr + curr->memory.size;
        }

        if (curr == prog.tail) break;

        curr = curr->next;
    }

    uint64_t buffer_length = (prog_end - prog_start) + 1; 
    uint8_t *buffer = calloc(buffer_length, 1);
    curr = prog.head;
    if (buffer != NULL) {
        if (curr) while (1) {
            uint64_t buffer_offset = curr->memory.vaddr - prog_start;
            memcpy(buffer + buffer_offset,
                    curr->memory.data, curr->memory.size);
            if (curr == prog.tail) break;
            curr = curr->next;
        }

        result.vaddr_offset = prog_start;
        result.length = buffer_length;
        result.bytes = buffer;
        result.entry_address = prog.elf_header.e_entry;
        result.tohost = tohost;
    }
    elf_unload(prog);
    return result;
}

void run(rv_cpu_program program)
{
    static rv_cpu_state state;
    running_state = &state;

    rv_decoded_instruction curr_inst;
    rv_load_simple_program(&state, program);
    state.watchpoint = (rvi_register)program.tohost;
    state.action = on_watchpoint;

    rv_program_free(program);

    while (1) {
        //rv_print_pc(&state);
        curr_inst.format = rv_fetch_instruction(&state);
        curr_inst.op = rv_decode_instruction(curr_inst.format);
        //rv_print_decoded_instruction(curr_inst);
        rv_execute_instruction(curr_inst, &state);
        rv_pc_increment(&state, curr_inst.op);
    }
}

int
main(int argc, char* argv[])
{
    rv_cpu_program program;

    if (argc != 2) {
        usage_error(argc, argv);
    }
    
    program = read_file(argv[1]);

    run(program);

    return 0;
}
