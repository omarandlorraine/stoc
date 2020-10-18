/* Fake6502 CPU emulator core v1.1 *******************
 * (c)2011 Mike Chambers (miker00lz@gmail.com)       *
 *****************************************************
 * v1.1 - Small bugfix in BIT opcode, but it was the *
 *        difference between a few games in my NES   *
 *        emulator working and being broken!         *
 *        I went through the rest carefully again    *
 *        after fixing it just to make sure I didn't *
 *        have any other typos! (Dec. 17, 2011)      *
 *                                                   *
 * v1.0 - First release (Nov. 24, 2011)              *
 *****************************************************
 * LICENSE: This source code is released into the    *
 * public domain, but if you use it please do give   *
 * credit. I put a lot of effort into writing this!  *
 *                                                   *
 *****************************************************
 * Fake6502 is a MOS Technology 6502 CPU emulation   *
 * engine in C. It was written as part of a Nintendo *
 * Entertainment System emulator I've been writing.  *
 *                                                   *
 * A couple important things to know about are two   *
 * defines in the code. One is "UNDOCUMENTED" which, *
 * when defined, allows Fake6502 to compile with     *
 * full support for the more predictable             *
 * undocumented instructions of the 6502. If it is   *
 * undefined, undocumented opcodes just act as NOPs. *
 *                                                   *
 * The other define is "NES_CPU", which causes the   *
 * code to compile without support for binary-coded  *
 * decimal (BCD) support for the ADC and SBC         *
 * opcodes. The Ricoh 2A03 CPU in the NES does not   *
 * support BCD, but is otherwise identical to the    *
 * standard MOS 6502. (Note that this define is      *
 * enabled in this file if you haven't changed it    *
 * yourself. If you're not emulating a NES, you      *
 * should comment it out.)                           *
 *                                                   *
 * If you do discover an error in timing accuracy,   *
 * or operation in general please e-mail me at the   *
 * address above so that I can fix it. Thank you!    *
 *                                                   *
 *****************************************************
 * Usage:                                            *
 *                                                   *
 * Fake6502 requires you to provide two external     *
 * functions:                                        *
 *                                                   *
 * uint8_t read6502(uint16_t address)                *
 * void write6502(uint16_t address, uint8_t value)   *
 *                                                   *
 * You may optionally pass Fake6502 the pointer to a *
 * function which you want to be called after every  *
 * emulated instruction. This function should be a   *
 * void with no parameters expected to be passed to  *
 * it.                                               *
 *                                                   *
 * This can be very useful. For example, in a NES    *
 * emulator, you check the number of clock ticks     *
 * that have passed so you can know when to handle   *
 * APU events.                                       *
 *                                                   *
 * To pass Fake6502 this pointer, use the            *
 * hookexternal(void *funcptr) function provided.    *
 *                                                   *
 * To disable the hook later, pass NULL to it.       *
 *****************************************************
 * Useful functions in this emulator:                *
 *                                                   *
 * void reset6502()                                  *
 *   - Call this once before you begin execution.    *
 *                                                   *
 * void exec6502(uint32_t tickcount)                 *
 *   - Execute 6502 code up to the next specified    *
 *     count of clock ticks.                         *
 *                                                   *
 * void step6502()                                   *
 *   - Execute a single instrution.                  *
 *                                                   *
 * void irq6502()                                    *
 *   - Trigger a hardware IRQ in the 6502 core.      *
 *                                                   *
 * void nmi6502()                                    *
 *   - Trigger an NMI in the 6502 core.              *
 *                                                   *
 * void hookexternal(void *funcptr)                  *
 *   - Pass a pointer to a void function taking no   *
 *     parameters. This will cause Fake6502 to call  *
 *     that function once after each emulated        *
 *     instruction.                                  *
 *                                                   *
 *****************************************************
 * Useful variables in this emulator:                *
 *                                                   *
 * uint32_t clockticks6502                           *
 *   - A running total of the emulated cycle count.  *
 *                                                   *
 *                                                   *
 *****************************************************/

#include <stdio.h>
#include <stdint.h>
#include "../stoc.h"
#include "fake6502.h"
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

