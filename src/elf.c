#include <stdio.h>
#include <string.h>
#include "elf.h"
#include "utils.h"

#define BSWAP16(x)  \
    ((uint16_t)                         \
    ( ((((uint16_t)x) & 0x00FF) << 8) | \
      ((((uint16_t)x) & 0xFF00) >> 8)  ))

#define BSWAP32(x)  \
    ((uint32_t)                               \
    ( ((((uint32_t)x) & 0x000000FF) << 24) |  \
      ((((uint32_t)x) & 0x0000FF00) << 8)  |  \
      ((((uint32_t)x) & 0x00FF0000) >> 8)  |  \
      ((((uint32_t)x) & 0xFF000000) >> 24)   ))

#define BSWAP64(x)  \
    ((uint64_t)                                       \
    ( ((((uint64_t)x) & 0x00000000000000FF) << 56) |  \
      ((((uint64_t)x) & 0x000000000000FF00) << 40) |  \
      ((((uint64_t)x) & 0x0000000000FF0000) << 24) |  \
      ((((uint64_t)x) & 0x00000000FF000000) << 8 ) |  \
      ((((uint64_t)x) & 0x000000FF00000000) >> 8 ) |  \
      ((((uint64_t)x) & 0x0000FF0000000000) >> 24) |  \
      ((((uint64_t)x) & 0x00FF000000000000) >> 40) |  \
      ((((uint64_t)x) & 0xFF00000000000000) >> 56)   ))

#define machine_bigendian ((*((char*)(int [1]){1}) == 0))
#define ENDIANNESS_DIFFERS(e_ident_array)   \
    ((e_ident_array[EI_DATA] == ELFDATA2MSB) != machine_bigendian)

static elf_program read_elf_header(FILE *infile);
static elf_program read_program_headers(FILE *infile, elf_program prog);
static elf_program read_section_headers(FILE *infile, elf_program prog);

static elf_program read_elf_header_machine(FILE *infile,
        const unsigned char[EI_NIDENT]);

static const char *read_one_program_header(FILE *infile,
        elf_program prog, Elf64_Phdr *out);

static const char *read_one_section_header(FILE *infile,
        elf_program prog, Elf64_Shdr *out);

static elf_program load_entire_program(FILE *infile, elf_program prog);
static elf_program load_program_segments(FILE *infile, elf_program prog);
static elf_program load_section_segments(FILE *infile, elf_program prog);
static elf_program load_symbol_table(FILE *infile,
        elf_program prog, size_t tab_index);
static elf_program load_string_table(FILE *infile, elf_program prog,
        size_t tab_index);
static elf_program load_single_program_segment(FILE *infile,
        elf_program prog, size_t phdr_index);
static elf_program load_single_section_segment(FILE *infile,
        elf_program prog, size_t shdr_index);
static elf_program load_nobits_section_segment(elf_program prog,
        size_t shdr_index);
static elf_memory read_memory_segment(FILE *infile,
        elf_memory segment_desc, size_t size_in_file);

static elf_program memory_map_add_segment(elf_program prog,
        elf_memory segment);

static Elf32_Ehdr convert_header32_endian(Elf32_Ehdr hdr);
static Elf64_Ehdr convert_header64_endian(Elf64_Ehdr hdr);
static Elf64_Ehdr convert_header32to64(Elf32_Ehdr hdr);

static Elf32_Phdr convert_pheader32_endian(Elf32_Phdr hdr);
static Elf64_Phdr convert_pheader64_endian(Elf64_Phdr hdr);
static Elf64_Phdr convert_pheader32to64(Elf32_Phdr hdr);

static Elf32_Shdr convert_sheader32_endian(Elf32_Shdr hdr);
static Elf64_Shdr convert_sheader64_endian(Elf64_Shdr hdr);
static Elf64_Shdr convert_sheader32to64(Elf32_Shdr hdr);

static Elf32_Sym convert_symbol32_endian(Elf32_Sym sym);
static Elf64_Sym convert_symbol64_endian(Elf64_Sym sym);
static Elf64_Sym convert_symbol32to64(Elf32_Sym sym);

static void print_elf_header(Elf64_Ehdr hdr);
static void print_program_header(Elf64_Phdr hdr);
static void print_section_header(Elf64_Shdr hdr);

static const char *check_ident(unsigned char e_ident[EI_NIDENT]);
static const char *check_elf_header(Elf64_Ehdr hdr);

