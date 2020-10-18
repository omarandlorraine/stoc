#pragma once
#include <stdbool.h>
#include <inttypes.h>

#define REWRITE_LEN 50

bool opcode_legal_p(uint8_t op);
int opcode_length(uint8_t op);
int opcode_branch_p(uint8_t op);
uint8_t implied_instruction(char * op);
int is_implied_instruction(uint8_t op);
uint8_t immediate_instruction(char * op);
int is_immediate_instruction(uint8_t op);
uint8_t indirect_x_instruction(char * op);
int is_indirect_x_instruction(uint8_t op);
uint8_t indirect_y_instruction(char * op);
int is_indirect_y_instruction(uint8_t op);
int is_absolute_x_instruction(uint8_t op);
int is_absolute_y_instruction(uint8_t op);
int is_absolute_instruction(uint8_t op);
int is_zero_page_instruction(uint8_t op);
int is_zero_page_x_instruction(uint8_t op);
int is_zero_page_y_instruction(uint8_t op);
int is_relative_instruction(uint8_t op);
typedef uint16_t addr_t;
typedef uint8_t  data_t;

typedef struct i1 {
	uint8_t opcode;
	addr_t address;
	uint16_t operand;
} instruction_t;

typedef struct r1 {
	uint16_t org;
	uint16_t length; // number of instructions
	uint16_t end; 	// first address after last instruction
	instruction_t instructions[REWRITE_LEN];
} rewrite_t;

typedef struct c65 {
    uint8_t a, x, y, status, sp, opcode;
    uint16_t pc, ea;
    int penaltyaddr;
    int penaltyop;
    int clockticks;
    uint8_t mem[65536];
} Context65;

void (*optable[256])(Context65 * c);
char *opnames[256];
