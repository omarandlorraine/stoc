#include "labels.h"
#include "stoc.h"
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLABELS 32

typedef struct {
	char name[LABEL_LEN + 1];
	uint16_t address;
	uint16_t length;
	bool live_in;
	bool live_out;
} label_t;

label_t label[MAXLABELS];

static int labnum = 0;

volatile static bool def = 0;
volatile static bool zpdef = 0;

bool labels_defined() { return def; }
bool zp_labels_defined() { return zpdef; }

void mklbli(char *key, uint16_t value) {
    if (strlen(key) > LABEL_LEN) {
        fprintf(
            stderr,
            "Cannot label %s; the identifier is too long. Maximum length is %d "
            "chars\n",
            key, LABEL_LEN);
        exit(2);
    }

    if (labnum >= MAXLABELS) {
        fprintf(stderr, "Cannot label %s @ %d; too many labels defined.\n", key,
                value);
        exit(2);
    }
    uint16_t nothing;

    if (getlbl(key, &nothing)) {
        fprintf(stderr, "Refusing to redefine %s.\n", key);
        exit(2);
    }

    def = true;
    if (value < 256)
        zpdef = true;

    strcpy(label[labnum].name, key);
    label[labnum].address = value;
    // fprintf(stderr, "%s: %d\n", key, value);
    labnum++;
}

void mklbl(char *key, char *val) {
    char *end = 0;
    int value = strtol(val, &end, 16);
    if (strlen(end)) {
        fprintf(stderr, "Cannot parse hexadecimal value %s\n", val);
        exit(2);
    }
    mklbli(key, value);
}

int getlbl(char *key, uint16_t *val) {
    if (!labnum)
        return 0;
    for (int i = 0; i < labnum; i++) {
        // fprintf(stderr, "%d\t%s\t%d\n", i, labels[i], addresses[i]);
        if (!strcmp(key, label[i].name)) {
            *val = label[i].address;
            return 1;
        }
    }
    return 0;
}

int label_valid(uint16_t val) {
    for (int i = 0; i < labnum; i++) {
        if (val == label[i].address) {
            return 1;
        }
    }
    return 0;
}

int liveout_label(char * key) {
    if (!labnum)
        return 0;
    for (int i = 0; i < labnum; i++) {
        // fprintf(stderr, "%d\t%s\t%d\n", i, labels[i], addresses[i]);
        if (!strcmp(key, label[i].name)) {
            label[i].liveout = true;
            return 1;
        }
    }
    return 0;
}

int livein_label(char * key) {
    if (!labnum)
        return 0;
    for (int i = 0; i < labnum; i++) {
        // fprintf(stderr, "%d\t%s\t%d\n", i, labels[i], addresses[i]);
        if (!strcmp(key, label[i].name)) {
            label[i].livein = true;
            return 1;
        }
    }
    return 0;
}

uint16_t random_label() {
    if (!labnum)
        return 0;
    return label[rand() % labnum].address;
}
