#include "arch.h"
#include "main.h"
#include "asm.h"
#include "optimization.h"
#include "pick.h"
#include "search.h"
#include "stoc.h"
#include "tests.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

void help(char *progname) {
    printf("Usage:\n\t%s [options...] decl_file [actions...]\n", progname);
    printf("\nPossible options:\n\t-O    optimize for speed, this is the "
           "default one\n");
    printf("\t-Os   optimize for size\n");
    printf("\nPossible actions:\n\t.dce  eliminate dead code\n");
    printf("\t.exh  exhaustive search for an equivalent program\n");
    printf("\t.opt  stochastically optimize the existing program\n");
    printf("\t.dis  disassemble the program in the decl_file\n");
    exit(1);
}

void parseoption(char *opt, char *progname) {
    if (!strcmp(opt, "-Os")) {
        set_optimization(optimize_size);
        return;
    }
    if (!strcmp(opt, "-O")) {
        set_optimization(optimize_speed);
        return;
    }
    fprintf(stderr, "Unknown option %s\n", opt);
    help(progname);
}

void actions(char *opt, stoc_t *c, char *progname) {
    if (!strcmp(opt, ".dis")) {
        hexdump(c);
        return;
    }

    if (!strcmp(opt, ".dce")) {
        deadcodeelim(c);
        return;
    }

    if (!strcmp(opt, ".opt")) {
        stoc_opt(c);
        return;
    }

    if (!strcmp(opt, ".exh")) {
        stoc_exh(c);
        return;
    }

    fprintf(stderr, "Unknown search strategy %s\n", opt);
    help(progname);
}

int main(int argc, char **argv) {
    int i;
    srand(time(NULL));

    pickinit();
    pick_set_common_constants();

    stoc_t c;
    c.clockticks = 0;
    set_optimization(optimize_speed);

    for (i = 1; i < argc; i++) {
        char *opt = argv[i];
        if (opt[0] != '-')
            break;
        parseoption(opt, argv[0]);
    }
    if (!argv[i])
        help(argv[0]);

    readfile(argv[i++], &c);
    measure(&c);
    search_init();

    for (; i < argc; i++) {
        char *opt = argv[i];
        actions(opt, &c, argv[0]);
    }
}
