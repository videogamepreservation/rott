; Rise of the Triad Modem String File
; Revision 2 - January 30th, 1995
;
; Rise of the Triad is (c) 1994 Apogee Software, Ltd.
;
; You can add your own strings if you so desire.  If you do this, however, you
; must follow the format of this file, or you may not be able to properly pick
; any modems at all.  Editing rules:
;
; 1) Line #1 is the name of the modem you wish to use
; 2) Line #2 is the initialization string.  We urge you to use &F before any
;    other commands.  This will restore the modem to factory defaults on most
;    every Hayes compatible modem out there.  Check your manual to ensure that
;    &F is the factory default command on your modem.
; 3) The third line is the hangup command.
; 4) The fourth line is blank.  The blank line is important.
; 5) Anything that starts with a semicolon (;) is ignored.
;
; ---------------------------------------------------------------------------
; Tested Strings
; ---------------------------------------------------------------------------
;
; The modems listed in this section of the file were actually used by Apogee
; Software during the testing of this game.  They worked for us, and they
; should work for you without any further intervention besides picking them
; in the ROTTHELP.EXE program that comes with ROTT.  If you run into trouble,
; please check the ROTTHELP.EXE file that comes with Rise of the Triad.
;

ATI 9600 ETC-E Internal v.32
AT &F &C1 &D1 &K0 &Q6 S36=3
AT Z H0

Hayes Optima/Accura External 144 v.32bis
AT &F &C1 &D1 &K0 &Q6 S36=3
AT Z H0

Hayes Optima/Accura External 288 v.fc
AT &F &C1 &D1 &K0 &Q6 S36=3
AT Z H0

Practical Peripherals 14.4
AT Z S46=0 &Q0 &D2
AT Z H0

Practical Peripherals PM14400FXMT v.32bis
AT Z S46=0 &Q0 &D2
AT Z H0

Practical Peripherals PM14400FXSA v.32bis
AT Z S46=0 &Q0 &D2
AT Z H0

Supra Fax 288 v.fc
AT &F &C1 &D1 &K0 &Q6 S36=3
AT Z H0

USRobotics Sportster 14.4 Internal
AT &F &K0 &H0 &I0 &M0 &D1
AT Z H0

USRobotics 16.8 HST/Dual Standard
AT &F &K0 &H0 &I0 &M0 &D1
AT Z H0

USRobotics V.Everything
AT &F &K0 &H0 &I0 &M0 &D1
AT Z H0

ZyXel U-1496B v.32bis
AT Z S46=0 &D2 &K0
AT Z H0
