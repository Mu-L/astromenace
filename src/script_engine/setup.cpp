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


//------------------------------------------------------------------------------------
// разарх данных методом Хаффмана с динамической таблицей
//------------------------------------------------------------------------------------
static int HAFFtoDATA(int size, uint8_t **dstVFS, uint8_t *srcVFS, int *dsizeVFS, int ssizeVFS)
{
	// читаем счетчик бит...
	int BitCount = 0 ;
	BitCount = srcVFS[3];
	BitCount *= 0x100; // сдвиг на 8 бит...
	BitCount += srcVFS[2];
	BitCount *= 0x100; // сдвиг на 8 бит...
	BitCount += srcVFS[1];
	BitCount *= 0x100; // сдвиг на 8 бит...
	BitCount += srcVFS[0];

	// читаем кол-во элементов таблицы
	int  TabCount = srcVFS[4] + 1;// кол-во элементов...

	// в цикле, читаем все элементы таблицы
	uint8_t Count[256]; // порядковый номер (код) элемента в таблице...
	for (int i = 5; i < TabCount+5; i++)
		Count[i-5] = srcVFS[i];// записываем только код...

	// первый проход - просчет памяти резулт. послед...
	(*dsizeVFS) = 0;
	uint8_t tmpMask = 0;// маска, с которой будем работать...
	uint8_t mMask = 0;// маска, которую составляем...
	int cBit = 0;// кол-во информационных бит в маске...
	int j = 0; // указатель на позицию в преобразованных данных.
	bool Dat = false;
	int tmpBitCount = BitCount;
	bool ex = false;

	if (size == 0) {
		for (int i = 0 + 4 + 1 + TabCount; i < ssizeVFS; i++) {
			tmpMask = srcVFS[i];
			if (tmpBitCount<=0) break;
			// работаем с каждым битом в байте...
			for (int k = 0; k < 8; k++) {
				ex=false;
				// если закрыли последовательность и след. бит - 0, символ 0...
				if ((cBit==0)&((tmpMask & 1) == 0)&(!Dat)&(!ex)) {
					// пишем в рез. послед. 0-й элемент...
					(*dsizeVFS)++;
					tmpBitCount --;
					if (tmpBitCount<=0) break;
					ex=true;
				}
				// если закрыли последовательность и след. бит - 1, работаем...
				if ((cBit>=0)&((tmpMask & 1) == 1)&(!Dat)&(!ex)) {
					// ув. счетчик бит...
					cBit++;
					tmpBitCount --;
					if (tmpBitCount<=0) break;
					ex=true;
				}
				if ((Dat)&(!ex)) {
					mMask = mMask*2;
					mMask = mMask & 0xFE;
					mMask = mMask | (tmpMask & 1);
					cBit --;
					if (cBit<=0) {
						Dat = false;
						(*dsizeVFS)++;
						cBit = 0;
					}
					tmpBitCount --;
					if (tmpBitCount<=0) break;
					ex=true;
				}
				if ((cBit>0)&((tmpMask & 1) == 0)&(!Dat)&(!ex)) {
					mMask = 1;
					cBit --;
					if (cBit>0) {
						Dat= true;
					} else {
						(*dsizeVFS)++;
					}
					tmpBitCount --;
					if (tmpBitCount<=0) break;
					ex=true;
				}
				// сдвиг. - установка на след бит...
				tmpMask = tmpMask/2;
			}
		}
	} else (*dsizeVFS) = size;

	// резервируем память...
	*dstVFS = new uint8_t[*dsizeVFS];
	if ((*dstVFS) == nullptr)
		return 0;

	// второй проход - получение результирующ. послед...
	cBit = 0;
	Dat = false;
	tmpBitCount = BitCount;
	ex = false;
	for (int i = 0 + 4 + 1 + TabCount; i < ssizeVFS; i++) {
		tmpMask = srcVFS[i];
		if (tmpBitCount<=0) break;
		// работаем с каждым битом в байте...
		for (int k = 0; k < 8; k++) {
			ex=false;
			// если закрыли последовательность и след. бит - 0, символ 0...
			if ((cBit==0)&((tmpMask & 1) == 0)&(!Dat)&(!ex)) {
				// пишем в рез. послед. 0-й элемент...
				(*dstVFS)[j] = Count[0];
				j++;
				tmpBitCount --;
				if (tmpBitCount<=0) break;
				ex=true;
			}
			// если закрыли последовательность и след. бит - 1, работаем...
			if ((cBit>=0)&((tmpMask & 1) == 1)&(!Dat)&(!ex)) {
				// ув. счетчик бит...
				cBit++;
				tmpBitCount --;
				if (tmpBitCount<=0) break;
				ex=true;
			}
			if ((Dat)&(!ex)) {
				mMask = mMask*2;
				mMask = mMask & 0xFE;
				mMask = mMask | (tmpMask & 1);
				cBit --;
				if (cBit<=0) {
					Dat = false;
					(*dstVFS)[j] = Count[mMask];
					cBit = 0;
					j++;
				}
				tmpBitCount --;
				if (tmpBitCount<=0) break;
				ex=true;
			}

			if ((cBit>0)&((tmpMask & 1) == 0)&(!Dat)&(!ex)) {
				mMask = 1;
				cBit --;
				if (cBit>0) {
					Dat= true;
				} else {
					(*dstVFS)[j] = Count[mMask];
					j++;
				}
				tmpBitCount --;
				if (tmpBitCount<=0) break;
				ex=true;
			}

			// сдвиг. - установка на след бит...
			tmpMask = tmpMask/2;
		}
	}

	return 0;
}




