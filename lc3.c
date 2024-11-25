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
enum {
    TRAP_GETC = 0x20,   /*get character from keyboard, not echoed onto the terminal*/
    TRAP_OUT = 0x21,    /*output a character*/
    TRAP_PUTS = 0x22,   /*output a word string*/
    TRAP_IN = 0x23,     /*get character from keyboard, echoed onto the terminal*/
    TRAP_PUTSP = 0x24,  /*output a byte string*/
    TRAP_HALT = 0x25    /*halt the program*/
};

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
uint16_t swap16(uint16_t x) {
    return (x << 8) | (x >> 8);
}
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
void read_image_file(FILE* file) {
    uint16_t origin;
    fread(&origin, sizeof(origin), 1, file);
    origin = swap16(origin);

    uint16_t max_read = MEMORY_MAX - origin;
    uint16_t* p = memory + origin;
    size_t read = fread(p, sizeof(uint16_t), max_read, file);

    while(read-- > 0) {
        *p = swap16(*p);
        ++p;
    }
}

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
                {
                    uint16_t r1 = (instr >> 6) & 0x7;
                    reg[R_PC] = reg[r1];
                }
                break;
            case OP_JSR:
                {
                    uint16_t long_flag = (instr >> 11) & 1;
                    reg[R_R7] = reg[R_PC];
                    if(long_flag) {
                        uint16_t long_pc_offset = sign_extend(instr & 0x7FF, 11);
                        reg[R_PC] += long_pc_offset;
                    } else {
                        uint16_t r1 = (instr >> 6) & 0x7;
                        reg[R_PC] = reg[r1];
                    }
                }
                break;
            case OP_LD:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[r0] = mem_read(reg[R_PC] + pc_offset);
                    update_flags(r0);
                }
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
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3f, 6);
                    reg[r0] = mem_read(reg[r1] + offset);
                    update_flags(r0);
                }
                break;
            case OP_LEA:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    reg[r0] = reg[r1] + pc_offset;
                    update_flags(r0);
                }
                break;
            case OP_ST:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(reg[R_PC] + pc_offset, reg[r0]);
                }
                break;
            case OP_STI:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t pc_offset = sign_extend(instr & 0x1FF, 9);
                    mem_write(mem_read(reg[R_PC] + pc_offset), reg[r0]);
                }
                break;
            case OP_STR:
                {
                    uint16_t r0 = (instr >> 9) & 0x7;
                    uint16_t r1 = (instr >> 6) & 0x7;
                    uint16_t offset = sign_extend(instr & 0x3F, 6);
                    mem_write(reg[r1] + offset, reg[r0]);
                }
                break;
            case OP_TRAP:
                {
                    reg[R_R7] = reg[R_PC];
                    switch (instr & 0xFF) {
                        case TRAP_GETC:
                            //trap_getc
                            break;
                        case TRAP_OUT:
                            //trap_out
                            break;
                        case TRAP_PUTS:
                            //trap_puts
                            break;
                        case TRAP_IN:
                            //trap_in
                            break;
                        case TRAP_PUTSP:
                            //trap_putsp
                            break;
                        case TRAP_HALT:
                            //trap_halt
                            break;
                    }
                }
                break;
            case OP_RES:
            case OP_RTI:
            default:
                exit(2);
                break;
        }
    }
}
