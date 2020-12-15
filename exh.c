#include <stdio.h>
#include <stdlib.h>
#include "stoc.h"
#include "tests.h"
#include "search.h"
#include "main.h"
#include "labels.h"
#include "instr.h"

void search_instruction(context_t * reference, context_t * rewrite, int offs) {
	// hexdump(rewrite);
	// If the shoe fits, print it to stdout and exit.
	if(offs == rewrite->program.length) {
		if(equivalence(reference, rewrite, 0)) {
			hexdump(rewrite);
			exit(0);
		}
		return;
	}

	// Try all the implied instructions
	for(int op = 0; op < 256; op++) {
		if(!opcode_legal_p(op)) continue;
		if(!is_implied_instruction(op)) continue;
		rewrite->program.instructions[offs].opcode = op;
		search_instruction(reference, rewrite, offs + 1);
	}

	// Try all the immediate instructions
	for(int op = 0; op < 256; op++) {
		// TODO: Maybe have some way to turn search only the most common
		// immediate values.
		if(!opcode_legal_p(op)) continue;
		if(!is_immediate_instruction(op)) continue;
		rewrite->program.instructions[offs].opcode = op;
		for(int arg = 0; arg < 256; arg++) {
			rewrite->program.instructions[offs].operand = arg;
			search_instruction(reference, rewrite, offs + 1);
		}
	}

	// Try all branches
	for(int op = 0; op < 256; op++) {
		if(!opcode_legal_p(op)) continue;
		if(!is_relative_instruction(op)) continue;
		rewrite->program.instructions[offs].opcode = op;
		// backward
		for(int dst = 0; dst < offs; dst++) {
			uint16_t pc = rewrite->program.instructions[offs].address;
			uint16_t dest = rewrite->program.instructions[dst].address;
			rewrite->program.instructions[offs].operand = pc - dest;
		}
		// forward
		for(int dst = 1; dst < 127; dst++) {
			rewrite->program.instructions[offs].operand = dst;
		}
	}

	// Try all zeropage instructions
	// (including indexed and indirect ones)
	for(int op = 0; op < 256; op++) {
		if(!opcode_legal_p(op)) continue;
		if(opcode_length(op) != 2) continue;
		if(is_immediate_instruction(op)) continue;
		if(is_relative_instruction(op)) continue;
	}
	
}

void exhaustive(context_t * reference) {
	context_t rewrite = *reference;
	if(!equivalence(reference, reference, 0)) {
        printf("reference is not equivalent to itself\n");
        exit(1);
    }
    init_program(&(rewrite.program));

	for(int l = 1; l < 10; l++) {
		rewrite.program.length = l;
		search_instruction(reference, &rewrite, 0);
	}
}
