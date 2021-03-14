     cmp #$0
     beq ret
     bmi min
     lda #$01
     bne ret
min  lda #255
ret  clc
     liveoutregisters	a
     liveinregisters	a