elf_program
elf_load(const char *path)
{
    elf_program prog = {0};
    FILE *prog_file = fopen(path, "r");

    if (prog_file == NULL) {
        prog.error = "Failed to open file";
        return prog;
    }

    prog = read_elf_header(prog_file);
    prog = read_program_headers(prog_file, prog);
    prog = read_section_headers(prog_file, prog);
    prog = load_entire_program(prog_file, prog);

    if (fclose(prog_file)) {
        prog.error = "Failed to close file";
        return prog;
    }

    return prog;
}

void
elf_unload(elf_program prog)
{
    free(prog.program_headers);
    free(prog.section_headers);
    free(prog.symbols);
    free(prog.str_tab);

    elf_memory_map *curr = prog.head;
    elf_memory_map *next = NULL;
    if (curr) while (1) {
        next = curr->next;
        
        free(curr->memory.data);
        free(curr);

        if (curr == prog.tail) break;
        curr = next;
    }
}

elf_symbol
elf_find_symbol(elf_program prog, const char *name)
{
    elf_symbol result = {0};
    if (prog.error) {
        return result;
    }

    for (size_t ii = 0; ii < prog.num_symbols; ii++) {
        size_t offset = prog.symbols[ii].st_name;
        const char *sym_name = &prog.str_tab[offset];
        if (strcmp(sym_name, name) == 0) {
            result.symbol = prog.symbols[ii];
            result.found = 1;
            return result;
        }
    }

    return result;
}

static elf_program
read_elf_header(FILE *infile)
{
    elf_program prog;
    unsigned char e_ident[EI_NIDENT];
    size_t nread = 0;

    nread = fread(e_ident, sizeof(e_ident[0]), EI_NIDENT, infile);
    if (nread == 0) {
        prog.error = "Failed to read header.e_ident[]";
        return prog;
    }

    prog.error = check_ident(e_ident);
    if (prog.error) {
        return prog;
    }

    TODO("Ignoring EI_OSABI and EI_ABIVERSION for now!\n");


    prog = read_elf_header_machine(infile, e_ident);
    prog.error = check_elf_header(prog.elf_header);
    return prog;
}

static elf_program
read_program_headers(FILE *infile, elf_program prog)
{
    if (prog.error) {
        return prog;
    }

    if (prog.elf_header.e_phoff > LONG_MAX) {
        prog.error = "Program header offset is too large";
        return prog;
    }
    
    long prog_hdr_off = (long)prog.elf_header.e_phoff;

    if (fseek(infile, prog_hdr_off, SEEK_SET)) {
        prog.error = "Unable move file position to program header";
        return prog;
    }

    Elf64_Phdr *phdrs = malloc(sizeof(Elf64_Phdr) * prog.elf_header.e_phnum);

    if (phdrs == NULL) {
        prog.error = "Failed to allocate storage for program headers";
        return prog;
    }

    for (uint64_t ii = 0; ii < prog.elf_header.e_phnum; ii++) {
        prog.error = read_one_program_header(infile, prog, &phdrs[ii]);
        if (prog.error) {
            free(phdrs);
            phdrs = NULL;
            return prog;
        }
    }

    prog.program_headers = phdrs;

    return prog;
}

static elf_program
read_section_headers(FILE *infile, elf_program prog)
{
    if (prog.error) {
        return prog;
    }

    if (prog.elf_header.e_shoff > LONG_MAX) {
        prog.error = "Section header offset is too large";
        return prog;
    }

    long sec_hdr_off = (long)prog.elf_header.e_shoff;

    if (fseek(infile, sec_hdr_off, SEEK_SET)) {
        prog.error = "Unable to move file position to section header";
        return prog;
    }

    Elf64_Shdr *shdrs = malloc(sizeof(Elf64_Shdr) * prog.elf_header.e_shnum);

    if (shdrs == NULL) {
        prog.error = "Failed to allocate storage for section headers";
        return prog;
    }

    for (uint64_t ii = 0; ii < prog.elf_header.e_shnum; ii++) {
        if (ii != SHN_UNDEF && (ii < SHN_LORESERVE || ii > SHN_HIRESERVE)) {
            prog.error = read_one_section_header(infile, prog, &shdrs[ii]);
            if (prog.error) {
                free(shdrs);
                shdrs = NULL;
                return prog;
            }
        }
        else {
            memset(&shdrs[ii], 0, sizeof(shdrs[ii]));
        }
    }

    prog.section_headers = shdrs;

    return prog;
}

static elf_program
load_entire_program(FILE *infile, elf_program prog)
{
    if (prog.error) {
        return prog;
    }

    prog.head = NULL;
    prog.tail = NULL;

    prog = load_program_segments(infile, prog);
    TODO("Ignoring elf sections for now\n");
    prog = load_section_segments(infile, prog);

    return prog;
}

