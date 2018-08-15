/************************************************************************************

	AstroMenace
	Hardcore 3D space scroll-shooter with spaceship upgrade possibilities.
	Copyright (c) 2006-2018 Mikhail Kurinnoi, Viewizard


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


	Website: https://www.viewizard.com/
	Project: https://github.com/viewizard/astromenace
	E-mail: viewizard@viewizard.com

*************************************************************************************/

#include "../object3d.h"
#include "../space_ship/space_ship.h"
#include "../ground_object/ground_object.h"
#include "../projectile/projectile.h"
#include "../space_object/space_object.h"

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

namespace {

constexpr float RadToDeg = 180.0f / 3.14159f; // convert radian to degree

} // unnamed namespace


/*
 * Find missile target and target intercept course for missile.
 * See "Dihedral angle" (geometry) for more info about what we are doing here.
 */
std::weak_ptr<cObject3D>
FindTargetAndInterceptCourse(eObjectStatus MissileObjectStatus, const sVECTOR3D &MissileLocation,
			     const sVECTOR3D &MissileRotation, const float (&MissileRotationMatrix)[9],
			     sVECTOR3D &NeedAngle, const float MaxMissileFlyDistance)
{
	NeedAngle = MissileRotation;
	float tmpDistanceToLockedTarget2{1000.0f * 1000.0f};
	std::weak_ptr<cObject3D> LockedTarget{};

	// missile "targeting" zone (missile targeting system's "view" zone)
	// TODO probably, we should connect this parameter to missile type (?)
	constexpr float TargetingZone{1.0f}; // in radians (!)

	sVECTOR3D Orientation{0.0f, 0.0f, 1.0f};
	vw_Matrix33CalcPoint(Orientation, MissileRotationMatrix);
	sVECTOR3D PointUp{0.0f, 1.0f, 0.0f};
	vw_Matrix33CalcPoint(PointUp, MissileRotationMatrix);
	sVECTOR3D PointRight{1.0f, 0.0f, 0.0f};
	vw_Matrix33CalcPoint(PointRight, MissileRotationMatrix);

	// vertical plane (ahead/behind)
	float A, B, C, D;
	vw_GetPlaneABCD(A, B, C, D, MissileLocation, MissileLocation + PointRight, MissileLocation + PointUp);

	// horizontal plane (up/down)
	float A2, B2, C2, D2;
	vw_GetPlaneABCD(A2, B2, C2, D2, MissileLocation, MissileLocation + Orientation, MissileLocation + PointRight);
	float A2B2C2D2NormalLength = vw_sqrtf(A2 * A2 + B2 * B2 + C2 * C2);

	// vertical plane (left/right)
	float A3, B3, C3, D3;
	vw_GetPlaneABCD(A3, B3, C3, D3, MissileLocation, MissileLocation + Orientation, MissileLocation + PointUp);
	float A3B3C3D3NormalLength = vw_sqrtf(A3 * A3 + B3 * B3 + C3 * C3);

	// the idea of tmpDistanceFactorByObjectType is provide targeting priority, we increase factor
	// for different types of objects, so, we need next type objects position closer than previous
	// in order to change target, but, reset factor to 1.0f in case we lock and check same type
	// objects (or don't have any target yet)
	float tmpDistanceFactorByObjectType{1.0f};

	// check object location with current missile course, if object in missile
	// "targeting" zone - find target intercept course for missile
	auto CheckObjectLocation = [&] (const sVECTOR3D &TargetLocation) {
		float tmpCheckSideBySign = A * TargetLocation.x + B * TargetLocation.y + C * TargetLocation.z + D;
		if (tmpCheckSideBySign < 0.0f) // target is behind
			return false;

		sVECTOR3D tmpDistance = TargetLocation - MissileLocation;
		float tmpLength2 = tmpDistance.x * tmpDistance.x +
				   tmpDistance.y * tmpDistance.y +
				   tmpDistance.z * tmpDistance.z;

		if ((tmpDistanceToLockedTarget2 / tmpDistanceFactorByObjectType) <= tmpLength2)
			return false;

		float tmpLength = vw_sqrtf(tmpLength2);
		if (tmpLength > MaxMissileFlyDistance)
			return false;

		if ((tmpLength > 0.0f) && (A2B2C2D2NormalLength > 0.0f)) {
			float tmpDihedralAngle = (A2 * tmpDistance.x + B2 * tmpDistance.y + C2 * tmpDistance.z) /
						 (tmpLength * A2B2C2D2NormalLength);
			if ((tmpDihedralAngle >= -TargetingZone) &&
			    (tmpDihedralAngle <= TargetingZone))
				NeedAngle.x = MissileRotation.x - asinf(tmpDihedralAngle) * RadToDeg;
		}

		if ((tmpLength > 0.0f) && (A3B3C3D3NormalLength > 0.0f)) {
			float tmpDihedralAngle = (A3 * tmpDistance.x + B3 * tmpDistance.y + C3 * tmpDistance.z) /
						 (tmpLength * A3B3C3D3NormalLength);
			if ((tmpDihedralAngle >= -TargetingZone) &&
			    (tmpDihedralAngle <= TargetingZone))
				NeedAngle.y = MissileRotation.y - asinf(tmpDihedralAngle) * RadToDeg;
		}

		tmpDistanceToLockedTarget2 = tmpLength2;
		tmpDistanceFactorByObjectType = 1.0f;
		return true;
	};

	ForEachProjectile([&] (const cProjectile &tmpProjectile) {
		if ((tmpProjectile.ProjectileType == 3) && // flares
		    NeedCheckCollision(tmpProjectile) &&
		    ObjectsStatusFoe(MissileObjectStatus, tmpProjectile.ObjectStatus) &&
		    CheckObjectLocation(tmpProjectile.Location))
			LockedTarget = GetProjectilePtr(tmpProjectile);
	});

	if (!LockedTarget.expired())
		tmpDistanceFactorByObjectType = 3.0f;
	ForEachGroundObject([&] (const cGroundObject &tmpGround) {
		if (NeedCheckCollision(tmpGround) &&
		    ObjectsStatusFoe(MissileObjectStatus, tmpGround.ObjectStatus)) {
			sVECTOR3D TargetLocation = tmpGround.GeometryCenter;
			vw_Matrix33CalcPoint(TargetLocation, tmpGround.CurrentRotationMat);
			TargetLocation += tmpGround.Location;

			if (CheckObjectLocation(TargetLocation))
				LockedTarget = GetGroundObjectPtr(tmpGround);
		}
	});

	if (!LockedTarget.expired())
		tmpDistanceFactorByObjectType = 6.0f;
	ForEachSpaceShip([&] (const cSpaceShip &tmpShip) {
		if (NeedCheckCollision(tmpShip) &&
		    ObjectsStatusFoe(MissileObjectStatus, tmpShip.ObjectStatus) &&
		    CheckObjectLocation(tmpShip.Location))
			LockedTarget = GetSpaceShipPtr(tmpShip);
	});

	if (!LockedTarget.expired())
		tmpDistanceFactorByObjectType = 10.0f;
	ForEachSpaceObject([&] (const cSpaceObject &tmpSpace) {
		if (NeedCheckCollision(tmpSpace) &&
		    ObjectsStatusFoe(MissileObjectStatus, tmpSpace.ObjectStatus) &&
		    (tmpSpace.ObjectType != eObjectType::SpaceDebris) &&
		    CheckObjectLocation(tmpSpace.Location))
			LockedTarget = GetSpaceObjectPtr(tmpSpace);
	});

	return LockedTarget;
}

