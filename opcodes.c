#include <stdint.h>
#include "opcodes.h"
#include "memory.h"
#include "misc.h"


/* ================= CARRY BIT INSTRUCTIONS ========================= */

void ctc(void)
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|1|1|1|1|1|1|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    if(F_CARRY(r.F)) RESET_Tc(r.F);
    else SET_Tc(r.F);
}

void stc(void)
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|1|1|0|1|1|1|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    SET_Tc(r.F);
}

/* =============== SINGLE REGISTER INSTRUCTIONS ===================== */

void inr(uint8_t Ri)
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|i|i|i|1|0|0|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */

    (*get_reg(Ri))++;

    if(*get_reg(Ri) == 0x10) SET_Th(r.F);
    else RESET_Th(r.F);

    set_szp_flags(*get_reg(Ri), &r.F);
}

void dcr(uint8_t Ri)
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|i|i|i|1|0|1|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */

    (*get_reg(Ri))--;

    if(*get_reg(Ri) == 0x0f) SET_Th(r.F);
    else RESET_Th(r.F);

    set_szp_flags(*get_reg(Ri), &r.F);
}

void cma(void)
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|1|0|1|1|1|1|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */

    r.A = ~r.A;
}

void daa(void)
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|1|0|0|1|1|1|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    if(LO(r.A) > 9 || F_HALF_CARRY(r.F)) {
        if(LO(r.A) > 9) SET_Th(r.F);
        else RESET_Th(r.F);
        r.A += 6;
    }
    if(HI(r.A) > 9 || F_CARRY(r.F)) {
        if(HI(r.A) > 9) SET_Tc(r.F);
        r.A = MERGE(LO(r.A), (HI(r.A) + 6) & 0x0f); 
    }
    set_szp_flags(r.A, &r.F);
}

/* ================= NOP INSTRUCTIONS ================================ */

void nop(void)
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|0|0|0|0|0|0|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */

    /* do nothing */ 
}
/* ================= DATA TRANSFER INSTRUCTIONS========================= */

void mov(uint8_t Ri, uint8_t Rj) 
{
    /* +-+-+-+-+-+-+-+-+
     * |0|1|i|i|i|j|j|j|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    if(Ri == REG_M && Rj == REG_M) fail();

    *(get_reg(Ri)) = *(get_reg(Rj));
    r.PC += 1;
}

void stax(uint8_t Ri) {
    /* +-+-+-+-+-+-+-+-+             +-+-+-+-+-+-+-+-+
     * |0|0|0|0|0|0|1|0| (STAX B) or |0|0|0|1|0|0|1|0| (STAX D)
     * +-+-+-+-+-+-+-+-+             +-+-+-+-+-+-+-+-+
     * 7               0             7               0
     */
    if(Ri == REG_B) {
        mem[MERGE_BC] = r.A;
    } else if (Ri == REG_D) {
        mem[MERGE_DE] = r.A;
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
        r.A = mem[MERGE_BC];
    } else if (Ri == REG_D) {
        r.A = mem[MERGE_DE];
    } else {
        fail();
    }
    r.PC += 1;
}

/* ========== REGISTER OR MEMORY TO ACCUMULATOR INSTRUCTIONS =========== */

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

    /* XXX 
     * Note that official Intel 8080 Programmers Manual says that
     * ANA doesn't affect the half-carry bit, but Soviet's i8080 clone,
     * КР580ВМ80А, sets half-carry bit according 
     * fourth bit of accumulator register.
     * This note can be also apllied to all AND/XOR instructions
     * XXX*/
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
    RESET_Th(r.F);
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

/* =================== ROTATE ACCUMULATOR INSTRUCTIONS  =================== */

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

/* =================== REGISTER PAIR INSTRUCTIONS ======================== */

/* TODO process stack overflow and underflow during stack operations */

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
        *(get_reg(Ri+1)) = mem[r.SP++];
        *(get_reg(Ri)) = mem[r.SP++];
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
    r.F = mem[r.SP++];
    r.A = mem[r.SP++];
    r.PC += 1;
}

