#ifndef MEMORY_H
#define MEMORY_H

#include <stdint.h>

struct {
  uint8_t A;
  uint8_t B;
  uint8_t C;
  uint8_t D;
  uint8_t E;
  uint8_t H;
  uint8_t L;
  /*struct {
    unsigned int s:1;
    unsigned int z:1;
    unsigned int nul1:1;
    unsigned int h:1;
    unsigned int nul2:1;
    unsigned int p:1;
    unsigned int one:1;
    unsigned int c:1;
    } F;*/
  uint8_t F;
  uint16_t SP;
  uint16_t PC;
} r;

/* 64K should be enough for everyone */
uint8_t mem[65536];

#endif