/*
 * Correct target intercept course for missile.
 * See "Dihedral angle" (geometry) for more info about what we are doing here.
 */
bool CorrectTargetInterceptCourse(const sVECTOR3D &MissileLocation, const sVECTOR3D &MissileRotation,
				  const float (&MissileRotationMatrix)[9],
				  std::weak_ptr<cObject3D> &Target, sVECTOR3D &NeedAngle)
{
	auto sharedTarget = Target.lock();
	if (!sharedTarget)
		return false;

	sVECTOR3D Orientation{0.0f, 0.0f, 1.0f};
	vw_Matrix33CalcPoint(Orientation, MissileRotationMatrix);
	sVECTOR3D PointUp{0.0f, 1.0f, 0.0f};
	vw_Matrix33CalcPoint(PointUp, MissileRotationMatrix);
	sVECTOR3D PointRight{1.0f, 0.0f, 0.0f};
	vw_Matrix33CalcPoint(PointRight, MissileRotationMatrix);

	// vertical plane (ahead/behind)
	float A, B, C, D;
	vw_GetPlaneABCD(A, B, C, D, MissileLocation, MissileLocation + PointRight, MissileLocation + PointUp);
	if ((A * sharedTarget->Location.x +
	     B * sharedTarget->Location.y +
	     C * sharedTarget->Location.z + D) <= 0.0f)
		return false;

	NeedAngle = MissileRotation;

	sVECTOR3D tmpTargetGeometryCenter = sharedTarget->GeometryCenter;
	vw_Matrix33CalcPoint(tmpTargetGeometryCenter, sharedTarget->CurrentRotationMat);
	sVECTOR3D tmpDistance = sharedTarget->Location + tmpTargetGeometryCenter - MissileLocation;
	float tmpLength = tmpDistance.Length();

	// missile "targeting" zone (missile targeting system's "view" zone)
	// TODO probably, we should connect this parameter to missile type (?)
	constexpr float TargetingZone{1.0f}; // in radians (!)

	// horizontal plane (up/down)
	vw_GetPlaneABCD(A, B, C, D, MissileLocation, MissileLocation + Orientation, MissileLocation + PointRight);
	float tmpNormalLength = vw_sqrtf(A * A + B * B + C * C);
	NeedAngle.x = MissileRotation.x;
	if ((tmpLength > 0.0f) && (tmpNormalLength > 0.0f)) {
		float tmpDihedralAngle = (A * tmpDistance.x + B * tmpDistance.y + C * tmpDistance.z) /
					 (tmpLength * tmpNormalLength);
		if ((tmpDihedralAngle >= -TargetingZone) &&
		    (tmpDihedralAngle <= TargetingZone))
			NeedAngle.x = MissileRotation.x - asinf(tmpDihedralAngle) * RadToDeg;
	}

	// vertical plane (left/right)
	vw_GetPlaneABCD(A, B, C, D, MissileLocation, MissileLocation + Orientation, MissileLocation + PointUp);
	tmpNormalLength = vw_sqrtf(A * A + B * B + C * C);
	NeedAngle.y = MissileRotation.y;
	if ((tmpLength > 0.0f) && (tmpNormalLength > 0.0f)) {
		float tmpDihedralAngle = (A * tmpDistance.x + B * tmpDistance.y + C * tmpDistance.z) /
					 (tmpLength * tmpNormalLength);
		if ((tmpDihedralAngle >= -TargetingZone) &&
		    (tmpDihedralAngle <= TargetingZone))
			NeedAngle.y = MissileRotation.y - asinf(tmpDihedralAngle) * RadToDeg;
	}

	return true;
}

