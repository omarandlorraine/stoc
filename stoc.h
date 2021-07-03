#pragma once
#include "pick.h"
#include <inttypes.h>
#include <stdbool.h>

#define REWRITE_LEN 50

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
bool implied_instruction(char *op, uint8_t *out);
int is_implied_instruction(uint8_t op);
bool immediate_instruction(char *op, uint8_t *out);
int is_immediate_instruction(uint8_t op);
bool indirect_x_instruction(char *op, uint8_t *out);
int is_indirect_x_instruction(uint8_t op);
bool indirect_y_instruction(char *op, uint8_t *out);
int is_indirect_y_instruction(uint8_t op);
int is_indirect_instruction(uint8_t op);
int is_absolute_x_instruction(uint8_t op);
int is_absolute_y_instruction(uint8_t op);
int is_absolute_instruction(uint8_t op);
bool absolute_instruction(char *op, uint8_t *out);
int is_zero_page_instruction(uint8_t op);
bool zero_page_instruction(char *op, uint8_t *out);
int is_zero_page_x_instruction(uint8_t op);
int is_zero_page_y_instruction(uint8_t op);
int is_relative_instruction(uint8_t op);
bool relative_instruction(char *op, uint8_t *out);

extern pick_t mode_absolute;
extern pick_t mode_absolute_x;
extern pick_t mode_absolute_y;
extern pick_t mode_indirect;
extern pick_t mode_zero_page;
extern pick_t mode_zero_page_x;
extern pick_t mode_zero_page_y;
extern pick_t mode_indirect_x;
extern pick_t mode_indirect_y;
extern pick_t mode_immediate;
extern pick_t mode_implied;
extern pick_t mode_relative;
extern pick_t *addressing_modes[256];

typedef uint16_t addr_t;
typedef uint8_t data_t;
#define ADDR_SPACE 0xffff

typedef struct i1 {
    addr_t address;
    uint8_t opcode;
    uint16_t operand;
} instruction_t;

/**
 * rewrite_t
 *
 * A rewrite is a list of instructions, plus associated data.
 */
typedef struct r1 {

    //! Where the rewrite starts
    uint16_t org;

    //! Number of instructions in the rewrite
    uint16_t length;

    //! The first address after the last instruction
    uint16_t end;

    //! Array of instructions
    instruction_t instructions[REWRITE_LEN];

    //! Fitness, or "how correct is the rewrite"
    long double fitness;

    //! machine cycles, or "how long does the program take"
    long double mcycles; // machine cycles

    //! The program's length, in bytes
    int blength;
} rewrite_t;

typedef struct {
    void *emu;
    int clockticks;
    rewrite_t program;
    struct _decl_t *decl;
} stoc_t;

void init_program(rewrite_t *r);
extern void (*optable[256])(stoc_t *c);
char *opnames[256];
