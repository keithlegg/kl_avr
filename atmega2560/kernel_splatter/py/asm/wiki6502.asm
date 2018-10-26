; memcpy --
; Copy a block of memory from one location to another.
;
; Entry parameters
;      SRC - Address of source data block
;      DST - Address of target data block
;      CNT - Number of bytes to copy

            ORG     $0040       ;Parameters at $0040
SRC         DW      $0000
DST         DW      $0000
CNT         DW      $0000

            ORG     $0600       ;Code at $0600
MEMCPY      LDY     CNT+0       ;Set Y = CNT.L
            BNE     LOOP        ;If CNT.L > 0, then loop
            LDA     CNT+1       ;If CNT.H > 0,
            BNE     LOOP        ; then loop
            RTS                 ;Return
LOOP        LDA     (SRC),Y     ;Load A from ((SRC)+Y)
            STA     (DST),Y     ;Store A to ((DST)+Y)
            DEY                 ;Decr CNT.L
            BNE     LOOP        ;if CNT.L > 0, then loop
            INC     SRC+1       ;Incr SRC += $0100
            INC     DST+1       ;Incr DST += $0100
            DEY                 ;Decr CNT.L
            DEC     CNT+1       ;Decr CNT.H
            BNE     LOOP        ;If CNT.H > 0, then loop
            RTS                 ;Return
            END