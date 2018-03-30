/************************************************************************************

	AstroMenace (Hardcore 3D space shooter with spaceship upgrade possibilities)
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
	along with AstroMenace. If not, see <http://www.gnu.org/licenses/>.


	Web Site: http://www.viewizard.com/
	Project: https://github.com/viewizard/astromenace
	E-mail: viewizard@viewizard.com

*************************************************************************************/

#include "../game.h"
#include "script.h"
#include "../object3d/object3d.h"
#include "../object3d/space_ship/space_ship.h"
#include "../object3d/ground_object/ground_object.h"
#include "../object3d/space_object/asteroid/asteroid.h"
#include "../object3d/space_object/planet/planet.h"
#include "../object3d/space_ship/earth_space_fighter/earth_space_fighter.h"
#include "../object3d/space_ship/alien_space_fighter/alien_space_fighter.h"
#include "../object3d/space_ship/alien_space_mothership/alien_space_mothership.h"
#include "../object3d/space_ship/pirate_ship/pirate_ship.h"
#include "../object3d/space_object/base_part/base_part.h"
#include "../object3d/space_object/big_asteroid/big_asteroid.h"
#include "../object3d/ground_object/military_building/military_building.h"
#include "../object3d/ground_object/building/building.h"
#include "../object3d/ground_object/wheeled/wheeled.h"
#include "../object3d/ground_object/tracked/tracked.h"
#include "../gfx/game_level_text/game_level_text.h"


void SetID(cObject3D *Object, sXMLEntry *xmlEntry, cXMLDocument *xmlDoc);
void SetDebugInformation(cObject3D *Object, sXMLEntry *xmlEntry);
void SetShowDeleteOnHide(cObject3D *Object, sXMLEntry *xmlEntry, cXMLDocument *xmlDoc);
void SetShipRotation(cSpaceShip *Object, sXMLEntry *xmlEntry, cXMLDocument *xmlDoc);
void SetShipLocation(cSpaceShip *Object, sXMLEntry *xmlEntry, cXMLDocument *xmlDoc, float TimeOpLag);
void SetProjectileRotation(cProjectile *Object, sXMLEntry *xmlEntry, cXMLDocument *xmlDoc);
void SetProjectileLocation(cProjectile *Object, sXMLEntry *xmlEntry, cXMLDocument *xmlDoc, float TimeOpLag);
void SetRotation(cObject3D *Object, sXMLEntry *xmlEntry, cXMLDocument *xmlDoc);
void SetLocation(cObject3D *Object, sXMLEntry *xmlEntry, cXMLDocument *xmlDoc, float TimeOpLag);
void SetAIMode(cObject3D *Object, sXMLEntry *xmlEntry, cXMLDocument *xmlDoc);




// проверка, если конец уровня как всех убъем

extern cSpaceShip *StartSpaceShip;
extern cSpaceShip *EndSpaceShip;
extern cGroundObject *StartGroundObject;
extern cGroundObject *EndGroundObject;

// отображение коробок... отладка
extern int NeedShowBB;
// неубиваемость... отладка
extern bool UndeadDebugMode;
// показывать время при скорости 1.5
extern bool ShowGameTime;






cScriptEngine::cScriptEngine()
{
	// отладочный режим
	NeedShowBB = 0;
	UndeadDebugMode = false;
};

cScriptEngine::~cScriptEngine()
{
	delete xmlDoc;
};



//-----------------------------------------------------------------------------
// запустить скрипт на выполнение
//-----------------------------------------------------------------------------
bool cScriptEngine::RunScript(const char *FileName, float InitTime)
{
	// установка значений
	StartTime = TimeLastOp = InitTime;

	TimeOpLag = 0;
	xmlEntry = nullptr;

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



	// отладочный режим
	ShowDebugModeLine = false;
	NeedShowBB = 0;
	UndeadDebugMode = false;
	ShowGameTime = false;


	if (xmlDoc != nullptr) {
		delete xmlDoc;
		xmlDoc = nullptr;
	}
	xmlDoc = new cXMLDocument(FileName);

	// проверяем корневой элемент
	if (!xmlDoc->GetRootEntry() || ("AstroMenaceScript" != xmlDoc->GetRootEntry()->Name)) {
		std::cerr << __func__ << "(): " << "Can't find AstroMenaceScript element in the: " << FileName << "\n";
		delete xmlDoc;
		xmlDoc = nullptr;
		return false;
	}


	// переходим на действия
	xmlEntry  = xmlDoc->FindEntryByName(xmlDoc->GetRootEntry(), "Action");
	if (xmlEntry == nullptr) {
		std::cerr << __func__ << "(): " << "Can't find Action element in the: " << FileName << "\n";
		delete xmlDoc;
		xmlDoc = nullptr;
		return false;
	}

	xmlEntry = xmlEntry->FirstChild;
	if (xmlEntry == nullptr) {
		std::cerr << __func__ << "(): " << "Can't find Action element in the: " << FileName << "\n";
		delete xmlDoc;
		xmlDoc = nullptr;
		return false;
	}





	// идем и выполняем то, что по времени 0 стоит, т.е. начальную инициализацию
	Update(StartTime);
	return true;
}






