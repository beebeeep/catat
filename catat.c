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
#define ADDR(LO, HI) (((HI) << 8) + (LO))


#define IS_BDH(x) ((x) == REG_B || (x) == REG_D || (x) == REG_H)


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
        case REG_B: { return &(r.B); } break;
        case REG_C: { return &(r.C); } break;
        case REG_D: { return &(r.D); } break;
        case REG_E: { return &(r.E); } break;
        case REG_H: { return &(r.H); } break;
        case REG_L: { return &(r.L); } break;
        //pseudo-register M = memory cell addressed by H & L registers
        case REG_M: { return &( mem[ADDR_HL] ); } break;
        case REG_A: { return &(r.A); } break;
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
    if(IS_BDH(Ri)) {
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

void sta(uint8_t addr_l, uint8_t addr_h)
{
/* +-+-+-+-+-+-+-+-+
 * |0|0|1|1|0|0|1|0|
 * +-+-+-+-+-+-+-+-+
 * |d|d|d|d|d|d|d|d| LSB
 * +-+-+-+-+-+-+-+-+
 * |d|d|d|d|d|d|d|d| MSB
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    mem[ADDR(addr_l, addr_h)] = r.A;
    r.PC += 3;
}

void lda(uint8_t addr_l, uint8_t addr_h)
{
/* +-+-+-+-+-+-+-+-+
 * |0|0|1|1|1|0|1|0|
 * +-+-+-+-+-+-+-+-+
 * |d|d|d|d|d|d|d|d| LSB
 * +-+-+-+-+-+-+-+-+
 * |d|d|d|d|d|d|d|d| MSB
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    r.A = mem[ADDR(addr_l, addr_h)];
    r.PC += 3;
}

void shld(uint8_t addr_l, uint8_t addr_h)
{
/* +-+-+-+-+-+-+-+-+
 * |0|0|1|0|0|0|1|0|
 * +-+-+-+-+-+-+-+-+
 * |d|d|d|d|d|d|d|d| LSB
 * +-+-+-+-+-+-+-+-+
 * |d|d|d|d|d|d|d|d| MSB
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    mem[ADDR(addr_l, addr_h)] = r.L;
    mem[ADDR(addr_l+1, addr_h)] = r.H;
    r.PC += 3;
}

void lhld(uint8_t addr_l, uint8_t addr_h)
{
/* +-+-+-+-+-+-+-+-+
 * |0|0|1|0|0|0|1|0|
 * +-+-+-+-+-+-+-+-+
 * |d|d|d|d|d|d|d|d| LSB
 * +-+-+-+-+-+-+-+-+
 * |d|d|d|d|d|d|d|d| MSB
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    r.L = mem[ADDR(addr_l, addr_h)];
    r.H = mem[ADDR(addr_l+1, addr_h)];
    r.PC += 3;
}

void push(uint8_t Ri) 
{
/* +-+-+-+-+-+-+-+-+
 * |1|1|i|i|i|1|0|1|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    if(IS_BDH(Ri)) {
        mem[--r.SP] = *(get_reg(Ri));
        mem[--r.SP] = *(get_reg(Ri+1));
    } else {
        fail();
    }
    r.PC += 1;
}

void push_psw(void) 
{
/* +-+-+-+-+-+-+-+-+
 * |1|1|1|1|0|1|0|1|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    mem[--r.SP] = r.A;
    mem[--r.SP] = r.F;
    r.PC += 1;
}

void pop(uint8_t Ri) 
{
/* +-+-+-+-+-+-+-+-+
 * |1|1|i|i|i|0|0|1|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    if(IS_BDH(Ri)) {
        *(get_reg(Ri+1)) = mem[++r.SP];
        *(get_reg(Ri)) = mem[++r.SP];
    } else {
        fail();
    }
    r.PC += 1;
}

void pop_psw(void) 
{
/* +-+-+-+-+-+-+-+-+
 * |1|1|1|1|0|0|0|1|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    mem[--r.SP] = r.F;
    mem[--r.SP] = r.A;
    r.PC += 1;
}

void xchg(void) 
{
/* +-+-+-+-+-+-+-+-+
 * |1|1|1|0|1|0|1|1|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    r.H = r.H ^ r.D; 
    r.D = r.H ^ r.D;
    r.H = r.H ^ r.D;

    r.L = r.L ^ r.E; 
    r.E = r.L ^ r.E;
    r.L = r.L ^ r.E;

    r.PC += 1;
}

void xthl(void) 
{
/* +-+-+-+-+-+-+-+-+
 * |1|1|1|0|0|0|1|1|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    r.L = r.L ^ mem[r.SP]; 
    mem[r.SP] = r.L ^ mem[r.SP];
    r.L = r.L ^ mem[r.SP];

    r.SP++; 

    r.H = r.H ^ mem[r.SP]; 
    mem[r.SP] = r.H ^ mem[r.SP];
    r.H = r.H ^ mem[r.SP];

    r.PC += 1;
}

void sphl(void) 
{
/* +-+-+-+-+-+-+-+-+
 * |1|1|1|1|1|0|0|1|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    r.SP = ADDR(r.L, r.H);

    r.PC += 1;
}

void pchl(void) 
{
/* +-+-+-+-+-+-+-+-+
 * |1|1|1|0|1|0|0|1|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    r.PC = ADDR(r.L, r.H);
}
