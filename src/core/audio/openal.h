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

// FIXME don't call ResetALError() all around, use it only before al call we need to check

/*
Should be used for 'sound' system internal purposes only. Don't include into other
sources, you are free to change or add methods to sound.h header file instead.
*/

#ifndef CORE_AUDIO_OPENAL_H
#define CORE_AUDIO_OPENAL_H

#include "../base.h"

#if defined(__APPLE__) && defined(__MACH__)
#include "OpenAL/al.h"
#include "OpenAL/alc.h"
#else // defined(__APPLE__) && defined(__MACH__)
#include "AL/al.h"
#include "AL/alc.h"
#endif // defined(__APPLE__) && defined(__MACH__)
#include "AL/alut.h"

namespace viewizard {

ALboolean CheckALCError(ALCdevice *Device, const char *FunctionName);
ALboolean CheckALError(const char *FunctionName);
void ResetALError();
ALboolean CheckALUTError(const char *FunctionName);
bool CheckALSourceState(ALuint Source, ALint State);

} // viewizard namespace

#endif // CORE_AUDIO_OPENAL_H
