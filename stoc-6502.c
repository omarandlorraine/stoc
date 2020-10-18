#include "stoc.h"
#include "6502.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


bool opcode_legal_p(uint8_t op) {
	switch(op) {
	case 0x01:
	case 0x05:
	case 0x06:
	case 0x08:
	case 0x09:
	case 0x0a:
	case 0x0d:
	case 0x0e:
	case 0x10:
	case 0x11:
	case 0x15:
	case 0x16:
	case 0x18:
	case 0x19:
	case 0x1d:
	case 0x1e:
	case 0x20:
	case 0x21:
	case 0x24:
	case 0x25:
	case 0x26:
	case 0x28:
	case 0x29:
	case 0x2a:
	case 0x2c:
	case 0x2d:
	case 0x2e:
	case 0x30:
	case 0x31:
	case 0x35:
	case 0x36:
	case 0x38:
	case 0x39:
	case 0x3d:
	case 0x3e:
	case 0x40:
	case 0x41:
	case 0x45:
	case 0x46:
	case 0x48:
	case 0x49:
	case 0x4a:
	case 0x4c:
	case 0x4d:
	case 0x4e:
	case 0x50:
	case 0x51:
	case 0x55:
	case 0x56:
	case 0x58:
	case 0x59:
	case 0x5d:
	case 0x5e:
	case 0x60:
	case 0x61:
	case 0x65:
	case 0x66:
	case 0x68:
	case 0x69:
	case 0x6a:
	case 0x6c:
	case 0x6d:
	case 0x6e:
	case 0x70:
	case 0x71:
	case 0x75:
	case 0x76:
	case 0x78:
	case 0x79:
	case 0x7d:
	case 0x7e:
	case 0x81:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0x88:
	case 0x8a:
	case 0x8c:
	case 0x8d:
	case 0x8e:
	case 0x90:
	case 0x91:
	case 0x94:
	case 0x95:
	case 0x96:
	case 0x98:
	case 0x99:
	case 0x9a:
	case 0x9d:
	case 0xa0:
	case 0xa1:
	case 0xa2:
	case 0xa4:
	case 0xa5:
	case 0xa6:
	case 0xa8:
	case 0xa9:
	case 0xaa:
	case 0xac:
	case 0xad:
	case 0xae:
	case 0xb0:
	case 0xb1:
	case 0xb4:
	case 0xb5:
	case 0xb6:
	case 0xb8:
	case 0xb9:
	case 0xba:
	case 0xbc:
	case 0xbd:
	case 0xbe:
	case 0xc0:
	case 0xc1:
	case 0xc4:
	case 0xc5:
	case 0xc6:
	case 0xc8:
	case 0xc9:
	case 0xca:
	case 0xcc:
	case 0xcd:
	case 0xce:
	case 0xd0:
	case 0xd1:
	case 0xd5:
	case 0xd6:
	case 0xd8:
	case 0xd9:
	case 0xdd:
	case 0xde:
	case 0xe0:
	case 0xe1:
	case 0xe4:
	case 0xe5:
	case 0xe6:
	case 0xe8:
	case 0xe9:
	case 0xea:
	case 0xec:
	case 0xed:
	case 0xee:
	case 0xf0:
	case 0xf1:
	case 0xf5:
	case 0xf6:
	case 0xf8:
	case 0xf9:
	case 0xfd:
	case 0xfe:
		return true;
	default:
		return false;
	}
}
int opcode_length(uint8_t op) {
	switch(op) {
	case 0x01: return 2; // ora indirect-x
	case 0x05: return 2; // ora zero-page
	case 0x06: return 2; // asl zero-page
	case 0x08: return 1; // php implied
	case 0x09: return 2; // ora immediate
	case 0x0a: return 1; // asl accumulator
	case 0x0d: return 3; // ora absolute
	case 0x0e: return 3; // asl absolute
	case 0x10: return 2; // bpl relative
	case 0x11: return 2; // ora indirect-y
	case 0x15: return 2; // ora zero-page-x
	case 0x16: return 2; // asl zero-page-x
	case 0x18: return 1; // clc implied
	case 0x19: return 3; // ora absolute-y
	case 0x1d: return 3; // ora absolute-x
	case 0x1e: return 3; // asl absolute-x
	case 0x20: return 1; // jsr implied
	case 0x21: return 2; // and indirect-x
	case 0x24: return 2; // bit zero-page
	case 0x25: return 2; // and zero-page
	case 0x26: return 2; // rol zero-page
	case 0x28: return 1; // plp implied
	case 0x29: return 2; // and immediate
	case 0x2a: return 1; // rol accumulator
	case 0x2c: return 3; // bit absolute
	case 0x2d: return 3; // and absolute
	case 0x2e: return 3; // rol absolute
	case 0x30: return 2; // bmi relative
	case 0x31: return 2; // and indirect-y
	case 0x35: return 2; // and zero-page-x
	case 0x36: return 2; // rol zero-page-x
	case 0x38: return 1; // sec implied
	case 0x39: return 3; // and absolute-y
	case 0x3d: return 3; // and absolute-x
	case 0x3e: return 3; // rol absolute-x
	case 0x40: return 1; // rti implied
	case 0x41: return 2; // eor indirect-x
	case 0x45: return 2; // eor zero-page
	case 0x46: return 2; // lsr zero-page
	case 0x48: return 1; // pha implied
	case 0x49: return 2; // eor immediate
	case 0x4a: return 1; // lsr implied
	case 0x4c: return 3; // jmp absolute
	case 0x4d: return 3; // eor absolute
	case 0x4e: return 3; // lsr absolute
	case 0x50: return 2; // bvc relative
	case 0x51: return 2; // eor indirect-y
	case 0x55: return 2; // eor zero-page-x
	case 0x56: return 2; // lsr zero-page-x
	case 0x58: return 1; // cli implied
	case 0x59: return 3; // eor absolute-y
	case 0x5d: return 3; // eor absolute-x
	case 0x5e: return 3; // lsr absolute-x
	case 0x60: return 1; // rts implied
	case 0x61: return 2; // adc indirect-x
	case 0x65: return 2; // adc zero-page
	case 0x66: return 2; // ror zero-page
	case 0x68: return 1; // pla implied
	case 0x69: return 2; // adc immediate
	case 0x6a: return 1; // ror accumulator
	case 0x6c: return 3; // jmp indirect
	case 0x6d: return 3; // adc absolute
	case 0x6e: return 3; // ror absolute
	case 0x70: return 2; // bvs relative
	case 0x71: return 2; // adc indirect-y
	case 0x75: return 2; // adc zero-page-x
	case 0x76: return 2; // ror zero-page-x
	case 0x78: return 1; // sei implied
	case 0x79: return 3; // adc absolute-y
	case 0x7d: return 3; // adc absolute-x
	case 0x7e: return 3; // ror absolute-x
	case 0x81: return 2; // sta indirect-x
	case 0x84: return 2; // sty zero-page
	case 0x85: return 2; // sta zero-page
	case 0x86: return 2; // stx zero-page
	case 0x88: return 1; // dey implied
	case 0x8a: return 1; // txa implied
	case 0x8c: return 3; // sty absolute
	case 0x8d: return 3; // sta absolute
	case 0x8e: return 3; // stx absolute
	case 0x90: return 2; // bcc relative
	case 0x91: return 2; // sta indirect-y
	case 0x94: return 2; // sty zero-page-x
	case 0x95: return 2; // sta zero-page-x
	case 0x96: return 2; // stx zero-page-y
	case 0x98: return 1; // tya implied
	case 0x99: return 3; // sta absolute-y
	case 0x9a: return 1; // txs implied
	case 0x9d: return 3; // sta absolute-x
	case 0xa0: return 2; // ldy immediate
	case 0xa1: return 2; // lda indirect-x
	case 0xa2: return 2; // ldx immediate
	case 0xa4: return 2; // ldy zero-page
	case 0xa5: return 2; // lda zero-page
	case 0xa6: return 2; // ldx zero-page
	case 0xa8: return 1; // tay implied
	case 0xa9: return 2; // lda immediate
	case 0xaa: return 1; // tax implied
	case 0xac: return 3; // ldy absolute
	case 0xad: return 3; // lda absolute
	case 0xae: return 3; // ldx absolute
	case 0xb0: return 2; // bcs relative
	case 0xb1: return 2; // lda indirect-y
	case 0xb4: return 2; // ldy zero-page-x
	case 0xb5: return 2; // lda zero-page-x
	case 0xb6: return 2; // ldx zero-page-y
	case 0xb8: return 1; // clv implied
	case 0xb9: return 3; // lda absolute-y
	case 0xba: return 1; // tsx implied
	case 0xbc: return 3; // ldy absolute-x
	case 0xbd: return 3; // lda absolute-x
	case 0xbe: return 3; // ldx absolute-y
	case 0xc0: return 2; // cpy immediate
	case 0xc1: return 2; // cmp indirect-x
	case 0xc4: return 2; // cpy zero-page
	case 0xc5: return 2; // cmp zero-page
	case 0xc6: return 2; // dec zero-page
	case 0xc8: return 1; // iny implied
	case 0xc9: return 2; // cmp immediate
	case 0xca: return 1; // dex implied
	case 0xcc: return 3; // cpy absolute
	case 0xcd: return 3; // cmp absolute
	case 0xce: return 3; // dec absolute
	case 0xd0: return 2; // bne relative
	case 0xd1: return 2; // cmp indirect-y
	case 0xd5: return 2; // cmp zero-page-x
	case 0xd6: return 2; // dec zero-page-x
	case 0xd8: return 1; // cld implied
	case 0xd9: return 3; // cmp absolute-y
	case 0xdd: return 3; // cmp absolute-x
	case 0xde: return 3; // dec absolute-x
	case 0xe0: return 2; // cpx immediate
	case 0xe1: return 2; // sbc indirect-x
	case 0xe4: return 2; // cpx zero-page
	case 0xe5: return 2; // sbc zero-page
	case 0xe6: return 2; // inc zero-page
	case 0xe8: return 1; // inx implied
	case 0xe9: return 2; // sbc immediate
	case 0xea: return 1; // nop implied
	case 0xec: return 3; // cpx absolute
	case 0xed: return 3; // sbc absolute
	case 0xee: return 3; // inc absolute
	case 0xf0: return 2; // beq relative
	case 0xf1: return 2; // sbc indirect-y
	case 0xf5: return 2; // sbc zero-page-x
	case 0xf6: return 2; // inc zero-page-x
	case 0xf8: return 1; // sed implied
	case 0xf9: return 3; // sbc absolute-y
	case 0xfd: return 3; // sbc absolute-x
	case 0xfe: return 3; // inc absolute-x
	default:
		return 0;
	}
}
int opcode_branch_p(uint8_t op) {
	switch(op) {
	case 0x10:	// bpl relative
	case 0x30:	// bmi relative
	case 0x50:	// bvc relative
	case 0x70:	// bvs relative
	case 0x90:	// bcc relative
	case 0xb0:	// bcs relative
	case 0xd0:	// bne relative
	case 0xf0:	// beq relative
		return true;
	default:
		return false;
	}
}
int is_absolute_instruction(uint8_t op) {
	switch(op) {
	case 0x0d:
	case 0x0e:
	case 0x2c:
	case 0x2d:
	case 0x2e:
	case 0x4c:
	case 0x4d:
	case 0x4e:
	case 0x6d:
	case 0x6e:
	case 0x8c:
	case 0x8d:
	case 0x8e:
	case 0xac:
	case 0xad:
	case 0xae:
	case 0xcc:
	case 0xcd:
	case 0xce:
	case 0xec:
	case 0xed:
	case 0xee:
		return 1;
	default:		return 0;
	}
}
uint8_t absolute_instruction(char * op) {
	if(!strncmp(op, "ora", 3)) return 0x0d;
	if(!strncmp(op, "asl", 3)) return 0x0e;
	if(!strncmp(op, "bit", 3)) return 0x2c;
	if(!strncmp(op, "and", 3)) return 0x2d;
	if(!strncmp(op, "rol", 3)) return 0x2e;
	if(!strncmp(op, "jmp", 3)) return 0x4c;
	if(!strncmp(op, "eor", 3)) return 0x4d;
	if(!strncmp(op, "lsr", 3)) return 0x4e;
	if(!strncmp(op, "adc", 3)) return 0x6d;
	if(!strncmp(op, "ror", 3)) return 0x6e;
	if(!strncmp(op, "sty", 3)) return 0x8c;
	if(!strncmp(op, "sta", 3)) return 0x8d;
	if(!strncmp(op, "stx", 3)) return 0x8e;
	if(!strncmp(op, "ldy", 3)) return 0xac;
	if(!strncmp(op, "lda", 3)) return 0xad;
	if(!strncmp(op, "ldx", 3)) return 0xae;
	if(!strncmp(op, "cpy", 3)) return 0xcc;
	if(!strncmp(op, "cmp", 3)) return 0xcd;
	if(!strncmp(op, "dec", 3)) return 0xce;
	if(!strncmp(op, "cpx", 3)) return 0xec;
	if(!strncmp(op, "sbc", 3)) return 0xed;
	if(!strncmp(op, "inc", 3)) return 0xee;
	fprintf(stderr, "No absolute addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_absolute_x_instruction(uint8_t op) {
	switch(op) {
	case 0x1d:
	case 0x1e:
	case 0x3d:
	case 0x3e:
	case 0x5d:
	case 0x5e:
	case 0x7d:
	case 0x7e:
	case 0x9d:
	case 0xbc:
	case 0xbd:
	case 0xdd:
	case 0xde:
	case 0xfd:
	case 0xfe:
		return 1;
	default:		return 0;
	}
}
uint8_t absolute_x_instruction(char * op) {
	if(!strncmp(op, "ora", 3)) return 0x1d;
	if(!strncmp(op, "asl", 3)) return 0x1e;
	if(!strncmp(op, "and", 3)) return 0x3d;
	if(!strncmp(op, "rol", 3)) return 0x3e;
	if(!strncmp(op, "eor", 3)) return 0x5d;
	if(!strncmp(op, "lsr", 3)) return 0x5e;
	if(!strncmp(op, "adc", 3)) return 0x7d;
	if(!strncmp(op, "ror", 3)) return 0x7e;
	if(!strncmp(op, "sta", 3)) return 0x9d;
	if(!strncmp(op, "ldy", 3)) return 0xbc;
	if(!strncmp(op, "lda", 3)) return 0xbd;
	if(!strncmp(op, "cmp", 3)) return 0xdd;
	if(!strncmp(op, "dec", 3)) return 0xde;
	if(!strncmp(op, "sbc", 3)) return 0xfd;
	if(!strncmp(op, "inc", 3)) return 0xfe;
	fprintf(stderr, "No absolute-x addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_absolute_y_instruction(uint8_t op) {
	switch(op) {
	case 0x19:
	case 0x39:
	case 0x59:
	case 0x79:
	case 0x99:
	case 0xb9:
	case 0xbe:
	case 0xd9:
	case 0xf9:
		return 1;
	default:		return 0;
	}
}
uint8_t absolute_y_instruction(char * op) {
	if(!strncmp(op, "ora", 3)) return 0x19;
	if(!strncmp(op, "and", 3)) return 0x39;
	if(!strncmp(op, "eor", 3)) return 0x59;
	if(!strncmp(op, "adc", 3)) return 0x79;
	if(!strncmp(op, "sta", 3)) return 0x99;
	if(!strncmp(op, "lda", 3)) return 0xb9;
	if(!strncmp(op, "ldx", 3)) return 0xbe;
	if(!strncmp(op, "cmp", 3)) return 0xd9;
	if(!strncmp(op, "sbc", 3)) return 0xf9;
	fprintf(stderr, "No absolute-y addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_indirect_instruction(uint8_t op) {
	switch(op) {
	case 0x6c:
		return 1;
	default:		return 0;
	}
}
uint8_t indirect_instruction(char * op) {
	if(!strncmp(op, "jmp", 3)) return 0x6c;
	fprintf(stderr, "No indirect addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_zero_page_instruction(uint8_t op) {
	switch(op) {
	case 0x05:
	case 0x06:
	case 0x24:
	case 0x25:
	case 0x26:
	case 0x45:
	case 0x46:
	case 0x65:
	case 0x66:
	case 0x84:
	case 0x85:
	case 0x86:
	case 0xa4:
	case 0xa5:
	case 0xa6:
	case 0xc4:
	case 0xc5:
	case 0xc6:
	case 0xe4:
	case 0xe5:
	case 0xe6:
		return 1;
	default:		return 0;
	}
}
uint8_t zero_page_instruction(char * op) {
	if(!strncmp(op, "ora", 3)) return 0x05;
	if(!strncmp(op, "asl", 3)) return 0x06;
	if(!strncmp(op, "bit", 3)) return 0x24;
	if(!strncmp(op, "and", 3)) return 0x25;
	if(!strncmp(op, "rol", 3)) return 0x26;
	if(!strncmp(op, "eor", 3)) return 0x45;
	if(!strncmp(op, "lsr", 3)) return 0x46;
	if(!strncmp(op, "adc", 3)) return 0x65;
	if(!strncmp(op, "ror", 3)) return 0x66;
	if(!strncmp(op, "sty", 3)) return 0x84;
	if(!strncmp(op, "sta", 3)) return 0x85;
	if(!strncmp(op, "stx", 3)) return 0x86;
	if(!strncmp(op, "ldy", 3)) return 0xa4;
	if(!strncmp(op, "lda", 3)) return 0xa5;
	if(!strncmp(op, "ldx", 3)) return 0xa6;
	if(!strncmp(op, "cpy", 3)) return 0xc4;
	if(!strncmp(op, "cmp", 3)) return 0xc5;
	if(!strncmp(op, "dec", 3)) return 0xc6;
	if(!strncmp(op, "cpx", 3)) return 0xe4;
	if(!strncmp(op, "sbc", 3)) return 0xe5;
	if(!strncmp(op, "inc", 3)) return 0xe6;
	fprintf(stderr, "No zero-page addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_zero_page_x_instruction(uint8_t op) {
	switch(op) {
	case 0x15:
	case 0x16:
	case 0x35:
	case 0x36:
	case 0x55:
	case 0x56:
	case 0x75:
	case 0x76:
	case 0x94:
	case 0x95:
	case 0xb4:
	case 0xb5:
	case 0xd5:
	case 0xd6:
	case 0xf5:
	case 0xf6:
		return 1;
	default:		return 0;
	}
}
uint8_t zero_page_x_instruction(char * op) {
	if(!strncmp(op, "ora", 3)) return 0x15;
	if(!strncmp(op, "asl", 3)) return 0x16;
	if(!strncmp(op, "and", 3)) return 0x35;
	if(!strncmp(op, "rol", 3)) return 0x36;
	if(!strncmp(op, "eor", 3)) return 0x55;
	if(!strncmp(op, "lsr", 3)) return 0x56;
	if(!strncmp(op, "adc", 3)) return 0x75;
	if(!strncmp(op, "ror", 3)) return 0x76;
	if(!strncmp(op, "sty", 3)) return 0x94;
	if(!strncmp(op, "sta", 3)) return 0x95;
	if(!strncmp(op, "ldy", 3)) return 0xb4;
	if(!strncmp(op, "lda", 3)) return 0xb5;
	if(!strncmp(op, "cmp", 3)) return 0xd5;
	if(!strncmp(op, "dec", 3)) return 0xd6;
	if(!strncmp(op, "sbc", 3)) return 0xf5;
	if(!strncmp(op, "inc", 3)) return 0xf6;
	fprintf(stderr, "No zero-page-x addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_zero_page_y_instruction(uint8_t op) {
	switch(op) {
	case 0x96:
	case 0xb6:
		return 1;
	default:		return 0;
	}
}
uint8_t zero_page_y_instruction(char * op) {
	if(!strncmp(op, "stx", 3)) return 0x96;
	if(!strncmp(op, "ldx", 3)) return 0xb6;
	fprintf(stderr, "No zero-page-y addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_indirect_x_instruction(uint8_t op) {
	switch(op) {
	case 0x01:
	case 0x21:
	case 0x41:
	case 0x61:
	case 0x81:
	case 0xa1:
	case 0xc1:
	case 0xe1:
		return 1;
	default:		return 0;
	}
}
uint8_t indirect_x_instruction(char * op) {
	if(!strncmp(op, "ora", 3)) return 0x01;
	if(!strncmp(op, "and", 3)) return 0x21;
	if(!strncmp(op, "eor", 3)) return 0x41;
	if(!strncmp(op, "adc", 3)) return 0x61;
	if(!strncmp(op, "sta", 3)) return 0x81;
	if(!strncmp(op, "lda", 3)) return 0xa1;
	if(!strncmp(op, "cmp", 3)) return 0xc1;
	if(!strncmp(op, "sbc", 3)) return 0xe1;
	fprintf(stderr, "No indirect-x addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_indirect_y_instruction(uint8_t op) {
	switch(op) {
	case 0x11:
	case 0x31:
	case 0x51:
	case 0x71:
	case 0x91:
	case 0xb1:
	case 0xd1:
	case 0xf1:
		return 1;
	default:		return 0;
	}
}
uint8_t indirect_y_instruction(char * op) {
	if(!strncmp(op, "ora", 3)) return 0x11;
	if(!strncmp(op, "and", 3)) return 0x31;
	if(!strncmp(op, "eor", 3)) return 0x51;
	if(!strncmp(op, "adc", 3)) return 0x71;
	if(!strncmp(op, "sta", 3)) return 0x91;
	if(!strncmp(op, "lda", 3)) return 0xb1;
	if(!strncmp(op, "cmp", 3)) return 0xd1;
	if(!strncmp(op, "sbc", 3)) return 0xf1;
	fprintf(stderr, "No indirect-y addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_immediate_instruction(uint8_t op) {
	switch(op) {
	case 0x09:
	case 0x29:
	case 0x49:
	case 0x69:
	case 0xa0:
	case 0xa2:
	case 0xa9:
	case 0xc0:
	case 0xc9:
	case 0xe0:
	case 0xe9:
		return 1;
	default:		return 0;
	}
}
uint8_t immediate_instruction(char * op) {
	if(!strncmp(op, "ora", 3)) return 0x09;
	if(!strncmp(op, "and", 3)) return 0x29;
	if(!strncmp(op, "eor", 3)) return 0x49;
	if(!strncmp(op, "adc", 3)) return 0x69;
	if(!strncmp(op, "ldy", 3)) return 0xa0;
	if(!strncmp(op, "ldx", 3)) return 0xa2;
	if(!strncmp(op, "lda", 3)) return 0xa9;
	if(!strncmp(op, "cpy", 3)) return 0xc0;
	if(!strncmp(op, "cmp", 3)) return 0xc9;
	if(!strncmp(op, "cpx", 3)) return 0xe0;
	if(!strncmp(op, "sbc", 3)) return 0xe9;
	fprintf(stderr, "No immediate addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_implied_instruction(uint8_t op) {
	switch(op) {
	case 0x08:
	case 0x18:
	case 0x20:
	case 0x28:
	case 0x38:
	case 0x40:
	case 0x48:
	case 0x4a:
	case 0x58:
	case 0x60:
	case 0x68:
	case 0x78:
	case 0x88:
	case 0x8a:
	case 0x98:
	case 0x9a:
	case 0xa8:
	case 0xaa:
	case 0xb8:
	case 0xba:
	case 0xc8:
	case 0xca:
	case 0xd8:
	case 0xe8:
	case 0xea:
	case 0xf8:
		return 1;
	default:		return 0;
	}
}
uint8_t implied_instruction(char * op) {
	if(!strncmp(op, "php", 3)) return 0x08;
	if(!strncmp(op, "clc", 3)) return 0x18;
	if(!strncmp(op, "jsr", 3)) return 0x20;
	if(!strncmp(op, "plp", 3)) return 0x28;
	if(!strncmp(op, "sec", 3)) return 0x38;
	if(!strncmp(op, "rti", 3)) return 0x40;
	if(!strncmp(op, "pha", 3)) return 0x48;
	if(!strncmp(op, "lsr", 3)) return 0x4a;
	if(!strncmp(op, "cli", 3)) return 0x58;
	if(!strncmp(op, "rts", 3)) return 0x60;
	if(!strncmp(op, "pla", 3)) return 0x68;
	if(!strncmp(op, "sei", 3)) return 0x78;
	if(!strncmp(op, "dey", 3)) return 0x88;
	if(!strncmp(op, "txa", 3)) return 0x8a;
	if(!strncmp(op, "tya", 3)) return 0x98;
	if(!strncmp(op, "txs", 3)) return 0x9a;
	if(!strncmp(op, "tay", 3)) return 0xa8;
	if(!strncmp(op, "tax", 3)) return 0xaa;
	if(!strncmp(op, "clv", 3)) return 0xb8;
	if(!strncmp(op, "tsx", 3)) return 0xba;
	if(!strncmp(op, "iny", 3)) return 0xc8;
	if(!strncmp(op, "dex", 3)) return 0xca;
	if(!strncmp(op, "cld", 3)) return 0xd8;
	if(!strncmp(op, "inx", 3)) return 0xe8;
	if(!strncmp(op, "nop", 3)) return 0xea;
	if(!strncmp(op, "sed", 3)) return 0xf8;
	fprintf(stderr, "No implied addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_relative_instruction(uint8_t op) {
	switch(op) {
	case 0x10:
	case 0x30:
	case 0x50:
	case 0x70:
	case 0x90:
	case 0xb0:
	case 0xd0:
	case 0xf0:
		return 1;
	default:		return 0;
	}
}
uint8_t relative_instruction(char * op) {
	if(!strncmp(op, "bpl", 3)) return 0x10;
	if(!strncmp(op, "bmi", 3)) return 0x30;
	if(!strncmp(op, "bvc", 3)) return 0x50;
	if(!strncmp(op, "bvs", 3)) return 0x70;
	if(!strncmp(op, "bcc", 3)) return 0x90;
	if(!strncmp(op, "bcs", 3)) return 0xb0;
	if(!strncmp(op, "bne", 3)) return 0xd0;
	if(!strncmp(op, "beq", 3)) return 0xf0;
	fprintf(stderr, "No relative addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
int is_accumulator_instruction(uint8_t op) {
	switch(op) {
	case 0x0a:
	case 0x2a:
	case 0x6a:
		return 1;
	default:		return 0;
	}
}
uint8_t accumulator_instruction(char * op) {
	if(!strncmp(op, "asl", 3)) return 0x0a;
	if(!strncmp(op, "rol", 3)) return 0x2a;
	if(!strncmp(op, "ror", 3)) return 0x6a;
	fprintf(stderr, "No accumulator addressing mode for instruction \"%s\"\n", op);
	exit(1);
}
