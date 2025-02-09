/****************************************************************************

    AstroMenace
    Hardcore 3D space scroll-shooter with spaceship upgrade possibilities.
    Copyright (C) 2006-2025 Mikhail Kurinnoi, Viewizard


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

// TODO NeedCheck in CheckMouseKeysJoystickState() should be moved to enumeration

#include "../core/core.h"
#include "../enum.h"
#include "../config/config.h"
#include "../platform/platform.h"
#include "../assets/audio.h"
#include "../assets/texture.h"
#include "../ui/cursor.h"
#include "../command.h"
#include "../game.h" // FIXME "game.h" should be replaced by individual headers
#include "SDL2/SDL.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

int ButQuant = 10;
float But[10] =
{1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f};

int NeedCheck = 0;
float LastTimeBut = 0.0f;



static bool ReservedKeys(int Key)
{
    switch (Key) {
    case SDLK_ESCAPE:
        NeedCheck = 0;
        // don't remove this code duplication for now, don't use "[[fallthrough]];" (since C++17)
        // or "[[gnu::fallthrough]];" here for supress GCC7 warnings, since this produce another
        // warnings on old compiller versions
        vw_SetKeyReleased(Key);
        return true;
    case SDLK_F1:
    case SDLK_F2:
    case SDLK_F3:
    case SDLK_F4:
    case SDLK_F5:
    case SDLK_F6:
    case SDLK_F7:
    case SDLK_F8:
    case SDLK_F9:
    case SDLK_F10:
    case SDLK_F11:
    case SDLK_F12:
    case SDLK_F13:
    case SDLK_F14:
    case SDLK_F15:
    case SDLK_F16:
    case SDLK_F17:
    case SDLK_F18:
    case SDLK_F19:
    case SDLK_F20:
    case SDLK_F21:
    case SDLK_F22:
    case SDLK_F23:
    case SDLK_F24:
    case SDLK_PRINTSCREEN:
        vw_SetKeyReleased(Key);
        return true;
    }

    return false;
}


// check mouse/keys/joystick during control configuration/weapon slot setup
void CheckMouseKeysJoystickState()
{
    if (!isDialogBoxDrawing() && NeedCheck > 0) {
        // keyboard
        if ((NeedCheck > 0 && NeedCheck <= 6) || NeedCheck == 100) {
            int KeyStateArraySize = 0;
            SDL_GetKeyboardState(&KeyStateArraySize);
            for (int k = 0; k < KeyStateArraySize; k++) {
                int i = SDL_GetKeyFromScancode((SDL_Scancode)k);
                if (vw_GetKeyStatus(i) && !ReservedKeys(i) && SDL_GetKeyName(i)) {
                    switch (NeedCheck) {
                    case 1:
                        ChangeGameConfig().KeyBoardUp = i;
                        break;
                    case 2:
                        ChangeGameConfig().KeyBoardDown = i;
                        break;
                    case 3:
                        ChangeGameConfig().KeyBoardLeft = i;
                         break;
                    case 4:
                        ChangeGameConfig().KeyBoardRight = i;
                        break;
                    case 5:
                        ChangeGameConfig().KeyBoardPrimary = i;
                        break;
                    case 6:
                        ChangeGameConfig().KeyBoardSecondary = i;
                        break;
                    case 100:
                        NewWeaponControlType = 1;
                        NewWeaponControlTypeData = i;
                        break;
                    }

                    vw_SetKeyReleased(i);
                    NeedCheck = 0;
                }
            }
        }


        // mouse
        if ((NeedCheck >= 7 && NeedCheck <= 8) || NeedCheck == 100) {
            // note, SDL provide button's number that starts from 1
            for (unsigned i = 1; i <= vw_GetMaxMouseButtonNum(); i++) {
                if (vw_GetMouseButtonStatus(i)) {
                    switch (NeedCheck) {
                    case 7:
                        ChangeGameConfig().MousePrimary = i;
                        break;
                    case 8:
                        ChangeGameConfig().MouseSecondary = i;
                        break;
                    case 100:
                        NewWeaponControlType = 2;
                        NewWeaponControlTypeData = i;
                        break;
                    }
                    NeedCheck = 0;
                    vw_GetMouseLeftClick(true);
                }
            }
        }


        // joystick
        if (isJoystickAvailable()) {
            if ((NeedCheck >= 9 && NeedCheck <= 10) || NeedCheck == 100) {
                for (int i = 0; i < GetJoystickButtonsQuantity(); i++) {
                    if (GetJoystickButton(i)) {
                        switch (NeedCheck) {
                        case 9:
                            ChangeGameConfig().JoystickPrimary = i;
                            break;
                        case 10:
                            ChangeGameConfig().JoystickSecondary = i;
                            break;
                        case 100:
                            NewWeaponControlType = 3;
                            NewWeaponControlTypeData = i;
                            break;
                        }
                        NeedCheck = 0;
                        SetJoystickButton(i, false);
                        vw_GetMouseLeftClick(true);
                    }
                }
            }
        }


        // in case Esc button - prevent future checks
        if (vw_GetKeyStatus(SDLK_ESCAPE)) {
            vw_SetKeyReleased(SDLK_ESCAPE);
            NeedCheck = 0;
        }
    }

    // button blinking (button that used for custom control configuration for current check)
    float Delta = vw_GetTimeThread(0) - LastTimeBut;
    for (int i=0; i<ButQuant; i++) {
        if (But[i] > 0.3f) {
            But[i] -= 2.0f*Delta;
        }
        if (But[i] < 0.3f) {
            But[i] = 1.0f;
        }
    }
    LastTimeBut = vw_GetTimeThread(0);
}



void CheckKeysBeforeExit()
{
    if (!GameConfig().KeyBoardUp) {
        ChangeGameConfig().KeyBoardUp = SDLK_UP;
    }
    if (!GameConfig().KeyBoardDown) {
        ChangeGameConfig().KeyBoardDown = SDLK_DOWN;
    }
    if (!GameConfig().KeyBoardLeft) {
        ChangeGameConfig().KeyBoardLeft = SDLK_LEFT;
    }
    if (!GameConfig().KeyBoardRight) {
        ChangeGameConfig().KeyBoardRight = SDLK_RIGHT;
    }
    if (!GameConfig().KeyBoardPrimary) {
        ChangeGameConfig().KeyBoardPrimary = SDLK_LCTRL;
    }
    if (!GameConfig().KeyBoardSecondary) {
        ChangeGameConfig().KeyBoardSecondary = SDLK_SPACE;
    }
    if (!GameConfig().MousePrimary) {
        ChangeGameConfig().MousePrimary = SDL_BUTTON_LEFT;
    }
    if (!GameConfig().MouseSecondary) {
        ChangeGameConfig().MouseSecondary = SDL_BUTTON_RIGHT;
    }
    NeedCheck = 0;
    for (int i = 0; i < ButQuant; i++) {
        But[i] = 1.0f;
    }
}




void ConfControlMenu(float ContentTransp, float &ButtonTransp1, float &LastButtonUpdateTime1)
{
    // check mouse/keys/joystick during control configuration setup if need
    CheckMouseKeysJoystickState();


    sRECT SrcRect, DstRect;
    SrcRect(0, 0, 2, 2);
    DstRect(0, 0, GameConfig().InternalWidth, 768);
    constexpr unsigned tmpHash1 = constexpr_hash_djb2a("menu/blackpoint.tga");
    vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash1), true, 0.5f * ContentTransp);







    int X1 = GameConfig().InternalWidth / 2 - 375;
    int Y1 = 65;
    int Prir1 = 55;





    vw_DrawTextUTF32(X1, Y1, -280, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, ContentTransp, vw_GetTextUTF32("Joystick DeadZone"));
    if (DrawButton128_2(X1+300, Y1-6, vw_GetTextUTF32("Decrease"), ContentTransp, GameConfig().JoystickDeadZone == 0)) {
        ChangeGameConfig().JoystickDeadZone--;
        if (GameConfig().JoystickDeadZone < 0) {
            ChangeGameConfig().JoystickDeadZone = 0;
        }
    }
    if (DrawButton128_2(X1+616, Y1-6, vw_GetTextUTF32("Increase"), ContentTransp, GameConfig().JoystickDeadZone == 10)) {
        ChangeGameConfig().JoystickDeadZone++;
        if (GameConfig().JoystickDeadZone > 10) {
            ChangeGameConfig().JoystickDeadZone = 10;
        }
    }
    for (int i=0; i<10; i++) {
        SrcRect(0,0,16,32);
        DstRect(X1+443+16*i,Y1-4,X1+443+16+16*i,Y1+32-4);
        if (vw_MouseOverRect(DstRect)) {
            SetCursorStatus(eCursorStatus::ActionAllowed);
            if (vw_GetMouseLeftClick(true)) {
                ChangeGameConfig().JoystickDeadZone = i + 1;
                PlayMenuSFX(eMenuSFX::Click, 1.0f);
            }
        }
        if (GameConfig().JoystickDeadZone > i) {
            constexpr unsigned tmpHash2 = constexpr_hash_djb2a("menu/perc.tga");
            vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash2), true, ContentTransp);
        } else {
            constexpr unsigned tmpHash3 = constexpr_hash_djb2a("menu/perc_none.tga");
            vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash3), true, ContentTransp);
        }
    }





    Y1 += Prir1;
    vw_DrawTextUTF32(X1, Y1, -280, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, ContentTransp, vw_GetTextUTF32("Control Sensitivity"));
    if (DrawButton128_2(X1+300, Y1-6, vw_GetTextUTF32("Decrease"), ContentTransp, GameConfig().ControlSensivity == 1)) {
        ChangeGameConfig().ControlSensivity--;
        if (GameConfig().ControlSensivity < 1) {
            ChangeGameConfig().ControlSensivity = 1;
        }
    }
    if (DrawButton128_2(X1+616, Y1-6, vw_GetTextUTF32("Increase"), ContentTransp, GameConfig().ControlSensivity == 10)) {
        ChangeGameConfig().ControlSensivity++;
        if (GameConfig().ControlSensivity > 10) {
            ChangeGameConfig().ControlSensivity = 10;
        }
    }
    for (int i=0; i<10; i++) {
        SrcRect(0,0,16,32);
        DstRect(X1+443+16*i,Y1-4,X1+443+16+16*i,Y1+32-4);
        if (vw_MouseOverRect(DstRect)) {
            SetCursorStatus(eCursorStatus::ActionAllowed);
            if (vw_GetMouseLeftClick(true)) {
                ChangeGameConfig().ControlSensivity = i + 1;
                PlayMenuSFX(eMenuSFX::Click, 1.0f);
            }
        }
        if (GameConfig().ControlSensivity > i) {
            constexpr unsigned tmpHash4 = constexpr_hash_djb2a("menu/perc.tga");
            vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash4), true, ContentTransp);
        } else {
            constexpr unsigned tmpHash5 = constexpr_hash_djb2a("menu/perc_none.tga");
            vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash5), true, ContentTransp);
        }
    }













    Y1 += Prir1;
    int SizeI1 = vw_TextWidthUTF32(vw_GetTextUTF32("MOUSE"));
    int SizeI = SizeI1 > 130 ? -15 : (100-SizeI1)/2;
    vw_DrawTextUTF32(X1+315+SizeI, Y1, SizeI1 > 130 ? -130 : 0, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, ContentTransp, vw_GetTextUTF32("MOUSE"));
    SizeI1 = vw_TextWidthUTF32(vw_GetTextUTF32("KEYBOARD"));
    SizeI = SizeI1 > 130 ? 10 : (150-SizeI1)/2;
    vw_DrawTextUTF32(X1+446+SizeI, Y1, SizeI1 > 130 ? -130 : 0, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, ContentTransp, vw_GetTextUTF32("KEYBOARD"));
    SizeI1 = vw_TextWidthUTF32(vw_GetTextUTF32("JOYSTICK"));
    SizeI = SizeI1 > 130 ? 10 : (150-SizeI1)/2;
    vw_DrawTextUTF32(X1+605+SizeI, Y1, SizeI1 > 130 ? -130 : 0, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, ContentTransp, vw_GetTextUTF32("JOYSTICK"));


    Y1 += Prir1;
    vw_DrawTextUTF32(X1, Y1, -280, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, ContentTransp, vw_GetTextUTF32("Primary Attack"));
    float Transp = 1.0f;
    bool Off = false;
    std::string ButtonName = MouseButtonName(GameConfig().MousePrimary);
    if (NeedCheck == 7) {
        Transp = But[6];
        Off = true;
        ButtonName = "?";
    };
    if (DrawButton128_2(X1+300, Y1-6, ConvertUTF8.from_bytes(ButtonName), Transp*ContentTransp, Off)) {
        if (NeedCheck == 0) {
            NeedCheck = 7;
            vw_ResetMouseButtons();
        }
    }

    Transp = 1.0f;
    Off = false;
    ButtonName = SDL_GetKeyName(GameConfig().KeyBoardPrimary);
    if (NeedCheck == 5) {
        Transp = But[4];
        Off = true;
        ButtonName = "?";
    };
    if (DrawButton128_2(X1+458, Y1-6, ConvertUTF8.from_bytes(ButtonName), Transp*ContentTransp, Off)) {
        if (NeedCheck == 0) {
            NeedCheck = 5;
        }
    }

    Transp = 1.0f;
    Off = false;
    ButtonName = JoystickButtonName(GameConfig().JoystickPrimary);
    if (NeedCheck == 9) {
        Transp = But[8];
        Off = true;
        ButtonName = "?";
    };
    if (DrawButton128_2(X1+616, Y1-6, ConvertUTF8.from_bytes(ButtonName), Transp*ContentTransp, !isJoystickAvailable() || Off)) {
        if (NeedCheck == 0) {
            NeedCheck = 9;
            for (int i = 0; i < GetJoystickButtonsQuantity(); i++) {
                SetJoystickButton(i, false);
            }
        }
    }



    Y1 += Prir1;
    vw_DrawTextUTF32(X1, Y1, -280, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, ContentTransp, vw_GetTextUTF32("Secondary Attack"));
    Transp = 1.0f;
    Off = false;
    ButtonName = MouseButtonName(GameConfig().MouseSecondary);
    if (NeedCheck == 8) {
        Transp = But[7];
        Off = true;
        ButtonName = "?";
    };
    if (DrawButton128_2(X1+300, Y1-6, ConvertUTF8.from_bytes(ButtonName), Transp*ContentTransp, Off)) {
        if (NeedCheck == 0) {
            NeedCheck = 8;
            vw_ResetMouseButtons();
        }
    }

    Transp = 1.0f;
    Off = false;
    ButtonName = SDL_GetKeyName(GameConfig().KeyBoardSecondary);
    if (NeedCheck == 6) {
        Transp = But[5];
        Off = true;
        ButtonName = "?";
    };
    if (DrawButton128_2(X1+458, Y1-6, ConvertUTF8.from_bytes(ButtonName), Transp*ContentTransp, Off)) {
        if (NeedCheck == 0) {
            NeedCheck = 6;
        }
    }

    Transp = 1.0f;
    Off = false;
    ButtonName = JoystickButtonName(GameConfig().JoystickSecondary);
    if (NeedCheck == 10) {
        Transp = But[9];
        Off = true;
        ButtonName = "?";
    };
    if (DrawButton128_2(X1+616, Y1-6, ConvertUTF8.from_bytes(ButtonName), Transp*ContentTransp, !isJoystickAvailable() || Off)) {
        if (NeedCheck == 0) {
            NeedCheck = 10;
            for (int i = 0; i < GetJoystickButtonsQuantity(); i++) {
                SetJoystickButton(i, false);
            }
        }
    }




    Y1 += Prir1;
    vw_DrawTextUTF32(X1, Y1, -280, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, ContentTransp, vw_GetTextUTF32("Move Forward"));
    Transp = 1.0f;
    Off = false;
    ButtonName = SDL_GetKeyName(GameConfig().KeyBoardUp);
    if (NeedCheck == 1) {
        Transp = But[0];
        Off = true;
        ButtonName = "?";
    };
    if (DrawButton128_2(X1+458, Y1-6, ConvertUTF8.from_bytes(ButtonName), Transp*ContentTransp, Off)) {
        if (NeedCheck == 0) {
            NeedCheck = 1;
        }
    }

    Y1 += Prir1;
    vw_DrawTextUTF32(X1, Y1, -280, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, ContentTransp, vw_GetTextUTF32("Move Backward"));
    Transp = 1.0f;
    Off = false;
    ButtonName = SDL_GetKeyName(GameConfig().KeyBoardDown);
    if (NeedCheck == 2) {
        Transp = But[1];
        Off = true;
        ButtonName = "?";
    };
    if (DrawButton128_2(X1+458, Y1-6, ConvertUTF8.from_bytes(ButtonName), Transp*ContentTransp, Off)) {
        if (NeedCheck == 0) {
            NeedCheck = 2;
        }
    }

    Y1 += Prir1;
    vw_DrawTextUTF32(X1, Y1, -280, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, ContentTransp, vw_GetTextUTF32("Move Left"));
    Transp = 1.0f;
    Off = false;
    ButtonName = SDL_GetKeyName(GameConfig().KeyBoardLeft);
    if (NeedCheck == 3) {
        Transp = But[2];
        Off = true;
        ButtonName = "?";
    };
    if (DrawButton128_2(X1+458, Y1-6, ConvertUTF8.from_bytes(ButtonName), Transp*ContentTransp, Off)) {
        if (NeedCheck == 0) {
            NeedCheck = 3;
        }
    }

    Y1 += Prir1;
    vw_DrawTextUTF32(X1, Y1, -280, 0,1.0f, sRGBCOLOR{eRGBCOLOR::white}, ContentTransp, vw_GetTextUTF32("Move Right"));
    Transp = 1.0f;
    Off = false;
    ButtonName = SDL_GetKeyName(GameConfig().KeyBoardRight);
    if (NeedCheck == 4) {
        Transp = But[3];
        Off = true;
        ButtonName = "?";
    };
    if (DrawButton128_2(X1+458, Y1-6, ConvertUTF8.from_bytes(ButtonName), Transp*ContentTransp, Off)) {
        if (NeedCheck == 0) {
            NeedCheck = 4;
        }
    }







    int Prir = 100;
    int X;
    int Y = 165+Prir*4;


    X = GameConfig().InternalWidth / 2 - 366;
    if (DrawButton200_2(X,Y+28, vw_GetTextUTF32("Advanced"), ContentTransp, false)) {
        if (MenuStatus == eMenuStatus::GAME) {
            SetOptionsMenu(eMenuStatus::OPTIONS_ADVANCED);
            GameMenuStatus = eGameMenuStatus::OPTIONS_ADVANCED;
        } else {
            cCommand::GetInstance().Set(eCommand::SWITCH_TO_OPTIONS_ADVANCED);
        }
        CheckKeysBeforeExit();
    }


    X = GameConfig().InternalWidth / 2 - 100;
    if (DrawButton200_2(X,Y+28, vw_GetTextUTF32("Video & Audio"), ContentTransp, false)) {
        if (MenuStatus == eMenuStatus::GAME) {
            SetOptionsMenu(eMenuStatus::OPTIONS);
            GameMenuStatus = eGameMenuStatus::OPTIONS;
        } else {
            cCommand::GetInstance().Set(eCommand::SWITCH_TO_OPTIONS);
        }
        CheckKeysBeforeExit();
    }


    X = GameConfig().InternalWidth / 2 + 166;
    if (DrawButton200_2(X,Y+28, vw_GetTextUTF32("Interface"), ContentTransp, false)) {
        if (MenuStatus == eMenuStatus::GAME) {
            SetOptionsMenu(eMenuStatus::INTERFACE);
            GameMenuStatus = eGameMenuStatus::INTERFACE;
        } else {
            cCommand::GetInstance().Set(eCommand::SWITCH_TO_INTERFACE);
        }
        CheckKeysBeforeExit();
    }




    X = (GameConfig().InternalWidth - 384) / 2;
    Y = Y+Prir;
    if (MenuStatus == eMenuStatus::GAME) {
        if (DrawButton384(X,Y, vw_GetTextUTF32("GAME MENU"), ContentTransp, ButtonTransp1, LastButtonUpdateTime1)) {
            GameMenuStatus = eGameMenuStatus::GAME_MENU;
            CheckKeysBeforeExit();
        }
    } else {
        if (DrawButton384(X,Y, vw_GetTextUTF32("MAIN MENU"), ContentTransp, ButtonTransp1, LastButtonUpdateTime1)) {
            cCommand::GetInstance().Set(eCommand::SWITCH_TO_MAIN_MENU);
            CheckKeysBeforeExit();
        }
    }

}

} // astromenace namespace
} // viewizard namespace
