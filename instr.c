/*
 * This file contains the source code for opcode selection.
 * The configuration of opcode selection allows the user to specify things like
 *   -  branchless code
 *   -  position-independent code
 */
#include "instr.h"

static int conditionals = 0;
static int pointers = 0;

/* for now, just return whether the opcode is legal or not. */
int valid_opcode(uint8_t opcode) {

	if(!conditionals && is_relative_instruction(opcode)) return 0;

	if(!pointers && is_indirect_x_instruction(opcode)) return 0;
	if(!pointers && is_indirect_y_instruction(opcode)) return 0;

	if(optable[opcode]) return 1;

	return 0;
}
