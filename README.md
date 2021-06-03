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

The above list is essentially what's provided by the *fake6502* submodule. If you are interested in adding other architectures, I would suggest that the easiest way would be to graft in another emulator. At build-time, a particular emulator is linked in, and this is what determines which architecture the binary supports. A separate program is built for each supported architecture.

### Theory of operation
The basic idea with this is to generate better programs than traditional compilers can, by copying a working program and making many small random successive changes to it. If the copy is found to be equivalent (or close enough), then it might get written to the standard output. Otherwise, another attempt is made, until an improvement is found.

There are a few different ways we can introduce mutations into the program, and these have names such as Dead Code Elimination, or `.dce`, Stochastic Optimisation, or `.opt`, etc. They are described in more detail below. 

How can we see if two programs are equivalent? We can spam them with random numbers, and then check whether they produce the same output. Earlier versions of stoc worked in this way, but there was a slight chance that the random numbers didn't exercise the entire program. This could lead to a buggy program being output. To mitigate this risk, I've introduced the concept of testcases. A testcase is partially derived from the RNG and partially derived from the reference program. A testcase specifies what output the program should yield given a specific input. stoc will remember a number of these, and test each putative program against them all. I would guess that 99% of the garbage produced by the search algorithm is caught by the first few testcases.

### Dead Code Elimination
This search strategy looks for a more optimal rewrite by selecting random instructions for deletion; up to five at a time (this is to give pairs of instructions, such as a `pha` and corresponding `pla`, a chance to get deleted together). If the program proves to be equivalent without the selected instructions, then the instructions are deleted and the same procedure is done again. Use this procedure by using the `.dce` action.

```
$ ./stoc-2a03 examples/add_two_constants.stoc .dis .dce
; starting at $2000
; 5 instructions
; 7 bytes
; 5 clockticks
	clc
	lda #$07
	sed
	clc
	adc #$05

; starting at $2000
; 3 instructions
; 5 bytes
; 18 clockticks
	lda #$07
	clc
	adc #$05

```

It might be worth noting that the input procedure above contains two instances of the `clc` instructions, and only one is needed. Either one may be deleted, and it is picked at random. Running the same program again might have yielded the instructions `clc` and `lda #$07` in a different order. The `sed` instruction is not needed at all on the 2A03 because this is a chip variant which lacks the decimal mode. On other varieties of the 6502, as emulated by stoc-6510 for example, the `sed` instruction will be deemed necessary by the equivalence tester.

### Stochastic optimisation
This search strategy walks around the search space by trying a number of mutations at a time, at sees if these mutations together either lower the cost or increase correctness (or both). If so, then the putative program (i.e. the one including the random mutations) replaces the current starting position, and another walk begins. I don't know if this one will prove promising or not. Here are the possible mutations it does:
 - Insert a random instruction
 - Delete an instruction at random
 - Modify a random instruction's operand
 - Change a random instruction's opcode for another one, having the same addressing mode
 - Pick two random instructions and swap them over
 - Pick one instruction, and overwrite it entirely with another one.

This will stop searching when the random walks stop finding improvements. I.e., if it's tried *n* times without finding a more optimal program, the search stops and the last found known good program is printed out. Invoke this search with the `.opt` action. So here is an example run:
```
$ ./stoc-2a03 examples/add_two_constants.stoc .dis .opt
; starting at $2000
; 5 instructions
; 7 bytes
; 5 clockticks
	clc
	lda #$07
	sed
	clc
	adc #$05

; starting at $2000
; 1 instructions
; 2 bytes
; 1 clockticks
	lda #$0c

```
The search has discovered that adding two constants together is equivalent to loading the sum of those constants, and suggested a replacement program that does so.
