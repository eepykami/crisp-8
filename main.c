#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "SDL.h"

#include "font.h"
#include "ibm_rom.h"

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
void sdlInit();

void sdlInit() {
    SDL_Init(SDL_INIT_VIDEO);

    // Creating window, setting title bar name, location on screen, and size.
    SDL_Window *window = SDL_CreateWindow(
        "Crisp-8",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640,
        480,
        0
    );

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_SOFTWARE);
    SDL_SetRenderDrawColor(renderer, 160, 32, 240, SDL_ALPHA_OPAQUE);
    SDL_RenderClear(renderer);
    SDL_RenderPresent(renderer);

    SDL_Delay(3000);

    SDL_DestroyWindow(window);
    SDL_Quit();
     
}

void innit() {
    memcpy(ram, font, sizeof(font)); // Copy font data to start of RAM
    memcpy(ram + 0x200, ibm_rom, sizeof(ibm_rom)); // Copy program to RAM starting at 0x200
    pc = 0x200; // Set program counter to start of program code
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
        case 0x0: { // Clear screen
            memset(display, 0, sizeof(display));
            break;
        }
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
            uint8_t vx = V[(instruction >> 8) & 0x000F]; // Fetch X value from instruction
            uint8_t vy = V[(instruction >> 4) & 0x000F]; // Fetch Y value from instruction
            uint8_t n = instruction & 0xF; // Number of bytes the sprite data takes up
            V[0xF] = 0; // setting collision register to 0 to start off with

            // Loop to step through the number of bytes to draw
            for(int i = 0; i < n; i++) {
                int rowI = (I + i) & 0xFFF; // Row index, wrapped. Mem. address the row starts at.
                int row = ram[rowI]; // Contents (bits) of the row. // TODO: fix stuff innit

                int y = (vy + i) & 31; // If above 31, wrap back around.

                // Loop to step through individual bits per byte of data
                for(int j = 0; j < 8; j++) { // 8 pixels in a row, n rows in a sprite.
                    int x = (vx + j) & 63; // If above 63, wrap back around.
                    int index = y * 64 + x; // Index for the display array.

                    // j is bit on row vy, I register is where sprite starts.
                    // while we havent reached the right edge of the row?
                    
                    if((row & 0x80) == 0x80 && display[index] == 1) {
                        display[index] = 0;
                        V[0xF] = 1; // setting collision register to 1
                    } else if((row & 0x80) == 0x80 && display[index] == 0) {
                        display[index] = 1;
                    }
                
                    row = row << 1;
                }
            } 
            break;
        }
        default: printf("Unknown instruction. Bit sad, innit.\n");
                 exit(-1);
    }
}

int main(int argc, char** argv) {
    //sdlInit();
    innit();

    while (1) {
        for (int i = 0; i < 512; i++) {
            step();
        }
        
        for (int index = 0; index < 2048; index++) {
            if(display[index] == 1) {
                printf("██"); 
            } else {
                printf("  ");
            }
            
            if(index % 64 == 63) { // if index is a multiple of 64 then we can go to a new line
                printf("\n");
            }
        }
        printf("\n\n");
    }
    /*
    // argc will contain the amount of arguments provided to the executable. If this is 1, we know nothing has been provided. (1 because the name of executable is first)
    if(argc == 1) {
        printf("No ROM provided!\n");
        exit(-1); // Exit with an error code, -1.
    }

    // We got here, so there's some arguments. Let's see what the second one is (first will be the executable name again)
    printf("%s\n", argv[1]);
    */
    
    return 0;
}
