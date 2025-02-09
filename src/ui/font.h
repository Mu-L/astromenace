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

#ifndef UI_FONT_H
#define UI_FONT_H

#include "../core/base.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

struct sFontMetadata {
    std::string FontTitle{};
    std::string FontFileName{};

    explicit sFontMetadata(const std::string &_FontTitle,
                           const std::string &_FontFileName) :
        FontTitle{_FontTitle},
        FontFileName{_FontFileName}
    {}
};

// Initialize font.
void InitFont(const std::string &FontName);
// Reset font size to main font size.
void ResetFontSize();
// Generate all in-game fonts for all sizes.
void GenerateFonts();

// Get font metadata by font number.
const sFontMetadata &GetFontMetadata(int FontNumber);
// Get fonts quantity.
int GetFontQuantity();

} // astromenace namespace
} // viewizard namespace

#endif // UI_FONT_H
