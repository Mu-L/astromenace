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

// NOTE in future, use make_unique() to make unique_ptr-s (since C++14)

#include "../core/core.h"
#include "../build_config.h"
#include "../config/config.h"
#include "../ui/font.h"
#include "../ui/cursor.h"
#include "../assets/audio.h"
#include "../assets/texture.h"
#include "../script/script.h"
#include "../object3d/object3d.h"
#include "../gfx/star_system.h"
#include "../gfx/shadow_map.h"
#include "../command.h"
#include "../game/camera.h"
#include "../game.h" // FIXME "game.h" should be replaced by individual headers
#include "SDL2/SDL.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

namespace {

std::unique_ptr<cMissionScript> MenuScript{};

} // unnamed namespace

float LastMenuUpdateTime = 0.0f;
float MenuContentTransp = 0.0f;
float LastMenuOnOffUpdateTime = 0.0f;
float MenuBlackTransp = 0.0f;
bool NeedOnMenu = false; // in case we need show menu (from previous black fade)
bool NeedOffMenu = false;
bool NeedShowMenu = false;
bool NeedHideMenu = false;
eMenuStatus MenuStatus;
eMenuStatus NextMenu;
eMenuStatus PrevMenu{eMenuStatus::MAIN_MENU};

float Button1Transp = 1.0f;
float LastButton1UpdateTime = 0.0f;
float Button2Transp = 1.0f;
float LastButton2UpdateTime = 0.0f;
float Button3Transp = 1.0f;
float LastButton3UpdateTime = 0.0f;
float Button4Transp = 1.0f;
float LastButton4UpdateTime = 0.0f;
float Button5Transp = 1.0f;
float LastButton5UpdateTime = 0.0f;
float Button6Transp = 1.0f;
float LastButton6UpdateTime = 0.0f;
float Button7Transp = 1.0f;
float LastButton7UpdateTime = 0.0f;
float Button8Transp = 1.0f;
float LastButton8UpdateTime = 0.0f;
float Button9Transp = 1.0f;
float LastButton9UpdateTime = 0.0f;
float Button10Transp = 1.0f;
float LastButton10UpdateTime = 0.0f;
float Button11Transp = 1.0f;
float LastButton11UpdateTime = 0.0f;
float Button12Transp = 1.0f;
float LastButton12UpdateTime = 0.0f;
float Button13Transp = 1.0f;
float LastButton13UpdateTime = 0.0f;
float Button14Transp = 1.0f;
float LastButton14UpdateTime = 0.0f;