//flag modifier macros

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


//a few general functions used by various other functions
void push16(Context65 * c, uint16_t pushval) {
    write6502(c, BASE_STACK + c->sp, (pushval >> 8) & 0xFF);
    write6502(c, BASE_STACK + ((c->sp - 1) & 0xFF), pushval & 0xFF);
    c->sp -= 2;
}

void push8(Context65 * c, uint8_t pushval) {
    write6502(c, BASE_STACK + c->sp--, pushval);
}

uint16_t pull16(Context65 * c) {
    uint16_t temp16;
    temp16 = read6502(c, BASE_STACK + ((c->sp + 1) & 0xFF)) | ((uint16_t) read6502(c, BASE_STACK + ((c->sp + 2) & 0xFF)) << 8);
    c->sp += 2;
    return(temp16);
}

uint8_t pull8(Context65 * c) {
    return (read6502(c, BASE_STACK + ++c->sp));
}

void reset6502(Context65 * c) {
    // pc = ((uint16_t)read6502(0xfffc) | ((uint16_t)read6502(0xfffd) << 8));
    c->pc = 0xff00;

    c->a = 0;
    c->x = 0;
    c->y = 0;
    c->sp = 0xFF;
    c->status |= FLAG_CONSTANT | FLAG_BREAK;
}

static void (*addrtable[256])();
//addressing mode functions, calculates effective addresses
static void imp(Context65 * c) { //implied
}

static void acc(Context65 * c) { //accumulator
}

static void imm(Context65 * c) { //immediate
    c->ea = c->pc++;
}

static void zp(Context65 * c) { //zero-page
    c->ea = (uint16_t)read6502(c, (uint16_t)c->pc++);
}

static void zpx(Context65 * c) { //zero-page,X
    c->ea = ((uint16_t)read6502(c, (uint16_t)c->pc++) + (uint16_t)c->x) & 0xFF; //zero-page wraparound
}

static void zpy(Context65 * c) { //zero-page,Y
    c->ea = ((uint16_t)read6502(c, (uint16_t)c->pc++) + (uint16_t)c->y) & 0xFF; //zero-page wraparound
}

static void rel(Context65 * c) { //relative for branch ops (8-bit immediate value, sign-extended)
    uint16_t rel = (uint16_t)read6502(c, c->pc++);
    if (rel & 0x80) rel |= 0xFF00;
    c->ea = c->pc + rel;
}

static void abso(Context65 * c) { //absolute
    c->ea = (uint16_t)read6502(c, c->pc) | ((uint16_t)read6502(c, c->pc+1) << 8);
    c->pc += 2;
}

static void absx(Context65 * c) { //absolute,X
    uint16_t startpage;
    c->ea = ((uint16_t)read6502(c, c->pc) | ((uint16_t)read6502(c, c->pc+1) << 8));
    startpage = c->ea & 0xFF00;
    c->ea += (uint16_t)c->x;

    if (startpage != (c->ea & 0xFF00)) { //one cycle penlty for page-crossing on some opcodes
        c->penaltyaddr = 1;
    }

    c->pc += 2;
}

static void absy(Context65 * c) { //absolute,Y
    uint16_t startpage;
    c->ea = ((uint16_t)read6502(c, c->pc) | ((uint16_t)read6502(c, c->pc+1) << 8));
    startpage = c->ea & 0xFF00;
    c->ea += (uint16_t)c->y;

    if (startpage != (c->ea & 0xFF00)) { //one cycle penlty for page-crossing on some opcodes
        c->penaltyaddr = 1;
    }

    c->pc += 2;
}

static void ind(Context65 * c) { //indirect
    uint16_t eahelp, eahelp2;
    eahelp = (uint16_t)read6502(c, c->pc) | (uint16_t)((uint16_t)read6502(c, c->pc+1) << 8);
    eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //replicate 6502 page-boundary wraparound bug
    c->ea = (uint16_t)read6502(c, eahelp) | ((uint16_t)read6502(c, eahelp2) << 8);
    c->pc += 2;
}

