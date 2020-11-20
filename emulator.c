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
 * uint8_t mem_read(uint16_t address)                *
 * void mem_write(uint16_t address, uint8_t value)   *
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
#include "stoc.h"
#include "main.h"

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
#define setcarry(c) c->flags |= FLAG_CARRY
#define clearcarry(c) c->flags &= (~FLAG_CARRY)
#define setzero(c) c->flags |= FLAG_ZERO
#define clearzero(c) c->flags &= (~FLAG_ZERO)
#define setinterrupt(c) c->flags |= FLAG_INTERRUPT
#define clearinterrupt(c) c->flags &= (~FLAG_INTERRUPT)
#define setdecimal(c) c->flags |= FLAG_DECIMAL
#define cleardecimal(c) c->flags &= (~FLAG_DECIMAL)
#define setoverflow(c) c->flags |= FLAG_OVERFLOW
#define clearoverflow(c) c->flags &= (~FLAG_OVERFLOW)
#define setsign(c) c->flags |= FLAG_SIGN
#define clearsign(c) c->flags &= (~FLAG_SIGN)
#define saveaccum(c, n) c->a = (uint8_t)((n) & 0x00FF)

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
void push16(context_t * c, uint16_t pushval) {
    mem_write(c, BASE_STACK + c->s, (pushval >> 8) & 0xFF);
    mem_write(c, BASE_STACK + ((c->s - 1) & 0xFF), pushval & 0xFF);
    c->s -= 2;
}

void push8(context_t * c, uint8_t pushval) {
    mem_write(c, BASE_STACK + c->s--, pushval);
}

uint16_t pull16(context_t * c) {
    uint16_t temp16;
    temp16 = mem_read(c, BASE_STACK + ((c->s + 1) & 0xFF)) | ((uint16_t) mem_read(c, BASE_STACK + ((c->s + 2) & 0xFF)) << 8);
    c->s += 2;
    return(temp16);
}

uint8_t pull8(context_t * c) {
    return (mem_read(c, BASE_STACK + ++c->s));
}

void reset6502(context_t * c) {
    // pc = ((uint16_t)mem_read(0xfffc) | ((uint16_t)mem_read(0xfffd) << 8));
    c->pc = 0xff00;

    c->a = 0;
    c->x = 0;
    c->y = 0;
    c->s = 0xFF;
    c->flags |= FLAG_CONSTANT | FLAG_BREAK;
}

static void (*addrtable[256])();
//addressing mode functions, calculates effective addresses
static void imp(context_t * c) { //implied
}

static void acc(context_t * c) { //accumulator
}

static void imm(context_t * c) { //immediate
    c->ea = c->pc++;
}

static void zp(context_t * c) { //zero-page
    c->ea = (uint16_t)mem_read(c, (uint16_t)c->pc++);
}

static void zpx(context_t * c) { //zero-page,X
    c->ea = ((uint16_t)mem_read(c, (uint16_t)c->pc++) + (uint16_t)c->x) & 0xFF; //zero-page wraparound
}

static void zpy(context_t * c) { //zero-page,Y
    c->ea = ((uint16_t)mem_read(c, (uint16_t)c->pc++) + (uint16_t)c->y) & 0xFF; //zero-page wraparound
}

static void rel(context_t * c) { //relative for branch ops (8-bit immediate value, sign-extended)
    uint16_t rel = (uint16_t)mem_read(c, c->pc++);
    if (rel & 0x80) rel |= 0xFF00;
    c->ea = c->pc + rel;
}

static void abso(context_t * c) { //absolute
    c->ea = (uint16_t)mem_read(c, c->pc) | ((uint16_t)mem_read(c, c->pc+1) << 8);
    c->pc += 2;
}

static void absx(context_t * c) { //absolute,X
    uint16_t startpage;
    c->ea = ((uint16_t)mem_read(c, c->pc) | ((uint16_t)mem_read(c, c->pc+1) << 8));
    startpage = c->ea & 0xFF00;
    c->ea += (uint16_t)c->x;

    c->pc += 2;
}

