#include "tests.h"
#include "arch.h"
#include "decl.h"
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
static bool tcout = false;

uint8_t *testcases[TESTCASE_NO];

void cmdlin_tcout() {
	tcout = true;
}

void print_test_case(uint8_t *test, size_t length) {
	if(!tcout)
		return;
    printf("testcase ");
    for (int i = 0; i < length; i++) {
        printf("$%02x ", test[i]);
    }

    printf("\n");
}

bool run_test_case(uint8_t *test, stoc_t *rewrite) {
    install(rewrite);
    decl_t *d = rewrite->decl;
    while (d) {
        if (d->fn(rewrite, d, &test))
            return true;
        d = d->next;
    }
    return false;
}

bool create_test_case(stoc_t *reference, stoc_t *rewrite) {
    uint8_t *tc = malloc(TESTCASE_SZ);

    uint8_t *test = tc;
    decl_t *d = reference->decl;
    while (d) {
        d->setup(reference, d, &test);
        d = d->next;
    }

    if (num_test_cases < TESTCASE_NO)
        testcases[num_test_cases++] = tc;
    print_test_case(tc, test - tc);
    return run_test_case(tc, rewrite);
}

bool equivalence(stoc_t *reference, stoc_t *rewrite) {
    rewrite->clockticks = 0;

    int i;
    for (i = 0; i < num_test_cases; i++) {
        if (run_test_case(testcases[i], rewrite)) {
            return false;
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

    int ticks = 0;
    int count = 1;
    for (int i = 0; i < num_test_cases; i++) {
        if (!run_test_case(testcases[i], c)) {
            ticks += c->clockticks;
            count++;
        }
    }
    c->clockticks = ticks / count;
}

void testcase_deinit() {
    for (int i = 0; i < num_test_cases; i++)
        free(testcases[i]);
}
