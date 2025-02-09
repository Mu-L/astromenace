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

// TODO "AsteroidField" should be moved to separate sources (gfx?)

// TODO "MissionCompleteAtNoEnemy" related code should be moved to propriate code (game?)

// NOTE in future, use make_unique() to make unique_ptr-s (since C++14)

/*
OpenGL use right-handed coordinate system, that should be used in scripts
for object positioning too.

Scene axises orientation:

     ^ Y
     |
     |  / Z
     | /
<-----/
X

X = 0 at center of the scene.
Y = 0 at player ship movement plane.
Z = 0 at the level start, and increased during level.
*/

#include "../core/core.h"
#include "script.h"
#include "../config/config.h"
#include "../ui/game/stopwatch.h"
#include "../assets/audio.h"
#include "../gfx/star_system.h"
#include "../game/camera.h"
#include "../object3d/object3d.h"
#include "../object3d/space_ship/space_ship.h"
#include "../object3d/ground_object/ground_object.h"
#include "../object3d/space_ship/space_ship.h"
#include "../object3d/space_object/space_object.h"
#include "../object3d/ground_object/ground_object.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

// FIXME should be fixed, don't allow global scope interaction for local variables
extern std::weak_ptr<cSpaceShip> PlayerFighter;
// FIXME should be fixed, use 'include' instead
void SetGameMissionComplete();


/*
 * Set TimeSheet's AI_Mode.
 */
static void SetAIMode(std::list<sTimeSheet> &TimeSheetList, const sXMLEntry &xmlEntry,
                      const std::unique_ptr<cXMLDocument> &xmlDoc, const std::shared_ptr<cXMLDocument> &xmlAI)
{
    int tmpAI_Mode{0};
    if (xmlDoc->iGetEntryAttribute(xmlEntry, "aimode", tmpAI_Mode)) {
        TimeSheetList.emplace_back();
        TimeSheetList.back().Time = -1;
        TimeSheetList.back().AI_Mode = tmpAI_Mode;
        TimeSheetList.back().xmlAI = xmlAI;
    }
}

/*
 * Set object location.
 */
static void SetLocation(cObject3D &Object, const sXMLEntry &xmlEntry,
                        const std::unique_ptr<cXMLDocument> &xmlDoc, float TimeOpLag)
{
    sVECTOR3D tmpPosition(0.0f, 0.0f, 0.0f);

    // absolute coordinates
    xmlDoc->fGetEntryAttribute(xmlEntry, "posax", tmpPosition.x);
    xmlDoc->fGetEntryAttribute(xmlEntry, "posay", tmpPosition.y);
    xmlDoc->fGetEntryAttribute(xmlEntry, "posaz", tmpPosition.z);

    // camera-related coordinates
    sVECTOR3D PosWithLag(0.0f,0.0f,0.0f);
    PosWithLag = GetCameraMovementDirection() ^ (-GetCameraSpeed() * TimeOpLag);
    if (xmlDoc->fGetEntryAttribute(xmlEntry, "posx", tmpPosition.x)) {
        tmpPosition.x += GetCameraCoveredDistance().x + PosWithLag.x;
    }
    if (xmlDoc->fGetEntryAttribute(xmlEntry, "posy", tmpPosition.y)) {
        tmpPosition.y += GetCameraCoveredDistance().y + PosWithLag.y;
    }
    if (xmlDoc->fGetEntryAttribute(xmlEntry, "posz", tmpPosition.z)) {
        tmpPosition.z += GetCameraCoveredDistance().z + PosWithLag.z;
    }

    Object.SetLocation(tmpPosition);
}

/*
 * Set object rotation.
 */
static void SetRotation(cObject3D &Object, const sXMLEntry &xmlEntry,
                        const std::unique_ptr<cXMLDocument> &xmlDoc)
{
    sVECTOR3D tmpAngle(0.0f, 0.0f, 0.0f);
    xmlDoc->fGetEntryAttribute(xmlEntry, "anglax", tmpAngle.x);
    xmlDoc->fGetEntryAttribute(xmlEntry, "anglay", tmpAngle.y);
    xmlDoc->fGetEntryAttribute(xmlEntry, "anglaz", tmpAngle.z);

    // absolute and camera-related angles are the same for now
    xmlDoc->fGetEntryAttribute(xmlEntry, "anglx", tmpAngle.x);
    xmlDoc->fGetEntryAttribute(xmlEntry, "angly", tmpAngle.y);
    xmlDoc->fGetEntryAttribute(xmlEntry, "anglz", tmpAngle.z);

    Object.SetRotation(tmpAngle);
}

/*
 * Set object's DeleteAfterLeaveScene field.
 */
static void SetDeleteAfterLeaveScene(cObject3D &Object, const sXMLEntry &xmlEntry,
                                     const std::unique_ptr<cXMLDocument> &xmlDoc)
{
    Object.DeleteAfterLeaveScene = eDeleteAfterLeaveScene::enabled;
    int tmpStatus{0};
    if (xmlDoc->iGetEntryAttribute(xmlEntry, "onhide", tmpStatus) && tmpStatus <= 0) {
        Object.DeleteAfterLeaveScene = eDeleteAfterLeaveScene::disabled;
    }
}

