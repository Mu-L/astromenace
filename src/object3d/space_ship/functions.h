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

#ifndef OBJECT3D_SPACESHIP_FUNCTIONS_H
#define OBJECT3D_SPACESHIP_FUNCTIONS_H

#include "../object3d.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

// Find angles to aim on target with prediction.
void GetWeaponOnTargetOrientation(eObjectStatus WeaponStatus, const sVECTOR3D &TargetingComputerLocation,
                                  const sVECTOR3D &WeaponLocation, const sVECTOR3D &CurrentWeaponRotation,
                                  float MinTargetingDistance, const float (&CurrentWeaponRotationMatrix)[9],
                                  sVECTOR3D &NeedAngle, bool NeedCenterOrientation, int WeaponType);

} // astromenace namespace
} // viewizard namespace

#endif // OBJECT3D_SPACESHIP_FUNCTIONS_H
