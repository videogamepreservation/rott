        .386
        .MODEL  flat

SCREENROW = 96

;================
;
; R_DrawFilmColumn
;
;================

SCREENWIDTH = 96


.data

loopcount       dd      0
pixelcount      dd      0

_cin_yl          dd      0
_cin_yh          dd      0
_cin_ycenter     dd      0
_cin_iscale      dd      0
_cin_texturemid  dd      0
_cin_source      dd      0

PUBLIC _cin_yl       
PUBLIC _cin_yh         
PUBLIC _cin_ycenter    
PUBLIC _cin_iscale     
PUBLIC _cin_texturemid 
PUBLIC _cin_source   

EXTRN   _ylookup:DWORD

.code
SEGMENT text USE32
        ALIGN   16

PROC   R_DrawFilmColumn_
PUBLIC   R_DrawFilmColumn_
        push    ebp
        mov     ebp,[_cin_yl]
        mov     ebx,ebp
        add     edi,[_ylookup+ebx*4]
        mov     eax,[_cin_yh]
        inc     eax
        sub     eax,ebp                         ; pixel count
        mov     [pixelcount],eax                ; save for final pixel
        js      done                            ; nothing to scale
        shr     eax,1                           ; double pixel count
        mov     [loopcount],eax

        mov     ecx,[_cin_iscale]

        mov     eax,[_cin_ycenter]
        sub     eax,ebp
        imul    ecx
        mov     ebp,[_cin_texturemid]
        sub     ebp,eax

        mov     esi,[_cin_source]


        mov     ebx,[_cin_iscale]
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

        mov     ecx,ebp                                 ; begin calculating first pixel
        add     ebp,ebx                                 ; advance frac pointer
        shr     ecx,16                                      ; finish calculation for first pixel
        mov     edx,ebp                                 ; begin calculating second pixel
        add     ebp,ebx                                 ; advance frac pointer
        shr     edx,16                                      ; finish calculation for second pixel
        mov     al,[esi+ecx]                    ; get first pixel
        mov     bl,[esi+edx]                    ; get second pixel

        test    [pixelcount],0fffffffeh
        jnz     doubleloop                              ; at least two pixels to map
        jmp     checklast

        ALIGN   16
doubleloop:
        mov     ecx,ebp                                 ; begin calculating third pixel
patch1:
        add     ebp,12345678h                   ; advance frac pointer
        mov     [edi],al                                ; write first pixel
        shr     ecx,16                                      ; finish calculation for third pixel
        mov     edx,ebp                                 ; begin calculating fourth pixel
patch2:
        add     ebp,12345678h                   ; advance frac pointer
        mov     [edi+SCREENWIDTH],bl    ; write second pixel
        shr     edx,16                                      ; finish calculation for fourth pixel
        mov     al,[esi+ecx]                    ; get third pixel
        add     edi,SCREENWIDTH*2               ; advance to third pixel destination
        mov     bl,[esi+edx]                    ; get fourth pixel
        dec     [loopcount]                             ; done with loop?
        jnz     doubleloop

; check for final pixel
checklast:
        test    [pixelcount],1
        jz      done
        mov     [edi],al                                ; write final pixel
done:
        pop     ebp
        ret

ENDP



;============================
;
; DrawFilmPost
;
;
;============================

IDEAL
PROC	DrawFilmPost_
PUBLIC	DrawFilmPost_

;EDI - Destination for post
;ESI - Source data
;ECX - Length of post

        mov     ebx,ecx
        shr     ebx,1
        jz      dfextra
dfloop:
        mov     ax,[esi]
        mov     [edi],al
        add     esi,2
        add     edi,SCREENROW*2
        mov     [edi-SCREENROW],ah
        dec     ebx
        jnz     dfloop
        MASKFLAG ecx,1
        jz      dfdone
dfextra:
        mov     al,[esi]
        mov     [edi],al
dfdone:
	ret

ENDP

ENDS


END

