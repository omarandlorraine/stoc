#!/usr/bin/python3

import sys
from collections import namedtuple

Opcode = namedtuple("Opcode", "codepoint mnemonic mode")

opcodes = []

lengths = {
        "absolute": 3, 
        "absolute-x": 3,
        "absolute-y": 3,
        "indirect": 3,
        "zero-page": 2, 
        "zero-page-x": 2, 
        "zero-page-y": 2,
        "indirect-x": 2,
        "indirect-y": 2,
        "immediate": 2,
        "implied": 1,
        "relative": 2,
        "accumulator": 1}

read_instrs = ["asl", "ror" ]

branches = {
        "bne": "!(c.status & ZEROFLAG)",
        "beq":   "c.status & ZEROFLAG",
        "bvc": "!(c.status & OVERFLOWFLAG)",
        "bvs":   "c.status & OVERFLOWFLAG",
        "bmi": "!(c.status & SIGNFLAG)",
        "bpl":   "c.status & SIGNFLAG",
        "bcc": "!(c.status & CARRYFLAG)",
        "bcs":   "c.status & CARRYFLAG",
        "bra": "1"
        }

for line in sys.stdin:
    fields = line.split("\t")
    codepoint = fields[0].strip()
    mnemonic = fields[1].strip()
    mode = fields[2].strip()
    opc = Opcode(codepoint, mnemonic, mode)
    opcodes.append(opc)

print("#include \"stoc.h\"")
print("#include <string.h>")
print("#include <stdio.h>")
print("#include <stdlib.h>")
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
	mode = addr.replace('-', '_')
	print("int is_%s_instruction(uint8_t op) {" % mode)
	print("\tswitch(op) {")
	for o in [o for o in opcodes if o.mode == addr]:
		print("\tcase 0x%s:" % o.codepoint)
	print("\t\treturn 1;\n\tdefault:\t\treturn 0;")
	print("\t}\n}")
	print("uint8_t %s_instruction(char * op) {" % mode)
	for o in [o for o in opcodes if o.mode == addr]:
		print("\tif(!strncmp(op, \"%s\", %u)) return 0x%s;" % (o.mnemonic, len(o.mnemonic), o.codepoint))
	print("\tfprintf(stderr, \"No %s addressing mode for instruction \\\"%%s\\\"\\n\", op);" % addr)
	print("\texit(1);")
	print("}")

for o in list(set([o.mnemonic for o in opcodes])):
	print("extern void %s(Context65 * c);" % o)


print("void (*optable[256])(Context65 * c) = {")
for i in range(256):
	o = [o.mnemonic for o in opcodes if int(o.codepoint, 16) == i]
	if len(o):
		op = o[0]
		print("\t%s, // %02x" % (o[0], i))
	else:
		print("\tNULL, // %02x" % i)
print("};")
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

for o in [o for o in opcodes if o.mode not in ["absolute", "absolute-x", "absolute-y", "indirect", "zero-page", "zero-page-x", "zero-page-y", "indirect-x", "indirect-y", "immediate", "implied", "relative", "accumulator"]]:
    print("unknown mode %s" % o.mode, file=sys.stderr)
    print("Please check codepoint %s" % o.codepoint, file=sys.stderr)
    sys.exit(1)

