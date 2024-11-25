#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdint.h>
#include <signal.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/termios.h>
#include <sys/mman.h>

// memory mapped registers
enum reg {
    R0 = 0,
    R1,
    R2,
    R3,
    R4,
    R5,
    R6,
    R7,
    R_PC,    /* program counter */
    R_COND,
    R_COUNT
};

// condition flags
enum {
    FL_POS = 1 << 0, /* p */
    FL_ZRO = 1 << 1, /* Z */
    FL_NEG = 1 << 2, /* N */
};

// opcodes
enum {
        OP_BR= 0, /* branch */
        OP_ADD,   // add
        OP_LD,    // load
        OP_ST,    // store
        OP_JSR,   // jump register
        OP_AND,   // bitwise and
        OP_LDR,   // load register
        OP_STR,   // store register
        OP_RTI,   // unused
        OP_NOT,   // bitwise not
        OP_LDI,   // load indirect
        OP_STI,   // store indirect
        OP_JMP,   // jump
        OP_RES,   // reserved (unused)
        OP_LEA,   // load effective address
        OP_TRAP   // execute trap
}; 

// trap codes

// memory storage
#define MEMORY_MAX (1 << 16)
uint16_t memory[MEMORY_MAX];


// register storage
uint16_t reg[R_COUNT];

// input buffering
// handle interrupt
// sign extend
// swap
// update flags
// read image file
// read image 
// memory access
// main loop
int main(int argc, char *argv[]) {

    /* load arguments */
    if(argc < 2) {
        printf("lc3 [image-file1]...\n");
        exit(2);
    }

    for(int j = 1; j < argc; ++j)
    {
        if(!read_image(argv[j])) {
            printf("failed to load image: %s\n", argv[j]);
            exit(1);
        }
    }
    /* setup */

    reg[R_COND] = FL_ZRO;

    enum {PC_START = 0x3000 };
    reg[R_PC] = PC_START;

    int running = 1;
    while(running) {
        uint16_t instr = mem_str(reg[R_PC]++);
        uint16_t op = instr >> 12;
        switch(op) {
            case OP_ADD:
                //add
                break;
            case OP_AND:
                //and
                break;
            case OP_NOT:
                //not
                break;
            case OP_BR:
                //br
                break;
            case OP_JMP:
                //jmp
                break;
            case OP_JSR:
                //jsr
                break;
            case OP_LD:
                //ld
                break;
            case OP_LDI:
                //ldi
                break;
            case OP_LDR:
                //ldr
                break;
            case OP_LEA:
                //lea
                break;
            case OP_ST:
                //st
                break;
            case OP_STI:
                //sti
                break;
            case OP_STR:
                //str
                break;
            case OP_TRAP:
                //trap
                break;
            case OP_RES:
            case OP_RTI:
            default:
                //bad opcode
                break;
        }
    }
}
