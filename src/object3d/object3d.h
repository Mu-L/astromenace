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

// TODO as soon, as cSpaceShip will be moved to STL usage with weapon (shared_ptr?),
//      switch to eDeleteAfterLeaveScene::enabled by default and remove SetDeleteAfterLeaveScene()
//      also explosions creation code should be corrected (remove DeleteAfterLeaveScene setup)

// TODO in case DeleteAfterLeaveScene is 'enabled', also should be limited by time
//      if object was never shown on the scene (during this time), should be deleted + warning output

// TODO check 'virtual' usage here, do we really need this class functions override?

#ifndef OBJECT3D_OBJECT3D_H
#define OBJECT3D_OBJECT3D_H

#include "../core/core.h"
#include "../enum.h"
#include "../script/script.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

// FIXME should be fixed, don't allow global scope interaction for local variables
extern int GameEnemyWeaponPenalty;
extern int GameEnemyArmorPenalty;
extern int GameEnemyTargetingSpeedPenalty;
extern int GameUnlimitedAmmo;
extern int GameUndestroyableWeapon;
extern int GameWeaponTargetingMode;
extern int GameSpaceShipControlMode;
extern int GameEngineSystem;
extern int GameTargetingSystem;
extern int GameAdvancedProtectionSystem;
extern int GameTargetingMechanicSystem;

enum class eObjectStatus {
    none,
    Enemy,
    Ally,
    Player
};

inline bool ObjectsStatusFoe(eObjectStatus Object1, eObjectStatus Object2)
{
    return ((Object1 == eObjectStatus::Enemy) && ((Object2 == eObjectStatus::Ally) || (Object2 == eObjectStatus::Player))) ||
           ((Object2 == eObjectStatus::Enemy) && ((Object1 == eObjectStatus::Ally) || (Object1 == eObjectStatus::Player)));
}

enum class eObjectType {
    none,
    EarthFighter,
    AlienFighter,
    AlienMotherShip,
    PirateShip,
    PirateVehicle,
    PirateBuilding,
    SmallAsteroid,
    SpaceDebris,
    ShipWeapon,
    Projectile,
    Explosion,
    CivilianBuilding,
    BasePart,
    Planet,
    Planetoid,
    BigAsteroid
    // if you change this enum, make sure that AddBonusForKilledEnemy() also reflect this changes
};

enum class eDeleteAfterLeaveScene {
    disabled,
    enabled,        // waiting, object should being shown on the scene first
    showed,         // object shown, waiting when it will be out of the scene
    need_delete,    // object shown and out of the scene, should be deleted
    wait_delay      // will be deleted after delay
};
// delay before object delete, since object could back to the scene
constexpr float DeleteAfterLeaveSceneDelay{1.0f};

/*
For now we have 2 types of damage:
Kinetic - good for armor and shield, stable damage value;
EM (electromagnetic) - good for shield (stable damage value), have a chance to kill alien (energy creature)
   by one hit, have a chance to damage armor (random damage value), have a chance to disable some internal system;
*/
// TODO probably, we also need "Explosion" damage type for shock wave, that now connected to 75% of Kinetic damage
class cDamage {
private:
    float Kinetic_{0.0f};
    float EM_{0.0f}; // electromagnetic

public:
    cDamage() = default;
    explicit cDamage(float Value) :
        Kinetic_{Value},
        EM_{Value}
    {}
    explicit cDamage(float Kinetic, float EM) :
        Kinetic_{Kinetic},
        EM_{EM}
    {}

    cDamage& operator = (float Value)
    {
        Kinetic_ = Value;
        EM_ = Value;
        return *this;
    }

    cDamage& operator /= (float Value)
    {
        Kinetic_ /= Value;
        EM_ /= Value;
        return *this;
    }

    cDamage& operator *= (float Value)
    {
        Kinetic_ *= Value;
        EM_ *= Value;
        return *this;
    }

    float Kinetic() const
    {
        return Kinetic_;
    }

    float &Kinetic()
    {
        return Kinetic_;
    }

    float EM() const
    {
        return EM_;
    }

    float &EM()
    {
        return EM_;
    }

    float Full() const
    {
        return Kinetic_ + EM_;
    }
};

inline cDamage operator * (const cDamage &Damage, float Value)
{
    return cDamage{Damage.Kinetic() * Value, Damage.EM() * Value};
}

class cObject3D : public sModel3D {
protected:
    // don't allow object of this class creation
    [[gnu::noinline, clang::noinline, msvc::noinline]]
    cObject3D() = default;
    [[gnu::noinline, clang::noinline, msvc::noinline]]
    ~cObject3D() = default;

