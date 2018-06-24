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


	Web Site: https://www.viewizard.com/
	Project: https://github.com/viewizard/astromenace
	E-mail: viewizard@viewizard.com

*************************************************************************************/

#include "../core/core.h"
#include "../enum.h"
#include "../config/config.h"
#include "../assets/texture.h"
#include "../game.h" // FIXME "game.h" should be replaced by individual headers


float MissionTitleLifeTime = 0.0f;
float LastMissionTitleUpdateTime = 0.0f;
int MissionTitleNum = 0;

float MissionFailedLifeTime = 0.0f;
float LastMissionFailedUpdateTime = 0.0f;

extern float CurrentAlert3;


//------------------------------------------------------------------------------------
// установка времени отображения ShowTime миссии номер Num
//------------------------------------------------------------------------------------
void GameSetMissionTitleData(float ShowTime, int Num)
{
	MissionTitleLifeTime = ShowTime;
	LastMissionTitleUpdateTime = vw_GetTimeThread(0);
	MissionTitleNum = Num;

	// пока ничего не показываем
	MissionFailedLifeTime = 0.0f;
	LastMissionFailedUpdateTime = vw_GetTimeThread(0);
}




//------------------------------------------------------------------------------------
// считаем, сколько в ширину займет прорисовка номера
//------------------------------------------------------------------------------------
int CheckMissionTitleNum(const char *Num)
{
	sRECT SrcRect;
	int XSum = 0;

	// считаем
	for (size_t i=0; i<strlen(Num); i++) {
		char Char = Num[i];

		switch (Char) {
		case '1':
			SrcRect(15, 1, 48, 63);
			break;
		case '2':
			SrcRect(48, 1, 86, 63);
			break;
		case '3':
			SrcRect(86, 1, 124, 63);
			break;
		case '4':
			SrcRect(124, 1, 160, 63);
			break;
		case '5':
			SrcRect(161, 1, 200, 63);
			break;
		case '6':
			SrcRect(200, 1, 238, 63);
			break;
		case '7':
			SrcRect(239, 1, 275, 63);
			break;
		case '8':
			SrcRect(276, 1, 315, 63);
			break;
		case '9':
			SrcRect(319, 1, 385, 63);
			break;
		case '0':
			SrcRect(360, 1, 399, 63);
			break;
		}

		XSum += SrcRect.right - SrcRect.left;
	}

	return XSum;
}



//------------------------------------------------------------------------------------
// прорисовка номера
//------------------------------------------------------------------------------------
void DrawMissionTitleNum(int X, int Y, const char *Num, float Transp)
{
	// здесь X - крайняя левая(!!!) точка
	// Y - крайняя верхняя точка (т.е. как и везде)

	sRECT SrcRect, DstRect;
	int XStart = X;


	// рисуем
	for (size_t i=0; i<strlen(Num); i++) {
		char Char = Num[i];

		switch (Char) {
		case '1':
			SrcRect(15,1,48,63);
			break;
		case '2':
			SrcRect(48,1,86,63);
			break;
		case '3':
			SrcRect(86,1,124,63);
			break;
		case '4':
			SrcRect(124,1,160,63);
			break;
		case '5':
			SrcRect(161,1,200,63);
			break;
		case '6':
			SrcRect(200,1,238,63);
			break;
		case '7':
			SrcRect(239,1,275,63);
			break;
		case '8':
			SrcRect(276,1,315,63);
			break;
		case '9':
			SrcRect(319,1,355,63);
			break;
		case '0':
			SrcRect(360,1,399,63);
			break;
		}


		DstRect(XStart,Y,XStart+(SrcRect.right - SrcRect.left),Y+(SrcRect.bottom - SrcRect.top));
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset("game/nums.tga"), true, Transp);
		XStart += SrcRect.right - SrcRect.left;
	}

}





