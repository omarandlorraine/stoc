#include "search.h"
#include "arch.h"
#include "optimization.h"
#include "pick.h"
#include "stoc.h"
#include "tests.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static void randomise_instruction(rewrite_t *p, instruction_t *i) {
    do {
        randomise_opcode(i);
    } while (!randomise_operand(p, i));
}

static void remove_instr(stoc_t *proposal) {
    if (proposal->program.length < 2)
        return;
    int offs = rand() % proposal->program.length;
    for (int i = offs; i < proposal->program.length - 1; i++) {
        proposal->program.instructions[i] =
            proposal->program.instructions[i + 1];
    }
    proposal->program.length--;
}

static void insert_instr(stoc_t *proposal) {
    int rnd = rand();
    int offs = rnd % (proposal->program.length);
    proposal->program.length++;
    for (int i = proposal->program.length; i > offs; i--) {
        proposal->program.instructions[i] =
            proposal->program.instructions[i - 1];
    }
    randomise_instruction(&proposal->program,
                          &proposal->program.instructions[offs]);
}

static void modify_operand(stoc_t *proposal) {
    if (proposal->program.length == 0)
        return;
    int rnd = rand();
    int offs = rnd % (proposal->program.length);
    if (!randomise_operand(&proposal->program,
                           &proposal->program.instructions[offs]))
        exit(printf("Could not modify operand, opcode %02x, for instruction "
                    "number %d, see search.c line %d\n",
                    proposal->program.instructions[offs].opcode, offs,
                    __LINE__));
}

static void modify_opcode(stoc_t *proposal) {
    if (proposal->program.length == 0)
        return;
    int offs = rand() % (proposal->program.length);
    mutate_opcode(&proposal->program.instructions[offs]);
}

static void replace_instr(stoc_t *proposal) {
    if (proposal->program.length == 0)
        return;

    int offs = rand() % (proposal->program.length);
    do {
        randomise_opcode(&proposal->program.instructions[offs]);
    } while (!randomise_operand(&proposal->program,
                                &proposal->program.instructions[offs]));
}

static void swap_instrs(stoc_t *proposal) {
    if (proposal->program.length < 2)
        return;
    int offs1 = rand() % (proposal->program.length);
    instruction_t temp = proposal->program.instructions[offs1];
    int offs2 = rand() % (proposal->program.length);
    proposal->program.instructions[offs1] =
        proposal->program.instructions[offs2];
    proposal->program.instructions[offs2] = temp;
}

static void random_mutation(stoc_t *proposal) {
    int r = rand() % 6;
    switch (r) {
    case 0:
        remove_instr(proposal);
        break;
    case 1:
        modify_opcode(proposal);
        break;
    case 2:
        modify_operand(proposal);
        break;
    case 3:
        swap_instrs(proposal);
        break;
    case 4:
        replace_instr(proposal);
        break;
    case 5:
        insert_instr(proposal);
        break;
    }
}

static bool iterate(stoc_t *reference, stoc_t *rewrite, stoc_t *proposal) {
    random_mutation(proposal);

    if (!equivalence(reference, proposal))
        return false;

    measure(proposal);
    measure(rewrite);
    if (compare(proposal, rewrite) < 0)
        return true;

    return false;
}

void stoc_opt(stoc_t *reference) {
    // We're going to try millions of random mutations until we find a program
    // that's both equivalent and more optimal
    measure(reference);
    stoc_t rewrite = *reference;
    stoc_t proposal;
    arch_init(&rewrite);
    arch_init(&proposal);
    proposal.decl = rewrite.decl;

    for (int i = 0; i < 1000; i++) {
        proposal.program = rewrite.program;
        for (int j = 0; j < 100; j++) {
            if (iterate(reference, &rewrite, &proposal)) {
                rewrite.program = proposal.program;
                i = 0;
                // hexdump(&rewrite);
            }
        }
    }
    hexdump(&rewrite);
    arch_deinit(&rewrite);
    arch_deinit(&proposal);
}

bool exhsearch(stoc_t *reference, stoc_t *rewrite, int i) {
    if (i < 0) {
        // End of this branch
        if (equivalence(reference, rewrite)) {
            hexdump(rewrite);
            return true;
        }
        return false;
    }

    iterator_t opciter;
    iterator_t operiter;

    uint16_t opcode;

    for (iterator_init(&mode_implied, &opciter);
         pick_iterate(&opciter, &opcode);) {
        rewrite->program.instructions[i].opcode = opcode;
        if (exhsearch(reference, rewrite, i - 1))
            return true;
    }

    for (iterator_init(&mode_immediate, &opciter);
         pick_iterate(&opciter, &opcode);) {
        rewrite->program.instructions[i].opcode = opcode;
        for (iterator_init(&constants, &operiter); pick_iterate(
                 &operiter, &rewrite->program.instructions[i].operand);)
            if (exhsearch(reference, rewrite, i - 1))
                return true;
    }

    // TODO: Add other kinds of instructions here.
    return false;
}

void stoc_exh(stoc_t *reference) {
    // Exhaustive search for equivalent program
    stoc_t rewrite = *reference;
    arch_init(&rewrite);

    for (int i = 0; i < 10; i++) {
        rewrite.program.length = i;
        if (exhsearch(reference, &rewrite, i - 1))
            break;
    }
    arch_deinit(&rewrite);
}

void deadcodeelim(stoc_t *reference) {
    // One common and simple but effective optimisation strategy is dead code
    // elimination. It removes instructions which are found to be ineffectual.
    stoc_t rewrite;
    stoc_t proposal;

    rewrite = *reference;

    for (int i = 0; i < 1000; i++) {
        proposal = rewrite;

        for (int j = 0; j < 5; j++) {
            remove_instr(&proposal);
            if (!proposal.program.length)
                break;
            if (equivalence(&rewrite, &proposal)) {
                rewrite = proposal;
            }
        }
    }
    hexdump(&rewrite);
    arch_deinit(&rewrite);
    arch_deinit(&proposal);
}

void search_init() { archsearch_init(); }
