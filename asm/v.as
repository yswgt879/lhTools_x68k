        .ORIGIN:        40C5
        .CODE
        %10a    .EQU    10
                LD      L,%10a
        %01l:
                AND     (BC),00
                INC     BC
                DJC     %01l
                RET
        .END

