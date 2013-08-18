#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

struct {
    uint8_t A;
    uint8_t B;
    uint8_t C;
    uint8_t D;
    uint8_t E;
    uint8_t H;
    uint8_t L;
    struct {
        unsigned int s:1;
        unsigned int z:1;
        unsigned int nul1:1;
        unsigned int h:1;
        unsigned int nul2:1;
        unsigned int p:1;
        unsigned int one:1;
        unsigned int c:1;
    } F;
    uint16_t SP;
    uint16_t PC;
} r;

//64K should be enough for everyone
uint8_t mem[65536];

int main (int argc, char *argv[])
{
    printf("ok\n");
    return 0;
}

//return address of register specified by its 3-bit code
uint8_t *get_reg(uint8_t code) {

    switch(code) {
        case 0: { return &(r.B); } break;
        case 1: { return &(r.C); } break;
        case 2: { return &(r.D); } break;
        case 3: { return &(r.E); } break;
        case 4: { return &(r.H); } break;
        case 5: { return &(r.L); } break;
        //pseudo-register M = memory cell addressed by H & L registers
        case 6: { return &( mem[(r.H << 8) + r.L] ); } break;   
        case 7: { return &(r.A); } break;
        default: { return NULL; }
    }
}

void mov(uint8_t Ri, uint8_t Rj) 
{
/* +-+-+-+-+-+-+-+-+
 * |0|1|i|i|i|j|j|j|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    *(get_reg(Ri)) = *(get_reg(Rj));
    r.PC++;
}

void mvi(uint8_t Ri, uint8_t data) 
{
/* +-+-+-+-+-+-+-+-+
 * |0|0|i|i|i|1|1|0|
 * +-+-+-+-+-+-+-+-+
 * |d|d|d|d|d|d|d|d|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    *(get_reg(Ri)) = data;
    r.PC += 2;
}

