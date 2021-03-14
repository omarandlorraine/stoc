#include "reg.h"
#include "stoc.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

#define REG_A 0x01
#define REG_X 0x02
#define REG_Y 0x04
#define REG_S 0x08

/*
 * Part of the live-variable analysis
 *
 * You can declare a register as being live-in and/or live-out, information
 * used by the testers to compare two routines for equivalence.
 */

uint8_t live_in = 0;
uint8_t live_out = 0;

int reg_cmp(context_t *a, context_t *b, uint8_t live) {
    int score = 0;
    if (live & REG_A)
        score += __builtin_popcount(a->a ^ b->a);
    if (live & REG_X)
        score += __builtin_popcount(a->x ^ b->x);
    if (live & REG_Y)
        score += __builtin_popcount(a->y ^ b->y);
    if (live & REG_S)
        score += __builtin_popcount(a->s ^ b->s);
    return score;
}

void reg_rand(context_t *c, uint8_t live) {
    if (live & REG_A)
        c->a = rand();
    if (live & REG_X)
        c->x = rand();
    if (live & REG_Y)
        c->y = rand();
}

void reg_out(char *s) {
    while (*s) {
        if (*s == 'a')
            live_out |= REG_A;
        else if (*s == 'x')
            live_out |= REG_X;
        else if (*s == 'y')
            live_out |= REG_Y;
        else if (*s == '\n')
            (void)0;
        else {
            fprintf(stderr, "unknown register %s\n", s);
            exit(1);
        }
        s++;
    }
}

void reg_in(char *s) {
    while (*s) {
        if (*s == 'a')
            live_in |= REG_A;
        else if (*s == 'x')
            live_in |= REG_X;
        else if (*s == 'y')
            live_in |= REG_Y;
        else {
            fprintf(stderr, "unknown register %s\n", s);
            exit(1);
        }
        s++;
    }
}

void reg_rand_in(context_t *c) { reg_rand(c, live_in); }

void reg_rand_out(context_t *c) { reg_rand(c, live_in | live_out); }

int reg_cmp_out(context_t *a, context_t *b) { return reg_cmp(a, b, live_out); }
