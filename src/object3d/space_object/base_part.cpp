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

#include "space_object.h"
#include "../../config/config.h"
#include "../../assets/texture.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

namespace {

enum class eEngineGFX {
    Primary,
    Auxiliary
};

} // unnamed namespace

/*
 * Setup engine gfx.
 */
static void SetupEngineGFX(std::shared_ptr<cParticleSystem> &ParticleSystem, const eEngineGFX EngineType)
{
    constexpr unsigned tmpHash = constexpr_hash_djb2a("gfx/flare1.tga");
    ParticleSystem->Texture = GetPreloadedTextureAsset(tmpHash);

    static const sRGBCOLOR MuddyYellow{0.6f, 0.6f, 0.3f};
    static const sRGBCOLOR LightGreen{0.3f, 1.0f, 0.3f};

    switch (EngineType) {
    case eEngineGFX::Primary:
        ParticleSystem->ColorStart = MuddyYellow;
        ParticleSystem->ColorEnd = LightGreen;
        ParticleSystem->AlphaStart = 1.00f;
        ParticleSystem->AlphaEnd = 0.00f;
        ParticleSystem->SizeStart = 0.30f;
        ParticleSystem->SizeVar = 0.30f;
        ParticleSystem->SizeEnd = 0.60f;
        ParticleSystem->Speed = 10.00f;
        ParticleSystem->SpeedOnCreation = -1.00f;
        ParticleSystem->SpeedVar = 2.00f;
        ParticleSystem->Theta = 5.00f;
        ParticleSystem->Life = 0.50f;
        ParticleSystem->ParticlesPerSec = 100;
        ParticleSystem->CreationType = eParticleCreationType::Sphere;
        ParticleSystem->CreationSize(0.8f, 0.1f, 0.8f);
        ParticleSystem->AlphaShowHide = true;
        ParticleSystem->Direction(0.0f, -1.0f, 0.0f);
        ParticleSystem->Light = vw_CreatePointLight(sVECTOR3D{0.0f, 0.0f, 0.0f}, 0.45f, 0.8f, 0.3f, 0.0f, 0.025f);
        break;

    case eEngineGFX::Auxiliary:
        ParticleSystem->ColorStart = MuddyYellow;
        ParticleSystem->ColorEnd = LightGreen;
        ParticleSystem->AlphaStart = 1.00f;
        ParticleSystem->AlphaEnd = 0.00f;
        ParticleSystem->SizeStart = 0.30f;
        ParticleSystem->SizeVar = 0.20f;
        ParticleSystem->SizeEnd = 0.10f;
        ParticleSystem->Speed = 3.00f;
        ParticleSystem->SpeedOnCreation = -1.00f;
        ParticleSystem->SpeedVar = 2.00f;
        ParticleSystem->Theta = 180.00f;
        ParticleSystem->Life = 0.50f;
        ParticleSystem->ParticlesPerSec = 100;
        ParticleSystem->AlphaShowHide = true;
        ParticleSystem->Direction(0.0f, 1.0f, 0.0f);
        ParticleSystem->Light = vw_CreatePointLight(sVECTOR3D{0.0f, 0.0f, 0.0f}, 0.45f, 0.8f, 0.3f, 0.0f, 0.3f);
        break;
    }
}

/*
 * Setup base part type 1.
 */