/*
 * Check, that missile target stay ahead.
 */
static bool MissileTargetStayAhead(const cObject3D &Target,
				   const sVECTOR3D &MissileLocation,
				   const float (&MissileRotationMatrix)[9])
{
	sVECTOR3D PointUp(0.0f, 1.0f, 0.0f);
	vw_Matrix33CalcPoint(PointUp, MissileRotationMatrix);
	sVECTOR3D PointRight(1.0f, 0.0f, 0.0f);
	vw_Matrix33CalcPoint(PointRight, MissileRotationMatrix);

	float A, B, C, D;
	vw_GetPlaneABCD(A, B, C, D, MissileLocation, MissileLocation + PointRight, MissileLocation + PointUp);

	if ((A * Target.Location.x +
	     B * Target.Location.y +
	     C * Target.Location.z + D) > 0.0f)
		return true;

	return false;
}

/*
 * Check locked by missle target.
 */
bool CheckMissileTarget(std::weak_ptr<cObject3D> &Target, const sVECTOR3D &MissileLocation,
			const float (&MissileRotationMatrix)[9])
{
	auto sharedTarget = Target.lock();
	if (!sharedTarget)
		return false;

	bool ObjectFound{false};
	ForEachProjectile([&sharedTarget, &ObjectFound] (const cProjectile &tmpProjectile, eProjectileCycle &Command) {
		if (&tmpProjectile == sharedTarget.get()) {
			ObjectFound = true;
			Command = eProjectileCycle::Break;
		}
	});
	if (ObjectFound)
		return MissileTargetStayAhead(*sharedTarget, MissileLocation, MissileRotationMatrix);

	ForEachGroundObject([&sharedTarget, &ObjectFound] (const cGroundObject &tmpGround, eGroundCycle &Command) {
		if (&tmpGround == sharedTarget.get()) {
			ObjectFound = true;
			Command = eGroundCycle::Break;
		}
	});
	if (ObjectFound)
		return MissileTargetStayAhead(*sharedTarget, MissileLocation, MissileRotationMatrix);

	ForEachSpaceShip([&sharedTarget, &ObjectFound] (const cSpaceShip &tmpShip, eShipCycle &Command) {
		if (&tmpShip == sharedTarget.get()) {
			ObjectFound = true;
			Command = eShipCycle::Break;
		}
	});
	if (ObjectFound)
		return MissileTargetStayAhead(*sharedTarget, MissileLocation, MissileRotationMatrix);

	ForEachSpaceObject([&sharedTarget, &ObjectFound] (const cSpaceObject &tmpSpace, eSpaceCycle &Command) {
		if (&tmpSpace == sharedTarget.get()) {
			ObjectFound = true;
			Command = eSpaceCycle::Break;
		}
	});
	if (ObjectFound)
		return MissileTargetStayAhead(*sharedTarget, MissileLocation, MissileRotationMatrix);

	return false;
}