//------------------------------------------------------------------------------------
// menu initialization
//------------------------------------------------------------------------------------
void InitMenu(eMenuStatus NewMenuStatus)
{
    MenuStatus = NewMenuStatus;

    ShadowMap_SizeSetup(eShadowMapSetup::Menu);

    // set mouse on center position
    float tmpViewportWidth, tmpViewportHeight;
    vw_GetViewport(nullptr, nullptr, &tmpViewportWidth, &tmpViewportHeight);
    SDL_WarpMouseInWindow(reinterpret_cast<SDL_Window*>(vw_GetSDLWindow()),
                          static_cast<int>((512.0f + 256.0f) / (GameConfig().InternalWidth / tmpViewportWidth)),
                          static_cast<int>(384.0f / (GameConfig().InternalHeight / tmpViewportHeight)));



    Button1Transp = 1.0f;
    LastButton1UpdateTime = 0.0f;
    Button2Transp = 1.0f;
    LastButton2UpdateTime = 0.0f;
    Button3Transp = 1.0f;
    LastButton3UpdateTime = 0.0f;
    Button4Transp = 1.0f;
    LastButton4UpdateTime = 0.0f;
    Button5Transp = 1.0f;
    LastButton5UpdateTime = 0.0f;
    Button6Transp = 1.0f;
    LastButton6UpdateTime = 0.0f;
    Button7Transp = 1.0f;
    LastButton7UpdateTime = 0.0f;
    Button8Transp = 1.0f;
    LastButton8UpdateTime = 0.0f;
    Button9Transp = 1.0f;
    LastButton9UpdateTime = 0.0f;
    Button10Transp = 1.0f;
    LastButton10UpdateTime = 0.0f;
    Button11Transp = 1.0f;
    LastButton11UpdateTime = 0.0f;
    Button12Transp = 1.0f;
    LastButton12UpdateTime = 0.0f;
    Button13Transp = 1.0f;
    LastButton13UpdateTime = 0.0f;
    Button14Transp = 1.0f;
    LastButton14UpdateTime = 0.0f;
    LastMenuUpdateTime = 0.0f;
    MenuContentTransp = 0.0f;


    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // camera initialization must be before script (!!!)
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    vw_ResizeScene(45.0f, GameConfig().InternalWidth / GameConfig().InternalHeight, 1.0f, 2000.0f);
    vw_SetCameraLocation(sVECTOR3D{-50.0f, 30.0f, -50.0f});
    vw_SetCameraMoveAroundPoint(sVECTOR3D{0.0f, 0.0f, 0.0f}, 0.0f, sVECTOR3D{0.0f, 0.0f, 0.0f});

    // since we use scripts with background animation in the same way as
    // games levels do, we should reset game camera first
    ResetCamera();

    StarSystemInitByType(eDrawType::MENU); // should be before RunScript()

    MenuScript.reset(new cMissionScript);

    if (GameConfig().MenuScript > 2) {
        ChangeGameConfig().MenuScript = 0;
    }
    switch (GameConfig().MenuScript) {
    case 0:
        MenuScript->RunScript("script/menu1.xml", vw_GetTimeThread(0));
        break;
    case 1:
        MenuScript->RunScript("script/menu2.xml", vw_GetTimeThread(0));
        break;
    case 2:
        MenuScript->RunScript("script/menu3.xml", vw_GetTimeThread(0));
        break;
    // just in case
    default:
        MenuScript->RunScript("script/menu1.xml", vw_GetTimeThread(0));
        break;
    }
    ChangeGameConfig().MenuScript++;

    // scroll the script a bit, in order to see action on the whole screen
    float Time1 = vw_GetTimeThread(0);
    MenuScript->StartTime = Time1-30;
    MenuScript->TimeLastOp = Time1-30;
    for (float i=Time1-30; i<Time1; i+=1.0f) {
        UpdateAllObject3D(i);
        MenuScript->Update(i);
    }
    MenuScript->StartTime = Time1;
    MenuScript->TimeLastOp = Time1;



    LastMenuUpdateTime = vw_GetTimeThread(0);
    NeedShowMenu = true;
    NeedHideMenu = false;
    LastMenuOnOffUpdateTime = vw_GetTimeThread(0);
    MenuBlackTransp = 1.0f;
    NeedOnMenu = true;
    NeedOffMenu = false;
    SetShowGameCursor(true);
}









//------------------------------------------------------------------------------------
// Options menu setup
//------------------------------------------------------------------------------------
void SetOptionsMenu(eMenuStatus Menu)
{
    switch (Menu) {
    case eMenuStatus::OPTIONS:
        Options_Width = GameConfig().Width;
        Options_Height = GameConfig().Height;
        Options_Fullscreen = GameConfig().Fullscreen;
        Options_VSync = GameConfig().VSync;
        break;

    case eMenuStatus::OPTIONS_ADVANCED:
        Options_UseGLSL120 = GameConfig().UseGLSL120;
        Options_MSAA = GameConfig().MSAA;
        Options_CSAA = GameConfig().CSAA;
        Options_TexturesAnisotropyLevel = GameConfig().AnisotropyLevel;
        Options_ShadowMap = GameConfig().ShadowMap;
        break;

    case eMenuStatus::CONFCONTROL:
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
        if (GameConfig().JoystickPrimary == -1) {
            ChangeGameConfig().JoystickPrimary = 0;
        }
        if (GameConfig().JoystickSecondary == -1) {
            ChangeGameConfig().JoystickSecondary = 1;
        }
        break;

    default:
        break;
    }
}
void SetMenu(eMenuStatus Menu)
{
    PlayMenuSFX(eMenuSFX::SwitchToAnotherMenu, 1.0f);

    switch (Menu) {
    case eMenuStatus::PROFILE:
        NewProfileName.clear();
        vw_SetCurrentUnicodeChar(nullptr);
        break;

    case eMenuStatus::OPTIONS:
    case eMenuStatus::OPTIONS_ADVANCED:
    case eMenuStatus::CONFCONTROL:
        SetOptionsMenu(Menu);
        break;

    case eMenuStatus::TOP_SCORES:
        InitTopScoresMenu();
        break;

    case eMenuStatus::MISSION:
        MissionListInit();
        vw_ResetWheelStatus();
        // calculate proper mission range view for mission list
        StartMission = 0;
        EndMission = 4;
        if (CurrentMission != -1 && CurrentMission > 2) { // move range start and end, so, selected element will be in the middle of the list
            StartMission = CurrentMission-2;
            EndMission = CurrentMission+2;

            if (CurrentMission >= AllMission-2) {
                StartMission = AllMission-5;
                EndMission = AllMission-1;
            }
        }
        break;

    case eMenuStatus::INFORMATION:
        vw_ResetWheelStatus();
        CreateNum = 1;
        CreateInfoObject();
        break;

    case eMenuStatus::CREDITS:
        InitCreditsMenu(vw_GetTimeThread(0));
        break;

    default:
        break;

    }

    NextMenu = Menu;

    // hide current menu
    NeedShowMenu = false;
    NeedHideMenu = true;
    LastMenuUpdateTime = vw_GetTimeThread(0);
}