void cBasePart::SetupBasePartType1()
{
    constexpr unsigned tmpTextureHash = constexpr_hash_djb2a("models/planet/d_class3.tga");
    Texture[0] = GetPreloadedTextureAsset(tmpTextureHash);
    TextureIllum[0] = 0;
    constexpr unsigned tmpNormalMapHash = constexpr_hash_djb2a("models/normalmap/d_class3_nm.tga");
    NormalMap[0] = GetPreloadedTextureAsset(tmpNormalMapHash);
    constexpr unsigned tmpTextureHash2 = constexpr_hash_djb2a("models/spacebase/allalpha.tga");
    Texture[1] = GetPreloadedTextureAsset(tmpTextureHash2);
    TextureIllum[1] = 0;
    constexpr unsigned tmpTextureHash3 = constexpr_hash_djb2a("models/spacebase/metal.tga");
    Texture[2] = GetPreloadedTextureAsset(tmpTextureHash3);
    TextureIllum[2] = 0;

    GraphicFXLocation.resize(8);
    GraphicFX.resize(8);

    GraphicFX[0] = vw_CreateParticleSystem();
    GraphicFXLocation[0] = sVECTOR3D{14.3f, -4.0f, -14.3f};
    if (auto sharedGFX = GraphicFX[0].lock()) {
        SetupEngineGFX(sharedGFX, eEngineGFX::Primary);
    }
    GraphicFX[1] = vw_CreateParticleSystem();
    GraphicFXLocation[1] = sVECTOR3D{14.3f, -4.0f, 14.3f};
    if (auto sharedGFX = GraphicFX[1].lock()) {
        SetupEngineGFX(sharedGFX, eEngineGFX::Primary);
    }
    GraphicFX[2] = vw_CreateParticleSystem();
    GraphicFXLocation[2] = sVECTOR3D{-14.3f, -4.0f, -14.3f};
    if (auto sharedGFX = GraphicFX[2].lock()) {
        SetupEngineGFX(sharedGFX, eEngineGFX::Primary);
    }
    GraphicFX[3] = vw_CreateParticleSystem();
    GraphicFXLocation[3] = sVECTOR3D{-14.3f, -4.0f, 14.3f};
    if (auto sharedGFX = GraphicFX[3].lock()) {
        SetupEngineGFX(sharedGFX, eEngineGFX::Primary);
    }

    GraphicFX[4] = vw_CreateParticleSystem();
    GraphicFXLocation[4] = sVECTOR3D{14.3f, 3.2f, -14.3f};
    if (auto sharedGFX = GraphicFX[4].lock()) {
        SetupEngineGFX(sharedGFX, eEngineGFX::Auxiliary);
    }
    GraphicFX[5] = vw_CreateParticleSystem();
    GraphicFXLocation[5] = sVECTOR3D{14.3f, 3.2f, 14.3f};
    if (auto sharedGFX = GraphicFX[5].lock()) {
        SetupEngineGFX(sharedGFX, eEngineGFX::Auxiliary);
    }
    GraphicFX[6] = vw_CreateParticleSystem();
    GraphicFXLocation[6] = sVECTOR3D{-14.3f, 3.2f, -14.3f};
    if (auto sharedGFX = GraphicFX[6].lock()) {
        SetupEngineGFX(sharedGFX, eEngineGFX::Auxiliary);
    }
    GraphicFX[7] = vw_CreateParticleSystem();
    GraphicFXLocation[7] = sVECTOR3D{-14.3f, 3.2f, 14.3f};
    if (auto sharedGFX = GraphicFX[7].lock()) {
        SetupEngineGFX(sharedGFX, eEngineGFX::Auxiliary);
    }

    for (unsigned int i = 0; i < GraphicFX.size(); i++) {
        if (auto sharedGFX = GraphicFX[i].lock()) {
            sharedGFX->MoveSystem(GraphicFXLocation[i]);
            sharedGFX->SetStartLocation(GraphicFXLocation[i]);
        }
    }
}

/*
 * Setup base part type 2.
 */
void cBasePart::SetupBasePartType2()
{
    constexpr unsigned tmpTextureHash = constexpr_hash_djb2a("models/spacebase/allalpha.tga");
    Texture[0] = GetPreloadedTextureAsset(tmpTextureHash);
    TextureIllum[0] = 0;
    constexpr unsigned tmpTextureHash2 = constexpr_hash_djb2a("models/planet/d_class3.tga");
    Texture[1] = GetPreloadedTextureAsset(tmpTextureHash2);
    TextureIllum[1] = 0;
    constexpr unsigned tmpNormalMapHash = constexpr_hash_djb2a("models/normalmap/d_class3_nm.tga");
    NormalMap[1] = GetPreloadedTextureAsset(tmpNormalMapHash);
    constexpr unsigned tmpTextureHash3 = constexpr_hash_djb2a("models/spacebase/metal.tga");
    Texture[2] = GetPreloadedTextureAsset(tmpTextureHash3);
    TextureIllum[2] = 0;
}

/*
 * Setup base part type 3.
 */
