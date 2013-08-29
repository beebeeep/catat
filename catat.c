#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <strings.h>


#include "misc.h"
#include "memory.h"
#include "opcodes.h"



void test_rlc(void)
{
  for(;;) {
    r.A = rand()%256; 
    bdump8(r.A);
    printf("Tc is %u\n", F_CARRY(r.F));
    rlc();
    bdump8(r.A);
    printf("Tc is %u\n=======\n", F_CARRY(r.F));

  }

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

  /* test_rlc(); */

  printf("%s\n", itoab8(0x80));
  printf("ok\n");
  return 0;
}
