#include "stoc.h"
#include "emulators/fake6502.h"
#include "labels.h"
#include "asm65.h"
#include "reg.h"
#include "tests.h"
#include "exh.h"
#include "main.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#define ASMBUFLEN 255

int org = 0;

rewrite_t r_reference;
rewrite_t r_rewrite;
rewrite_t r_proposal;

Context65 reference;
Context65 rewrite;
Context65 proposal;

uint8_t reference_mem[65536];
uint8_t rewrite_mem[65536];
uint8_t proposal_mem[65536];

uint8_t read6502(Context65 * c, uint16_t address) {
	if(c == &reference) return reference_mem[address];
	if(c == &rewrite)   return rewrite_mem[address];
	if(c == &proposal)  return proposal_mem[address];
	fprintf(stderr, "Unknown Context65; exiting\n");
	exit(1);
}

void write6502(Context65 * c, uint16_t address, uint8_t val) {
	if(c == &reference) reference_mem[address] = val;
	else if(c == &rewrite)   rewrite_mem[address] = val;
	else if(c == &proposal)  proposal_mem[address] = val;
	else {
		fprintf(stderr, "Unknown Context65; exiting\n");
		exit(1);
	}
}

void hexdump(rewrite_t * r) {
	printf("; %d instructions\n", r->length);
	for(int i = 0; i < r->length; i++) {
		uint8_t instr = r->instructions[i].opcode;
		if(is_implied_instruction(instr)) {
			fprintf(stderr, "\t%s\n", opnames[instr]);
		} else if(is_immediate_instruction(instr)) {
			fprintf(stderr, "\t%s #$%02x\n", opnames[instr], r->instructions[i].operand & 0x00ff);
		} else {
			fprintf(stderr, "\t$%02x", instr);
			if(opcode_length(instr) > 1) fprintf(stderr, " $%02x", r->instructions[i].operand & 0x00ff);
			if(opcode_length(instr) > 2) fprintf(stderr, " $%02x", r->instructions[i].operand >> 8);
			fprintf(stderr, "\n");
		}
	}
	fprintf(stderr, "\n");
}

void jobs(char * tmp) {
	int l = atoi(tmp);
	int pid;
	int i;
	for(int i; i < l; i++) {
		sleep(1);
		if(!(pid = fork())) break;
		else printf("%d\n", i);
	}
	srand(time(NULL) + i);
	printf("Launching process %d\n", pid);
}

void parseoption(char * opt) {
	char tmp[100];
	strncpy(tmp, opt, 100);

	if(strncmp("--", opt, 2)) {
		assemble(opt);
	}

	char * key = tmp + 2;
	char * equal = strstr(tmp, "=");
	char * colon = strstr(tmp, ":");

	if(equal){
		char * val = equal + 1;
		equal[0] = '\0';
		mklbl(key, val);
		return;
	}

	char * value = colon + 1;
	if(!strncmp("--org:", tmp, 6)) {
		org = strtol(value, NULL, 16);
		return;
	}
    if(!strncmp("--assemble:", tmp, 11)) {
        assemble(tmp + 11);
        return;
    }
    if(!strncmp("--reg-out:", tmp, 10)) {
        reg_out(tmp + 10);
        return;
    }
    if(!strncmp("--reg-in:", tmp, 9)) {
        reg_in(tmp + 9);
        return;
    }
	if(!strcmp("--hexdump", tmp)) {
		hexdump(&r_reference);
		return;
	}
	if(!strncmp("--jobs:", tmp, 7)) {
		jobs(tmp + 7);
		return;
	}
	if(!strcmp("--exh", tmp)) {
		exhaustive(&r_reference, &r_rewrite);
		return;
	}

	fprintf(stderr, "Unknown option %s\n", opt);
	exit(1);
}

int main(int argc, char ** argv) {
	int i = 0;
	srand(time(NULL));
	for(i = 1; i < argc; i++) {
		char * opt = argv[i];
		if(!strncmp(opt, "--", 2)) parseoption(opt);
		else assemble(opt);
	}

	uint16_t value;
	if(getlbl("hello", &value)) {
		printf("found it %u\n", value);
	}
}

