/****************************************************************************

    AstroMenace
    Hardcore 3D space scroll-shooter with spaceship upgrade possibilities.
    Copyright (c) 2006-2019 Mikhail Kurinnoi, Viewizard


    AstroMenace is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    AstroMenace is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with AstroMenace. If not, see <https://www.gnu.org/licenses/>.


    Website: https://viewizard.com/
    Project: https://github.com/viewizard/astromenace
    E-mail: viewizard@viewizard.com

*****************************************************************************/

#include "../core/core.h"
#include "../assets/audio.h"
#include "../assets/texture.h"
#include "../ui/cursor.h"
#include "../game.h" // FIXME "game.h" should be replaced by individual headers
#include "SDL2/SDL.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

extern bool DragWeapon;



// for keyboard control (for bypass menu items)
int CurrentActiveMenuElement = 0;
int CurrentKeyboardSelectMenuElement = 0;




// for mouse/joystick control, play only one SFX instance at moves over button
// hold last mouse position coordinates (X, Y)
int NeedPlayOnButtonSoundX = 0;
int NeedPlayOnButtonSoundY = 0;



//------------------------------------------------------------------------------------
// 384 button
//------------------------------------------------------------------------------------
bool DrawButton384(int X, int Y, const std::u32string &Text, float Transp, float &ButTransp, float &Update)
{
    sRECT SrcRect, DstRect;
    bool ON = false;
    float IntTransp = Transp;


    // keyboard
    if (Transp >= 0.99f && !isDialogBoxDrawing() && GetShowGameCursor()) {
        CurrentActiveMenuElement++;
    }

    bool InFocusByKeyboard = false;
    if (CurrentKeyboardSelectMenuElement > 0
        && CurrentKeyboardSelectMenuElement == CurrentActiveMenuElement) {
        InFocusByKeyboard = true;
    }



    DstRect(X+2,Y+1,X+384,Y+63);
    if ((vw_MouseOverRect(DstRect) || InFocusByKeyboard)
        && !isDialogBoxDrawing() && GetShowGameCursor()) {
        ON = true;
        if (Transp == 1.0f) { // in case showing/hiding menu - button must be not active
            SetCursorStatus(eCursorStatus::ActionAllowed);
        }

        if (ButTransp == 1.0f) {
            PlayMenuSFX(eMenuSFX::OverBigButton, 1.0f);
            Update = vw_GetTimeThread(0);
            ButTransp = 0.99f;
        }

        ButTransp -= 3.0f*(vw_GetTimeThread(0) - Update);
        if (ButTransp < 0.60f) {
            ButTransp = 0.60f;
        }
        Update = vw_GetTimeThread(0);

        IntTransp = ButTransp * Transp;
    } else {
        if (ButTransp < 1.0f) {
            ButTransp += 3.0f*(vw_GetTimeThread(0) - Update);
            if (ButTransp > 1.0f) {
                ButTransp =1.0f;
            }
            Update = vw_GetTimeThread(0);
        }

        IntTransp = ButTransp * Transp;
    }



    // draw shadow
    SrcRect(2,2,512-2,96-2 );
    DstRect(X-64+2,Y-17+2,X-64+512-2,Y-17+96-2);
    constexpr unsigned tmpHash1 = constexpr_hash_djb2a("menu/button384_back.tga");
    vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash1), true, IntTransp);
    // draw button
    SrcRect(0,0,384,64 );
    DstRect(X,Y,X+384,Y+64);
    if (!ON) {
        constexpr unsigned tmpHash2 = constexpr_hash_djb2a("menu/button384_out.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash2), true, Transp);
    } else {
        constexpr unsigned tmpHash3 = constexpr_hash_djb2a("menu/button384_in.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash3), true, Transp);
    }



    int Size = vw_TextWidthUTF32(Text);
    // in case text is too long for this button size, make some scaling
    float WScale = 0;
    if (Size > 310) {
        Size = 310;
        WScale = -310;
    }

    // text start draw position
    int SizeI = X + (SrcRect.right-SrcRect.left-Size)/2;

    // draw text
    if (!ON) {
        vw_DrawTextUTF32(SizeI, Y+21, WScale, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, (0.7f*Transp)/2.0f, Text);
    } else {
        vw_DrawTextUTF32(SizeI, Y+21, WScale, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, Transp, Text);
    }



    if (GetCursorStatus() == eCursorStatus::ActionAllowed && ON
        && (vw_GetMouseLeftClick(true)
            || (InFocusByKeyboard && (vw_GetKeyStatus(SDLK_KP_ENTER) || vw_GetKeyStatus(SDLK_RETURN))))) {
        PlayMenuSFX(eMenuSFX::Click, 1.0f);
        if (InFocusByKeyboard) {
            vw_SetKeyReleased(SDLK_KP_ENTER);
            vw_SetKeyReleased(SDLK_RETURN);
        }
        return true;
    }

    return false;
}










//------------------------------------------------------------------------------------
// 256 button
//------------------------------------------------------------------------------------
bool DrawButton256(int X, int Y, const std::u32string &Text, float Transp, float &ButTransp, float &Update, bool Off)
{
    sRECT SrcRect, DstRect;


    if (Off || DragWeapon) {

        SrcRect(2,2,512-2,96-2 );
        DstRect(X-125+2,Y-16+2,X-125+512-2,Y-16+96-2);
        constexpr unsigned tmpHash1 = constexpr_hash_djb2a("menu/button256_back.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash1), true, Transp);

        SrcRect(0,0,256,64 );
        DstRect(X,Y,X+256,Y+64);
        constexpr unsigned tmpHash2 = constexpr_hash_djb2a("menu/button256_off.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash2), true, Transp);


        int Size = vw_TextWidthUTF32(Text);
        int SizeI = DstRect.left + (SrcRect.right-SrcRect.left-Size)/2;
        vw_DrawTextUTF32(SizeI, Y+21, 0, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, (0.7f*Transp)/2.0f, Text);

        DstRect(X+2,Y+1,X+256,Y+63);
        if (vw_MouseOverRect(DstRect) && !isDialogBoxDrawing() && Transp == 1.0f && !DragWeapon) {
            SetCursorStatus(eCursorStatus::ActionProhibited);
            if (vw_GetMouseLeftClick(true)) {
                PlayMenuSFX(eMenuSFX::CanNotClick, 1.0f);
            }
        }

        return false;
    }



    bool ON = false;
    float IntTransp = Transp;


    // keyboard
    if ((Transp >= 0.99f) && !isDialogBoxDrawing() && GetShowGameCursor()) {
        CurrentActiveMenuElement++;
    }

    bool InFocusByKeyboard = false;
    if (CurrentKeyboardSelectMenuElement > 0
        && CurrentKeyboardSelectMenuElement == CurrentActiveMenuElement) {
        InFocusByKeyboard = true;
    }



    DstRect(X+2,Y+1,X+256,Y+63);
    if ((vw_MouseOverRect(DstRect) || InFocusByKeyboard)
        && !isDialogBoxDrawing() && GetShowGameCursor()) {
        ON = true;
        if (Transp == 1.0f) { // in case showing/hiding menu - button must be not active
            SetCursorStatus(eCursorStatus::ActionAllowed);
        }

        if (ButTransp == 1.0f) {
            PlayMenuSFX(eMenuSFX::OverBigButton, 1.0f);
            Update = vw_GetTimeThread(0);
            ButTransp = 0.98f;
        }

        ButTransp -= 3.0f*(vw_GetTimeThread(0) - Update);
        if (ButTransp < 0.60f) {
            ButTransp = 0.60f;
        }
        Update = vw_GetTimeThread(0);

        IntTransp = ButTransp * Transp;
    } else {
        if (ButTransp < 1.0f) {
            ButTransp += 3.0f*(vw_GetTimeThread(0) - Update);
            if (ButTransp > 1.0f) {
                ButTransp =1.0f;
            }
            Update = vw_GetTimeThread(0);
        }

        IntTransp = ButTransp * Transp;
    }







    // draw shadow
    SrcRect(2,2,512-2,96-2 );
    DstRect(X-125+2,Y-16+2,X-125+512-2,Y-16+96-2);
    constexpr unsigned tmpHash3 = constexpr_hash_djb2a("menu/button256_back.tga");
    vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash3), true, IntTransp);
    // draw button
    SrcRect(0,0,256,64 );
    DstRect(X,Y,X+256,Y+64);
    if (!ON) {
        constexpr unsigned tmpHash4 = constexpr_hash_djb2a("menu/button256_out.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash4), true, Transp);
    } else {
        constexpr unsigned tmpHash5 = constexpr_hash_djb2a("menu/button256_in.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash5), true, Transp);
    }


    int Size = vw_TextWidthUTF32(Text);
    // in case text is too long for this button size, make some scaling
    float WScale = 0;
    if (Size > 190) {
        Size = 190;
        WScale = -190;
    }

    // text start draw position
    int SizeI = DstRect.left + (SrcRect.right-SrcRect.left-Size)/2;
    // draw text
    if (!ON) {
        vw_DrawTextUTF32(SizeI, Y+21, WScale, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, (0.7f*Transp)/2.0f, Text);
    } else {
        vw_DrawTextUTF32(SizeI, Y+21, WScale, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, Transp, Text);
    }

    if (GetCursorStatus() == eCursorStatus::ActionAllowed && ON
        && (vw_GetMouseLeftClick(true)
            || (InFocusByKeyboard && (vw_GetKeyStatus(SDLK_KP_ENTER) || vw_GetKeyStatus(SDLK_RETURN))))) {
        PlayMenuSFX(eMenuSFX::Click, 1.0f);
        if (InFocusByKeyboard) {
            vw_SetKeyReleased(SDLK_KP_ENTER);
            vw_SetKeyReleased(SDLK_RETURN);
        }
        return true;
    }

    return false;
}





//------------------------------------------------------------------------------------
// 200 dialog button
//------------------------------------------------------------------------------------
bool DrawButton200_2(int X, int Y, const std::u32string &Text, float Transp, bool Off)
{
    sRECT SrcRect, DstRect, MouseRect;
    SrcRect(2,2,230-2,64-2);
    DstRect(X-14+2,Y-14+2,X+230-14-2,Y+64-14-2);
    MouseRect(X,Y,X+204,Y+35);

    int Size = vw_TextWidthUTF32(Text);
    // in case text is too long for this button size, make some scaling
    float WScale = 0;
    if (Size > 176) {
        Size = 176;
        WScale = -176;
    }
    int SizeI = DstRect.left + (SrcRect.right-SrcRect.left-Size)/2;


    if (Off || DragWeapon) {
        constexpr unsigned tmpHash1 = constexpr_hash_djb2a("menu/button_dialog200_off.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash1), true, Transp);

        vw_DrawTextUTF32(SizeI, Y+6, WScale, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, (0.7f*Transp)/2.0f, Text);

        DstRect(X,Y,X+204,Y+35);
        if (vw_MouseOverRect(DstRect) && !isDialogBoxDrawing() && Transp == 1.0f && !DragWeapon) {
            SetCursorStatus(eCursorStatus::ActionProhibited);
            if (vw_GetMouseLeftClick(true)) {
                PlayMenuSFX(eMenuSFX::CanNotClick, 1.0f);
            }
        }

        return false;
    }


    bool ON = false;


    // keyboard
    if (Transp >= 0.99f && !isDialogBoxDrawing() && GetShowGameCursor()) {
        CurrentActiveMenuElement++;
    }

    bool InFocusByKeyboard = false;
    if (CurrentKeyboardSelectMenuElement > 0
        && CurrentKeyboardSelectMenuElement == CurrentActiveMenuElement) {
        InFocusByKeyboard = true;
    }


    if ((vw_MouseOverRect(MouseRect) || InFocusByKeyboard)
        && !isDialogBoxDrawing() && GetShowGameCursor()) {
        ON = true;
        if (Transp == 1.0f) { // in case showing/hiding menu - button must be not active
            SetCursorStatus(eCursorStatus::ActionAllowed);
        }

        if (NeedPlayOnButtonSoundX != X || NeedPlayOnButtonSoundY != Y) {
            PlayMenuSFX(eMenuSFX::OverSmallButton, 1.0f);
            NeedPlayOnButtonSoundX = X;
            NeedPlayOnButtonSoundY = Y;
        }
    } else {
        if (NeedPlayOnButtonSoundX == X && NeedPlayOnButtonSoundY == Y) {
            NeedPlayOnButtonSoundX = 0;
            NeedPlayOnButtonSoundY = 0;
        }
    }


    if (!ON) {
        constexpr unsigned tmpHash2 = constexpr_hash_djb2a("menu/button_dialog200_out.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash2), true, Transp);
    } else {
        constexpr unsigned tmpHash3 = constexpr_hash_djb2a("menu/button_dialog200_in.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash3), true, Transp);
    }

    // draw text
    if (!ON) {
        vw_DrawTextUTF32(SizeI, Y+6, WScale, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, (0.7f*Transp)/2.0f, Text);
    } else {
        vw_DrawTextUTF32(SizeI, Y+6, WScale, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, Transp, Text);
    }

    if (GetCursorStatus() == eCursorStatus::ActionAllowed && ON
        && (vw_GetMouseLeftClick(true)
            || (InFocusByKeyboard && (vw_GetKeyStatus(SDLK_KP_ENTER) || vw_GetKeyStatus(SDLK_RETURN))))) {
        PlayMenuSFX(eMenuSFX::Click, 1.0f);
        if (InFocusByKeyboard) {
            vw_SetKeyReleased(SDLK_KP_ENTER);
            vw_SetKeyReleased(SDLK_RETURN);
        }
        return true;
    }

    return false;
}


//------------------------------------------------------------------------------------
// 128 dialog button
//------------------------------------------------------------------------------------
bool DrawButton128_2(int X, int Y, const std::u32string &Text, float Transp, bool Off, bool SoundClick)
{
    sRECT SrcRect, DstRect, MouseRect;
    SrcRect(2,2,158-2,64-2);
    DstRect(X-14+2,Y-14+2,X+158-14-2,Y+64-14-2);
    MouseRect(X,Y,X+132,Y+35);

    int Size = vw_TextWidthUTF32(Text);
    // in case text is too long for this button size, make some scaling
    float WScale = 0;
    if (Size > 108) {
        Size = 108;
        WScale = -108;
    }

    // text start draw position
    int SizeI = DstRect.left + (SrcRect.right-SrcRect.left-Size)/2;


    if (Off || DragWeapon) {
        constexpr unsigned tmpHash1 = constexpr_hash_djb2a("menu/button_dialog128_off.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash1), true, Transp);

        vw_DrawTextUTF32(SizeI, Y+6, WScale, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, (0.7f*Transp)/2.0f, Text);

        DstRect(X,Y,X+132,Y+35);
        if (vw_MouseOverRect(DstRect) && !isDialogBoxDrawing() && Transp == 1.0f && !DragWeapon) {
            SetCursorStatus(eCursorStatus::ActionProhibited);
            if (vw_GetMouseLeftClick(true)) {
                PlayMenuSFX(eMenuSFX::CanNotClick, 1.0f);
            }
        }

        return false;
    }



    bool ON = false;


    // keyboard
    if (Transp >= 0.99f && !isDialogBoxDrawing() && GetShowGameCursor()) {
        CurrentActiveMenuElement++;
    }

    bool InFocusByKeyboard = false;
    if (CurrentKeyboardSelectMenuElement > 0
        && CurrentKeyboardSelectMenuElement == CurrentActiveMenuElement) {
        InFocusByKeyboard = true;
    }


    if ((vw_MouseOverRect(MouseRect) || InFocusByKeyboard)
        && !isDialogBoxDrawing() && GetShowGameCursor()) {
        ON = true;
        if (Transp == 1.0f) { // in case showing/hiding menu - button must be not active
            SetCursorStatus(eCursorStatus::ActionAllowed);
        }

        if (NeedPlayOnButtonSoundX != X || NeedPlayOnButtonSoundY != Y) {
            PlayMenuSFX(eMenuSFX::OverSmallButton, 1.0f);
            NeedPlayOnButtonSoundX = X;
            NeedPlayOnButtonSoundY = Y;
        }
    } else {
        if (NeedPlayOnButtonSoundX == X && NeedPlayOnButtonSoundY == Y) {
            NeedPlayOnButtonSoundX = 0;
            NeedPlayOnButtonSoundY = 0;
        }
    }


    if (!ON) {
        constexpr unsigned tmpHash2 = constexpr_hash_djb2a("menu/button_dialog128_out.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash2), true, Transp);
    } else {
        constexpr unsigned tmpHash3 = constexpr_hash_djb2a("menu/button_dialog128_in.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash3), true, Transp);
    }

    // draw text
    if (!ON) {
        vw_DrawTextUTF32(SizeI, Y+6, WScale, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, (0.7f*Transp)/2.0f, Text);
    } else {
        vw_DrawTextUTF32(SizeI, Y+6, WScale, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, Transp, Text);
    }

    if (GetCursorStatus() == eCursorStatus::ActionAllowed && ON
        && (vw_GetMouseLeftClick(true)
            || (InFocusByKeyboard && (vw_GetKeyStatus(SDLK_KP_ENTER) || vw_GetKeyStatus(SDLK_RETURN))))) {
        if (SoundClick) {
            PlayMenuSFX(eMenuSFX::Click, 1.0f);
        }
        if (InFocusByKeyboard) {
            vw_SetKeyReleased(SDLK_KP_ENTER);
            vw_SetKeyReleased(SDLK_RETURN);
        }
        return true;
    }

    return false;
}










//------------------------------------------------------------------------------------
// checkbox
//------------------------------------------------------------------------------------
void DrawCheckBox(int X, int Y, bool &CheckBoxStatus, const std::u32string &Text, float Transp)
{
    sRECT SrcRect, DstRect;

    int Size = vw_TextWidthUTF32(Text);

    bool ON = false;


    // keyboard
    if (Transp >= 0.99f && !isDialogBoxDrawing() && GetShowGameCursor()) {
        CurrentActiveMenuElement++;
    }

    bool InFocusByKeyboard = false;
    if (CurrentKeyboardSelectMenuElement > 0
        && CurrentKeyboardSelectMenuElement == CurrentActiveMenuElement) {
        InFocusByKeyboard = true;
    }


    // 20 - spacing between text
    DstRect(X+4,Y+4,X+40+20+Size,Y+40-4);
    if ((vw_MouseOverRect(DstRect) || InFocusByKeyboard)
        && !isDialogBoxDrawing() && GetShowGameCursor()) {
        ON = true;
        if (Transp == 1.0f) { // in case showing/hiding menu - button must be not active
            SetCursorStatus(eCursorStatus::ActionAllowed);
        }
    }



    SrcRect(0,0,40,38);
    DstRect(X,Y,X+40,Y+38);
    if (!ON || DragWeapon) {
        vw_DrawTextUTF32(X+40+20, Y+8, 0, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, Transp, Text);
    } else {
        vw_DrawTextUTF32(X+40+20, Y+8, 0, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::orange}, Transp, Text);
    }

    constexpr unsigned tmpHash1 = constexpr_hash_djb2a("menu/checkbox_main.tga");
    vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash1), true, Transp);
    if (CheckBoxStatus) {
        constexpr unsigned tmpHash2 = constexpr_hash_djb2a("menu/checkbox_in.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash2), true, Transp);
    }


    if (GetCursorStatus() == eCursorStatus::ActionAllowed && ON
        && !DragWeapon
        && (vw_GetMouseLeftClick(true)
            || (InFocusByKeyboard && (vw_GetKeyStatus(SDLK_KP_ENTER) || vw_GetKeyStatus(SDLK_RETURN))))) {
        CheckBoxStatus = !CheckBoxStatus;
        PlayMenuSFX(eMenuSFX::Click, 1.0f);
        if (InFocusByKeyboard) {
            vw_SetKeyReleased(SDLK_KP_ENTER);
            vw_SetKeyReleased(SDLK_RETURN);
        }
    }
}





//------------------------------------------------------------------------------------
// up button for list
//------------------------------------------------------------------------------------
bool DrawListUpButton(int X, int Y, float Transp, bool Off)
{
    sRECT SrcRect, DstRect, MouseRect;
    SrcRect(0,0,32,32);
    DstRect(X,Y,X+32,Y+32);
    MouseRect(X,Y,X+32,Y+32);


    if (Off || DragWeapon) {
        DstRect(X+2,Y+2,X+32-2,Y+32-2);
        constexpr unsigned tmpHash1 = constexpr_hash_djb2a("menu/arrow_list_up.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash1), true, 0.3f*Transp);

        if  (vw_MouseOverRect(MouseRect) && !isDialogBoxDrawing() && Transp == 1.0f && !DragWeapon) {
            SetCursorStatus(eCursorStatus::ActionProhibited);
            if (vw_GetMouseLeftClick(true)) {
                PlayMenuSFX(eMenuSFX::CanNotClick, 1.0f);
            }
        }

        return false;
    }



    bool ON = false;


    // keyboard
    if (Transp >= 0.99f && !isDialogBoxDrawing() && GetShowGameCursor()) {
        CurrentActiveMenuElement++;
    }

    bool InFocusByKeyboard = false;
    if (CurrentKeyboardSelectMenuElement > 0
        && CurrentKeyboardSelectMenuElement == CurrentActiveMenuElement) {
        InFocusByKeyboard = true;
    }


    if ((vw_MouseOverRect(MouseRect) || InFocusByKeyboard)
        && !isDialogBoxDrawing() && GetShowGameCursor()) {
        ON = true;
        if (Transp == 1.0f) { // in case showing/hiding menu - button must be not active
            SetCursorStatus(eCursorStatus::ActionAllowed);
        }

        if (NeedPlayOnButtonSoundX != X || NeedPlayOnButtonSoundY != Y) {
            PlayMenuSFX(eMenuSFX::OverSmallButton, 1.0f);
            NeedPlayOnButtonSoundX = X;
            NeedPlayOnButtonSoundY = Y;
        }
    } else {
        if (NeedPlayOnButtonSoundX == X && NeedPlayOnButtonSoundY == Y) {
            NeedPlayOnButtonSoundX = 0;
            NeedPlayOnButtonSoundY = 0;
        }
    }


    if (!ON) {
        DstRect(X+2,Y+2,X+32-2,Y+32-2);
        constexpr unsigned tmpHash2 = constexpr_hash_djb2a("menu/arrow_list_up.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash2), true, 0.3f*Transp);
    } else {
        constexpr unsigned tmpHash3 = constexpr_hash_djb2a("menu/arrow_list_up.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash3), true, Transp);
    }


    if (GetCursorStatus() == eCursorStatus::ActionAllowed && ON
        && (vw_GetMouseLeftClick(true)
            || (InFocusByKeyboard && (vw_GetKeyStatus(SDLK_KP_ENTER) || vw_GetKeyStatus(SDLK_RETURN))))) {
        PlayMenuSFX(eMenuSFX::Click, 1.0f);
        if (InFocusByKeyboard) {
            vw_SetKeyReleased(SDLK_KP_ENTER);
            vw_SetKeyReleased(SDLK_RETURN);
        }
        return true;
    }

    return false;
}



//------------------------------------------------------------------------------------
// down button for list
//------------------------------------------------------------------------------------
bool DrawListDownButton(int X, int Y, float Transp, bool Off)
{
    sRECT SrcRect, DstRect, MouseRect;
    SrcRect(0,0,32,32);
    DstRect(X,Y,X+32,Y+32);
    MouseRect(X,Y,X+32,Y+32);


    if (Off || DragWeapon) {
        DstRect(X+2,Y+2,X+32-2,Y+32-2);
        constexpr unsigned tmpHash1 = constexpr_hash_djb2a("menu/arrow_list_down.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash1), true, 0.3f*Transp);

        if (vw_MouseOverRect(MouseRect) && !isDialogBoxDrawing() && Transp == 1.0f && !DragWeapon) {
            SetCursorStatus(eCursorStatus::ActionProhibited);
            if (vw_GetMouseLeftClick(true)) {
                PlayMenuSFX(eMenuSFX::CanNotClick, 1.0f);
            }
        }

        return false;
    }



    bool ON = false;


    // keyboard
    if ((Transp >= 0.99f) && !isDialogBoxDrawing() && GetShowGameCursor()) {
        CurrentActiveMenuElement++;
    }

    bool InFocusByKeyboard = false;
    if (CurrentKeyboardSelectMenuElement > 0
        && CurrentKeyboardSelectMenuElement == CurrentActiveMenuElement) {
        InFocusByKeyboard = true;
    }


    if ((vw_MouseOverRect(MouseRect) || InFocusByKeyboard)
        && !isDialogBoxDrawing() && GetShowGameCursor()) {
        ON = true;
        if (Transp == 1.0f) { // in case showing/hiding menu - button must be not active
            SetCursorStatus(eCursorStatus::ActionAllowed);
        }

        if (NeedPlayOnButtonSoundX != X || NeedPlayOnButtonSoundY != Y) {
            PlayMenuSFX(eMenuSFX::OverSmallButton, 1.0f);
            NeedPlayOnButtonSoundX = X;
            NeedPlayOnButtonSoundY = Y;
        }
    } else {
        if (NeedPlayOnButtonSoundX == X && NeedPlayOnButtonSoundY == Y) {
            NeedPlayOnButtonSoundX = 0;
            NeedPlayOnButtonSoundY = 0;
        }
    }


    if (!ON) {
        DstRect(X+2,Y+2,X+32-2,Y+32-2);
        constexpr unsigned tmpHash2 = constexpr_hash_djb2a("menu/arrow_list_down.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash2), true, 0.3f*Transp);
    } else {
        constexpr unsigned tmpHash3 = constexpr_hash_djb2a("menu/arrow_list_down.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash3), true, Transp);
    }


    if (GetCursorStatus() == eCursorStatus::ActionAllowed && ON
        && (vw_GetMouseLeftClick(true)
            || (InFocusByKeyboard && (vw_GetKeyStatus(SDLK_KP_ENTER) || vw_GetKeyStatus(SDLK_RETURN))))) {
        PlayMenuSFX(eMenuSFX::Click, 1.0f);
        if (InFocusByKeyboard) {
            vw_SetKeyReleased(SDLK_KP_ENTER);
            vw_SetKeyReleased(SDLK_RETURN);
        }
        return true;
    }

    return false;
}

} // astromenace namespace
} // viewizard namespace