void cBasePart::SetupBasePartType3()
{
    constexpr unsigned tmpTextureHash = constexpr_hash_djb2a("models/spacebase/metal.tga");
    Texture[0] = GetPreloadedTextureAsset(tmpTextureHash);
    TextureIllum[0] = 0;
    constexpr unsigned tmpTextureHash2 = constexpr_hash_djb2a("models/spacebase/allalpha.tga");
    Texture[1] = GetPreloadedTextureAsset(tmpTextureHash2);
    TextureIllum[1] = 0;

    GraphicFXLocation.resize(2);
    GraphicFX.resize(2);

    GraphicFX[0] = vw_CreateParticleSystem();
    GraphicFXLocation[0] = sVECTOR3D{0.0f, -5.0f, 13.7f};
    if (auto sharedGFX = GraphicFX[0].lock()) {
        SetupEngineGFX(sharedGFX, eEngineGFX::Primary);
    }
    GraphicFX[1] = vw_CreateParticleSystem();
    GraphicFXLocation[1] = sVECTOR3D{0.0f, 5.0f, 13.7f};
    if (auto sharedGFX = GraphicFX[1].lock()) {
        SetupEngineGFX(sharedGFX, eEngineGFX::Auxiliary);
    }

    for (unsigned int i = 0; i < GraphicFX.size(); i++) {
        if (auto sharedGFX = GraphicFX[i].lock()) {
            sharedGFX->MoveSystem(GraphicFXLocation[i]);
            sharedGFX->SetStartLocation(GraphicFXLocation[i]);
        }
    }
}

/*
 * Setup base part type 4.
 */
void cBasePart::SetupBasePartType4()
{
    constexpr unsigned tmpTextureHash = constexpr_hash_djb2a("models/spacebase/allalpha.tga");
    Texture[0] = GetPreloadedTextureAsset(tmpTextureHash);
    TextureIllum[0] = 0;
}

/*
 * Setup base part type 5.
 */
void cBasePart::SetupBasePartType5()
{
    constexpr unsigned tmpTextureHash = constexpr_hash_djb2a("models/spacebase/allalpha.tga");
    Texture[0] = GetPreloadedTextureAsset(tmpTextureHash);
    TextureIllum[0] = 0;
}

/*
 * Setup base part type 6.
 */
void cBasePart::SetupBasePartType6()
{
    constexpr unsigned tmpTextureHash = constexpr_hash_djb2a("models/spacebase/allalpha.tga");
    Texture[0] = GetPreloadedTextureAsset(tmpTextureHash);
    TextureIllum[0] = 0;
    constexpr unsigned tmpTextureHash2 = constexpr_hash_djb2a("models/planet/d_class3.tga");
    Texture[1] = GetPreloadedTextureAsset(tmpTextureHash2);
    TextureIllum[1] = 0;
    constexpr unsigned tmpNormalMapHash = constexpr_hash_djb2a("models/normalmap/d_class3_nm.tga");
    NormalMap[1] = GetPreloadedTextureAsset(tmpNormalMapHash);
}

/*
 * Setup base part type 7.
 */
void cBasePart::SetupBasePartType7()
{
    constexpr unsigned tmpTextureHash = constexpr_hash_djb2a("models/spacebase/allalpha.tga");
    Texture[0] = GetPreloadedTextureAsset(tmpTextureHash);
    TextureIllum[0] = 0;
}

/*
 * Setup base part type 8.
 */
void cBasePart::SetupBasePartType8()
{
    constexpr unsigned tmpTextureHash = constexpr_hash_djb2a("models/spacebase/allalpha.tga");
    Texture[0] = GetPreloadedTextureAsset(tmpTextureHash);
    TextureIllum[0] = 0;
    constexpr unsigned tmpTextureHash2 = constexpr_hash_djb2a("models/spacebase/metal.tga");
    Texture[1] = GetPreloadedTextureAsset(tmpTextureHash2);
    TextureIllum[1] = 0;
    constexpr unsigned tmpTextureHash3 = constexpr_hash_djb2a("models/planet/d_class3.tga");
    Texture[2] = GetPreloadedTextureAsset(tmpTextureHash3);
    TextureIllum[2] = 0;
    constexpr unsigned tmpNormalMapHash = constexpr_hash_djb2a("models/normalmap/d_class3_nm.tga");
    NormalMap[2] = GetPreloadedTextureAsset(tmpNormalMapHash);
}