static elf_program
load_program_segments(FILE *infile, elf_program prog)
{
    if (prog.error) {
        return prog;
    }

    for (size_t ii = 0; ii < prog.elf_header.e_phnum; ii++) {
        const Elf64_Phdr phdr = prog.program_headers[ii];
        switch (phdr.p_type) {
            case PT_NULL:
                /* Do nothing */
                break;
            case 0x70000003: /* PT_RISCV_ATTRIBUTES */
                TODO("Handle special case for *_RISCV_ATTRIBUTE sections\n");
                break;
            case PT_LOAD:
                prog = load_single_program_segment(infile, prog, ii);
                break;
            default:
                prog.error = "Cannot load program header of this type";
                return prog;
        }
    }
    return prog;
}

static elf_program
load_section_segments(FILE *infile, elf_program prog)
{
    if (prog.error) {
        return prog;
    }

    for (size_t ii = 0; ii < prog.elf_header.e_shnum; ii++) {
        uint32_t type = prog.section_headers[ii].sh_type;
        switch (type) {
            case SHT_NULL:
                /* Do nothing */
                break;
            case SHT_PROGBITS:
                TODO("Handle SHT_NOBITS\n");
                //load_single_section_segment(infile, prog, ii);
                break;
            case 0x70000003: /* SHT_RISCV_ATTRIBUTES */
                TODO("Handle special case for *_RISCV_ATTRIBUTE sections\n");
                break;
            case SHT_NOBITS:
                TODO("Handle SHT_NOBITS\n");
                //(void)load_nobits_section_segment(prog, ii);
                break;
            case SHT_SYMTAB:
                prog = load_symbol_table(infile, prog, ii);
                break;
            case SHT_STRTAB:
                prog = load_string_table(infile, prog, ii);
                break;
            default:
                prog.error = "Cannot load section header of this type";
                return prog;
        }
    }
    return prog;
}

static elf_program
load_symbol_table(FILE *infile, elf_program prog, size_t tab_index)
{
    if (prog.error) {
        return prog;
    }

    Elf64_Shdr symtab = prog.section_headers[tab_index];
    if (symtab.sh_offset > LONG_MAX) {
        prog.error = "Symbol table offset is too large";
        return prog;
    }

    if (fseek(infile, (long)symtab.sh_offset, SEEK_SET)) {
        prog.error = "Unable to move file position to symbol table";
        return prog;
    }

    size_t num_symbols = symtab.sh_size / symtab.sh_entsize;
    int do_bswap = ENDIANNESS_DIFFERS(prog.elf_header.e_ident);
    Elf64_Sym *table = malloc(num_symbols * sizeof(*table));

    if (table == NULL) {
        prog.error = "Failed to allocate symbol table";
        return prog;
    }

    if (prog.elf_header.e_ident[EI_CLASS] == ELFCLASS32) {
        Elf32_Sym sym;
        for (size_t ii = 0; ii < num_symbols; ii++) {

            if (fread(&sym, 1, sizeof(sym), infile) != sizeof(sym)) {
                prog.error = "Failed to read Elf32_Sym from file";
                free(table);
                return prog;
            }

            if (do_bswap) {
                sym = convert_symbol32_endian(sym);
            }

            table[ii] = convert_symbol32to64(sym);
        }
    }
    else if (prog.elf_header.e_ident[EI_CLASS] == ELFCLASS64) {
        Elf64_Sym sym;
        for (size_t ii = 0; ii < num_symbols; ii++) {

            if (fread(&sym, 1, sizeof(sym), infile) != sizeof(sym)) {
                prog.error = "Failed to read Elf64_Sym from file";
                free(table);
                return prog;
            }

            if (do_bswap) {
                sym = convert_symbol64_endian(sym);
            }

            table[ii] = sym;
        }
    }
    else {
        free(table);
        prog.error = "Unknown elfclass";
        return prog;
    }
    prog.symbols = table;
    prog.num_symbols = num_symbols;
    return prog;
}

static elf_program
load_string_table(FILE *infile, elf_program prog, size_t index)
{
    uint64_t file_position = prog.section_headers[index].sh_offset;
    if (file_position> LONG_MAX) {
        prog.error = "Position of string table too large";
        return prog;
    }

    if (fseek(infile, (long)file_position, SEEK_SET)) {
        prog.error = "Failed to move file position to string table";
        return prog;
    }

    uint64_t length = prog.section_headers[index].sh_size;
    char *strtab = malloc(length);
    if (strtab == NULL) {
        prog.error = "Failed to allocated memory for string table";
        return prog;
    }

    if (fread(strtab, 1, length, infile) != length) {
        prog.error = "Failed to read string table from file";
        free(strtab);
        return prog;
    }

    prog.str_tab = strtab;
    return prog;
}

