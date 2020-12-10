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

	if(is_indirect_x_instruction(i->opcode))
		return label_valid(i->operand);

	if(is_indirect_y_instruction(i->opcode))
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
		for(int j; j <= r->length; j++) {
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

void iterate(rewrite_t * rewrite, int offs) {
	if(rewrite->length < offs + 1) rewrite->length = offs + 1;

	instruction_t * instruction = &rewrite->instructions[offs];

	if(!++instruction->operand)
		goto next_opcode;

	uint8_t old_insn = instruction->opcode;

	if(is_zero_page_instruction(old_insn) || is_zero_page_x_instruction(old_insn) || is_zero_page_y_instruction(old_insn)) {
		for(;;) {
			if(instruction->operand > 255)
				goto next_opcode;
			if(label_valid(instruction->operand))
				return;
			instruction->operand++;
		}
	}
	else if(is_immediate_instruction(old_insn)) {
		if(instruction->operand < 255)
			return;
	}
	else if(is_implied_instruction(old_insn)) {
		if(instruction->operand == 0) {
			return;
		}
		goto next_opcode;
	}
	// TODO: absolute
	// TODO: absolute-x
	// TODO: absolute-y
	// TODO: relative
	// TODO: indirect
next_opcode:
	instruction->operand = 0;
	if(!++instruction->opcode)
		iterate(rewrite, offs + 1);
	if(!opcode_legal_p(instruction->opcode))
		goto next_opcode;
}

void exhaustive(context_t * reference, context_t * rewrite) {

	if(!equivalence(reference, reference, 0)) {
		printf("reference is not equivalent to itself\n");
		exit(1);
	}
	init_program(&(rewrite->program));
	int oldlen = rewrite->program.length;

	iterate(&rewrite->program, 0);

	while(!equivalence(reference, rewrite, 0)) {
		if(oldlen != rewrite->program.length) {
			oldlen = rewrite->program.length;
		}
		iterate(&rewrite->program, 0);
	}
	hexdump(rewrite);
}