//------------------------------------------------------------------------------------
// архивир данных методом Хаффмана с динамической таблицей
//------------------------------------------------------------------------------------
static int DATAtoHAFF(uint8_t **dstVFS, uint8_t *srcVFS, int *dsizeVFS, int ssizeVFS)
{
	// мат. анализ структуры входных данных...
	// собираем данные о вхождении каждого символа...
	uint8_t Count[256]; // порядковый номер (код) элемента в таблице...
	int  mathTab[256]; // кол-во вхождений каждого эл-та...
	int  TabCount = 0;// кол-во элементов... (для составления динам. таблици...)
	short bitMask[256]; // битовая маска для каждого эл-та таблици...(2 бита - по макс.)
	int bitMaskS[256]; // количество бит в маске...
	*dsizeVFS = 0;

	// заносим номера (коды) в таблицу...иниц. табл. вхождений...
	for (int i = 0; i < 256; i++) {
		Count[i] = i;
		mathTab[i] = 0;
		bitMask[i] = 0;
		bitMaskS[i] = 0;
	}

	// исследуем данные...
	for (int i = 0; i < ssizeVFS; i++) mathTab[srcVFS[i]]++;

	// упорядочевание полученной таблицы...
	// сортируем данные по возростанию кол-ва вхождений...Пузырьковая Сортировка
	for (int i = 0; i < 255; i++)
		for (int j = 0; j < 255; j++)
			if (mathTab[j+1] > mathTab[j]) {
				int tmp1 = mathTab[j+1];
				mathTab[j+1] = mathTab[j];
				mathTab[j] = tmp1;
				uint8_t tmp2 = Count[j+1];
				Count[j+1] = Count[j];
				Count[j] = tmp2;
			}

	// находим элементы, которые есть в последовательности (т.е первые эл-ты упоряд. посл.)
	for (int i = 0; i < 256; i++)
		if (mathTab[i] >0) TabCount++;

	// составляем маски для существующих элементов....
	// для 0...
	bitMask[0] = 0;
	bitMaskS[0] = 1;
	// для 1...
	bitMask[1] = 2;
	bitMaskS[1] = 2;
	// для 2,3...
	for (int i = 2; i < 4; i++) {
		bitMask[i] = i-2;
		bitMask[i] = bitMask[i] | 12; // 1100
		bitMaskS[i] = 4;
	}
	// для 4,7...
	for (int i = 4; i < 8; i++) {
		bitMask[i] = i-4;
		bitMask[i] = bitMask[i] | 56; // 111000
		bitMaskS[i] = 6;
	}
	// для 8,15...
	for (int i = 8; i < 16; i++) {
		bitMask[i] = i-8;
		bitMask[i] = bitMask[i] | 240; // 11110000
		bitMaskS[i] = 8;
	}
	// для 16,31...
	for (int i = 16; i < 32; i++) {
		bitMask[i] = i-16;
		bitMask[i] = bitMask[i] | 992; // 1111100000
		bitMaskS[i] = 10;
	}
	// для 32,63...
	for (int i = 32; i < 64; i++) {
		bitMask[i] = i-32;
		bitMask[i] = bitMask[i] | 4032; // 11111110000000
		bitMaskS[i] = 12;
	}
	// для 64,127...
	for (int i = 64; i < 128; i++) {
		bitMask[i] = i-64;
		bitMask[i] = bitMask[i] | 16256; // 11111110000000
		bitMaskS[i] = 14;
	}
	// для 128,255...
	for (int i = 128; i < 256; i++) {
		bitMask[i] = i-128;
		bitMask[i] = bitMask[i] | 65280; // 1111111100000000
		bitMaskS[i] = 16;
	}

	// просчет кол-ва необходимого места...
	(*dsizeVFS) ++;// кол-во элементов в массиве...
	(*dsizeVFS) += TabCount;// место для таблици...
	// вычисляем место для данных...
	int tmp = 0;
	for (int i = 0; i < TabCount; i++) {
		tmp += (mathTab[i] * bitMaskS[i]);
	}
	(*dsizeVFS) += 4 ; //счетчик бит...
	(*dsizeVFS) += tmp / 8 +1;// т.к. считали в битах...

	// резервируем память...
	*dstVFS = new uint8_t[*dsizeVFS];

	// создаем выходную последовательность...
	int j = 0; // указатель на позицию в преобразованных данных.

	// записываем счетчик бит...
	(*dstVFS)[j] =  (uint8_t)tmp;
	j++;
	tmp /= 0x100;
	(*dstVFS)[j] =  (uint8_t)tmp;
	j++;
	tmp /= 0x100;
	(*dstVFS)[j] =  (uint8_t)tmp;
	j++;
	tmp /= 0x100;
	(*dstVFS)[j] =  (uint8_t)tmp;
	j++;

	// записываем кол-во данных в таблице...
	(*dstVFS)[j] = (uint8_t)(TabCount-1) ;// чтобы было от 0 до 255 ...
	j++;
	// пишем таблицу...
	for (int i = 0; i < TabCount; i++) {
		(*dstVFS)[j] = Count[i];// записываем только код...
		j++;
	}
	// "разупорядочиваем" массивы во временные последовательности...
	short tmbitMask[256];
	int tmbitMaskS[256];
	for (int i = 0; i < 256; i++) {
		tmbitMask[Count[i]] = bitMask[i];
		tmbitMaskS[Count[i]] = bitMaskS[i];
	}


	// перебираем данные и пакуем...
	int DstBA = 0; // текущий указатель на активный бит в байте результ. массива...
	uint8_t T = 0; // текущий бит на установку...
	(*dstVFS)[j] = 0; // предустановка на ИЛИ...

	for (int i = 0; i < ssizeVFS; i++) {
		// берем исходные данные...
		short tmpMask = tmbitMask[srcVFS[i]]; // маска, с которой будем работать...
		int BitWr = tmbitMaskS[srcVFS[i]]; // кол-во бит, которые надо записать из маски...

		// уст. крайний бит...
		//tmpMask = (short)(tmpMask * exp( (16-BitWr) * log(2)));
		if (BitWr == 1) tmpMask = tmpMask * 32768;
		if (BitWr == 2) tmpMask = tmpMask * 16384;
		if (BitWr == 4) tmpMask = tmpMask * 4096;
		if (BitWr == 6) tmpMask = tmpMask * 1024;
		if (BitWr == 8) tmpMask = tmpMask * 256;
		if (BitWr == 10) tmpMask = tmpMask * 64;
		if (BitWr == 12) tmpMask = tmpMask * 16;
		if (BitWr == 14) tmpMask = tmpMask * 4;

		// в цикле по BitWr
		while (BitWr > 0) {
			// проверка крайнего бита в tmpMask
			// установка аналога в dst[j]
			if ((tmpMask & 0x8000) == 0x8000) T = 1;
			else T = 0;
			// убираем крайний (уже исследованный) бит
			tmpMask = tmpMask * 2;
			// сдвиг на DstBA в tmpMask
			if (DstBA == 1) T = T * 2;
			if (DstBA == 2) T = T * 4;
			if (DstBA == 3) T = T * 8;
			if (DstBA == 4) T = T * 16;
			if (DstBA == 5) T = T * 32;
			if (DstBA == 6) T = T * 64;
			if (DstBA == 7) T = T * 128;

			(*dstVFS)[j] = (*dstVFS)[j] + T;
			BitWr -- ;
			// увеличение DstBA
			DstBA++;
			// проверка на достижение DstBA==8
			if (DstBA > 7) {
				// если да - j++, dst[j] = 0, DstBA=0
				j++;
				(*dstVFS)[j] = 0;
				DstBA=0;
			}
		}
	}

	return 0;
}


