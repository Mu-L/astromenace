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

#include "space_object.h"
#include "../explosion/explosion.h"
#include "../../assets/texture.h"
#include "../../game/camera.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

namespace {

// all space object list
std::list<std::shared_ptr<cSpaceObject>> SpaceObjectList{};

} // unnamed namespace


/*
 * Create cSmallAsteroid object.
 */
std::weak_ptr<cSpaceObject> CreateSmallAsteroid()
{
    // NOTE emplace_front() return reference to the inserted element (since C++17)
    //      this two lines could be combined
    SpaceObjectList.emplace_front(new cSmallAsteroid, [](cSmallAsteroid *p) {delete p;});
    return SpaceObjectList.front();
}

/*
 * Create cBigAsteroid object.
 */
std::weak_ptr<cSpaceObject> CreateBigAsteroid(const int AsteroidNum)
{
    // NOTE emplace_front() return reference to the inserted element (since C++17)
    //      this two lines could be combined
    SpaceObjectList.emplace_front(new cBigAsteroid{AsteroidNum}, [](cBigAsteroid *p) {delete p;});
    return SpaceObjectList.front();
}

/*
 * Create cPlanet object.
 */
std::weak_ptr<cSpaceObject> CreatePlanet(const int PlanetNum)
{
    // NOTE emplace_front() return reference to the inserted element (since C++17)
    //      this two lines could be combined
    SpaceObjectList.emplace_front(new cPlanet{PlanetNum}, [](cPlanet *p) {delete p;});
    return SpaceObjectList.front();
}

/*
 * Create cPlanetoid object.
 */
std::weak_ptr<cSpaceObject> CreatePlanetoid(const int PlanetoidNum)
{
    // NOTE emplace_front() return reference to the inserted element (since C++17)
    //      this two lines could be combined
    SpaceObjectList.emplace_front(new cPlanetoid{PlanetoidNum}, [](cPlanetoid *p) {delete p;});
    return SpaceObjectList.front();
}

/*
 * Create cSpaceDebris object.
 */
std::weak_ptr<cSpaceObject> CreateSpaceDebris()
{
    // NOTE emplace_front() return reference to the inserted element (since C++17)
    //      this two lines could be combined
    SpaceObjectList.emplace_front(new cSpaceDebris, [](cSpaceDebris *p) {delete p;});
    return SpaceObjectList.front();
}

/*
 * Create cBasePart object.
 */
std::weak_ptr<cSpaceObject> CreateBasePart(const int BasePartNum)
{
    // NOTE emplace_front() return reference to the inserted element (since C++17)
    //      this two lines could be combined
    SpaceObjectList.emplace_front(new cBasePart{BasePartNum}, [](cBasePart *p) {delete p;});
    return SpaceObjectList.front();
}

/*
 * Update and remove (erase) dead objects.
 */
void UpdateAllSpaceObject(float Time)
{
    // NOTE use std::erase_if here (since C++20)
    for (auto iter = SpaceObjectList.begin(); iter != SpaceObjectList.end();) {
        if (!iter->get()->UpdateWithTimeSheetList(Time)) {
            iter = SpaceObjectList.erase(iter);
        } else {
            ++iter;
        }
    }
}

/*
 * Draw all objects.
 */
void DrawAllSpaceObjects(bool VertexOnlyPass, unsigned int ShadowMap)
{
    for (auto &tmpObject : SpaceObjectList) {
        // render planets and asteroids before tile animation
        if (tmpObject.get()->ObjectType != eObjectType::Planet
            && tmpObject.get()->ObjectType != eObjectType::Planetoid) {
            tmpObject.get()->Draw(VertexOnlyPass, ShadowMap);
        }
    }
}

/*
 * Release particular space object.
 */
void ReleaseSpaceObject(std::weak_ptr<cSpaceObject> &Object)
{
    auto sharedObject = Object.lock();
    if (!sharedObject) {
        return;
    }

    for (auto iter = SpaceObjectList.begin(); iter != SpaceObjectList.end();) {
        if (iter->get() == sharedObject.get()) {
            SpaceObjectList.erase(iter);
            return;
        }
        ++iter;
    }
}

/*
 * Release all objects.
 */
void ReleaseAllSpaceObjects()
{
    SpaceObjectList.clear();
}

