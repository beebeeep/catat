#ifndef OPCODES_H
#define OPCODES_H

#include <stdint.h>

/* ================= LOAD/STORE/MOVE ================================ */

void mov(uint8_t Ri, uint8_t Rj);
void mvi(uint8_t Ri, uint8_t data); 
void lxi(uint8_t Ri, uint8_t data_l, uint8_t data_h);
void stax(uint8_t Ri);
void ldax(uint8_t Ri);
void sta(uint8_t addr_l, uint8_t addr_h);
void lda(uint8_t addr_l, uint8_t addr_h);
void shld(uint8_t addr_l, uint8_t addr_h);
void lhld(uint8_t addr_l, uint8_t addr_h);
void push(uint8_t Ri);
void push_psw(void);
void pop(uint8_t Ri);
void pop_psw(void);
void xchg(void);
void xthl(void);
void sphl(void);
void pchl(void);

/* ================= ARITHMETIC AND LOGIC ================================ */

void add(uint8_t Ri);
void adc(uint8_t Ri);
void sub(uint8_t Ri);
void sbb(uint8_t Ri);
void ana(uint8_t Ri);
void xra(uint8_t Ri);
void ora(uint8_t Ri);
void cmp(uint8_t Ri);
void rlc(void);
void rrc(void);
void ral(void);
void rar(void);

#endif