/*
 * Set object's DebugInfo field.
 */
#ifdef NDEBUG
static void SetDebugInformation([[gnu::unused, maybe_unused]] cObject3D &Object,
                                [[gnu::unused, maybe_unused]] const sXMLEntry &xmlEntry,
                                [[gnu::unused, maybe_unused]] bool ShowLineNumber)
{
    return;
}
#else
static void SetDebugInformation(cObject3D &Object, const sXMLEntry &xmlEntry, bool ShowLineNumber)
{
    if (!ShowLineNumber) {
        return;
    }

    Object.ScriptLineNumberUTF32 = ConvertUTF8.from_bytes(std::to_string(xmlEntry.LineNumber).data());
}
#endif // NDEBUG

/*
 * Load and run script.
 */
bool cMissionScript::RunScript(const std::string &FileName, float InitTime)
{
    if (FileName.empty()) {
        return false;
    }

    StartTime = TimeLastOp = InitTime;

    TimeOpLag = 0;

    NeedCheckSpaceShip = false;
    NeedCheckGroundObject = false;
    EndDelayMissionComplete = 0.0f;
    LastTimeMissionComplete = -1.0f;

    AsterQuant = 2.0f;
    AsterW = 280.0f;
    AsterH = 7.5f;
    AsterXPos = 0.0f;
    AsterYPos = -10.0f;
    AsterZPos = 340.0f;
    AsterRealNeed = 0.0f;
    AsterMaxSpeed = 5.0f;
    AsterMinFastSpeed = 35.0f;
    AsterFastCount = 0;
    AsterOn = false;
    AsterLastTime = -1.0f;

    ShowLineNumber = false;
    SetObjectsBBRenderMode(eRenderBoundingBoxes::None);

    xmlDoc.reset(new cXMLDocument(FileName, true));

    if (!xmlDoc->GetRootEntry() || xmlDoc->GetRootEntry()->Name != "AstroMenaceScript") {
        std::cerr << __func__ << "(): " << "Can't find AstroMenaceScript element in the: " << FileName << "\n";
        xmlDoc.reset();
        return false;
    }

    if (xmlDoc->GetRootEntry()->ChildrenList.empty()) {
        std::cerr << __func__ << "(): " << "Can't find element's children in the: " << FileName << "\n";
        xmlDoc.reset();
        return false;
    }

    xmlEntryIter = xmlDoc->GetRootEntry()->ChildrenList.begin();

    // check all tags for 0 time
    Update(StartTime);
    return true;
}

/*
 * Update script status.
 */
