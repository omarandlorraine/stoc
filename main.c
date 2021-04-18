#include "main.h"
#include "exh.h"
#include "labels.h"
#include "asm65.h"
#include "reg.h"
#include "search.h"
#include "stoc.h"
#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>
#define ASMBUFLEN 255

int org = 0;

uint8_t mem_read(context_t *c, uint16_t address) { return c->mem[address]; }

void mem_write(context_t *c, uint16_t address, uint8_t val) {
    c->mem[address] = val;
}

void hexdump(context_t *c) {
    rewrite_t *r = &c->program;
    printf("; %d instructions\n", r->length);
    printf("; %d bytes\n; %lld clockticks\n", r->blength, c->clockticks);
    printf("; hamming distance %lld\n", c->hamming);
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

void parseoption(char *opt) {
    char tmp[100];
    strncpy(tmp, opt, 100);

	if(strncmp("--", opt, 2)) {
		assemble(opt);
	}

	char * key = tmp + 2;
	char * equal = strstr(tmp, "=");

    if (equal) {
        char *val = equal + 1;
        equal[0] = '\0';
        mklbl(key, val);
        return;
    }

    fprintf(stderr, "Unknown option %s\n", opt);
    exit(1);
}

int main(int argc, char ** argv) {
	int i = 0;
	srand(time(NULL));
	for(i = 1; i < argc; i++) {
		char * opt = argv[i];
		if(!strncmp(opt, "--", 2)) parseoption(opt);
		else assemble(opt);
	}

    uint16_t value;
    if (getlbl("hello", &value)) {
        printf("found it %u\n", value);
    }
}