static void indx(Context65 * c) { // (indirect,X)
    uint16_t eahelp;
    eahelp = (uint16_t)(((uint16_t)read6502(c, c->pc++) + (uint16_t)c->x) & 0xFF); //zero-page wraparound for table pointer
    c->ea = (uint16_t)read6502(c, eahelp & 0x00FF) | ((uint16_t)read6502(c, (eahelp+1) & 0x00FF) << 8);
}

static void indy(Context65 * c) { // (indirect),Y
    uint16_t eahelp, eahelp2, startpage;
    eahelp = (uint16_t)read6502(c, c->pc++);
    eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //zero-page wraparound
    c->ea = (uint16_t)read6502(c, eahelp) | ((uint16_t)read6502(c, eahelp2) << 8);
    startpage = c->ea & 0xFF00;
    c->ea += (uint16_t)c->y;

    if (startpage != (c->ea & 0xFF00)) { //one cycle penlty for page-crossing on some opcodes
        c->penaltyaddr = 1;
    }
}

static uint16_t getvalue(Context65 * c) {
    if (addrtable[c->opcode] == acc) return ((uint16_t)c->a);
        else return ((uint16_t)read6502(c, c->ea));
}

static uint16_t getvalue16(Context65 * c) {
    return ((uint16_t)read6502(c, c->ea) | ((uint16_t)read6502(c, c->ea+1) << 8));
}

static void putvalue(Context65 * c, uint16_t saveval) {
    if (addrtable[c->opcode] == acc) c->a = (uint8_t)(saveval & 0x00FF);
        else write6502(c, c->ea, (saveval & 0x00FF));
}


//instruction handler functions
void adc(Context65 * c) {
    c->penaltyop = 1;
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->a + value + (uint16_t)(c->status & FLAG_CARRY);
   
    carrycalc(c, result);
    zerocalc(c, result);
    overflowcalc(c, result, c->a, value);
    signcalc(c, result);
    
    #ifndef NES_CPU
    if (c->status & FLAG_DECIMAL) {
        clearcarry(c);
        
        if ((c->a & 0x0F) > 0x09) {
            c->a += 0x06;
        }
        if ((c->a & 0xF0) > 0x90) {
            c->a += 0x60;
            setcarry(c);
        }
        
        c->clockticks++;
    }
    #endif
   
    saveaccum(c, result);
}

void and(Context65 * c) {
    c->penaltyop = 1;
    uint8_t m = getvalue(c);
    uint16_t result = (uint16_t)c->a & m;
    signcalc(c, result);
   
    saveaccum(c, result);
}

void asl(Context65 * c) {
    uint8_t m = getvalue(c);
    uint16_t result = m << 1;

    carrycalc(c, result);
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}