bool cMissionScript::Update(float Time)
{
    if (!xmlDoc) {
        return false;
    }

    float TimeDelta = Time - TimeLastOp;

    // in case of "AsteroidField", we need generate asteroid field
    if (AsterOn) {
        if (AsterLastTime == -1.0f) {
            AsterLastTime = Time;
        }
        float AsterTimeDelta = Time - AsterLastTime;
        AsterLastTime = Time;

        float NeedGener = AsterQuant * AsterTimeDelta + AsterRealNeed;
        unsigned NeedGenerInt = static_cast<unsigned>(NeedGener);
        AsterRealNeed = NeedGener - static_cast<float>(NeedGenerInt);

        while (NeedGenerInt > 0) {
            std::weak_ptr<cSpaceObject> CreateAsteroid = CreateSmallAsteroid();
            if (auto sharedCreateAsteroid = CreateAsteroid.lock()) {
                if (AsterFastCount != 20) {
                    sharedCreateAsteroid->Speed = AsterMaxSpeed * vw_fRand();
                } else {
                    sharedCreateAsteroid->Speed = AsterMinFastSpeed + AsterMaxSpeed * vw_fRand();
                }
                sharedCreateAsteroid->DeleteAfterLeaveScene = eDeleteAfterLeaveScene::enabled;
                sharedCreateAsteroid->SetRotation(sVECTOR3D{0.0f, 180.0f, 0.0f});

                if (AsterFastCount != 20) {
                    sharedCreateAsteroid->SetLocation(sVECTOR3D{AsterXPos + AsterW * vw_fRand0(),
                                                      AsterYPos * 2 + AsterH * vw_fRand(),
                                                      AsterZPos + 20.0f} +
                                                      GetCameraCoveredDistance());
                } else {
                    sharedCreateAsteroid->SetLocation(sVECTOR3D{AsterXPos + AsterW * vw_fRand0(),
                                                      AsterYPos * 2 + AsterH * vw_fRand(),
                                                      AsterZPos} +
                                                      GetCameraCoveredDistance());
                }
            }
            NeedGenerInt--;
        }

        AsterFastCount++;
        if (AsterFastCount > 30) {
            AsterFastCount = 0;
        }
    }

    // in case of "MissionCompleteAtNoEnemy" we should check enemy on the field
    if (EndDelayMissionComplete > 0.0f || NeedCheckSpaceShip || NeedCheckGroundObject) {
        if (LastTimeMissionComplete == -1.0) {
            LastTimeMissionComplete = Time;
        }
        float TimeDeltaMissionComplete = Time - LastTimeMissionComplete;
        LastTimeMissionComplete = Time;

        bool NeedDecreaseTime{true};

        if (NeedCheckSpaceShip) {
            ForEachSpaceShip([&NeedDecreaseTime] (const cSpaceShip &tmpShip, eShipCycle &Command) {
                if (tmpShip.ObjectStatus == eObjectStatus::Enemy
                    && tmpShip.DeleteAfterLeaveScene != eDeleteAfterLeaveScene::enabled
                    && tmpShip.DeleteAfterLeaveScene != eDeleteAfterLeaveScene::disabled) {
                    NeedDecreaseTime = false;
                    Command = eShipCycle::Break;
                }
            });
        }
        if (NeedDecreaseTime && NeedCheckGroundObject) {
            ForEachGroundObject([&NeedDecreaseTime] (const cGroundObject &tmpGround, eGroundCycle &Command) {
                if (NeedCheckCollision(tmpGround)
                    && tmpGround.DeleteAfterLeaveScene != eDeleteAfterLeaveScene::enabled
                    && tmpGround.DeleteAfterLeaveScene != eDeleteAfterLeaveScene::disabled) {
                    NeedDecreaseTime = false;
                    Command = eGroundCycle::Break;
                }
            });
        }

        if (NeedDecreaseTime) {
            EndDelayMissionComplete -= TimeDeltaMissionComplete;
        }

        if (EndDelayMissionComplete <= 0.0f) {
            EndDelayMissionComplete = 0.0f;
            NeedCheckSpaceShip = false;
            NeedCheckGroundObject = false;
            SetGameMissionComplete();
            return false;
        }

        return true;
    }

    // we don't check GetRootEntry() result, since we checked it in RunScript()
    // constexpr_hash_djb2a() could generate error at compile-time, make sure hashes is unique
    for (; xmlEntryIter != xmlDoc->GetRootEntry()->ChildrenList.end(); ++xmlEntryIter) {
        sXMLEntry &xmlEntry = *xmlEntryIter;
        switch (xmlEntry.NameHash) {
        case constexpr_hash_djb2a("TimeLine"): {
            float onTime{0.0f};
            xmlDoc->fGetEntryAttribute(xmlEntry, "value", onTime);

            if (onTime > TimeDelta) {
                return true;
            }

            TimeOpLag = TimeDelta - onTime;
            UpdateTimeLine();

            TimeLastOp = Time - TimeOpLag;
            TimeDelta = TimeOpLag;
        }
        break;

        case constexpr_hash_djb2a("AIFile"):
            if (!xmlEntry.Content.empty()) {
                xmlAI.reset(new cXMLDocument(xmlEntry.Content));
                if (!xmlAI->GetRootEntry()) {
                    xmlAI.reset();
                    std::cerr << __func__ << "(): " << "AIFile not loaded.\n";
                }
            }
            break;

        case constexpr_hash_djb2a("Debug"): {
            ShowLineNumber = false;
            xmlDoc->bGetEntryAttribute(xmlEntry, "showline", ShowLineNumber);

            {
                int tmpBBRenderMode{0};
                if (xmlDoc->iGetEntryAttribute(xmlEntry, "showbb", tmpBBRenderMode) && tmpBBRenderMode >= 0) {
                    SetObjectsBBRenderMode(static_cast<eRenderBoundingBoxes>(tmpBBRenderMode));
                }
            }

            bool tmpStopwatchStatus{false};
            xmlDoc->bGetEntryAttribute(xmlEntry, "time", tmpStopwatchStatus);
            cStopwatch::GetInstance().Reset(tmpStopwatchStatus);
        }
        break;

        case constexpr_hash_djb2a("StarSystem"): {
            int SystemNum{0};
            if (xmlDoc->iGetEntryAttribute(xmlEntry, "system", SystemNum)) {
                sVECTOR3D TmpBaseRotation(0.0f, 0.0f, 0.0f);
                xmlDoc->fGetEntryAttribute(xmlEntry, "anglex", TmpBaseRotation.x);
                xmlDoc->fGetEntryAttribute(xmlEntry, "angley", TmpBaseRotation.y);
                xmlDoc->fGetEntryAttribute(xmlEntry, "anglez", TmpBaseRotation.z);
                StarSystemInit(SystemNum, TmpBaseRotation);
            }
        }
        break;

        case constexpr_hash_djb2a("LayersTransp"): {
            float Layer1TranspStart{0.2f};
            float Layer1TranspEnd{0.7f};
            float Layer2TranspStart{0.9f};
            float Layer2TranspEnd{0.7f};

            xmlDoc->fGetEntryAttribute(xmlEntry, "FirstStart", Layer1TranspStart);
            xmlDoc->fGetEntryAttribute(xmlEntry, "FirstEnd", Layer1TranspEnd);
            xmlDoc->fGetEntryAttribute(xmlEntry, "SecondStart", Layer2TranspStart);
            xmlDoc->fGetEntryAttribute(xmlEntry, "SecondEnd", Layer2TranspEnd);

            StarSystemLayer1Transp(Layer1TranspStart, Layer1TranspEnd);
            StarSystemLayer3Transp(Layer2TranspStart, Layer2TranspEnd);
        }
        break;

        case constexpr_hash_djb2a("Music"):
            // change music only if player's ship still alive, otherwise we are playing 'fail' theme
            if (auto sharedPlayerFighter = PlayerFighter.lock()) {
                if (sharedPlayerFighter->ArmorCurrentStatus > 0.0f) {
                    int Theme{0};
                    if (xmlDoc->iGetEntryAttribute(xmlEntry, "theme", Theme)) {
                        if (Theme == 1) {
                            PlayMusicTheme(eMusicTheme::GAME, 2000, 2000);
                        }
                        if (Theme == 2) {
                            PlayMusicTheme(eMusicTheme::BOSS, 1500, 2000);
                        }
                    }
                }
            }
            break;

        case constexpr_hash_djb2a("CreatePlanet"): {
            int tmpType{0};
            if (xmlDoc->iGetEntryAttribute(xmlEntry, "type", tmpType)) {
                std::weak_ptr<cSpaceObject> Planet = CreatePlanet(tmpType);
                if (auto sharedPlanet = Planet.lock()) {
                    SetRotation(*sharedPlanet, xmlEntry, xmlDoc);
                    SetLocation(*sharedPlanet, xmlEntry, xmlDoc, 0.0f);
                    sharedPlanet->DeleteAfterLeaveScene = eDeleteAfterLeaveScene::enabled;
                    xmlDoc->fGetEntryAttribute(xmlEntry, "speed", sharedPlanet->Speed);
                }
            }
        }
        break;

        case constexpr_hash_djb2a("CreatePlanetoid"): {
            int tmpType{0};
            // we could create random planetoid, or provide particular planetoid id
            xmlDoc->iGetEntryAttribute(xmlEntry, "type", tmpType);
            std::weak_ptr<cSpaceObject> Planetoid = CreatePlanetoid(tmpType);
            if (auto sharedPlanetoid = Planetoid.lock()) {
                SetRotation(*sharedPlanetoid, xmlEntry, xmlDoc);
                SetLocation(*sharedPlanetoid, xmlEntry, xmlDoc, 0.0f);
                sharedPlanetoid->DeleteAfterLeaveScene = eDeleteAfterLeaveScene::enabled;
                xmlDoc->fGetEntryAttribute(xmlEntry, "speed", sharedPlanetoid->Speed);
            }
        }
        break;

        case constexpr_hash_djb2a("AsteroidField"):
            xmlDoc->bGetEntryAttribute(xmlEntry, "status", AsterOn);
            xmlDoc->fGetEntryAttribute(xmlEntry, "persec", AsterQuant);
            xmlDoc->fGetEntryAttribute(xmlEntry, "w", AsterW);
            xmlDoc->fGetEntryAttribute(xmlEntry, "h", AsterH);
            xmlDoc->fGetEntryAttribute(xmlEntry, "posx", AsterXPos);
            xmlDoc->fGetEntryAttribute(xmlEntry, "posy", AsterYPos);
            xmlDoc->fGetEntryAttribute(xmlEntry, "posz", AsterZPos);
            xmlDoc->fGetEntryAttribute(xmlEntry, "slow", AsterMaxSpeed);
            xmlDoc->fGetEntryAttribute(xmlEntry, "fast", AsterMinFastSpeed);
            break;

        case constexpr_hash_djb2a("Light"): {
            eLightType LightType{eLightType::Directional};
            int tmpType{0};
            if (xmlDoc->iGetEntryAttribute(xmlEntry, "type", tmpType) && tmpType == 1) {
                LightType = eLightType::Point;
            }

            std::weak_ptr<cLight> NewLight = vw_CreateLight(LightType);
            if (auto sharedLight = NewLight.lock()) {
                xmlDoc->fGetEntryAttribute(xmlEntry, "diffr", sharedLight->Diffuse[0]);
                xmlDoc->fGetEntryAttribute(xmlEntry, "diffg", sharedLight->Diffuse[1]);
                xmlDoc->fGetEntryAttribute(xmlEntry, "diffb", sharedLight->Diffuse[2]);
                xmlDoc->fGetEntryAttribute(xmlEntry, "diffa", sharedLight->Diffuse[3]);

                xmlDoc->fGetEntryAttribute(xmlEntry, "specr", sharedLight->Specular[0]);
                xmlDoc->fGetEntryAttribute(xmlEntry, "specg", sharedLight->Specular[1]);
                xmlDoc->fGetEntryAttribute(xmlEntry, "specb", sharedLight->Specular[2]);
                xmlDoc->fGetEntryAttribute(xmlEntry, "speca", sharedLight->Specular[3]);

                xmlDoc->fGetEntryAttribute(xmlEntry, "ambir", sharedLight->Ambient[0]);
                xmlDoc->fGetEntryAttribute(xmlEntry, "ambig", sharedLight->Ambient[1]);
                xmlDoc->fGetEntryAttribute(xmlEntry, "ambib", sharedLight->Ambient[2]);
                xmlDoc->fGetEntryAttribute(xmlEntry, "ambia", sharedLight->Ambient[3]);

                xmlDoc->fGetEntryAttribute(xmlEntry, "dirx", sharedLight->Direction.x);
                xmlDoc->fGetEntryAttribute(xmlEntry, "diry", sharedLight->Direction.y);
                xmlDoc->fGetEntryAttribute(xmlEntry, "dirz", sharedLight->Direction.z);
                sharedLight->Direction.Normalize();

                xmlDoc->fGetEntryAttribute(xmlEntry, "posx", sharedLight->Location.x);
                xmlDoc->fGetEntryAttribute(xmlEntry, "posy", sharedLight->Location.y);
                xmlDoc->fGetEntryAttribute(xmlEntry, "posz", sharedLight->Location.z);

                xmlDoc->bGetEntryAttribute(xmlEntry, "status", sharedLight->On);
            }
        }
        break;

        case constexpr_hash_djb2a("Label"):
            break;

        case constexpr_hash_djb2a("Goto"): {
            std::string tmpLabel{};
            if (xmlDoc->GetEntryAttribute(xmlEntry, "label", tmpLabel)) {
                // we don't check GetRootEntry() result, since we checked it in RunScript()
                sXMLEntry *tmpCycle = xmlDoc->GetRootEntry();
                for (auto iter = tmpCycle->ChildrenList.begin(); iter != tmpCycle->ChildrenList.end(); ++iter) {
                    sXMLEntry &tmpEntry = *iter;
                    if (tmpEntry.Name == "Label") {
                        std::string tmpName{};
                        if (xmlDoc->GetEntryAttribute(tmpEntry, "name", tmpName) && tmpLabel == tmpName) {
                            xmlEntryIter = iter;
                            return true;
                        }
                    }
                }
            }
        }
        break;

        case constexpr_hash_djb2a("MissionComplete"):
            SetGameMissionComplete();
            break;

        case constexpr_hash_djb2a("MissionCompleteAtNoEnemy"): {
            bool SetGameMissionFlag{false};
            NeedCheckSpaceShip = false;
            if (xmlDoc->bGetEntryAttribute(xmlEntry, "ships", NeedCheckSpaceShip)) {
                SetGameMissionFlag = true;
            }
            NeedCheckGroundObject = false;
            if (xmlDoc->bGetEntryAttribute(xmlEntry, "grounds", NeedCheckGroundObject)) {
                SetGameMissionFlag = true;
            }
            EndDelayMissionComplete = 0.0f;
            if (xmlDoc->fGetEntryAttribute(xmlEntry, "delay", EndDelayMissionComplete)) {
                SetGameMissionFlag = true;
            }

            if (!SetGameMissionFlag) {
                // if no flags setted up, act as "MissionComplete"
                SetGameMissionComplete();
            } else {
                LastTimeMissionComplete = Time;
                return true;
            }
        }
        break;

        default:
            std::cerr << __func__ << "(): " << "tag " << xmlEntry.Name
                      << " not found, line " << xmlEntry.LineNumber << "\n";
            break;
        }
    }

    return false;
}

