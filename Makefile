CC :=gcc
GNU_TOOLCHAIN_PREFIX ?=/opt/riscv

ifdef GNU_TOOLCHAIN_PREFIX
CROSS_CC :=$(GNU_TOOLCHAIN_PREFIX)/bin/riscv64-unknown-elf-gcc
CROSS_LD :=$(GNU_TOOLCHAIN_PREFIX)/bin/riscv64-unknown-elf-ld
else
CROSS_CC :=riscv64-unknown-elf-gcc
CROSS_LD :=riscv64-unknown-elf-ld
endif

USR_DEFS ?=
OBJ_DIR :=build
INC_DIR :=include
CFLAGS :=-std=c99 -O3 -g -Wall -Wextra -Wpedantic -Wconversion -I$(INC_DIR)
LDFLAGS :=$(USR_DEFS)
CROSS_CFLAGS :=-mabi=lp64 -march=rv64g -mcmodel=medany -nostdlib -nostartfiles 
CROSS_LDFLAGS :=-static -T link.ld

######### Options #########
# Sanitizers
#CFLAGS +=-fsanitize=address
#CFLAGS +=-fsanitize=leak
#CFLAGS +=-fsanitize=thread
#CFLAGS +=-fsanitize=undefined
# Set to enable only RV32
#CFLAGS +=-DRV32_ONLY=1 -DXLEN=32

MAIN_SRCS :=$(notdir $(wildcard src/*.c))
MAIN_OBJS :=$(foreach src, $(MAIN_SRCS), $(OBJ_DIR)/$(src:.c=.o))

PROGRAM_SRCS :=$(notdir $(wildcard programs/*.c))
PROGRAM_EXES :=$(foreach src, $(PROGRAM_SRCS), $(src:.c=.bin))

TARGETS :=main $(PROGRAM_EXES)

all: $(TARGETS)

main: $(MAIN_OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LDFLAGS)

$(OBJ_DIR)/%.o: src/%.c %.h | $(OBJ_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ_DIR)/%.o: src/%.c | $(OBJ_DIR)
	$(CC) -c -o $@ $< $(CFLAGS)

$(OBJ_DIR):
	mkdir $@

programs: $(PROGRAM_EXES)

%.bin: programs/%.o programs/entry/entry.o
	$(CROSS_LD) $(CROSS_LDFLAGS) -o $@ $^

programs/%.o: programs/%.c
	$(CROSS_CC) $(CROSS_CFLAGS) -c -o $@ $<

programs/%.o: programs/%.S
	$(CROSS_CC) $(CROSS_CFLAGS) -c -o $@ $<

clean:
	rm -r $(OBJ_DIR)
	rm $(TARGETS)
	rm programs/*.o
