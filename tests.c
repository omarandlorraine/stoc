#include "tests.h"
#include "decl.h"
#include "emulator.h"
#include "stoc.h"
#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>

// How many test cases should we keep in memory?
#define TESTCASE_NO 10000

// How big is each test case
#define TESTCASE_SZ 17

// How many times are we going to try throwing random numbers at the routine?
#define MAXITER 1000

// For how many cycles may the routine run?
#define COMPUTEBUDGET 10000

static int num_test_cases = 0;

uint8_t *testcases[TESTCASE_NO];

void install(stoc_t *c) {
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
    }
    r->blength = addr - r->org;
    r->end = r->org + r->blength;
}

int run(stoc_t *c) {
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

void print_test_case(uint8_t *test, size_t length) {
    printf("testcase ");
    for (int i = 0; i < length; i++) {
        printf("$%02x ", test[i]);
    }

    printf("\n");
}

int run_test_case(uint8_t *test, stoc_t *rewrite) {
    decl_t *d = rewrite->decl;
    while (d) {
        if (d->fn(rewrite, d, &test))
            return 1;
        d = d->next;
    }
    return 0;
}

int create_test_case(stoc_t *reference, stoc_t *rewrite) {
    uint8_t *tc = malloc(TESTCASE_SZ);

    uint8_t *test = tc;
    decl_t *d = reference->decl;
    while (d) {
        d->setup(reference, d, &test);
        d = d->next;
    }

    if (num_test_cases < TESTCASE_NO)
        testcases[num_test_cases++] = tc;
    // print_test_case(tc, test - tc);
    return run_test_case(tc, rewrite);
}

int equivalence(stoc_t *reference, stoc_t *rewrite) {
    install(reference);
    install(rewrite);

    rewrite->clockticks = 0;

    int i;
    for (i = 0; i < num_test_cases; i++) {
        if (run_test_case(testcases[i], rewrite)) {
            return 0;
        }
    }
    return !create_test_case(reference, rewrite);
}

void measure(stoc_t *c) {
    install(c);
    if (!num_test_cases) {
        uint8_t *tc = malloc(TESTCASE_SZ);

        uint8_t *test = tc;
        decl_t *d = c->decl;
        while (d) {
            d->setup(c, d, &test);
            d = d->next;
        }

        if (num_test_cases < TESTCASE_NO)
            testcases[num_test_cases++] = tc;
    }

    c->clockticks = 0;
    c->exitcode = 0;

    int i;
    for (i = 0; i < num_test_cases; i++) {
        if (run_test_case(testcases[i], c))
            break;
    }
    c->clockticks /= (i + 1);
}
