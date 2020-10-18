#pragma once
//6502 defines
// #define UNDOCUMENTED //when this is defined, undocumented opcodes are handled.
                        //otherwise, they're simply treated as NOPs.

// #define NES_CPU      //when this is defined, the binary-coded decimal (BCD)
                     //status flag is not honored by ADC and SBC. the 2A03
                     //CPU in the Nintendo Entertainment System does not
                     //support BCD operation.

#define FLAG_CARRY     0x01
#define FLAG_ZERO      0x02
#define FLAG_INTERRUPT 0x04
#define FLAG_DECIMAL   0x08
#define FLAG_BREAK     0x10
#define FLAG_CONSTANT  0x20
#define FLAG_OVERFLOW  0x40
#define FLAG_SIGN      0x80

#define BASE_STACK     0x100

#define saveaccum(c, n) c->a = (uint8_t)((n) & 0x00FF)

//flag modifier macros
#define setcarry(c) c->status |= FLAG_CARRY
#define clearcarry(c) c->status &= (~FLAG_CARRY)
#define setzero(c) c->status |= FLAG_ZERO
#define clearzero(c) c->status &= (~FLAG_ZERO)
#define setinterrupt(c) c->status |= FLAG_INTERRUPT
#define clearinterrupt(c) c->status &= (~FLAG_INTERRUPT)
#define setdecimal(c) c->status |= FLAG_DECIMAL
#define cleardecimal(c) c->status &= (~FLAG_DECIMAL)
#define setoverflow(c) c->status |= FLAG_OVERFLOW
#define clearoverflow(c) c->status &= (~FLAG_OVERFLOW)
#define setsign(c) c->status |= FLAG_SIGN
#define clearsign(c) c->status &= (~FLAG_SIGN)


//flag calculation macros
#define zerocalc(c, n) {\
    if ((n) & 0x00FF) clearzero(c);\
        else setzero(c);\
}

#define signcalc(c, n) {\
    if ((n) & 0x0080) setsign(c);\
        else clearsign(c);\
}

#define carrycalc(c, n) {\
    if ((n) & 0xFF00) setcarry(c);\
        else clearcarry(c);\
}

#define overflowcalc(c, n, m, o) { /* n = result, m = accumulator, o = memory */ \
    if (((n) ^ (uint16_t)(m)) & ((n) ^ (o)) & 0x0080) setoverflow(c);\
        else clearoverflow(c);\
}

void write6502(Context65 * ctx, uint16_t addr, uint8_t datum);
uint8_t read6502(Context65 * ctx, uint16_t addr);
void step6502(Context65 * ctx);

