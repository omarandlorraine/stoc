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

### Exhaustive search strategy
The exhaustive search strategy (implemented in `exh.c`) tries to find a solution by iterating over all possible instructions for each instruction in the program. First, it tries all programs containing 1 instruction. Then it tries all programs containing two instructions. Then three, and keeps going in this way until it finds a program by calling the equivalence tester, when it stops and prints the program. This means that the program it finds is always the shortest possible program, as measured in lines of assembly.

This strategy is extremely bad at finding any significantly long code sequences because of the explosion of number of programs it tries to explore. On my machine (Intel(R) Core(TM) i5-4310U CPU @ 2.00GHz), it takes around 100ms to find a program that's one instruction long, but 1.3 seconds to find a program of length two. A program of length three takes 4300 seconds, which is more than an hour. I have not tried searching for anything longer. There are ways we could speed this up but this runtime explosion will always be a problem with this search strategy.

I should think that we could narrow the search space down a bit in some ways:
 - a Hidden Markov Chain or some other heuristic method to cull uninteresting programs (more research needed).
 - consider only the commonest immediate values (This is what the GNU Superoptimiser does)
 - Not considering any code sequence which cannot occur in an optimal program (these might do daft things like `ldx #$01, dex` instead of `ldx #$00`. This would cull the search space by effectively being a peephole optimiser. I think this is what Henry Massalin did.

### Equivalence testing
The way I'm currently testing two programs for equivalence is by running them a squillion times, each time with a random input. This means that the registers and memory locations read by the program are set to random values. Then the two programs are run. The output of the program is checked by looking in the live-out registers and live-out memory locations. If for the same input, the programs produce different output, then of course the programs are found to not be equivalent. Otherwise, the equivalence test passes. 

There is a small chance that this test produces a false positive. This would happen if the numbers produced by the random number generator do not exercise all the code paths in the reference program for example. 
