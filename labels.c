#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "labels.h"
#include "stoc.h"
#define MAXLABELS 32

char labels[MAXLABELS][LABEL_LEN + 1];
uint16_t addresses[MAXLABELS];
static int labnum = 0;

void mklbli(char * key, uint16_t value) {
	if(strlen(key) > LABEL_LEN) {
		fprintf(stderr, "Cannot label %s; the identifier is too long. Maximum length is %d chars\n", key, LABEL_LEN);
		exit(2);
	}

	if(labnum >= MAXLABELS) {
		fprintf(stderr, "Cannot label %s @ %d; too many labels defined.\n", key, value);
		exit(2);
	}
	uint16_t nothing;

	if(getlbl(key, &nothing)) {
		fprintf(stderr, "Refusing to redefine %s.\n", key);
		exit(2);
	}

	strcpy(labels[labnum], key);
	addresses[labnum] = value;
	//fprintf(stderr, "%s: %d\n", key, value);
	labnum++;
}

void mklbl(char * key, char * val) {
	char * end = 0;
	int value = strtol(val, &end, 16);
	if(strlen(end)) {
		fprintf(stderr, "Cannot parse hexadecimal value %s\n", val);
		exit(2);
	}
	mklbli(key, value);
}

int getlbl(char * key, uint16_t * val) {
	if(!labnum) return 0;
	for(int i = 0; i < labnum; i++) {
		//fprintf(stderr, "%d\t%s\t%d\n", i, labels[i], addresses[i]);
		if(!strcmp(key, labels[i])) {
			*val = addresses[i];
			return 1;
		}
	}
	return 0;
}

int label_valid(uint16_t val) {
	for(int i = 0; i < labnum; i++) {
		if(val == addresses[i]) {
			printf("Address %02x is valid, named %s.\n", val, labels[i]);
			return 1;
		}
	}
	return 0;
}

uint16_t random_label() {
	return rand() % labnum;
}
