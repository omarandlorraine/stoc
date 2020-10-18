// Call this once before you begin execution.
void reset6502();
 
// Execute 6502 code up to the next specified count of clock ticks.
void exec6502(uint32_t tickcount);

// Execute a single instrution.
void step6502();

// Trigger a hardware IRQ in the 6502 core.
void irq6502();

// Trigger an NMI in the 6502 core.
void nmi6502();

// Pass a pointer to a void function taking no parameters. This will cause
// Fake6502 to call that function once after each emulated instruction.
void hookexternal(void *funcptr);

#define CARRYFLAG     0x01
#define ZEROFLAG      0x02
#define INTERRUPTFLAG 0x04
#define DECIMALFLAG   0x08
#define BREAKFLAG     0x10
#define CONSTANTFLAG  0x20
#define OVERFLOWFLAG  0x40
#define SIGNFLAG      0x80