/*
 * Load TimeSheet related script data.
 */
static void LoadTimeSheetData(cXMLDocument &xmlDoc, const sXMLEntry &XMLEntry,
                              sTimeSheet &TimeSheet, const std::shared_ptr<cXMLDocument> &xmlAI)
{
    if (xmlDoc.iGetEntryAttribute(XMLEntry, "aimode", TimeSheet.AI_Mode)) {
        xmlDoc.fGetEntryAttribute(XMLEntry, "time", TimeSheet.Time);
        TimeSheet.xmlAI = xmlAI;
        return;
    }

    xmlDoc.fGetEntryAttribute(XMLEntry, "time", TimeSheet.Time);

    xmlDoc.fGetEntryAttribute(XMLEntry, "speed", TimeSheet.Speed);
    xmlDoc.fGetEntryAttribute(XMLEntry, "acceler", TimeSheet.Acceler);
    vw_Clamp(TimeSheet.Acceler, 0.0f, 1.0f);

    xmlDoc.fGetEntryAttribute(XMLEntry, "speedlr", TimeSheet.SpeedLR);
    xmlDoc.fGetEntryAttribute(XMLEntry, "accelerlr", TimeSheet.AccelerLR);
    vw_Clamp(TimeSheet.AccelerLR, 0.0f, 1.0f);

    xmlDoc.fGetEntryAttribute(XMLEntry, "speedud", TimeSheet.SpeedUD);
    xmlDoc.fGetEntryAttribute(XMLEntry, "accelerud", TimeSheet.AccelerUD);
    vw_Clamp(TimeSheet.AccelerUD, 0.0f, 1.0f);

    xmlDoc.fGetEntryAttribute(XMLEntry, "speedbycamfb", TimeSheet.SpeedByCamFB);
    xmlDoc.fGetEntryAttribute(XMLEntry, "accelerbycamfb", TimeSheet.AccelerByCamFB);
    vw_Clamp(TimeSheet.AccelerByCamFB, 0.0f, 1.0f);

    xmlDoc.fGetEntryAttribute(XMLEntry, "speedbycamlr", TimeSheet.SpeedByCamLR);
    xmlDoc.fGetEntryAttribute(XMLEntry, "accelerbycamlr", TimeSheet.AccelerByCamLR);
    vw_Clamp(TimeSheet.AccelerByCamLR, 0.0f, 1.0f);

    xmlDoc.fGetEntryAttribute(XMLEntry, "speedbycamud", TimeSheet.SpeedByCamUD);
    xmlDoc.fGetEntryAttribute(XMLEntry, "accelerbycamud", TimeSheet.AccelerByCamUD);
    vw_Clamp(TimeSheet.AccelerByCamUD, 0.0f, 1.0f);

    xmlDoc.fGetEntryAttribute(XMLEntry, "rotx", TimeSheet.Rotation.x);
    xmlDoc.fGetEntryAttribute(XMLEntry, "roty", TimeSheet.Rotation.y);
    xmlDoc.fGetEntryAttribute(XMLEntry, "rotz", TimeSheet.Rotation.z);

    xmlDoc.fGetEntryAttribute(XMLEntry, "rotacx", TimeSheet.RotationAcceler.x);
    xmlDoc.fGetEntryAttribute(XMLEntry, "rotacy", TimeSheet.RotationAcceler.y);
    xmlDoc.fGetEntryAttribute(XMLEntry, "rotacz", TimeSheet.RotationAcceler.z);
    vw_Clamp(TimeSheet.RotationAcceler.x, 0.0f, 1.0f);
    vw_Clamp(TimeSheet.RotationAcceler.y, 0.0f, 1.0f);
    vw_Clamp(TimeSheet.RotationAcceler.z, 0.0f, 1.0f);
    if (xmlDoc.fGetEntryAttribute(XMLEntry, "anglx", TimeSheet.SetAngle.x)) {
        TimeSheet.NeedSetAngle = true;
    }
    if (xmlDoc.fGetEntryAttribute(XMLEntry, "angly", TimeSheet.SetAngle.y)) {
        TimeSheet.NeedSetAngle = true;
    }
    if (xmlDoc.fGetEntryAttribute(XMLEntry, "anglz", TimeSheet.SetAngle.z)) {
        TimeSheet.NeedSetAngle = true;
    }

    int tmpFire{0};
    if (xmlDoc.iGetEntryAttribute(XMLEntry, "fire", tmpFire) && tmpFire > 0) {
        TimeSheet.Fire = true;
    }

    int tmpBossFire{0};
    if (xmlDoc.iGetEntryAttribute(XMLEntry, "bossfire", tmpBossFire) && tmpBossFire > 0) {
        TimeSheet.BossFire = true;
    }

    int tmpTargeting{0};
    if (xmlDoc.iGetEntryAttribute(XMLEntry, "targeting", tmpTargeting) && tmpTargeting != 0) {
        TimeSheet.Targeting = true;
    }
}

