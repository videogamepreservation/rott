        .386
        .MODEL  flat

.data

EXTRN   _texture_u:DWORD        ; esi
EXTRN   _texture_v:DWORD        ; ebx
EXTRN   _texture_count:DWORD    ; ecx
EXTRN   _texture_du:DWORD       ; self-modify
EXTRN   _texture_dv:DWORD       ; self-modify
EXTRN   _texture_source:DWORD   ; self-modify
EXTRN   _texture_dest:DWORD     ; self-modify
EXTRN   _texture_destincr:DWORD ; edx

.code
SEGMENT text USE32
ALIGN   16
;================
;
; TextureLine
;
;================
PROC   TextureLine_
PUBLIC   TextureLine_

;
; eax - scratch
; ebx - v
; ecx - count
; edx - destination
; esi - u
; edi - destination incrementer pointer
; ebp - source
;
;
;
   pushad
   mov    eax,[_texture_du]
   mov    ebx,OFFSET patch1+2
   mov    ecx,[_texture_dv]
   mov    [ebx],eax          ; patch du
   mov    edx,OFFSET patch2+2
   mov    eax,[_texture_source]
   mov    ebx,OFFSET patch3+2
   mov    [edx],ecx          ; patch dv
   mov    [ebx],eax          ; patch source
   mov    edx,OFFSET patch4+2
   mov    ecx,[_texture_dest]
   mov    esi,[_texture_u]
   mov    [edx],ecx          ; patch dest
   mov    ebx,[_texture_v]
   mov    ecx,[_texture_count]
   mov    edi,[_texture_destincr]

   ALIGN   16
textureloop:
   mov    ebp,ebx       ; move v into source
   mov    eax,esi         move u into scratch
patch2:
   add    ebx,12345678h ; increment v
   shr    ebp,10        ; shift off v's fraction and multiply by 64
patch1:
   add    esi,12345678h ; increment u
patch3:
   add    ebp,12345678h ; add source offset to source
   mov    edx,[edi]     ; move next destination offset into destination
   shr    eax,16        ; shift off u's fraction
patch4:
   add    edx,12345678h ; add destination offset to destination
   add    ebp,eax       ; add u component to source
   add    edi,1
   mov    al,[ebp]      ; get source pixel
   dec    ecx           ; done with loop?
   mov    [edx],al      ; plot pixel
   jnz    textureloop
   popad
   ret

ENDP

ENDS


END

