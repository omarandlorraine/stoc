	clc
	lda #$07
	sed
	clc
	adc #$05
	liveout a
	dead_code_elimination
