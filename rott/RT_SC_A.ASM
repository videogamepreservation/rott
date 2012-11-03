        .386
        .MODEL  flat


SCREENWIDTH = 96


.data
loopcount       dd      0
pixelcount      dd      0
EXTRN   _centery:DWORD
EXTRN   _centeryclipped:DWORD
EXTRN   _dc_yl:DWORD
EXTRN   _dc_yh:DWORD
EXTRN   _dc_iscale:DWORD
EXTRN   _dc_texturemid:DWORD
EXTRN   _ylookup:DWORD
;EXTRN   _dc_firstsource:DWORD
EXTRN   _dc_source:DWORD
EXTRN   _shadingtable:DWORD

.code
SEGMENT text USE32
        ALIGN   16

;================
;
; R_DrawColumn
;
;================
PROC   R_DrawColumn_
PUBLIC   R_DrawColumn_
        push    ebp
        mov     ebp,[_dc_yl]
        mov     ebx,ebp
        add     edi,[_ylookup+ebx*4]
        mov     eax,[_dc_yh]
        inc     eax
        sub     eax,ebp                         ; pixel count
        mov     [pixelcount],eax                ; save for final pixel
        js      done                            ; nothing to scale
        shr     eax,1                           ; double pixel count
        mov     [loopcount],eax

        mov     ecx,[_dc_iscale]

        mov     eax,[_centery]
        sub     eax,ebp
        imul    ecx
        mov     ebp,[_dc_texturemid]
        sub     ebp,eax

        mov     esi,[_dc_source]


        mov     ebx,[_dc_iscale]
        mov     eax,OFFSET patch1+2             ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET patch2+2             ; convice tasm to modify code...
        mov     [eax],ebx

; eax           aligned colormap
; ebx           aligned colormap
; ecx,edx       scratch
; esi           virtual source
; edi           moving destination pointer
; ebp           frac

        mov     ecx,ebp                         ; begin calculating first pixel
        add     ebp,ebx                         ; advance frac pointer
        shr     ecx,16                          ; finish calculation for first pixel
        mov     edx,ebp                         ; begin calculating second pixel
        add     ebp,ebx                         ; advance frac pointer
        shr     edx,16                          ; finish calculation for second pixel
        mov     eax,[_shadingtable]
        mov     ebx,eax
        ;mov     [_dc_firstsource],esi
        ;add     [_dc_firstsource],ecx
        mov     al,[esi+ecx]                    ; get first pixel
        mov     bl,[esi+edx]                    ; get second pixel
        mov     al,[eax]                        ; color translate first pixel
        mov     bl,[ebx]                        ; color translate second pixel

        test    [pixelcount],0fffffffeh
        jnz     doubleloop                      ; at least two pixels to map
        jmp     checklast

        ALIGN   16
doubleloop:
        mov     ecx,ebp                         ; begin calculating third pixel
patch1:
        add     ebp,12345678h                   ; advance frac pointer
        mov     [edi],al                        ; write first pixel
        shr     ecx,16                          ; finish calculation for third pixel
        mov     edx,ebp                         ; begin calculating fourth pixel
patch2:
        add     ebp,12345678h                   ; advance frac pointer
        mov     [edi+SCREENWIDTH],bl            ; write second pixel
        shr     edx,16                                      ; finish calculation for fourth pixel
        mov     al,[esi+ecx]                    ; get third pixel
        add     edi,SCREENWIDTH*2               ; advance to third pixel destination
        mov     bl,[esi+edx]                    ; get fourth pixel
        dec     [loopcount]                     ; done with loop?
        mov     al,[eax]                        ; color translate third pixel
        mov     bl,[ebx]                        ; color translate fourth pixel
        jnz     doubleloop

; check for final pixel
checklast:
        test    [pixelcount],1
        jz      done
        mov     [edi],al                        ; write final pixel
done:
        pop     ebp
        ret

ENDP


;================
;
; R_DrawSolidColumn
;
;================
PROC   R_DrawSolidColumn_
PUBLIC   R_DrawSolidColumn_
        mov     ecx,[_dc_yl]
        mov     eax,ecx
        add     edi,[_ylookup+eax*4]
        mov     eax,[_dc_yh]
        inc     eax
        sub     eax,ecx                         ; pixel count
        mov     [pixelcount],eax                ; save for final pixel
        js      soliddone                       ; nothing to scale
        shr     eax,1                           ; double pixel count
        mov     [loopcount],eax

        test    [pixelcount],0fffffffeh
        jnz     soliddoubleloop                      ; at least two pixels to map
        jmp     solidchecklast

        ALIGN   16