static elf_program
load_single_program_segment(FILE *infile, elf_program prog, size_t phdr_index)
{
    if (prog.error) {
        return prog;
    }

    if (prog.program_headers[phdr_index].p_offset > LONG_MAX) {
        prog.error = "Program segment file location too large";
        return prog;
    }

    long file_offset = (long)prog.program_headers[phdr_index].p_offset;

    if (fseek(infile, file_offset, SEEK_SET)) {
        prog.error = "Failed to move file position to program segment";
        return prog;
    }

    uint64_t size_in_mem = prog.program_headers[phdr_index].p_memsz;
    uint64_t size_in_file = prog.program_headers[phdr_index].p_filesz;
    uint64_t vaddr = prog.program_headers[phdr_index].p_vaddr;
    uint64_t flags = prog.program_headers[phdr_index].p_flags;

    elf_memory segment;
    segment.size = size_in_mem;
    segment.vaddr = vaddr;
    segment.flags = flags;
    segment.data = NULL;

    segment = read_memory_segment(infile, segment, size_in_file);

    if (segment.data == NULL) {
        prog.error = "Failed to read memory segment";
    }
    
    prog = memory_map_add_segment(prog, segment);
    if (prog.error) {
        free(segment.data);
    }
    return prog;
}

UNUSED static elf_program
load_single_section_segment(FILE *infile, elf_program prog, size_t shdr_index)
{
    if (prog.error) {
        return prog;
    }

    if (prog.section_headers[shdr_index].sh_offset > LONG_MAX) {
        prog.error = "Section segment file location too large";
        return prog; 
    }

    long file_offset = (long)prog.section_headers[shdr_index].sh_offset;

    if (fseek(infile, file_offset, SEEK_SET)) {
        prog.error = "Failed to move file position to section segment";
        return prog;
    }

    elf_memory segment = {0};
    segment.size = prog.section_headers[shdr_index].sh_size;
    segment.vaddr = prog.section_headers[shdr_index].sh_addr;
    segment.flags = prog.section_headers[shdr_index].sh_flags;
    segment.data = NULL;

    size_t size_in_file = 0;

    segment = read_memory_segment(infile, segment, size_in_file);

    if (segment.data == NULL) {
        prog.error = "Failed to read section segment";
        return prog;
    }

    prog = memory_map_add_segment(prog, segment);
    if (prog.error) {
        free(segment.data);
    }
    return prog;
}

UNUSED static elf_program
load_nobits_section_segment(elf_program prog, size_t shdr_index)
{
    if (prog.error) {
        return prog;
    }

    elf_memory segment;

    segment.size = prog.section_headers[shdr_index].sh_size;
    segment.vaddr = prog.section_headers[shdr_index].sh_addr;
    segment.flags = prog.section_headers[shdr_index].sh_flags;
    segment.data = calloc(segment.size, 1);

    if (segment.data == NULL) {
        prog.error = "Unable to allocate nobits section segment";
        return prog;
    }

    prog = memory_map_add_segment(prog, segment);
    if (prog.error) {
        free(segment.data);
    }

    return prog;
}

static elf_memory
read_memory_segment(FILE *infile,
        elf_memory segment, size_t size_in_file)
{
    segment.data = calloc(segment.size, 1);

    if (segment.data == NULL) {
        return segment;
    }

    if (size_in_file > segment.size) {
        printf("Size in file (%ld) greater than segment size (%ld) \n",
                size_in_file, segment.size);
        free(segment.data);
        segment.data = NULL;
        return segment;
    }

    if (segment.data == NULL) {
        printf("Segment data failed to allocate\n");
        free(segment.data);
        segment.data = NULL;
        return segment;
    }

    if (fread(segment.data, 1, size_in_file, infile) != size_in_file) {
        printf("Segment data could not be read from file\n");
        free(segment.data);
        segment.data = NULL;
        return segment;
    }

    return segment;
}

static elf_program
memory_map_add_segment(elf_program prog, elf_memory segment)
{
    if (prog.error) {
        return prog;
    }

    elf_memory_map *new_map = malloc(sizeof(*new_map));
    if (new_map == NULL) {
        prog.error = "Failed to allocate new memory map structure";
        return prog;
    }

    if (prog.head == NULL) {
        prog.head = new_map;
    }
    if (prog.tail == NULL) {
        prog.tail = new_map;
    }

    *new_map = (elf_memory_map) {
        .memory = segment,
        .next = prog.head,
        .prev = prog.tail,
    };

    new_map->prev->next = new_map;
    new_map->next->prev = new_map;
    prog.tail = new_map;

    return prog;
}

