; rt_fc_a.ASM

.386P
.MODEL	flat

SCREENROW = 96

;============================================================================

.DATA
loopcount       dd      0
pixelcount      dd      0

EXTRN	_mr_rowofs:DWORD
EXTRN	_mr_count:DWORD
EXTRN	_mr_xstep:DWORD
EXTRN	_mr_ystep:DWORD
EXTRN	_mr_xfrac:DWORD
EXTRN	_mr_yfrac:DWORD
EXTRN	_mr_dest:DWORD
EXTRN   _shadingtable:DWORD


;============================================================================

.CODE
SEGMENT text USE32
        ALIGN   16

;============================
;
; DrawRow
;
;============================

IDEAL
        ALIGN   16
PROC	DrawRow_
PUBLIC	DrawRow_

        push    ebp
        mov     [pixelcount],ecx                                ; save for final pixel
        shr     ecx,1                                           ; double pixel count
        mov     [loopcount],ecx
;        mov     esi,[_mr_src]
	mov	ebx,[_mr_ystep]
        shl     ebx,16
	mov	eax,[_mr_xstep]
        mov     bx,ax
        mov     eax,OFFSET hpatch1+2            ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET hpatch2+2            ; convice tasm to modify code...
        mov     [eax],ebx
	mov	ebp,[_mr_yfrac]
	shl	ebp,16
	mov	eax,[_mr_xfrac]
	mov	bp,ax
;        mov     eax,[_shadingtable]

; eax           aligned colormap
; ebx           aligned colormap
; ecx,edx       scratch
; esi           virtual source
; edi           moving destination pointer
; ebp           frac

        shld    ecx,ebp,23                         ; begin calculating third pixel (y units)
        shld    ecx,ebp,7                          ; begin calculating third pixel (x units)
        add     ebp,ebx                                 ; advance frac pointer
        and     ecx,16383                            ; finish calculation for third pixel
        shld    edx,ebp,23                         ; begin calculating fourth pixel (y units)
        shld    edx,ebp,7                          ; begin calculating fourth pixel (x units)
        add     ebp,ebx                                 ; advance frac pointer
        and     edx,16383                            ; finish calculation for fourth pixel
        mov     eax,[_shadingtable]
        mov     ebx,eax
        mov     al,[esi+ecx]                    ; get first pixel
        mov     bl,[esi+edx]                    ; get second pixel
        mov     al,[eax]                                ; color translate first pixel
        mov     bl,[ebx]                                ; color translate second pixel

        test    [pixelcount],0fffffffeh
        jnz     hdoubleloop                             ; at least two pixels to map
        jmp     hchecklast


        ALIGN   16
hdoubleloop:
        shld    ecx,ebp,23                         ; begin calculating third pixel (y units)
        shld    ecx,ebp,7                          ; begin calculating third pixel (x units)
hpatch1:
        add     ebp,12345678h                      ; advance frac pointer
        mov     [edi],al                           ; write first pixel
        and     ecx,16383                          ; finish calculation for third pixel
        shld    edx,ebp,23                         ; begin calculating fourth pixel (y units)
        shld    edx,ebp,7                          ; begin calculating fourth pixel (x units)
hpatch2:
        add     ebp,12345678h                      ; advance frac pointer
        mov     [edi+1],bl                         ; write second pixel
        and     edx,16383                          ; finish calculation for fourth pixel
        mov     al,[esi+ecx]                       ; get third pixel
        add     edi,2                              ; advance to third pixel destination
        mov     bl,[esi+edx]                       ; get fourth pixel
        dec     [loopcount]                        ; done with loop?
        mov     al,[eax]                           ; color translate third pixel
        mov     bl,[ebx]                           ; color translate fourth pixel
        jnz     hdoubleloop

; check for final pixel
hchecklast:
        test    [pixelcount],1
        jz      hdone
        mov     [edi],al                                ; write final pixel

hdone:
        pop     ebp
        ret

ENDP








;============================
;
; DrawRotRow
;
;============================

IDEAL
        ALIGN   16
PROC	DrawRotRow_
PUBLIC	DrawRotRow_

        push    ebp
        mov     [pixelcount],ecx                                ; save for final pixel
        shr     ecx,1                                           ; double pixel count
        mov     [loopcount],ecx
	mov	ebx,[_mr_xstep]
        mov     edx,OFFSET hrpatch1+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET hrpatch1a+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET hrpatch1b+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET hrpatch1c+2            ; convice tasm to modify code...
        mov     [edx],ebx
	mov	ebx,[_mr_ystep]
        mov     edx,OFFSET hrpatch2+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET hrpatch2a+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET hrpatch2b+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET hrpatch2c+2            ; convice tasm to modify code...
        mov     [edx],ebx
	mov	ecx,[_mr_yfrac]
	mov	edx,[_mr_xfrac]

        mov     eax,edx                 ; Move yfrac into y
        mov     ebp,ecx                 ; Move xfrac into x
        shr     eax,16                  ; shift yfrac off
