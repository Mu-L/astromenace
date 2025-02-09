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

// FIXME fix mess with vw_PlaySound() AtType parameter, probably, we need setup this
//       by additional function call instead of hard code it

// TODO struct sSound should use encapsulation (switch to class)

#include "buffer.h"
#include "../math/math.h"
#include "SDL2/SDL.h"

namespace viewizard {

namespace {

struct sSound {
    ~sSound()
    {
        if (!alIsSource(Source)) {
            return;
        }
        // stop playing before other actions
        alSourceStop(Source);
        alDeleteSources(1, &Source);
        ResetALError();
    }

    void Replay();
    void Stop(uint32_t StopDelayTicks);
    void SetLocation(float x, float y, float z);
    void SetGlobalVolume(float NewMainVolume);

    std::string FileName{};

    ALuint Source{0};
    float LocalVolume{0.0f};
    float GlobalVolume{0.0f};
    bool AllowedStop{false}; // allowed stop during vw_StopAllSoundsIfAllowed() call

    uint32_t LastUpdateTick{0};

    uint32_t DestroyPeriod{0};
    uint32_t DestroyTicks{0};
};

std::unordered_map<unsigned int, sSound> SoundsMap;

} // unnamed namespace


/*
 * Generate new unique sound ID. Returned number should be non zero,
 * and sounds map should not have an element with this key.
 */
static unsigned int GenerateUniqueSoundID()
{
    static unsigned int LastSoundID{0};

    LastSoundID++;

    // since we could reach the "unsigned int" maximum (4294967295) and back to 0 again
    while (!LastSoundID && SoundsMap.find(LastSoundID) != SoundsMap.end()) {
        LastSoundID++;
    }

    return LastSoundID;
}

/*
 * Load sound buffer data according to file extension.
 */
unsigned int vw_LoadSoundBuffer(const std::string &Name)
{
    if (vw_CheckFileExtension(Name, ".wav")) {
        return vw_CreateSoundBufferFromWAV(Name);
    } else if (vw_CheckFileExtension(Name, ".ogg")) {
        return vw_CreateSoundBufferFromOGG(Name);
    }

    return 0;
}

/*
 * Play sound.
 */
unsigned int vw_PlaySound(const std::string &Name, float LocalVolume, float GlobalVolume,
                          const sVECTOR3D &Location, bool Relative, bool AllowStop, int AtType)
{
    if (Name.empty()) {
        return 0;
    }

    unsigned int tmpSoundID = GenerateUniqueSoundID();

    SoundsMap[tmpSoundID].Source = 0; // create entry on first access
    SoundsMap[tmpSoundID].LastUpdateTick = SDL_GetTicks();
    SoundsMap[tmpSoundID].FileName = Name;
    SoundsMap[tmpSoundID].DestroyPeriod = 0;
    SoundsMap[tmpSoundID].DestroyTicks = 0;
    SoundsMap[tmpSoundID].AllowedStop = AllowStop;

    ALuint Buffer = vw_LoadSoundBuffer(Name);
    if (!Buffer) {
        SoundsMap.erase(tmpSoundID);
        return 0;
    }

    SoundsMap[tmpSoundID].LocalVolume = LocalVolume;
    SoundsMap[tmpSoundID].GlobalVolume = GlobalVolume;

    // position of the source sound
    ALfloat SourcePos[]{Location.x, Location.y, Location.z};
    // velocity of the source sound
    constexpr ALfloat SourceVel[]{0.0f, 0.0f, 0.0f};

    // bind the buffer with the source
    alGenSources(1, &SoundsMap[tmpSoundID].Source);
    if (!CheckALError(__func__)) {
        SoundsMap.erase(tmpSoundID);
        return 0;
    }

    alSourcei(SoundsMap[tmpSoundID].Source, AL_BUFFER, Buffer);

    alSourcef(SoundsMap[tmpSoundID].Source, AL_GAIN, GlobalVolume * LocalVolume);
    alSourcefv(SoundsMap[tmpSoundID].Source, AL_POSITION, SourcePos);
    alSourcefv(SoundsMap[tmpSoundID].Source, AL_VELOCITY, SourceVel);
    alSourcei(SoundsMap[tmpSoundID].Source, AL_LOOPING, false);
    alSourcei(SoundsMap[tmpSoundID].Source, AL_SOURCE_RELATIVE, Relative);

    // attenuation-related factors
    if (AtType == 1) {
        alSourcef(SoundsMap[tmpSoundID].Source, AL_REFERENCE_DISTANCE, 30.0f);
        alSourcef(SoundsMap[tmpSoundID].Source, AL_MAX_DISTANCE, 250.0f);
        alSourcef(SoundsMap[tmpSoundID].Source, AL_ROLLOFF_FACTOR, 0.5f);
    } else if (AtType == 2) {
        alSourcef(SoundsMap[tmpSoundID].Source, AL_REFERENCE_DISTANCE, 150.0f);
        alSourcef(SoundsMap[tmpSoundID].Source, AL_MAX_DISTANCE, 600.0f);
        alSourcef(SoundsMap[tmpSoundID].Source, AL_ROLLOFF_FACTOR, 0.2f);
    }

    ResetALError();

    alSourcePlay(SoundsMap[tmpSoundID].Source);
    if (!CheckALError(__func__)) {
        SoundsMap.erase(tmpSoundID);
        return 0;
    }

    return tmpSoundID;
}

/*
 * Replay sound.
 */
void sSound::Replay()
{
    if (!alIsSource(Source)) {
        return;
    }

    alSourceRewind(Source);
    alSourcePlay(Source);
}

/*
 * Stop sound.
 * Note, that in this method we stop the sound, but it will be released in Update()
 * call if nothing happens (replay(), for example).
 */
void sSound::Stop(uint32_t StopDelayTicks)
{
    if (!alIsSource(Source)) {
        return;
    }

    if (!StopDelayTicks) {
        alSourceStop(Source);
        ResetALError();
    } else {
        DestroyPeriod = StopDelayTicks;
        DestroyTicks = 0;
    }
}

/*
 * Set source location.
 */
void sSound::SetLocation(float x, float y, float z)
{
    if (!alIsSource(Source)) {
        return;
    }

    ALfloat SourcePos[] = {x, y, z};
    alSourcefv(Source, AL_POSITION, SourcePos);
    ResetALError();
}

/*
 * Set global volume.
 */
void sSound::SetGlobalVolume(float NewGlobalVolume)
{
    if (!alIsSource(Source)) {
        return;
    }

    GlobalVolume = NewGlobalVolume;
    alSourcef(Source, AL_GAIN, GlobalVolume * LocalVolume);
    ResetALError();
}

/*
 * Release all sounds.
 */
void vw_ReleaseAllSounds()
{
    SoundsMap.clear();
}

/*
 * Stop (release) only allowed sounds.
 */
void vw_StopAllSoundsIfAllowed()
{
    // NOTE use std::erase_if here (since C++20)
    for (auto iter = SoundsMap.begin(); iter != SoundsMap.end();) {
        if (iter->second.AllowedStop) {
            // stopped sound will be released on update() call,
            // so, no reason stop sound, release it right now
            iter = SoundsMap.erase(iter);
        } else {
            ++iter;
        }
    }
}

/*
 * Check, is sound available (created) or not.
 */
bool vw_IsSoundAvailable(unsigned int ID)
{
    if (!ID) {
        return false;
    }

    auto tmpSound = SoundsMap.find(ID);
    if (tmpSound != SoundsMap.end()) {
        return true;
    }

    return false;
}

/*
 * Replay from the beginning first sound, found by name.
 */
unsigned int vw_ReplayFirstFoundSound(const std::string &Name)
{
    if (Name.empty()) {
        return 0;
    }

    for (auto &tmpSound : SoundsMap) {
        if (tmpSound.second.FileName == Name) {
            tmpSound.second.Replay();
            return tmpSound.first;
        }
    }

    return 0;
}

/*
 * Update all sounds.
 */
void vw_UpdateSound(uint32_t CurrentTick)
{
    for (auto iter = SoundsMap.begin(); iter != SoundsMap.end();) {
        bool NeedRelease{false};
        // calculate, how long we are playing this sound
        uint32_t DeltaTicks = CurrentTick - iter->second.LastUpdateTick;
        iter->second.LastUpdateTick = CurrentTick;

        if (alIsSource(iter->second.Source) && iter->second.DestroyPeriod > 0) {
            iter->second.DestroyTicks += DeltaTicks;

            if (iter->second.DestroyTicks < iter->second.DestroyPeriod) {
                // we are safe with static_cast here, since DestroyTicks and DestroyPeriod
                // will not exceed 'float' in our case for sure (usually, <10000 ticks)
                alSourcef(iter->second.Source, AL_GAIN,
                          iter->second.GlobalVolume * iter->second.LocalVolume *
                          (1.0f - static_cast<float>(iter->second.DestroyTicks) /
                           static_cast<float>(iter->second.DestroyPeriod)));
                ResetALError();
            } else {
                // release, volume less or equal zero
                NeedRelease = true;
            }
        }

        // release, if it is stopped (we don't reuse sound, no reason leave it)
        if (alIsSource(iter->second.Source) && CheckALSourceState(iter->second.Source, AL_STOPPED)) {
            NeedRelease = true;
        }

        // NOTE (?) use std::erase_if here (since C++20)
        if (NeedRelease) {
            iter = SoundsMap.erase(iter);
        } else {
            ++iter;
        }
    }
}

/*
 * Set sound global volume.
 */
void vw_SetSoundGlobalVolume(const std::string &Name, float NewGlobalVolume)
{
    if (Name.empty()) {
        return;
    }

    for (auto &tmpSound : SoundsMap) {
        if (tmpSound.second.FileName == Name) {
            tmpSound.second.SetGlobalVolume(NewGlobalVolume);
            return;
        }
    }
}

/*
 * Set sound location.
 */
void vw_SetSoundLocation(unsigned int ID, float x, float y, float z)
{
    if (!ID) {
        return;
    }

    auto tmpSound = SoundsMap.find(ID);
    if (tmpSound != SoundsMap.end()) {
        tmpSound->second.SetLocation(x, y, z);
    }
}

/*
 * Stop sound with delay.
 */
void vw_StopSound(unsigned int ID, uint32_t StopDelayTicks)
{
    if (!ID) {
        return;
    }

    auto tmpSound = SoundsMap.find(ID);
    if (tmpSound != SoundsMap.end()) {
        tmpSound->second.Stop(StopDelayTicks);
    }
}

} // viewizard namespace
