#include "search.h"
#include "main.h"
#include "optimization.h"
#include "pick.h"
#include "stoc.h"
#include "tests.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static pick_t zp_addresses;

static uint8_t random_opcode() {
    uint8_t opcode;
retry:
    opcode = rand();
    if (!opcode_legal_p(opcode))
        goto retry;
    if (!addressing_modes[opcode])
        goto retry;
    if (addressing_modes[opcode] == &mode_relative)
        goto retry;
    return opcode;
}

static bool randomise_operand(rewrite_t *p, instruction_t *i) {
    // TODO: Can we think of a more sensible solution here?
    pick_t *mode = addressing_modes[i->opcode];
    if (!mode)
        return false;

    if (mode == &mode_implied)
        return true;

    if (mode == &mode_immediate)
        return random_constant(&i->operand);

    if (mode == &mode_indirect_x)
        return pick_at_random(&zp_addresses, &i->operand);

    if (mode == &mode_indirect_y)
        return pick_at_random(&zp_addresses, &i->operand);

    if (mode == &mode_zero_page)
        return pick_at_random(&zp_addresses, &i->operand);

    if (mode == &mode_zero_page_x)
        return pick_at_random(&zp_addresses, &i->operand);

    if (mode == &mode_zero_page_y)
        return pick_at_random(&zp_addresses, &i->operand);

    if (mode == &mode_indirect)
        return random_address(&i->operand);

    if (mode == &mode_absolute)
        return random_address(&i->operand);

    if (mode == &mode_absolute_x)
        return random_address(&i->operand);

    if (mode == &mode_absolute_y)
        return random_address(&i->operand);

    if (mode == &mode_relative) {
        printf("relative instruction error\n");
        exit(1);
    }

    printf("unknown mode for %02x error\n", i->opcode);
    exit(1);
}

static void randomise_instruction(rewrite_t *p, instruction_t *i) {
    do {
        i->opcode = random_opcode();
    } while (!randomise_operand(p, i));
}

static void remove_instr(context_t *proposal) {
    if (proposal->program.length < 2)
        return;
    int offs = rand() % proposal->program.length;
    for (int i = offs; i < proposal->program.length - 1; i++) {
        proposal->program.instructions[i] =
            proposal->program.instructions[i + 1];
    }
    proposal->program.length--;
}

static void insert_instr(context_t *proposal) {
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

static void modify_operand(context_t *proposal) {
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

static void modify_opcode(context_t *proposal) {
    if (proposal->program.length == 0)
        return;
    int offs = rand() % (proposal->program.length);
    instruction_t i = proposal->program.instructions[offs];
    uint16_t opcode = i.opcode;
    pick_t *m = addressing_modes[opcode];
    pick_at_random(m, &opcode);
    i.opcode = opcode;
}

static void replace_instr(context_t *proposal) {
    if (proposal->program.length == 0)
        return;

    int offs = rand() % (proposal->program.length);
    do {
        proposal->program.instructions[offs].opcode = random_opcode();
    } while (!randomise_operand(&proposal->program,
                                &proposal->program.instructions[offs]));
}

static void swap_instrs(context_t *proposal) {
    if (proposal->program.length < 2)
        return;
    int offs1 = rand() % (proposal->program.length);
    instruction_t temp = proposal->program.instructions[offs1];
    int offs2 = rand() % (proposal->program.length);
    proposal->program.instructions[offs1] =
        proposal->program.instructions[offs2];
    proposal->program.instructions[offs2] = temp;
}

static bool checkem(rewrite_t *r) {
    if (r->length >= REWRITE_LEN) {
        // the rewrite has exceeded the maximum length
        return false;
    }

    for (int i = 0; i < r->length - 1; i++)
        if (!opcode_legal_p(r->instructions[i].opcode))
            return false;

    return true;
}

static void random_mutation(context_t *proposal) {
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

static bool iterate(context_t *reference, context_t *rewrite,
                    context_t *proposal) {
    random_mutation(proposal);

    if (!checkem(&proposal->program))
        return false;

    if (!equivalence(reference, proposal))
        return false;

    measure(proposal);
    if (compare(proposal, rewrite) < 0)
        return true;

    return false;
}

void stoc_opt(context_t *reference) {
    // We're going to try millions of random mutations until we find a program
    // that's both equivalent and more optimal
    measure(reference);
    context_t rewrite = *reference;
    context_t proposal;

    for (int i = 0; i < 100; i++) {
        proposal = rewrite;
        for (int j = 0; j < 100; j++) {
            if (iterate(reference, &rewrite, &proposal)) {
                rewrite = proposal;
                i = 0;
            }
        }
    }
    hexdump(&rewrite);
}

void stoc_gen(context_t *reference) {
    // We're going to try millions of random mutations until we find a program
    // that's equivalent.
    context_t rewrite = *reference;
    context_t proposal;
    init_program(&rewrite.program);
    insert_instr(&rewrite);
    hexdump(&rewrite);

    for (int i = 0; i < 10000000; i++) {
        proposal = rewrite;

        for (int j = 0; j < 10; j++) {
            if (iterate(reference, &rewrite, &proposal)) {
                hexdump(&proposal);
                rewrite = proposal;
            }
        }
    }
    hexdump(&rewrite);
}

void deadcodeelim(context_t *reference) {
    // One common and simple but effective optimisation strategy is dead code
    // elimination. It removes instructions which are found to be ineffectual.
    context_t rewrite;
    context_t proposal;

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
}

void search_init() {

    // We're going to find all defined zero-page addresses and add them to the
    // zp_addresses pick.
    initialize_pick(&zp_addresses);

    iterator_t addr_it;
    uint16_t address;

    for (iterator_init(&addresses, &addr_it); pick_iterate(&addr_it, &address);)
        if (address < 256)
            pick_insert(&zp_addresses, address);
}
