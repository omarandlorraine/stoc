#include "main.h"
#include "asm.h"
#include "optimization.h"
#include "pick.h"
#include "search.h"
#include "stoc.h"
#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

uint8_t mem_read(context_t *c, uint16_t address) { return c->mem[address]; }

void mem_write(context_t *c, uint16_t address, uint8_t val) {
    c->mem[address] = val;
}

void hexdump(context_t *c) {
    rewrite_t *r = &c->program;
    printf("; starting at $%04x\n", r->org);
    printf("; %d instructions\n", r->length);
    printf("; %d bytes\n; %lld clockticks\n", r->blength, c->clockticks);
    for (int i = 0; i < r->length; i++) {
        uint8_t instr = r->instructions[i].opcode;
        if (is_implied_instruction(instr)) {
            fprintf(stderr, "\t%s\n", opnames[instr]);
        } else if (is_immediate_instruction(instr)) {
            fprintf(stderr, "\t%s #$%02x\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_zero_page_instruction(instr)) {
            fprintf(stderr, "\t%s $%02x\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_zero_page_x_instruction(instr)) {
            fprintf(stderr, "\t%s $%02x,x\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_zero_page_y_instruction(instr)) {
            fprintf(stderr, "\t%s $%02x,y\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_absolute_instruction(instr)) {
            fprintf(stderr, "\t%s $%04x\n", opnames[instr],
                    r->instructions[i].operand);
        } else if (is_absolute_y_instruction(instr)) {
            fprintf(stderr, "\t%s $%04x,y\n", opnames[instr],
                    r->instructions[i].operand);
        } else if (is_absolute_x_instruction(instr)) {
            fprintf(stderr, "\t%s $%04x,x\n", opnames[instr],
                    r->instructions[i].operand);
        } else if (is_indirect_instruction(instr)) {
            fprintf(stderr, "\t%s $(%04x)\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_indirect_x_instruction(instr)) {
            fprintf(stderr, "\t%s $(%02x),x\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_relative_instruction(instr)) {
            fprintf(stderr, "\t%s * + %d\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else if (is_indirect_y_instruction(instr)) {
            fprintf(stderr, "\t%s $(%02x,y)\n", opnames[instr],
                    r->instructions[i].operand & 0x00ff);
        } else {
            fprintf(stderr, "\t$%02x", instr);
            if (opcode_length(instr) > 1)
                fprintf(stderr, " $%02x", r->instructions[i].operand & 0x00ff);
            if (opcode_length(instr) > 2)
                fprintf(stderr, " $%02x", r->instructions[i].operand >> 8);
            fprintf(stderr, "\n");
        }
    }
    fprintf(stderr, "\n");
}

void help(char * progname) {
    printf("Usage:\n\t%s [options...] decl_file [actions...]\n", progname);
    printf("\nPossible options:\n\t-O    optimize for speed, this is the "
           "default one\n");
    printf("\t-Os   optimize for size\n");
    printf("\nPossible actions:\n\t.dce  eliminate dead code\n");
    printf("\t.gen  stochastically generate another program\n");
    printf("\t.opt  stochastically optimize the existing program\n");
    printf("\t.dis  disassemble the program in the decl_file\n");
    exit(1);
}

void parseoption(char *opt, char * progname) {
    if (!strcmp(opt, "-Os")) {
        set_optimization(optimize_size);
        return;
    }
    if (!strcmp(opt, "-O")) {
        set_optimization(optimize_speed);
        return;
    }
    fprintf(stderr, "Unknown option %s\n", opt);
    help(progname);
}

void actions(char *opt, context_t *c, char * progname) {
    if (!strcmp(opt, ".dis")) {
        hexdump(c);
        return;
    }

    if (!strcmp(opt, ".dce")) {
        deadcodeelim(c);
        return;
    }

    if (!strcmp(opt, ".opt")) {
        stoc_opt(c);
        return;
    }

    fprintf(stderr, "Unknown search strategy %s\n", opt);
    help(progname);
}

int main(int argc, char **argv) {
    int i;
    srand(time(NULL));

    pickinit();
    pick_set_common_constants();

    context_t c;
    c.clockticks = 0;
    set_optimization(optimize_speed);

    for (i = 1; i < argc; i++) {
        char *opt = argv[i];
        if (opt[0] != '-')
            break;
        parseoption(opt, argv[0]);
    }
    if (!argv[i])
        help(argv[0]);

    readfile(argv[i++], &c);
    measure(&c);
	search_init();

    for (; i < argc; i++) {
        char *opt = argv[i];
        actions(opt, &c, argv[0]);
    }
}