void dad(uint8_t Ri) 
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|i|i|1|0|0|1|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */

    uint16_t a;
    uint16_t b = MERGE_HL;

    if(Ri == 0) a = MERGE_BC;
    if(Ri == 1) a = MERGE_DE;
    if(Ri == 2) a = MERGE_HL;
    if(Ri == 3) a = r.SP;

    uint8_t carry = (a > 0xffff - b);
    if(carry) SET_Tc(r.F);
    else RESET_Tc(r.F);

    b += a;
    
    r.L = LO(b);
    r.H = HI(b);
}

void inx(uint8_t Ri) 
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|i|i|0|0|1|1|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */

    uint16_t a;

    if(Ri == 0) a = MERGE_BC;
    if(Ri == 1) a = MERGE_DE;
    if(Ri == 2) a = MERGE_HL;
    if(Ri == 3) a = r.SP;

    a++;
    
    r.L = LO(a);
    r.H = HI(a);
}

void dcx(uint8_t Ri) 
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|i|i|1|0|1|1|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */

    uint16_t a;

    if(Ri == 0) a = MERGE_BC;
    if(Ri == 1) a = MERGE_DE;
    if(Ri == 2) a = MERGE_HL;
    if(Ri == 3) a = r.SP;

    a--;
    
    r.L = LO(a);
    r.H = HI(a);
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
    r.SP = MERGE(r.L, r.H);

    r.PC += 1;
}

/* ======================== IMMEDIATE INSTRUCTIONS ======================== */

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
     * |0|0|i|i|0|0|0|1|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| LSB
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| MSB
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    if(Ri == 0) {
      r.B = data_h;
      r.C = data_l;
    } 
    if(Ri == 1) {
      r.D = data_h;
      r.E = data_l;
    }
    if(Ri == 2) {
      r.H = data_h;
      r.L = data_l;
    }
    if(Ri == 3) {
      r.SP = MERGE(data_l, data_h);
    }

    r.PC += 3;
}

void adi(uint8_t data)
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|0|0|0|1|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    uint8_t carry = (r.A > 0xff - data);
    if(carry) SET_Tc(r.F);
    else RESET_Tc(r.F);

    uint8_t half_carry = ((r.A & 0x0f) > 0x0f - ((data) & 0x0f));
    if(half_carry) SET_Th(r.F);
    else RESET_Th(r.F);

    r.A = r.A + data;

    set_szp_flags(r.A, &r.F);
}

void aci(uint8_t data)
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|0|0|1|1|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    uint8_t carry_bit = F_CARRY(r.F);
    uint8_t carry = (r.A > 0xff - carry_bit - data);
    uint8_t half_carry = ((r.A & 0x0f) > 0x0f - carry_bit - (data & 0x0f));

    if(carry) SET_Tc(r.F);
    else RESET_Tc(r.F);
    if(half_carry) SET_Th(r.F);
    else RESET_Th(r.F);

    r.A = r.A + carry_bit + data;

    set_szp_flags(r.A, &r.F);
}

void sui(uint8_t data)
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|0|1|0|1|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    uint8_t comp2 = COMP2(data);
    uint8_t carry = (r.A > 0xff - comp2);
    /* carry bit must be set if there were NO borrow ocurred,
     * i.e. there were carry after adding two-complement number */
    if(carry) SET_Tc(r.F);
    else RESET_Tc(r.F);

    /* uint8_t half_carry = ((r.A & 0x0f) >= 0x0f - (comp2 & 0x0f)); */
    uint8_t half_carry = ((r.A & 0x0f) < (data & 0x0f));
    if(half_carry) SET_Th(r.F);
    else RESET_Th(r.F);

    r.A = r.A + comp2;

    set_szp_flags(r.A, &r.F);
}

void sbi(uint8_t data) 
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|0|1|1|1|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    uint8_t carry_bit = F_CARRY(r.F);
    uint8_t comp2 = COMP2(data+carry_bit);
    uint8_t carry = (r.A > 0xff - comp2);
    /* carry bit must be set if there were NO borrow ocurred,
     * i.e. there were carry after adding two-complement number */
    if(carry) SET_Tc(r.F);
    else RESET_Tc(r.F);

    /* uint8_t half_carry = ((r.A & 0x0f) >= 0x0f - (comp2 & 0x0f)); */
    uint8_t half_carry = ((r.A & 0x0f) < ((data+carry_bit) & 0x0f));
    if(half_carry) SET_Th(r.F);
    else RESET_Th(r.F);

    r.A = r.A + comp2;

    set_szp_flags(r.A, &r.F);
}