static const char *
check_ident(unsigned char e_ident[EI_NIDENT])
{
    if (e_ident[EI_MAG0] != ELFMAG0 ||
            e_ident[EI_MAG1] != ELFMAG1 ||
            e_ident[EI_MAG2] != ELFMAG2 ||
            e_ident[EI_MAG3] != ELFMAG3) {
        return "Program does not contain elf magic";
    }

    if (e_ident[EI_CLASS] != ELFCLASS32 &&
            e_ident[EI_CLASS] != ELFCLASS64) {
        return "Invalid ELFCLASS";
    }

    if (e_ident[EI_DATA] != ELFDATA2LSB &&
            e_ident[EI_DATA] != ELFDATA2MSB) {
        return "Invalid data format";
    }

    if (e_ident[EI_VERSION] != EV_CURRENT) {
        return "Invalid version";
    }

    return NULL;
}


static const char *
check_elf_header(Elf64_Ehdr hdr)
{
    if (hdr.e_type != ET_EXEC) {
        return "Invalid elf type, we only understand RT_EXEC for now";
    }

    if (hdr.e_machine != EM_RISCV) {
        return "Invalid machine type, RISC-V uses EM_RISCV";
    }

    return NULL;
}

static elf_program
read_elf_header_machine(FILE *infile, const unsigned char e_ident[EI_NIDENT])
{
    elf_program prog = {0};

    if (fseek(infile, 0, SEEK_SET)) {
        prog.error = "Unable to move file position";
        return prog;
    }
        
    if (e_ident[EI_CLASS] == ELFCLASS32) {
        Elf32_Ehdr temp_hdr;
        if (fread(&temp_hdr, 1, sizeof(temp_hdr), infile) != sizeof(temp_hdr)) {
            prog.error = "Failed to read Elf32_Ehdr header";
            return prog;
        }

        if (ENDIANNESS_DIFFERS(e_ident)) {
            temp_hdr = convert_header32_endian(temp_hdr);
        }
        
        prog.elf_header = convert_header32to64(temp_hdr);
    }
    else {
        Elf64_Ehdr temp_hdr;
        if (fread(&temp_hdr, 1, sizeof(temp_hdr), infile) != sizeof(temp_hdr)) {
            prog.error = "Failed to read Elf64_Ehdr header";
            return prog;
        }

        if (ENDIANNESS_DIFFERS(e_ident)) {
            temp_hdr = convert_header64_endian(temp_hdr);
        }

        prog.elf_header = temp_hdr;
    }

    return prog;
}

static const char *
read_one_program_header(FILE *infile, elf_program prog, Elf64_Phdr *out)
{
    if (prog.error) {
        return prog.error;
    }

    if (prog.elf_header.e_phentsize == sizeof(Elf32_Phdr)) {
        Elf32_Phdr temp_hdr;

        if (fread(&temp_hdr, 1, sizeof(temp_hdr), infile) != sizeof(temp_hdr)) {
            return "Failed to read program header Elf32_Phdr";
        }

        if (ENDIANNESS_DIFFERS(prog.elf_header.e_ident)) {
            temp_hdr = convert_pheader32_endian(temp_hdr);
        }
        
        *out = convert_pheader32to64(temp_hdr);
    }
    else if (prog.elf_header.e_phentsize == sizeof(Elf64_Phdr)) {
        Elf64_Phdr temp_hdr;

        if (fread(&temp_hdr, 1, sizeof(temp_hdr), infile) != sizeof(temp_hdr)) {
            return "Failed to read program header Elf64_Phdr";
        }

        if (ENDIANNESS_DIFFERS(prog.elf_header.e_ident)) {
            temp_hdr = convert_pheader64_endian(temp_hdr);
        }

        *out = temp_hdr;
    }
    else {
        return "Unknown program header size specified in elf header";
    }
    return NULL;
}

