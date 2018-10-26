
;you can put macros in comments!!!
;6502ADDR 65000


;my very first test of David Beazly's python assembler

;Addressing modes are as follows:
;         #value         ; immediate mode (8-bit value)
;         %value         ; zero-page mode
;         %value,X       ; zero-page X indexed
;         %value,Y       ; zero-page Y indexed
;         value          ; absolute
;         value,X        ; absolute Y indexed
;         value,Y        ; absolute Y indexed
;         [value]        ; indirect
;         [value,X]      ; indirect, X indexed
;         [value,Y]      ; indirect, Y indexed


NOP
NOP
LDA #170   ; $AA
STA 00     ;
LDA #00    ; 
INC %00    ;
STA 00     ;
INC %00    ;
LDA 00     ; 
NOP
NOP
JMP 65344  ; $FF40
