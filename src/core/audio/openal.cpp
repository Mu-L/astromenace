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

#include "openal.h"

namespace viewizard {

/*
 * Check ALC errors.
 */
ALboolean CheckALCError(ALCdevice *Device, const char *FunctionName)
{
    ALenum ErrCode;
    if ((ErrCode = alcGetError(Device)) != ALC_NO_ERROR) {
        std::cerr << FunctionName << "(): " << "ALC error: " << alcGetString(Device, ErrCode) << "\n";
        return AL_FALSE;
    }
    return AL_TRUE;
}

/*
 * Check AL errors.
 */
ALboolean CheckALError(const char *FunctionName)
{
    ALenum ErrCode;
    if ((ErrCode = alGetError()) != AL_NO_ERROR) {
        std::cerr << FunctionName << "(): " << "OpenAL error: " << alGetString(ErrCode) << "\n";
        return AL_FALSE;
    }
    return AL_TRUE;
}

/*
 * Reset AL errors.
 */
void ResetALError()
{
    alGetError();
}

/*
 * Check ALUT errors.
 */
ALboolean CheckALUTError(const char *FunctionName)
{
    ALenum ErrCode;
    if ((ErrCode = alutGetError()) != ALUT_ERROR_NO_ERROR) {
        std::cerr << FunctionName << "(): " << "OpenAL alut error: " << alutGetErrorString(ErrCode) << "\n";
        return AL_FALSE;
    }
    return AL_TRUE;
}

/*
 * Check OpenAL source status.
 */
bool CheckALSourceState(ALuint Source, ALint State)
{
    ALint tmpState;
    alGetSourcei(Source, AL_SOURCE_STATE, &tmpState);
    ResetALError();
    return (tmpState == State);
}

} // viewizard namespace