void SetMenu2(eMenuStatus Menu)
{
    // current menu has already become invisible (with fade), release memory after workshop menu + turn off voice
    // NOTE don't release memory before this point, data from 3d objects are still in use (!)
    switch (MenuStatus) {
    case eMenuStatus::WORKSHOP:
        WorkshopDestroyData();
        VoiceNeedMoreEnergy = 0;
        VoiceAmmoOut = 0;
        break;
    default:
        break;
    }



    PrevMenu = MenuStatus;
    MenuStatus = Menu;

    float Time = vw_GetTimeThread(0);
    Button1Transp = 1.0f;
    LastButton1UpdateTime = Time;
    Button2Transp = 1.0f;
    LastButton2UpdateTime = Time;
    Button3Transp = 1.0f;
    LastButton3UpdateTime = Time;
    Button4Transp = 1.0f;
    LastButton4UpdateTime = Time;
    Button5Transp = 1.0f;
    LastButton5UpdateTime = Time;
    Button6Transp = 1.0f;
    LastButton6UpdateTime = Time;
    Button7Transp = 1.0f;
    LastButton7UpdateTime = Time;
    Button8Transp = 1.0f;
    LastButton8UpdateTime = Time;
    Button9Transp = 1.0f;
    LastButton9UpdateTime = Time;
    Button10Transp = 1.0f;
    LastButton10UpdateTime = Time;
    Button11Transp = 1.0f;
    LastButton11UpdateTime = Time;
    Button12Transp = 1.0f;
    LastButton12UpdateTime = Time;
    Button13Transp = 1.0f;
    LastButton13UpdateTime = Time;
    Button14Transp = 1.0f;
    LastButton14UpdateTime = Time;
}






