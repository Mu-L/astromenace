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

// TODO don't call GetPreloadedTextureAsset() all the time, use cached texture instead

// TODO why we need ArmorCurrentStatus/ArmorInitialStatus setup for immortal object?

#include "ground_object.h"
#include "../../config/config.h"
#include "../../assets/texture.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

namespace {

struct sBuildingData {
    float Armor;
    unsigned Model3DFileNameHash;
};

const std::vector<sBuildingData> PresetBuildingData{
    {150,    constexpr_hash_djb2a("models/building/bld-01.vw3d")},
    {150,    constexpr_hash_djb2a("models/building/bld-02.vw3d")},
    {150,    constexpr_hash_djb2a("models/building/bld-03.vw3d")},
    {150,    constexpr_hash_djb2a("models/building/bld-04.vw3d")},
    {150,    constexpr_hash_djb2a("models/building/bld-05.vw3d")},
    {150,    constexpr_hash_djb2a("models/building/bld-06.vw3d")},
    {150,    constexpr_hash_djb2a("models/building/bld-07.vw3d")},
    {150,    constexpr_hash_djb2a("models/building/bld-08.vw3d")},
    {150,    constexpr_hash_djb2a("models/building/bld-09.vw3d")},
    {150,    constexpr_hash_djb2a("models/building/bld-10.vw3d")},
    {150,    constexpr_hash_djb2a("models/building/bld-11.vw3d")}
};

} // unnamed namespace


/*
 * Constructor.
 */
cCivilianBuilding::cCivilianBuilding(const int BuildingNum)
{
    if (BuildingNum <= 0 || static_cast<unsigned>(BuildingNum) > PresetBuildingData.size()) {
        std::cerr << __func__ << "(): " << "Could not init cBuilding object with Number " << BuildingNum << "\n";
        return;
    }

    ObjectType = eObjectType::CivilianBuilding;
    InternalType = BuildingNum;
    ShowStatus = false;
    PromptDrawDist2 = 100.0f;

    LoadObjectData(PresetBuildingData[BuildingNum - 1].Model3DFileNameHash, *this);

    constexpr unsigned tmpTextureHash = constexpr_hash_djb2a("models/building/bld.vw2d");
    GLtexture tmpTexture = GetPreloadedTextureAsset(tmpTextureHash);
    constexpr unsigned tmpTextureIllumHash = constexpr_hash_djb2a("models/building/bld_illum.vw2d");
    GLtexture tmpTextureIllum = GetPreloadedTextureAsset(tmpTextureIllumHash);
    constexpr unsigned tmpNormalMapHash = constexpr_hash_djb2a("models/normalmap/buildings_nm.tga");
    GLtexture tmpNormalMap = GetPreloadedTextureAsset(tmpNormalMapHash);
    for (unsigned int i = 0; i < Chunks.size(); i++) {
        Texture[i] = tmpTexture;
        TextureIllum[i] = tmpTextureIllum;
        NormalMap[i] = tmpNormalMap;
    }

    ArmorCurrentStatus = ArmorInitialStatus = PresetBuildingData[BuildingNum - 1].Armor / GameEnemyArmorPenalty;
}

} // astromenace namespace
} // viewizard namespace
