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
	// TODO: iterate across operands first.
	uint8_t old_insn = rewrite->instructions[offs].opcode;

	if(is_zero_page_instruction(old_insn) || is_zero_page_x_instruction(old_insn) || is_zero_page_y_instruction(old_insn)) {
		// Scan across the defined zero-page locations.
		while(rewrite->instructions[offs].operand < 256) {
			rewrite->instructions[offs].operand++;
			if(label_valid(rewrite->instructions[offs].operand)) return;
			// else, we have exhausted the defined ZP locations
			//    and so we need to try the next instruction instead.
		}
	}
	else if(is_immediate_instruction(old_insn)) {
		if(rewrite->instructions[offs].operand < 255) {
			rewrite->instructions[offs].operand++;
			return;
		}
	}
	// TODO: absolute
	// TODO: absolute-x
	// TODO: absolute-y
	// TODO: relative
	// TODO: indirect
	do {
		if(rewrite->instructions[offs].opcode == 255) iterate(rewrite, offs + 1);
		rewrite->instructions[offs].opcode++;
		rewrite->instructions[offs].operand = 0;
	} while(!valid_opcode(rewrite->instructions[offs].opcode));
	iterate(rewrite, offs);
}

void exhaustive(rewrite_t * reference, rewrite_t * rewrite) {

	if(!equivalence(reference, reference)) {
		printf("reference is not equivalent to itself\n");
		exit(1);
	}

	rewrite->length = 1;
	rewrite->instructions[0].opcode = 0;
	iterate(rewrite, 0);

	while(!equivalence(reference, rewrite)) {
		iterate(rewrite, 0);
	}
	hexdump(rewrite);
}
