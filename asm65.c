#include "stoc.h"
#include "labels.h"
#include "asm65.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "reg.h"

#define ASMBUFLEN 255

extern int org;
extern context_t reference;

char label[ASMBUFLEN] = {0};
char opcode[ASMBUFLEN] = {0};
char operand[ASMBUFLEN] = {0};
uint8_t mopcode;
uint16_t moperand;

bool alpha(char c) {
	if(c < 'a') return false;
	if(c > 'z') return false;
	return true;
}

bool whitespace(char c) {
	if(c == ' ') return true;
	if(c == '\t') return true;
	return false;
}

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
	if(strcmp(end, "\n")) {
		return 0;
	} else {
		return 1;
	}
}

bool parse_operand_zeropage(char * operand, uint16_t * out) {
	if(parse_hexadecimal(operand, out) && *out < 256) return 1;
	if(parse_decimal(operand, out) && *out < 256) return 1;
	if(getlbl(operand, out) && *out < 256) return 1;
	return 0;
}

bool parse_operand_absolute(char * operand, uint16_t * out) {
	if(getlbl(operand, out)) return 1;
	if(parse_hexadecimal(operand, out)) return 1;
	return 0;
}

bool parse_operand_immediate() {
	char * op = operand;
	if(!op) {
		return 0;
	}
	if(op[0] != '#') {
		return 0;
	}
	op++;
	if(parse_hexadecimal(op, &moperand)) return 1;
	if(parse_decimal(op, &moperand)) return 1;
	return 0;
}

int swallow_whitespace(char ** c) {
	if(!whitespace(**c))
		return 0;
	while(whitespace(**c)) {
		++*c;
	}
	return 1;
}

int begin_line(char ** c) {
	char * dst = label;
	while(!whitespace(**c)) {
		*dst++ = **c;
		++*c;
	}
	while(whitespace(**c))
		++*c;
	return 1;
}

int read_opcode(char ** c) {
	char * dst = opcode;
	while(!whitespace(**c)) {
		if(!**c) return strlen(opcode);
		*dst++ = **c;
		*dst = '\0';
		++*c;
	}
	return swallow_whitespace(c);
}

int read_immediate_operand(char ** c) {
	char * dst = operand;
	while(!alpha(**c)) {
		if(!**c) return strlen(operand);
		*dst++ = **c;
		*dst = '\0';
		++*c;
	}
	return 1;
}

int read_alpha(char ** c) {
	char * dst = operand;
	while(alpha(**c)) {
		if(!**c) return strlen(operand);
		*dst++ = **c;
		*dst = '\0';
		++*c;
	}
	return 1;
}

int end_line(char ** c) {
	while(whitespace(**c)) {
		++*c;
	}
	if(**c == '\n') return 1;
	if(!**c) return 1;
	return 0;
}

int directive(char * line) {
	if(!begin_line(&line)) return 0;
	if(!read_opcode(&line)) return 0;

	if(!strcmp(opcode, "liveoutregisters")) {
		if(!read_alpha(&line)) return 0;
		reg_out(operand);
		return 1;
	}
	else if(!strcmp(opcode, "liveinregisters")) {
		if(!read_alpha(&line)) return 0;
		reg_in(operand);
		return 1;
	}
	else if(!strcmp(opcode, "org")) {
		uint16_t norg;
		swallow_whitespace(&line);
		if(!parse_operand_absolute(line, &norg)) {
			fprintf(stderr, "Couldn't parse argument to org: \"%s\"\n", line);
			exit(1);
		}
		org = norg;
		return 1;
	}
	return 0;
}

int immediate(char * line) {
	if(!begin_line(&line)) return 0;
	if(!read_opcode(&line)) return 0;
	if(!read_immediate_operand(&line)) return 0;
	if(!end_line(&line)) return 0;
	if(!parse_operand_immediate()) return 0;
	if(immediate_instruction(opcode, &mopcode)) return 1;
	return 0;
}

int relative(char * line) {
	if(!begin_line(&line)) return 0;
	if(!read_opcode(&line)) return 0;
	if(!read_alpha(&line)) return 0;
	if(!end_line(&line)) return 0;
	return relative_instruction(opcode, &mopcode);
}

static int zeropage(char * line) {
	if(!begin_line(&line)) return 0;
	if(!read_opcode(&line)) return 0;
	if(!read_alpha(&line)) return 0;
	if(!end_line(&line)) return 0;
	return zero_page_instruction(opcode, &mopcode);
}

int implied(char * line) {
	if(!begin_line(&line)) return 0;
	if(!read_opcode(&line)) return 0;
	if(!end_line(&line)) return 0;
	if(implied_instruction(opcode, &mopcode)) return 1;
	return 0;
}

int absolute(char * line) {
	if(!begin_line(&line)) return 0;
	if(!read_opcode(&line)) return 0;
	if(!read_alpha(&line)) return 0;
	if(!end_line(&line)) return 0;
	return absolute_instruction(opcode, &mopcode);
}

void assemble(char * file) {
	int linenumber = 0;
	uint16_t address = org;

	FILE* filePointer;
	char line[ASMBUFLEN];

	filePointer = fopen(file, "r");
	if(!filePointer) {
		fprintf(stderr, "Couldn't open %s\n", file);
		exit(1);
	}

	for(int pass = 0; pass < 2; pass++) {	
		int rewrite_offset = 0;
		linenumber = 0;
		while(fgets(line, ASMBUFLEN, filePointer)) {
			linenumber++;
			label[0] = '\0';
			opcode[0] = '\0';
			operand[0] = '\0';

			if(directive(line)) continue;

			if(implied(line))   goto next_instruction;
			if(immediate(line)) goto next_instruction;
			if(zeropage(line))  goto next_instruction;
			if(relative(line))  goto next_instruction;
			if(absolute(line))  goto next_instruction;
		
			goto error;
	
next_instruction:
			reference.program.instructions[rewrite_offset].operand = moperand;
			reference.program.instructions[rewrite_offset].opcode = mopcode;
			rewrite_offset++;
			reference.program.length = rewrite_offset;
		}
		rewind(filePointer);
	}
	

	fclose(filePointer);
	return;
error:
	fprintf(stderr, "Error on line %d\nlabel = \"%s\"\nopcode = \"%s\"\noperand = \"%s\"\n", linenumber, label, opcode, operand);
	printf("What does this line mean: %s %s\nline %d\n", line, operand, linenumber);
	exit(1);
}
