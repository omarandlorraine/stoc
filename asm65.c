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

uint8_t parse_operand(char * val) {
    int value;
    if(val[0] == '$') {
        char * end = NULL;
        value = strtol(val + 1, &end, 16);
        if(strcmp(end, "\n")) {
            fprintf(stderr, "Cannot parse hexadecimal value %s\n", val);
            exit(2);
        }
    } else {
        char * end = NULL;
        value = strtol(val, &end, 10);
        if(strcmp(end, "\n")) {
            fprintf(stderr, "Cannot parse decimal value %s\n", val);
            exit(2);
        }
    }
    return value;
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
			else if(!operand) {
				// this line has no operand, therefore the addressing mode must be "implied"
				r_reference.instructions[rewrite_offset].opcode = implied_instruction(instr);
				address++;
			} else if(operand[0] == '#') {
				// this line has the addressing mode "immediate"
				r_reference.instructions[rewrite_offset].opcode = immediate_instruction(instr);
				r_reference.instructions[rewrite_offset].operand = parse_operand(operand + 1);
				address += 2;
			} else if(operand[0] == '(' && strstr(operand, "),y")) {
				operand++;
				r_reference.instructions[rewrite_offset].opcode = indirect_y_instruction(instr);
				strstr(operand, "),y")[0] = '\0';
				if(!getlbl(operand + 1, &r_reference.instructions[rewrite_offset].operand)) {
					fprintf(stderr, "No such label as %s\ndouble check line %u\n", operand, linenumber);
					exit(1);
				}
			} else if(operand[0] == '(' && strstr(operand, ",x)")) {
				operand++;
				r_reference.instructions[rewrite_offset].opcode = indirect_x_instruction(instr);
				strstr(operand, ",x)")[0] = '\0';
				if(!getlbl(operand + 1, &r_reference.instructions[rewrite_offset].operand)) {
					fprintf(stderr, "No such label as %s\ndouble check line %u\n", operand, linenumber);
					exit(1);
				}
			}
			rewrite_offset++;
			r_reference.length = rewrite_offset;
			//printf("%s\n", line);
		}
		rewind(filePointer);
	}

	fclose(filePointer);
}
