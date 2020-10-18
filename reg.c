#include <inttypes.h>
#include <stdlib.h>
#include <stdio.h>
#include "stoc.h"
#include "reg.h"
#include "emulators/fake6502.h"

#define REG_A 0x01
#define REG_X 0x02
#define REG_Y 0x04

/*
 * Part of the live-variable analysis
 *
 * You can declare a register as being live-in and/or live-out, information
 * used by the testers to compare two routines for equivalence.
 */

uint8_t live_in = 0;
uint8_t live_out = 0;

int reg_cmp(Context65 * a, Context65 * b, uint8_t live) {
	if((live & REG_A) && (a->a != b->a)) return 1;
	if((live & REG_X) && (a->x != b->x)) return 1;
	if((live & REG_Y) && (a->y != b->y)) return 1;
	return 0;
}

void reg_rand(Context65 * c, uint8_t live) {
	if(live & REG_A) c->a = rand();
	if(live & REG_X) c->x = rand();
	if(live & REG_Y) c->y = rand();
}

void reg_out(char * s) {
	while(*s) {
		if(*s == 'a') live_out |= REG_A;
		else if(*s == 'x') live_out |= REG_X;
		else if(*s == 'y') live_out |= REG_Y;
		else if(*s == '\n') (void)0;
		else {
			fprintf(stderr, "unknown register %s\n", s);
			exit(1);
		}
		s++;
	}
}

void reg_in(char * s) {
	while(*s) {
		if(*s == 'a') live_in |= REG_A;
		else if(*s == 'x') live_in |= REG_X;
		else if(*s == 'y') live_in |= REG_Y;
		else {
			fprintf(stderr, "unknown register %s\n", s);
			exit(1);
		}
		s++;
	}
}

void reg_rand_in(Context65 * c) {
	reg_rand(c, live_in);
}

void reg_rand_out(Context65 * c) {
	reg_rand(c, live_in | live_out);
}

int reg_cmp_out(Context65 * a, Context65 * b) {
	return reg_cmp(a, b, live_out);
}