//-----------------------------------------------------------------------------
// задаем базовые настройки
//-----------------------------------------------------------------------------
void InitSetup()
{
	// базовые настройки, устанавливаем всегда - кто там знает, что в xml файле есть
	// всегда устанавливаем текущее разрешение рабочего стола

	// устанавливаем номер билда
	Setup.BuildVersion = GAME_BUILD;

	Setup.MenuLanguage = 1; // en by default
	Setup.VoiceLanguage = 1; // en by default
	Setup.FontNumber = 0;
	strcpy(Setup.FontName, default_font_family);
	Setup.FontSize = 16;

	Setup.Width = 1228;
	Setup.Height = 768;
	Setup.BPP = 0;
	Setup.fAspectRatioWidth = 1228.0f;
	Setup.fAspectRatioHeight = 768.0f;
	Setup.iAspectRatioWidth = 1228;
	Setup.iAspectRatioHeight = 768;

	Setup.CameraModeWithStandardAspectRatio = 0;

	Setup.TextureFilteringMode = 2;
	Setup.TexturesQuality = 3;
	Setup.MSAA = 0;
	Setup.CSAA = 0;
	Setup.VisualEffectsQuality = 0;
	Setup.AnisotropyLevel = 1;
	Setup.TexturesCompressionType = 1;
	Setup.UseGLSL = false;
	Setup.ShadowMap = 0;
	Setup.MaxPointLights = 3;
	Setup.MusicSw = 8;
	Setup.Music_check = true;
	Setup.SoundSw = 10;
	Setup.VoiceSw = 10;
	Setup.Sound_check = true;
	Setup.VSync = 0;
	Setup.Brightness = 5;
	Setup.ShowFPS = false;
	Setup.GameWeaponInfoType = 1;
	Setup.GameSpeed = 1.5f;
	Setup.LoadingHint = 0;


	Setup.KeyboardDecreaseGameSpeed = SDLK_F5;
	Setup.KeyboardResetGameSpeed = SDLK_F6;
	Setup.KeyboardIncreaseGameSpeed = SDLK_F7;
	Setup.KeyboardGameWeaponInfoType = SDLK_F8;
	Setup.KeyboardPrimaryWeaponFireMode = SDLK_F9;
	Setup.KeyboardSecondaryWeaponFireMode = SDLK_F10;

	Setup.KeyBoardLeft = SDLK_LEFT;
	Setup.KeyBoardRight = SDLK_RIGHT;
	Setup.KeyBoardUp = SDLK_UP;
	Setup.KeyBoardDown = SDLK_DOWN;
	Setup.KeyBoardPrimary = SDLK_LCTRL;
	Setup.KeyBoardSecondary = SDLK_SPACE;
	Setup.MousePrimary = SDL_BUTTON_LEFT;
	Setup.MouseSecondary = SDL_BUTTON_RIGHT;
	Setup.JoystickPrimary = 0;
	Setup.JoystickSecondary = 1;
	Setup.JoystickNum = 0;
	Setup.JoystickDeadZone = 2;

	Setup.ControlSensivity = 5;
	Setup.MouseControl = true;
	Setup.LastProfile = -1;
	Setup.MenuScript = 0;


	strcpy(Setup.TopScores[0].Name, "Viewizard");
	Setup.TopScores[0].Score = 100000;
	strcpy(Setup.TopScores[1].Name, "Alex");
	Setup.TopScores[1].Score = 90000;
	strcpy(Setup.TopScores[2].Name, "Michael");
	Setup.TopScores[2].Score = 80000;
	strcpy(Setup.TopScores[3].Name, "Jeffrey");
	Setup.TopScores[3].Score = 70000;
	strcpy(Setup.TopScores[4].Name, "Christopher Hughson");
	Setup.TopScores[4].Score = 60000;
	strcpy(Setup.TopScores[5].Name, "Becky");
	Setup.TopScores[5].Score = 40000;
	strcpy(Setup.TopScores[6].Name, "Greg");
	Setup.TopScores[6].Score = 20000;
	strcpy(Setup.TopScores[7].Name, "Jay Coleman");
	Setup.TopScores[7].Score = 10000;
	strcpy(Setup.TopScores[8].Name, "Kelvin");
	Setup.TopScores[8].Score = 5000;
	strcpy(Setup.TopScores[9].Name, "Stephan Gregory");
	Setup.TopScores[9].Score = 1000;


	for(int i=0; i<5; i++)
		Setup.Profile[i].Used = false;

	// сбрасываем хинты в тру, чтобы показывать
	for(int i=0; i<10; i++)
		Setup.NeedShowHint[i] = true;


	// всегда включен по умолчанию, если что железо само поймет что ему надо
	Setup.VBOCoreMode = 1;
	Setup.FBOCoreMode = 1;
	// по умолчанию выключаем VAO из-за проблем у пользователей некоторых видео карт AMD и Intel
	Setup.VAOCoreMode = 0;
	// по умолчанию всегда меньше 128 метров
	Setup.EqualOrMore128MBVideoRAM = false;
	// по умолчанию генерируем в видео карте (после иниц. окна поставим правильное), по крайней мере в виндовс немного быстрее
	Setup.HardwareMipMapGeneration = true;
}