/*
 * Load SpaceShip related script data.
 */
static void LoadSpaceShipScript(std::weak_ptr<cSpaceShip> &SpaceShip, const std::unique_ptr<cXMLDocument> &xmlDoc,
                                const sXMLEntry &xmlEntry, bool ShowLineNumber, float TimeOpLag,
                                const std::shared_ptr<cXMLDocument> &xmlAI)
{
    auto sharedSpaceShip = SpaceShip.lock();
    if (!sharedSpaceShip) {
        return;
    }

    if (ShowLineNumber) {
        SetDebugInformation(*sharedSpaceShip, xmlEntry, ShowLineNumber);
    }

    if (xmlDoc->fGetEntryAttribute(xmlEntry, "speed", sharedSpaceShip->NeedSpeed)) {
        sharedSpaceShip->Speed = sharedSpaceShip->NeedSpeed;
    }
    if (xmlDoc->fGetEntryAttribute(xmlEntry, "speedlr", sharedSpaceShip->NeedSpeedLR)) {
        sharedSpaceShip->SpeedLR = sharedSpaceShip->NeedSpeedLR;
    }
    if (xmlDoc->fGetEntryAttribute(xmlEntry, "speedud", sharedSpaceShip->NeedSpeedUD)) {
        sharedSpaceShip->SpeedUD = sharedSpaceShip->NeedSpeedUD;
    }

    if (xmlDoc->fGetEntryAttribute(xmlEntry, "speedbycamfb", sharedSpaceShip->NeedSpeedByCamFB)) {
        sharedSpaceShip->SpeedByCamFB = sharedSpaceShip->NeedSpeedByCamFB;
    }
    if (xmlDoc->fGetEntryAttribute(xmlEntry, "speedbycamlr", sharedSpaceShip->NeedSpeedByCamLR)) {
        sharedSpaceShip->SpeedByCamLR = sharedSpaceShip->NeedSpeedByCamLR;
    }
    if (xmlDoc->fGetEntryAttribute(xmlEntry, "speedbycamud", sharedSpaceShip->NeedSpeedByCamUD)) {
        sharedSpaceShip->SpeedByCamUD = sharedSpaceShip->NeedSpeedByCamUD;
    }

    SetDeleteAfterLeaveScene(*sharedSpaceShip, xmlEntry, xmlDoc);
    SetAIMode(sharedSpaceShip->TimeSheetList, xmlEntry, xmlDoc, xmlAI);
    SetRotation(*sharedSpaceShip, xmlEntry, xmlDoc);
    SetLocation(*sharedSpaceShip, xmlEntry, xmlDoc, TimeOpLag);

    for (const auto &tmpXMLEntry : xmlEntry.ChildrenList) {
        if (tmpXMLEntry.Name == "TimeSheet") {
            sharedSpaceShip->TimeSheetList.emplace_back();
            LoadTimeSheetData(*xmlDoc.get(), tmpXMLEntry, sharedSpaceShip->TimeSheetList.back(), xmlAI);
        }
    }
}

