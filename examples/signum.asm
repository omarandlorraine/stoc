     cmp #$0
     beq ret
     bmi min
     lda #$01
     bne ret
min  lda #$ff
ret  clc
     liveoutregisters	a
     liveinregisters	a
