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
#include "../audio/audio.h"
#include "camera.h"
#include <cmath>

namespace viewizard {

namespace {

// Camera update flag (need for frustum calculation on camera update).
bool CameraUpdated{true};
// Camera location.
sVECTOR3D CameraLocation{0.0f, 0.0f, 0.0f};
// Camera rotation angles.
sVECTOR3D CameraRotation{0.0f, 0.0f, 0.0f};
// Camera deviation (need for camera shake effect).
sVECTOR3D CameraDeviation{0.0f, 0.0f, 0.0f};
// Camera focus point (anchor).
sVECTOR3D CameraFocusPoint{0.0f, 0.0f, 0.0f};

} // unnamed namespace


/*
 * Update listener position and orientation.
 */
static void UpdateAudioListenerPosition()
{
    if (!vw_GetAudioStatus()) {
        return;
    }

    float ListenerPosition[3]{CameraLocation.x, CameraLocation.y, CameraLocation.z};
    float ListenerVelocity[3]{0.0f, 0.0f, 0.0f};

    // orientation expressed as "at" and "up" vectors
    sVECTOR3D ListenerOrientationAT(0.0f, 0.0f, -1.0f);
    vw_RotatePoint(ListenerOrientationAT, CameraRotation);
    sVECTOR3D ListenerOrientationUP(0.0f, 1.0f, 0.0f);
    vw_RotatePoint(ListenerOrientationUP, CameraRotation);
    float ListenerOrientation[6]{ListenerOrientationAT.x, ListenerOrientationAT.y, ListenerOrientationAT.z,
                                 ListenerOrientationUP.x, ListenerOrientationUP.y, ListenerOrientationUP.z};

    vw_Listener(ListenerPosition, ListenerVelocity, ListenerOrientation);
}

/*
 * Set camera location.
 */
void vw_SetCameraLocation(const sVECTOR3D &NewLocation)
{
    CameraLocation = NewLocation;
    CameraUpdated = true;

    UpdateAudioListenerPosition();
}

/*
 * Increment camera location by vector.
 */
void vw_IncCameraLocation(const sVECTOR3D &IncLocation)
{
    CameraLocation += IncLocation;
    CameraFocusPoint += IncLocation;
    CameraUpdated = true;

    UpdateAudioListenerPosition();
}

/*
 * Get camera location.
 */
sVECTOR3D vw_GetCameraLocation(sVECTOR3D *CurrentLocation)
{
    if (CurrentLocation) {
        *CurrentLocation = CameraLocation;
    }
    return CameraLocation;
}

/*
 * Get camera rotation angles.
 */
sVECTOR3D vw_GetCameraRotation(sVECTOR3D *CurrentRotation)
{
    if (CurrentRotation) {
        *CurrentRotation = CameraRotation;
    }
    return CameraRotation;
}

/*
 * Get camera focus point (anchor).
 */
sVECTOR3D vw_GetCameraFocusPoint()
{
    return CameraFocusPoint;
}

/*
 * Move camera by direction.
 */
void vw_SetCameraMove(const sVECTOR3D &NewRotation, float ChangeDistance, const sVECTOR3D &Point)
{
    // revert back all movements
    CameraLocation -= Point;
    vw_RotatePointInv(CameraLocation, CameraRotation);
    // change distance
    CameraLocation.z += ChangeDistance;
    // change rotation angles
    CameraRotation += NewRotation;
    // apply corrected distance and angles to camera
    vw_RotatePoint(CameraLocation, CameraRotation ^ (-1.0f));
    CameraLocation += Point;
    CameraUpdated = true;

    UpdateAudioListenerPosition();
}

/*
 * Move camera around point (anchor).
 */
void vw_SetCameraMoveAroundPoint(const sVECTOR3D &Point, float ChangeDistance, const sVECTOR3D &ChangeRotation)
{
    constexpr float DegToRadFactor = 0.0174532925f; // conversion factor to convert degrees to radians
    CameraFocusPoint = Point;
    // initial camera move
    vw_SetCameraMove(ChangeRotation, ChangeDistance, Point);

    // rotate camera to the point
    sVECTOR3D exV{0.0f, 0.0f, 0.0f};
    sVECTOR3D V{Point.x - CameraLocation.x,
                Point.y - CameraLocation.y,
                Point.z - CameraLocation.z};
    V.Normalize();
    if (V.x * V.x + V.y * V.y + V.z * V.z != 0.0f) {
        exV = V;
    }

    float newrotY{0.0f};
    if (exV.z != 0.0f) {
        if ((0.0f > exV.x && 0.0f > -exV.z) || (0.0f < exV.x && 0.0f < -exV.z)) {
            newrotY = atanf(fabsf(exV.x / exV.z)) / DegToRadFactor;
        } else {
            newrotY = -atanf(fabsf(exV.x / exV.z)) / DegToRadFactor;
        }
    }

    float newrotX{0.0f};
    float kat{(exV.z) * (exV.z) + (exV.x) * (exV.x)};
    if (kat != 0.0f) {
        kat = sqrtf(kat);
        if (0 < exV.y) {
            newrotX = -atanf(fabsf(exV.y) / kat) / DegToRadFactor;
        } else {
            newrotX = atanf(fabsf(exV.y) / kat) / DegToRadFactor;
        }
    }
    if (0 > -exV.z) {
        newrotY += 180;
    }

    CameraRotation.x = -newrotX;
    CameraRotation.y = -newrotY;
    CameraUpdated = true;

    UpdateAudioListenerPosition();
}

/*
 * Camera deviation setup (need for camera shake effect).
 */
void vw_SetCameraDeviation(const sVECTOR3D &NewCameraDeviation)
{
    CameraDeviation = NewCameraDeviation;
}

/*
 * Camera setup.
 */
void vw_CameraLookAt()
{
    vw_Rotate(-CameraRotation.x, 1.0f, 0.0f, 0.0f);
    vw_Rotate(-CameraRotation.y, 0.0f, 1.0f, 0.0f);
    vw_Rotate(-CameraRotation.z, 0.0f, 0.0f, 1.0f);

    vw_Translate((CameraLocation^(-1.0f)) - CameraDeviation);

    // recalculate frustum on camera update
    if (CameraUpdated) {
        vw_CalculateFrustum();
        CameraUpdated = false;
    }
}

} // viewizard namespace
