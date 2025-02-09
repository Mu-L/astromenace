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

#include "../core/core.h"
#include "../config/config.h"
#include "../assets/audio.h"
#include "../command.h"
#include "../game.h" // FIXME "game.h" should be replaced by individual headers

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

namespace {

const std::vector<std::string> CreditsArray{
    "PROJECT LEADER, PROGRAMMING",
    "Mikhail Kurinnoi",
    "",
    "2D ARTWORK",
    "Tatiana Savitskaya",
    "",
    "3D MODELS",
    "Max Shelekhov, 3drt.com",
    "Oleg Linkov",
    "Egor Kurteanu",
    "",
    "PLANETARY TEXTURES",
    "Alex (Esgaroth) Reuter",
    "Robert Stein",
    "",
    "MUSIC",
    "Alexander Matchugovsky",
    "",
    "SOUND EFFECTS",
    "Mikhail Sirotkin",
    "Alexander Matchugovsky",
    "Alexander Vassilyevsky",
    "",
    "VOICE",
    "Petra Schulten (de)",
    "Elena Zavalishina (en, ru)",
    "",
    "TRANSLATORS",
    "Bernd Dau (de)",
    "Jürgen Buchmüller (de)", // Not in `text.csv` file.
    "Michael Lauenburger (de)", // Not in `text.csv` file.
    "Mikhail Kurinnoi (ru)",
    "Jacek Fiedorowicz (pl)",
    "Juan Francisco Astudillo Palacios (es)",
    "p1p2r (tr)", // Not in `text.csv` file.
    "",
    "COMMUNITY CONTRIBUTORS",
    "Alistair Findlay",
    "Dmitry Marakasov",
    "Gwyn Ciesla", // Not in `text.csv` file.
    "Stefan Strogin", // Not in `text.csv` file.
    "sevu", // Not in `text.csv` file.
    "",
    "TESTERS",
    "Dmitriy Schigolev",
    "Ivan Bobrov",
    "Vadim Sosenko",
    "Dmitriy Tatarinov",
    "Alexey Mosesov",
    "Bernd Dau",
    "Ryan Foster",
    "Jason Crowell",
    "",
    "SPECIAL THANKS TO",
    "Pavel Voylov",
    "Richard Benson",
    "Alexey Madalinskiy",
    "Olga Zhuravleva",
    "Nick O`Connell",
    "Mikhail Storcheous",
    "Boris Pek",
    "",
    "",
    "",
    " ", /* this is correct, we need not empty line for white text color in next line */
    "Copyright © 2007-2025, Viewizard",
    "https://viewizard.com"
};

float CurrentTextPos{0.0f};
float LastUpdate{0.0f};

} // unnamed namespace


/*
 * Credits menu initialization.
 */
void InitCreditsMenu(float Time)
{
    CurrentTextPos = 0.0f;
    LastUpdate = Time;
}

/*
 * Credits menu (draw and interaction).
 */
void CreditsMenu()
{
    // text animation, about 40 pixels in second
    CurrentTextPos -= 40.0f * (vw_GetTimeThread(0) - LastUpdate);
    LastUpdate = vw_GetTimeThread(0);

    // 690 - start inside internal fixed size of screen (768) with "offset"
    int TextY = 690 + static_cast<int>(CurrentTextPos);
    int Offset = 30;

    // cycling (internal fixed size of screen (768) - "borders" + CreditsArray.size() * Offset)
    if (CurrentTextPos <= -(550 + static_cast<int>(CreditsArray.size()) * Offset)) {
        CurrentTextPos = 0.0f;
    }

    // we switch white/yellow colors, first is 'title', should be yellow
    eRGBCOLOR tmpColor{eRGBCOLOR::yellow};
    for (unsigned i = 0; i < CreditsArray.size(); i++) {
        float tmpTransp = 0.0f;
        if (TextY > 300) { // fade-in
            tmpTransp = (650 - TextY) / 100.0f;
        } else { // fade-out
            tmpTransp = (TextY - 100) / 100.0f;
        }
        vw_Clamp(tmpTransp, 0.0f, 1.0f);

        if (!CreditsArray[i].empty()) {
            const std::u32string &tmpText = vw_GetTextUTF32(CreditsArray[i]);
            int CenteredX = (GameConfig().InternalWidth - vw_TextWidthUTF32(tmpText)) / 2;
            vw_DrawTextUTF32(CenteredX, TextY, 0, 0, 1.0f,
                             sRGBCOLOR{tmpColor}, tmpTransp * MenuContentTransp,
                             tmpText);
        }
        TextY += Offset;

        // if current line is empty, next one will be yellow, otherwise - white
        tmpColor = CreditsArray[i].empty() ? eRGBCOLOR::yellow : eRGBCOLOR::white;
    }

/*
    int X = GameConfig().InternalWidth / 2 - 256 - 38;
    int Y = 165 + 100 * 5;
    if (DrawButton256(X,Y, vw_GetTextUTF32("MAIN MENU"), MenuContentTransp, Button10Transp, LastButton10UpdateTime)) {
        cCommand::GetInstance().Set(eCommand::SWITCH_TO_MAIN_MENU);
        PlayMusicTheme(eMusicTheme::MENU, 2000, 2000);
    }
    X = GameConfig().InternalWidth / 2 + 38;
    if (DrawButton256(X,Y, vw_GetTextUTF32("DONATE"), MenuContentTransp, Button11Transp, LastButton11UpdateTime)) {
        vw_OpenWebsiteURL("https://viewizard.com/donate.html");
    }
*/
    int X = (GameConfig().InternalWidth - 384) / 2;
    int Y = 165 + 100 * 5;
    if (DrawButton384(X,Y, vw_GetTextUTF32("MAIN MENU"), MenuContentTransp, Button10Transp, LastButton10UpdateTime)) {
        cCommand::GetInstance().Set(eCommand::SWITCH_TO_MAIN_MENU);
        PlayMusicTheme(eMusicTheme::MENU, 2000, 2000);
    }
}

} // astromenace namespace
} // viewizard namespace
