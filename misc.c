#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "misc.h"
#include "memory.h"

char *itoab8(uint8_t x)
{
    static char result[9];
    int i;
    for(i = 0; i <= 7; i++) {
        result[7-i] = ((x >> i) & 0x01)?'1':'0';
    }
    result[8] = '\0';
    return result;
}

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

/* return address of register specified by its 3-bit code */
uint8_t *get_reg(uint8_t code) 
{
    switch(code) {
        case REG_B: { return &(r.B); } break;
        case REG_C: { return &(r.C); } break;
        case REG_D: { return &(r.D); } break;
        case REG_E: { return &(r.E); } break;
        case REG_H: { return &(r.H); } break;
        case REG_L: { return &(r.L); } break;
                    /* pseudo-register M = memory cell addressed by H & L registers */
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
