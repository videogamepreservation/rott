; rt_eng.ASM

.386P
.MODEL	flat

;============================================================================

.DATA

temphold       dd      0

EXTRN   _spotvis:DWORD
EXTRN   _mapseen:DWORD
EXTRN   _tilemap:DWORD
EXTRN   _rc_off:DWORD


;============================================================================

.CODE
SEGMENT text USE32 

EXTRN   IsOpaque_:NEAR

;============================
;
; RayCast
;
;============================

IDEAL
ALIGN   16
PROC	RayCast_
PUBLIC	RayCast_

; edi		incr
; eax		xtilestep 
; ebx     	ytilestep
; ecx		xstep 
; edx		ystep
; esi		xtile<<7+ytile

ALIGN   16
castloop:
        SETFLAG edi,edi
        jns     topcast
        mov     [BYTE PTR _spotvis+esi],1
bottomcast:
        add     esi,eax
        mov     [BYTE PTR _spotvis+esi],1
        add     edi,ecx
        cmp     [WORD PTR _tilemap+esi*2],0
        jz      castloop
bottomdone:
        pushad
        mov     eax,esi
        call    IsOpaque_
;        mov     [DWORD PTR temphold],eax
        cmp     eax,0
        popad
        jnz      castloop
        sub     edi,ecx
        mov     [BYTE PTR _mapseen+esi],1
        mov     [_rc_off],esi
        ret
topcast:
        add     esi,ebx
        mov     [BYTE PTR _spotvis+esi],1
        add     edi,edx
        cmp     [WORD PTR _tilemap+esi*2],0
        jz      castloop
topdone:
        pushad
        mov     eax,esi
        call    IsOpaque_
;        mov     [DWORD PTR temphold],eax
        cmp     eax,0
        popad
        jnz      castloop
        sub     edi,edx
        mov     [BYTE PTR _mapseen+esi],1
        mov     [_rc_off],esi
        ret

; Secondary cast loop for going through closed doors
modcastloop:
        cmp     [DWORD PTR temphold],0
        jnz     castloop
doorcastloop:
        SETFLAG edi,edi
        jns     doortopcast
doorbottomcast:
;        mov     [BYTE PTR _spotvis+esi],1
        add     esi,eax
        add     edi,ecx
        cmp     [WORD PTR _tilemap+esi*2],0
        jz      doorcastloop
doorbottomdone:
        pushad
        mov     eax,esi
        call    IsOpaque_
        cmp     eax,2
        popad
        jnz     doorcastloop
        sub     edi,ecx
        mov     [_rc_off],esi
        ret
doortopcast:
;        mov     [BYTE PTR _spotvis+esi],1
        add     esi,ebx
        add     edi,edx
        cmp     [WORD PTR _tilemap+esi*2],0
        jz      doorcastloop
doortopdone:
        pushad
        mov     eax,esi
        call    IsOpaque_
        cmp     eax,2
        popad
        jnz     doorcastloop
        sub     edi,edx
        mov     [_rc_off],esi
        ret
ENDP


ENDS

END