soliddoubleloop:
        mov     [edi],bl                        ; write first pixel
        add     edi,SCREENWIDTH                 ; advance to third pixel destination
        mov     [edi],bl                        ; write second pixel
        add     edi,SCREENWIDTH                 ; advance to fourth pixel destination
        dec     [loopcount]                     ; done with loop?
        jnz     soliddoubleloop

; check for final pixel
solidchecklast:
        test    [pixelcount],1
        jz      soliddone
        mov     [edi],bl                        ; write final pixel
soliddone:
        ret

ENDP


;================
;
; R_TransColumn
;
;================
PROC   R_TransColumn_
PUBLIC   R_TransColumn_
        mov     ebx,[_dc_yl]
        mov     eax,[_dc_yh]
        add     edi,[_ylookup+ebx*4]
        inc     eax
        sub     eax,ebx                         ; pixel count
        mov     [pixelcount],eax                ; save for final pixel
        js      tdone                           ; nothing to scale
        shr     eax,1                           ; double pixel count
        mov     [loopcount],eax

; eax           aligned colormap
; ebx           aligned colormap
; esi           moving destination pointer
; edi           moving destination pointer

        mov     eax,[_shadingtable]
        mov     esi,edi
        mov     ebx,eax
        add     esi,SCREENWIDTH
        mov     al,[BYTE PTR edi]               ; get first pixel
        mov     bl,[BYTE PTR esi]               ; get second pixel
        mov     al,[BYTE PTR eax]               ; color translate first pixel
        mov     bl,[BYTE PTR ebx]               ; color translate second pixel

        test    [pixelcount],0fffffffeh
        jnz     tdoubleloop                     ; at least two pixels to map
        jmp     tchecklast

        ALIGN   16
tdoubleloop:
        mov     [edi],al                        ; write first pixel
        mov     [esi],bl                        ; write second pixel
        add     edi,SCREENWIDTH*2               ; advance to third pixel destination
        add     esi,SCREENWIDTH*2               ; advance to third pixel destination
        mov     al,[edi]                        ; get third pixel
        mov     bl,[esi]                        ; get fourth pixel
        mov     al,[eax]                        ; color translate third pixel
        dec     [loopcount]                     ; done with loop?
        mov     bl,[ebx]                        ; color translate fourth pixel
        jnz     tdoubleloop

; check for final pixel
tchecklast:
        test    [pixelcount],1
        jz      tdone
        mov     [edi],al                        ; write final pixel
tdone:
        ret

ENDP








;================
;
; R_DrawClippedColumn
;
;================
PROC   R_DrawClippedColumn_
PUBLIC   R_DrawClippedColumn_
        push    ebp
        mov     ebp,[_dc_yl]
        mov     ebx,ebp
        add     edi,[_ylookup+ebx*4]
        mov     eax,[_dc_yh]
        inc     eax
        sub     eax,ebp                         ; pixel count
        mov     [pixelcount],eax                ; save for final pixel
        js      adone                            ; nothing to scale
        shr     eax,1                           ; double pixel count
        mov     [loopcount],eax

        mov     ecx,[_dc_iscale]

        mov     eax,[_centeryclipped]
        sub     eax,ebp
        imul    ecx
        mov     ebp,[_dc_texturemid]
        sub     ebp,eax

        mov     esi,[_dc_source]


        mov     ebx,[_dc_iscale]
        mov     eax,OFFSET apatch1+2            ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET apatch2+2            ; convice tasm to modify code...
        mov     [eax],ebx

; eax           aligned colormap
; ebx           aligned colormap
; ecx,edx       scratch
; esi           virtual source
; edi           moving destination pointer
; ebp           frac

        mov     ecx,ebp                         ; begin calculating first pixel
        add     ebp,ebx                         ; advance frac pointer
        shr     ecx,16                          ; finish calculation for first pixel
        mov     edx,ebp                         ; begin calculating second pixel
        add     ebp,ebx                         ; advance frac pointer
        shr     edx,16                          ; finish calculation for second pixel
        mov     eax,[_shadingtable]
        mov     ebx,eax
        mov     al,[esi+ecx]                    ; get first pixel
        mov     bl,[esi+edx]                    ; get second pixel
        mov     al,[eax]                        ; color translate first pixel
        mov     bl,[ebx]                        ; color translate second pixel

        test    [pixelcount],0fffffffeh
        jnz     adoubleloop                     ; at least two pixels to map
        jmp     achecklast

        ALIGN   16