//------------------------------------------------------------------------------------
// draw menu
//------------------------------------------------------------------------------------
void DrawMenu()
{

    // make the menu appear smoothly
    if (NeedShowMenu) {
        MenuContentTransp = 2.4f*(vw_GetTimeThread(0) - LastMenuUpdateTime);
        if (MenuContentTransp >= 1.0f) {
            MenuContentTransp = 1.0f;
            NeedShowMenu = false;
            LastMenuUpdateTime = vw_GetTimeThread(0);

            // show hint
            if (MenuStatus == eMenuStatus::PROFILE && GameConfig().NeedShowHint[0]) {
                SetCurrentDialogBox(eDialogBox::ProfileTipsAndTricks);
            }
            if (MenuStatus == eMenuStatus::WORKSHOP) {
                if (CurrentWorkshop == 1 && GameConfig().NeedShowHint[1]) {
                    SetCurrentDialogBox(eDialogBox::ShipyardTipsAndTricks);
                }
                if (CurrentWorkshop == 2 && GameConfig().NeedShowHint[2]) {
                    SetCurrentDialogBox(eDialogBox::SystemsTipsAndTricks);
                }
                if (CurrentWorkshop == 3 && GameConfig().NeedShowHint[3]) {
                    SetCurrentDialogBox(eDialogBox::WeaponryTipsAndTricks);
                }
            }
        }
    }

    // make the menu fade out
    if (NeedHideMenu) {
        MenuContentTransp = 1.0f - 2.4f*(vw_GetTimeThread(0) - LastMenuUpdateTime);
        if (MenuContentTransp <= 0.0f) {
            MenuContentTransp = 0.0f;
            SetMenu2(NextMenu);
            LastMenuUpdateTime = vw_GetTimeThread(0);
            NeedShowMenu = true;
            NeedHideMenu = false;
        }
    }


    vw_SetCameraMoveAroundPoint(sVECTOR3D{0.0f, 0.0f, 0.0f}, 0.0f, sVECTOR3D{0.0f, 0.0f, 0.0f});
    vw_CameraLookAt();




    // always draw the skybox and the far away environment first
    StarSystemDraw(eDrawType::MENU);




    // draw AstroMenace title
    vw_Start2DMode(-1,1);
    sRECT SrcRect, DstRect;
    SrcRect(0,0,863,128 );
    int StartX = (GameConfig().InternalWidth - 863)/2;
    DstRect(StartX,10,StartX+863,10+128);

    if (MenuStatus != eMenuStatus::WORKSHOP
        && MenuStatus != eMenuStatus::INTERFACE
        && MenuStatus != eMenuStatus::OPTIONS
        && MenuStatus != eMenuStatus::CONFCONTROL
        && MenuStatus != eMenuStatus::OPTIONS_ADVANCED) {
        constexpr unsigned tmpHash = constexpr_hash_djb2a("menu/astromenace.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash), true, MenuContentTransp);
    }

    vw_End2DMode();



    // render 3D objects
    DrawAllObject3D(eDrawType::MENU);



    // update data for all objects after render
    UpdateAllObject3D(vw_GetTimeThread(0));
    vw_UpdateAllParticleSystems(vw_GetTimeThread(0));




    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // working with script
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    if (MenuScript && !MenuScript->Update(vw_GetTimeThread(0))) {
        MenuScript.reset();
    }





    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // switch to 2D render for draw all 2D menu parts
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    vw_Start2DMode(-1,1);

    switch (MenuStatus) {
    case eMenuStatus::MAIN_MENU:
        MainMenu();
        break;
    case eMenuStatus::TOP_SCORES:
        TopScoresMenu();
        break;
    case eMenuStatus::INTERFACE:
        InterfaceMenu(MenuContentTransp, Button10Transp, LastButton10UpdateTime);
        break;
    case eMenuStatus::OPTIONS:
        OptionsMenu(MenuContentTransp, Button10Transp, LastButton10UpdateTime, Button11Transp, LastButton11UpdateTime);
        break;
    case eMenuStatus::OPTIONS_ADVANCED:
        OptionsAdvMenu(MenuContentTransp, Button10Transp, LastButton10UpdateTime, Button11Transp, LastButton11UpdateTime);
        break;
    case eMenuStatus::INFORMATION:
        InformationMenu();
        break;
    case eMenuStatus::CREDITS:
        CreditsMenu();
        break;
    case eMenuStatus::CONFCONTROL:
        ConfControlMenu(MenuContentTransp, Button10Transp, LastButton10UpdateTime);
        break;
    case eMenuStatus::PROFILE:
        ProfileMenu();
        break;
    case eMenuStatus::DIFFICULTY:
        DifficultyMenu();
        break;
    case eMenuStatus::MISSION:
        MissionMenu();
        break;
    case eMenuStatus::WORKSHOP:
        WorkshopMenu();
        break;

    default:
        break;
    }




    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Version and Copyright
    //+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    vw_SetFontSize(10);

    // Version
    int VSize = vw_TextWidthUTF32(vw_GetTextUTF32("Version"));
    vw_DrawTextUTF32(6, GameConfig().InternalHeight - 16, 0, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, 0.99f, vw_GetTextUTF32("Version"));
    vw_DrawText(16 + VSize, GameConfig().InternalHeight - 16, 0, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, 0.99f, GAME_VERSION);
    // Copyright
    int CSize = vw_TextWidth("Copyright © 2007-2025, Viewizard");
    vw_DrawText(GameConfig().InternalWidth - 6 - CSize, GameConfig().InternalHeight - 16, 0, 0, 1.0f, sRGBCOLOR{eRGBCOLOR::white}, 0.99f,
                "Copyright © 2007-2025, Viewizard");

    ResetFontSize();
    vw_End2DMode();


    switch (MenuStatus) {
    case eMenuStatus::INFORMATION:
        InformationDrawObject();
        break;

    default:
        break;
    }




    // fade-in
    if (NeedOnMenu) {
        MenuBlackTransp = 1.0f - 2.4f*(vw_GetTimeThread(0) - LastMenuOnOffUpdateTime);
        if (MenuBlackTransp <= 0.0f) {
            MenuBlackTransp = 0.0f;
            NeedOnMenu = false;
            LastMenuOnOffUpdateTime = vw_GetTimeThread(0);
        }

        vw_Start2DMode(-1,1);

        SrcRect(0, 0, 2, 2);
        DstRect(0, 0, GameConfig().InternalWidth, 768);
        constexpr unsigned tmpHash = constexpr_hash_djb2a("menu/blackpoint.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash), true, MenuBlackTransp);

        vw_End2DMode();
    }

    // fade-out, switch from menu to game
    if (NeedOffMenu) {
        MenuBlackTransp = 2.4f*(vw_GetTimeThread(0) - LastMenuOnOffUpdateTime);
        if (MenuBlackTransp >= 1.0f) {
            MenuBlackTransp = 1.0f;
            NeedOffMenu = false;
            LastMenuOnOffUpdateTime = vw_GetTimeThread(0);
            WorkshopDestroyData();
            cCommand::GetInstance().Set(eCommand::SWITCH_FROM_MENU_TO_GAME);
        }

        vw_Start2DMode(-1,1);

        SrcRect(0, 0, 2, 2);
        DstRect(0, 0, GameConfig().InternalWidth, 768);
        constexpr unsigned tmpHash = constexpr_hash_djb2a("menu/blackpoint.tga");
        vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(tmpHash), true, MenuBlackTransp);

        vw_End2DMode();
    }

}












//------------------------------------------------------------------------------------
// main menu
//------------------------------------------------------------------------------------
void MainMenu()
{

    int Prir = 100;
    int X = (GameConfig().InternalWidth - 384) / 2;
    int Y = 165;

    if (DrawButton384(X,Y, vw_GetTextUTF32("START GAME"), MenuContentTransp, Button1Transp, LastButton1UpdateTime)) {
        // if there is no current profile - switch to profile list menu, otherwise switch to mission list menu
        if (CurrentProfile < 0) {
            cCommand::GetInstance().Set(eCommand::SWITCH_TO_PROFILE);
        } else {
            cCommand::GetInstance().Set(eCommand::SWITCH_TO_MISSION);
        }
    }





    Y = Y+Prir;
    if (DrawButton384(X,Y, vw_GetTextUTF32("TOP SCORES"), MenuContentTransp, Button2Transp, LastButton2UpdateTime)) {
        cCommand::GetInstance().Set(eCommand::SWITCH_TO_TOP_SCORES);
    }

    Y = Y+Prir;
    if (DrawButton384(X,Y, vw_GetTextUTF32("OPTIONS"), MenuContentTransp, Button3Transp, LastButton3UpdateTime)) {
        cCommand::GetInstance().Set(eCommand::SWITCH_TO_OPTIONS);
    }

    Y = Y+Prir;
    if (DrawButton384(X,Y, vw_GetTextUTF32("INFORMATION"), MenuContentTransp, Button4Transp, LastButton4UpdateTime)) {
        cCommand::GetInstance().Set(eCommand::SWITCH_TO_INFORMATION);
    }

    Y = Y+Prir;
    if (DrawButton384(X,Y, vw_GetTextUTF32("CREDITS"), MenuContentTransp, Button5Transp, LastButton5UpdateTime)) {
        cCommand::GetInstance().Set(eCommand::SWITCH_TO_CREDITS);
        PlayMusicTheme(eMusicTheme::CREDITS, 2000, 2000);
    }

    Y = Y+Prir;
    if (DrawButton384(X,Y, vw_GetTextUTF32("QUIT"), MenuContentTransp, Button6Transp, LastButton6UpdateTime)) {
        SetCurrentDialogBox(eDialogBox::QuitFromGame);
    }

}

} // astromenace namespace
} // viewizard namespace
