# stoc
Stochastic superoptimiser targetting the 6502

So far the "stochastic" bit has not been implemented (early days yet). But we've at least got an exhaustive search and this exercises the emulator, equivalence tester and everything. Some assembly language files in `testing/` contain goofy code sequences that contain obvious inefficiencies. They are just there to demonstrate stoc.

To build the system, type `make`. For each architecture, (currently two or three varieties of 6502) `make` will generate the appropriate source code and compile an executable named `stoc-$arch`.

An example of what you can do:

```
./stoc-6510 --org:0200 examples/test3.asm --hexdump --exh 
; 8 instructions
	lda #$80
	lsr
	lsr
	lsr
	lsr
	lsr
	lsr
	lsr

; 1 instructions
	lda #$01
```

This run loads the program in examples/test3.asm at the address 0x0200, and disassembles it to the terminal. It then searches for another program which has the same result, and prints what it found. 
