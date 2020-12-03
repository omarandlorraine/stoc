/* 
 * Dead Code Eliminator
 *
 * One common and simple but effective optimisation strategy is the dead code
 * elimination. It removes instructions which are found to be ineffectual.
 * 
 */
#include <stdio.h>
#include <stdlib.h>
#include "stoc.h"
#include "tests.h"
#include "exh.h"
#include "main.h"
#include "labels.h"
#include "instr.h"

static void remove_instr(context_t * proposal) {
	int offs = rand() % proposal->program.length;
	for(int i = offs; i < proposal->program.length - 1; i++) {
		proposal->program.instructions[i] = proposal->program.instructions[i + 1];
	}
	proposal->program.length--;
}

void deadcodeelim(context_t * reference) {
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
