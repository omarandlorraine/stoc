#include "stoc.h"
#include "emulators/fake6502.h"
#include "labels.h"
#include "asm65.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reg.h"

#define ASMBUFLEN 255

extern int org;
extern rewrite_t r_reference;

bool parse_decimal(char * val, uint16_t * out) {
	char * end = NULL;
    int value = strtol(val, &end, 10);
	*out = value;
	return !strcmp(end, "\n");
}

bool parse_hexadecimal(char * val, uint16_t * out) {
	if(val[0] != '$') return false;
	char * end = NULL;
    int value = strtol(val + 1, &end, 16);
	*out = value;
	return !strcmp(end, "\n");
}

bool parse_operand_absolute(char * operand, uint16_t * out) {
	if(getlbl(operand, out)) return 1;
	return 0;
}

bool parse_operand_immediate(char * operand, uint16_t * out) {
	if(!operand)
		return 0;
	if(operand[0] != '#')
		return 0;
	operand++;
	if(parse_hexadecimal(operand, out)) return 1;
	if(parse_decimal(operand, out)) return 1;
	return 0;
}

void assemble(char * file) {
	int linenumber = 0;
	uint16_t address = org;

	FILE* filePointer;
	char label[ASMBUFLEN];
	char line[ASMBUFLEN];
	char opcode[ASMBUFLEN];
	char operand[ASMBUFLEN];


	filePointer = fopen(file, "r");
	if(!filePointer) {
		fprintf(stderr, "Couldn't open %s\n", file);
		exit(1);
	}

	if(org == 0) {
		fprintf(stderr, "Warning: org is zero; this is probably not what you meant to do.\n");
		fprintf(stderr, "define org before assembling with eg: --org:0200\n");
	}

	for(int pass = 0; pass < 2; pass++) {	
		int rewrite_offset = 0;
		linenumber = 0;
		while(fgets(line, ASMBUFLEN, filePointer)) {
			//fprintf(stderr, line);
			linenumber++;

			char * instr = strstr(line, "\t");

			char * colon = strstr(line, ":");
			if(colon && !pass) {
				colon[0] = '\0';
				mklbli(line, address);
			}

			if(!instr) continue;
			instr++;
			char * operand = strstr(instr, "\t");
			uint16_t * moperand = &r_reference.instructions[rewrite_offset].operand;
			uint8_t * mopcode = &r_reference.instructions[rewrite_offset].opcode;
			if(operand) {
				operand[0] = '\0';
				operand++;
			}
			if(!strcmp(instr, "live-out-registers")) {
				reg_out(operand);
				continue;
			}
			else if(!strcmp(instr, "live-in-registers")) {
				reg_in(operand);
				continue;
			}
			else if(parse_operand_absolute(operand, moperand)) {
				if(zero_page_instruction(instr, mopcode) && *moperand < 256) goto next_instruction;
				if(absolute_instruction(instr, mopcode)) goto next_instruction;
				if(relative_instruction(instr, mopcode)) goto next_instruction;
				goto error;
			}
			else if(parse_operand_immediate(operand, moperand)) {
				if(immediate_instruction(instr, mopcode)) goto next_instruction;
			}
			else if(!operand) {
				if(implied_instruction(instr, mopcode)) goto next_instruction;
			}
			goto error;
next_instruction:
			rewrite_offset++;
			r_reference.length = rewrite_offset;
		}
		rewind(filePointer);
	}
	

	fclose(filePointer);
	return;
error:
	printf("What does this line mean: %s\n", line);
}