//-----------------------------------------------------------------------------
// записываем файл с настройками
//-----------------------------------------------------------------------------
void SaveXMLSetupFile()
{
	cXMLDocument *XMLdoc = new cXMLDocument;

	cXMLEntry *RootXMLEntry = XMLdoc->AddEntry(nullptr, "AstroMenaceSettings");

	XMLdoc->AddComment(RootXMLEntry, " AstroMenace game Settings ");



	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "GAME_BUILD"), "value", GAME_BUILD);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "MenuLanguage"), "value", vw_GetLanguageList()[Setup.MenuLanguage-1].code);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "VoiceLanguage"), "value", vw_GetLanguageList()[Setup.VoiceLanguage-1].code);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "FontNumber"), "value", Setup.FontNumber);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "FontName"), "value", Setup.FontName);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "FontSize"), "value", Setup.FontSize);


	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "Width"), "value", Setup.Width);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "Height"), "value", Setup.Height);
	XMLdoc->AddComment(RootXMLEntry, " Window (BPP = 0) or Full Screen (BPP = 16, 24 or 32) ");
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "BPP"), "value", Setup.BPP);
	XMLdoc->AddComment(RootXMLEntry, " Aspect Ratio must be 4:3 or 16:10 ");
	if (Setup.iAspectRatioWidth == 1024)
		XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "AspectRatio"), "value", "4:3");
	else
		XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "AspectRatio"), "value", "16:10");
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "CameraModeWithStandardAspectRatio"), "value", Setup.CameraModeWithStandardAspectRatio);

	XMLdoc->AddComment(RootXMLEntry, " Don't change this setting unless you know what you are doing ");
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "VBOCoreMode"), "value", Setup.VBOCoreMode);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "VAOCoreMode"), "value", Setup.VAOCoreMode);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "FBOCoreMode"), "value", Setup.FBOCoreMode);
	XMLdoc->AddComment(RootXMLEntry, " If your video card have 128+ MB VRAM on board - turn it on ");
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "EqualOrMore128MBVideoRAM"), "value", Setup.EqualOrMore128MBVideoRAM);
	XMLdoc->AddComment(RootXMLEntry, " Don't change this setting unless you know what you are doing ");
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "HardwareMipMapGeneration"), "value", Setup.HardwareMipMapGeneration);


	XMLdoc->AddComment(RootXMLEntry, " Common settings ");
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "TextureFilteringMode"), "value", Setup.TextureFilteringMode);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "TexturesQuality"), "value", Setup.TexturesQuality);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "MSAA"), "value", Setup.MSAA);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "CSAA"), "value", Setup.CSAA);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "VisualEffectsQuality"), "value", Setup.VisualEffectsQuality);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "AnisotropyLevel"), "value", Setup.AnisotropyLevel);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "TexturesCompressionType"), "value", Setup.TexturesCompressionType);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "UseGLSL"), "value", Setup.UseGLSL);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "ShadowMap"), "value", Setup.ShadowMap);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "MaxPointLights"), "value", Setup.MaxPointLights);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "MusicSw"), "value", Setup.MusicSw);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "SoundSw"), "value", Setup.SoundSw);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "VoiceSw"), "value", Setup.VoiceSw);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "VSync"), "value", Setup.VSync);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "Brightness"), "value", Setup.Brightness);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "ShowFPS"), "value", Setup.ShowFPS);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "GameWeaponInfoType"), "value", Setup.GameWeaponInfoType);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "GameSpeed"), "value", Setup.GameSpeed);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "LoadingHint"), "value", Setup.LoadingHint);


	XMLdoc->AddComment(RootXMLEntry, " Control settings ");
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyboardDecreaseGameSpeed"), "value", vw_KeyboardCodeName(Setup.KeyboardDecreaseGameSpeed));
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyboardResetGameSpeed"), "value", vw_KeyboardCodeName(Setup.KeyboardResetGameSpeed));
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyboardIncreaseGameSpeed"), "value", vw_KeyboardCodeName(Setup.KeyboardIncreaseGameSpeed));
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyboardGameWeaponInfoType"), "value", vw_KeyboardCodeName(Setup.KeyboardGameWeaponInfoType));
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyboardPrimaryWeaponFireMode"), "value", vw_KeyboardCodeName(Setup.KeyboardPrimaryWeaponFireMode));
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyboardSecondaryWeaponFireMode"), "value", vw_KeyboardCodeName(Setup.KeyboardSecondaryWeaponFireMode));

	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyBoardLeft"), "value", vw_KeyboardCodeName(Setup.KeyBoardLeft));
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyBoardRight"), "value", vw_KeyboardCodeName(Setup.KeyBoardRight));
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyBoardUp"), "value", vw_KeyboardCodeName(Setup.KeyBoardUp));
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyBoardDown"), "value", vw_KeyboardCodeName(Setup.KeyBoardDown));
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyBoardPrimary"), "value", vw_KeyboardCodeName(Setup.KeyBoardPrimary));
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "KeyBoardSecondary"), "value", vw_KeyboardCodeName(Setup.KeyBoardSecondary));
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "MousePrimary"), "value", Setup.MousePrimary);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "MouseSecondary"), "value", Setup.MouseSecondary);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "JoystickPrimary"), "value", Setup.JoystickPrimary);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "JoystickSecondary"), "value", Setup.JoystickSecondary);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "JoystickNum"), "value", Setup.JoystickNum);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "JoystickDeadZone"), "value", Setup.JoystickDeadZone);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "ControlSensivity"), "value", Setup.ControlSensivity);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "MouseControl"), "value", Setup.MouseControl);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "LastProfile"), "value", Setup.LastProfile);
	XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, "MenuScript"), "value", Setup.MenuScript);

	for(int i=0; i<10; i++) {
		char Name[128];
		sprintf(Name, "HintStatus%i", i+1);
		XMLdoc->AddEntryAttribute(XMLdoc->AddEntry(RootXMLEntry, Name), "value", Setup.NeedShowHint[i]);
	}



	//
	// всегда последние
	//


	// упаковка таблицы рекордов
	int TopScoresDataSize = sizeof(sTopScores)*10;
	unsigned char *TopScoresData = new unsigned char[TopScoresDataSize];
	memcpy(TopScoresData, Setup.TopScores, TopScoresDataSize);

	// сразу архивируем Хаффманом
	uint8_t *dstVFS;
	int dsizeVFS;
	DATAtoHAFF(&dstVFS, (uint8_t *)TopScoresData, &dsizeVFS, TopScoresDataSize);
	delete [] TopScoresData;
	TopScoresDataSize = dsizeVFS;
	TopScoresData = (unsigned char *)dstVFS;

	unsigned char *TopScoresDataXORCode = new unsigned char[TopScoresDataSize*3];
	char *TopScoresResultString = new char[TopScoresDataSize*4+1];


	{
		// XOR
		for (int i=0; i < TopScoresDataSize; i++) {
			int k1 = i;
			int k2 = TopScoresDataSize + i*2;
			TopScoresDataXORCode[k1] = 97 + (unsigned char)vw_iRandNum(25);
			TopScoresDataXORCode[k2] = 0;
			TopScoresDataXORCode[k2+1] = TopScoresData[i]^TopScoresDataXORCode[k1];
			// в первую - десятки, во вторую - еденицы
			TopScoresDataXORCode[k2] = 97 + (unsigned char)(TopScoresDataXORCode[k2+1]/10.0f);
			TopScoresDataXORCode[k2+1] = 97 + (TopScoresDataXORCode[k2+1] - (TopScoresDataXORCode[k2]-97)*10);
		}

		// чтобы разбить на блоки вставляем пробел
		// тогда красиво отображаться будет (если врапинг выставлен в редакторе)
		int k=0;
		int l=0;
		for (int i=0; i < TopScoresDataSize*3; i++) {
			TopScoresResultString[k] = TopScoresDataXORCode[i];
			k++;
			l++;
			if (l >= 125) {
				TopScoresResultString[k] = 0x20;
				k++;
				l=0;
			}
		}
		TopScoresResultString[k] = 0;
	}

	XMLdoc->AddEntryContent(XMLdoc->AddEntry(RootXMLEntry, "TopScores"), TopScoresResultString);

	if (TopScoresResultString != nullptr)
		delete [] TopScoresResultString;
	if (TopScoresData != nullptr)
		delete [] TopScoresData;
	if (TopScoresDataXORCode != nullptr)
		delete [] TopScoresDataXORCode;




	// упаковка профайлов


	int ProfileDataSize = sizeof(GameProfile)*5;
	unsigned char *ProfileData = new unsigned char[ProfileDataSize];
	memcpy(ProfileData, Setup.Profile, ProfileDataSize);

	// сразу архивируем Хаффманом
	DATAtoHAFF(&dstVFS, (uint8_t *)ProfileData, &dsizeVFS, ProfileDataSize);
	delete [] ProfileData;
	ProfileDataSize = dsizeVFS;
	ProfileData = (unsigned char *)dstVFS;

	unsigned char *ProfileDataXORCode = new unsigned char[ProfileDataSize*3];
	char *ResultString = new char[ProfileDataSize*4+1];


	// XOR
	for (int i=0; i < ProfileDataSize; i++) {
		int k1 = i;
		int k2 = ProfileDataSize + i*2;
		ProfileDataXORCode[k1] = 97 + (unsigned char)vw_iRandNum(25);
		ProfileDataXORCode[k2] = 0;
		ProfileDataXORCode[k2+1] = ProfileData[i]^ProfileDataXORCode[k1];
		// в первую - десятки, во вторую - еденицы
		ProfileDataXORCode[k2] = 97 + (unsigned char)(ProfileDataXORCode[k2+1]/10.0f);
		ProfileDataXORCode[k2+1] = 97 + (ProfileDataXORCode[k2+1] - (ProfileDataXORCode[k2]-97)*10);
		//fprintf(stderr, "--%i %i %i %i\n", ProfileDataXORCode[k], ProfileDataXORCode[k+1],ProfileDataXORCode[k+2],ProfileDataXORCode[k+3]);
	}


	// чтобы разбить на блоки вставляем пробел
	// тогда красиво отображаться будет (если врапинг выставлен в редакторе)
	int k=0;
	int l=0;
	for (int i=0; i < ProfileDataSize*3; i++) {
		ResultString[k] = ProfileDataXORCode[i];
		k++;
		l++;
		if (l >= 125) {
			ResultString[k] = 0x20;
			k++;
			l=0;
		}
	}
	ResultString[k] = 0;

	XMLdoc->AddEntryContent(XMLdoc->AddEntry(RootXMLEntry, "PilotsProfiles"), ResultString);

	delete [] ResultString;
	if (ProfileData != nullptr)
		delete [] ProfileData;
	if (ProfileDataXORCode != nullptr)
		delete [] ProfileDataXORCode;




	XMLdoc->Save(ConfigFileName);
	delete XMLdoc;
}













