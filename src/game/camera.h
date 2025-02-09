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

#ifndef GAME_CAMERA_H
#define GAME_CAMERA_H

#include "../core/base.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {

struct sVECTOR3D;

namespace astromenace {

// Initialize camera.
void InitCamera();
// Reset camera status.
void ResetCamera();
// Setup camera shake.
void SetupCameraShake(const sVECTOR3D &Location, float Power);
// Update camera.
void CameraUpdate(float Time);
// Get camera shake.
float GetCameraShake();
// Get camera speed.
float GetCameraSpeed();
// Get camera covered distance.
const sVECTOR3D &GetCameraCoveredDistance();
// Get camera movement direction.
const sVECTOR3D &GetCameraMovementDirection();

} // astromenace namespace
} // viewizard namespace

#endif // GAME_CAMERA_H
