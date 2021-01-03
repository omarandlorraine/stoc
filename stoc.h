#pragma once
#include <stdbool.h>
#include <inttypes.h>

#define REWRITE_LEN 50
#define LABEL_LEN   20
#define DATA_LEN    20


#define ABSOLUTE 0
#define ABSOLUTE_X 1
#define ABSOLUTE_Y 2
#define INDIRECT 3
#define INDIRECT_X 4
#define INDIRECT_Y 5
#define ZERO_PAGE 6
#define ZERO_PAGE_X 7
#define ZERO_PAGE_Y 8
#define RELATIVE 9
#define IMMEDIATE 10
#define IMPLIED 11

bool opcode_legal_p(uint8_t op);
int opcode_length(uint8_t op);
int opcode_branch_p(uint8_t op);
bool implied_instruction(char * op, uint8_t * out);
int is_implied_instruction(uint8_t op);
bool immediate_instruction(char * op, uint8_t * out);
int is_immediate_instruction(uint8_t op);
bool indirect_x_instruction(char * op, uint8_t * out);
int is_indirect_x_instruction(uint8_t op);
bool indirect_y_instruction(char * op, uint8_t * out);
int is_indirect_y_instruction(uint8_t op);
int is_indirect_instruction(uint8_t op);
int is_absolute_x_instruction(uint8_t op);
int is_absolute_y_instruction(uint8_t op);
int is_absolute_instruction(uint8_t op);
bool absolute_instruction(char * op, uint8_t * out);
int is_zero_page_instruction(uint8_t op);
bool zero_page_instruction(char * op, uint8_t * out);
int is_zero_page_x_instruction(uint8_t op);
int is_zero_page_y_instruction(uint8_t op);
int is_relative_instruction(uint8_t op);
bool relative_instruction(char * op, uint8_t * out);

typedef struct {
	int len;
	int mode;
	uint8_t * opcodes;
} addressing_mode_t;

extern addressing_mode_t mode_absolute;
extern addressing_mode_t mode_absolute_x;
extern addressing_mode_t mode_absolute_y;
extern addressing_mode_t mode_indirect;
extern addressing_mode_t mode_zero_page;
extern addressing_mode_t mode_zero_page_x;
extern addressing_mode_t mode_zero_page_y;
extern addressing_mode_t mode_indirect_x;
extern addressing_mode_t mode_indirect_y;
extern addressing_mode_t mode_immediate;
extern addressing_mode_t mode_implied;
extern addressing_mode_t mode_relative;
extern addressing_mode_t* addressing_modes[256];


typedef uint16_t addr_t;
typedef uint8_t  data_t;
#define ADDR_SPACE 0xffff

typedef struct i1 {
	addr_t address;
	uint8_t opcode;
	int target;
	int operand;
} instruction_t;

typedef struct r1 {
	uint16_t org;
	uint16_t length; // number of instructions
	uint16_t end; 	// first address after last instruction
	instruction_t instructions[REWRITE_LEN];
	long double fitness;
	long double mcycles; // machine cycles
	int blength; // length in bytes
} rewrite_t;

typedef struct c1 {
	uint8_t a;
	uint8_t x;
	uint8_t y;
	uint8_t flags;
	uint8_t s;
	uint16_t pc;
    long long int clockticks;
	long long int hamming;
    rewrite_t program;
	data_t mem[ADDR_SPACE];
	uint8_t memf[ADDR_SPACE];
	uint16_t ea;
	uint8_t opcode;
	int exitcode;
} context_t;

void init_program(rewrite_t * r);
void install_program(rewrite_t * r);
extern void (*optable[256])(context_t * c);
char *opnames[256];
