#ifndef MISC_H
#define MISC_H

#include <stdint.h>

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

char *itoab8(uint8_t x);

void bdump8(uint8_t x);

void bdump16(uint16_t x);

void dump_flags(uint8_t f);

void fail(void);

uint8_t *get_reg(uint8_t code);

void set_szp_flags(uint8_t reg, uint8_t *p_flags);

#endif
