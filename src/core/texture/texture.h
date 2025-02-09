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

#ifndef CORE_TEXTURE_TEXTURE_H
#define CORE_TEXTURE_TEXTURE_H

#include "../base.h"
#include "../graphics/graphics.h"

namespace viewizard {

enum class eLoadTextureAs {
    AUTO,   // Detect by file extension
    VW2D,   // VW2D file
    TGA     // TGA file
};

enum class eAlphaCreateMode {
    NONE,   // Fill alpha channel with 255
    GREYSC, // Create alpha channel by image greyscale color
    EQUAL   // Create alpha channel by equal Alpha color
};

// Load texture from file.
// Note, in case of resize, we should provide width and height (but not just one of them).
GLtexture vw_LoadTexture(const std::string &TextureName,
                         eTextureCompressionType CompressionType = eTextureCompressionType::NONE,
                         eLoadTextureAs LoadAs = eLoadTextureAs::AUTO,
                         int NeedResizeW = 0, int NeedResizeH = 0);
// Create texture from memory.
GLtexture vw_CreateTextureFromMemory(const std::string &TextureName, std::unique_ptr<uint8_t[]> &PixelsArray,
                                     int ImageWidth, int ImageHeight, int ImageChanels,
                                     eTextureCompressionType CompressionType = eTextureCompressionType::NONE,
                                     int NeedResizeW = 0, int NeedResizeH = 0);
// Release texture.
void vw_ReleaseTexture(GLtexture TextureID);
// Release all textures.
void vw_ReleaseAllTextures();
// Set textures properties.
void vw_SetTextureProp(const sTextureFilter &Filtering, GLint AnisotropyLevel, const sTextureWrap &AddressMode,
                       bool Alpha = false, eAlphaCreateMode AFlag = eAlphaCreateMode::EQUAL, bool MipMap = true);
// Set textures alpha color.
void vw_SetTextureAlpha(uint8_t nARed, uint8_t nAGreen, uint8_t nABlue);
// Find texture's size by ID.
bool vw_FindTextureSizeByID(GLtexture TextureID, float *Width = nullptr, float *Height = nullptr);
// Convert supported image file format to VW2D format.
void vw_ConvertImageToVW2D(const std::string &SrcName, const std::string &DestName);

} // viewizard namespace

#endif // CORE_TEXTURE_TEXTURE_H
