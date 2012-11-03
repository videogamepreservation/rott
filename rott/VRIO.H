/*
Copyright (C) 1994-1995 Apogee Software, Ltd.

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

See the GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

*/
//
// Virtual Reality ROTT API
//
//

//
// All angles in ROTT are [0..2048)
// there are no negative angles so all angles must be normalized
//

// INT 0x33 AX = 0x30
// GetVRInput
//
// Registers passed into interrupt:
//
// BX = current player angle [0..2048)
// CX = current player tilt angle [0..2048)
//      looking up angles [0..171]
//      looking down angles [2047..1876]
//
// Registers after interrupt:
//
// BX = button status of VR input device
//      bits are as follow:
//      VR_RUNBUTTON           (0)
//      VR_STRAFELEFTBUTTON    (1)
//      VR_STRAFERIGHTBUTTON   (2)
//      VR_ATTACKBUTTON        (3)
//      VR_LOOKUPBUTTON        (4)
//      VR_LOOKDOWNBUTTON      (5)
//      VR_SWAPWEAPONBUTTON    (6)
//      VR_USEBUTTON           (7)
//      VR_HORIZONUPBUTTON     (8)
//      VR_HORIZONDOWNBUTTON   (9)
//      VR_MAPBUTTON           (10)
//      VR_PISTOLBUTTON        (11)
//      VR_DUALPISTOLBUTTON    (12)
//      VR_MP40BUTTON          (13)
//      VR_MISSILEWEAPONBUTTON (14)
//      VR_RECORDBUTTON        (15)
//
// CX = Mouse X mickeys
// DX = Mouse Y mickeys


// INT 0x33 AX = 0x31
// VRFeedback
//
// Registers passed into interrupt:
//
// BX = [0..1]
//      0 - Stop Feedback
//      1 - Start Feedback
//
// CX = [1..2]
//      0 - Gun Weapon
//      1 - Missile Weapon