/*
 * Load GroundObject related script data.
 */
static void LoadGroundObjectScript(std::weak_ptr<cGroundObject> &GroundObject,
                                   const std::unique_ptr<cXMLDocument> &xmlDoc,
                                   const sXMLEntry &xmlEntry, bool ShowLineNumber, float TimeOpLag,
                                   const std::shared_ptr<cXMLDocument> &xmlAI)
{
    auto sharedGroundObject = GroundObject.lock();
    if (!sharedGroundObject) {
        return;
    }

    if (ShowLineNumber) {
        SetDebugInformation(*sharedGroundObject, xmlEntry, ShowLineNumber);
    }
    if (xmlDoc->fGetEntryAttribute(xmlEntry, "speed", sharedGroundObject->NeedSpeed)) {
        sharedGroundObject->Speed = sharedGroundObject->NeedSpeed;
    }

    SetDeleteAfterLeaveScene(*sharedGroundObject, xmlEntry, xmlDoc);
    SetAIMode(sharedGroundObject->TimeSheetList, xmlEntry, xmlDoc, xmlAI);
    SetRotation(*sharedGroundObject, xmlEntry, xmlDoc);
    SetLocation(*sharedGroundObject, xmlEntry, xmlDoc, TimeOpLag);

    for (const auto &tmpXMLEntry : xmlEntry.ChildrenList) {
        if (tmpXMLEntry.Name == "TimeSheet") {
            sharedGroundObject->TimeSheetList.emplace_back();
            LoadTimeSheetData(*xmlDoc.get(), tmpXMLEntry, sharedGroundObject->TimeSheetList.back(), xmlAI);
        }
    }
}