/*
 * Cycle for each space object.
 * Note, caller must guarantee, that 'Object' will not released in callback function call.
 */
void ForEachSpaceObject(std::function<void (cSpaceObject &Object)> function)
{
    for (auto &tmpSpace : SpaceObjectList) {
        function(*tmpSpace);
    }
}

/*
 * Managed cycle for each space object.
 * Note, caller must guarantee, that 'Object' will not released in callback function call.
 */
void ForEachSpaceObject(std::function<void (cSpaceObject &Object, eSpaceCycle &Command)> function)
{
    // NOTE use std::erase_if here (since C++20)
    for (auto iter = SpaceObjectList.begin(); iter != SpaceObjectList.end();) {
        eSpaceCycle Command{eSpaceCycle::Continue};
        function(*iter->get(), Command);

        switch (Command) {
        case eSpaceCycle::Continue:
            ++iter;
            break;
        case eSpaceCycle::Break:
            return;
        case eSpaceCycle::DeleteObjectAndContinue:
            iter = SpaceObjectList.erase(iter);
            break;
        case eSpaceCycle::DeleteObjectAndBreak:
            SpaceObjectList.erase(iter);
            return;
        }
    }
}

/*
 * Managed cycle for each space object pair.
 * Note, caller must guarantee, that 'FirstObject' and 'SecondObject' will not released in callback function call.
 */
void ForEachSpaceObjectPair(std::function<void (cSpaceObject &FirstObject,
                            cSpaceObject &SecondObject,
                            eSpacePairCycle &Command)> function)
{
    for (auto iterFirst = SpaceObjectList.begin(); iterFirst != SpaceObjectList.end();) {
        eSpacePairCycle Command{eSpacePairCycle::Continue};

        for (auto iterSecond = std::next(iterFirst, 1); iterSecond != SpaceObjectList.end();) {
            Command = eSpacePairCycle::Continue;
            function(*iterFirst->get(), *iterSecond->get(), Command);

            // NOTE (?) use std::erase_if here (since C++20)
            if (Command == eSpacePairCycle::DeleteSecondObjectAndContinue
                || Command == eSpacePairCycle::DeleteBothObjectsAndContinue) {
                iterSecond = SpaceObjectList.erase(iterSecond);
            } else {
                ++iterSecond;
            }

            // break second cycle
            if (Command == eSpacePairCycle::DeleteFirstObjectAndContinue
                || Command == eSpacePairCycle::DeleteBothObjectsAndContinue) {
                break;
            }
        }

        // NOTE (?) use std::erase_if here (since C++20)
        if (Command == eSpacePairCycle::DeleteFirstObjectAndContinue
            || Command == eSpacePairCycle::DeleteBothObjectsAndContinue) {
            iterFirst = SpaceObjectList.erase(iterFirst);
        } else {
            ++iterFirst;
        }
    }
}

/*
 * Get object ptr by reference.
 */
std::weak_ptr<cObject3D> GetSpaceObjectPtr(const cSpaceObject &Object)
{
    for (auto &tmpSpace : SpaceObjectList) {
        if (tmpSpace.get() == &Object) {
            return tmpSpace;
        }
    }

    return std::weak_ptr<cObject3D>{};
}

/*
 * Constructor.
 */
cSpaceObject::cSpaceObject()
{
    ObjectStatus = eObjectStatus::Enemy;
    LastCameraPoint = GetCameraCoveredDistance();
}

/*
 * Update.
 */