//------------------------------------------------------------------------------------
// прорисовка номера миссии
//------------------------------------------------------------------------------------
void GameDrawMissionTitle()
{
	// нам тут делать нечего
	if (MissionTitleLifeTime <= 0.0f) return;

	float TimeDelta = vw_GetTimeThread(0) - LastMissionTitleUpdateTime;
	LastMissionTitleUpdateTime = vw_GetTimeThread(0);

	MissionTitleLifeTime -= TimeDelta;


	sRECT SrcRect, DstRect;

	std::string buffer{std::to_string(MissionTitleNum)};

	// вывод надписи Mission

	// считаем сколько в ширину надпись всего... 30 - пробел между надписью и номером
	int TotalW = 226+20+CheckMissionTitleNum(buffer.c_str());
	// находим откуда начинать рисовать
	int XStart = (GameConfig().InternalWidth - TotalW) / 2;

	SrcRect(0,0,226,64);
	DstRect(XStart,352,XStart+226,352+64);

	if (MissionTitleLifeTime >= 1.0f) {
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(vw_GetText("lang/en/game/mission.tga")), true);
		// вывод номера миссии
		DrawMissionTitleNum(XStart+226+20, 352+1, buffer.c_str(), 1.0f);
	} else {
		vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(vw_GetText("lang/en/game/mission.tga")),
			  true, MissionTitleLifeTime);
		// вывод номера миссии
		DrawMissionTitleNum(XStart+226+20, 352+1, buffer.c_str(), MissionTitleLifeTime);
	}


}






//------------------------------------------------------------------------------------
// установка времени отображения ShowTime миссия провалена
//------------------------------------------------------------------------------------
void GameSetMissionFailedData(float ShowTime)
{
	MissionFailedLifeTime = ShowTime;
	LastMissionFailedUpdateTime = vw_GetTimeThread(0);
	// выводим курсор
	DrawGameCursor = true;
	// сброс кнопки мышки, чтобы случайно не нажали
	vw_GetMouseLeftClick(true);
	// release mouse control
	SDL_SetWindowGrab(vw_GetSDLWindow(), SDL_FALSE);
}


//------------------------------------------------------------------------------------
// прорисовка миссия провалена
//------------------------------------------------------------------------------------
void GameDrawMissionFailed()
{
	// нам тут делать нечего
	if (MissionFailedLifeTime <= 0.0f) return;

	float TimeDelta = vw_GetTimeThread(0) - LastMissionFailedUpdateTime;
	LastMissionFailedUpdateTime = vw_GetTimeThread(0);

	// считаем только если не отображается меню
	if (GameContentTransp <= 0.0f)
		MissionFailedLifeTime -= TimeDelta;

	// все, время вышло... нужно сделать сброс и загрузить меню
	if (MissionFailedLifeTime <= 0.0f) {
		ComBuffer = eCommand::SWITCH_FROM_GAME_TO_MISSION_MENU;
		ExitGame();
	}

	sRECT SrcRect, DstRect;

	// вывод надписи Mission
	SrcRect(0,0,512,84);
	DstRect(GameConfig().InternalWidth / 2 - 256, 342, GameConfig().InternalWidth / 2 + 256, 342 + 84);

	vw_Draw2D(DstRect, SrcRect, GetPreloadedTextureAsset(vw_GetText("lang/en/game/missionfailed.tga")), true);


	// выводим кнопки...

	int Y = 180+270;

	int X = GameConfig().InternalWidth / 2 - 230;
	if (DrawButton200_2(X,Y, vw_GetText("QUIT"), 1.0f, false)) {
		ComBuffer = eCommand::SWITCH_FROM_GAME_TO_MAIN_MENU;
		ExitGame();
	}


	X = GameConfig().InternalWidth / 2 + 30;
	if (DrawButton200_2(X,Y, vw_GetText("RESTART"), 1.0f, false)) {
		ComBuffer = eCommand::SWITCH_TO_GAME;
		ExitGame();
	}

}