static void absy(context_t * c) { //absolute,Y
    uint16_t startpage;
    c->ea = ((uint16_t)mem_read(c, c->pc) | ((uint16_t)mem_read(c, c->pc+1) << 8));
    startpage = c->ea & 0xFF00;
    c->ea += (uint16_t)c->y;

    c->pc += 2;
}

static void ind(context_t * c) { //indirect
    uint16_t eahelp, eahelp2;
    eahelp = (uint16_t)mem_read(c, c->pc) | (uint16_t)((uint16_t)mem_read(c, c->pc+1) << 8);
    eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //replicate 6502 page-boundary wraparound bug
    c->ea = (uint16_t)mem_read(c, eahelp) | ((uint16_t)mem_read(c, eahelp2) << 8);
    c->pc += 2;
}

static void indx(context_t * c) { // (indirect,X)
    uint16_t eahelp;
    eahelp = (uint16_t)(((uint16_t)mem_read(c, c->pc++) + (uint16_t)c->x) & 0xFF); //zero-page wraparound for table pointer
    c->ea = (uint16_t)mem_read(c, eahelp & 0x00FF) | ((uint16_t)mem_read(c, (eahelp+1) & 0x00FF) << 8);
}

static void indy(context_t * c) { // (indirect),Y
    uint16_t eahelp, eahelp2, startpage;
    eahelp = (uint16_t)mem_read(c, c->pc++);
    eahelp2 = (eahelp & 0xFF00) | ((eahelp + 1) & 0x00FF); //zero-page wraparound
    c->ea = (uint16_t)mem_read(c, eahelp) | ((uint16_t)mem_read(c, eahelp2) << 8);
    startpage = c->ea & 0xFF00;
    c->ea += (uint16_t)c->y;
}

static uint16_t getvalue(context_t * c) {
    if (addrtable[c->opcode] == acc) return ((uint16_t)c->a);
        else return ((uint16_t)mem_read(c, c->ea));
}

static uint16_t getvalue16(context_t * c) {
    return ((uint16_t)mem_read(c, c->ea) | ((uint16_t)mem_read(c, c->ea+1) << 8));
}

static void putvalue(context_t * c, uint16_t saveval) {
    if (addrtable[c->opcode] == acc) c->a = (uint8_t)(saveval & 0x00FF);
        else mem_write(c, c->ea, (saveval & 0x00FF));
}