void bcc(Context65 * c) {
    uint16_t reladdr = (uint16_t)read6502(c, c->pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
    if ((c->status & FLAG_CARRY) == 0) {
        uint16_t oldpc = c->pc;
        c->pc += reladdr;
        if ((oldpc & 0xFF00) != (c->pc & 0xFF00)) c->clockticks += 2; //check if jump crossed a page boundary
            else c->clockticks++;
    }
}

void bcs(Context65 * c) {
    uint16_t reladdr = (uint16_t)read6502(c, c->pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
    if ((c->status & FLAG_CARRY) == FLAG_CARRY) {
        uint16_t oldpc = c->pc;
        c->pc += reladdr;
        if ((oldpc & 0xFF00) != (c->pc & 0xFF00)) c->clockticks += 2; //check if jump crossed a page boundary
            else c->clockticks++;
    }
}

void beq(Context65 * c) {
    uint16_t reladdr = (uint16_t)read6502(c, c->pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
    if ((c->status & FLAG_ZERO) == FLAG_ZERO) {
        uint16_t oldpc = c->pc;
        c->pc += reladdr;
        if ((oldpc & 0xFF00) != (c->pc & 0xFF00)) c->clockticks += 2; //check if jump crossed a page boundary
            else c->clockticks++;
    }
}

void bit(Context65 * c) {
    uint8_t value = getvalue(c);
    uint8_t result = (uint16_t)c->a & value;
   
    zerocalc(c, result);
    c->status = (c->status & 0x3F) | (uint8_t)(value & 0xC0);
}

void bmi(Context65 * c) {
    uint16_t reladdr = (uint16_t)read6502(c, c->pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
    if ((c->status & FLAG_SIGN) == FLAG_SIGN) {
        uint16_t oldpc = c->pc;
        c->pc += reladdr;
        if ((oldpc & 0xFF00) != (c->pc & 0xFF00)) c->clockticks += 2; //check if jump crossed a page boundary
            else c->clockticks++;
    }
}

void bne(Context65 * c) {
    uint16_t reladdr = (uint16_t)read6502(c, c->pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
    if ((c->status & FLAG_ZERO) == 0) {
        uint8_t oldpc = c->pc;
        c->pc += reladdr;
        if ((oldpc & 0xFF00) != (c->pc & 0xFF00)) c->clockticks += 2; //check if jump crossed a page boundary
            else c->clockticks++;
    }
}

void bpl(Context65 * c) {
    uint16_t reladdr = (uint16_t)read6502(c, c->pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
    if ((c->status & FLAG_SIGN) == 0) {
        uint16_t oldpc = c->pc;
        c->pc += reladdr;
        if ((oldpc & 0xFF00) != (c->pc & 0xFF00)) c->clockticks += 2; //check if jump crossed a page boundary
            else c->clockticks++;
    }
}

void brk(Context65 * c) {
    c->pc++;
    push16(c, c->pc); //push next instruction address onto stack
    push8(c, c->status | FLAG_BREAK); //push CPU status to stack
    setinterrupt(c); //set interrupt flag
    c->pc = (uint16_t)read6502(c, 0xFFFE) | ((uint16_t)read6502(c, 0xFFFF) << 8);
}

void bvc(Context65 * c) {
    uint16_t reladdr = (uint16_t)read6502(c, c->pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
    if ((c->status & FLAG_OVERFLOW) == 0) {
        uint16_t oldpc = c->pc;
        c->pc += reladdr;
        if ((oldpc & 0xFF00) != (c->pc & 0xFF00)) c->clockticks += 2; //check if jump crossed a page boundary
            else c->clockticks++;
    }
}

void bvs(Context65 * c) {
    uint16_t reladdr = (uint16_t)read6502(c, c->pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
    if ((c->status & FLAG_OVERFLOW) == FLAG_OVERFLOW) {
        uint16_t oldpc = c->pc;
        c->pc += reladdr;
        if ((oldpc & 0xFF00) != (c->pc & 0xFF00)) c->clockticks += 2; //check if jump crossed a page boundary
            else c->clockticks++;
    }
}

void clc(Context65 * c) {
    clearcarry(c);
}

void cld(Context65 * c) {
    cleardecimal(c);
}

void cli(Context65 * c) {
    clearinterrupt(c);
}

void clv(Context65 * c) {
    clearoverflow(c);
}

void cmp(Context65 * c) {
    c->penaltyop = 1;
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->a - value;
   
    if (c->a >= (uint8_t)(value & 0x00FF)) setcarry(c);
        else clearcarry(c);
    if (c->a == (uint8_t)(value & 0x00FF)) setzero(c);
        else clearzero(c);
    signcalc(c, result);
}

void cpx(Context65 * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->x - value;
   
    if (c->x >= (uint8_t)(value & 0x00FF)) setcarry(c);
        else clearcarry(c);
    if (c->x == (uint8_t)(value & 0x00FF)) setzero(c);
        else clearzero(c);
    signcalc(c, result);
}

void cpy(Context65 * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->y - value;
   
    if (c->y >= (uint8_t)(value & 0x00FF)) setcarry(c);
        else clearcarry(c);
    if (c->y == (uint8_t)(value & 0x00FF)) setzero(c);
        else clearzero(c);
    signcalc(c, result);
}

void dec(Context65 * c) {
    uint16_t value = getvalue(c);
    uint16_t result = value - 1;
   
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}

void dex(Context65 * c) {
    c->x--;
   
    zerocalc(c, c->x);
    signcalc(c, c->x);
}

void dey(Context65 * c) {
    c->y--;
   
    zerocalc(c, c->y);
    signcalc(c, c->y);
}

void eor(Context65 * c) {
    c->penaltyop = 1;
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->a ^ value;
   
    zerocalc(c, result);
    signcalc(c, result);
   
    saveaccum(c, result);
}

void inc(Context65 * c) {
    uint16_t value = getvalue(c);
    uint16_t result = value + 1;
   
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}

void inx(Context65 * c) {
    c->x++;
   
    zerocalc(c, c->x);
    signcalc(c, c->x);
}

void iny(Context65 * c) {
    c->y++;
   
    zerocalc(c, c->y);
    signcalc(c, c->y);
}

void jmp(Context65 * c) {
    c->pc = c->ea;
}

void jsr(Context65 * c) {
    push16(c, c->pc - 1);
    c->pc = c->ea;
}

void lda(Context65 * c) {
    c->penaltyop = 1;
    uint16_t value = getvalue(c);
    c->a = (uint8_t)(value & 0x00FF);
   
    zerocalc(c, c->a);
    signcalc(c, c->a);
}

void ldx(Context65 * c) {
    c->penaltyop = 1;
    uint16_t value = getvalue(c);
    c->x = (uint8_t)(value & 0x00FF);
   
    zerocalc(c, c->x);
    signcalc(c, c->x);
}

void ldy(Context65 * c) {
    c->penaltyop = 1;
    uint16_t value = getvalue(c);
    c->y = (uint8_t)(value & 0x00FF);
   
    zerocalc(c, c->y);
    signcalc(c, c->y);
}

void lsr(Context65 * c) {
    uint16_t value = getvalue(c);
    uint16_t result = value >> 1;
   
    if (value & 1) setcarry(c);
        else clearcarry(c);
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}

void nop(Context65 * c) {
    switch (c->opcode) {
        case 0x1C:
        case 0x3C:
        case 0x5C:
        case 0x7C:
        case 0xDC:
        case 0xFC:
            c->penaltyop = 1;
            break;
    }
}

void ora(Context65 * c) {
    c->penaltyop = 1;
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->a | value;
   
    zerocalc(c, result);
    signcalc(c, result);
   
    saveaccum(c, result);
}

void pha(Context65 * c) {
    push8(c, c->a);
}

void php(Context65 * c) {
    push8(c, c->status | FLAG_BREAK);
}

void pla(Context65 * c) {
    c->a = pull8(c);
   
    zerocalc(c, c->a);
    signcalc(c, c->a);
}

void plp(Context65 * c) {
    c->status = pull8(c) | FLAG_CONSTANT;
}

void rol(Context65 * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (value << 1) | (c->status & FLAG_CARRY);
   
    carrycalc(c, result);
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}

void ror(Context65 * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (value >> 1) | ((c->status & FLAG_CARRY) << 7);
   
    if (value & 1) setcarry(c);
        else clearcarry(c);
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}

void rti(Context65 * c) {
    c->status = pull8(c);
    c->pc = pull16(c);
}

void rts(Context65 * c) {
    c->pc = pull16(c) + 1;
}

void sbc(Context65 * c) {
    c->penaltyop = 1;
    uint16_t value = getvalue(c) ^ 0x00FF;
    uint16_t result = (uint16_t)c->a + value + (uint16_t)(c->status & FLAG_CARRY);
   
    carrycalc(c, result);
    zerocalc(c, result);
    overflowcalc(c, result, c->a, value);
    signcalc(c, result);

    #ifndef NES_CPU
    if (c->status & FLAG_DECIMAL) {
        clearcarry(c);
        
        c->a -= 0x66;
        if ((c->a & 0x0F) > 0x09) {
            c->a += 0x06;
        }
        if ((c->a & 0xF0) > 0x90) {
            c->a += 0x60;
            setcarry(c);
        }
        
        c->clockticks++;
    }
    #endif
   
    saveaccum(c, result);
}

void sec(Context65 * c) {
    setcarry(c);
}

void sed(Context65 * c) {
    setdecimal(c);
}

void sei(Context65 * c) {
    setinterrupt(c);
}

void sta(Context65 * c) {
    putvalue(c, c->a);
}

void stx(Context65 * c) {
    putvalue(c, c->x);
}

void sty(Context65 * c) {
    putvalue(c, c->y);
}

void tax(Context65 * c) {
    c->x = c->a;
   
    zerocalc(c, c->x);
    signcalc(c, c->x);
}

void tay(Context65 * c) {
    c->y = c->a;
   
    zerocalc(c, c->y);
    signcalc(c, c->y);
}

void tsx(Context65 * c) {
    c->x = c->sp;
   
    zerocalc(c, c->x);
    signcalc(c, c->x);
}

void txa(Context65 * c) {
    c->a = c->x;
   
    zerocalc(c, c->a);
    signcalc(c, c->a);
}

void txs(Context65 * c) {
    c->sp = c->x;
}

void tya(Context65 * c) {
    c->a = c->y;
   
    zerocalc(c, c->a);
    signcalc(c, c->a);
}

void lax(Context65 * c) {
	lda(c);
	ldx(c);
}

void sax(Context65 * c) {
	sta(c);
	stx(c);
	putvalue(c, c->a & c->x);
	if (c->penaltyop && c->penaltyaddr) c->clockticks--;
}

void dcp(Context65 * c) {
	dec(c);
	cmp(c);
	if (c->penaltyop && c->penaltyaddr) c->clockticks--;
}

void isb(Context65 * c) {
	inc(c);
	sbc(c);
	if (c->penaltyop && c->penaltyaddr) c->clockticks--;
}

void slo(Context65 * c) {
	asl(c);
	ora(c);
	if (c->penaltyop && c->penaltyaddr) c->clockticks--;
}

void rla(Context65 * c) {
	rol(c);
	and(c);
	if (c->penaltyop && c->penaltyaddr) c->clockticks--;
}

void sre(Context65 * c) {
	lsr(c);
	eor(c);
	if (c->penaltyop && c->penaltyaddr) c->clockticks--;
}

void rra(Context65 * c) {
	ror(c);
	adc(c);
	if (c->penaltyop && c->penaltyaddr) c->clockticks--;
}

static void (*addrtable[256])(Context65 * c) = {
/*        |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  A  |  B  |  C  |  D  |  E  |  F  |     */
/* 0 */     imp, indx,  imp, indx,   zp,   zp,   zp,   zp,  imp,  imm,  acc,  imm, abso, abso, abso, abso, /* 0 */
/* 1 */     rel, indy,  imp, indy,  zpx,  zpx,  zpx,  zpx,  imp, absy,  imp, absy, absx, absx, absx, absx, /* 1 */
/* 2 */    abso, indx,  imp, indx,   zp,   zp,   zp,   zp,  imp,  imm,  acc,  imm, abso, abso, abso, abso, /* 2 */
/* 3 */     rel, indy,  imp, indy,  zpx,  zpx,  zpx,  zpx,  imp, absy,  imp, absy, absx, absx, absx, absx, /* 3 */
/* 4 */     imp, indx,  imp, indx,   zp,   zp,   zp,   zp,  imp,  imm,  acc,  imm, abso, abso, abso, abso, /* 4 */
/* 5 */     rel, indy,  imp, indy,  zpx,  zpx,  zpx,  zpx,  imp, absy,  imp, absy, absx, absx, absx, absx, /* 5 */
/* 6 */     imp, indx,  imp, indx,   zp,   zp,   zp,   zp,  imp,  imm,  acc,  imm,  ind, abso, abso, abso, /* 6 */
/* 7 */     rel, indy,  imp, indy,  zpx,  zpx,  zpx,  zpx,  imp, absy,  imp, absy, absx, absx, absx, absx, /* 7 */
/* 8 */     imm, indx,  imm, indx,   zp,   zp,   zp,   zp,  imp,  imm,  imp,  imm, abso, abso, abso, abso, /* 8 */
/* 9 */     rel, indy,  imp, indy,  zpx,  zpx,  zpy,  zpy,  imp, absy,  imp, absy, absx, absx, absy, absy, /* 9 */
/* A */     imm, indx,  imm, indx,   zp,   zp,   zp,   zp,  imp,  imm,  imp,  imm, abso, abso, abso, abso, /* A */
/* B */     rel, indy,  imp, indy,  zpx,  zpx,  zpy,  zpy,  imp, absy,  imp, absy, absx, absx, absy, absy, /* B */
/* C */     imm, indx,  imm, indx,   zp,   zp,   zp,   zp,  imp,  imm,  imp,  imm, abso, abso, abso, abso, /* C */
/* D */     rel, indy,  imp, indy,  zpx,  zpx,  zpx,  zpx,  imp, absy,  imp, absy, absx, absx, absx, absx, /* D */
/* E */     imm, indx,  imm, indx,   zp,   zp,   zp,   zp,  imp,  imm,  imp,  imm, abso, abso, abso, abso, /* E */
/* F */     rel, indy,  imp, indy,  zpx,  zpx,  zpx,  zpx,  imp, absy,  imp, absy, absx, absx, absx, absx  /* F */
};

static const uint8_t ticktable[256] = {
/*        |  0  |  1  |  2  |  3  |  4  |  5  |  6  |  7  |  8  |  9  |  A  |  B  |  C  |  D  |  E  |  F  |     */
/* 0 */      7,    6,    2,    8,    3,    3,    5,    5,    3,    2,    2,    2,    4,    4,    6,    6,  /* 0 */
/* 1 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 1 */
/* 2 */      6,    6,    2,    8,    3,    3,    5,    5,    4,    2,    2,    2,    4,    4,    6,    6,  /* 2 */
/* 3 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 3 */
/* 4 */      6,    6,    2,    8,    3,    3,    5,    5,    3,    2,    2,    2,    3,    4,    6,    6,  /* 4 */
/* 5 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 5 */
/* 6 */      6,    6,    2,    8,    3,    3,    5,    5,    4,    2,    2,    2,    5,    4,    6,    6,  /* 6 */
/* 7 */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* 7 */
/* 8 */      2,    6,    2,    6,    3,    3,    3,    3,    2,    2,    2,    2,    4,    4,    4,    4,  /* 8 */
/* 9 */      2,    6,    2,    6,    4,    4,    4,    4,    2,    5,    2,    5,    5,    5,    5,    5,  /* 9 */
/* A */      2,    6,    2,    6,    3,    3,    3,    3,    2,    2,    2,    2,    4,    4,    4,    4,  /* A */
/* B */      2,    5,    2,    5,    4,    4,    4,    4,    2,    4,    2,    4,    4,    4,    4,    4,  /* B */
/* C */      2,    6,    2,    8,    3,    3,    5,    5,    2,    2,    2,    2,    4,    4,    6,    6,  /* C */
/* D */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7,  /* D */
/* E */      2,    6,    2,    8,    3,    3,    5,    5,    2,    2,    2,    2,    4,    4,    6,    6,  /* E */
/* F */      2,    5,    2,    8,    4,    4,    6,    6,    2,    4,    2,    7,    4,    4,    7,    7   /* F */
};


void nmi6502(Context65 * c) {
    push16(c, c->pc);
    push8(c, c->status);
    c->status |= FLAG_INTERRUPT;
    c->pc = (uint16_t)read6502(c, 0xFFFA) | ((uint16_t)read6502(c, 0xFFFB) << 8);
}

uint16_t getPC(Context65 * c) {
    return c->pc;
}

void irq6502(Context65 * c) {
    push16(c, c->pc);
    push8(c, c->status);
    c->status |= FLAG_INTERRUPT;
    c->pc = (uint16_t)read6502(c, 0xFFFE) | ((uint16_t)read6502(c, 0xFFFF) << 8);
}

uint8_t callexternal = 0;
void (*loopexternal)();

void step6502(Context65 * c) {
    uint8_t opcode = read6502(c, c->pc++);
    c->opcode = opcode;
    c->status |= FLAG_CONSTANT;

    c->penaltyop = 0;
    c->penaltyaddr = 0;

//	printf("pc = %x\topcode %x\n", c->pc - 1, opcode);
    (*addrtable[opcode])(c);
    (*optable[opcode])(c);
    c->clockticks+= ticktable[opcode];
    if (c->penaltyop && c->penaltyaddr) c->clockticks++;
}


