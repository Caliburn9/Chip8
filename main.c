#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include "chip8.h"

#undef main

// Screen dimension constants
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

// Map keyboard keys to Chip-8 keys
uint8_t keymap[16] = {
	SDLK_1, // 1 
	SDLK_2, // 2
	SDLK_3, // 3
	SDLK_4, // C
	SDLK_q, // 4
	SDLK_w, // 5
	SDLK_e, // 6
	SDLK_r, // D
	SDLK_a, // 7
	SDLK_s, // 8
	SDLK_d, // 9
	SDLK_f, // E
	SDLK_z, // A
	SDLK_x, // 0
	SDLK_c, // B
	SDLK_v  // F
};	 

int main(int argc, char **argv) {
    // Check for correct command-line arguments
    if (argc != 2) {
        perror("Usage: chip8 <rom file>");
        return -1;
    }

    // Initialize SDL
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        printf("SDL could not initialze! SDL Error: %s\n", SDL_GetError());
        return -2;
    } 

    // Create SDL window
    SDL_Window* window = SDL_CreateWindow("Cheddar8", 
                            SDL_WINDOWPOS_UNDEFINED, 
                            SDL_WINDOWPOS_UNDEFINED, 
                            SCREEN_WIDTH, 
                            SCREEN_HEIGHT, 
                            SDL_WINDOW_SHOWN);
    if (window == NULL) {
        printf("Unable to create window! SDL Error: %s\n", SDL_GetError());
        return -2;
    }

    // Create SDL renderer
    SDL_Renderer* renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);

    // Create texture to store some buffer
    SDL_Texture* texture = SDL_CreateTexture(
        renderer,
        SDL_PIXELFORMAT_ABGR8888,
        SDL_TEXTUREACCESS_STREAMING,
        64,
        32
    );

    // Pixel buffer
    uint32_t px[2048];

    // Attempt to load rom
    if (loadRom(argv[1]) == 0) {
        perror("Failed to load ROM!");
        return -3;
    }

    while (1) {
        emulate();

        // Input and Window interaction handling
        SDL_Event event;
        while(SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT) {
                exit(0);
            }

            if (event.type == SDL_KEYDOWN) {
                printf("Key pressed...\n");
                if (event.key.keysym.sym == SDLK_ESCAPE) {
                    exit(0);
                }

                for (int i = 0; i < 16; i++) {
                    if (event.key.keysym.sym == keymap[i]) {
                        setKeyPadAtI(i, 1);
                    }
                }
            }

            if (event.type == SDL_KEYUP) {
                for (int i = 0; i < 16; i++) {
                    if (event.key.keysym.sym == keymap[i]) {
                        setKeyPadAtI(i, 0);
                    }
                }
            }
        }

        // Drawing to the screen
        if (getDrawFlag() == 1) {
            printf("Drawing...\n");
            setDrawFlag(0);
            
            // Set pixel buffer based on display buffer
            for (int i = 0; i < 2048; i++) {
                if (getDisplayBufferAtI(i) == 0) {
                    px[i] = 0x0;
                } else {
                    printf("Setting pixel: %d\n", i);
                    px[i] = 0xffffffff;
                }
            }

            // Update texture
            SDL_UpdateTexture(texture, NULL, px, sizeof(uint32_t) * 64);

            // Clear and present renderer
			SDL_RenderClear(renderer);
			SDL_RenderCopy(renderer, texture, NULL, NULL);
			SDL_RenderPresent(renderer);
        }

        // Delay
        SDL_Delay(5);
    }

    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}