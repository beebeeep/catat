#include <stdint.h>
#include "opcodes.h"
#include "memory.h"
#include "misc.h"

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

  /* uint8_t half_carry = ((r.A & 0x0f) >= 0x0f - (comp2 & 0x0f)); */
  uint8_t half_carry = ((r.A & 0x0f) < (*get_reg(Ri) & 0x0f));
  if(half_carry) SET_Th(r.F);
  else RESET_Th(r.F);

  r.A = r.A + comp2;

  set_szp_flags(r.A, &r.F);

}


void sbb(uint8_t Ri) 
{
  /* +-+-+-+-+-+-+-+-+
   * |1|0|0|1|1|i|i|i|
   * +-+-+-+-+-+-+-+-+
   * 7               0
   */
  uint8_t carry_bit = F_CARRY(r.F);
  uint8_t comp2 = COMP2(*get_reg(Ri)+carry_bit);
  uint8_t carry = (r.A > 0xff - comp2);
  /* carry bit must be set if there were NO borrow ocurred,
   * i.e. there were carry after adding two-complement number */
  if(carry) SET_Tc(r.F);
  else RESET_Tc(r.F);

  /* uint8_t half_carry = ((r.A & 0x0f) >= 0x0f - (comp2 & 0x0f)); */
  uint8_t half_carry = ((r.A & 0x0f) < ((*get_reg(Ri)+carry_bit) & 0x0f));
  if(half_carry) SET_Th(r.F);
  else RESET_Th(r.F);

  r.A = r.A + comp2;

  set_szp_flags(r.A, &r.F);
}

void ana(uint8_t Ri) 
{
  /* +-+-+-+-+-+-+-+-+
   * |1|0|1|0|0|i|i|i|
   * +-+-+-+-+-+-+-+-+
   * 7               0
   */
  r.A = r.A & *get_reg(Ri);
  RESET_Tc(r.F);
  if(r.A & 0x08) SET_Th(r.F);
  else RESET_Th(r.F);
  set_szp_flags(r.A, &r.F);
}

void xra(uint8_t Ri) 
{
  /* +-+-+-+-+-+-+-+-+
   * |1|0|1|0|1|i|i|i|
   * +-+-+-+-+-+-+-+-+
   * 7               0
   */
  r.A = r.A ^ *get_reg(Ri);
  RESET_Tc(r.F);
  if(r.A & 0x08) SET_Th(r.F);
  else RESET_Th(r.F);
  set_szp_flags(r.A, &r.F);
}

void ora(uint8_t Ri) 
{
  /* +-+-+-+-+-+-+-+-+
   * |1|0|1|1|0|i|i|i|
   * +-+-+-+-+-+-+-+-+
   * 7               0
   */
  r.A = r.A | *get_reg(Ri);
  RESET_Tc(r.F);
  RESET_Th(r.F);
  set_szp_flags(r.A, &r.F);
}

void cmp(uint8_t Ri) 
{
  /* +-+-+-+-+-+-+-+-+
   * |1|0|1|1|1|i|i|i|
   * +-+-+-+-+-+-+-+-+
   * 7               0
   */
  /* carry bit is set if Ri is greater than accumulator */
  if(*get_reg(Ri) > r.A) SET_Tc(r.F);
  else RESET_Tc(r.F);

  /* half-carry and SZP flags are set as it would be 'sub Ri' operation */
  uint8_t half_carry = ((r.A & 0x0f) < (*get_reg(Ri) & 0x0f));
  if(half_carry) SET_Th(r.F);
  else RESET_Th(r.F);

  set_szp_flags(r.A - *get_reg(Ri), &r.F);
}

void rlc(void) 
{
  /* +-+-+-+-+-+-+-+-+
   * |0|0|0|0|0|1|1|1|
   * +-+-+-+-+-+-+-+-+
   * 7               0
   */
  if(r.A & 0x80) SET_Tc(r.F);
  else RESET_Tc(r.F);

  r.A = r.A << 1 | (r.A >> 7);
}

void rrc(void) 
{
  /* +-+-+-+-+-+-+-+-+
   * |0|0|0|0|1|1|1|1|
   * +-+-+-+-+-+-+-+-+
   * 7               0
   */
  if(r.A & 0x80) SET_Tc(r.F);
  else RESET_Tc(r.F);

  r.A = r.A >> 1 | (r.A << 7);
}

void ral(void) 
{
  /* +-+-+-+-+-+-+-+-+
   * |0|0|0|1|0|1|1|1|
   * +-+-+-+-+-+-+-+-+
   * 7               0
   */
  uint8_t t = r.A >> 7;
  r.A = r.A << 1 | F_CARRY(r.F);
  if(t) SET_Tc(r.F);
  else RESET_Tc(r.F);
}

void rar(void) 
{
  /* +-+-+-+-+-+-+-+-+
   * |0|0|0|1|1|1|1|1|
   * +-+-+-+-+-+-+-+-+
   * 7               0
   */
  uint8_t t = r.A & 0x01;
  r.A = r.A >> 1 | (F_CARRY(r.F) << 7);
  if(t) SET_Tc(r.F);
  else RESET_Tc(r.F);
}