    // should be called in UpdateWithTimeSheetList() only
    virtual bool Update(float Time);

public:
    virtual void Draw(bool VertexOnlyPass, bool ShadowMap = false);
    bool NeedCullFaces{true};
    bool NeedAlphaTest{false};
    bool UpdateWithTimeSheetList(float Time);

    void SetChunkLocation(const sVECTOR3D &NewLocation, unsigned ChunkNum);
    void SetChunkRotation(const sVECTOR3D &NewRotation, unsigned ChunkNum);
    virtual void SetLocation(const sVECTOR3D &NewLocation);
    virtual void SetRotation(const sVECTOR3D &NewRotation);

    // in-game object's status relatively to player
    eObjectStatus ObjectStatus{eObjectStatus::none};
    // global object type
    eObjectType ObjectType{eObjectType::none};
    // internal object's type for objects with same ObjectType, usually, same as creation type (num)
    int InternalType{0};

    // in case we need show object and delete after it leave scene (after DeleteAfterLeaveSceneDelay time)
    eDeleteAfterLeaveScene DeleteAfterLeaveScene{eDeleteAfterLeaveScene::disabled};
    // note, Lifetime could be changed by DeleteAfterLeaveScene settings
    float Lifetime{-1.0f};

    sVECTOR3D Orientation{0.0f, 0.0f, 1.0f};
    sVECTOR3D Rotation{0.0f, 0.0f, 0.0f};
    sVECTOR3D OldRotationInv{0.0f, 0.0f, 0.0f};
    sVECTOR3D Location{0.0f, 0.0f, 0.0f};
    sVECTOR3D PrevLocation{0.0f, 0.0f, 0.0f};

    float TimeLastUpdate{-1.0f};
    float TimeDelta{0.0f};

    std::vector<GLtexture> Texture{};
    std::vector<GLtexture> TextureIllum{};
    std::vector<GLtexture> NormalMap{};

    float PromptDrawDist2{-1.0f}; // LOD related
    int InternalLights{0};

    // material related
    float Diffuse[4]{1.0f, 1.0f, 1.0f, 1.0f};
    float Specular[4]{1.0f, 1.0f, 1.0f, 1.0f};
    float Ambient[4]{0.1f, 0.1f, 0.1f, 0.1f};
    float Power[1]{64.0f};

    float ArmorCurrentStatus{0.0f};
    float ArmorInitialStatus{0.0f};

    float ShieldCurrentStatus{0.0f};
    float ShieldInitialStatus{0.0f};
    float ShieldRechargeRate{0.0f};

    bool ShowStatus{true};
    bool ShowStatusAllTime{false};

    float CurrentRotationMat[9]{1.0f, 0.0f, 0.0f,
                                0.0f, 1.0f, 0.0f,
                                0.0f, 0.0f, 1.0f};
    float OldInvRotationMat[9]{1.0f, 0.0f, 0.0f,
                               0.0f, 1.0f, 0.0f,
                               0.0f, 0.0f, 1.0f};

    std::u32string ScriptLineNumberUTF32{}; // debug info, line number in script file

    std::list<sTimeSheet> TimeSheetList{};

    // GLSL-related
    int ShaderType{1};
    float ExplShaderSpeedDelta{0.0f};
    float ExplShaderRangeFactor{0.0f};
};


// we need fixed integers here, since we use them in scripts
enum class eRenderBoundingBoxes : int {
    None = 0,
    AABB_Only = 1,
    AABB_And_OBB = 2,
    All = 3 // AABB, OBB, HitBB
};

// Set bounding boxes render mode.
void SetObjectsBBRenderMode(eRenderBoundingBoxes Mode);


/*
 * object3d_manager
 */

// Draw all oblect3d.
void DrawAllObject3D(eDrawType DrawType);
// Update all oblect3d.
void UpdateAllObject3D(float Time);
// Release all oblect3d.
void ReleaseAllObject3D();

/*
 * object3d_collision
 */

// Check collision for all objects
void DetectCollisionAllObject3D();

/*
 * object3d_functions
 */

// Check for "mortal" objects, that could be used for collision detection.
bool NeedCheckCollision(const cObject3D &Object3D);
// Load 3D model data.
void LoadObjectData(unsigned FileNameHash, cObject3D &Object3D);
// Setup shaders.
bool SetupObject3DShaders();

} // astromenace namespace
} // viewizard namespace

#endif // OBJECT3D_OBJECT3D_H
