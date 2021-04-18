#include "stoc.h"

#define REG_A 0x01
#define REG_X 0x02
#define REG_Y 0x04
#define REG_S 0x08

void reg_rand_in(context_t * c);
void reg_rand_out(context_t * c);
void reg_in(int reg);
void reg_out(int reg);
int reg_cmp_out(context_t * a, context_t * b);
