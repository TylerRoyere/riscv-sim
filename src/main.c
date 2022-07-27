#include <stdio.h>
#include <stdlib.h>

#include "cpu.h"
#include "fetch.h"
#include "decode.h"
#include "execute.h"

#define FAIL() exit(EXIT_FAILURE)\

static rv_cpu_state *running_state = NULL;

void print_state(void)
{
    if (running_state != NULL) {
        rv_print_regs(running_state);
        //rv_print_mem();
    }
}


void
usage_error(int argc, char *argv[])
{
    (void) argc; (void) argv;
    printf("Usage: %s [file]\n", argv[0]);
    FAIL();
}

char *
read_file(const char *filename, unsigned long *length)
{
    /* TODO: use variable length buffer */
    char *filebuf = NULL;
    unsigned long bytes_read = 0;
    FILE *infile = NULL;
    const unsigned long filebuf_size = 128 * 1024;

    filebuf = malloc(filebuf_size);
    infile = fopen(filename, "r");

    if (filebuf != NULL && infile != NULL) {
        bytes_read = fread(filebuf, sizeof(filebuf[0]), filebuf_size, infile);
        if (bytes_read != filebuf_size) {
            fclose(infile);
            filebuf[bytes_read] = '\0';
            *length = bytes_read;
            return filebuf;
        }
    }

    if (infile != NULL) fclose(infile);
    free(filebuf);
    *length = 0;
    return NULL;
}

void run(const char *program, unsigned long length)
{
    static rv_cpu_state state;
    running_state = &state;
    atexit(print_state);

    rv_decoded_instruction curr_inst;
    rv_load_simple_program(&state, (const uint8_t*) program, length);
    state.rvi_pc = 0x1000;

    while (1) {
        curr_inst.format = rv_fetch_instruction(&state);
        curr_inst.op = rv_decode_instruction(curr_inst.format);
        rv_print_decoded_instruction(curr_inst);
        rv_execute_instruction(curr_inst, &state);
        rv_pc_increment(&state, curr_inst.op);
    }
}

int
main(int argc, char* argv[])
{
    char *filestr = NULL;
    unsigned long length;

    if (argc != 2) {
        usage_error(argc, argv);
    }
    
    filestr = read_file(argv[1], &length);
    if (filestr == NULL) {
        FAIL();
    }

    run(filestr, length);

    return 0;
}
