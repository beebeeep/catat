#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>


#define REG_A 7
#define REG_B 0
#define REG_C 1
#define REG_D 2
#define REG_E 3
#define REG_H 4
#define REG_L 5
#define REG_M 6

#define ADDR_BC ((r.B << 8) + r.C)
#define ADDR_DE ((r.D << 8) + r.E)
#define ADDR_HL ((r.H << 8) + r.L)


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

void fail(void) {
    printf("Fatal error occured\n");
    exit(-1);
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
        case 6: { return &( mem[ADDR_HL] ); } break;
        case 7: { return &(r.A); } break;
        default: { fail(); return NULL; }
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
    r.PC += 1;
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

void lxi(uint8_t Ri, uint8_t data_l, uint8_t data_h)
{
/* +-+-+-+-+-+-+-+-+
 * |0|0|i|i|i|0|0|1|
 * +-+-+-+-+-+-+-+-+
 * |d|d|d|d|d|d|d|d| LSB
 * +-+-+-+-+-+-+-+-+
 * |d|d|d|d|d|d|d|d| MSB
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    if(Ri == REG_B || Ri == REG_D || Ri == REG_H) {
        *(get_reg(Ri+1)) = data_l;
        *(get_reg(Ri)) = data_h;
    } else {
        fail();
    }
    r.PC += 3;
}

void stax(uint8_t Ri) {
/* +-+-+-+-+-+-+-+-+             +-+-+-+-+-+-+-+-+
 * |0|0|0|0|0|0|1|0| (STAX B) or |0|0|0|1|0|0|1|0| (STAX D)
 * +-+-+-+-+-+-+-+-+             +-+-+-+-+-+-+-+-+
 * 7               0             7               0
 */
    if(Ri == REG_B) {
        mem[ADDR_BC] = r.A;
    } else if (Ri == REG_D) {
        mem[ADDR_DE] = r.A;
    } else {
        fail();
    }
    r.PC += 1;
}


void ldax(uint8_t Ri) {
/* +-+-+-+-+-+-+-+-+             +-+-+-+-+-+-+-+-+
 * |0|0|0|0|1|0|1|0| (LDAX B) or |0|0|0|1|1|0|1|0| (LDAX D)
 * +-+-+-+-+-+-+-+-+             +-+-+-+-+-+-+-+-+
 * 7               0             7               0
 */
    if(Ri == REG_B) {
        r.A = mem[ADDR_BC];
    } else if (Ri == REG_D) {
        r.A = mem[ADDR_DE];
    } else {
        fail();
    }
    r.PC += 1;
}