//-----------------------------------------------------------------------------
// считываем файл с настройками
//-----------------------------------------------------------------------------
bool LoadXMLSetupFile(bool NeedSafeMode)
{
	cXMLDocument *XMLdoc = new cXMLDocument;

	// устанавливаем базовые настройки
	InitSetup();


	// читаем данные
	if (!XMLdoc->Load(ConfigFileName)) {
		delete XMLdoc;
		SaveXMLSetupFile();
		return true;
	}



	// берем первый элемент в скрипте
	cXMLEntry *RootXMLEntry = XMLdoc->RootXMLEntry;



	// дополнительная проверка на содержимое конфигурационного файла
	if (RootXMLEntry == nullptr) {
		fprintf(stderr, "Game configuration file corrupted: %s\n", ConfigFileName);
		// файл поврежден, надо завершить работу с ним
		delete XMLdoc;
		// сохранить дефолтные настройки, перезаписав файл
		SaveXMLSetupFile();
		// и сказать игре что это "первый запуск"
		return true;
	}
	if (strcmp("AstroMenaceSettings", RootXMLEntry->Name)) {
		fprintf(stderr, "Game configuration file corrupted: %s\n", ConfigFileName);
		// файл поврежден, надо завершить работу с ним
		delete XMLdoc;
		// сохранить дефолтные настройки, перезаписав файл
		SaveXMLSetupFile();
		// и сказать игре что это "первый запуск"
		return true;
	}




	// если установлен NeedSafeMode, не грузим часть данных
	if (NeedSafeMode) goto LoadProfiles;


	if (XMLdoc->FindEntryByName(RootXMLEntry, "MenuLanguage") != nullptr) {
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MenuLanguage"), "value") != nullptr) {
			for (int i=0; i<vw_GetLanguageListCount(); i++) {
				if (!strcmp(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MenuLanguage"), "value"), vw_GetLanguageList()[i].code)) {
					Setup.MenuLanguage = i+1;
					break;
				}
			}
		}
	}
	if (XMLdoc->FindEntryByName(RootXMLEntry, "VoiceLanguage") != nullptr) {
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VoiceLanguage"), "value") != nullptr) {
			for (int i=0; i<vw_GetLanguageListCount(); i++) {
				if (!strcmp(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VoiceLanguage"), "value"), vw_GetLanguageList()[i].code)) {
					Setup.VoiceLanguage = i+1;
					break;
				}
			}
		}
	}
	if (XMLdoc->FindEntryByName(RootXMLEntry, "FontNumber") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "FontNumber"), "value") != nullptr)
			Setup.FontNumber = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "FontNumber"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "FontName") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "FontName"), "value") != nullptr)
			strcpy(Setup.FontName, XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "FontName"), "value"));
	if (XMLdoc->FindEntryByName(RootXMLEntry, "FontSize") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "FontSize"), "value") != nullptr)
			Setup.FontSize = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "FontSize"), "value");

	if (XMLdoc->FindEntryByName(RootXMLEntry, "Width") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "Width"), "value") != nullptr)
			Setup.Width = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "Width"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "Height") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "Height"), "value") != nullptr)
			Setup.Height = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "Height"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "BPP") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "BPP"), "value") != nullptr)
			Setup.BPP = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "BPP"), "value");

	if (XMLdoc->FindEntryByName(RootXMLEntry, "AspectRatio") != nullptr) {
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "AspectRatio"), "value") != nullptr) {
			if (!strcmp(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "AspectRatio"), "value"), "16:10")) {
				Setup.fAspectRatioWidth = 1228.0f;
				Setup.fAspectRatioHeight = 768.0f;
				Setup.iAspectRatioWidth = 1228;
				Setup.iAspectRatioHeight = 768;
			} else {
				Setup.fAspectRatioWidth = 1024.0f;
				Setup.fAspectRatioHeight = 768.0f;
				Setup.iAspectRatioWidth = 1024;
				Setup.iAspectRatioHeight = 768;
			}
		}
	}
	if (XMLdoc->FindEntryByName(RootXMLEntry, "CameraModeWithStandardAspectRatio") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "CameraModeWithStandardAspectRatio"), "value") != nullptr)
			Setup.CameraModeWithStandardAspectRatio = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "CameraModeWithStandardAspectRatio"), "value");

	if (XMLdoc->FindEntryByName(RootXMLEntry, "VBOCoreMode") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VBOCoreMode"), "value") != nullptr)
			Setup.VBOCoreMode = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VBOCoreMode"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "VAOCoreMode") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VAOCoreMode"), "value") != nullptr)
			Setup.VAOCoreMode = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VAOCoreMode"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "FBOCoreMode") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "FBOCoreMode"), "value") != nullptr)
			Setup.FBOCoreMode = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "FBOCoreMode"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "EqualOrMore128MBVideoRAM") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "EqualOrMore128MBVideoRAM"), "value") != nullptr)
			Setup.EqualOrMore128MBVideoRAM = XMLdoc->bGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "EqualOrMore128MBVideoRAM"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "HardwareMipMapGeneration") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "HardwareMipMapGeneration"), "value") != nullptr)
			Setup.HardwareMipMapGeneration = XMLdoc->bGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "HardwareMipMapGeneration"), "value");

	if (XMLdoc->FindEntryByName(RootXMLEntry, "TextureFilteringMode") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "TextureFilteringMode"), "value") != nullptr)
			Setup.TextureFilteringMode = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "TextureFilteringMode"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "TexturesQuality") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "TexturesQuality"), "value") != nullptr)
			Setup.TexturesQuality = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "TexturesQuality"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "MSAA") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MSAA"), "value") != nullptr)
			Setup.MSAA = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MSAA"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "CSAA") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "CSAA"), "value") != nullptr)
			Setup.CSAA = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "CSAA"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "VisualEffectsQuality") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VisualEffectsQuality"), "value") != nullptr)
			Setup.VisualEffectsQuality = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VisualEffectsQuality"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "AnisotropyLevel") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "AnisotropyLevel"), "value") != nullptr)
			Setup.AnisotropyLevel = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "AnisotropyLevel"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "TexturesCompressionType") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "TexturesCompressionType"), "value") != nullptr)
			Setup.TexturesCompressionType = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "TexturesCompressionType"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "UseGLSL") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "UseGLSL"), "value") != nullptr)
			Setup.UseGLSL = XMLdoc->bGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "UseGLSL"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "ShadowMap") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "ShadowMap"), "value") != nullptr)
			Setup.ShadowMap = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "ShadowMap"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "MaxPointLights") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MaxPointLights"), "value") != nullptr)
			Setup.MaxPointLights = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MaxPointLights"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "MusicSw") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MusicSw"), "value") != nullptr)
			Setup.MusicSw = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MusicSw"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "SoundSw") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "SoundSw"), "value") != nullptr)
			Setup.SoundSw = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "SoundSw"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "VoiceSw") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VoiceSw"), "value") != nullptr)
			Setup.VoiceSw = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VoiceSw"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "VSync") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VSync"), "value") != nullptr)
			Setup.VSync = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "VSync"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "Brightness") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "Brightness"), "value") != nullptr)
			Setup.Brightness = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "Brightness"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "ShowFPS") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "ShowFPS"), "value") != nullptr)
			Setup.ShowFPS = XMLdoc->bGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "ShowFPS"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "GameWeaponInfoType") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "GameWeaponInfoType"), "value") != nullptr)
			Setup.GameWeaponInfoType = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "GameWeaponInfoType"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "GameSpeed") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "GameSpeed"), "value") != nullptr)
			Setup.GameSpeed = XMLdoc->fGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "GameSpeed"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "LoadingHint") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "LoadingHint"), "value") != nullptr)
			Setup.LoadingHint = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "LoadingHint"), "value");



	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardDecreaseGameSpeed") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardDecreaseGameSpeed"), "value") != nullptr)
			Setup.KeyboardDecreaseGameSpeed = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardDecreaseGameSpeed"), "value"));
	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardResetGameSpeed") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardResetGameSpeed"), "value") != nullptr)
			Setup.KeyboardResetGameSpeed = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardResetGameSpeed"), "value"));
	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardIncreaseGameSpeed") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardIncreaseGameSpeed"), "value") != nullptr)
			Setup.KeyboardIncreaseGameSpeed = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardIncreaseGameSpeed"), "value"));
	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardGameWeaponInfoType") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardGameWeaponInfoType"), "value") != nullptr)
			Setup.KeyboardGameWeaponInfoType = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardGameWeaponInfoType"), "value"));
	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardPrimaryWeaponFireMode") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardPrimaryWeaponFireMode"), "value") != nullptr)
			Setup.KeyboardPrimaryWeaponFireMode = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardPrimaryWeaponFireMode"), "value"));
	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardSecondaryWeaponFireMode") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardSecondaryWeaponFireMode"), "value") != nullptr)
			Setup.KeyboardSecondaryWeaponFireMode = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyboardSecondaryWeaponFireMode"), "value"));


	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardLeft") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardLeft"), "value") != nullptr)
			Setup.KeyBoardLeft = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardLeft"), "value"));
	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardRight") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardRight"), "value") != nullptr)
			Setup.KeyBoardRight = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardRight"), "value"));
	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardUp") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardUp"), "value") != nullptr)
			Setup.KeyBoardUp = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardUp"), "value"));
	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardDown") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardDown"), "value") != nullptr)
			Setup.KeyBoardDown = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardDown"), "value"));
	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardPrimary") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardPrimary"), "value") != nullptr)
			Setup.KeyBoardPrimary = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardPrimary"), "value"));
	if (XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardSecondary") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardSecondary"), "value") != nullptr)
			Setup.KeyBoardSecondary = vw_KeyboardNameCode(XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "KeyBoardSecondary"), "value"));

	if (XMLdoc->FindEntryByName(RootXMLEntry, "MousePrimary") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MousePrimary"), "value") != nullptr)
			Setup.MousePrimary = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MousePrimary"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "MouseSecondary") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MouseSecondary"), "value") != nullptr)
			Setup.MouseSecondary = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MouseSecondary"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "JoystickPrimary") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "JoystickPrimary"), "value") != nullptr)
			Setup.JoystickPrimary = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "JoystickPrimary"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "JoystickSecondary") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "JoystickSecondary"), "value") != nullptr)
			Setup.JoystickSecondary = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "JoystickSecondary"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "JoystickNum") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "JoystickNum"), "value") != nullptr)
			Setup.JoystickNum = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "JoystickNum"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "JoystickDeadZone") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "JoystickDeadZone"), "value") != nullptr)
			Setup.JoystickDeadZone = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "JoystickDeadZone"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "ControlSensivity") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "ControlSensivity"), "value") != nullptr)
			Setup.ControlSensivity = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "ControlSensivity"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "MouseControl") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MouseControl"), "value") != nullptr)
			Setup.MouseControl = XMLdoc->bGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MouseControl"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "LastProfile") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "LastProfile"), "value") != nullptr)
			Setup.LastProfile = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "LastProfile"), "value");
	if (XMLdoc->FindEntryByName(RootXMLEntry, "MenuScript") != nullptr)
		if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MenuScript"), "value") != nullptr)
			Setup.MenuScript = XMLdoc->iGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, "MenuScript"), "value");


	for(int i=0; i<10; i++) {
		char Name[128];
		sprintf(Name, "HintStatus%i", i+1);
		if (XMLdoc->FindEntryByName(RootXMLEntry, Name) != nullptr)
			if (XMLdoc->GetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, Name), "value") != nullptr)
				Setup.NeedShowHint[i] = XMLdoc->bGetEntryAttribute(XMLdoc->FindEntryByName(RootXMLEntry, Name), "value");
	}




	//
	// заполняем таблицу рекордов
	//

	if ((XMLdoc->FindEntryByName(RootXMLEntry, "TopScores") != nullptr) && (XMLdoc->FindEntryByName(RootXMLEntry, "TopScores")->Content != nullptr)) {

		int TopScoresDataSize = strlen(XMLdoc->FindEntryByName(RootXMLEntry, "TopScores")->Content);
		unsigned char *TopScoresData = new unsigned char[TopScoresDataSize+1];
		unsigned char *TopScoresDataXORCode = new unsigned char[TopScoresDataSize+1];
		char *TopScoresResultString = new char[TopScoresDataSize+1];

		strcpy(TopScoresResultString, XMLdoc->FindEntryByName(RootXMLEntry, "TopScores")->Content);

		// первый цикл, восстанавливаем правильную последовательность, убираем все лишние элементы
		int k=0;
		for (int i=0; i<TopScoresDataSize; i++) {
			// берем только нужные символы
			if (TopScoresResultString[i] >= 97 && TopScoresResultString[i] <= 97+26) {
				TopScoresDataXORCode[k] = TopScoresResultString[i];
				k++;
			}
		}
		// находим правильное значение, т.к. загружали использую длину строки (!!!)
		TopScoresDataSize = k/3;

		// второй цикл, восстанавливаем последовательность структуры
		for (int i=0; i<TopScoresDataSize; i++) {
			int k1 = i;
			int k2 = TopScoresDataSize + i*2;

			unsigned char XORhash = TopScoresDataXORCode[k1];
			unsigned char XORdata = ((TopScoresDataXORCode[k2]-97)*10) + (TopScoresDataXORCode[k2+1]-97);

			TopScoresData[i] = XORdata^XORhash;
		}

		// третий цикл, распаковка Хаффмана
		uint8_t *dstVFS;
		int dsizeVFS;
		HAFFtoDATA(sizeof(sTopScores)*10, &dstVFS, (uint8_t *)TopScoresData, &dsizeVFS, TopScoresDataSize);
		delete [] TopScoresData;
		TopScoresDataSize = dsizeVFS;
		TopScoresData = (unsigned char *)dstVFS;

		// переносим данные в структуру
		memcpy(Setup.TopScores, TopScoresData, TopScoresDataSize);

		if (TopScoresResultString != nullptr)
			delete [] TopScoresResultString;
		if (TopScoresData != nullptr)
			delete [] TopScoresData;
		if (TopScoresDataXORCode != nullptr)
			delete [] TopScoresDataXORCode;
	}





