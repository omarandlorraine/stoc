#include "search.h"
#include "main.h"
#include "optimization.h"
#include "pick.h"
#include "stoc.h"
#include "tests.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static uint8_t random_opcode() {
    uint8_t opcode;
retry:
    opcode = rand();
    if (!opcode_legal_p(opcode))
        goto retry;
    if (!addressing_modes[opcode])
        goto retry;
    if (addressing_modes[opcode]->mode == RELATIVE)
        goto retry;
    return opcode;
}

// FIXME: Never ignore this return value.
#pragma GCC diagnostic error "-Wunused-result"
static bool randomise_operand(rewrite_t *p, instruction_t *i) {
    addressing_mode_t *mode = addressing_modes[i->opcode];
    if (!mode)
        return false;

    switch (mode->mode) {
    case IMPLIED:
        return true;

    case IMMEDIATE:
        return random_constant(&i->operand);

    case INDIRECT_X:
    case INDIRECT_Y:
    case ZERO_PAGE:
    case ZERO_PAGE_X:
    case ZERO_PAGE_Y:
        return random_zp_address(&i->operand);

    case INDIRECT:
    case ABSOLUTE:
    case ABSOLUTE_X:
    case ABSOLUTE_Y:
        return random_address(&i->operand);

    case RELATIVE:
        printf("relative instruction error\n");
        exit(1);

    default:
        printf("unknown mode for %02x error\n", i->opcode);
        exit(1);
    }
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
    uint8_t opcode = i.opcode;
    addressing_mode_t *m = addressing_modes[opcode];
    if (m) {
        uint8_t new = m->opcodes[rand() % m->len];
        i.opcode = new;
    }
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
    int r = rand() %
            6; // Keep on incrementing this until all mutations get tested okay.
    switch (r) {
    case 0:
        // printf("remove_instr(proposal)\n");
        remove_instr(proposal);
        break;
    case 1:
        // printf("modify_opcode(proposal)\n");
        modify_opcode(proposal);
        break;
    case 2:
        // printf("modify_operand(proposal)\n");
        modify_operand(proposal);
        break;
    case 3:
        // printf("swap_instrs(proposal)\n");
        swap_instrs(proposal);
        break;
    case 4:
        // printf("replace_instr(proposal)\n");
        replace_instr(proposal);
        break;
    case 5:
        // printf("insert_instr(proposal)\n");
        insert_instr(proposal);
        break;
    }
}

static bool iterate(context_t *reference, context_t *rewrite,
                    context_t *proposal) {
    random_mutation(proposal);
    // hexdump(proposal);
    // printf("============================================\n");

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
            } else {
                //				hexdump(&rewrite);
                //				hexdump(&proposal);
                //				printf("============================================\n");
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
            } else {
                hexdump(&rewrite);
                hexdump(&proposal);
                printf("============================================\n");
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
