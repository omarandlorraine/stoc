#pragma once
#define LABEL_LEN 20
#include "stoc.h"

typedef struct _decl_t decl_t;
struct _decl_t {
    int (*fn)(context_t *c, struct _decl_t *d, uint8_t **scram);
    int (*setup)(context_t *c, struct _decl_t *d, uint8_t **scram);
    char label[LABEL_LEN];
    uint16_t start;
    unsigned int length;
    struct _decl_t *next;
};

int setup_start_decl(context_t *c, decl_t *d, uint8_t **scram);
int start_decl(context_t *c, decl_t *d, uint8_t **scram);
int run_decl(context_t *c, decl_t *d, uint8_t **scram);

int live_in_a(context_t *c, decl_t *d, uint8_t **scram);
int live_in_x(context_t *c, decl_t *d, uint8_t **scram);
int live_in_y(context_t *c, decl_t *d, uint8_t **scram);
int live_in_stack(context_t *c, decl_t *d, uint8_t **scram);
int live_in_memory(context_t *c, decl_t *d, uint8_t **scram);
int live_in_pointer(context_t *c, decl_t *d, uint8_t **scram);

int live_out_a(context_t *c, decl_t *d, uint8_t **scram);
int live_out_x(context_t *c, decl_t *d, uint8_t **scram);
int live_out_y(context_t *c, decl_t *d, uint8_t **scram);
int live_out_stack(context_t *c, decl_t *d, uint8_t **scram);
int live_out_memory(context_t *c, decl_t *d, uint8_t **scram);
int live_out_pointer(context_t *c, decl_t *d, uint8_t **scram);
int setup_byte_in(context_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_a(context_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_x(context_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_y(context_t *c, decl_t *d, uint8_t **scram);
int setup_live_in_stack(context_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_stack(context_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_memory(context_t *c, decl_t *d, uint8_t **scram);
int setup_live_out_pointer(context_t *c, decl_t *d, uint8_t **scram);

void extract_result(decl_t *d, context_t *reference);