/*
 * Get closest target to mine.
 */
std::weak_ptr<cObject3D> GetClosestTargetToMine(eObjectStatus MineStatus, const sVECTOR3D &MineLocation)
{
	std::weak_ptr<cObject3D> ClosestTarget{};
	float MinDistance2{-1.0f};

	ForEachSpaceShip([&] (const cSpaceShip &tmpShip) {
		if (NeedCheckCollision(tmpShip) &&
		    ObjectsStatusFoe(MineStatus, tmpShip.ObjectStatus)) {
			float tmpDistance2 = (tmpShip.Location.x - MineLocation.x) * (tmpShip.Location.x - MineLocation.x) +
					     (tmpShip.Location.y - MineLocation.y) * (tmpShip.Location.y - MineLocation.y) +
					     (tmpShip.Location.z - MineLocation.z) * (tmpShip.Location.z - MineLocation.z);

			if (MinDistance2 < 0.0f) {
				MinDistance2 = tmpDistance2;
				ClosestTarget = GetSpaceShipPtr(tmpShip);
			} else if (tmpDistance2 < MinDistance2) {
				MinDistance2 = tmpDistance2;
				ClosestTarget = GetSpaceShipPtr(tmpShip);
			}
		}
	});

	return ClosestTarget;
}

} // astromenace namespace
} // viewizard namespace
