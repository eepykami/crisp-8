#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include "SDL.h"

#include "font.h"
#include "ibm_rom.h"
#include <stdbool.h>

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
void sdlInit();
void emulationInnit();
void step();
void draw();

SDL_Window *window = NULL;
SDL_Surface *surface = NULL;

#define SCREEN_W 640
#define SCREEN_H 320

void sdlInit() {
    SDL_Init(SDL_INIT_VIDEO);

    // Creating window, setting title bar name, location on screen, and size.
    window = SDL_CreateWindow(
        "Crisp-8",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        SCREEN_W,
        SCREEN_H,
        0
    );

    surface = SDL_GetWindowSurface(window);
}

uint8_t *romPointer = NULL;

void emulationInnit(void* romPointer, size_t romSize) {
    memcpy(ram, font, sizeof(font)); // Copy font data to start of RAM
    //memcpy(ram + 0x200, ibm_rom, sizeof(ibm_rom)); // Copy program to RAM starting at 0x200
    memcpy(ram + 0x200, romPointer, romSize); // Copy program to RAM starting at 0x200
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

                    if((row & 0x80) > 0) {
                        if(display[index] == 1) {
                            V[0xF] = 1; // setting collision register to 1
                        }
                        display[index] ^= 1;
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

void setPixel(SDL_Surface *surface, int x, int y, Uint32 pixel)
{
    Uint8 *target_pixel = (Uint8 *)surface->pixels + y * surface->pitch + x * 4;
    *(Uint32 *)target_pixel = pixel;
}


void draw() {
    uint32_t byte = 0;
    int x1;
    int y1;
    int index;

    for(int x = 0; x < SCREEN_W; x++) {
        for(int y = 0; y < SCREEN_H; y++) {

            x1 = ((float)x / SCREEN_W) * 64;
            y1 = ((float)y / SCREEN_H) * 32;
            index = y1 * 64 + x1;

            if(display[index] == 1) {
                byte = 0xffffffff;
            } else {
                byte = 0;
            }

            setPixel(surface, x, y, byte);
        }
    }
    SDL_UpdateWindowSurface(window);
}

int main(int argc, char* args[]) {    
    int romSize;
    FILE* rom;
    sdlInit(); 
   
    // argc will contain the amount of arguments provided to the executable. If this is 1, we know nothing has been provided. (1 because the name of executable is first)
    if(argc == 1) {
        printf("No ROM provided! Using IBM ROM.\n");
        SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_WARNING, "Crisp-8", "No ROM provided! Using IBM ROM.", window);
        romSize = sizeof(ibm_rom);
        romPointer = malloc(romSize);
        memcpy(romPointer, ibm_rom, romSize);
    } else {
        rom = fopen(args[1], "rb");
        printf("File name is %s.\n", args[1]);  
        if(rom == NULL) {
            perror("Error opening file");
            return 1;
        }

        if(fseek(rom, 0, SEEK_END)) {
            perror("Error");
            return 1;
        }

        romSize = ftell(rom);
        rewind(rom);
        printf("Size of ROM is %d bytes.\n", romSize);

        romPointer = malloc(romSize);
        size_t result = fread(romPointer, romSize, 1, rom);
        if(result != 1) {
            perror("Error");
            return 1;
        }
        fclose(rom);
    }

    emulationInnit(romPointer, romSize);

    int close = 0;
    while (!close) {
        SDL_Event event;
        // Events management
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
 
            case SDL_QUIT: // handling of close button
                close = 1;
                break;
            }
        }

        // Program loop
        for (int i = 0; i < 512; i++) {
            step();
        }    
        draw();
    }

    // Quit
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}
