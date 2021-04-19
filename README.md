# stoc
Stochastic superoptimiser targetting the 6502

We've got a few different search strategies implemented actually, and these exercise the emulator, equivalence tester and everything. Some assembly language files in `examples/` contain goofy code sequences that contain obvious inefficiencies. They are just there to demonstrate stoc.

To build the system, type `make`. For each architecture, (currently a few varieties of 6502) `make` will generate the appropriate source code and compile an executable named `stoc-$arch`.

### Supported architectures
So far, we've got a few varieties of 6502. These are:

- *stoc-6502* which is a generic NMOS 6502, including the jmp indirect bug, but does not use any of the illegal opcodes
- *stoc-6510*, another NMOS 6502, and has some of the same illegal opcodes that the Commodore 64 guys use
- *stoc-65c02*, targets the later CMOS chips with extra opcodes like `phx` and so on
- *stoc-2a03*, basically the same as stoc-6502 but has no decimal mode. Dead-code elimination here will remove instructions `sed` and `cld`.

The above list is essentially what's provided by the *fake6502* submodule. If you are interested in adding other architectures, I would suggest that the easiest way would be to graft in another emulator. At build-time, a particular emulator is linked in, and this is what determines which architecture the binary supports.

### Exhaustive search strategy
The exhaustive search strategy (implemented in `exh.c`) tries to find a solution by iterating over all possible instructions for each instruction in the program. First, it tries all programs containing 1 instruction. Then it tries all programs containing two instructions. Then three, and keeps going in this way until it finds a program by calling the equivalence tester, when it stops and prints the program. This means that the program it finds is always the shortest possible program, as measured in lines of assembly.

This strategy is extremely bad at finding any significantly long code sequences because of the explosion of number of programs it tries to explore. On my machine (Intel(R) Core(TM) i5-4310U CPU @ 2.00GHz), it takes around 100ms to find a program that's one instruction long, but 1.3 seconds to find a program of length two. A program of length three takes 4300 seconds, which is more than an hour. I have not tried searching for anything longer. There are ways we could speed this up but this runtime explosion will always be a problem with this search strategy.

I should think that we could narrow the search space down a bit in some ways:
 - a Hidden Markov Chain or some other heuristic method to cull uninteresting programs (more research needed).
 - consider only the commonest immediate values (This is what the GNU Superoptimiser does)
 - Not considering any code sequence which cannot occur in an optimal program (these might do daft things like `ldx #$01, dex` instead of `ldx #$00`. This would cull the search space by effectively being a peephole optimiser. I think this is what Henry Massalin did.

Below is an example run. `examples/exhaustive_search_test.asm` is a program which does a load of computation and then overwrites whatever it does with a single `lda #$12` instruction at the end, so that all the preceding work is effectively discarded.
```
$ ./stoc-6510 examples/exhaustive_search_test.asm
; 1 instructions
; 2 bytes
; 2 clockticks
; hamming distance 0
	lda #$12
```

### Dead Code Elimination
This search strategy looks for a more optimal rewrite by selecting random instructions for deletion; up to five at a time. If the program proves to be equivalent without the selected instructions, then the instructions are deleted and the same procedure is done again. Use this procedure by first loading a program and then using the `--dce` command line argument.

```
$ ./stoc-2a03 examples/dead_code_elimination.asm 
; 3 instructions
; 5 bytes
; 11922 clockticks
; hamming distance 0
	clc
	lda #$07
	adc #$05
```

It might be worth noting that the input procedure above contains two instances of the `clc` instructions, and only one is needed. Either one may be deleted, and it is picked at random. Running the same program again might have yielded the instructions `clc` and `lda #$02` in a different order. The `sed` instruction is not needed at all on the 2A03 because this is a chip variant which lacks the decimal mode. On other varieties of the 6502, as emulated by stoc-6510 for example, the `sed` instruction will be deemed necessary by the equivalence tester.

### Stochastic optimisation
This search strategy walks around the search space by trying a number of mutations at a time, at sees if these mutations together either lower the cost or increase correctness (or both). If so, then the putative program (i.e. the one including the random mutations) replaces the current starting position, and another walk begins. I don't know if this one will prove promising or not. Here are the possible mutations it does:
 - Insert a random instruction
 - Delete an instruction at random
 - Modify a random instruction's operand
 - Change a random instruction's opcode for another one, having the same addressing mode
 - Pick two random instructions and swap them over
 - Pick one instruction, and overwrite it entirely with another one.

This will stop searching when the random walks stop finding improvements. I.e., if it's tried *n* times without finding a more optimal program, the search stops and the last found known good program is printed out. So here is an example run:
```
$ ./stoc-2a03 examples/test2.asm
; 2 instructions
; 4 bytes
; 4 clockticks
; hamming distance 0
	ldx #$23
	ldy #$01
```

### Equivalence testing
The way I'm currently testing two programs for equivalence is by running them a squillion times, each time with a random input. This means that the registers and memory locations read by the program are set to random values. Then the two programs are run. The output of the program is checked by looking in the live-out registers and live-out memory locations. If for the same input, the programs produce different output, then of course the programs are found to not be equivalent. Otherwise, the equivalence test passes. 

There is a small chance that this test produces a false positive. This would happen if the numbers produced by the random number generator do not exercise all the code paths in the reference program for example. 

The equivalence tester may be invoked from the command line. This may be useful for debugging or other purposes. An example invocation:

```
./stoc-6510 --org:0200 reference.asm --eq:test.asm
a = 7b, x = 7b, y = 28
a = b5, x = 0f, y = 1b
a = 8c, x = 1a, y = 21
a = 14, x = 51, y = 1a
a = fd, x = 73, y = 4d
a = 5a, x = be, y = 44
a = a3, x = b7, y = 3f
a = 57, x = 47, y = 93
a = 1d, x = 23, y = 0c
a = 85, x = cf, y = 12
The two programs are not equivalent.
Above is a list of example inputs that give rise to differing behaviours.
```

This demonstrates how the equivalence tester works; the random inputs are listed if it means that they cause the programs `reference.asm` and `test.asm` to behave differently.