hrpatch1:
        add     edx,012345678h          ; increment y fraction
hrpatch2:
        add     ecx,012345678h          ; increment x fraction
        cmp     eax,255                 ; test bounds of y fraction
        ja      nok3
        shr     ebp,16                  ; shift xfrac off
        cmp     ebp,511                 ; test bounds of x fraction
        ja      nok3
        shl     ebp,23                  ;
        shld    eax,ebp,9               ; form composite offset
ok3:
        mov     bl,[esi+eax]            ; get first pixel
        mov     eax,edx                 ; Move yfrac into y
        mov     ebp,ecx                 ; Move xfrac into x
        shr     eax,16                  ; shift yfrac off
hrpatch1a:
        add     edx,012345678h          ; increment y fraction
hrpatch2a:
        add     ecx,012345678h          ; increment x fraction
        cmp     eax,255                 ; test bounds of y fraction
        ja      nok4
        shr     ebp,16                  ; shift xfrac off
        cmp     ebp,511                 ; test bounds of x fraction
        ja      nok4
        shl     ebp,23                  ;
        shld    eax,ebp,9               ; form composite offset
ok4:
        mov     bh,[esi+eax]            ; get second pixel

        test    [pixelcount],0fffffffeh
        jnz     hrdoubleloop                       ; at least two pixels to map
        jmp     hrchecklast
nok3:
        xor  eax,eax
        jmp  ok3
nok4:
        xor  eax,eax
        jmp  ok4

        ALIGN   16
hrdoubleloop:
        mov     eax,edx                 ; Move yfrac into y
        mov     ebp,ecx                 ; Move xfrac into x
        shr     eax,16                  ; shift yfrac off
hrpatch1b:
        add     edx,012345678h          ; increment y fraction
hrpatch2b:
        add     ecx,012345678h          ; increment x fraction
        cmp     eax,255                 ; test bounds of y fraction
        ja      nok1
        shr     ebp,16                  ; shift xfrac off
        cmp     ebp,511                 ; test bounds of x fraction
        ja      nok1
        shl     ebp,23                  ;
        shld    eax,ebp,9               ; form composite offset
ok1:
        mov     [edi],bl                ; write first pixel
        mov     ebp,ecx                 ; Move xfrac into x
        mov     bl,[esi+eax]            ; get third pixel

        mov     eax,edx                 ; Move yfrac into y
        shr     eax,16                  ; shift yfrac off
hrpatch1c:
        add     edx,012345678h          ; increment y fraction
hrpatch2c:
        add     ecx,012345678h          ; increment x fraction
        cmp     eax,255                 ; test bounds of y fraction
        ja      nok2
        shr     ebp,16                  ; shift xfrac off
        cmp     ebp,511                 ; test bounds of x fraction
        ja      nok2
        shl     ebp,23                  ;
        shld    eax,ebp,9               ; form composite offset
ok2:
        mov     [edi+1],bh                ; write second pixel
        add     edi,2
        dec     [loopcount]                     ; done with loop?
        mov     bh,[esi+eax]            ; get second pixel
        jnz     hrdoubleloop
; check for final pixel
hrchecklast:
        test    [pixelcount],1
        jz      hrdone
        mov     [edi],bl                                ; write final pixel

hrdone:
        pop     ebp
        ret

nok1:
        xor  eax,eax
        jmp  ok1
nok2:
        xor  eax,eax
        jmp  ok2


ENDP



;============================
;
; DrawMaskedRotRow
;
;============================

IDEAL
        ALIGN   16
PROC  DrawMaskedRotRow_
PUBLIC   DrawMaskedRotRow_

        push    ebp
        mov     [pixelcount],ecx                                ; save for final pixel
        shr     ecx,1                                           ; double pixel count
        mov     [loopcount],ecx
	mov	ebx,[_mr_xstep]
        mov     edx,OFFSET mhrpatch1+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET mhrpatch1a+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET mhrpatch1b+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET mhrpatch1c+2            ; convice tasm to modify code...
        mov     [edx],ebx
	mov	ebx,[_mr_ystep]
        mov     edx,OFFSET mhrpatch2+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET mhrpatch2a+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET mhrpatch2b+2            ; convice tasm to modify code...
        mov     [edx],ebx
        mov     edx,OFFSET mhrpatch2c+2            ; convice tasm to modify code...
        mov     [edx],ebx
	mov	ecx,[_mr_yfrac]
	mov	edx,[_mr_xfrac]

        mov     eax,edx                 ; Move yfrac into y
        mov     ebp,ecx                 ; Move xfrac into x
        shr     eax,16                  ; shift yfrac off