//-----------------------------------------------------------------------------
// проверяем скрипт
//-----------------------------------------------------------------------------
bool cScriptEngine::Update(float Time)
{
	// скрипт не загружен
	if (xmlDoc == nullptr)
		return false;

	// находим дельту времени
	float TimeDelta = Time - TimeLastOp;


	// генерация астероидов
	if (AsterOn) {
		if (AsterLastTime == -1.0) AsterLastTime = Time;
		float AsterTimeDelta = Time - AsterLastTime;
		AsterLastTime = Time;

		// складываем все
		float NeedGener = AsterQuant*AsterTimeDelta+AsterRealNeed;
		// получаем целое кол-во на генерацию
		unsigned int NeedGenerInt = (unsigned int)NeedGener;
		// находим остаток... который нужно будет потом учесть
		AsterRealNeed = NeedGener - NeedGenerInt;

		while (NeedGenerInt>0) {
			cAsteroid *CreateAsteroid = new cAsteroid;
			CreateAsteroid->Create(1);
			if (AsterFastCount != 20)
				CreateAsteroid->Speed = AsterMaxSpeed*vw_Randf1;
			else
				CreateAsteroid->Speed = AsterMinFastSpeed + AsterMaxSpeed*vw_Randf1;
			CreateAsteroid->ShowDeleteOnHide = 0;
			CreateAsteroid->SetRotation(sVECTOR3D(0.0f, 180.0f, 0.0f));// !!!учесть камеру

			if (AsterFastCount != 20)
				CreateAsteroid->SetLocation(
					sVECTOR3D(AsterW * vw_Randf0 + AsterXPos, AsterYPos * 2 + AsterH * vw_Randf1, AsterZPos + 20.0f)
					+GamePoint);
			else
				CreateAsteroid->SetLocation(
					sVECTOR3D(AsterW * vw_Randf0 + AsterXPos, AsterYPos * 2 + AsterH * vw_Randf1, AsterZPos)
					+GamePoint);

			NeedGenerInt--;
		}

		AsterFastCount++;
		if (AsterFastCount > 30) AsterFastCount = 0;
	}



	// если нужно, смотрим когда заканчивать миссию
	if ((EndDelayMissionComplete > 0.0f) || NeedCheckSpaceShip || NeedCheckGroundObject) {
		if (LastTimeMissionComplete == -1.0) LastTimeMissionComplete = Time;
		float TimeDeltaMissionComplete = Time - LastTimeMissionComplete;
		LastTimeMissionComplete = Time;

		// уменьшаем только тогда, когда уже никого нет... т.е. отступ это от смерти последнего
		bool NeedDecrease = true;


		// собственно сами проверки...
		if (NeedCheckSpaceShip) {
			// если нет врагов
			int count = 0;
			cSpaceShip *Tmp1 = StartSpaceShip;
			while (Tmp1 != nullptr) {
				// если это враг, и мы его показали (иначе он где-то там летает)
				if ((Tmp1->ObjectStatus == 1) && (Tmp1->ShowDeleteOnHide != 0))
					count++;
				Tmp1 = Tmp1->Next;
			}
			if (count > 0) NeedDecrease = false;

		}
		if (NeedCheckGroundObject) {
			// если нет врагов, которых можем уничтожить
			int count = 0;
			cGroundObject *Tmp1 = StartGroundObject;
			while (Tmp1 != nullptr) {
				// если это враг, и мы его показали
				if (NeedCheckCollision(Tmp1) && (Tmp1->ShowDeleteOnHide != 0))
					count++;
				Tmp1 = Tmp1->Next;
			}
			if (count > 0)
				NeedDecrease = false;
		}


		if (NeedDecrease) EndDelayMissionComplete -= TimeDeltaMissionComplete;
		if (EndDelayMissionComplete<=0.0f) {
			EndDelayMissionComplete = 0.0f;
			NeedCheckSpaceShip = false;
			NeedCheckGroundObject = false;
			SetGameMissionComplete();
			return false;
		}

		// говорим, что скрипт еще не закончился!
		return true;
	}






	while (xmlEntry) {
		// TimeLine
		if (xmlEntry->Name == "TimeLine") {
			float onTime = 0.0f;
			xmlDoc->fGetEntryAttribute(*xmlEntry, "value", onTime);

			// если еще не время выполнять... нужно уйти из процедуры
			if (onTime > TimeDelta) return true;

			// считаем лаг, чтобы правильно вычеслить положение при создании
			TimeOpLag = TimeDelta - onTime;

			// дальше смотрим, что нужно сделать...
			UpdateTimeLine();

			TimeLastOp = Time - TimeOpLag;
			TimeDelta = TimeOpLag;//Time - TimeLastOp;
		} else
			// Debug
			if (xmlEntry->Name == "Debug") {
				ShowDebugModeLine = false;
				xmlDoc->bGetEntryAttribute(*xmlEntry, "showline", ShowDebugModeLine);

				NeedShowBB = 0;
				if (xmlDoc->TestEntryAttribute(xmlEntry, "showbb"))
					NeedShowBB = xmlDoc->iGetEntryAttribute(xmlEntry, "showbb");

				UndeadDebugMode = false;
				xmlDoc->bGetEntryAttribute(*xmlEntry, "undead", UndeadDebugMode);

				ShowGameTime = false;
				xmlDoc->bGetEntryAttribute(*xmlEntry, "time", ShowGameTime);

			} else
				// StarSystem
				if (xmlEntry->Name == "StarSystem") {
					if (xmlDoc->TestEntryAttribute(xmlEntry, "system")) {
						int SystemNum = xmlDoc->iGetEntryAttribute(xmlEntry, "system");
						sVECTOR3D TmpBaseRotation(0.0f, 0.0f, 0.0f);
						xmlDoc->fGetEntryAttribute(*xmlEntry, "anglex", TmpBaseRotation.x);
						xmlDoc->fGetEntryAttribute(*xmlEntry, "angley", TmpBaseRotation.y);
						xmlDoc->fGetEntryAttribute(*xmlEntry, "anglez", TmpBaseRotation.z);
						StarSystemInit(SystemNum, TmpBaseRotation);
					}
				} else
					// Music
					if (xmlEntry->Name == "Music") {
						// если корабль игрока уничтожен - не меняем уже музыку в игре вообще,
						// должна проигрываться только музыка поражения
						if (PlayerFighter != nullptr)
							if (PlayerFighter->Strength > 0.0f) {
								if (xmlDoc->TestEntryAttribute(xmlEntry, "theme")) {
									int Theme = xmlDoc->iGetEntryAttribute(xmlEntry, "theme");
									if (Theme == 1)
										StartMusicWithFade(eMusicTheme::GAME, 2000, 2000);
									if (Theme == 2)
										StartMusicWithFade(eMusicTheme::BOSS, 1500, 2000);
								}
							}
					} else
						// CreatePlanet
						if (xmlEntry->Name == "CreatePlanet") {
							cPlanet *Planet = new cPlanet;
							if (xmlDoc->TestEntryAttribute(xmlEntry, "type")) {
								Planet->Create(xmlDoc->iGetEntryAttribute(xmlEntry, "type"));
								SetRotation(Planet, xmlEntry, xmlDoc);
								SetLocation(Planet, xmlEntry, xmlDoc, 0.0f);
								Planet->ShowDeleteOnHide = 0;

								xmlDoc->fGetEntryAttribute(*xmlEntry, "speed", Planet->Speed);
							}
						} else
							// AsteroidField
							if (xmlEntry->Name == "AsteroidField") {
								xmlDoc->bGetEntryAttribute(*xmlEntry, "status", AsterOn);
								xmlDoc->fGetEntryAttribute(*xmlEntry, "persec", AsterQuant);
								xmlDoc->fGetEntryAttribute(*xmlEntry, "w", AsterW);
								xmlDoc->fGetEntryAttribute(*xmlEntry, "h", AsterH);
								xmlDoc->fGetEntryAttribute(*xmlEntry, "posx", AsterXPos);
								xmlDoc->fGetEntryAttribute(*xmlEntry, "posy", AsterYPos);
								xmlDoc->fGetEntryAttribute(*xmlEntry, "posz", AsterZPos);
								xmlDoc->fGetEntryAttribute(*xmlEntry, "slow", AsterMaxSpeed);
								xmlDoc->fGetEntryAttribute(*xmlEntry, "fast", AsterMinFastSpeed);
							} else
								// Light
								if (xmlEntry->Name == "Light") {
									eLightType LightType{eLightType::Directional}; // по умолчанию, солнце
									if (xmlDoc->TestEntryAttribute(xmlEntry, "type"))
										if (xmlDoc->iGetEntryAttribute(xmlEntry, "type") == 1)
											LightType = eLightType::Point;

									sLight *NewLight = vw_CreateLight(LightType);

									NewLight->Diffuse[0] = 0.0f;
									NewLight->Diffuse[1] = 0.0f;
									NewLight->Diffuse[2] = 0.0f;
									NewLight->Diffuse[3] = 1.0f;
									xmlDoc->fGetEntryAttribute(*xmlEntry, "diffr", NewLight->Diffuse[0]);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "diffg", NewLight->Diffuse[1]);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "diffb", NewLight->Diffuse[2]);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "diffa", NewLight->Diffuse[3]);

									NewLight->Specular[0] = 0.0f;
									NewLight->Specular[1] = 0.0f;
									NewLight->Specular[2] = 0.0f;
									NewLight->Specular[3] = 1.0f;
									xmlDoc->fGetEntryAttribute(*xmlEntry, "specr", NewLight->Specular[0]);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "specg", NewLight->Specular[1]);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "specb", NewLight->Specular[2]);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "speca", NewLight->Specular[3]);

									NewLight->Ambient[0] = 0.0f;
									NewLight->Ambient[1] = 0.0f;
									NewLight->Ambient[2] = 0.0f;
									NewLight->Ambient[3] = 1.0f;
									xmlDoc->fGetEntryAttribute(*xmlEntry, "ambir", NewLight->Ambient[0]);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "ambig", NewLight->Ambient[1]);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "ambib", NewLight->Ambient[2]);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "ambia", NewLight->Ambient[3]);

									NewLight->Direction = sVECTOR3D(0.0f,0.0f,1.0f);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "dirx", NewLight->Direction.x);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "diry", NewLight->Direction.y);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "dirz", NewLight->Direction.z);
									NewLight->Direction.Normalize();

									NewLight->Location = sVECTOR3D(0.0f,0.0f,0.0f);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "posx", NewLight->Location.x);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "posy", NewLight->Location.y);
									xmlDoc->fGetEntryAttribute(*xmlEntry, "posz", NewLight->Location.z);


									NewLight->On = true;
									xmlDoc->bGetEntryAttribute(*xmlEntry, "status", NewLight->On);
								} else
									// Label
									if (xmlEntry->Name == "Label") {
										// ничего не делаем
										xmlEntry = xmlEntry->Next;
										continue;
									} else
										// Goto
										if (xmlEntry->Name == "Goto") {
											// если есть указатель на метку
											if (xmlDoc->TestEntryAttribute(xmlEntry, "label")) {
												// нужно перебрать все метки и остановится на нужной
												sXMLEntry *tmpEntry = xmlDoc->FindEntryByName(xmlDoc->FindEntryByName(xmlDoc->GetRootEntry(), "Action"), "Label");

												// перебор по всем меткам
												while (tmpEntry) {
													//	if (!strcmp(tmpEntry->Name, "Label"))
													if (xmlDoc->TestEntryAttribute(tmpEntry, "name"))
														if (xmlDoc->GetEntryAttribute(tmpEntry, "name") == xmlDoc->GetEntryAttribute(xmlEntry, "label")) {
															// ставим новый указатель
															xmlEntry = tmpEntry;
															return true;
														}
													// берем следующую метку
													tmpEntry = tmpEntry->Next;
												}
											}
										} else
											// MissionComplete
											if (xmlEntry->Name == "MissionComplete") {
												SetGameMissionComplete();
											} else
												// MissionCompleteAtNoEnemy
												if (xmlEntry->Name == "MissionCompleteAtNoEnemy") {
													bool SetGameMissionFlag = false;
													NeedCheckSpaceShip = false;
													if (xmlDoc->bGetEntryAttribute(*xmlEntry, "ships", NeedCheckSpaceShip))
														SetGameMissionFlag = true;
													NeedCheckGroundObject = false;
													if (xmlDoc->bGetEntryAttribute(*xmlEntry, "grounds", NeedCheckGroundObject))
														SetGameMissionFlag = true;
													EndDelayMissionComplete = 0.0f;
													if (xmlDoc->fGetEntryAttribute(*xmlEntry, "delay", EndDelayMissionComplete))
														SetGameMissionFlag = true;

													if (!SetGameMissionFlag) {
														// если время не выставили и нечего ждать, работаем как и с обычным завершением
														SetGameMissionComplete();
													} else {
														LastTimeMissionComplete = Time;
														return true;
													}
												} else
													// Text
													if (xmlEntry->Name == "Text") {
														cGameLvlText *NewText;
														NewText = new cGameLvlText;


														NewText->Lifetime = -1.0f;
														xmlDoc->fGetEntryAttribute(*xmlEntry, "life", NewText->Lifetime);

														if (xmlDoc->TestEntryAttribute(xmlEntry, "text")) {
															NewText->DrawText = new char[xmlDoc->GetEntryAttribute(xmlEntry, "text").size() + 1];
															strcpy(NewText->DrawText, xmlDoc->GetEntryAttribute(xmlEntry, "text").c_str());
														}

														if (xmlDoc->TestEntryAttribute(xmlEntry, "posx"))
															NewText->PosX = xmlDoc->iGetEntryAttribute(xmlEntry, "posx");
														if (xmlDoc->TestEntryAttribute(xmlEntry, "posy"))
															NewText->PosY = xmlDoc->iGetEntryAttribute(xmlEntry, "posy");

														NewText->Color = 0;
														if (xmlDoc->TestEntryAttribute(xmlEntry, "color"))
															NewText->Color = xmlDoc->iGetEntryAttribute(xmlEntry, "color");
													} else {
														// если тут - значит не нашли директиву, или произошла ошибка
														std::cerr << __func__ << "(): " << "ScriptEngine: tag " << xmlEntry->Name
															  << " not found, line " << xmlEntry->LineNumber << "\n";
													}

		// берем следующий элемент по порядку
		xmlEntry = xmlEntry->Next;
	}

	// выходим, скрипт закончился
	return false;
}









