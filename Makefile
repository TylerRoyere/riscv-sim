CC ?=cc
USR_DEFS ?=
OBJ_DIR :=build
INC_DIR :=include
CFLAGS :=-std=c99 -O3 -g -Wall -Wextra -Wpedantic -Wconversion -I$(INC_DIR)
LDFLAGS :=$(USR_DEFS)

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

PROGRAM_SRCS :=$(notdir $(wildcard programs/*.S))
PROGRAM_EXES :=$(foreach src, $(PROGRAM_SRCS), $(src:.S=.bin))

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

%.bin: programs/%.S
	riscv64-linux-gnu-as -o $(OBJ_DIR)/$@.o $<
	riscv64-linux-gnu-ld -T linker.ld -o $@ $(OBJ_DIR)/$@.o

clean:
	rm -r $(OBJ_DIR)
	rm $(TARGETS)
