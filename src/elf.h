#ifndef ELF_H
#define ELF_H

#include <elf.h>

typedef struct elf_memory {
    uint64_t vaddr;
    uint64_t size;
    uint8_t *data;
    uint64_t flags;
} elf_memory;

typedef struct elf_memory_map {
    elf_memory memory;
    struct elf_memory_map *next;
    struct elf_memory_map *prev;
} elf_memory_map;

typedef struct elf_program {
    const char *error;
    Elf64_Ehdr  elf_header;
    Elf64_Phdr *program_headers;
    Elf64_Shdr *section_headers;
    Elf64_Sym  *symbols;
    char       *str_tab;
    uint64_t    num_symbols;
    elf_memory_map *head;
    elf_memory_map *tail;
} elf_program;

typedef struct elf_symbol {
    int         found;
    Elf64_Sym   symbol;
} elf_symbol;

elf_program elf_load(const char *path);
void        elf_unload(elf_program prog);

elf_symbol  elf_find_symbol(elf_program prog, const char *name);


void elf_print_program(elf_program prog);

#endif