//-----------------------------------------------------------------------------
// проверяем скрипт дополнительно для TimeLine
//-----------------------------------------------------------------------------
void cScriptEngine::UpdateTimeLine()
{
	// получаем первый тэг
	sXMLEntry *TL = xmlEntry->FirstChild;

	while (TL) {

		// EarthFighter
		if (TL->Name == "EarthFighter") {
			cEarthSpaceFighter *Fighter = nullptr;
			Fighter = new cEarthSpaceFighter;
			if (xmlDoc->TestEntryAttribute(TL, "type"))
				Fighter->Create(xmlDoc->iGetEntryAttribute(TL, "type"));
			else {
				TL = TL->Next;
				continue;
			}

			SetID(Fighter, TL, xmlDoc);
			if (ShowDebugModeLine) SetDebugInformation(Fighter, TL);

			if (xmlDoc->fGetEntryAttribute(*TL, "speed", Fighter->NeedSpeed))
				Fighter->Speed = Fighter->NeedSpeed;
			if (xmlDoc->fGetEntryAttribute(*TL, "speedlr", Fighter->NeedSpeedLR))
				Fighter->SpeedLR = Fighter->NeedSpeedLR;
			if (xmlDoc->fGetEntryAttribute(*TL, "speedud", Fighter->NeedSpeedUD))
				Fighter->SpeedUD = Fighter->NeedSpeedUD;

			if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamfb", Fighter->NeedSpeedByCamFB))
				Fighter->SpeedByCamFB = Fighter->NeedSpeedByCamFB;
			if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamlr", Fighter->NeedSpeedByCamLR))
				Fighter->SpeedByCamLR = Fighter->NeedSpeedByCamLR;
			if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamud", Fighter->NeedSpeedByCamUD))
				Fighter->SpeedByCamUD = Fighter->NeedSpeedByCamUD;


			if (xmlDoc->TestEntryAttribute(TL, "armour"))
				SetEarthSpaceFighterArmour(Fighter, xmlDoc->iGetEntryAttribute(TL, "armour"));
			if (xmlDoc->TestEntryAttribute(TL, "weapon1"))
				SetEarthSpaceFighterWeapon(Fighter, 1, xmlDoc->iGetEntryAttribute(TL, "weapon1"));
			if (xmlDoc->TestEntryAttribute(TL, "weapon2"))
				SetEarthSpaceFighterWeapon(Fighter, 2, xmlDoc->iGetEntryAttribute(TL, "weapon2"));
			if (xmlDoc->TestEntryAttribute(TL, "weapon3"))
				SetEarthSpaceFighterWeapon(Fighter, 3, xmlDoc->iGetEntryAttribute(TL, "weapon3"));
			if (xmlDoc->TestEntryAttribute(TL, "weapon4"))
				SetEarthSpaceFighterWeapon(Fighter, 4, xmlDoc->iGetEntryAttribute(TL, "weapon4"));
			if (xmlDoc->TestEntryAttribute(TL, "weapon5"))
				SetEarthSpaceFighterWeapon(Fighter, 5, xmlDoc->iGetEntryAttribute(TL, "weapon5"));
			if (xmlDoc->TestEntryAttribute(TL, "weapon6"))
				SetEarthSpaceFighterWeapon(Fighter, 6, xmlDoc->iGetEntryAttribute(TL, "weapon6"));

			SetShowDeleteOnHide(Fighter, TL, xmlDoc);
			SetAIMode(Fighter, TL, xmlDoc); // на тот случае если просто ставим и все...
			SetShipRotation(Fighter, TL, xmlDoc);
			SetShipLocation(Fighter, TL, xmlDoc, TimeOpLag);

			// дальше смотрим, что нужно сделать...
			sXMLEntry *TLEarthFighter = TL->FirstChild;
			while (TLEarthFighter) {
				if (TLEarthFighter->Name == "TimeSheet") {
					// собираем новый элемент
					sTimeSheet *TimeSheet;
					TimeSheet = new sTimeSheet;
					Fighter->AttachTimeSheet(TimeSheet);

					if (xmlDoc->TestEntryAttribute(TLEarthFighter, "aimode")) {
						TimeSheet->AI_Mode = xmlDoc->iGetEntryAttribute(TLEarthFighter, "aimode");
						TimeSheet->Time = 0.0f;
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "time", TimeSheet->Time);

						TimeSheet->InUse = false;

						TimeSheet->Speed = 0.0f;
						TimeSheet->Acceler = 1.0f;//0-1
						TimeSheet->SpeedLR = 0.0f;
						TimeSheet->AccelerLR = 1.0f;//0-1
						TimeSheet->SpeedUD = 0.0f;
						TimeSheet->AccelerUD = 1.0f;//0-1
						TimeSheet->SpeedByCamFB = 0.0f;
						TimeSheet->AccelerByCamFB = 1.0f;//0-1
						TimeSheet->SpeedByCamLR = 0.0f;
						TimeSheet->AccelerByCamLR = 1.0f;//0-1
						TimeSheet->SpeedByCamUD = 0.0f;
						TimeSheet->AccelerByCamUD = 1.0f;//0-1
						TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
						TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
						TimeSheet->Fire = false;
						TimeSheet->BossFire = false;
						TimeSheet->Targeting = false;
					} else {
						TimeSheet->AI_Mode = 0;

						TimeSheet->Time = 0.0f;
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "time", TimeSheet->Time);
						TimeSheet->InUse = false;

						TimeSheet->Speed = 0.0f;
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "speed", TimeSheet->Speed);

						TimeSheet->Acceler = 1.0f;//0-1
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "acceler", TimeSheet->Acceler);
						vw_Clamp(TimeSheet->Acceler, 0.0f, 1.0f);

						TimeSheet->SpeedLR = 0.0f;
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "speedlr", TimeSheet->SpeedLR);

						TimeSheet->AccelerLR = 1.0f;//0-1
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "accelerlr", TimeSheet->AccelerLR);
						vw_Clamp(TimeSheet->AccelerLR, 0.0f, 1.0f);

						TimeSheet->SpeedUD = 0.0f;
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "speedud", TimeSheet->SpeedUD);

						TimeSheet->AccelerUD = 1.0f;//0-1
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "accelerud", TimeSheet->AccelerUD);
						vw_Clamp(TimeSheet->AccelerUD, 0.0f, 1.0f);


						TimeSheet->SpeedByCamFB = 0.0f;
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "speedbycamfb", TimeSheet->SpeedByCamFB);

						TimeSheet->AccelerByCamFB = 1.0f;//0-1
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "accelerbycamfb", TimeSheet->AccelerByCamFB);
						vw_Clamp(TimeSheet->AccelerByCamFB, 0.0f, 1.0f);

						TimeSheet->SpeedByCamLR = 0.0f;
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "speedbycamlr", TimeSheet->SpeedByCamLR);

						TimeSheet->AccelerByCamLR = 1.0f;//0-1
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "accelerbycamlr", TimeSheet->AccelerByCamLR);
						vw_Clamp(TimeSheet->AccelerByCamLR, 0.0f, 1.0f);

						TimeSheet->SpeedByCamUD = 0.0f;
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "speedbycamud", TimeSheet->SpeedByCamUD);

						TimeSheet->AccelerByCamUD = 1.0f;//0-1
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "accelerbycamud", TimeSheet->AccelerByCamUD);
						vw_Clamp(TimeSheet->AccelerByCamUD, 0.0f, 1.0f);


						TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "rotx", TimeSheet->Rotation.x);
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "roty", TimeSheet->Rotation.y);
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "rotz", TimeSheet->Rotation.z);

						TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "rotacx", TimeSheet->RotationAcceler.x);
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "rotacy", TimeSheet->RotationAcceler.y);
						xmlDoc->fGetEntryAttribute(*TLEarthFighter, "rotacz", TimeSheet->RotationAcceler.z);
						vw_Clamp(TimeSheet->RotationAcceler.x, 0.0f, 1.0f);
						vw_Clamp(TimeSheet->RotationAcceler.y, 0.0f, 1.0f);
						vw_Clamp(TimeSheet->RotationAcceler.z, 0.0f, 1.0f);

						TimeSheet->Fire = false;
						if (xmlDoc->TestEntryAttribute(TLEarthFighter, "fire"))
							if (xmlDoc->iGetEntryAttribute(TLEarthFighter, "fire") > 0)
								TimeSheet->Fire = true;
						TimeSheet->BossFire = false;
						TimeSheet->Targeting = false;
					}

				}

				// берем следующий элемент
				TLEarthFighter = TLEarthFighter->Next;
			}
		} else
			// AlienFighter
			if (TL->Name == "AlienFighter") {
				cAlienSpaceFighter *Fighter = new cAlienSpaceFighter;
				if (xmlDoc->TestEntryAttribute(TL, "type"))
					Fighter->Create(xmlDoc->iGetEntryAttribute(TL, "type"));
				else {
					TL = TL->Next;
					continue;
				}

				SetID(Fighter, TL, xmlDoc);
				if (ShowDebugModeLine) SetDebugInformation(Fighter, TL);

				if (xmlDoc->fGetEntryAttribute(*TL, "speed", Fighter->NeedSpeed))
					Fighter->Speed = Fighter->NeedSpeed;
				if (xmlDoc->fGetEntryAttribute(*TL, "speedlr", Fighter->NeedSpeedLR))
					Fighter->SpeedLR = Fighter->NeedSpeedLR;
				if (xmlDoc->fGetEntryAttribute(*TL, "speedud", Fighter->NeedSpeedUD))
					Fighter->SpeedUD = Fighter->NeedSpeedUD;

				if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamfb", Fighter->NeedSpeedByCamFB))
					Fighter->SpeedByCamFB = Fighter->NeedSpeedByCamFB;
				if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamlr", Fighter->NeedSpeedByCamLR))
					Fighter->SpeedByCamLR = Fighter->NeedSpeedByCamLR;
				if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamud", Fighter->NeedSpeedByCamUD))
					Fighter->SpeedByCamUD = Fighter->NeedSpeedByCamUD;

				SetShowDeleteOnHide(Fighter, TL, xmlDoc);
				SetAIMode(Fighter, TL, xmlDoc); // на тот случае если просто ставим и все...
				SetShipRotation(Fighter, TL, xmlDoc);
				SetShipLocation(Fighter, TL, xmlDoc, TimeOpLag);

				// дальше смотрим, что нужно сделать...
				sXMLEntry *TLAlienFighter = TL->FirstChild;
				while (TLAlienFighter) {
					if (TLAlienFighter->Name == "TimeSheet") {
						// собираем новый элемент
						sTimeSheet *TimeSheet;
						TimeSheet = new sTimeSheet;
						Fighter->AttachTimeSheet(TimeSheet);

						if (xmlDoc->TestEntryAttribute(TLAlienFighter, "aimode")) {
							TimeSheet->AI_Mode = xmlDoc->iGetEntryAttribute(TLAlienFighter, "aimode");
							TimeSheet->Time = 0.0f;
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "time", TimeSheet->Time);
							TimeSheet->InUse = false;

							TimeSheet->Speed = 0.0f;
							TimeSheet->Acceler = 1.0f;//0-1
							TimeSheet->SpeedLR = 0.0f;
							TimeSheet->AccelerLR = 1.0f;//0-1
							TimeSheet->SpeedUD = 0.0f;
							TimeSheet->AccelerUD = 1.0f;//0-1
							TimeSheet->SpeedByCamFB = 0.0f;
							TimeSheet->AccelerByCamFB = 1.0f;//0-1
							TimeSheet->SpeedByCamLR = 0.0f;
							TimeSheet->AccelerByCamLR = 1.0f;//0-1
							TimeSheet->SpeedByCamUD = 0.0f;
							TimeSheet->AccelerByCamUD = 1.0f;//0-1
							TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
							TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
							TimeSheet->Fire = false;
							TimeSheet->BossFire = false;
							TimeSheet->Targeting = false;
						} else {
							TimeSheet->AI_Mode = 0;

							TimeSheet->Time = 0.0f;
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "time", TimeSheet->Time);
							TimeSheet->InUse = false;

							TimeSheet->Speed = 0.0f;
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "speed", TimeSheet->Speed);

							TimeSheet->Acceler = 1.0f;//0-1
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "acceler", TimeSheet->Acceler);
							vw_Clamp(TimeSheet->Acceler, 0.0f, 1.0f);

							TimeSheet->SpeedLR = 0.0f;
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "speedlr", TimeSheet->SpeedLR);

							TimeSheet->AccelerLR = 1.0f;//0-1
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "accelerlr", TimeSheet->AccelerLR);
							vw_Clamp(TimeSheet->AccelerLR, 0.0f, 1.0f);

							TimeSheet->SpeedUD = 0.0f;
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "speedud", TimeSheet->SpeedUD);

							TimeSheet->AccelerUD = 1.0f;//0-1
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "accelerud", TimeSheet->AccelerUD);
							vw_Clamp(TimeSheet->AccelerUD, 0.0f, 1.0f);

							TimeSheet->SpeedByCamFB = 0.0f;
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "speedbycamfb", TimeSheet->SpeedByCamFB);

							TimeSheet->AccelerByCamFB = 1.0f;//0-1
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "accelerbycamfb", TimeSheet->AccelerByCamFB);
							vw_Clamp(TimeSheet->AccelerByCamFB, 0.0f, 1.0f);

							TimeSheet->SpeedByCamLR = 0.0f;
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "speedbycamlr", TimeSheet->SpeedByCamLR);

							TimeSheet->AccelerByCamLR = 1.0f;//0-1
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "accelerbycamlr", TimeSheet->AccelerByCamLR);
							vw_Clamp(TimeSheet->AccelerByCamLR, 0.0f, 1.0f);

							TimeSheet->SpeedByCamUD = 0.0f;
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "speedbycamud", TimeSheet->SpeedByCamUD);

							TimeSheet->AccelerByCamUD = 1.0f;//0-1
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "accelerbycamud", TimeSheet->AccelerByCamUD);
							vw_Clamp(TimeSheet->AccelerByCamUD, 0.0f, 1.0f);

							TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "rotx", TimeSheet->Rotation.x);
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "roty", TimeSheet->Rotation.y);
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "rotz", TimeSheet->Rotation.z);

							TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "rotacx", TimeSheet->RotationAcceler.x);
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "rotacy", TimeSheet->RotationAcceler.y);
							xmlDoc->fGetEntryAttribute(*TLAlienFighter, "rotacz", TimeSheet->RotationAcceler.z);
							vw_Clamp(TimeSheet->RotationAcceler.x, 0.0f, 1.0f);
							vw_Clamp(TimeSheet->RotationAcceler.y, 0.0f, 1.0f);
							vw_Clamp(TimeSheet->RotationAcceler.z, 0.0f, 1.0f);

							TimeSheet->Fire = false;
							if (xmlDoc->TestEntryAttribute(TLAlienFighter, "fire"))
								if (xmlDoc->iGetEntryAttribute(TLAlienFighter, "fire") > 0)
									TimeSheet->Fire = true;
							TimeSheet->BossFire = false;

							TimeSheet->Targeting = false;
						}

					}

					// берем следующий элемент
					TLAlienFighter = TLAlienFighter->Next;
				}
			} else
				// AlienMotherShip
				if (TL->Name == "AlienMotherShip") {
					cAlienSpaceMotherShip *Fighter = new cAlienSpaceMotherShip;
					if (xmlDoc->TestEntryAttribute(TL, "type"))
						Fighter->Create(xmlDoc->iGetEntryAttribute(TL, "type"));
					else {
						TL = TL->Next;
						continue;
					}

					SetID(Fighter, TL, xmlDoc);
					if (ShowDebugModeLine) SetDebugInformation(Fighter, TL);

					if (xmlDoc->fGetEntryAttribute(*TL, "speed", Fighter->NeedSpeed))
						Fighter->Speed = Fighter->NeedSpeed;
					if (xmlDoc->fGetEntryAttribute(*TL, "speedlr", Fighter->NeedSpeedLR))
						Fighter->SpeedLR = Fighter->NeedSpeedLR;
					if (xmlDoc->fGetEntryAttribute(*TL, "speedud", Fighter->NeedSpeedUD))
						Fighter->SpeedUD = Fighter->NeedSpeedUD;

					if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamfb", Fighter->NeedSpeedByCamFB))
						Fighter->SpeedByCamFB = Fighter->NeedSpeedByCamFB;
					if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamlr", Fighter->NeedSpeedByCamLR))
						Fighter->SpeedByCamLR = Fighter->NeedSpeedByCamLR;
					if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamud", Fighter->NeedSpeedByCamUD))
						Fighter->SpeedByCamUD = Fighter->NeedSpeedByCamUD;

					SetShowDeleteOnHide(Fighter, TL, xmlDoc);
					SetAIMode(Fighter, TL, xmlDoc); // на тот случае если просто ставим и все...
					SetShipRotation(Fighter, TL, xmlDoc);
					SetShipLocation(Fighter, TL, xmlDoc, TimeOpLag);

					// дальше смотрим, что нужно сделать...
					sXMLEntry *TLAlienMotherShip = TL->FirstChild;
					while (TLAlienMotherShip) {
						if (TLAlienMotherShip->Name == "TimeSheet") {
							// собираем новый элемент
							sTimeSheet *TimeSheet;
							TimeSheet = new sTimeSheet;
							Fighter->AttachTimeSheet(TimeSheet);

							if (xmlDoc->TestEntryAttribute(TLAlienMotherShip, "aimode")) {
								TimeSheet->AI_Mode = xmlDoc->iGetEntryAttribute(TLAlienMotherShip, "aimode");
								TimeSheet->Time = 0.0f;
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "time", TimeSheet->Time);
								TimeSheet->InUse = false;

								TimeSheet->Speed = 0.0f;
								TimeSheet->Acceler = 1.0f;//0-1
								TimeSheet->SpeedLR = 0.0f;
								TimeSheet->AccelerLR = 1.0f;//0-1
								TimeSheet->SpeedUD = 0.0f;
								TimeSheet->AccelerUD = 1.0f;//0-1
								TimeSheet->SpeedByCamFB = 0.0f;
								TimeSheet->AccelerByCamFB = 1.0f;//0-1
								TimeSheet->SpeedByCamLR = 0.0f;
								TimeSheet->AccelerByCamLR = 1.0f;//0-1
								TimeSheet->SpeedByCamUD = 0.0f;
								TimeSheet->AccelerByCamUD = 1.0f;//0-1
								TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
								TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
								TimeSheet->Fire = false;
								TimeSheet->BossFire = false;
								TimeSheet->Targeting = false;
							} else {
								TimeSheet->AI_Mode = 0;

								TimeSheet->Time = 0.0f;
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "time", TimeSheet->Time);
								TimeSheet->InUse = false;

								TimeSheet->Speed = 0.0f;
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "speed", TimeSheet->Speed);

								TimeSheet->Acceler = 1.0f;//0-1
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "acceler", TimeSheet->Acceler);
								vw_Clamp(TimeSheet->Acceler, 0.0f, 1.0f);

								TimeSheet->SpeedLR = 0.0f;
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "speedlr", TimeSheet->SpeedLR);

								TimeSheet->AccelerLR = 1.0f;//0-1
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "accelerlr", TimeSheet->AccelerLR);
								vw_Clamp(TimeSheet->AccelerLR, 0.0f, 1.0f);

								TimeSheet->SpeedUD = 0.0f;
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "speedud", TimeSheet->SpeedUD);

								TimeSheet->AccelerUD = 1.0f;//0-1
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "accelerud", TimeSheet->AccelerUD);
								vw_Clamp(TimeSheet->AccelerUD, 0.0f, 1.0f);

								TimeSheet->SpeedByCamFB = 0.0f;
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "speedbycamfb", TimeSheet->SpeedByCamFB);

								TimeSheet->AccelerByCamFB = 1.0f;//0-1
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "accelerbycamfb", TimeSheet->AccelerByCamFB);
								vw_Clamp(TimeSheet->AccelerByCamFB, 0.0f, 1.0f);

								TimeSheet->SpeedByCamLR = 0.0f;
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "speedbycamlr", TimeSheet->SpeedByCamLR);

								TimeSheet->AccelerByCamLR = 1.0f;//0-1
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "accelerbycamlr", TimeSheet->AccelerByCamLR);
								vw_Clamp(TimeSheet->AccelerByCamLR, 0.0f, 1.0f);

								TimeSheet->SpeedByCamUD = 0.0f;
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "speedbycamud", TimeSheet->SpeedByCamUD);

								TimeSheet->AccelerByCamUD = 1.0f;//0-1
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "accelerbycamud", TimeSheet->AccelerByCamUD);
								vw_Clamp(TimeSheet->AccelerByCamUD, 0.0f, 1.0f);

								TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "rotx", TimeSheet->Rotation.x);
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "roty", TimeSheet->Rotation.y);
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "rotz", TimeSheet->Rotation.z);

								TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "rotacx", TimeSheet->RotationAcceler.x);
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "rotacy", TimeSheet->RotationAcceler.y);
								xmlDoc->fGetEntryAttribute(*TLAlienMotherShip, "rotacz", TimeSheet->RotationAcceler.z);
								vw_Clamp(TimeSheet->RotationAcceler.x, 0.0f, 1.0f);
								vw_Clamp(TimeSheet->RotationAcceler.y, 0.0f, 1.0f);
								vw_Clamp(TimeSheet->RotationAcceler.z, 0.0f, 1.0f);

								TimeSheet->Fire = false;
								if (xmlDoc->TestEntryAttribute(TLAlienMotherShip, "fire"))
									if (xmlDoc->iGetEntryAttribute(TLAlienMotherShip, "fire") > 0)
										TimeSheet->Fire = true;
								TimeSheet->BossFire = false;
								if (xmlDoc->TestEntryAttribute(TLAlienMotherShip, "bossfire"))
									if (xmlDoc->iGetEntryAttribute(TLAlienMotherShip, "bossfire") > 0)
										TimeSheet->BossFire = true;

								TimeSheet->Targeting = false;
							}

						}

						// берем следующий элемент
						TLAlienMotherShip = TLAlienMotherShip->Next;
					}
				} else
					// PirateShip
					if (TL->Name == "PirateShip") {
						cPirateShip *Fighter = new cPirateShip;
						if (xmlDoc->TestEntryAttribute(TL, "type"))
							Fighter->Create(xmlDoc->iGetEntryAttribute(TL, "type"));
						else {
							TL = TL->Next;
							continue;
						}

						SetID(Fighter, TL, xmlDoc);
						if (ShowDebugModeLine) SetDebugInformation(Fighter, TL);

						if (xmlDoc->fGetEntryAttribute(*TL, "speed", Fighter->NeedSpeed))
							Fighter->Speed = Fighter->NeedSpeed;
						if (xmlDoc->fGetEntryAttribute(*TL, "speedlr", Fighter->NeedSpeedLR))
							Fighter->SpeedLR = Fighter->NeedSpeedLR;
						if (xmlDoc->fGetEntryAttribute(*TL, "speedud", Fighter->NeedSpeedUD))
							Fighter->SpeedUD = Fighter->NeedSpeedUD;

						if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamfb", Fighter->NeedSpeedByCamFB))
							Fighter->SpeedByCamFB = Fighter->NeedSpeedByCamFB;
						if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamlr", Fighter->NeedSpeedByCamLR))
							Fighter->SpeedByCamLR = Fighter->NeedSpeedByCamLR;
						if (xmlDoc->fGetEntryAttribute(*TL, "speedbycamud", Fighter->NeedSpeedByCamUD))
							Fighter->SpeedByCamUD = Fighter->NeedSpeedByCamUD;

						SetShowDeleteOnHide(Fighter, TL, xmlDoc);
						SetAIMode(Fighter, TL, xmlDoc); // на тот случае если просто ставим и все...
						SetShipRotation(Fighter, TL, xmlDoc);
						SetShipLocation(Fighter, TL, xmlDoc, TimeOpLag);

						// дальше смотрим, что нужно сделать...
						sXMLEntry *TLPirateShip = TL->FirstChild;
						while (TLPirateShip) {
							if (TLPirateShip->Name == "TimeSheet") {
								// собираем новый элемент
								sTimeSheet *TimeSheet;
								TimeSheet = new sTimeSheet;
								Fighter->AttachTimeSheet(TimeSheet);

								if (xmlDoc->TestEntryAttribute(TLPirateShip, "aimode")) {
									TimeSheet->AI_Mode = xmlDoc->iGetEntryAttribute(TLPirateShip, "aimode");
									TimeSheet->Time = 0.0f;
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "time", TimeSheet->Time);
									TimeSheet->InUse = false;

									TimeSheet->Speed = 0.0f;
									TimeSheet->Acceler = 1.0f;//0-1
									TimeSheet->SpeedLR = 0.0f;
									TimeSheet->AccelerLR = 1.0f;//0-1
									TimeSheet->SpeedUD = 0.0f;
									TimeSheet->AccelerUD = 1.0f;//0-1
									TimeSheet->SpeedByCamFB = 0.0f;
									TimeSheet->AccelerByCamFB = 1.0f;//0-1
									TimeSheet->SpeedByCamLR = 0.0f;
									TimeSheet->AccelerByCamLR = 1.0f;//0-1
									TimeSheet->SpeedByCamUD = 0.0f;
									TimeSheet->AccelerByCamUD = 1.0f;//0-1
									TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
									TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
									TimeSheet->Fire = false;
									TimeSheet->BossFire = false;
									TimeSheet->Targeting = false;
								} else {
									TimeSheet->AI_Mode = 0;

									TimeSheet->Time = 0.0f;
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "time", TimeSheet->Time);
									TimeSheet->InUse = false;

									TimeSheet->Speed = 0.0f;
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "speed", TimeSheet->Speed);

									TimeSheet->Acceler = 1.0f;//0-1
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "acceler", TimeSheet->Acceler);
									vw_Clamp(TimeSheet->Acceler, 0.0f, 1.0f);

									TimeSheet->SpeedLR = 0.0f;
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "speedlr", TimeSheet->SpeedLR);

									TimeSheet->AccelerLR = 1.0f;//0-1
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "accelerlr", TimeSheet->AccelerLR);
									vw_Clamp(TimeSheet->AccelerLR, 0.0f, 1.0f);

									TimeSheet->SpeedUD = 0.0f;
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "speedud", TimeSheet->SpeedUD);

									TimeSheet->AccelerUD = 1.0f;//0-1
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "accelerud", TimeSheet->AccelerUD);
									vw_Clamp(TimeSheet->AccelerUD, 0.0f, 1.0f);

									TimeSheet->SpeedByCamFB = 0.0f;
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "speedbycamfb", TimeSheet->SpeedByCamFB);

									TimeSheet->AccelerByCamFB = 1.0f;//0-1
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "accelerbycamfb", TimeSheet->AccelerByCamFB);
									vw_Clamp(TimeSheet->AccelerByCamFB, 0.0f, 1.0f);

									TimeSheet->SpeedByCamLR = 0.0f;
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "speedbycamlr", TimeSheet->SpeedByCamLR);

									TimeSheet->AccelerByCamLR = 1.0f;//0-1
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "accelerbycamlr", TimeSheet->AccelerByCamLR);
									vw_Clamp(TimeSheet->AccelerByCamLR, 0.0f, 1.0f);

									TimeSheet->SpeedByCamUD = 0.0f;
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "speedbycamud", TimeSheet->SpeedByCamUD);

									TimeSheet->AccelerByCamUD = 1.0f;//0-1
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "accelerbycamud", TimeSheet->AccelerByCamUD);
									vw_Clamp(TimeSheet->AccelerByCamUD, 0.0f, 1.0f);

									TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "rotx", TimeSheet->Rotation.x);
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "roty", TimeSheet->Rotation.y);
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "rotz", TimeSheet->Rotation.z);

									TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "rotacx", TimeSheet->RotationAcceler.x);
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "rotacy", TimeSheet->RotationAcceler.y);
									xmlDoc->fGetEntryAttribute(*TLPirateShip, "rotacz", TimeSheet->RotationAcceler.z);
									vw_Clamp(TimeSheet->RotationAcceler.x, 0.0f, 1.0f);
									vw_Clamp(TimeSheet->RotationAcceler.y, 0.0f, 1.0f);
									vw_Clamp(TimeSheet->RotationAcceler.z, 0.0f, 1.0f);

									TimeSheet->Fire = false;
									if (xmlDoc->TestEntryAttribute(TLPirateShip, "fire"))
										if (xmlDoc->iGetEntryAttribute(TLPirateShip, "fire") > 0)
											TimeSheet->Fire = true;

									TimeSheet->BossFire = false;
									if (xmlDoc->TestEntryAttribute(TLPirateShip, "bossfire"))
										if (xmlDoc->iGetEntryAttribute(TLPirateShip, "bossfire") > 0)
											TimeSheet->BossFire = true;

									TimeSheet->Targeting = false;
								}

							}

							// берем следующий элемент
							TLPirateShip = TLPirateShip->Next;
						}
					} else
						// CreateAsteroid
						if (TL->Name == "CreateAsteroid") {
							cAsteroid *Asteroid = new cAsteroid;

							// тип сейчас не задействован, всегда ставим 1
							Asteroid->Create(1);

							SetID(Asteroid, TL, xmlDoc);
							if (ShowDebugModeLine)
								SetDebugInformation(Asteroid, TL);
							xmlDoc->fGetEntryAttribute(*TL, "speed", Asteroid->Speed);
							SetShowDeleteOnHide(Asteroid, TL, xmlDoc);

							SetRotation(Asteroid, TL, xmlDoc);
							SetLocation(Asteroid, TL, xmlDoc, TimeOpLag);

							xmlDoc->fGetEntryAttribute(*TL, "rotx", Asteroid->RotationSpeed.x);
							xmlDoc->fGetEntryAttribute(*TL, "roty", Asteroid->RotationSpeed.y);
							xmlDoc->fGetEntryAttribute(*TL, "rotz", Asteroid->RotationSpeed.z);
						} else
							// CreateBasePart
							if (TL->Name == "CreateBasePart") {
								cBasePart *BasePart = new cBasePart;

								// тип части
								if (xmlDoc->TestEntryAttribute(TL, "type"))
									BasePart->Create(xmlDoc->iGetEntryAttribute(TL, "type"));
								else {
									TL = TL->Next;
									continue;
								}

								xmlDoc->fGetEntryAttribute(*TL, "speed", BasePart->Speed);

								SetID(BasePart, TL, xmlDoc);
								if (ShowDebugModeLine) SetDebugInformation(BasePart, TL);
								SetShowDeleteOnHide(BasePart, TL, xmlDoc);

								SetRotation(BasePart, TL, xmlDoc);
								SetLocation(BasePart, TL, xmlDoc, TimeOpLag);
							} else
								// CreateBigAsteroid
								if (TL->Name == "CreateBigAsteroid") {
									cBigAsteroid *BigAsteroid = new cBigAsteroid;

									// тип части
									if (xmlDoc->TestEntryAttribute(TL, "type"))
										BigAsteroid->Create(xmlDoc->iGetEntryAttribute(TL, "type"));
									else {
										TL = TL->Next;
										continue;
									}

									xmlDoc->fGetEntryAttribute(*TL, "speed", BigAsteroid->Speed);

									SetID(BigAsteroid, TL, xmlDoc);
									if (ShowDebugModeLine) SetDebugInformation(BigAsteroid, TL);
									SetShowDeleteOnHide(BigAsteroid, TL, xmlDoc);

									SetRotation(BigAsteroid, TL, xmlDoc);
									SetLocation(BigAsteroid, TL, xmlDoc, TimeOpLag);
								} else
									// CreateMBuilding
									if (TL->Name == "CreateMBuilding") {
										cMilitaryBuilding *GroundObject = new cMilitaryBuilding;

										// тип части
										if (xmlDoc->TestEntryAttribute(TL, "type"))
											GroundObject->Create(xmlDoc->iGetEntryAttribute(TL, "type"));
										else {
											TL = TL->Next;
											continue;
										}

										SetID(GroundObject, TL, xmlDoc);
										if (ShowDebugModeLine) SetDebugInformation(GroundObject, TL);
										SetShowDeleteOnHide(GroundObject, TL, xmlDoc);
										SetAIMode(GroundObject, TL, xmlDoc); // на тот случае если просто ставим и все...

										SetRotation(GroundObject, TL, xmlDoc);
										SetLocation(GroundObject, TL, xmlDoc, TimeOpLag);

										// дальше смотрим, что нужно сделать...
										sXMLEntry *TLGroundObject = TL->FirstChild;
										while (TLGroundObject) {
											if (TLGroundObject->Name == "TimeSheet") {
												// собираем новый элемент
												sTimeSheet *TimeSheet;
												TimeSheet = new sTimeSheet;
												GroundObject->AttachTimeSheet(TimeSheet);

												if (xmlDoc->TestEntryAttribute(TLGroundObject, "aimode")) {
													TimeSheet->AI_Mode = xmlDoc->iGetEntryAttribute(TLGroundObject, "aimode");
													TimeSheet->Time = 0.0f;
													xmlDoc->fGetEntryAttribute(*TLGroundObject, "time", TimeSheet->Time);
													TimeSheet->InUse = false;

													TimeSheet->Speed = 0.0f;
													TimeSheet->Acceler = 1.0f;//0-1
													TimeSheet->SpeedLR = 0.0f;
													TimeSheet->AccelerLR = 1.0f;//0-1
													TimeSheet->SpeedUD = 0.0f;
													TimeSheet->AccelerUD = 1.0f;//0-1
													TimeSheet->SpeedByCamFB = 0.0f;
													TimeSheet->AccelerByCamFB = 1.0f;//0-1
													TimeSheet->SpeedByCamLR = 0.0f;
													TimeSheet->AccelerByCamLR = 1.0f;//0-1
													TimeSheet->SpeedByCamUD = 0.0f;
													TimeSheet->AccelerByCamUD = 1.0f;//0-1
													TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
													TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
													TimeSheet->Fire = false;
													TimeSheet->BossFire = false;
													TimeSheet->Targeting = false;
												} else {
													TimeSheet->AI_Mode = 0;

													TimeSheet->Time = 0.0f;
													xmlDoc->fGetEntryAttribute(*TLGroundObject, "time", TimeSheet->Time);
													TimeSheet->InUse = false;

													TimeSheet->Speed = 0.0f;
													TimeSheet->Acceler = 1.0f;//0-1
													TimeSheet->SpeedLR = 0.0f;
													TimeSheet->AccelerLR = 1.0f;//0-1
													TimeSheet->SpeedUD = 0.0f;
													TimeSheet->AccelerUD = 1.0f;//0-1
													TimeSheet->SpeedByCamFB = 0.0f;
													TimeSheet->AccelerByCamFB = 1.0f;//0-1
													TimeSheet->SpeedByCamLR = 0.0f;
													TimeSheet->AccelerByCamLR = 1.0f;//0-1
													TimeSheet->SpeedByCamUD = 0.0f;
													TimeSheet->AccelerByCamUD = 1.0f;//0-1
													TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
													TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1

													TimeSheet->Fire = false;
													if (xmlDoc->TestEntryAttribute(TLGroundObject, "fire"))
														if (xmlDoc->iGetEntryAttribute(TLGroundObject, "fire") > 0)
															TimeSheet->Fire = true;

													TimeSheet->BossFire = false;

													TimeSheet->Targeting = false;
													if (xmlDoc->TestEntryAttribute(TLGroundObject, "targeting"))
														if (xmlDoc->iGetEntryAttribute(TLGroundObject, "targeting") != 0) TimeSheet->Targeting = true;
												}

											}

											// берем следующий элемент
											TLGroundObject = TLGroundObject->Next;
										}
									} else
										// CreateBuilding
										if (TL->Name == "CreateBuilding") {
											cBuilding *GroundObject = new cBuilding;
											if (xmlDoc->TestEntryAttribute(TL, "type"))
												GroundObject->Create(xmlDoc->iGetEntryAttribute(TL, "type"));
											else {
												TL = TL->Next;
												continue;
											}

											SetID(GroundObject, TL, xmlDoc);
											if (ShowDebugModeLine) SetDebugInformation(GroundObject, TL);
											SetShowDeleteOnHide(GroundObject, TL, xmlDoc);

											SetRotation(GroundObject, TL, xmlDoc);
											SetLocation(GroundObject, TL, xmlDoc, TimeOpLag);
										} else
											// CreateMine
											if (TL->Name == "CreateMine") {
												cProjectile *Mine = new cProjectile;
												// т.к. мины у нас с 214-217, делаем +213
												if (xmlDoc->TestEntryAttribute(TL, "type")) {
													int MineType = xmlDoc->iGetEntryAttribute(TL, "type")+213;
													if (MineType < 214) MineType = 214;
													if (MineType > 217) MineType = 217;
													Mine->Create(MineType);
												} else {
													TL = TL->Next;
													continue;
												}

												Mine->ProjectileType = 1;

												// по умолчанию враг
												Mine->ObjectStatus = 1;
												if (xmlDoc->TestEntryAttribute(TL, "status"))
													Mine->ObjectStatus = xmlDoc->iGetEntryAttribute(TL, "status");

												// общий - пенальти, если не игрок
												float CurrentPenalty = GameNPCWeaponPenalty*1.0f;
												// если игрок или свои - ничего не надо...
												if (Mine->ObjectStatus >= 2) CurrentPenalty = 1.0f;

												Mine->DamageHull = Mine->DamageHull/CurrentPenalty;
												Mine->DamageSystems = Mine->DamageSystems/CurrentPenalty;
												Mine->SpeedStart = Mine->SpeedEnd = Mine->Speed = Mine->SpeedStart/CurrentPenalty;

												SetID(Mine, TL, xmlDoc);
												if (ShowDebugModeLine) SetDebugInformation(Mine, TL);
												SetShowDeleteOnHide(Mine, TL, xmlDoc);


												SetProjectileRotation(Mine, TL, xmlDoc);
												SetProjectileLocation(Mine, TL, xmlDoc, TimeOpLag);
											} else
												// CreateTracked
												if (TL->Name == "CreateTracked") {
													cTracked *GroundObject = new cTracked;
													if (xmlDoc->TestEntryAttribute(TL, "type"))
														GroundObject->Create(xmlDoc->iGetEntryAttribute(TL, "type"));
													else {
														TL = TL->Next;
														continue;
													}

													SetID(GroundObject, TL, xmlDoc);
													if (ShowDebugModeLine) SetDebugInformation(GroundObject, TL);
													if (xmlDoc->fGetEntryAttribute(*TL, "speed", GroundObject->NeedSpeed))
														GroundObject->Speed = GroundObject->NeedSpeed;

													SetShowDeleteOnHide(GroundObject, TL, xmlDoc);
													SetAIMode(GroundObject, TL, xmlDoc); // на тот случае если просто ставим и все...
													SetRotation(GroundObject, TL, xmlDoc);
													SetLocation(GroundObject, TL, xmlDoc, TimeOpLag);

													// дальше смотрим, что нужно сделать...
													sXMLEntry *TLGroundObject = TL->FirstChild;
													while (TLGroundObject) {
														if (TLGroundObject->Name == "TimeSheet") {
															// собираем новый элемент
															sTimeSheet *TimeSheet;
															TimeSheet = new sTimeSheet;
															GroundObject->AttachTimeSheet(TimeSheet);

															if (xmlDoc->TestEntryAttribute(TLGroundObject, "aimode")) {
																TimeSheet->AI_Mode = xmlDoc->iGetEntryAttribute(TLGroundObject, "aimode");
																TimeSheet->Time = 0.0f;
																xmlDoc->fGetEntryAttribute(*TLGroundObject, "time", TimeSheet->Time);
																TimeSheet->InUse = false;

																TimeSheet->Speed = 0.0f;
																TimeSheet->Acceler = 1.0f;//0-1
																TimeSheet->SpeedLR = 0.0f;
																TimeSheet->AccelerLR = 1.0f;//0-1
																TimeSheet->SpeedUD = 0.0f;
																TimeSheet->AccelerUD = 1.0f;//0-1
																TimeSheet->SpeedByCamFB = 0.0f;
																TimeSheet->AccelerByCamFB = 1.0f;//0-1
																TimeSheet->SpeedByCamLR = 0.0f;
																TimeSheet->AccelerByCamLR = 1.0f;//0-1
																TimeSheet->SpeedByCamUD = 0.0f;
																TimeSheet->AccelerByCamUD = 1.0f;//0-1
																TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
																TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
																TimeSheet->Fire = false;
																TimeSheet->BossFire = false;
																TimeSheet->Targeting = false;
															} else {
																TimeSheet->AI_Mode = 0;

																TimeSheet->Time = 0.0f;
																xmlDoc->fGetEntryAttribute(*TLGroundObject, "time", TimeSheet->Time);
																TimeSheet->InUse = false;

																TimeSheet->Speed = 0.0f;
																xmlDoc->fGetEntryAttribute(*TLGroundObject, "speed", TimeSheet->Speed);

																TimeSheet->Acceler = 1.0f;//0-1
																xmlDoc->fGetEntryAttribute(*TLGroundObject, "acceler", TimeSheet->Acceler);
																vw_Clamp(TimeSheet->Acceler, 0.0f, 1.0f);

																TimeSheet->SpeedLR = 0.0f;
																TimeSheet->AccelerLR = 1.0f;//0-1
																TimeSheet->SpeedUD = 0.0f;
																TimeSheet->AccelerUD = 1.0f;//0-1
																TimeSheet->SpeedByCamFB = 0.0f;
																TimeSheet->AccelerByCamFB = 1.0f;//0-1
																TimeSheet->SpeedByCamLR = 0.0f;
																TimeSheet->AccelerByCamLR = 1.0f;//0-1
																TimeSheet->SpeedByCamUD = 0.0f;
																TimeSheet->AccelerByCamUD = 1.0f;//0-1

																TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
																xmlDoc->fGetEntryAttribute(*TLGroundObject, "rotx", TimeSheet->Rotation.x);
																xmlDoc->fGetEntryAttribute(*TLGroundObject, "roty", TimeSheet->Rotation.y);
																xmlDoc->fGetEntryAttribute(*TLGroundObject, "rotz", TimeSheet->Rotation.z);

																TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
																xmlDoc->fGetEntryAttribute(*TLGroundObject, "rotacx", TimeSheet->RotationAcceler.x);
																xmlDoc->fGetEntryAttribute(*TLGroundObject, "rotacy", TimeSheet->RotationAcceler.y);
																xmlDoc->fGetEntryAttribute(*TLGroundObject, "rotacz", TimeSheet->RotationAcceler.z);
																vw_Clamp(TimeSheet->RotationAcceler.x, 0.0f, 1.0f);
																vw_Clamp(TimeSheet->RotationAcceler.y, 0.0f, 1.0f);
																vw_Clamp(TimeSheet->RotationAcceler.z, 0.0f, 1.0f);

																TimeSheet->Fire = false;
																if (xmlDoc->TestEntryAttribute(TLGroundObject, "fire"))
																	if (xmlDoc->iGetEntryAttribute(TLGroundObject, "fire") > 0)
																		TimeSheet->Fire = true;

																TimeSheet->BossFire = false;

																TimeSheet->Targeting = false;
																if (xmlDoc->TestEntryAttribute(TLGroundObject, "targeting"))
																	if (xmlDoc->iGetEntryAttribute(TLGroundObject, "targeting") > 0)
																		TimeSheet->Targeting = true;
															}

														}

														// берем следующий элемент
														TLGroundObject = TLGroundObject->Next;
													}
												} else
													// CreateWheeled
													if (TL->Name == "CreateWheeled") {
														cWheeled *GroundObject = new cWheeled;
														if (xmlDoc->TestEntryAttribute(TL, "type"))
															GroundObject->Create(xmlDoc->iGetEntryAttribute(TL, "type"));
														else {
															TL = TL->Next;
															continue;
														}

														SetID(GroundObject, TL, xmlDoc);
														if (ShowDebugModeLine) SetDebugInformation(GroundObject, TL);
														if (xmlDoc->fGetEntryAttribute(*TL, "speed", GroundObject->NeedSpeed))
															GroundObject->Speed = GroundObject->NeedSpeed;

														SetShowDeleteOnHide(GroundObject, TL, xmlDoc);
														SetAIMode(GroundObject, TL, xmlDoc); // на тот случае если просто ставим и все...
														SetRotation(GroundObject, TL, xmlDoc);
														SetLocation(GroundObject, TL, xmlDoc, TimeOpLag);

														// дальше смотрим, что нужно сделать...
														sXMLEntry *TLGroundObject = TL->FirstChild;
														while (TLGroundObject) {
															if (TLGroundObject->Name == "TimeSheet") {
																// собираем новый элемент
																sTimeSheet *TimeSheet;
																TimeSheet = new sTimeSheet;
																GroundObject->AttachTimeSheet(TimeSheet);

																if (xmlDoc->TestEntryAttribute(TLGroundObject, "aimode")) {
																	TimeSheet->AI_Mode = xmlDoc->iGetEntryAttribute(TLGroundObject, "aimode");
																	TimeSheet->Time = 0.0f;
																	xmlDoc->fGetEntryAttribute(*TLGroundObject, "time", TimeSheet->Time);
																	TimeSheet->InUse = false;

																	TimeSheet->Speed = 0.0f;
																	TimeSheet->Acceler = 1.0f;//0-1
																	TimeSheet->SpeedLR = 0.0f;
																	TimeSheet->AccelerLR = 1.0f;//0-1
																	TimeSheet->SpeedUD = 0.0f;
																	TimeSheet->AccelerUD = 1.0f;//0-1
																	TimeSheet->SpeedByCamFB = 0.0f;
																	TimeSheet->AccelerByCamFB = 1.0f;//0-1
																	TimeSheet->SpeedByCamLR = 0.0f;
																	TimeSheet->AccelerByCamLR = 1.0f;//0-1
																	TimeSheet->SpeedByCamUD = 0.0f;
																	TimeSheet->AccelerByCamUD = 1.0f;//0-1
																	TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
																	TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
																	TimeSheet->Fire = false;
																	TimeSheet->BossFire = false;
																	TimeSheet->Targeting = false;
																} else {
																	TimeSheet->AI_Mode = 0;

																	TimeSheet->Time = 0.0f;
																	xmlDoc->fGetEntryAttribute(*TLGroundObject, "time", TimeSheet->Time);
																	TimeSheet->InUse = false;

																	TimeSheet->Speed = 0.0f;
																	xmlDoc->fGetEntryAttribute(*TLGroundObject, "speed", TimeSheet->Speed);


																	TimeSheet->Acceler = 1.0f;//0-1
																	xmlDoc->fGetEntryAttribute(*TLGroundObject, "acceler", TimeSheet->Acceler);
																	vw_Clamp(TimeSheet->Acceler, 0.0f, 1.0f);

																	TimeSheet->SpeedLR = 0.0f;
																	TimeSheet->AccelerLR = 1.0f;//0-1
																	TimeSheet->SpeedUD = 0.0f;
																	TimeSheet->AccelerUD = 1.0f;//0-1
																	TimeSheet->SpeedByCamFB = 0.0f;
																	TimeSheet->AccelerByCamFB = 1.0f;//0-1
																	TimeSheet->SpeedByCamLR = 0.0f;
																	TimeSheet->AccelerByCamLR = 1.0f;//0-1
																	TimeSheet->SpeedByCamUD = 0.0f;
																	TimeSheet->AccelerByCamUD = 1.0f;//0-1

																	TimeSheet->Rotation = sVECTOR3D(0.0f, 0.0f, 0.0f);
																	xmlDoc->fGetEntryAttribute(*TLGroundObject, "rotx", TimeSheet->Rotation.x);
																	xmlDoc->fGetEntryAttribute(*TLGroundObject, "roty", TimeSheet->Rotation.y);
																	xmlDoc->fGetEntryAttribute(*TLGroundObject, "rotz", TimeSheet->Rotation.z);

																	TimeSheet->RotationAcceler = sVECTOR3D(1.0f, 1.0f, 1.0f);//0-1
																	xmlDoc->fGetEntryAttribute(*TLGroundObject, "rotacx", TimeSheet->RotationAcceler.x);
																	xmlDoc->fGetEntryAttribute(*TLGroundObject, "rotacy", TimeSheet->RotationAcceler.y);
																	xmlDoc->fGetEntryAttribute(*TLGroundObject, "rotacz", TimeSheet->RotationAcceler.z);
																	vw_Clamp(TimeSheet->RotationAcceler.x, 0.0f, 1.0f);
																	vw_Clamp(TimeSheet->RotationAcceler.y, 0.0f, 1.0f);
																	vw_Clamp(TimeSheet->RotationAcceler.z, 0.0f, 1.0f);

																	TimeSheet->Fire = false;
																	if (xmlDoc->TestEntryAttribute(TLGroundObject, "fire"))
																		if (xmlDoc->iGetEntryAttribute(TLGroundObject, "fire") > 0)
																			TimeSheet->Fire = true;

																	TimeSheet->BossFire = false;

																	TimeSheet->Targeting = false;
																	if (xmlDoc->TestEntryAttribute(TLGroundObject, "targeting"))
																		if (xmlDoc->iGetEntryAttribute(TLGroundObject, "targeting") != 0) TimeSheet->Targeting = true;
																}

															}

															// берем следующий элемент
															TLGroundObject = TLGroundObject->Next;
														}
													} else {
														// если тут - значит не нашли директиву, или произошла ошибка
														std::cerr << __func__ << "(): " << "ScriptEngine: tag " << TL->Name
															  << " not found, line " << TL->LineNumber << "\n";
													}


		// берем следующий элемент по порядку
		TL = TL->Next;
	}
}




