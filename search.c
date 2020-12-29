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

static void randomise_instruction(rewrite_t * p, instruction_t * i) {
	// TODO: There's a bug here that means this function will never return if
	// the randomly selected opcode is a branch instruction. To work around it,
	// I've excluded these instructions from consideration
	uint8_t opcode;
retry:
	do {
		opcode = rand();
	} while(!opcode_legal_p(opcode));
	if(is_relative_instruction(opcode)) goto retry;
	i->opcode = opcode;
	
	do {	
		i->operand = random_label();
	} while(!valid_operand(p, i));
}

static void remove_instr(context_t * proposal) {
	if(proposal->program.length == 0) return;
	int offs = rand() % proposal->program.length + 1;
	for(int i = offs; i < proposal->program.length - 1; i++) {
		proposal->program.instructions[i] = proposal->program.instructions[i + 1];
	}
	proposal->program.length--;
}

static void insert_instr(context_t * proposal) {
	int rnd = rand();
	int offs = rnd % (proposal->program.length + 1);
	for(int i = offs; i < proposal->program.length - 1; i++) {
		proposal->program.instructions[i] = proposal->program.instructions[i - 1];
	}
	randomise_instruction(&proposal->program, &proposal->program.instructions[offs]);
}

static void random_mutation(context_t * proposal) {
	int r = rand() % 2;
	switch(r) {
	case 0:
		remove_instr(proposal);
		return;
	case 1:
		insert_instr(proposal);
		return;
	}
}

static int cost(context_t * c) {
	return c->clockticks;
}

void stoc_opt(context_t * reference) {
	// We're going to try millions of random mutations until we find a program
	// that's both equivalent and more optimal
	context_t rewrite = *reference;
	context_t proposal;

	bool first = true;

	for(int i = 0; i < 1000; i++) {
		proposal = rewrite;

		for(int j = 0; j < 5; j++) {
			random_mutation(&proposal);
			if(equivalence(reference, &proposal, 0)) {
				int ocost = cost(reference);
				int pcost = cost(&proposal);
				if(pcost < ocost)
					rewrite = proposal;
			}
		}
	}
	hexdump(&rewrite);
}

void stoc_gen(context_t * reference) {
	// We're going to try millions of random mutations until we find a program
	// that's both equivalent and more optimal
	context_t rewrite;
	context_t proposal;
	
	init_program(&(rewrite.program));
	init_program(&(proposal.program));

	rewrite = *reference;
	hexdump(&rewrite);
	measure(reference, reference);
	hexdump(&rewrite);

	bool first = true;
	int i = 0;
	for(;;) {
		printf("iteration %d\n", i++);
		proposal = rewrite;
		hexdump(&rewrite);

		for(int j = 0; j < 5; j++) {
			printf("inner loop\n");
			hexdump(&rewrite);
			random_mutation(&proposal);
			hexdump(&rewrite);
			if(!proposal.program.length) break;
			if(proposal.exitcode) break;

			measure(&rewrite, &proposal);

			int rfit = proposal.hamming + proposal.program.fitness;
			int pfit = rewrite.hamming + rewrite.program.fitness;

			if(rfit > pfit) {
				rewrite = proposal;
				first = false;
			}
			if(!first && equivalence(reference, &proposal, 0)) {
				stoc_opt(&proposal);
				rewrite = proposal;
			}
		}
	}
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
