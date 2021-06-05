#!/usr/bin/python3

import sys
from collections import namedtuple

Opcode = namedtuple("Opcode", "codepoint mnemonic mode")

opcodes = []

lengths = {
        "absolute": 3, 
        "absolute_x": 3,
        "absolute_y": 3,
        "indirect": 3,
        "zero_page": 2, 
        "zero_page_x": 2, 
        "zero_page_y": 2,
        "indirect_x": 2,
        "indirect_y": 2,
        "immediate": 2,
        "implied": 1,
        "relative": 2}

for line in sys.stdin:
    fields = line.split("\t")
    codepoint = fields[0].strip()
    mnemonic = fields[1].strip()
    mode = fields[2].strip()
    opc = Opcode(codepoint, mnemonic, mode)
    opcodes.append(opc)

print("#define M6502")
print("#include \"stoc.h\"")
print("#include <string.h>")
print("#include <stdio.h>")
print("#include <stdlib.h>")

# modes
for m in lengths:
	codes = sorted(["0x%s" % o.codepoint for o in opcodes if o.mode == m])
	print("pick_t mode_%s = {%d, {%s}};" % (m, len(codes), ", ".join(codes)))

print("pick_t * addressing_modes[] = {")
for c in range(256):
	o = [o.mode for o in opcodes if int(o.codepoint, 16) == c]
	if len(o):
		print("\t&mode_%s," % o[0])
	else:
		print("\t0,")
print("};")


# opcode_legal_p
print("\n\nbool opcode_legal_p(uint8_t op) {")
print("\tswitch(op) {")
for o in opcodes:
    print("\tcase 0x%s:" % o.codepoint)
print("\t\treturn true;");
print("\tdefault:\n\t\treturn false;\n\t}")
print("}")

# opcode_length
print("int opcode_length(uint8_t op) {")
print("\tswitch(op) {")
for o in opcodes:
    print("\tcase 0x%s: return %d; // %s %s" % (o.codepoint, lengths[o.mode], o.mnemonic, o.mode))
print("\tdefault:\n\t\treturn 0;\n\t}")
print("}")

print("int opcode_branch_p(uint8_t op) {")
print("\tswitch(op) {")

for o in [o for o in opcodes if o.mode == "relative"]:
    print("\tcase 0x%s:\t// %s %s" % (o.codepoint, o.mnemonic, o.mode))
print("\t\treturn true;");
print("\tdefault:\n\t\treturn false;\n\t}")
print("}")

for addr in lengths:
	print("int is_%s_instruction(uint8_t op) {" % addr)
	print("\tswitch(op) {")
	for o in [o for o in opcodes if o.mode == addr]:
		print("\tcase 0x%s:" % o.codepoint)
	print("\t\treturn 1;\n\tdefault:\t\treturn 0;")
	print("\t}\n}")
	print("bool %s_instruction(char * op, uint8_t * out) {" % addr)
	for o in [o for o in opcodes if o.mode == addr]:
		print("\tif(!strncmp(op, \"%s\", %u)) { *out = 0x%s; return true; }" % (o.mnemonic, len(o.mnemonic), o.codepoint))
	print("\treturn 0;")
	print("}")


print("char *opnames[256] = {")
for i in range(256):
	o = [o.mnemonic for o in opcodes if int(o.codepoint, 16) == i]
	if len(o):
		op = o[0]
		print("\t\"%s\", // %02x" % (o[0], i))
	else:
		print("\tNULL, // %02x" % i)
print("};")

c = []
for o in opcodes:
    if o.codepoint in c:
        print("Duplicate codepoint %s" % o.codepoint, file=sys.stderr)
        sys.exit(1)
    c.append(o.codepoint)

for o in [o for o in opcodes if o.mode not in lengths]:
    print("unknown mode %s" % o.mode, file=sys.stderr)
    print("Please check codepoint %s" % o.codepoint, file=sys.stderr)
    sys.exit(1)

