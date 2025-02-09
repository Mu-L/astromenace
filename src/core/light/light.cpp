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

#include "../graphics/graphics.h"
#include "../math/math.h"
#include "light.h"

namespace viewizard {

namespace {

// no point to calculate attenuation for all scene, limit it by 10
constexpr float AttenuationLimit{10.0f};
// all lights, indexed by light's type
std::unordered_multimap<eLightType, std::shared_ptr<cLight>, sEnumHash> LightsMap;

} // unnamed namespace


/*
 * Calculate affected lights counter and create sorted map with affected lights.
 * Note, all attenuation-related calculations not involved in real rendering by OpenGL,
 * and need for internal use only in order to activate (via OpenGL) proper lights.
 */
int vw_CalculateAllPointLightsAttenuation(const sVECTOR3D &Location, float Radius2,
                                          std::multimap<float, cLight*> *AffectedLightsMap)
{
    int AffectedLightsCount{0};

    auto range = LightsMap.equal_range(eLightType::Point);
    for (; range.first != range.second; ++range.first) {
        auto &tmpLight = *range.first;
        if (tmpLight.second->On) {
            float tmpAttenuation = tmpLight.second->ConstantAttenuation;

            // care about distance to object
            sVECTOR3D DistV{Location.x - tmpLight.second->Location.x,
                            Location.y - tmpLight.second->Location.y,
                            Location.z - tmpLight.second->Location.z};
            float Dist2 = DistV.x * DistV.x + DistV.y * DistV.y + DistV.z * DistV.z;
            if (Dist2 > Radius2) {
                Dist2 -= Radius2;
                // Constant and Quadratic first (this is all about sqrt(), that we need for Linear)
                tmpAttenuation += tmpLight.second->QuadraticAttenuation * Dist2;

                if (tmpAttenuation < AttenuationLimit && tmpLight.second->LinearAttenuation > 0.0f) {
                    tmpAttenuation += tmpLight.second->LinearAttenuation * vw_sqrtf(Dist2);
                }
            }

            if (tmpAttenuation <= AttenuationLimit) {
                AffectedLightsCount++;
                if (AffectedLightsMap) {
                    AffectedLightsMap->emplace(tmpAttenuation, tmpLight.second.get());
                }
            }
        }
    }

    return AffectedLightsCount;
}

/*
 * Activate proper lights for particular object (presented by location and radius^2).
 */
void vw_CheckAndActivateAllLights(const sVECTOR3D &Location, float Radius2, int DirLimit,
                                  int PointLimit, const float (&Matrix)[16])
{
    int countType1 = 0; // counter for directional light
    int countType2 = 0; // counter for point light

    // directional light should be first, since this is the main scene light
    auto range = LightsMap.equal_range(eLightType::Directional);
    for (; range.first != range.second
           && countType1 < DirLimit
           && countType1 < vw_DevCaps().MaxActiveLights
         ; ++range.first) {
        auto &tmpLight = *range.first;
        if (tmpLight.second->Activate(countType1, Matrix)) {
            countType1++;
        }
    }

    // point lights
    if (PointLimit > 0) {
        std::multimap<float, cLight*> AffectedLightsMap;
        // call for std::map calculation with sorted by attenuation affected lights
        vw_CalculateAllPointLightsAttenuation(Location, Radius2, &AffectedLightsMap);

        // enable lights with less attenuation first
        for (auto &tmpLight : AffectedLightsMap) {
            if (countType2 >= PointLimit || countType1 + countType2 >= vw_DevCaps().MaxActiveLights) {
                break;
            }
            if (tmpLight.second->Activate(countType1 + countType2, Matrix)) {
                countType2++;
            }
        }
    }

    vw_Lighting(true);
}

/*
 * Deactivate all lights.
 */
void vw_DeActivateAllLights()
{
    for (auto &tmpLight : LightsMap) {
        tmpLight.second->DeActivate();
    }

    vw_Lighting(false);
}

/*
 * Release light.
 */
void vw_ReleaseLight(std::weak_ptr<cLight> &Light)
{
    if (auto sharedLight = Light.lock()) {
        for (auto iter = LightsMap.begin(); iter != LightsMap.end(); ++iter) {
            if (iter->second.get() == sharedLight.get()) {
                LightsMap.erase(iter);
                // current iterator invalidated by erase()
                return;
            }
        }
    }
}

/*
 * Release all lights.
 */
void vw_ReleaseAllLights()
{
    LightsMap.clear();
}

/*
 * Create light.
 */
std::weak_ptr<cLight> vw_CreateLight(eLightType Type)
{
    auto Light = LightsMap.emplace(Type, std::shared_ptr<cLight>{new cLight, [](cLight *p) {delete p;}});
    Light->second->LightType = Type;
    return Light->second;
}

/*
 * Create point light with initialization.
 */
std::weak_ptr<cLight> vw_CreatePointLight(const sVECTOR3D &Location,
                                          float R, float G, float B,
                                          float Linear, float Quadratic)
{
    auto Light = vw_CreateLight(eLightType::Point);

    if (auto sharedLight = Light.lock()) {
        sharedLight->Diffuse[0] = sharedLight->Specular[0] = R;
        sharedLight->Diffuse[1] = sharedLight->Specular[1] = G;
        sharedLight->Diffuse[2] = sharedLight->Specular[2] = B;
        sharedLight->Diffuse[3] = sharedLight->Specular[3] = 1.0f;
        sharedLight->LinearAttenuation = sharedLight->LinearAttenuationBase = Linear;
        sharedLight->QuadraticAttenuation = sharedLight->QuadraticAttenuationBase = Quadratic;
        sharedLight->Location = Location;
    }

    return Light;
}

/*
 * Get main direct light. Usually, first one is the main.
 */
bool vw_GetMainDirectLight(std::weak_ptr<cLight> &Light)
{
    auto tmpLight = LightsMap.find(eLightType::Directional);
    if (tmpLight != LightsMap.end()) {
        Light = tmpLight->second;
        return true;
    }

    return false;
}

/*
 * Activate and setup for proper light type (OpenGL-related).
 */
bool cLight::Activate(int CurrentLightNum, const float (&Matrix)[16])
{
    if (!On) {
        return false;
    }
    RealLightNum = CurrentLightNum;

    vw_PushMatrix();
    vw_LoadIdentity();
    vw_SetMatrix(Matrix);

    if (LightType == eLightType::Directional) {
        float RenderDirection[4]{-Direction.x, -Direction.y, -Direction.z, 0.0f};
        float RenderLocation[4]{-Direction.x, -Direction.y, -Direction.z, 0.0f};

        // we don't reset OpenGL lights status, forced to reset everything for current light
        vw_SetLightV(RealLightNum, eLightVParameter::DIFFUSE, Diffuse);
        vw_SetLightV(RealLightNum, eLightVParameter::SPECULAR, Specular);
        vw_SetLightV(RealLightNum, eLightVParameter::AMBIENT, Ambient);
        vw_SetLightV(RealLightNum, eLightVParameter::SPOT_DIRECTION, RenderDirection);
        vw_SetLightV(RealLightNum, eLightVParameter::POSITION, RenderLocation);
    } else {
        float RenderDirection[4]{0.0f, 0.0f, 0.0f, 0.0f};
        float RenderLocation[4]{Location.x, Location.y, Location.z, 1.0f};

        // we don't reset OpenGL lights status, forced to reset everything for current light
        vw_SetLight(RealLightNum, eLightParameter::CONSTANT_ATTENUATION, ConstantAttenuation);
        vw_SetLight(RealLightNum, eLightParameter::LINEAR_ATTENUATION, LinearAttenuation);
        vw_SetLight(RealLightNum, eLightParameter::QUADRATIC_ATTENUATION, QuadraticAttenuation);

        vw_SetLightV(RealLightNum, eLightVParameter::DIFFUSE, Diffuse);
        vw_SetLightV(RealLightNum, eLightVParameter::SPECULAR, Specular);
        vw_SetLightV(RealLightNum, eLightVParameter::AMBIENT, Ambient);
        vw_SetLightV(RealLightNum, eLightVParameter::SPOT_DIRECTION, RenderDirection);
        vw_SetLightV(RealLightNum, eLightVParameter::POSITION, RenderLocation);
    }

    vw_LightEnable(RealLightNum, true);
    vw_PopMatrix();

    return true;
}

/*
 *  Deactivate (OpenGL-related).
 */
void cLight::DeActivate()
{
    if (!On) {
        return;
    }

    if (RealLightNum > -1) {
        vw_LightEnable(RealLightNum, false);
        RealLightNum = -1;
    }
}

/*
 * Set location.
 */
void cLight::SetLocation(sVECTOR3D NewLocation)
{
    if (LightType != eLightType::Directional) {
        Location = NewLocation;
    }
}

} // viewizard namespace
