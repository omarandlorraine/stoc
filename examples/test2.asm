	org $0200
test2: ldx	#$23
	ldy	#$01
	txa
	pha
	pla
	liveout x
	liveout y
	optimize
