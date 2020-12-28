#include <stdio.h>
#include <stdlib.h>
#include "stoc.h"
#include "tests.h"
#include "search.h"
#include "main.h"
#include "labels.h"
#include "instr.h"

static bool valid_operand(rewrite_t * r, instruction_t * i) {
	// Sees whether an instruction's operand is valid.
	if(is_implied_instruction(i->opcode))
		return true;

	if(is_immediate_instruction(i->opcode))
		return true;

	if(is_indirect_instruction(i->opcode))
		return label_valid(i->operand);

	if(is_indirect_x_instruction(i->opcode))
		return label_valid(i->operand);

	if(is_indirect_y_instruction(i->opcode))
		return label_valid(i->operand);

	if(is_absolute_instruction(i->opcode))
		return label_valid(i->operand);

	if(is_absolute_x_instruction(i->opcode))
		return label_valid(i->operand);

	if(is_absolute_y_instruction(i->opcode))
		return label_valid(i->operand);

	if(is_zero_page_instruction(i->opcode))
		return label_valid(i->operand);

	if(is_zero_page_x_instruction(i->opcode))
		return label_valid(i->operand);

	if(is_zero_page_y_instruction(i->opcode))
		return label_valid(i->operand);

	if(is_relative_instruction(i->opcode)) {
		// search across the whole program to make sure this relative branch
		// points to an instruction
		for(int j = 0; j <= r->length; j++) {
			if(r->instructions[j].address == i->address + (int8_t)i->operand)
				return true;
		}
		return false;
	}

	printf("I don't know how to check this instruction %02x for validity\n", i->opcode);
	exit(1);
}

static void remove_instr(context_t * proposal) {
	int offs = rand() % proposal->program.length;
	for(int i = offs; i < proposal->program.length - 1; i++) {
		proposal->program.instructions[i] = proposal->program.instructions[i + 1];
	}
	proposal->program.length--;
}

void deadcodeelim(context_t * reference) {
	// One common and simple but effective optimisation strategy is dead code
 	// elimination. It removes instructions which are found to be ineffectual.
	context_t rewrite;
	context_t proposal;

	rewrite = *reference;

	for(int i = 0; i < 1000; i++) {
		proposal = rewrite;

		for(int j = 0; j < 5; j++) {
			remove_instr(&proposal);
			if(!proposal.program.length) break;
			if(equivalence(&rewrite, &proposal, 0)) {
				rewrite = proposal;
			}
		}
	}
	hexdump(&rewrite);
}
