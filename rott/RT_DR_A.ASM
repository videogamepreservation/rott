.386p
.model flat

 SC_INDEX   =           03C4h
 SC_DATA    =           03C5h
 SC_RESET   =           0
 SC_CLOCK   =           1
 SC_MAPMASK =           2
 CRTC_INDEX =           03D4h
 SCREENBWIDE=           96
 MISC_OUTPUT=           03c2h
 CEILINGCOLOR=          24
 FLOORCOLOR=            32

.DATA

loopcount       dd      0
pixelcount      dd      0
EXTRN    _spotvis:BYTE
EXTRN    _viewwidth:DWORD
EXTRN    _viewheight:DWORD
EXTRN    _bufferofs:DWORD
EXTRN    _fandc:DWORD
EXTRN    _ylookup:DWORD
EXTRN    _centery:DWORD
EXTRN    _shadingtable:DWORD
EXTRN    _hp_startfrac:DWORD
EXTRN    _hp_srcstep:DWORD


.CODE
SEGMENT text USE32
        ALIGN   16


PROC   RefreshClear_
PUBLIC RefreshClear_

   pushad
   mov     edi,OFFSET _spotvis
   xor     eax,eax
   mov     ecx,01000h
; 128*128
   rep     stosd

   mov     eax,[_fandc]
   or      eax,eax
   jz     cont
   popad
   ret
; clear the screen
cont:
   mov     edx,SC_DATA
   mov     eax,15
; write through all planes
   out     dx,al
   mov     edx,SCREENBWIDE
   mov     eax,[_viewwidth]
   shr     eax,2
   sub     edx,eax

   mov     ebx,eax
   shr     ebx,1
   mov     ah,CEILINGCOLOR
   mov     al,CEILINGCOLOR
   mov     edi,[_bufferofs]
   mov     esi,[_centery]
   cmp     esi,0
   jle     skiptop
   cmp     esi,[_viewheight]
   jle     toploop
   mov     esi,[_viewheight]
toploop:
   mov     ecx,ebx
   rep     stosw
   add     edi,edx
   dec     esi
   jnz     toploop
skiptop:
   mov     al,FLOORCOLOR
   mov     ah,FLOORCOLOR
   mov     esi,[_viewheight]
   sub     esi,[_centery]
   cmp     esi,0
   jle     skipbottom
   cmp     esi,[_viewheight]
   jle     bottomloop
   mov     esi,[_viewheight]
bottomloop:
   mov     ecx,ebx
   rep     stosw
   add     edi,edx
   dec     esi
   jnz     bottomloop
skipbottom:
   popad
   ret

ENDP   RefreshClear_



PROC   SetMode240_
PUBLIC SetMode240_

   pushad
   mov   dx,SC_INDEX
   mov   ax,0604h
        out   dx,ax                 ; disable chain4 mode
        mov   ax,0100h
        out   dx,ax                 ; synchronous reset while setting Misc
                                    ;  Output for safety, even though clock
                                    ;  unchanged
        mov   dx,MISC_OUTPUT
        mov   al,0e3h
        out   dx,al                 ; select the dot clock and Horiz
        mov   dx,SC_INDEX
        mov   ax,0300h
        out   dx,ax                ; undo reset (restart sequencer)


        mov   dx,CRTC_INDEX       ; reprogram the CRT Controller
        mov   al,11h              ; VSync End reg contains register write
        out   dx,al               ; protect bit
        inc   dx                  ; CRT Controller Data register
        in    al,dx               ; get current VSync End register setting
        and   al,07fh             ; remove write protect on various
        out   dx,al               ; CRTC registers
        dec   dx                  ; CRT Controller Index
        cld
        mov   ax,00d06h
        out   dx,ax
        mov   ax,03e07h
        out   dx,ax
        mov   ax,04109h
        out   dx,ax
        mov   ax,0ea10h
        out   dx,ax
        mov   ax,0ac11h
        out   dx,ax
        mov   ax,0df12h
        out   dx,ax
        mov   ax,00014h
        out   dx,ax
        mov   ax,0e715h
        out   dx,ax
        mov   ax,00616h
        out   dx,ax
        mov   ax,0e317h
        out   dx,ax
        mov   ax,0f02h
        mov   dx,SC_INDEX
        out   dx,ax
        mov   edi,0a0000h
        mov   eax,0
        mov   ecx,08000h
        cld
        rep   stosw

   popad
   ret

ENDP   SetMode240_

;----------------------------------------------------------------------------
;
; DrawPost - Draws an unmasked post centered in the viewport
;
;            ecx - height to scale post to
;            esi - source pointer to proper column (x offsetted)
;            edi - destination pointer to proper screen column (xoffsetted)
;
;----------------------------------------------------------------------------