LoadProfiles:

	//
	// загрузка профайлов пилотов
	//

	if ((XMLdoc->FindEntryByName(RootXMLEntry, "PilotsProfiles") != nullptr) && (XMLdoc->FindEntryByName(RootXMLEntry, "PilotsProfiles")->Content != nullptr)) {
		int ProfileDataSize = strlen(XMLdoc->FindEntryByName(RootXMLEntry, "PilotsProfiles")->Content);
		unsigned char *ProfileData = new unsigned char[ProfileDataSize+1];
		unsigned char *ProfileDataXORCode = new unsigned char[ProfileDataSize+1];
		char *ResultString = new char[ProfileDataSize+1];

		strcpy(ResultString, XMLdoc->FindEntryByName(RootXMLEntry, "PilotsProfiles")->Content);

		// первый цикл, восстанавливаем правильную последовательность, убираем все лишние элементы
		int k=0;
		for (unsigned int i=0; i<strlen(ResultString); i++) {
			// берем только нужные символы
			if (ResultString[i] >= 97 && ResultString[i] <= 97+26) {
				ProfileDataXORCode[k] = ResultString[i];
				k++;
			}
		}
		// находим правильное значение, т.к. загружали используя длину строки (!!!)
		ProfileDataSize = k/3;

		// второй цикл, восстанавливаем последовательность структуры
		for (int i=0; i<ProfileDataSize; i++) {
			int k1 = i;
			int k2 = ProfileDataSize + i*2;

			unsigned char XORhash = ProfileDataXORCode[k1];
			unsigned char XORdata = ((ProfileDataXORCode[k2]-97)*10) + (ProfileDataXORCode[k2+1]-97);

			ProfileData[i] = XORdata^XORhash;
		}

		// третий цикл, распаковка Хаффмана
		uint8_t *dstVFS;
		int dsizeVFS;
		HAFFtoDATA(sizeof(GameProfile)*5, &dstVFS, (uint8_t *)ProfileData, &dsizeVFS, ProfileDataSize);
		delete [] ProfileData;
		ProfileDataSize = dsizeVFS;
		ProfileData = (unsigned char *)dstVFS;

		// переносим данные в структуру
		memcpy(Setup.Profile, ProfileData, ProfileDataSize);

		if (ResultString != nullptr)
			delete [] ResultString;
		if (ProfileData != nullptr)
			delete [] ProfileData;
		if (ProfileDataXORCode != nullptr)
			delete [] ProfileDataXORCode;
	}







	//
	// делаем изменения, проверки и т.п.
	//

	if (Setup.KeyBoardUp == 0) Setup.KeyBoardUp = SDLK_UP;
	if (Setup.KeyBoardDown == 0) Setup.KeyBoardDown = SDLK_DOWN;
	if (Setup.KeyBoardLeft == 0) Setup.KeyBoardLeft = SDLK_LEFT;
	if (Setup.KeyBoardRight == 0) Setup.KeyBoardRight = SDLK_RIGHT;
	if (Setup.KeyBoardPrimary == 0) Setup.KeyBoardPrimary = SDLK_LCTRL;
	if (Setup.KeyBoardSecondary == 0) Setup.KeyBoardSecondary = SDLK_SPACE;
	if (Setup.MousePrimary == 0) Setup.MousePrimary = SDL_BUTTON_LEFT;
	if (Setup.MouseSecondary == 0) Setup.MouseSecondary = SDL_BUTTON_RIGHT;
	if (Setup.JoystickPrimary == -1) Setup.JoystickPrimary = 0;
	if (Setup.JoystickSecondary == -1) Setup.JoystickSecondary = 1;

	CurrentProfile = Setup.LastProfile;
	if (CurrentProfile != -1) CurrentMission = Setup.Profile[Setup.LastProfile].LastMission;

	if ((Setup.FontNumber > FontQuantity-1) || (Setup.FontNumber < 0)) Setup.FontNumber = 0;
	if ((Setup.FontSize > 18) || (Setup.FontSize < 14)) Setup.FontSize = 16;
	if (Setup.ControlSensivity > 10) Setup.ControlSensivity = 10;
	if (Setup.Brightness > 10) Setup.Brightness = 10;
	if (Setup.JoystickDeadZone > 10) Setup.JoystickDeadZone = 10;


	delete XMLdoc;

	return false;
}



