.386p
.model small


 SC_INDEX   =  03C4h
 SC_MAPMASK =  2
 GC_INDEX   =  03CEh
 GC_MODE    =  5


 UPDATEWIDE =  20
 UPDATEHIGH =  13

 MaxJoyValue = 5000

.DATA


EXTRN _bufferofs        :DWORD
EXTRN _displayofs       :DWORD
EXTRN _ylookup          :DWORD
EXTRN _linewidth        :DWORD
EXTRN _blockstarts      :DWORD ;offsets from drawofs for each update block

EXTRN _Joy_xb : BYTE
EXTRN _Joy_yb : BYTE
EXTRN _Joy_xs : BYTE
EXTRN _Joy_ys : BYTE
EXTRN _Joy_x  : WORD
EXTRN _Joy_y  : WORD

EXTRN _update            :DWORD



.CODE

ALIGN  4

;=================
;
; VH_UpdateScreen
;
;=================

PROC   VH_UpdateScreen_
PUBLIC VH_UpdateScreen_

   pushad

   mov   edx,SC_INDEX
   mov   eax,SC_MAPMASK+15*256

; write through all planes

   out   dx, ax

   mov   edx, GC_INDEX
   mov   al, GC_MODE
   out   dx, al

   inc   dx
   in    al, dx
   and   al, 252
   or    al, 1
   out   dx, al

   mov   ebx, UPDATEWIDE*UPDATEHIGH-1    ; bx is the tile number
   mov   edx, [_linewidth]

;
; see if the tile needs to be copied
;
@@checktile:
   test  [_update+ebx],1
   jnz   @@copytile
@@next:
   dec   ebx
   jns   @@checktile

;
; done
;
   mov   dx, GC_INDEX+1
   in    al, dx
   and   al, NOT 3
   or    al, 0
   out   dx, al


   popad

   ret

;
; copy a tile
;
@@copytile:
   mov   [BYTE PTR _update+ebx], 0
   mov   esi, [_blockstarts+ebx*4]
   mov   edi, esi
   add   esi, [_bufferofs]
   add   edi, [_displayofs]

;   mov   ax, SCREENSEG
;   mov   ds, ax

REPT  16
   mov   al, [esi]
   mov   [edi],   al
   mov   al, [esi+1]
   mov   [edi+1], al
   mov   al, [esi+2]
   mov   [edi+2], al
   mov   al, [esi+3]
   mov   [edi+3], al
   add   esi, edx
   add   edi, edx
ENDM

   jmp   @@next

ENDP VH_UpdateScreen_


ALIGN  4

;=================
;
; JoyStick_Vals
;
;=================

PROC   JoyStick_Vals_
PUBLIC JoyStick_Vals_


; Read the absolute joystick values

   pushf                ; Save some registers
   push  ebp
   cli                  ; Make sure an interrupt doesn't screw the timings

   mov   dx, 0201h
   in    al, dx
   out   dx, al         ; Clear the resistors

   mov   ah, BYTE PTR [_Joy_xb]   ; Get masks into registers
   mov   ch, BYTE PTR [_Joy_yb]

   xor   si, si         ; Clear count registers
   xor   di, di
   xor   bh, bh         ; Clear high byte of bx for later

   mov   ebp, MaxJoyValue

   @@LOOP:
   in    al, dx         ; Get bits indicating whether all are finished

   dec   ebp             ; Check bounding register
   jz    done           ; We have a silly value - abort

   mov   bl, al         ; Duplicate the bits
   and   bl, ah         ; Mask off useless bits (in [xb])
   add   si, bx         ; Possibly increment count register
   mov   cl, bl         ; Save for testing later

   mov   bl, al
   and   bl, ch         ; [yb]
   add   di, bx

   add   cl, bl
   jnz   @@LOOP         ; If both bits were 0, drop out

   done:

   mov   cl, [_Joy_xs]  ; Get the number of bits to shift
   shr   si, cl         ;  and shift the count that many times

   mov   cl, [_Joy_ys]
   shr   di, cl

   mov   [_Joy_x], si   ; Store the values into the variables
   mov   [_Joy_y], di

   pop   ebp
   popf                 ; Restore the registers

   ret

ENDP   JoyStick_Vals_


END