//instruction handler functions
void adc(context_t * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->a + value + (uint16_t)(c->flags & FLAG_CARRY);
   
    carrycalc(c, result);
    zerocalc(c, result);
    overflowcalc(c, result, c->a, value);
    signcalc(c, result);
    
    #ifndef NES_CPU
    if (c->flags & FLAG_DECIMAL) {
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

void and(context_t * c) {
    uint8_t m = getvalue(c);
    uint16_t result = (uint16_t)c->a & m;
    signcalc(c, result);
   
    saveaccum(c, result);
}

void asl(context_t * c) {
    uint8_t m = getvalue(c);
    uint16_t result = m << 1;

    carrycalc(c, result);
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}


void bra(context_t * c) {
    uint16_t reladdr = (uint16_t)mem_read(c, c->pc++);
    if (reladdr & 0x80) reladdr |= 0xFF00;
	uint16_t oldpc = c->pc;
	c->pc += reladdr;
	if ((oldpc & 0xFF00) != (c->pc & 0xFF00)) c->clockticks += 2; //check if jump crossed a page boundary
		else c->clockticks++;
}

void bcc(context_t * c) {
    if ((c->flags & FLAG_CARRY) == 0) 
    	bra(c);
}

void bcs(context_t * c) {
    if ((c->flags & FLAG_CARRY) == FLAG_CARRY) 
		bra(c);
}

void beq(context_t * c) {
    if ((c->flags & FLAG_ZERO) == FLAG_ZERO)
		bra(c);
}

void bit(context_t * c) {
    uint8_t value = getvalue(c);
    uint8_t result = (uint16_t)c->a & value;
   
    zerocalc(c, result);
    c->flags = (c->flags & 0x3F) | (uint8_t)(value & 0xC0);
}

void bmi(context_t * c) {
    if ((c->flags & FLAG_SIGN) == FLAG_SIGN)
		bra(c);
}

void bne(context_t * c) {
    if ((c->flags & FLAG_ZERO) == 0)
		bra(c);
}

void bpl(context_t * c) {
    if ((c->flags & FLAG_SIGN) == 0)
		bra(c);
}

void brk(context_t * c) {
    c->pc++;
    push16(c, c->pc); //push next instruction address onto stack
    push8(c, c->flags | FLAG_BREAK); //push CPU flags to stack
    setinterrupt(c); //set interrupt flag
    c->pc = (uint16_t)mem_read(c, 0xFFFE) | ((uint16_t)mem_read(c, 0xFFFF) << 8);
}

void bvc(context_t * c) {
    if ((c->flags & FLAG_OVERFLOW) == 0)
		bra(c);
}

void bvs(context_t * c) {
    if ((c->flags & FLAG_OVERFLOW) == FLAG_OVERFLOW)
		bra(c);
}

void clc(context_t * c) {
    clearcarry(c);
}

void cld(context_t * c) {
    cleardecimal(c);
}

void cli(context_t * c) {
    clearinterrupt(c);
}

void clv(context_t * c) {
    clearoverflow(c);
}

void cmp(context_t * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->a - value;
   
    if (c->a >= (uint8_t)(value & 0x00FF)) setcarry(c);
        else clearcarry(c);
    if (c->a == (uint8_t)(value & 0x00FF)) setzero(c);
        else clearzero(c);
    signcalc(c, result);
}

void cpx(context_t * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->x - value;
   
    if (c->x >= (uint8_t)(value & 0x00FF)) setcarry(c);
        else clearcarry(c);
    if (c->x == (uint8_t)(value & 0x00FF)) setzero(c);
        else clearzero(c);
    signcalc(c, result);
}

void cpy(context_t * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->y - value;
   
    if (c->y >= (uint8_t)(value & 0x00FF)) setcarry(c);
        else clearcarry(c);
    if (c->y == (uint8_t)(value & 0x00FF)) setzero(c);
        else clearzero(c);
    signcalc(c, result);
}

void dec(context_t * c) {
    uint16_t value = getvalue(c);
    uint16_t result = value - 1;
   
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}

void dex(context_t * c) {
    c->x--;
   
    zerocalc(c, c->x);
    signcalc(c, c->x);
}

void dey(context_t * c) {
    c->y--;
   
    zerocalc(c, c->y);
    signcalc(c, c->y);
}

void eor(context_t * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->a ^ value;
   
    zerocalc(c, result);
    signcalc(c, result);
   
    saveaccum(c, result);
}

void inc(context_t * c) {
    uint16_t value = getvalue(c);
    uint16_t result = value + 1;
   
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}

void inx(context_t * c) {
    c->x++;
   
    zerocalc(c, c->x);
    signcalc(c, c->x);
}

void iny(context_t * c) {
    c->y++;
   
    zerocalc(c, c->y);
    signcalc(c, c->y);
}

void jmp(context_t * c) {
    c->pc = c->ea;
}

void jsr(context_t * c) {
    push16(c, c->pc - 1);
    c->pc = c->ea;
}

void lda(context_t * c) {
    uint16_t value = getvalue(c);
    c->a = (uint8_t)(value & 0x00FF);
   
    zerocalc(c, c->a);
    signcalc(c, c->a);
}

void ldx(context_t * c) {
    uint16_t value = getvalue(c);
    c->x = (uint8_t)(value & 0x00FF);
   
    zerocalc(c, c->x);
    signcalc(c, c->x);
}

void ldy(context_t * c) {
    uint16_t value = getvalue(c);
    c->y = (uint8_t)(value & 0x00FF);
   
    zerocalc(c, c->y);
    signcalc(c, c->y);
}

void lsr(context_t * c) {
    uint16_t value = getvalue(c);
    uint16_t result = value >> 1;
   
    if (value & 1) setcarry(c);
        else clearcarry(c);
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}

void nop(context_t * c) {
}

void ora(context_t * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (uint16_t)c->a | value;
   
    zerocalc(c, result);
    signcalc(c, result);
   
    saveaccum(c, result);
}

void pha(context_t * c) {
    push8(c, c->a);
}

void php(context_t * c) {
    push8(c, c->flags | FLAG_BREAK);
}

void pla(context_t * c) {
    c->a = pull8(c);
   
    zerocalc(c, c->a);
    signcalc(c, c->a);
}

void plp(context_t * c) {
    c->flags = pull8(c) | FLAG_CONSTANT;
}

void rol(context_t * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (value << 1) | (c->flags & FLAG_CARRY);
   
    carrycalc(c, result);
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}

void ror(context_t * c) {
    uint16_t value = getvalue(c);
    uint16_t result = (value >> 1) | ((c->flags & FLAG_CARRY) << 7);
   
    if (value & 1) setcarry(c);
        else clearcarry(c);
    zerocalc(c, result);
    signcalc(c, result);
   
    putvalue(c, result);
}

void rti(context_t * c) {
    c->flags = pull8(c);
    c->pc = pull16(c);
}

void rts(context_t * c) {
    c->pc = pull16(c) + 1;
}

void sbc(context_t * c) {
    uint16_t value = getvalue(c) ^ 0x00FF;
    uint16_t result = (uint16_t)c->a + value + (uint16_t)(c->flags & FLAG_CARRY);
   
    carrycalc(c, result);
    zerocalc(c, result);
    overflowcalc(c, result, c->a, value);
    signcalc(c, result);

    #ifndef NES_CPU
    if (c->flags & FLAG_DECIMAL) {
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

void sec(context_t * c) {
    setcarry(c);
}

void sed(context_t * c) {
    setdecimal(c);
}

void sei(context_t * c) {
    setinterrupt(c);
}

void sta(context_t * c) {
    putvalue(c, c->a);
}

void stx(context_t * c) {
    putvalue(c, c->x);
}

void sty(context_t * c) {
    putvalue(c, c->y);
}

void tax(context_t * c) {
    c->x = c->a;
   
    zerocalc(c, c->x);
    signcalc(c, c->x);
}

void tay(context_t * c) {
    c->y = c->a;
   
    zerocalc(c, c->y);
    signcalc(c, c->y);
}

void tsx(context_t * c) {
    c->x = c->s;
   
    zerocalc(c, c->x);
    signcalc(c, c->x);
}

void txa(context_t * c) {
    c->a = c->x;
   
    zerocalc(c, c->a);
    signcalc(c, c->a);
}

void txs(context_t * c) {
    c->s = c->x;
}

void tya(context_t * c) {
    c->a = c->y;
   
    zerocalc(c, c->a);
    signcalc(c, c->a);
}

void lax(context_t * c) {
	lda(c);
	ldx(c);
}

void sax(context_t * c) {
	sta(c);
	stx(c);
	putvalue(c, c->a & c->x);
}

void dcp(context_t * c) {
	dec(c);
	cmp(c);
}

void isb(context_t * c) {
	inc(c);
	sbc(c);
}

void slo(context_t * c) {
	asl(c);
	ora(c);
}

void rla(context_t * c) {
	rol(c);
	and(c);
}

void sre(context_t * c) {
	lsr(c);
	eor(c);
}

void rra(context_t * c) {
	ror(c);
	adc(c);
}

static void (*addrtable[256])(context_t * c) = {
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


void nmi6502(context_t * c) {
    push16(c, c->pc);
    push8(c, c->flags);
    c->flags |= FLAG_INTERRUPT;
    c->pc = (uint16_t)mem_read(c, 0xFFFA) | ((uint16_t)mem_read(c, 0xFFFB) << 8);
}

uint16_t getPC(context_t * c) {
    return c->pc;
}

void irq6502(context_t * c) {
    push16(c, c->pc);
    push8(c, c->flags);
    c->flags |= FLAG_INTERRUPT;
    c->pc = (uint16_t)mem_read(c, 0xFFFE) | ((uint16_t)mem_read(c, 0xFFFF) << 8);
}

uint8_t callexternal = 0;
void (*loopexternal)();

void step(context_t * c) {
    uint8_t opcode = mem_read(c, c->pc++);
    c->opcode = opcode;
    c->flags |= FLAG_CONSTANT;

    (*addrtable[opcode])(c);
    (*optable[opcode])(c);
    c->clockticks+= ticktable[opcode];
}