/*
 * Constructor.
 */
cBasePart::cBasePart(const int BasePartNum)
{
    ObjectStatus = eObjectStatus::Enemy;
    ObjectType = eObjectType::BasePart;
    NeedAlphaTest = true;

    // for base part type 1, 2, 6 or 8 we could pick random block or provide particular id
    if (BasePartNum == 1 || BasePartNum == 2 || BasePartNum == 6 || BasePartNum == 8) {
        InternalType = BasePartNum * 10 + 1 + vw_uRandNum(4);
    } else {
        InternalType = BasePartNum;
    }

    switch (InternalType) {
    case 11:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/1/1.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType1();
        break;
    case 12:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/1/2.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType1();
        break;
    case 13:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/1/3.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType1();
        break;
    case 14:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/1/4.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType1();
        break;
    case 15:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/1/5.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType1();
        break;

    case 21:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/2/1.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType2();
        break;
    case 22:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/2/2.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType2();
        break;
    case 23:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/2/3.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType2();
        break;
    case 24:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/2/4.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType2();
        break;
    case 25:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/2/5.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType2();
        break;

    case 3:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/3/1.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType3();
        break;

    case 4:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/4/1.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType4();
        break;

    case 5:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/5/1.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType5();
        break;

    case 61:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/6/1.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType6();
        break;
    case 62:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/6/2.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType6();
        break;
    case 63:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/6/3.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType6();
        break;
    case 64:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/6/4.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType6();
        break;
    case 65:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/6/5.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType6();
        break;

    case 7:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/7/1.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType7();
        break;

    case 81:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/8/1.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType8();
        break;
    case 82:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/8/2.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType8();
        break;
    case 83:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/8/3.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType8();
        break;
    case 84:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/8/4.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType8();
        break;
    case 85:
        {
            constexpr unsigned tmpHash = constexpr_hash_djb2a("models/spacebase/8/5.vw3d");
            LoadObjectData(tmpHash, *this);
        }
        SetupBasePartType8();
        break;
    }
}

/*
 * Destructor.
 */
cBasePart::~cBasePart()
{
    if (GraphicFX.empty()) {
        return;
    }

    for (auto &tmpGFX : GraphicFX) {
        if (auto sharedGFX = tmpGFX.lock()) {
            sharedGFX->IsSuppressed = true;
            sharedGFX->DestroyIfNoParticles = true;
        }
    }
}

/*
 * Set location.
 */
void cBasePart::SetLocation(const sVECTOR3D &NewLocation)
{
    cObject3D::SetLocation(NewLocation);

    if (GraphicFX.empty()) {
        return;
    }

    for (unsigned int i = 0; i < GraphicFX.size(); i++) {
        if (auto sharedGFX = GraphicFX[i].lock()) {
            sharedGFX->MoveSystem(GraphicFXLocation[i] + NewLocation);
            sharedGFX->SetStartLocation(GraphicFXLocation[i] + NewLocation);
        }
    }
}

/*
 * Set rotation.
 */
void cBasePart::SetRotation(const sVECTOR3D &NewRotation)
{
    cObject3D::SetRotation(NewRotation);

    if (GraphicFX.empty()) {
        return;
    }

    for (unsigned int i = 0; i < GraphicFX.size(); i++) {
        if (auto sharedGFX = GraphicFX[i].lock()) {
            vw_Matrix33CalcPoint(GraphicFXLocation[i], OldInvRotationMat);
            vw_Matrix33CalcPoint(GraphicFXLocation[i], CurrentRotationMat);

            if (sharedGFX->SpeedOnCreation == -1.0f) {
                sharedGFX->MoveSystem(GraphicFXLocation[i] + Location);
                sharedGFX->SetStartLocation(GraphicFXLocation[i] + Location);
                sharedGFX->RotateSystemAndParticlesByAngle(Rotation);
            } else {
                sharedGFX->MoveSystemLocation(GraphicFXLocation[i] + Location);
                sharedGFX->RotateSystemByAngle(Rotation);
            }
        }
    }
}

} // astromenace namespace
} // viewizard namespace
