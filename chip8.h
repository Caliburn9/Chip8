#ifndef __CHIP8_H__
#define __CHIP8_H__

void initialize();
int loadRom(const char *path);
void emulate();
int getDrawFlag();
void setDrawFlag(int b);
unsigned int getDisplayBufferAtI(int index);
void setKeyPadAtI(int index, int value);

#endif