static const char *
read_one_section_header(FILE *infile, elf_program prog, Elf64_Shdr *out)
{
    if (prog.error) {
        return prog.error;
    }

    if (prog.elf_header.e_shentsize == sizeof(Elf32_Shdr)) {
        Elf32_Shdr temp_hdr;

        if (fread(&temp_hdr, 1, sizeof(temp_hdr), infile) != sizeof(temp_hdr)) {
            return "Failed to read program header Elf32_Shdr";
        }

        if (ENDIANNESS_DIFFERS(prog.elf_header.e_ident)) {
            temp_hdr = convert_sheader32_endian(temp_hdr);
        }

        *out = convert_sheader32to64(temp_hdr);
    }
    else if (prog.elf_header.e_shentsize == sizeof(Elf64_Shdr)) {
        Elf64_Shdr temp_hdr;

        if (fread(&temp_hdr, 1, sizeof(temp_hdr), infile) != sizeof(temp_hdr)) {
            return "Failed to read program header Elf64_Shdr";
        }

        if (ENDIANNESS_DIFFERS(prog.elf_header.e_ident)) {
            temp_hdr = convert_sheader64_endian(temp_hdr);
        }

        *out = temp_hdr;
    }
    else {
        return "Unknown program header size specified in elf header";
    }
    return NULL;
}

static Elf32_Ehdr
convert_header32_endian(Elf32_Ehdr hdr)
{
    hdr.e_type      = BSWAP16(hdr.e_type);
    hdr.e_machine   = BSWAP16(hdr.e_type);
    hdr.e_version   = BSWAP32(hdr.e_version);
    hdr.e_entry     = BSWAP32(hdr.e_entry);
    hdr.e_phoff     = BSWAP32(hdr.e_phoff);
    hdr.e_shoff     = BSWAP32(hdr.e_shoff);
    hdr.e_flags     = BSWAP32(hdr.e_flags);
    hdr.e_ehsize    = BSWAP16(hdr.e_ehsize);
    hdr.e_phentsize = BSWAP16(hdr.e_phentsize);
    hdr.e_phnum     = BSWAP16(hdr.e_phnum);
    hdr.e_shentsize = BSWAP16(hdr.e_shentsize);
    hdr.e_shnum     = BSWAP16(hdr.e_shnum);
    hdr.e_shstrndx  = BSWAP16(hdr.e_shstrndx);

    return hdr;
}

static Elf32_Phdr
convert_pheader32_endian(Elf32_Phdr hdr)
{
    hdr.p_type      = BSWAP32(hdr.p_type);
    hdr.p_offset    = BSWAP32(hdr.p_offset);
    hdr.p_vaddr     = BSWAP32(hdr.p_vaddr);
    hdr.p_paddr     = BSWAP32(hdr.p_paddr);
    hdr.p_filesz    = BSWAP32(hdr.p_filesz);
    hdr.p_memsz     = BSWAP32(hdr.p_memsz);
    hdr.p_flags     = BSWAP32(hdr.p_flags);
    hdr.p_align     = BSWAP32(hdr.p_align);

    return hdr;
}

static Elf32_Shdr
convert_sheader32_endian(Elf32_Shdr hdr)
{
    hdr.sh_name	        = BSWAP32(hdr.sh_name);
    hdr.sh_type	        = BSWAP32(hdr.sh_type);
    hdr.sh_flags	    = BSWAP32(hdr.sh_flags);
    hdr.sh_addr	        = BSWAP32(hdr.sh_addr);
    hdr.sh_offset	    = BSWAP32(hdr.sh_offset);
    hdr.sh_size	        = BSWAP32(hdr.sh_size);
    hdr.sh_link	        = BSWAP32(hdr.sh_link);
    hdr.sh_info	        = BSWAP32(hdr.sh_info);
    hdr.sh_addralign	= BSWAP32(hdr.sh_addralign);
    hdr.sh_entsize	    = BSWAP32(hdr.sh_entsize);

    return hdr;
}

static Elf32_Sym
convert_symbol32_endian(Elf32_Sym sym)
{
    sym.st_name     = BSWAP32(sym.st_name);
    sym.st_value    = BSWAP32(sym.st_value);
    sym.st_size     = BSWAP32(sym.st_size);
    sym.st_info     = sym.st_info;
    sym.st_other    = sym.st_other;
    sym.st_shndx    = BSWAP16(sym.st_shndx);

    return sym;
}

static Elf64_Ehdr
convert_header64_endian(Elf64_Ehdr hdr)
{
    hdr.e_type      = BSWAP16(hdr.e_type);
    hdr.e_machine   = BSWAP16(hdr.e_machine);
    hdr.e_version   = BSWAP32(hdr.e_version);
    hdr.e_entry     = BSWAP64(hdr.e_entry);
    hdr.e_phoff     = BSWAP64(hdr.e_phoff);
    hdr.e_shoff     = BSWAP64(hdr.e_shoff);
    hdr.e_flags     = BSWAP32(hdr.e_flags);
    hdr.e_ehsize    = BSWAP16(hdr.e_ehsize);
    hdr.e_phentsize = BSWAP16(hdr.e_phentsize);
    hdr.e_phnum     = BSWAP16(hdr.e_phnum);
    hdr.e_shentsize = BSWAP16(hdr.e_shentsize);
    hdr.e_shnum     = BSWAP16(hdr.e_shnum);
    hdr.e_shstrndx  = BSWAP16(hdr.e_shstrndx);

    return hdr;
}

