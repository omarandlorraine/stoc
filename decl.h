#pragma once
#define LABEL_LEN 20
#include "stoc.h"

typedef struct _decl_t decl_t;
struct _decl_t {
    int (*fn)(stoc_t *c, struct _decl_t *d, uint8_t **scram);
    int (*setup)(stoc_t *c, struct _decl_t *d, uint8_t **scram);
    char label[LABEL_LEN];
    uint16_t start;
    unsigned int length;
    struct _decl_t *next;
};

void output_scram(uint8_t **scram, uint8_t out);
uint8_t consume_scram(uint8_t **scram);

int setup_start_decl(stoc_t *c, decl_t *d, uint8_t **scram);
int start_decl(stoc_t *c, decl_t *d, uint8_t **scram);
int run_decl(stoc_t *c, decl_t *d, uint8_t **scram);

int live_in_a(stoc_t *c, decl_t *d, uint8_t **scram);
int live_in_x(stoc_t *c, decl_t *d, uint8_t **scram);
int live_in_y(stoc_t *c, decl_t *d, uint8_t **scram);
int live_in_stack(stoc_t *c, decl_t *d, uint8_t **scram);
int live_in_memory(stoc_t *c, decl_t *d, uint8_t **scram);
int live_in_pointer(stoc_t *c, decl_t *d, uint8_t **scram);

int live_out_a(stoc_t *c, decl_t *d, uint8_t **scram);
int live_out_x(stoc_t *c, decl_t *d, uint8_t **scram);
int live_out_y(stoc_t *c, decl_t *d, uint8_t **scram);
int live_out_stack(stoc_t *c, decl_t *d, uint8_t **scram);
int live_out_memory(stoc_t *c, decl_t *d, uint8_t **scram);
int live_out_pointer(stoc_t *c, decl_t *d, uint8_t **scram);
int setup_byte_in(stoc_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_a(stoc_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_x(stoc_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_y(stoc_t *c, decl_t *d, uint8_t **scram);
int setup_live_in_stack(stoc_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_stack(stoc_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_memory(stoc_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_pointer(stoc_t *c, decl_t *d, uint8_t **scram);

void extract_result(decl_t *d, stoc_t *reference);
void free_decl(decl_t *d);
