#!/usr/bin/python3

import sys
from collections import namedtuple

groups = ["implied", "rpimm16"]
prefixes = ["none", "ed", "cb", "dd", "fd", "ddcb", "fdcb"]

Opcode = namedtuple("Opcode", "prefix codepoint group dis1 dis2 length")

opcodes = []

rp = ["bc", "de", "hl", "sp"]
rp2 = ["bc", "de", "hl", "af"]
cc = ["nz", "z", "nc", "c", "po", "pe", "p", "m"]
alu = ["add a,", "adc a,", "sub", "sbc a,", "and", "xor", "or", "cp"]

def addopc(prefix, codepoint, group, dis1, dis2, length):
	assert prefix in prefixes
	assert codepoint in range(256)
	opcodes.append(Opcode(prefix, codepoint, group, dis1, dis2, length))

def codepoint(prefix=0, x=0, y=0, z=0, p=0, q=0):
	assert x in range(4)
	assert p in range(4)
	assert q in [0, 1]
	assert y in range(8)
	assert z in range(8)
	if y != 0:
		assert p == 0
		assert q == 0
	if p != 0 or q != 0:
		assert y == 0
	
	return z + y * 8 + q * 8 + p * 16 + x * 64



# First add all the base opcodes; these are supported by the Intel 8080 and all others.
addopc("none", 0x00, "implied", "nop", "", 0)
addopc("none", 0x27, "implied", "daa", "", 0)
addopc("none", 0x07, "implied", "rlca", "", 0)
addopc("none", 0x17, "implied", "rla", "", 0)
addopc("none", 0x0f, "implied", "rrca", "", 0)
addopc("none", 0x1f, "implied", "rra", "", 0)

for a,r in enumerate(rp):
	p = a << 4
	addopc("none", 1 + p, "rpimm16", "ld %s, " % rp[a], "", 2)
	addopc("none", 9 + p, "rpimm16", "add hl, %s" % rp[a], "", 0)

# Next add the LR35902 specific opcodes
if sys.argv[1] == "lr35902":
	addopc("none", codepoint(y=1), "rpimm16", "ld (", "), sp", 0)
	addopc("none", codepoint(y=2), "rpimm16", "stop", "", 0)


# Next add the Z80 specific opcodes
if sys.argv[1] == "z80":
	addopc("none", codepoint(y=1), "implied", "ex af, af'", "", 0)
# TODO: DJNZ



print("#include \"stoc.h\"")
print("#include \"arch-%s.h\"" % sys.argv[1])
print("#include <string.h>")
print("#include <stdio.h>")
print("#include <stdlib.h>")

o = [ "\n\tPREFIX_%s | 0x%02x" % (o.prefix.upper(), o.codepoint) for o in opcodes ]
print("pick_t all_instructions = {%d, {%s}};\n" % (len(o), ", ".join(o)))

for g in groups:
	o = [ "\n\tPREFIX_%s | 0x%02x" % (o.prefix.upper(), o.codepoint) for o in opcodes if o.group == g ]
	print("pick_t instructions_%s = {%d, {%s}};\n" % (g, len(o), ", ".join(o)))

for p in prefixes:
	print("instrdata_t instrdata_%s[256] = {" % p)
	for c in range(256):
		o = [ o for o in opcodes if o.prefix == p and o.codepoint == c]
		if len(o) == 1:
			oc = o[0]
			print("\t/* 0x%02x */ {disasm_%s, altop_%s, rndoper_%s, &instructions_%s, \"%s\", \"%s\", %d}," %
				(oc.codepoint, oc.group, oc.group, oc.group, oc.group, oc.dis1, oc.dis2, oc.length))
		elif len(o) > 1:
			print("ERROR: Duplicate codepoint 0x%02x" % c, file=sys.stderr);
		else:
			print("\t/* 0x%02x */ {0}," % c)
	print("};\n\n")
