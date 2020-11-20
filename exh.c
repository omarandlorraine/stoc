/* 
 * Exhaustive search strategies
 *
 * One of the ways to search for an optimal program is an exhaustive search
 * across the entire search space. Here are two ways to do that:
 * 
 * 1. Genuinely search across EVERYTHING
 *    (this tactic is very thorough and very slow, because it searches across
 *    a lot of nonsense (such a basic blocks beginning with an `rti`, or 
 *    jumps to locations that don't contain anything useful)
 *
 * 2. Search across the commonest instructions only
 *    (this is a much faster search, but has the potential to miss something a
 *    more thorough tactic would find)
 *
 * 3. Somewhere in between these two
 *    I've got the idea that we can do a fast and cheap cost function (probably
 *    a hidden Markov chain or something like that) which can be used to
 *    discard a lot of obvious nonsense. This could be a scale from 1 to 10
 *    and could be controlled by a command-line argument
 */
#include <stdio.h>
#include <stdlib.h>
#include "stoc.h"
#include "tests.h"
#include "exh.h"
#include "main.h"
#include "labels.h"
#include "instr.h"

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

	if(!equivalence(reference, reference)) {
		printf("reference is not equivalent to itself\n");
		exit(1);
	}
	init_program(&(rewrite->program));
	int oldlen = rewrite->program.length;

	iterate(&rewrite->program, 0);

	while(!equivalence(reference, rewrite)) {
		if(oldlen != rewrite->program.length) {
			oldlen = rewrite->program.length;
		}
		iterate(&rewrite->program, 0);
	}
	hexdump(rewrite);
}
