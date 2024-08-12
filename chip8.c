#include <stdio.h>
#include <stdlib.h>
#include "chip8.h"

// Custom number types
typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;
typedef int bool;
#define true 1;
#define false 0;

// Declare global variables
uint8_t memory[4096]; // RAM
uint8_t V[16]; // Registers
uint16_t stack[16]; // Stack
uint8_t keypad[16]; // 16-key hexadecimal Keypad
uint32_t displayBuffer[64*32]; 
uint8_t memoryRegister; // Memory Register or I
uint8_t delayTimer; 
uint8_t soundTimer;
uint16_t pc; // Program Counter or Instruction Pointer
uint8_t sp; // Stack Pointer
uint16_t opcode; // Operation Code
bool draw_flag = false;
uint8_t fontset[80] = {
	0xF0, 0x90, 0x90, 0x90, 0xF0, // "0"
	0x20, 0x60, 0x20, 0x20, 0x70, // "1"
	0xF0, 0x10, 0xF0, 0x80, 0xF0, // "2"
	0xF0, 0x10, 0xF0, 0x10, 0xF0, // "3"
	0x90, 0x90, 0xF0, 0x10, 0x10, // "4"
	0xF0, 0x80, 0xF0, 0x10, 0xF0, // "5"
	0xF0, 0x80, 0xF0, 0x90, 0xF0, // "6"
	0xF0, 0x10, 0x20, 0x40, 0x40, // "7"
	0xF0, 0x90, 0xF0, 0x90, 0xF0, // "8"
	0xF0, 0x90, 0xF0, 0x10, 0xF0, // "9"
	0xF0, 0x90, 0xF0, 0x90, 0x90, // "A"
	0xE0, 0x90, 0xE0, 0x90, 0xE0, // "B"
	0xF0, 0x80, 0x80, 0x80, 0xF0, // "C"
	0xE0, 0x90, 0x90, 0x90, 0xE0, // "D"
	0xF0, 0x80, 0xF0, 0x80, 0xF0, // "E"
	0xF0, 0x80, 0xF0, 0x80, 0x80  // "F"
};

// Emulator Functions
void initialize() {
    // Intialize special purpose registers
    memoryRegister = V[15];
    pc = 0x200;
    sp = 0;
    opcode = 0;

    // Clear memory
    for (int i = 0; i < 4096; i++) {
        memory[i] = 0;
    }

    // Clear stack, registers, and keypad
    for (int i = 0; i < 16; i++) {
        stack[i] = 0;
        V[i] = 0;
        keypad[i] = 0;
    }

    // Clear display buffer
    for (int i = 0; i < 2048; i++) {
        displayBuffer[i] = 0;
    }

    // Load fontset
    for (int i = 0; i < 80; i++) {
        memory[i] = fontset[i];
    }

    // Clear timers
    delayTimer = 0;
    soundTimer = 0;
    return;
}

int loadRom(const char *path) {
    initialize();

    printf("Loading file...\n");
    FILE *file = fopen(path, "rb");

    if (file != NULL) {
        printf("Reading file...\n");

        // Get rom size
        fseek(file, 0, SEEK_END);
        const int rom_size = ftell(file);
        fseek(file, 0, SEEK_SET);

        // Check if rom size is greater than data space in Chip-8 RAM (4096 - 512) 
        if ((4096 - 512) > rom_size) {
            // Initialize buffer
            char *buffer = (char*)malloc(rom_size*sizeof(char));
            if (buffer == NULL) {
                perror("Memory allocation error");
                fclose(file);
                return 0;
            }

            // Load rom file to buffer
            fread(buffer, 1, rom_size, file);

            // Load buffer to Chip-8 memory
            for (int i = 0; i < 4096; i++) {
                memory[i+512] = buffer[i];
            }

            free(buffer);
        } else {
            perror("Rom file is too big");
            fclose(file);
            return 0;
        }
    } else {
        perror("Unable to open file");
        fclose(file);
        return 0;
    }

    fclose(file);
    return 1;
}