static Elf64_Phdr
convert_pheader64_endian(Elf64_Phdr hdr)
{
    hdr.p_type      = BSWAP32(hdr.p_type);
    hdr.p_flags     = BSWAP32(hdr.p_flags);
    hdr.p_offset    = BSWAP64(hdr.p_offset);
    hdr.p_vaddr     = BSWAP64(hdr.p_vaddr);
    hdr.p_paddr     = BSWAP64(hdr.p_paddr);
    hdr.p_filesz    = BSWAP64(hdr.p_filesz);
    hdr.p_memsz     = BSWAP64(hdr.p_memsz);
    hdr.p_align     = BSWAP64(hdr.p_align);

    return hdr;
}

static Elf64_Shdr
convert_sheader64_endian(Elf64_Shdr hdr)
{
    hdr.sh_name         = BSWAP32(hdr.sh_name);
    hdr.sh_type         = BSWAP32(hdr.sh_type);
    hdr.sh_flags        = BSWAP64(hdr.sh_flags);
    hdr.sh_addr         = BSWAP64(hdr.sh_addr);
    hdr.sh_offset       = BSWAP64(hdr.sh_offset);
    hdr.sh_size         = BSWAP64(hdr.sh_size);
    hdr.sh_link         = BSWAP32(hdr.sh_link);
    hdr.sh_info         = BSWAP32(hdr.sh_info);
    hdr.sh_addralign    = BSWAP64(hdr.sh_addralign);
    hdr.sh_entsize      = BSWAP64(hdr.sh_entsize);

    return hdr;
}

static
Elf64_Sym convert_symbol64_endian(Elf64_Sym sym)
{
    sym.st_name     = BSWAP32(sym.st_name);
    sym.st_info     = sym.st_info;
    sym.st_other    = sym.st_other;
    sym.st_shndx    = BSWAP16(sym.st_shndx);
    sym.st_value    = BSWAP64(sym.st_value);
    sym.st_size     = BSWAP64(sym.st_size);

    return sym;
}

static Elf64_Ehdr
convert_header32to64(Elf32_Ehdr hdr)
{
    Elf64_Ehdr temp_hdr;
    memcpy(temp_hdr.e_ident, hdr.e_ident, sizeof(temp_hdr.e_ident));

    temp_hdr.e_type         = hdr.e_type;
    temp_hdr.e_machine      = hdr.e_machine;
    temp_hdr.e_version      = hdr.e_version;
    temp_hdr.e_entry        = hdr.e_entry;
    temp_hdr.e_phoff        = hdr.e_phoff;
    temp_hdr.e_shoff        = hdr.e_shoff;
    temp_hdr.e_flags        = hdr.e_flags;
    temp_hdr.e_ehsize       = hdr.e_ehsize;
    temp_hdr.e_phentsize    = hdr.e_phentsize;
    temp_hdr.e_phnum        = hdr.e_phnum;
    temp_hdr.e_shentsize    = hdr.e_shentsize;
    temp_hdr.e_shnum        = hdr.e_shnum;
    temp_hdr.e_shstrndx     = hdr.e_shstrndx;

    return temp_hdr;
}

static Elf64_Phdr
convert_pheader32to64(Elf32_Phdr hdr)
{
    Elf64_Phdr temp_hdr;

    temp_hdr.p_type      = hdr.p_type;
    temp_hdr.p_flags     = hdr.p_flags;
    temp_hdr.p_offset    = hdr.p_offset;
    temp_hdr.p_vaddr     = hdr.p_vaddr;
    temp_hdr.p_paddr     = hdr.p_paddr;
    temp_hdr.p_filesz    = hdr.p_filesz;
    temp_hdr.p_memsz     = hdr.p_memsz;
    temp_hdr.p_align     = hdr.p_align;

    return temp_hdr;
}

static Elf64_Shdr
convert_sheader32to64(Elf32_Shdr hdr)
{
    Elf64_Shdr temp_hdr;

    temp_hdr.sh_name        = hdr.sh_name;
    temp_hdr.sh_type        = hdr.sh_type;
    temp_hdr.sh_flags       = hdr.sh_flags;
    temp_hdr.sh_addr        = hdr.sh_addr;
    temp_hdr.sh_offset      = hdr.sh_offset;
    temp_hdr.sh_size        = hdr.sh_size;
    temp_hdr.sh_link        = hdr.sh_link;
    temp_hdr.sh_info        = hdr.sh_info;
    temp_hdr.sh_addralign   = hdr.sh_addralign;
    temp_hdr.sh_entsize     = hdr.sh_entsize;

    return temp_hdr;
}

