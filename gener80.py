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

# First add all the base opcodes; these are supported by the Intel 8080 and all others.

addopc("none", 0x00, "implied", "nop", "", 1)
for a,r in enumerate(rp):
	addopc("none", 1 + a * 8, "rpimm16", "ld %s, " % rp[a], "", 3)

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
		if len(o):
			oc = o[0]
			print("\t{altop_%s, rndoper_%s, \"%s\", \"%s\", %d}," %
				(oc.group, oc.group, oc.dis1, oc.dis2, oc.length))
		else:
			print("\t{0},")
	print("};\n\n")
