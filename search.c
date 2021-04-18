#include "search.h"
#include "instr.h"
#include "labels.h"
#include "main.h"
#include "stoc.h"
#include "tests.h"
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

static bool valid_operand(rewrite_t *r, instruction_t *i) {
    switch (addressing_modes[i->opcode]->mode) {
    case IMPLIED:
    case IMMEDIATE:
        return true;

    case INDIRECT_X:
    case INDIRECT_Y:
    case ZERO_PAGE:
    case ZERO_PAGE_X:
    case ZERO_PAGE_Y:
        if (i->operand > 255)
            return false;
    case INDIRECT:
    case ABSOLUTE:
    case ABSOLUTE_X:
    case ABSOLUTE_Y:
        return label_valid(i->operand);

    case RELATIVE:
        // search across the whole program to make sure this relative branch
        // points to an instruction
        for (int j = 0; j <= r->length; j++) {
            if (r->instructions[j].address ==
                i->address + (int8_t)(i->operand & 0x00ff))
                return true;
        }
        return false;
    }
    printf("I don't know how to check this instruction %02x for validity\n",
           i->opcode);
    exit(1);
}

static uint8_t random_opcode() {
    uint8_t opcode;
retry:
    do {
        opcode = rand();
    } while (!opcode_legal_p(opcode));

    int mode = addressing_modes[opcode]->mode;

    if (!labels_defined()) {
        // No labels have been defined at all
        // So some addressing modes cannot be used.
        if (mode == ABSOLUTE)
            goto retry;
        if (mode == ABSOLUTE_X)
            goto retry;
        if (mode == ABSOLUTE_Y)
            goto retry;
        if (mode == ZERO_PAGE)
            goto retry;
        if (mode == ZERO_PAGE_X)
            goto retry;
        if (mode == ZERO_PAGE_Y)
            goto retry;
        if (mode == INDIRECT)
            goto retry;
        if (mode == INDIRECT_X)
            goto retry;
        if (mode == INDIRECT_Y)
            goto retry;
    }
    if (!zp_labels_defined()) {
        // No labels have been defined in Zero Page,
        // So some addressing modes cannot be used.
        if (mode == ZERO_PAGE)
            goto retry;
        if (mode == ZERO_PAGE_X)
            goto retry;
        if (mode == ZERO_PAGE_Y)
            goto retry;
        if (mode == INDIRECT_X)
            goto retry;
        if (mode == INDIRECT_Y)
            goto retry;
    }

    return opcode;
}

static void randomise_operand(rewrite_t *p, instruction_t *i) {
    addressing_mode_t *mode = addressing_modes[i->opcode];
    if (!mode)
        return;
    switch (mode->mode) {
    case IMPLIED:
        break;

    case IMMEDIATE:
        i->operand = rand();
        break;

    case RELATIVE:
        do {
            i->operand = random_label();
        } while (!valid_operand(p, i));
        break;

    default:
        do {
            i->operand = random_label();
        } while (!valid_operand(p, i));
        break;
    }
}

static void randomise_instruction(rewrite_t *p, instruction_t *i) {
    i->opcode = random_opcode();
    randomise_operand(p, i);
}

static void remove_instr(context_t *proposal) {
    if (proposal->program.length == 0)
        return;
    int offs = rand() % proposal->program.length + 1;
    for (int i = offs; i < proposal->program.length - 1; i++) {
        proposal->program.instructions[i] =
            proposal->program.instructions[i + 1];
    }
    proposal->program.length--;
}

static void insert_instr(context_t *proposal) {
    int rnd = rand();
    int offs = rnd % (proposal->program.length + 1);
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
    int offs = rnd % (proposal->program.length + 1);
    randomise_operand(&proposal->program,
                      &proposal->program.instructions[offs]);
}

static void modify_opcode(context_t *proposal) {
    if (proposal->program.length == 0)
        return;
    int offs = rand() % (proposal->program.length + 1);
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
    int offs = rand() % (proposal->program.length + 1);
    proposal->program.instructions[offs].opcode = random_opcode();
    randomise_operand(&proposal->program,
                      &proposal->program.instructions[offs]);
}

static void swap_instrs(context_t *proposal) {
    if (proposal->program.length == 0)
        return;
    int offs1 = rand() % (proposal->program.length + 1);
    instruction_t temp = proposal->program.instructions[offs1];
    int offs2 = rand() % (proposal->program.length + 1);
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
        remove_instr(proposal);
        break;
    case 1:
        insert_instr(proposal);
        break;
    case 2:
        modify_opcode(proposal);
        break;
    case 3:
        modify_operand(proposal);
        break;
    case 4:
        replace_instr(proposal);
        break;
    case 5:
        swap_instrs(proposal);
        break;
    }
}

static int clockticks_cost(context_t *c) {
    if (c->exitcode)
        return INT_MAX;
    return c->clockticks + c->hamming;
}

static int hamming_cost(context_t *c) {
    if (c->exitcode)
        return INT_MAX;
    return c->hamming;
}

static bool iterate(context_t *reference, context_t *rewrite,
                    context_t *proposal, int (*cost)(context_t *r)) {
    random_mutation(proposal);
    if (checkem(&proposal->program)) {
        measure_two(reference, rewrite, proposal);
        int ocost = cost(rewrite);
        int pcost = cost(proposal);
        if (pcost < ocost) {
            return true;
        }
    }
    return false;
}

void stoc_opt(context_t *reference) {
    // We're going to try millions of random mutations until we find a program
    // that's both equivalent and more optimal
    context_t rewrite = *reference;
    context_t proposal;

    for (int i = 0; i < 1000000; i++) {
        proposal = rewrite;
        printf("iteration %d\n", i);

        for (int j = 0; j < 100; j++) {
            if (iterate(reference, &rewrite, &proposal, &clockticks_cost)) {
                hexdump(&proposal);
                rewrite = proposal;
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
            if (iterate(reference, &rewrite, &proposal, &hamming_cost)) {
                hexdump(&proposal);
                rewrite = proposal;
            } else {
                //				hexdump(&rewrite);
                //				hexdump(&proposal);
                //				printf("============================================\n");
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
            if (equivalence(&rewrite, &proposal, 0)) {
                rewrite = proposal;
            }
        }
    }
    hexdump(&rewrite);
}