PROC    DrawPost_
PUBLIC  DrawPost_

; ECX - loop variable (height and counter)
; EDI - top offset + bufferofs
; EDX - destination offset
; EBX - high word contains bottom offset, low word contains source data
; EBP - fraction

  push ebp
  SETFLAG ecx,ecx
  jz    donedraw
  mov   edx,0
  mov   eax,32*65536
  div   ecx
  mov   edx,eax
; edx holds fractional step
  mov   [DWORD PTR patch1],edx
  mov   ebp,edx
  shr   ebp,1
  mov   eax,[_centery]
  add   edi,[_ylookup+4*eax]
  mov   edx,edi
  sub   edi,SCREENBWIDE
  mov   ebx,[_shadingtable]
  cmp   ecx,eax
  jle   heightok
  mov   ecx,eax
heightok:        ; height is ok.. < viewheigth

ALIGN 16

drawloop:
  mov   eax,ebp
  shr   eax,16
  mov   ax,[WORD PTR esi+eax*2]
patch1 equ $+2
  add   ebp,12345678h
  mov   bl,al
  mov   al,[ebx]
  mov   [edi],al
  mov   bl,ah
  sub   edi,SCREENBWIDE
  mov   ah,[ebx]
  mov   [edx],ah
  add   edx,SCREENBWIDE
  dec   ecx
  jnz   drawloop

donedraw:
  pop   ebp
  ret

ENDP DrawPost_



;============================
;
; DrawHeightPost
;
;============================

IDEAL
        ALIGN 16
PROC	DrawHeightPost_
PUBLIC	DrawHeightPost_

;EDI - Destination for post
;ESI - Source data
;ECX - Length of post
;EBX - shadingtable
;EBX - shadingtable

        push    ebp
        SETFLAG ecx,ecx
        jz      donehp
        mov     [pixelcount],ecx                ; save for final pixel
        shr     ecx,1                           ; double pixel count
        mov     [loopcount],ecx

        mov     ebp,[_hp_startfrac]
        mov     ebx,[_hp_srcstep]
        mov     eax,OFFSET hp1+2             ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET hp2+2             ; convice tasm to modify code...
        mov     [eax],ebx

; eax           shadingtable
; ebx           shadingtable
; ecx,edx       scratch
; esi           source
; edi           destination
; ebp           fraction

        mov     ecx,ebp                      ; begin calculating first pixel
        add     ebp,ebx                      ; advance frac pointer
        shr     ecx,16                       ; finish calculation for first pixel
        mov     edx,ebp                      ; begin calculating second pixel
        and     ecx,63
        add     ebp,ebx                      ; advance frac pointer
        shr     edx,16                       ; finish calculation for second pixel
        mov     eax,[_shadingtable]
        and     edx,63
        mov     ebx,eax
        mov     al,[esi+ecx]                 ; get first pixel
        mov     bl,[esi+edx]                 ; get second pixel
        mov     al,[eax]                     ; color translate first pixel
        mov     bl,[ebx]                     ; color translate second pixel

        test    [pixelcount],0fffffffeh
        jnz     dsloop                       ; at least two pixels to map
        jmp     last

ALIGN 16
dsloop:
        mov     ecx,ebp                      ; begin calculating third pixel
hp1:
        add     ebp,12345678h                ; advance frac pointer
        mov     [edi],al                     ; write first pixel
        shr     ecx,16                       ; finish calculation for third pixel
        mov     edx,ebp                      ; begin calculating fourth pixel
hp2:
        add     ebp,12345678h                ; advance frac pointer
        and     ecx,63
        mov     [edi+SCREENBWIDE],bl         ; write second pixel
        shr     edx,16                       ; finish calculation for fourth pixel
        mov     al,[esi+ecx]                 ; get third pixel
        and     edx,63
        add     edi,SCREENBWIDE*2            ; advance to third pixel destination
        mov     bl,[esi+edx]                 ; get fourth pixel
        dec     [loopcount]                  ; done with loop?
        mov     al,[eax]                     ; color translate third pixel
        mov     bl,[ebx]                     ; color translate fourth pixel
        jnz     dsloop

last:                                        ; one more?
        test    [pixelcount],1
        jz      donehp
        mov     [edi],al                     ; write final pixel
donehp:
        pop   ebp
	ret

ENDP


;============================
;
; DrawMenuPost
;
;============================

IDEAL
        ALIGN 16
PROC	DrawMenuPost_
PUBLIC	DrawMenuPost_