/*
 * Load SpaceObject related script data.
 */
static void LoadSpaceObjectScript(std::weak_ptr<cSpaceObject> &SpaceObject,
                                  const std::unique_ptr<cXMLDocument> &xmlDoc,
                                  const sXMLEntry &xmlEntry, bool ShowLineNumber, float TimeOpLag)
{
    auto sharedSpaceObject = SpaceObject.lock();
    if (!sharedSpaceObject) {
        return;
    }

    if (ShowLineNumber) {
        SetDebugInformation(*sharedSpaceObject, xmlEntry, ShowLineNumber);
    }
    xmlDoc->fGetEntryAttribute(xmlEntry, "speed", sharedSpaceObject->Speed);
    SetDeleteAfterLeaveScene(*sharedSpaceObject, xmlEntry, xmlDoc);

    SetRotation(*sharedSpaceObject, xmlEntry, xmlDoc);
    SetLocation(*sharedSpaceObject, xmlEntry, xmlDoc, TimeOpLag);

    xmlDoc->fGetEntryAttribute(xmlEntry, "rotx", sharedSpaceObject->RotationSpeed.x);
    xmlDoc->fGetEntryAttribute(xmlEntry, "roty", sharedSpaceObject->RotationSpeed.y);
    xmlDoc->fGetEntryAttribute(xmlEntry, "rotz", sharedSpaceObject->RotationSpeed.z);
}

/*
 * Update TimeLine.
 */
