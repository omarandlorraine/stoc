#include "asm.h"
#include "arch.h"
#include "decl.h"
#include "pick.h"
#include "stoc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Maximum length for each line.
#define ASMBUFLEN 255

// Whitespace, used for delimiting tokens, passed to strtok
#define DELIM "\n\t "

static int line_no = 0;

void endofline() {
    if (strtok(NULL, DELIM)) {
        fprintf(stderr, "Too many tokens error on line %d.\n", line_no);
        exit(1);
    }
}

decl_t *parse_register_in() {
    endofline();
    decl_t *d = malloc(sizeof(decl_t));
    d->fn = NULL;
    d->setup = setup_byte_in;
    d->start = 0;
    d->length = 1;
    d->next = NULL;
    return d;
}

decl_t *parse_register_out() {
    endofline();
    decl_t *d = malloc(sizeof(decl_t));
    d->fn = NULL;
    d->setup = NULL;
    d->start = 0;
    d->length = 1;
    d->next = NULL;
    return d;
}

decl_t *parse_stack_in() {
    int len = atoi(strtok(NULL, DELIM));
    endofline();
    decl_t *d = malloc(sizeof(decl_t));
    d->fn = live_in_stack;
    d->setup = setup_live_in_stack;
    d->start = 0;
    d->length = len;
    d->next = NULL;
    return d;
}

decl_t *parse_stack_out() {
    int len = atoi(strtok(NULL, DELIM));
    endofline();
    decl_t *d = malloc(sizeof(decl_t));
    d->fn = live_out_stack;
    d->setup = setup_live_out_stack;
    d->start = 0;
    d->length = len;
    d->next = NULL;
    return d;
}

uint8_t parse_byte(char *t) {
    if (t[0] != '$') {
        printf("syntax error while parsing run_decl, byte should start with "
               "'$'\n");
        exit(1);
    }
    uint8_t byte = strtol(t + 1, NULL, 16);
    return byte;
}

decl_t *parse_run(rewrite_t *r) {
    char *start_address = strtok(NULL, DELIM);

    if (start_address[0] != '$') {
        printf("syntax error while parsing run_decl, start address should "
               "start with '$'\n");
        exit(1);
    }
    int start = strtol(start_address + 1, NULL, 16);
    decl_t *d = malloc(sizeof(decl_t));
    d->start = start;
    d->length = 0;

    char *t;
    uint8_t program[1024];
    int offs = 0;
    while ((t = strtok(NULL, DELIM))) {
        program[offs++] = parse_byte(t);
        d->length++;
    }
    d->length--;
    read_prog(r, program, d->length);
    r->blength = d->length;
    r->org = d->start;
    d->fn = run_decl;
    d->setup = run_decl;
    d->next = NULL;

    return d;
}

decl_t *parseline(char *line, stoc_t *c) {
    char *f = strtok(line, DELIM);

    if (!f)
        return NULL;
    if (!strcmp(f, "rem"))
        return NULL;
    if (!strcmp(f, "stack-in"))
        return parse_stack_in();
    if (!strcmp(f, "stack-out"))
        return parse_stack_out();
    if (!strcmp(f, "run"))
        return parse_run(&c->program);
    if (!strcmp(f, "register-in")) {
        char *regname = strtok(NULL, DELIM);
        decl_t *d = parse_register_in();
        register_in_name(d, regname);
        return d;
    }
    if (!strcmp(f, "register-out")) {
        char *regname = strtok(NULL, DELIM);
        decl_t *d = parse_register_out();
        register_out_name(d, regname);
        return d;
    }

    fprintf(stderr, "unknown decltype \"%s\"\n", f);
    exit(1);
}

void readfile(char *filename, stoc_t *reference) {
    FILE *filePointer;
    char line[ASMBUFLEN];

    filePointer = fopen(filename, "r");
    if (!filePointer) {
        fprintf(stderr, "Couldn't open %s\n", filename);
        exit(1);
    }

    decl_t *d = malloc(sizeof(decl_t));
    d->fn = start_decl;
    d->setup = setup_start_decl;
    d->next = NULL;

    decl_t *last = d;

    while (fgets(line, ASMBUFLEN, filePointer)) {
        line_no++;
        last->next = parseline(line, reference);
        while (last->next)
            last = last->next;
    }
    fclose(filePointer);
    reference->decl = d;
}