bool cSpaceObject::Update(float Time)
{
    if (!cObject3D::Update(Time)) {
        return false;
    }

    if (BossPartCountDown > -1.0f) {
        BossPartCountDown -= TimeDelta;

        if (BossPartCountDown <= 0.0f) {
            CreateSpaceExplosion(*this, 34, Location, Speed, -1);
            return false;
        }
    }

    if (ObjectType == eObjectType::SmallAsteroid
        || ObjectType == eObjectType::BigAsteroid
        || ObjectType == eObjectType::Planetoid) {
        if (ObjectType == eObjectType::BigAsteroid) {
            SetRotation(sVECTOR3D{RotationSpeed.x * TimeDelta,
                                  RotationSpeed.y * TimeDelta,
                                  0.0f});
        } else {
            if (RotationSpeed.x != 0.0f) {
                Rotation.x -= RotationSpeed.x * TimeDelta;
                if (Rotation.x <= 360.0f) {
                    Rotation.x += 360.0f;
                }
            }
            if (RotationSpeed.y != 0.0f) {
                Rotation.y -= RotationSpeed.y * TimeDelta;
                if (Rotation.y <= 360.0f) {
                    Rotation.y += 360.0f;
                }
            }
        }
    }

    if (ObjectType == eObjectType::SpaceDebris) {
        if (Speed > 0.0f) {
            Speed -= TimeDelta;
            if (Speed < 0.0f) {
                Speed = 0.0f;
            }
        }
        if (Speed < 0.0f) {
            Speed += TimeDelta;
            if (Speed > 0.0f) {
                Speed = 0.0f;
            }
        }

        if (RotationSpeed.x != 0.0f) {
            SetRotation(sVECTOR3D{-RotationSpeed.x * TimeDelta, 0.0f, 0.0f});
            RotationSpeed.x -= (RotationSpeed.x / 4.0f) * TimeDelta;
        }
        if (RotationSpeed.y != 0.0f) {
            SetRotation(sVECTOR3D{0.0f, -RotationSpeed.y * TimeDelta, 0.0f});
            RotationSpeed.y -= (RotationSpeed.y / 4.0f) * TimeDelta;
        }
        if (RotationSpeed.z != 0.0f) {
            SetRotation(sVECTOR3D{0.0f, 0.0f, -RotationSpeed.z * TimeDelta});
            RotationSpeed.z -= (RotationSpeed.z / 4.0f) * TimeDelta;
        }
    }

    if (ObjectType == eObjectType::Planet
        || ObjectType == eObjectType::Planetoid
        || ObjectType == eObjectType::BigAsteroid) {
        sVECTOR3D Temp = GetCameraCoveredDistance() - LastCameraPoint;

        sVECTOR3D OrientTTT = Temp ^ (-1);
        OrientTTT.Normalize();
        SetLocation(Location + (OrientTTT ^ (Speed * TimeDelta)) + Temp);
        LastCameraPoint = GetCameraCoveredDistance();

        if (ObjectType == eObjectType::Planet) {
            switch (InternalType) {
            case 1:
                Chunks[0].Rotation.y += 0.5f * TimeDelta;
                Chunks[1].Rotation.y += 0.7f * TimeDelta;
                Chunks[2].Rotation.y += 0.8f * TimeDelta;
                Chunks[3].Rotation.y += 0.9f * TimeDelta;
                break;
            case 2:
                Rotation.y += 0.5f * TimeDelta;
                if (Rotation.y >= 360.0f) {
                    Rotation.y -= 360.0f;
                }
                break;
            case 3:
                Chunks[0].Rotation.y += 0.5f * TimeDelta;
                Chunks[1].Rotation.y -= 0.7 * TimeDelta;
                break;
            case 4:
                Chunks[0].Rotation.y += TimeDelta;
                break;
            case 5:
                Chunks[0].Rotation.y += 0.5f * TimeDelta;
                break;
            case 6:
                Chunks[0].Rotation.y += 0.5f * TimeDelta;
                break;
            }

            for (auto &tmpChunk : Chunks) {
                if (tmpChunk.Rotation.x >= 360.0f) {
                    tmpChunk.Rotation.x -= 360.0f;
                }
                if (tmpChunk.Rotation.x <= -360.0f) {
                    tmpChunk.Rotation.x += 360.0f;
                }
                if (tmpChunk.Rotation.y >= 360.0f) {
                    tmpChunk.Rotation.y -= 360.0f;
                }
                if (tmpChunk.Rotation.y <= -360.0f) {
                    tmpChunk.Rotation.y += 360.0f;
                }
            }
        }
    } else {
        SetLocation(Location + (Orientation ^ (Speed * TimeDelta)));
    }

    if (Velocity.x + Velocity.y + Velocity.z != 0.0f) {
        SetLocation(Location + (Velocity ^ TimeDelta));
        Velocity -= Velocity ^ (0.5f * TimeDelta);
    }

    return true;
}

} // astromenace namespace
} // viewizard namespace
