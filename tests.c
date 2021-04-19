#include "tests.h"
#include "emulator.h"
#include "main.h"
#include "reg.h"
#include "stoc.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// How many times are we going to try throwing random numbers at the routine?
#define MAXITER 1000

// For how many cycles may the routine run?
#define COMPUTEBUDGET 10000

void install(context_t *c) {
    rewrite_t *r = &c->program;
    uint16_t addr = r->org;
    for (int i = 0; i < r->length; i++) {
        uint8_t instr = r->instructions[i].opcode;
        r->instructions[i].address = addr;
        mem_write(c, addr++, instr);
        if (opcode_length(instr) > 1)
            mem_write(c, addr++, r->instructions[i].operand & 0x00ff);
        if (opcode_length(instr) > 2)
            mem_write(c, addr++, r->instructions[i].operand >> 8);
        r->end = addr;
    }
    r->blength = addr - r->org;
}

int run(context_t *c) {
    addr_t org = c->program.org;
    c->pc = org;
    for (int i = 0; i < COMPUTEBUDGET; i++) {
        if (c->pc == c->program.end)
            return NORMAL_EXIT;
        if (c->pc < org)
            return PC_OUT_OF_BOUNDS;
        if (c->pc > c->program.end)
            return PC_OUT_OF_BOUNDS;
        step(c);
    }
    return TOOK_TOO_LONG;
}

void run_both(context_t *reference, context_t *rewrite) {
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t f;
    uint8_t s;

    a = rand();
    x = rand();
    y = rand();
    f = rand();
    s = rand();

    rewrite->a = reference->a = a;
    rewrite->x = reference->x = x;
    rewrite->y = reference->y = y;
    rewrite->s = reference->s = s;
    rewrite->flags = reference->flags = f;

    rewrite->exitcode |= run(rewrite);
    reference->exitcode |= run(reference);

    rewrite->hamming += reg_cmp_out(reference, rewrite);
}

void test_both(context_t *reference, context_t *rewrite_a,
               context_t *rewrite_b) {
    uint8_t a;
    uint8_t x;
    uint8_t y;
    uint8_t f;
    uint8_t s;

    a = rand();
    x = rand();
    y = rand();
    f = rand();
    s = rand();

    rewrite_a->a = rewrite_b->a = reference->a = a;
    rewrite_a->x = rewrite_b->x = reference->x = x;
    rewrite_a->y = rewrite_b->y = reference->y = y;
    rewrite_a->s = rewrite_b->s = reference->s = s;
    rewrite_a->flags = rewrite_b->flags = reference->flags = f;

    rewrite_a->exitcode |= run(rewrite_a);
    rewrite_b->exitcode |= run(rewrite_b);
    reference->exitcode |= run(reference);

    rewrite_a->hamming += reg_cmp_out(reference, rewrite_a);
    rewrite_b->hamming += reg_cmp_out(reference, rewrite_b);
}

int equivalence(context_t *reference, context_t *rewrite, int log) {
    install(reference);
    install(rewrite);
    for (int i = 0; i < MAXITER; i++) {
        uint8_t a = rand();
        uint8_t x = rand();
        uint8_t y = rand();
        uint8_t f = rand();

        rewrite->a = reference->a = a;
        rewrite->x = reference->x = x;
        rewrite->y = reference->y = y;
        rewrite->flags = reference->flags = f;

        run_both(reference, rewrite);

        if (rewrite->hamming) {
            if (log--) {
                printf("a = %02x, x = %02x, y = %02x\n", a, x, y);
            } else {
                return 0;
            }
        }
    }
    rewrite->clockticks /= MAXITER;
    return 1;
}

void measure(context_t *reference, context_t *rewrite_a) {
    install(reference);
    install(rewrite_a);

    rewrite_a->clockticks = 0;
    reference->clockticks = 0;

    rewrite_a->exitcode = 0;
    reference->exitcode = 0;

    rewrite_a->hamming = 0;

    int i;
    for (i = 0; i < MAXITER; i++) {
        run_both(reference, rewrite_a);
    }
    reference->clockticks /= i;
    rewrite_a->clockticks /= i;
}

void measure_two(context_t *reference, context_t *rewrite_a,
                 context_t *rewrite_b) {
    install(reference);
    install(rewrite_a);
    install(rewrite_b);

    rewrite_a->clockticks = 0;
    rewrite_b->clockticks = 0;
    reference->clockticks = 0;

    rewrite_a->exitcode = 0;
    rewrite_b->exitcode = 0;
    reference->exitcode = 0;

    rewrite_a->hamming = 0;

    int i;
    for (i = 0; i < MAXITER; i++) {
        test_both(reference, rewrite_a, rewrite_b);
    }
    reference->clockticks /= i;
    rewrite_a->clockticks /= i;
    rewrite_b->clockticks /= i;
}