void ani(uint8_t data) 
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|1|0|0|1|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    r.A = r.A & data;
    RESET_Tc(r.F);
    if(r.A & 0x08) SET_Th(r.F);
    else RESET_Th(r.F);
    set_szp_flags(r.A, &r.F);
}

void xri(uint8_t data) 
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|1|0|1|1|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    r.A = r.A ^ data;

    RESET_Tc(r.F);
    if(r.A & 0x08) SET_Th(r.F);
    else RESET_Th(r.F);
    set_szp_flags(r.A, &r.F);
}

void ori(uint8_t data) 
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|1|1|0|1|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    r.A = r.A | data;

    RESET_Tc(r.F);
    if(r.A & 0x08) SET_Th(r.F);
    else RESET_Th(r.F);
    set_szp_flags(r.A, &r.F);
}

void cpi(uint8_t data) 
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|1|1|1|1|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    /* carry bit is set if Ri is greater than accumulator */
    if(data > r.A) SET_Tc(r.F);
    else RESET_Tc(r.F);

    /* half-carry and SZP flags are set as it would be 'sub Ri' operation */
    uint8_t half_carry = ((r.A & 0x0f) < (data & 0x0f));
    if(half_carry) SET_Th(r.F);
    else RESET_Th(r.F);

    set_szp_flags(r.A - data, &r.F);
}

/* ====================== DIRECT ADDRESSING INSTRUCTIONS ================== */

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
    mem[MERGE(addr_l, addr_h)] = r.A;
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
    r.A = mem[MERGE(addr_l, addr_h)];
    r.PC += 3;
}

void shld(uint8_t addr_l, uint8_t addr_h)
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|1|0|0|0|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| LSB = addr_l
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| MSB = addr_h
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    mem[MERGE(addr_l, addr_h)] = r.L;
    mem[MERGE(addr_l+1, addr_h)] = r.H;
    r.PC += 3;
}

void lhld(uint8_t addr_l, uint8_t addr_h)
{
    /* +-+-+-+-+-+-+-+-+
     * |0|0|1|0|0|0|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| LSB = addr_l
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| MSB = addr_h
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    r.L = mem[MERGE(addr_l, addr_h)];
    r.H = mem[MERGE(addr_l+1, addr_h)];
    r.PC += 3;
}

/* ========================== JUMP INSTRUCTIONS ========================== */

void pchl(void) 
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|1|0|1|0|0|1|
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    r.PC = MERGE(r.L, r.H);
}

void jmp(uint8_t addr_l, uint8_t addr_h)
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|0|0|0|0|1|1|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| LSB = addr_l
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| MSB = addr_h
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    r.PC = MERGE(addr_l, addr_h);
}

void jnc(uint8_t addr_l, uint8_t addr_h)
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|0|1|0|0|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| LSB = addr_l
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| MSB = addr_h
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    if(F_CARRY) r.PC = MERGE(addr_l, addr_h);
}

void jz(uint8_t addr_l, uint8_t addr_h)
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|0|0|1|0|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| LSB = addr_l
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| MSB = addr_h
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    if(F_ZERO) r.PC = MERGE(addr_l, addr_h);
}

void jnz(uint8_t addr_l, uint8_t addr_h)
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|0|0|0|0|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| LSB = addr_l
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| MSB = addr_h
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    if(!F_ZERO) r.PC = MERGE(addr_l, addr_h);
}

void jm(uint8_t addr_l, uint8_t addr_h)
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|1|1|0|0|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| LSB = addr_l
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| MSB = addr_h
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    if(F_SIGN) r.PC = MERGE(addr_l, addr_h);
}

void jp(uint8_t addr_l, uint8_t addr_h)
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|1|0|1|0|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| LSB = addr_l
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| MSB = addr_h
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    if(!F_SIGN) r.PC = MERGE(addr_l, addr_h);
}

void jpo(uint8_t addr_l, uint8_t addr_h)
{
    /* +-+-+-+-+-+-+-+-+
     * |1|1|1|0|0|0|1|0|
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| LSB = addr_l
     * +-+-+-+-+-+-+-+-+
     * |d|d|d|d|d|d|d|d| MSB = addr_h
     * +-+-+-+-+-+-+-+-+
     * 7               0
     */
    if(!F_PARITY) r.PC = MERGE(addr_l, addr_h);
}