;EDI - Destination for post
;ESI - Source data
;ECX - Length of post
;EBX - shadingtable
;EBX - shadingtable

        push    ebp
        SETFLAG ecx,ecx
        jz      mdonehp
        mov     [pixelcount],ecx                ; save for final pixel
        shr     ecx,1                           ; double pixel count
        mov     [loopcount],ecx

        mov     ebp,[_hp_startfrac]
        mov     ebx,[_hp_srcstep]
        mov     eax,OFFSET mhp1+2             ; convice tasm to modify code...
        mov     [eax],ebx
        mov     eax,OFFSET mhp2+2             ; convice tasm to modify code...
        mov     [eax],ebx

; eax           shadingtable
; ebx           shadingtable
; ecx,edx       scratch
; esi           source
; edi           destination
; ebp           fraction

        mov     ecx,ebp                      ; begin calculating first pixel
        add     ebp,ebx                      ; advance frac pointer
        shr     ecx,16                       ; finish calculation for first pixel
        mov     edx,ebp                      ; begin calculating second pixel
        add     ebp,ebx                      ; advance frac pointer
        shr     edx,16                       ; finish calculation for second pixel
        mov     al,[esi+ecx]                 ; get first pixel
        mov     bl,[esi+edx]                 ; get second pixel

        test    [pixelcount],0fffffffeh
        jnz     mdsloop                       ; at least two pixels to map
        jmp     mlast

ALIGN 16
mdsloop:
        mov     ecx,ebp                      ; begin calculating third pixel
mhp1:
        add     ebp,12345678h                ; advance frac pointer
        mov     [edi],al                     ; write first pixel
        shr     ecx,16                       ; finish calculation for third pixel
        mov     edx,ebp                      ; begin calculating fourth pixel
mhp2:
        add     ebp,12345678h                ; advance frac pointer
        mov     [edi+SCREENBWIDE],bl         ; write second pixel
        shr     edx,16                       ; finish calculation for fourth pixel
        mov     al,[esi+ecx]                 ; get third pixel
        add     edi,SCREENBWIDE*2            ; advance to third pixel destination
        mov     bl,[esi+edx]                 ; get fourth pixel
        dec     [loopcount]                  ; done with loop?
        jnz     mdsloop

mlast:                                        ; one more?
        test    [pixelcount],1
        jz      mdonehp
        mov     [edi],al                     ; write final pixel
mdonehp:
        pop   ebp
	ret

ENDP


;============================
;
; DrawMapPost
;
;============================

IDEAL
        ALIGN 16
PROC	DrawMapPost_
PUBLIC	DrawMapPost_

;EDI - Destination for post
;ESI - Source data
;ECX - Length of post

        push    ebp
        SETFLAG ecx,ecx
        jz      mpdonehp
        mov     [pixelcount],ecx                ; save for final pixel
        shr     ecx,1                           ; double pixel count
        mov     [loopcount],ecx

        mov     ebp,0
        mov     ebx,[_hp_srcstep]

; ecx,edx       scratch
; esi           source
; edi           destination
; ebp           fraction

        mov     ecx,ebp                      ; begin calculating first pixel
        add     ebp,ebx                      ; advance frac pointer
        shr     ecx,16                       ; finish calculation for first pixel
        mov     edx,ebp                      ; begin calculating second pixel
        add     ebp,ebx                      ; advance frac pointer
        shr     edx,16                       ; finish calculation for second pixel
        mov     al,[esi+ecx]                 ; get first pixel
        mov     ah,[esi+edx]                 ; get second pixel

        test    [pixelcount],0fffffffeh
        jnz     mpdsloop                       ; at least two pixels to map
        jmp     mplast

ALIGN 16
mpdsloop:
        mov     ecx,ebp                      ; begin calculating third pixel
        add     ebp,ebx                      ; advance frac pointer
        mov     [edi],al                     ; write first pixel
        shr     ecx,16                       ; finish calculation for third pixel
        mov     edx,ebp                      ; begin calculating fourth pixel
        add     ebp,ebx                      ; advance frac pointer
        mov     [edi+SCREENBWIDE],ah         ; write second pixel
        shr     edx,16                       ; finish calculation for fourth pixel
        mov     al,[esi+ecx]                 ; get third pixel
        add     edi,SCREENBWIDE*2            ; advance to third pixel destination
        mov     ah,[esi+edx]                 ; get fourth pixel
        dec     [loopcount]                  ; done with loop?
        jnz     mpdsloop

mplast:                                        ; one more?
        test    [pixelcount],1
        jz      mpdonehp
        mov     [edi],al                     ; write final pixel
mpdonehp:
        pop   ebp
	ret

ENDP


ENDS

END








