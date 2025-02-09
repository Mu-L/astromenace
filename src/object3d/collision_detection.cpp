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

// TODO since HitBB is OBB, "collision_detection" functions should be used
//      all code duplication should be removed

// TODO change from cObject3D to sModel3D

#include "object3d.h"
#include <algorithm>
#include <cmath>
#include <cstring>

// NOTE switch to nested namespace definition (namespace A::B::C { ... }) (since C++17)
namespace viewizard {
namespace astromenace {

/*
 * Mesh-Sphere collision detection.
 */
bool CheckMeshSphereCollisionDetection(const cObject3D &Object1, const cObject3D &Object2,
                                       sVECTOR3D &NewLoc, int &Object1PieceNum)
{
    if (Object1.Chunks.empty()) {
        return false;
    }

    for (unsigned int j = 0; j < Object1.Chunks.size(); j++) {
        if (!Object1.HitBB.empty()) {
            float Distance2 = (Object1.Location.x + Object1.HitBB[j].Location.x - Object2.Location.x) *
                              (Object1.Location.x + Object1.HitBB[j].Location.x - Object2.Location.x) +
                              (Object1.Location.y + Object1.HitBB[j].Location.y - Object2.Location.y) *
                              (Object1.Location.y + Object1.HitBB[j].Location.y - Object2.Location.y) +
                              (Object1.Location.z + Object1.HitBB[j].Location.z - Object2.Location.z) *
                              (Object1.Location.z + Object1.HitBB[j].Location.z - Object2.Location.z);

            if (Distance2 > Object1.HitBB[j].Radius2 + Object2.Radius * Object2.Radius) {
                sVECTOR3D MidPoint = (Object2.Location + Object2.PrevLocation) / 2.0f;
                sVECTOR3D Direction = Object2.Location - Object2.PrevLocation;
                float HalfLength = Direction.Length() / 2.0f;
                Direction.Normalize();

                sVECTOR3D T = Object1.Location + Object1.HitBB[j].Location - MidPoint;
                float r;

                // check with X, Y, Z axis
                if (fabs(T.x) > Object1.HitBB[j].Box[0].x + HalfLength * fabs(Direction.x) ||
                    fabs(T.y) > Object1.HitBB[j].Box[0].y + HalfLength * fabs(Direction.y) ||
                    fabs(T.z) > Object1.HitBB[j].Box[0].z + HalfLength * fabs(Direction.z)) {
                    continue;
                }

                // X ^ Direction
                r = Object1.HitBB[j].Box[0].y * fabs(Direction.z) + Object1.HitBB[j].Box[0].z * fabs(Direction.y);
                if (fabs(T.y * Direction.z - T.z * Direction.y) > r) {
                    continue;
                }

                // Y ^ Direction
                r = Object1.HitBB[j].Box[0].x * fabs(Direction.z) + Object1.HitBB[j].Box[0].z * fabs(Direction.x);
                if (fabs(T.z * Direction.x - T.x * Direction.z) > r) {
                    continue;
                }

                // Z ^ Direction
                r = Object1.HitBB[j].Box[0].x * fabs(Direction.y) + Object1.HitBB[j].Box[0].y * fabs(Direction.x);
                if (fabs(T.x * Direction.y - T.y * Direction.x) > r) {
                    continue;
                }
            }
        }

        if (vw_SphereMeshCollision(Object1.Location, Object1.Chunks[j],
                                   Object1.CurrentRotationMat, Object2.Radius, Object2.Location,
                                   Object2.PrevLocation, NewLoc)) {
            Object1PieceNum = j;
            return true;
        }
    }

    return false;
}

/*
 * HitBB-HitBB collision detection.
 */
bool CheckHitBBHitBBCollisionDetection(const cObject3D &Object1, const cObject3D &Object2,
                                       int &Object1PieceNum, int &Object2PieceNum)
{
    for (unsigned int i = 0; i < Object1.Chunks.size(); i++) {
        for (unsigned int j = 0; j < Object2.Chunks.size(); j++) {
            float Distance2 = (Object1.Location.x + Object1.HitBB[i].Location.x -
                               Object2.Location.x - Object2.HitBB[j].Location.x) *
                              (Object1.Location.x + Object1.HitBB[i].Location.x -
                               Object2.Location.x - Object2.HitBB[j].Location.x) +
                              (Object1.Location.y + Object1.HitBB[i].Location.y -
                               Object2.Location.y - Object2.HitBB[j].Location.y) *
                              (Object1.Location.y + Object1.HitBB[i].Location.y -
                               Object2.Location.y - Object2.HitBB[j].Location.y) +
                              (Object1.Location.z + Object1.HitBB[i].Location.z -
                               Object2.Location.z - Object2.HitBB[j].Location.z) *
                              (Object1.Location.z + Object1.HitBB[i].Location.z -
                               Object2.Location.z - Object2.HitBB[j].Location.z);

            if (Distance2 > Object2.HitBB[j].Radius2 + Object1.HitBB[i].Radius2) {
                continue;
            }

            float TMPOldInvRotationMat[9];
            memcpy(TMPOldInvRotationMat, Object1.CurrentRotationMat, 9 * sizeof(Object1.CurrentRotationMat[0]));
            vw_Matrix33InverseRotate(TMPOldInvRotationMat);

            float matB[9];
            memcpy(matB, Object2.CurrentRotationMat, 9 * sizeof(Object2.CurrentRotationMat[0]));
            vw_Matrix33Mult(matB, TMPOldInvRotationMat);

            sVECTOR3D vPosB = (Object2.Location + Object2.HitBB[j].Location) -
                              (Object1.Location + Object1.HitBB[i].Location);
            vw_Matrix33CalcPoint(vPosB, TMPOldInvRotationMat);

            sVECTOR3D XAxis(matB[0], matB[3], matB[6]);
            sVECTOR3D YAxis(matB[1], matB[4], matB[7]);
            sVECTOR3D ZAxis(matB[2], matB[5], matB[8]);

            sVECTOR3D Obj1_data(Object1.HitBB[i].Size.x / 2.0f,
                                Object1.HitBB[i].Size.y / 2.0f,
                                Object1.HitBB[i].Size.z / 2.0f);
            sVECTOR3D Obj2_data(Object2.HitBB[j].Size.x / 2.0f,
                                Object2.HitBB[j].Size.y / 2.0f,
                                Object2.HitBB[j].Size.z / 2.0f);

            //1 (Ra)x
            if (fabsf(vPosB.x) > (Obj1_data.x + Obj2_data.x * fabsf(XAxis.x) +
                                  Obj2_data.y * fabsf(XAxis.y) +
                                  Obj2_data.z * fabsf(XAxis.z))) {
                continue;
            }

            //2 (Ra)y
            if (fabsf(vPosB.y) > (Obj1_data.y + Obj2_data.x * fabsf(YAxis.x) +
                                  Obj2_data.y * fabsf(YAxis.y) +
                                  Obj2_data.z * fabsf(YAxis.z))) {
                continue;
            }

            //3 (Ra)z
            if (fabsf(vPosB.z) > (Obj1_data.z + Obj2_data.x * fabsf(ZAxis.x) +
                                  Obj2_data.y * fabsf(ZAxis.y) +
                                  Obj2_data.z * fabsf(ZAxis.z))) {
                continue;
            }

            //4 (Rb)x
            if (fabsf(vPosB.x * XAxis.x +
                      vPosB.y * YAxis.x +
                      vPosB.z * ZAxis.x) > (Obj2_data.x + Obj1_data.x * fabsf(XAxis.x) +
                                            Obj1_data.y * fabsf(YAxis.x) +
                                            Obj1_data.z * fabsf(ZAxis.x))) {
                continue;
            }

            //5 (Rb)y
            if (fabsf(vPosB.x * XAxis.y +
                      vPosB.y * YAxis.y +
                      vPosB.z * ZAxis.y) > (Obj2_data.y + Obj1_data.x * fabsf(XAxis.y) +
                                            Obj1_data.y * fabsf(YAxis.y) +
                                            Obj1_data.z * fabsf(ZAxis.y))) {
                continue;
            }

            //6 (Rb)z
            if (fabsf(vPosB.x * XAxis.z +
                      vPosB.y * YAxis.z +
                      vPosB.z * ZAxis.z) > (Obj2_data.z + Obj1_data.x * fabsf(XAxis.z) +
                                            Obj1_data.y * fabsf(YAxis.z) +
                                            Obj1_data.z * fabsf(ZAxis.z))) {
                continue;
            }

            //7 (Ra)x X (Rb)x
            if (fabsf(vPosB.z * YAxis.x - vPosB.y * ZAxis.x) > (Obj1_data.y * fabsf(ZAxis.x) +
                                                                Obj1_data.z * fabsf(YAxis.x) +
                                                                Obj2_data.y * fabsf(XAxis.z) +
                                                                Obj2_data.z * fabsf(XAxis.y))) {
                continue;
            }

            //8 (Ra)x X (Rb)y
            if (fabsf(vPosB.z * YAxis.y - vPosB.y * ZAxis.y) > (Obj1_data.y * fabsf(ZAxis.y) +
                                                                Obj1_data.z * fabsf(YAxis.y) +
                                                                Obj2_data.x * fabsf(XAxis.z) +
                                                                Obj2_data.z * fabsf(XAxis.x))) {
                continue;
            }

            //9 (Ra)x X (Rb)z
            if (fabsf(vPosB.z * YAxis.z - vPosB.y * ZAxis.z) > (Obj1_data.y * fabsf(ZAxis.z) +
                                                                Obj1_data.z * fabsf(YAxis.z) +
                                                                Obj2_data.x * fabsf(XAxis.y) +
                                                                Obj2_data.y * fabsf(XAxis.x))) {
                continue;
            }

            //10 (Ra)y X (Rb)x
            if (fabsf(vPosB.x * ZAxis.x - vPosB.z * XAxis.x) > (Obj1_data.x * fabsf(ZAxis.x) +
                                                                Obj1_data.z * fabsf(XAxis.x) +
                                                                Obj2_data.y * fabsf(YAxis.z) +
                                                                Obj2_data.z * fabsf(YAxis.y))) {
                continue;
            }

            //11 (Ra)y X (Rb)y
            if (fabsf(vPosB.x * ZAxis.y - vPosB.z * XAxis.y) > (Obj1_data.x * fabsf(ZAxis.y) +
                                                                Obj1_data.z * fabsf(XAxis.y) +
                                                                Obj2_data.x * fabsf(YAxis.z) +
                                                                Obj2_data.z * fabsf(YAxis.x))) {
                continue;
            }

            //12 (Ra)y X (Rb)z
            if (fabsf(vPosB.x * ZAxis.z - vPosB.z * XAxis.z) > (Obj1_data.x * fabsf(ZAxis.z) +
                                                                Obj1_data.z * fabsf(XAxis.z) +
                                                                Obj2_data.x * fabsf(YAxis.y) +
                                                                Obj2_data.y * fabsf(YAxis.x))) {
                continue;
            }

            //13 (Ra)z X (Rb)x
            if (fabsf(vPosB.y * XAxis.x - vPosB.x * YAxis.x) > (Obj1_data.x * fabsf(YAxis.x) +
                                                                Obj1_data.y * fabsf(XAxis.x) +
                                                                Obj2_data.y * fabsf(ZAxis.z) +
                                                                Obj2_data.z * fabsf(ZAxis.y))) {
                continue;
            }

            //14 (Ra)z X (Rb)y
            if (fabsf(vPosB.y * XAxis.y - vPosB.x * YAxis.y) > (Obj1_data.x * fabsf(YAxis.y) +
                                                                Obj1_data.y * fabsf(XAxis.y) +
                                                                Obj2_data.x * fabsf(ZAxis.z) +
                                                                Obj2_data.z * fabsf(ZAxis.x))) {
                continue;
            }

            //15 (Ra)z X (Rb)z
            if (fabsf(vPosB.y * XAxis.z - vPosB.x * YAxis.z) > (Obj1_data.x * fabsf(YAxis.z) +
                                                                Obj1_data.y * fabsf(XAxis.z) +
                                                                Obj2_data.x * fabsf(ZAxis.y) +
                                                                Obj2_data.y * fabsf(ZAxis.x))) {
                continue;
            }

            Object1PieceNum = i;
            Object2PieceNum = j;
            return true;
        }
    }

    return false;
}

/*
 * HitBB-OBB collision detection.
 */
bool CheckHitBBOBBCollisionDetection(const cObject3D &Object1, const cObject3D &Object2, int &Object1PieceNum)
{
    for (unsigned int i = 0; i < Object1.Chunks.size(); i++) {
        float TMPOldInvRotationMat[9];
        memcpy(TMPOldInvRotationMat, Object2.CurrentRotationMat, 9 * sizeof(Object2.CurrentRotationMat[0]));
        vw_Matrix33InverseRotate(TMPOldInvRotationMat);

        float matB[9];
        memcpy(matB, Object1.CurrentRotationMat, 9 * sizeof(Object1.CurrentRotationMat[0]));
        vw_Matrix33Mult(matB, TMPOldInvRotationMat);

        sVECTOR3D vPosB = (Object1.Location + Object1.HitBB[i].Location) -
                          (Object2.Location + Object2.OBB.Location);
        vw_Matrix33CalcPoint(vPosB, TMPOldInvRotationMat);

        sVECTOR3D XAxis(matB[0], matB[3], matB[6]);
        sVECTOR3D YAxis(matB[1], matB[4], matB[7]);
        sVECTOR3D ZAxis(matB[2], matB[5], matB[8]);

        sVECTOR3D Obj2_data{Object1.HitBB[i].Size.x / 2.0f,
                            Object1.HitBB[i].Size.y / 2.0f,
                            Object1.HitBB[i].Size.z / 2.0f};
        sVECTOR3D Obj1_data{Object2.Width / 2.0f, Object2.Height / 2.0f, Object2.Length / 2.0f};

        //1 (Ra)x
        if (fabsf(vPosB.x) > (Obj1_data.x + Obj2_data.x * fabsf(XAxis.x) +
                              Obj2_data.y * fabsf(XAxis.y) +
                              Obj2_data.z * fabsf(XAxis.z))) {
            continue;
        }

        //2 (Ra)y
        if (fabsf(vPosB.y) > (Obj1_data.y + Obj2_data.x * fabsf(YAxis.x) +
                              Obj2_data.y * fabsf(YAxis.y) +
                              Obj2_data.z * fabsf(YAxis.z))) {
            continue;
        }

        //3 (Ra)z
        if (fabsf(vPosB.z) > (Obj1_data.z + Obj2_data.x * fabsf(ZAxis.x) +
                              Obj2_data.y * fabsf(ZAxis.y) +
                              Obj2_data.z * fabsf(ZAxis.z))) {
            continue;
        }

        //4 (Rb)x
        if (fabsf(vPosB.x * XAxis.x +
                  vPosB.y * YAxis.x +
                  vPosB.z * ZAxis.x) > (Obj2_data.x + Obj1_data.x * fabsf(XAxis.x) +
                                        Obj1_data.y * fabsf(YAxis.x) +
                                        Obj1_data.z * fabsf(ZAxis.x))) {
            continue;
        }

        //5 (Rb)y
        if (fabsf(vPosB.x * XAxis.y +
                  vPosB.y * YAxis.y +
                  vPosB.z * ZAxis.y) > (Obj2_data.y + Obj1_data.x * fabsf(XAxis.y) +
                                        Obj1_data.y * fabsf(YAxis.y) +
                                        Obj1_data.z * fabsf(ZAxis.y))) {
            continue;
        }

        //6 (Rb)z
        if (fabsf(vPosB.x * XAxis.z +
                  vPosB.y * YAxis.z +
                  vPosB.z * ZAxis.z) > (Obj2_data.z + Obj1_data.x * fabsf(XAxis.z) +
                                        Obj1_data.y * fabsf(YAxis.z) +
                                        Obj1_data.z * fabsf(ZAxis.z))) {
            continue;
        }

        //7 (Ra)x X (Rb)x
        if (fabsf(vPosB.z * YAxis.x - vPosB.y * ZAxis.x) > Obj1_data.y*fabsf(ZAxis.x) +
                                                           Obj1_data.z*fabsf(YAxis.x) +
                                                           Obj2_data.y*fabsf(XAxis.z) +
                                                           Obj2_data.z*fabsf(XAxis.y)) {
            continue;
        }

        //8 (Ra)x X (Rb)y
        if (fabsf(vPosB.z * YAxis.y - vPosB.y * ZAxis.y) > Obj1_data.y*fabsf(ZAxis.y) +
                                                           Obj1_data.z*fabsf(YAxis.y) +
                                                           Obj2_data.x*fabsf(XAxis.z) +
                                                           Obj2_data.z*fabsf(XAxis.x)) {
            continue;
        }

        //9 (Ra)x X (Rb)z
        if (fabsf(vPosB.z * YAxis.z - vPosB.y * ZAxis.z) > Obj1_data.y*fabsf(ZAxis.z) +
                                                           Obj1_data.z*fabsf(YAxis.z) +
                                                           Obj2_data.x*fabsf(XAxis.y) +
                                                           Obj2_data.y*fabsf(XAxis.x)) {
            continue;
        }

        //10 (Ra)y X (Rb)x
        if (fabsf(vPosB.x * ZAxis.x - vPosB.z * XAxis.x) > Obj1_data.x * fabsf(ZAxis.x) +
                                                           Obj1_data.z * fabsf(XAxis.x) +
                                                           Obj2_data.y * fabsf(YAxis.z) +
                                                           Obj2_data.z * fabsf(YAxis.y)) {
            continue;
        }

        //11 (Ra)y X (Rb)y
        if (fabsf(vPosB.x * ZAxis.y - vPosB.z * XAxis.y) > Obj1_data.x * fabsf(ZAxis.y) +
                                                           Obj1_data.z * fabsf(XAxis.y) +
                                                           Obj2_data.x * fabsf(YAxis.z) +
                                                           Obj2_data.z * fabsf(YAxis.x)) {
            continue;
        }

        //12 (Ra)y X (Rb)z
        if (fabsf(vPosB.x * ZAxis.z - vPosB.z * XAxis.z) > Obj1_data.x * fabsf(ZAxis.z) +
                                                           Obj1_data.z * fabsf(XAxis.z) +
                                                           Obj2_data.x * fabsf(YAxis.y) +
                                                           Obj2_data.y * fabsf(YAxis.x)) {
            continue;
        }

        //13 (Ra)z X (Rb)x
        if (fabsf(vPosB.y * XAxis.x - vPosB.x * YAxis.x) > Obj1_data.x * fabsf(YAxis.x) +
                                                           Obj1_data.y * fabsf(XAxis.x) +
                                                           Obj2_data.y * fabsf(ZAxis.z) +
                                                           Obj2_data.z * fabsf(ZAxis.y)) {
            continue;
        }

        //14 (Ra)z X (Rb)y
        if (fabsf(vPosB.y * XAxis.y - vPosB.x * YAxis.y) > Obj1_data.x * fabsf(YAxis.y) +
                                                           Obj1_data.y * fabsf(XAxis.y) +
                                                           Obj2_data.x * fabsf(ZAxis.z) +
                                                           Obj2_data.z * fabsf(ZAxis.x)) {
            continue;
        }

        //15 (Ra)z X (Rb)z
        if (fabsf(vPosB.y * XAxis.z - vPosB.x * YAxis.z) > Obj1_data.x * fabsf(YAxis.z) +
                                                           Obj1_data.y * fabsf(XAxis.z) +
                                                           Obj2_data.x * fabsf(ZAxis.y) +
                                                           Obj2_data.y * fabsf(ZAxis.x)) {
            continue;
        }

        Object1PieceNum = i;
        return true;
    }

    return false;
}

/*
 * HitBB-Mesh collision detection.
 */
bool CheckHitBBMeshCollisionDetection(const cObject3D &Object1, const cObject3D &Object2, int &Object1PieceNum)
{
    float TransMat[16]{
        Object2.CurrentRotationMat[0], Object2.CurrentRotationMat[1], Object2.CurrentRotationMat[2], 0.0f,
        Object2.CurrentRotationMat[3], Object2.CurrentRotationMat[4], Object2.CurrentRotationMat[5], 0.0f,
        Object2.CurrentRotationMat[6], Object2.CurrentRotationMat[7], Object2.CurrentRotationMat[8], 0.0f,
        Object2.Location.x,            Object2.Location.y,            Object2.Location.z,            1.0f};

    for (unsigned int i = 0; i < Object1.Chunks.size(); i++) {
        sVECTOR3D TMPMax = Object1.HitBB[i].Box[0];
        sVECTOR3D TMPMin = Object1.HitBB[i].Box[6];

        float TMPOldInvRotationMat[9];
        memcpy(TMPOldInvRotationMat, Object1.CurrentRotationMat, 9 * sizeof(Object1.CurrentRotationMat[0]));
        vw_Matrix33InverseRotate(TMPOldInvRotationMat);

        vw_Matrix33CalcPoint(TMPMax, TMPOldInvRotationMat);
        vw_Matrix33CalcPoint(TMPMin, TMPOldInvRotationMat);

        sVECTOR3D Center = Object1.HitBB[i].Location + Object1.Location;
        vw_Matrix33CalcPoint(Center, TMPOldInvRotationMat);

        for (unsigned int j = 0; j < Object2.Chunks.size(); j++) {
            sVECTOR3D LocalLocation(Object2.Chunks[j].Location);
            vw_Matrix33CalcPoint(LocalLocation, Object2.CurrentRotationMat);

            float ObjTransMat[16];
            memcpy(ObjTransMat, TransMat, 16 * sizeof(TransMat[0]));

            if (Object2.Chunks[j].Rotation.x != 0.0f ||
                Object2.Chunks[j].Rotation.y != 0.0f ||
                Object2.Chunks[j].Rotation.z != 0.0f) {
                float TransMatTMP[16];
                vw_Matrix44Identity(TransMatTMP);

                vw_Matrix44CreateRotate(TransMatTMP, Object2.Chunks[j].Rotation);

                vw_Matrix44Translate(TransMatTMP, LocalLocation);
                vw_Matrix44Mult(ObjTransMat, TransMatTMP);
            } else {
                vw_Matrix44Translate(ObjTransMat, LocalLocation);
            }

            for (unsigned int k = 0; k < Object2.Chunks[j].VertexQuantity; k+=3) {
                int j2;
                if (Object2.Chunks[j].IndexArray) {
                    j2 = Object2.Chunks[j].IndexArray.get()[Object2.Chunks[j].RangeStart + k]
                         * Object2.Chunks[j].VertexStride;
                } else {
                    j2 = (Object2.Chunks[j].RangeStart + k) * Object2.Chunks[j].VertexStride;
                }

                sVECTOR3D Point1;
                Point1.x = Object2.Chunks[j].VertexArray.get()[j2];
                Point1.y = Object2.Chunks[j].VertexArray.get()[j2 + 1];
                Point1.z = Object2.Chunks[j].VertexArray.get()[j2 + 2];
                vw_Matrix44CalcPoint(Point1, ObjTransMat);

                if (Object2.Chunks[j].IndexArray) {
                    j2 = Object2.Chunks[j].IndexArray.get()[Object2.Chunks[j].RangeStart + k + 1]
                         * Object2.Chunks[j].VertexStride;
                } else {
                    j2 = (Object2.Chunks[j].RangeStart + k + 1) * Object2.Chunks[j].VertexStride;
                }

                sVECTOR3D Point2;
                Point2.x = Object2.Chunks[j].VertexArray.get()[j2];
                Point2.y = Object2.Chunks[j].VertexArray.get()[j2 + 1];
                Point2.z = Object2.Chunks[j].VertexArray.get()[j2 + 2];
                vw_Matrix44CalcPoint(Point2, ObjTransMat);

                if (Object2.Chunks[j].IndexArray) {
                    j2 = Object2.Chunks[j].IndexArray.get()[Object2.Chunks[j].RangeStart + k + 2] *
                         Object2.Chunks[j].VertexStride;
                } else {
                    j2 = (Object2.Chunks[j].RangeStart + k + 2) * Object2.Chunks[j].VertexStride;
                }

                sVECTOR3D Point3;
                Point3.x = Object2.Chunks[j].VertexArray.get()[j2];
                Point3.y = Object2.Chunks[j].VertexArray.get()[j2 + 1];
                Point3.z = Object2.Chunks[j].VertexArray.get()[j2 + 2];
                vw_Matrix44CalcPoint(Point3, ObjTransMat);

                vw_Matrix33CalcPoint(Point1, TMPOldInvRotationMat);
                vw_Matrix33CalcPoint(Point2, TMPOldInvRotationMat);
                vw_Matrix33CalcPoint(Point3, TMPOldInvRotationMat);

                Object1PieceNum = i;

                if (fabsf(Center.x - Point1.x) <= TMPMax.x &&
                    fabsf(Center.y - Point1.y) <= TMPMax.y &&
                    fabsf(Center.z - Point1.z) <= TMPMax.z) {
                    return true;
                }
                if (fabsf(Center.x - Point2.x) <= TMPMax.x &&
                    fabsf(Center.y - Point2.y) <= TMPMax.y &&
                    fabsf(Center.z - Point2.z) <= TMPMax.z) {
                    return true;
                }
                if (fabsf(Center.x - Point3.x) <= TMPMax.x &&
                    fabsf(Center.y - Point3.y) <= TMPMax.y &&
                    fabsf(Center.z - Point3.z) <= TMPMax.z) {
                    return true;
                }

                sVECTOR3D TMPMax2;
                sVECTOR3D TMPMin2;
                TMPMax2.x = std::max({Point1.x, Point2.x, Point3.x});
                TMPMin2.x = std::min({Point1.x, Point2.x, Point3.x});

                TMPMax2.y = std::max({Point1.y, Point2.y, Point3.y});
                TMPMin2.y = std::min({Point1.y, Point2.y, Point3.y});

                TMPMax2.z = std::max({Point1.z, Point2.z, Point3.z});
                TMPMin2.z = std::min({Point1.z, Point2.z, Point3.z});

                sVECTOR3D Center2((TMPMax2.x + TMPMin2.x) / 2.0f,
                                  (TMPMax2.y + TMPMin2.y) / 2.0f,
                                  (TMPMax2.z + TMPMin2.z) / 2.0f);

                TMPMax2.x = TMPMax2.x - Center2.x;
                TMPMax2.y = TMPMax2.y - Center2.y;
                TMPMax2.z = TMPMax2.z - Center2.z;

                if (fabsf(Center.x - Center2.x) > TMPMax.x + TMPMax2.x) {
                    continue;
                }
                if (fabsf(Center.y - Center2.y) > TMPMax.y + TMPMax2.y) {
                    continue;
                }
                if (fabsf(Center.z - Center2.z) > TMPMax.z + TMPMax2.z) {
                    continue;
                }

                return true;
            }
        }
    }

    return false;
}

} // astromenace namespace
} // viewizard namespace