static Elf64_Sym
convert_symbol32to64(Elf32_Sym sym)
{
    Elf64_Sym temp_sym;

    temp_sym.st_name     = sym.st_name;
    temp_sym.st_info     = sym.st_info;
    temp_sym.st_other    = sym.st_other;
    temp_sym.st_shndx    = sym.st_shndx;
    temp_sym.st_value    = sym.st_value;
    temp_sym.st_size     = sym.st_size;

    return temp_sym;
}

static void
print_elf_header(Elf64_Ehdr hdr)
{
    printf("hdr.e_type      = %d\n", hdr.e_type);
    printf("hdr.e_machine   = %d\n", hdr.e_machine);
    printf("hdr.e_version   = %d\n", hdr.e_version);
    printf("hdr.e_entry     = %ld\n", hdr.e_entry);
    printf("hdr.e_phoff     = %ld\n", hdr.e_phoff);
    printf("hdr.e_shoff     = %ld\n", hdr.e_shoff);
    printf("hdr.e_flags     = %u\n", hdr.e_flags);
    printf("hdr.e_ehsize    = %u\n", hdr.e_ehsize);
    printf("hdr.e_phentsize = %u\n", hdr.e_phentsize);
    printf("hdr.e_phnum     = %u\n", hdr.e_phnum);
    printf("hdr.e_shentsize = %u\n", hdr.e_shentsize);
    printf("hdr.e_shnum     = %u\n", hdr.e_shnum);
    printf("hdr.e_shstrndx  = %u\n", hdr.e_shstrndx);
}

static void
print_program_header(Elf64_Phdr hdr)
{
    printf("hdr.p_type = %u\n", hdr.p_type);
    printf("hdr.p_flags = %u\n", hdr.p_flags);
    printf("hdr.p_offset = %lu\n", hdr.p_offset);
    printf("hdr.p_vaddr = %lu\n", hdr.p_vaddr);
    printf("hdr.p_paddr = %lu\n", hdr.p_paddr);
    printf("hdr.p_filesz = %lu\n", hdr.p_filesz);
    printf("hdr.p_memsz = %lu\n", hdr.p_memsz);
    printf("hdr.p_align = %lu\n", hdr.p_align);
}

static void
print_section_header(Elf64_Shdr hdr)
{
    printf("hdr.sh_name = %u\n", hdr.sh_name);
    printf("hdr.sh_type = %u\n", hdr.sh_type);
    printf("hdr.sh_flags = %lu\n", hdr.sh_flags);
    printf("hdr.sh_addr = %lu\n", hdr.sh_addr);
    printf("hdr.sh_offset = %lu\n", hdr.sh_offset);
    printf("hdr.sh_size = %lu\n", hdr.sh_size);
    printf("hdr.sh_link = %u\n", hdr.sh_link);
    printf("hdr.sh_info = %u\n", hdr.sh_info);
    printf("hdr.sh_addralign = %lu\n", hdr.sh_addralign);
    printf("hdr.sh_entsize = %lu\n", hdr.sh_entsize);
}

static void
print_memory_map(elf_program prog)
{
    if (prog.head == NULL) {
        printf("No memory map!\n");
        return;
    }

    for (elf_memory_map *curr = prog.head; ; curr = curr->next) {
        printf("{\n");
        printf("\tv_addr = 0x%016lX\n", curr->memory.vaddr);
        printf("\tsize = 0x%016lX\n", curr->memory.size);
        printf("\tflags = ");
            print_binary(curr->memory.flags, 64);
            printf("\n");
        printf("}\n");
        if (curr == prog.tail) break;
    }
}

void
elf_print_program(elf_program prog)
{
    if (prog.error) {
        printf("Unable to print program due to error %s\n", prog.error);
        return;
    }
    printf("Printing elf program...\n");
    print_elf_header(prog.elf_header);

    for (uint64_t ii = 0; ii < prog.elf_header.e_phnum; ii++) {
        printf("Printing program header %lu\n", ii);
        print_program_header(prog.program_headers[ii]);
    }

    for (uint64_t ii = 0; ii < prog.elf_header.e_shnum; ii++) {
        printf("Printing section header %lu\n", ii);
        print_section_header(prog.section_headers[ii]);
    }

    print_memory_map(prog);
}
