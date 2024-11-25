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
uint16_t sign_extend(uint16_t x, int bit_count) {
    if((x >> (bit_count - 1)) & 1) {
        x |= (0xFFFF << bit_count);
    }
    return x;
}

// swap
// update flags
void update_flags(uint16_t r) {
    if(reg[r] == 0) {
        reg[R_COND] = FL_ZRO;
    } else if(reg[r] >> 15) {
        reg[R_COND] = FL_NEG;
    } else {
        reg[R_COND] = FL_POS;
    }
}



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
                {
       /*DR */      uint16_t r0 = (instr >> 9) & 0x7;
       /*SR1 */     uint16_t r1 = (instr >> 6) & 0x7;
/*check IM-MODE*/   uint16_t imm_flag = (instr >> 5) & 0x1;
                    if(imm_flag) {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[r0] = reg[r1] + imm5;
                    } else {
                        uint16_t r2 = instr & 0x7;
                        reg[r0] = reg[r1] + reg[r2];
                    }
                    update_flags(r0);
                }
                break;
            case OP_AND:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t imm_flag = (instr >> 5) & 0x1;
                    if(imm_flag) {
                        uint16_t imm5 = sign_extend(instr & 0x1F, 5);
                        reg[r0] = reg[r1] & imm5;
                    } else {
                        uint16_t r2 = instr & 0x7;
                        reg[r0] = reg[r1] & reg[r2];
                    }
                    update_flags(r0);
                }
                break;
            case OP_NOT:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;

                    reg[r0] = -reg[r1];
                    update_flags(r0);
                }
                break;
            case OP_BR:
                {
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    uint16_t cond_flag = (instr >> 9) & 0x7;
                    if(cond_flag & reg[R_COND]) {
                        reg[R_PC] += pc_offset;
                    }
                }
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
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extent(instr & 0x1FF, 9);
                    reg[r0] = mem_read(mem_read(reg[R_PC] + pc_offset));
                    update_flags(r0);
                }
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
                exit(2);
                break;
        }
    }
}
