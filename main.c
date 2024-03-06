#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

// Memory
uint8_t ram[4096]; // 4KiB of RAM
uint16_t stack[16]; // 16 byte stack
uint8_t display[2048]; // Size of display, 64 x 32 

// Registers
uint8_t V[16];
uint16_t I;
uint8_t delayTimer;
uint8_t soundTimer;
uint16_t pc;
uint8_t sp;

// Prototypes
void innit();
void step();

void innit() {
    pc = 0x200; // Program counter, start of program code

    /* Standard 4x5 font */
    static const uint8_t font[] = {
    /* '0' */ 0xF0, 0x90, 0x90, 0x90, 0xF0,
    /* '1' */ 0x20, 0x60, 0x20, 0x20, 0x70,
    /* '2' */ 0xF0, 0x10, 0xF0, 0x80, 0xF0,
    /* '3' */ 0xF0, 0x10, 0xF0, 0x10, 0xF0,
    /* '4' */ 0x90, 0x90, 0xF0, 0x10, 0x10,
    /* '5' */ 0xF0, 0x80, 0xF0, 0x10, 0xF0,
    /* '6' */ 0xF0, 0x80, 0xF0, 0x90, 0xF0,
    /* '7' */ 0xF0, 0x10, 0x20, 0x40, 0x40,
    /* '8' */ 0xF0, 0x90, 0xF0, 0x90, 0xF0,
    /* '9' */ 0xF0, 0x90, 0xF0, 0x10, 0xF0,
    /* 'A' */ 0xF0, 0x90, 0xF0, 0x90, 0x90,
    /* 'B' */ 0xE0, 0x90, 0xE0, 0x90, 0xE0,
    /* 'C' */ 0xF0, 0x80, 0x80, 0x80, 0xF0,
    /* 'D' */ 0xE0, 0x90, 0x90, 0x90, 0xE0,
    /* 'E' */ 0xF0, 0x80, 0xF0, 0x80, 0xF0,
    /* 'F' */ 0xF0, 0x80, 0xF0, 0x80, 0x80,
    };
}

void step() {
    // Read from PC
    uint8_t highByte = ram[pc++]; // Read high byte, then increment PC
    uint8_t lowByte = ram[pc++]; // Read low byte, then increment PC

    pc &= 0xFFF; // Pointers are 12 bits, but PC is 16 bits, we need to do this to handle wrapping, so that we ignore anything higher than 12-bits.

    // Now we have the full instruction, we need to combine two uint8_t's into one uint16_t.

    uint16_t instruction = lowByte |(highByte << 8); // Shift highByte left by 8 bits, and OR with lowByte.  
                                                     // highByte ex. 10101010 00000000, when OR'ing with 0's the lowByte is unchanged.

    // We have the full instruction, and can use a switch statement to figure out what we need to do.

    /*
    TODO: Instuctions to implement:

    00E0 (clear screen)
    1NNN (jump)
    6XNN (set register VX)
    7XNN (add value to register VX)
    ANNN (set index register I)
    DXYN (display/draw)   
    */

    switch(instruction >> 12) { // We only care about the last 4 bits
        case 0x0: break; // Clear screen
        case 0x1: { // Jump - set PC to NNN 
            uint16_t nnn = instruction & 0xFFF; // Mask first 4 bits by AND'ing the instruction with 0xFFF, we don't care about the last bit as that's the opcode. 
            pc = nnn;
            break;
        }
        case 0x6: {
            uint8_t x = (instruction >> 8) & 0x000F; // First move x 8 bits left, then mask the rest of the instruction.
            uint8_t nn = instruction & 0x00FF; // Mask value of register from instruction.
            V[x] = nn;
            break;
        }
        case 0x7: {
            uint8_t x = (instruction >> 8) & 0x000F; // First move x 8 bits left, then mask the rest of the instruction.
            uint8_t nn = instruction & 0x00FF; // Mask value of register from instruction.
            V[x] += nn;
            break;
        }
        case 0xA: {
            uint16_t nnn = instruction & 0xFFF; // Mask first 4 bits by AND'ing the instruction with 0xFFF, we don't care about the last bit as that's the opcode. 
            I = nnn;
            break;
        }
        case 0xD: {
            uint8_t vx = V[(instruction >> 8) & 0x000F];
            uint8_t vy = V[(instruction >> 4) & 0x000F];
            uint8_t n = instruction & 0xF;

            for(int i = 0; i < n; i++) {
                int rowI = (I + i) & 0xFFF; // Row index, wrapped. Mem. address the row starts at.
                int row = ram[rowI]; // Contents (bits) of the row. // TODO: fix stuff innit 
                int y = (vy + i) & 31; // If above 31, wrap back around.
                for(int j = 0; j < 8; j++) { // 8 pixels in a row, n rows in a sprite.
                    int x = (vx + j) & 63; // If above 63, wrap back around.
                    int index = y * 64 + x; // Index for the display array.
                    // j is bit on row vy, I register is where sprite starts.
                }
            } 
            break;
        }
        default: printf("you fucked up\n");
                 exit(-1);
    }
}

int main(int argc, char** argv) {
    printf("%s\n", argv[1]);

    return 0;
}
