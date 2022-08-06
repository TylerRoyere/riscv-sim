# riscv-sim

Simulators that exexutes instructions from the RISC-V ISA.

The intent of this project is to learn about the RISC-V ISA by creating a program that can simulate the environment of a RISC-V hart (core). It is functional enough to execute primitive C programs and some of the `riscv-tests` that use supported instruction.

## Goals

These pieces of functionality are ultimately desireable as they will allow a greater breadth of RISC-V programs and features to be exploited
- [x] RV32I/RV64I base unprivileged integer intruction set
- [ ] RV32I/RV64I base privileged integer instruction set
    - [ ] Proper CSR support
    - [ ] Interrupt/Trap/Exception propogation and handling
- [ ] RV32/RV64 zifencei and zicsr standard extensions
- [ ] RV32M/RV64M multiplication and division standard extensions
- [ ] RV32A/RV64A atomic standard extensions
- [ ] RV32F/RV64F floating point standard extensions
- [ ] RV32D/RV64D double floating point stanard extensions

## Build

### RISC-V Simulator

Riscv-sim uses a simple `Makefile` for producing the simulator. To build the simulator `gcc` is required.

```
$ make main
```

This will produce an executable `main` that can be used to run programs compiled for a RISC-V target

### RISC-V Programs

Example C programs are included in the `programs/` directory of the project. These are simple programs that use the runtime provided in `programs/entry/entry.S` to work. To compile these programs a `riscv64-unknown-elf-gcc` cross compiler and `riscv64-unknown-elf-ld` linker is required. The makefile assumes the cross compiler is installed at `/opt/riscv/share/bin`. For you, if this is not the case, provide a value for `GNU_TOOLCHAIN_PREFIX` where your gnu toolchain is installed. Otherwise, if these programs are on your `PATH`, set this to `''`.

```
$ make programs
```

## Running

Provided you have compiled the example `gcd.c` program into `gcd.bin` this program can be executed using the `main` simulator executable.

```
$ ./main gcd.bin
```

The simulator will load `gcd.bin`, parse its ELF information, load the requisite program segments into memory, and start executing of the RISC-V program.