adoubleloop:
        mov     ecx,ebp                         ; begin calculating third pixel
apatch1:
        add     ebp,12345678h                   ; advance frac pointer
        mov     [edi],al                        ; write first pixel
        shr     ecx,16                          ; finish calculation for third pixel
        mov     edx,ebp                         ; begin calculating fourth pixel
apatch2:
        add     ebp,12345678h                   ; advance frac pointer
        mov     [edi+SCREENWIDTH],bl            ; write second pixel
        shr     edx,16                          ; finish calculation for fourth pixel
        mov     al,[esi+ecx]                    ; get third pixel
        add     edi,SCREENWIDTH*2               ; advance to third pixel destination
        mov     bl,[esi+edx]                    ; get fourth pixel
        dec     [loopcount]                     ; done with loop?
        mov     al,[eax]                        ; color translate third pixel
        mov     bl,[ebx]                        ; color translate fourth pixel
        jnz     adoubleloop

; check for final pixel
achecklast:
        test    [pixelcount],1
        jz      adone
        mov     [edi],al                                ; write final pixel
adone:
        pop     ebp
        ret

ENDP


;================
;
; R_DrawWallColumn
;
;================
PROC     R_DrawWallColumn_
PUBLIC   R_DrawWallColumn_
        push    ebp
        mov     ebp,[_dc_yl]
        mov     ebx,ebp
        add     edi,[_ylookup+ebx*4]
        mov     eax,[_dc_yh]
;        inc     eax
        sub     eax,ebp                         ; pixel count
        mov     [pixelcount],eax                ; save for final pixel
        js      wcdone                            ; nothing to scale
        shr     eax,1                           ; double pixel count
        mov     [loopcount],eax

        mov     ecx,[_dc_iscale]

        mov     eax,[_centery]
        sub     eax,ebp
        imul    ecx
        mov     ebp,[_dc_texturemid]
        sub     ebp,eax
        shl     ebp,10

        mov     esi,[_dc_source]


        mov     ebx,[_dc_iscale]
        shl     ebx,10
        mov     eax,OFFSET wcpatch1+2             ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET wcpatch2+2             ; convice tasm to modify code...
        mov     [eax],ebx

; eax           aligned colormap
; ebx           aligned colormap
; ecx,edx       scratch
; esi           virtual source
; edi           moving destination pointer
; ebp           frac

        mov     ecx,ebp                         ; begin calculating first pixel
        add     ebp,ebx                         ; advance frac pointer
        shr     ecx,26                          ; finish calculation for first pixel
        mov     edx,ebp                         ; begin calculating second pixel
;        and     ecx,63
        add     ebp,ebx                         ; advance frac pointer
        shr     edx,26                          ; finish calculation for second pixel
        mov     eax,[_shadingtable]
;        and     edx,63
        mov     ebx,eax
        mov     al,[esi+ecx]                    ; get first pixel
        mov     bl,[esi+edx]                    ; get second pixel
        mov     al,[eax]                        ; color translate first pixel
        mov     bl,[ebx]                        ; color translate second pixel

        test    [pixelcount],0fffffffeh
        jnz     wcdoubleloop                      ; at least two pixels to map
        jmp     wcchecklast

        ALIGN   16
wcdoubleloop:
        mov     ecx,ebp                         ; begin calculating third pixel
wcpatch1:
        add     ebp,12345678h                   ; advance frac pointer
        mov     [edi],al                        ; write first pixel
        shr     ecx,26                          ; finish calculation for third pixel
        mov     edx,ebp                         ; begin calculating fourth pixel
;        and     ecx,63
wcpatch2:
        add     ebp,12345678h                   ; advance frac pointer
        mov     [edi+SCREENWIDTH],bl            ; write second pixel
        shr     edx,26                                      ; finish calculation for fourth pixel
        mov     al,[esi+ecx]                    ; get third pixel
;        and     edx,63
        add     edi,SCREENWIDTH*2               ; advance to third pixel destination
        mov     bl,[esi+edx]                    ; get fourth pixel
        dec     [loopcount]                     ; done with loop?
        mov     al,[eax]                        ; color translate third pixel
        mov     bl,[ebx]                        ; color translate fourth pixel
        jnz     wcdoubleloop

; check for final pixel
wcchecklast:
        test    [pixelcount],1
        jz      wcdone
        mov     [edi],al                        ; write final pixel
wcdone:
        pop     ebp
        ret

ENDP


ENDS


END

