#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <strings.h>

/*
#include "misc.h"
#include "opcodes.h"
*/

#define REG_A 7
#define REG_B 0
#define REG_C 1
#define REG_D 2
#define REG_E 3
#define REG_H 4
#define REG_L 5
#define REG_M 6

#define ADDR_BC         ((r.B << 8) + r.C)
#define ADDR_DE         ((r.D << 8) + r.E)
#define ADDR_HL         ((r.H << 8) + r.L)
#define ADDR(LO, HI)    (((HI) << 8) + (LO))

#define IS_BDH(x) ((x) == REG_B || (x) == REG_D || (x) == REG_H)

#define SET_Ts(x)   ((x) |= 0x80) 
#define RESET_Ts(x) ((x) &= ~0x80) 
#define F_SIGN(x)   ( ((x) & 0x80) >> 7 )

#define SET_Tz(x)   ((x) |= 0x40) 
#define RESET_Tz(x) ((x) &= ~0x40) 
#define F_ZERO(x)   ( ((x) & 0x40) >> 6 )

#define SET_Th(x)   ((x) |= 0x10) 
#define RESET_Th(x) ((x) &= ~0x10) 
#define F_HALF_CARRY(x)   ( ((x) & 0x10) >> 4 )

#define SET_Tp(x)   ((x) |= 0x04) 
#define RESET_Tp(x) ((x) &= ~0x04) 
#define F_PARITY(x)   ( ((x) & 0x04) >> 2 )

#define SET_Tc(x)   ((x) |= 0x01) 
#define RESET_Tc(x) ((x) &= ~0x01) 
#define F_CARRY(x)   ( ((x) & 0x01) )

#define COMP2(x)    (~(x)+1)

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

void bdump8(uint8_t x)
{
    int i;
    printf("%3u <", (unsigned int)x);
    for(i = 7; i >= 0; i--) {
        if (!((i+1)%4) && i != 7) printf(" ");
        printf("%u", (x >> i) & 0x01);
    }
    printf(">\n");
}

void bdump16(uint16_t x)
{
    int i;
    printf("%6u <", (unsigned int)x);
    for(i = 15; i >= 0; i--) {
        if (!((i+1)%4) && i != 15) printf(" ");
        printf("%u", (x >> i) & 0x01);
    }
    printf(">\n");
}

void dump_flags(uint8_t f)
{
    printf("Flags register: "); bdump8(f);
    printf("Carry:      %s\n", F_CARRY(f)?"set":"unset");
    printf("Aux carry:  %s\n", F_HALF_CARRY(f)?"set":"unset");
    printf("Sign:       %s\n", F_SIGN(f)?"set":"unset");
    printf("Parity:     %s\n", F_PARITY(f)?"set":"unset");
    printf("Zero:       %s\n", F_ZERO(f)?"set":"unset");
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

void set_szp_flags(uint8_t reg, uint8_t *p_flags) 
{
    uint8_t flags = *p_flags;

    if(reg & 0x80) SET_Ts(flags);
    else RESET_Ts(flags);

    if(reg == 0) SET_Tz(flags);
    else RESET_Tz(flags);

    uint8_t parity = 1, i;
    for(i = 0; i <= 7; i++)
        if((reg >> i) & 0x01) parity = !parity;
    if(parity) SET_Tp(flags);
    else RESET_Tp(flags);

    *p_flags = flags;
}

/* ================= LOAD/STORE/MOVE ================================ */

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


/* ================= ARITHMETIC AND LOGIC ================================ */

void add(uint8_t Ri) 
{
/* +-+-+-+-+-+-+-+-+
 * |1|0|0|0|0|i|i|i|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    uint8_t carry = (r.A > 0xff - *get_reg(Ri));
    if(carry) SET_Tc(r.F);
    else RESET_Tc(r.F);

    uint8_t half_carry = ((r.A & 0x0f) > 0x0f - ((*get_reg(Ri)) & 0x0f));
    if(half_carry) SET_Th(r.F);
    else RESET_Th(r.F);

    r.A = r.A + *get_reg(Ri);

    set_szp_flags(r.A, &r.F);

}

void adc(uint8_t Ri) 
{
/* +-+-+-+-+-+-+-+-+
 * |1|0|0|0|1|i|i|i|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    uint8_t carry_bit = F_CARRY(r.F);
    uint8_t carry = (r.A > 0xff - carry_bit - *get_reg(Ri));
    uint8_t half_carry = ((r.A & 0x0f) > 0x0f - carry_bit -
                            ((*get_reg(Ri)) & 0x0f));
    if(carry) SET_Tc(r.F);
    else RESET_Tc(r.F);
    if(half_carry) SET_Th(r.F);
    else RESET_Th(r.F);

    r.A = r.A + carry_bit + *get_reg(Ri);

    set_szp_flags(r.A, &r.F);

}

void sub(uint8_t Ri) 
{
/* +-+-+-+-+-+-+-+-+
 * |1|0|0|1|0|i|i|i|
 * +-+-+-+-+-+-+-+-+
 * 7               0
 */
    uint8_t comp2 = COMP2(*get_reg(Ri));
    uint8_t carry = (r.A > 0xff - comp2);
    /* carry bit must be set if there were NO borrow ocurred,
     * i.e. there were carry after adding two-complement number */
    if(carry) SET_Tc(r.F);
    else RESET_Tc(r.F);

    uint8_t half_carry = ((r.A & 0x0f) > 0x0f - (comp2 & 0x0f));
    /* half carry bit must be set if there borrow occured,
     * i.e. there were NO carry out in low tetrad after adding
     * two-complement number */ 
    if(!half_carry) SET_Th(r.F);
    else RESET_Th(r.F);

    r.A = r.A + comp2;

    set_szp_flags(r.A, &r.F);

}

void test_add(void)
{
    for(;;) { 
      
        r.A = rand()%256;
        r.B = rand()%256;
        printf("%hhu + %hhu = %hhu\n", r.A, r.B, (uint8_t)(r.A + r.B));
        bdump8(r.A);
        bdump8(r.B);
        add(REG_B);
        printf("-----------\n");
        bdump8(r.A);
        dump_flags(r.F);
        printf("\n=======================================\n");
    }
}


void test_sub(void)
{
    for(;;) { 
        r.A = rand()%256;
        r.B = rand()%256;
        printf("%hhu - %hhu = %hhu\n", r.A, r.B, (uint8_t)(r.A - r.B));
        bdump8(r.A);
        bdump8(r.B);
        sub(REG_B);
        printf("-----------\n");
        bdump8(r.A);
        dump_flags(r.F);
        printf("\n=======================================\n");
    }
}

int main (int argc, char *argv[])
{
    bzero(&r, sizeof(r));
    bzero(mem, sizeof(mem));
    r.F = 0x02;
    
    test_sub();

    printf("ok\n");
    return 0;
}