void cMissionScript::UpdateTimeLine()
{
    for (auto &TL : xmlEntryIter->ChildrenList) {

        int tmpType{0};

        // constexpr_hash_djb2a() could generate error at compile-time, make sure hashes is unique
        switch (TL.NameHash) {
        case constexpr_hash_djb2a("EarthFighter"):
            if (xmlDoc->iGetEntryAttribute(TL, "type", tmpType)) {
                std::weak_ptr<cSpaceShip> SpaceShip = CreateEarthSpaceFighter(tmpType);
                LoadSpaceShipScript(SpaceShip, xmlDoc, TL, ShowLineNumber, TimeOpLag, xmlAI);

                int tmpInteger{0};
                if (xmlDoc->iGetEntryAttribute(TL, "armor", tmpInteger)) {
                    SetEarthSpaceFighterArmor(SpaceShip, tmpInteger);
                }

                for (unsigned int i = 1; i <= config::MAX_WEAPONS; i++) {
                    if (xmlDoc->iGetEntryAttribute(TL, "weapon" + std::to_string(i), tmpInteger)) {
                        SetEarthSpaceFighterWeapon(SpaceShip, i, tmpInteger);
                    }
                }
            }
            break;

        case constexpr_hash_djb2a("AlienFighter"):
            if (xmlDoc->iGetEntryAttribute(TL, "type", tmpType)) {
                std::weak_ptr<cSpaceShip> SpaceShip = CreateAlienSpaceFighter(tmpType);
                LoadSpaceShipScript(SpaceShip, xmlDoc, TL, ShowLineNumber, TimeOpLag, xmlAI);
            }
            break;

        case constexpr_hash_djb2a("AlienMotherShip"):
            if (xmlDoc->iGetEntryAttribute(TL, "type", tmpType)) {
                std::weak_ptr<cSpaceShip> SpaceShip = CreateAlienSpaceMotherShip(tmpType);
                LoadSpaceShipScript(SpaceShip, xmlDoc, TL, ShowLineNumber, TimeOpLag, xmlAI);
            }
            break;

        case constexpr_hash_djb2a("PirateShip"):
            if (xmlDoc->iGetEntryAttribute(TL, "type", tmpType)) {
                std::weak_ptr<cSpaceShip> SpaceShip = CreatePirateShip(tmpType);
                LoadSpaceShipScript(SpaceShip, xmlDoc, TL, ShowLineNumber, TimeOpLag, xmlAI);
            }
            break;

        case constexpr_hash_djb2a("CreateAsteroid"): {
            std::weak_ptr<cSpaceObject> SpaceObject = CreateSmallAsteroid();
            LoadSpaceObjectScript(SpaceObject, xmlDoc, TL, ShowLineNumber, TimeOpLag);
        }
        break;

        case constexpr_hash_djb2a("CreateBasePart"):
            if (xmlDoc->iGetEntryAttribute(TL, "type", tmpType)) {
                std::weak_ptr<cSpaceObject> SpaceObject = CreateBasePart(tmpType);
                LoadSpaceObjectScript(SpaceObject, xmlDoc, TL, ShowLineNumber, TimeOpLag);
            }
            break;

        case constexpr_hash_djb2a("CreateBigAsteroid"): {
            // we could create random asteroid, or provide particular asteroid id
            xmlDoc->iGetEntryAttribute(TL, "type", tmpType);
            std::weak_ptr<cSpaceObject> SpaceObject = CreateBigAsteroid(tmpType);
            LoadSpaceObjectScript(SpaceObject, xmlDoc, TL, ShowLineNumber, TimeOpLag);
            break;
        }

        case constexpr_hash_djb2a("CreateMBuilding"):
            if (xmlDoc->iGetEntryAttribute(TL, "type", tmpType)) {
                std::weak_ptr<cGroundObject> GroundObject = CreateMilitaryBuilding(tmpType);
                LoadGroundObjectScript(GroundObject, xmlDoc, TL, ShowLineNumber, TimeOpLag, xmlAI);
            }
            break;

        case constexpr_hash_djb2a("CreateBuilding"):
            if (xmlDoc->iGetEntryAttribute(TL, "type", tmpType)) {
                std::weak_ptr<cGroundObject> GroundObject = CreateCivilianBuilding(tmpType);
                LoadGroundObjectScript(GroundObject, xmlDoc, TL, ShowLineNumber, TimeOpLag, xmlAI);
            }
            break;

        case constexpr_hash_djb2a("CreateTracked"):
            if (xmlDoc->iGetEntryAttribute(TL, "type", tmpType)) {
                std::weak_ptr<cGroundObject> GroundObject = CreateTracked(tmpType);
                LoadGroundObjectScript(GroundObject, xmlDoc, TL, ShowLineNumber, TimeOpLag, xmlAI);
            }
            break;

        case constexpr_hash_djb2a("CreateWheeled"):
            if (xmlDoc->iGetEntryAttribute(TL, "type", tmpType)) {
                std::weak_ptr<cGroundObject> GroundObject = CreateWheeled(tmpType);
                LoadGroundObjectScript(GroundObject, xmlDoc, TL, ShowLineNumber, TimeOpLag, xmlAI);
            }
            break;

        default:
            std::cerr << __func__ << "(): " << "tag " << TL.Name
                      << " not found, line " << TL.LineNumber << "\n";
            break;
        }
    }
}

/*
 * Unpack TimeSheet to the list.
 */
void UnpackAIMode(std::list<sTimeSheet> &TimeSheetList)
{
    auto iter = TimeSheetList.begin();

    for (auto &xmlEntry : TimeSheetList.front().xmlAI->GetRootEntry()->ChildrenList) {
        int tmpAI_Mode{0};
        if (TimeSheetList.front().xmlAI->iGetEntryAttribute(xmlEntry, "num", tmpAI_Mode)
            && tmpAI_Mode == TimeSheetList.front().AI_Mode) {

            // "unpack" all the elements
            for (auto &tmpChildEntry : xmlEntry.ChildrenList) {
                if (tmpChildEntry.Name == "TimeSheet") {
                    iter = TimeSheetList.emplace(++iter);
                    LoadTimeSheetData(*TimeSheetList.front().xmlAI.get(), tmpChildEntry,
                                      *iter, TimeSheetList.front().xmlAI);
                }
            }

            // for cycled, create duplicate at the end of unpacked elements
            if (TimeSheetList.front().Time == -1.0f) {
                TimeSheetList.emplace(++iter, TimeSheetList.front());
            }

            // since we already unpack this entry, remove it
            TimeSheetList.pop_front();
            return;
        }
    }

    std::cerr << __func__ << "(): " << "AI_Mode " << TimeSheetList.front().AI_Mode << " not found.\n";
}

} // astromenace namespace
} // viewizard namespace