mhrpatch1:
        add     edx,012345678h          ; increment y fraction
mhrpatch2:
        add     ecx,012345678h          ; increment x fraction
        cmp     eax,255                 ; test bounds of y fraction
        ja      mnok3
        shr     ebp,16                  ; shift xfrac off
        cmp     ebp,511                 ; test bounds of x fraction
        ja      mnok3
        shl     ebp,23                  ;
        shld    eax,ebp,9               ; form composite offset
mok3:
        mov     bl,[esi+eax]            ; get first pixel
        mov     eax,edx                 ; Move yfrac into y
        mov     ebp,ecx                 ; Move xfrac into x
        shr     eax,16                  ; shift yfrac off
mhrpatch1a:
        add     edx,012345678h          ; increment y fraction
mhrpatch2a:
        add     ecx,012345678h          ; increment x fraction
        cmp     eax,255                 ; test bounds of y fraction
        ja      mnok4
        shr     ebp,16                  ; shift xfrac off
        cmp     ebp,511                 ; test bounds of x fraction
        ja      mnok4
        shl     ebp,23                  ;
        shld    eax,ebp,9               ; form composite offset
mok4:
        mov     bh,[esi+eax]            ; get second pixel

        test    [pixelcount],0fffffffeh
        jnz     mhrdoubleloop                       ; at least two pixels to map
        jmp     mhrchecklast
mnok3:
        xor  eax,eax
        jmp  mok3
mnok4:
        xor  eax,eax
        jmp  mok4

        ALIGN   16
mhrdoubleloop:
        mov     eax,edx                 ; Move yfrac into y
        mov     ebp,ecx                 ; Move xfrac into x
        shr     eax,16                  ; shift yfrac off
mhrpatch1b:
        add     edx,012345678h          ; increment y fraction
mhrpatch2b:
        add     ecx,012345678h          ; increment x fraction
        cmp     eax,255                 ; test bounds of y fraction
        ja      mnok1
        shr     ebp,16                  ; shift xfrac off
        cmp     ebp,511                 ; test bounds of x fraction
        ja      mnok1
        shl     ebp,23                  ;
        shld    eax,ebp,9               ; form composite offset
mok1:
        cmp     bl,0ffh
        je      skip1
        mov     [edi],bl                ; write first pixel
skip1:
        mov     ebp,ecx                 ; Move xfrac into x
        mov     bl,[esi+eax]            ; get third pixel

        mov     eax,edx                 ; Move yfrac into y
        shr     eax,16                  ; shift yfrac off
mhrpatch1c:
        add     edx,012345678h          ; increment y fraction
mhrpatch2c:
        add     ecx,012345678h          ; increment x fraction
        cmp     eax,255                 ; test bounds of y fraction
        ja      mnok2
        shr     ebp,16                  ; shift xfrac off
        cmp     ebp,511                 ; test bounds of x fraction
        ja      mnok2
        shl     ebp,23                  ;
        shld    eax,ebp,9               ; form composite offset
mok2:
        cmp     bh,0ffh
        je      skip2
        mov     [edi+1],bh                ; write second pixel
skip2:
        add     edi,2
        dec     [loopcount]                     ; done with loop?
        mov     bh,[esi+eax]            ; get second pixel
        jnz     mhrdoubleloop
; check for final pixel
mhrchecklast:
        test    [pixelcount],1
        jz      mhrdone
        mov     [edi],bl                                ; write final pixel

mhrdone:
        pop     ebp
        ret

mnok1:
        xor  eax,eax
        jmp  mok1
mnok2:
        xor  eax,eax
        jmp  mok2


ENDP


;============================
;
; DrawSkyPost
;
;
;============================

IDEAL
PROC	DrawSkyPost_
PUBLIC	DrawSkyPost_

;EDI - Destination for post
;ESI - Source data
;ECX - Length of post

        mov     ebx,ecx
        mov     edx,[_shadingtable]
        shr     ebx,1
        jz      dsextra
dsloop:
        mov     ax,[esi]
        mov     dl,al
        mov     al,[edx]                                ; color translate third pixel
        mov     [edi],al
        add     esi,2
        mov     dl,ah
        add     edi,SCREENROW*2
        mov     al,[edx]                                ; color translate third pixel
        mov     [edi-SCREENROW],al
        dec     ebx
        jnz     dsloop
        MASKFLAG ecx,1
        jz      done
dsextra:
        mov     al,[esi]
        mov     dl,al
        mov     al,[edx]                                ; color translate third pixel
        mov     [edi],al
done:
	ret

ENDP


ENDS

END