void emulate() {
    // Set opcode
    opcode = (memory[pc] << 8) | (memory[pc+1]);

    // Instruction variables
    uint16_t nnn = opcode & 0x0fff; // Lowest 12 bytes of the instruction
    uint8_t n = opcode & 0x000f; // Lowest 4 bytes of the instruction
    uint8_t x = opcode >> 8 & 0x000f; // Lower 4 bytes of the high byte of the instruction
    uint8_t y = opcode >> 4 & 0x000f; // Lower 4 bytes of the low byte of the instruction
    uint8_t kk = opcode & 0x00ff; // Lowest 8 bytes  of the instruction

    switch (opcode & 0xf000) {
    case 0x0000:
        switch (kk) {
            // CLS
        case 0x00e0:
            printf("00e0 ");
            for (int i = 0; i < 2048; i++) {
                displayBuffer[i] = 0x0;
            }
            draw_flag = true;
            pc += 2;
            break;
        
            // RET
        case 0x00ee:
            printf("00ee ");
            pc = stack[sp];
            sp--;
            pc += 2;
            break;

        default:
            perror("Invalid Opcode");
            break;
        }
        break;
    
    case 0x1000:
        // JP addr
        printf("1nnn ");
        pc = nnn;
        break;

    case 0x2000:
        // CALL addr
        printf("2nnn ");
        sp++;
        stack[sp] = pc;
        pc = nnn;
        break;
    
    case 0x3000:
        // SE Vx, byte
        printf("3xkk ");
        if (V[x] == kk) {
            pc += 4;
        } else {
            pc += 2;
        }
        break;

    case 0x4000:
        // SNE Vx, byte
        printf("4xkk ");
        if (V[x] != kk) {
            pc += 4;
        } else {
            pc += 2;
        }
        break;
    
    case 0x5000:
        // SE Vx, Vy
        printf("5xy0 ");
        if (V[x] == V[y]) {
            pc += 4;
        } else {
            pc += 2;
        }
        break;

    case 0x6000:
        // LD Vx, byte
        printf("6xkk ");
        V[x] = kk;
        pc += 2;
        break;
    
    case 0x7000:
        // ADD Vx, byte
        printf("7xkk ");
        V[x] += kk;
        pc += 2;
        break;

    case 0x8000:
        switch (n) {
            // LD Vx, Vy
        case 0x0000:
            printf("8xy0 ");
            V[x] = V[y];
            pc += 2;
            break;
        
            // OR Vx, Vy
        case 0x0001:
            printf("8xy1 ");
            V[x] |= V[y];
            pc += 2;
            break;
        
            // AND Vx, Vy
        case 0x0002:
            printf("8xy2 ");
            V[x] &= V[y];
            pc += 2;
            break;

            // XOR Vx, Vy
        case 0x0003:
            printf("8xy3 ");
            V[x] ^= V[y];
            pc += 2;
            break;

            // ADD Vx, Vy
        case 0x0004:
            printf("8xy4 ");
            if (V[y] > (0xff - V[x])) {
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}
            V[x] += V[y];
            pc += 2;
            break;

            // SUB Vx, Vy
        case 0x0005:
            printf("8xy5 ");
            if (V[x] > V[y]) {
				V[0xF] = 1;
			}
			else {
				V[0xF] = 0;
			}
            V[x] -= V[y];
            pc += 2;
            break;

            // SHR Vx, Vy
        case 0x0006:
            printf("8xy6 ");
            if ((V[x] & 0x01) == 1) {
                V[0xf] = 1;
            } else {
                V[0xf] = 0;
            }
            V[x] >>= 1; // can also be V[x] /= 2;
            pc += 2;
            break;

            // SUBN Vx, Vy
        case 0x0007:
            printf("8xy7 ");
            if (V[y] > V[x]) {
                V[0xf] = 1;
            } else {
                V[0xf] = 0;
            }
            V[x] = V[y] - V[x];
            pc += 2;
            break;

            // SHL Vx, Vy
        case 0x000E:
            printf("8xyE ");
            if (((V[x] >> 7) & 0x01) == 1) {
                V[0xf] = 1;
            } else {
                V[0xf] = 0;
            }
            V[x] <<= 1; // can also be V[x] *= 2;
            pc += 2;
            break;
        
        default:
            perror("Invalid Opcode");
            break;
        }
        break;
    
    case 0x9000:
        // SNE Vx, Vy
        printf("9xy0 ");
        if (V[x] != V[y]) {
            pc += 4;
        } else {
            pc += 2;
        }
        break;

    case 0xa000:
        // LD I, addr
        printf("Annn ");
        memoryRegister = nnn;
        pc += 2;
        break;
    
    case 0xb000:
        // JP V0, addr
        printf("Bnnn ");
        pc = nnn + V[0];
        break;

    case 0xc000:
        // RND Vx, byte
        printf("Cxkk ");
        V[x] = (rand() % 0xff) & kk;
        pc += 2;
        break;
    
    case 0xd000:
        // DRW Vx, Vy, nibble
        printf ("Dxyn ");
        uint8_t xx = V[x];
        uint8_t yy = V[y];
        uint8_t pixel;
        V[0xf] = 0;
        for (int i = 0; i < n; i++) {
            pixel = memory[memoryRegister + i];
            uint8_t row = (yy + i) % 32;
            for (int j = 0; j < 8; j++) {
                uint8_t col = (xx + j) % 64;     
                uint8_t offset = row * 64 + col;
                if (displayBuffer[offset] == 1) {
                    V[0xf] = 1;
                }
                if (displayBuffer[offset] != ((pixel >> (7 - j)) & 0x1)) {
                    displayBuffer[offset] ^= 1;
                }
            }
        }
        draw_flag = true;
        pc += 2;
        break;

    case 0xe000:
        switch (kk) {
            // SKP Vx
        case 0x009e:
            printf("Ex9E ");
            if (keypad[V[x]] != 0) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;

            // SKNP Vx
        case 0x00a1:
            printf("ExA1 ");
            if (keypad[V[x]] == 0) {
                pc += 4;
            } else {
                pc += 2;
            }
            break;
        
        default:
            perror("Invalid Opcode");
            break;
        }
        break;
    
    case 0xf000:
        switch (kk) {
            // LD Vx, DT
        case 0x0007:
            printf("Fx07 ");
            V[x] = delayTimer;
            pc += 2;
            break;
        
        // LD Vx, K
        case 0x000a:
            printf("Fx0A ");
            bool keyPressed = false;
            for (int i = 0; i < 16; i++) {
                if (keypad[i] != 0) {
                    V[x] = i;
                    keyPressed = true;
                    break;
                }
            }
            if (!keyPressed) {
                pc -= 2;
            } else {
                pc += 2;
            }
            break;

        // LD DT, Vx
        case 0x0015:
            printf("Fx15 ");
            delayTimer = V[x];
            pc += 2;
            break;

        // LD ST, Vx
        case 0x0018:
            printf("Fx07 ");
            soundTimer = V[x];
            pc += 2;
            break;

        // ADD I, Vx
        case 0x001e:
            printf("Fx1E ");
            if (memoryRegister + V[x] > 0xfff) {
                V[0xf] = 1;
            } else {
                V[0xf] = 0;
            }
            memoryRegister += V[x];
            pc += 2;
            break;

        // LD F, Vx
        case 0x0029:
            printf("Fx29 ");
            memoryRegister = V[x] * 0x5;
            pc += 2;
            break;

        // LD B, Vx
        case 0x0033:
            printf("Fx33 ");
            memory[memoryRegister] = V[x] / 100;
            memory[memoryRegister + 1] = (V[x] / 10) % 10;
            memory[memoryRegister + 2] = V[x] % 10; 
            pc += 2;
            break;

        // LD [I], Vx
        case 0x0055:
            printf("Fx55 ");
            for (int i = 0; i <= x; i++) {
                memory[memoryRegister+i] = V[i];
            }
            memoryRegister += (x + 1);
            pc += 2;
            break;

        // LD Vx, [I]
        case 0x0065:
            printf("Fx65 ");
            for (int i = 0; i <= x; i++) {
                V[i] = memory[memoryRegister+i];
            }
            memoryRegister += (x + 1);
            pc += 2;
            break;
        
        default:
            break;
        }
        break;

    default:
        perror("Invalid Opcode");
        break;
    }

    // Sound and Delay Timers
    if (delayTimer > 0) {
        delayTimer--;
    }
    if (soundTimer > 0) {
        printf("BEEP\n");
        soundTimer--;
    }

    return;
}

int getDrawFlag() {
    return draw_flag;
    printf("Draw flag is: %d\n", draw_flag);
}

void setDrawFlag(int b) {
    printf("Draw flag was: %d\n", draw_flag);
    draw_flag = b;
    printf("Draw flag is now: %d\n", draw_flag);
}

unsigned int getDisplayBufferAtI(int index) {
    return displayBuffer[index];
}

void setKeyPadAtI(int index, int value) {
    if (index >= 0 && index < 16) {
        keypad[index] = value;
    }
}