#define PREFIX_NONE 0
#define PREFIX_ED 0x0100
#define PREFIX_CB 0x0200
#define PREFIX_DD 0x1000
#define PREFIX_FD 0x2000
#define PREFIX_DDCB (PREFIX_DD | PREFIX_CB)
#define PREFIX_FDCB (PREFIX_FD | PREFIX_CB)
#define PREFIX_MASK 0xff00
#define OPCODE_MASK 0x00ff
#include "arch.h"

void disasm_implied(instruction_t *i);
void disasm_rpimm16(instruction_t *i);
void altop_implied(instruction_t *i);
void altop_rpimm16(instruction_t *i);
void rndoper_implied(rewrite_t *p, instruction_t *i);
void rndoper_rpimm16(rewrite_t *p, instruction_t *i);

typedef struct {
    void (*disasm)(instruction_t *i);
    void (*altop)(instruction_t *i);
    void (*rndoperand)(rewrite_t *p, instruction_t *i);
	pick_t * instrgroup;
    char *dis1;
    char *dis2;
    int operandlength;
} instrdata_t;

extern instrdata_t instrdata_none[256];
extern instrdata_t instrdata_ed[256];
extern instrdata_t instrdata_cb[256];
extern instrdata_t instrdata_dd[256];
extern instrdata_t instrdata_fd[256];
extern instrdata_t instrdata_ddcb[256];
extern instrdata_t instrdata_fdcb[256];

extern pick_t all_instructions;
extern pick_t instructions_implied;
extern pick_t instructions_rpimm16;
