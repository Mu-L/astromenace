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

// TODO struct sMusic should use encapsulation (switch to class)

/*
In the case of music we based on several main principles:
1. All music connected to its name, so, name is the key (for map).
   If music have a 'loop' part, only main part (its file name) taken into account
   and used as key, 'loop' part file name will be ignored.
2. Only one copy of music can be played simultaneously.
   This is connected not only to p.1, but also to our stream buffers realization,
   that will not allow create more then one stream buffer for same source (file name).

Try to avoid call music-related functions in loop (all the time, frequently),
in theory, we should call music-related functions for actions and only
vw_UpdateMusic() and vw_IsAnyMusicPlaying() designed to be called in loop.
*/

#include "buffer.h"
#include "SDL2/SDL.h"

namespace viewizard {

namespace {

struct sMusic {
    ~sMusic()
    {
        if (!alIsSource(Source)) {
            return;
        }
        // stop playing before other actions
        alSourceStop(Source);
        if (Stream) {
            vw_UnqueueStreamBuffer(Stream, Source);
        }
        alDeleteSources(1, &Source);
        ResetALError();
    }

    void FadeIn(float EndVol, uint32_t Ticks);
    void FadeOut(uint32_t Ticks);
    bool Update(uint32_t CurrentTick);
    void SetGlobalVolume(float NewGlobalVolume);

    sStreamBuffer *Stream{nullptr};
    ALuint Source{0};
    float LocalVolume{0.0f};
    float GlobalVolume{0.0f};
    bool Looped{false};
    std::string LoopPart{};

    // effects-related variables
    bool FadeInSwitch{false};
    float FadeEndVol{0.0f};
    float FadeStartVol{0.0f};
    bool FadeOutSwitch{false};
    uint32_t FadeTicks{0};
    uint32_t FadePeriod{0};
    uint32_t LastTick{0};
};

std::unordered_map<std::string, sMusic> MusicMap;

} // unnamed namespace


/*
 * Create and play music.
 */
bool vw_PlayMusic(const std::string &Name, float _LocalVolume, float _GlobalVolume,
                  bool Loop, const std::string &LoopFileName)
{
    if (Name.empty()) { // LoopFileName could be empty
        return false;
    }

    auto tmpMusic = MusicMap.find(Name); // check, did we already create or not
    if (tmpMusic != MusicMap.end()) {
        return true;
    }

    alGenSources(1, &MusicMap[Name].Source); // create entry on first access
    if (!CheckALError(__func__)) {
        return false;
    }

    MusicMap[Name].LocalVolume = _LocalVolume;
    MusicMap[Name].GlobalVolume = _GlobalVolume;
    MusicMap[Name].LoopPart = LoopFileName;
    MusicMap[Name].FadeStartVol = _LocalVolume;
    MusicMap[Name].FadeEndVol = _LocalVolume;
    MusicMap[Name].LastTick = SDL_GetTicks();

    // we don't use position and velocity for music
    constexpr ALfloat SourcePos[]{0.0f, 0.0f, 0.0f};
    constexpr ALfloat SourceVel[]{0.0f, 0.0f, 0.0f};

    alSourcef(MusicMap[Name].Source, AL_PITCH, 1.0);
    alSourcef(MusicMap[Name].Source, AL_GAIN, _GlobalVolume * _LocalVolume);
    alSourcefv(MusicMap[Name].Source, AL_POSITION, SourcePos);
    alSourcefv(MusicMap[Name].Source, AL_VELOCITY, SourceVel);
    alSourcei(MusicMap[Name].Source, AL_SOURCE_RELATIVE, AL_TRUE);
    alSourcei(MusicMap[Name].Source, AL_LOOPING, AL_FALSE);
    ResetALError();
    MusicMap[Name].Looped = Loop;

    MusicMap[Name].Stream = vw_CreateStreamBufferFromOGG(Name, LoopFileName);
    if (!MusicMap[Name].Stream) {
        return false;
    }

    if (!vw_QueueStreamBuffer(MusicMap[Name].Stream, MusicMap[Name].Source)) {
        return false;
    }

    alSourcePlay(MusicMap[Name].Source);
    if (!CheckALError(__func__)) {
        return false;
    }

    return true;
}

/*
 * Update music status and calculate effects.
 */
bool sMusic::Update(uint32_t CurrentTick)
{
    vw_UpdateStreamBuffer(Stream, Source, Looped, LoopPart);

    // we could play music during SDL_Init(), when SDL_GetTicks() reset to 0
    if (LastTick > CurrentTick) {
        LastTick = CurrentTick;
        return true;
    }
    uint32_t TicksDelta = CurrentTick - LastTick;

    if (FadeInSwitch) {
        FadeTicks += TicksDelta;
        // we are safe with static_cast here, since FadeTicks and FadePeriod
        // will not exceed 'float' in our case for sure (usually, <10000 ticks)
        LocalVolume = FadeEndVol * static_cast<float>(FadeTicks) / static_cast<float>(FadePeriod);
        if (LocalVolume >= FadeEndVol) {
            LocalVolume = FadeEndVol;
            FadeInSwitch = false;
        }
        alSourcef(Source, AL_GAIN, GlobalVolume * LocalVolume );
        ResetALError();
    }

    if (FadeOutSwitch) {
        FadeTicks += TicksDelta;
        // we are safe with static_cast here, since FadeTicks and FadePeriod
        // will not exceed 'float' in our case for sure (usually, <10000 ticks)
        LocalVolume = 1.0f - FadeStartVol * static_cast<float>(FadeTicks) / static_cast<float>(FadePeriod);
        if (LocalVolume < 0.0f) {
            LocalVolume = 0.0f;
            FadeOutSwitch = false;
        }
        alSourcef(Source, AL_GAIN, GlobalVolume * LocalVolume);
        ResetALError();
        if (!FadeOutSwitch) { // use boolean check in order to avoid float's comparison
            return false;
        }
    }

    if (CheckALSourceState(Source, AL_STOPPED)) {
        return false;
    }

    LastTick = CurrentTick;

    return true;
}

/*
 * Set global volume.
 */
void sMusic::SetGlobalVolume(float NewGlobalVolume)
{
    if (!alIsSource(Source)) {
        return;
    }

    GlobalVolume = NewGlobalVolume;
    alSourcef(Source, AL_GAIN, GlobalVolume * LocalVolume);
}

/*
 * Music fade-in setup.
 */
void sMusic::FadeIn(float EndVol, uint32_t Ticks)
{
    if (FadeInSwitch) {
        return; // don't reset current fade-in
    }
    FadeInSwitch = true;
    FadeOutSwitch = false;
    FadeEndVol = EndVol;
    FadeTicks = 0;
    FadePeriod = Ticks;
    LastTick = SDL_GetTicks();
}

/*
 * Music fade-out setup.
 */
void sMusic::FadeOut(uint32_t Ticks)
{
    if (FadeOutSwitch) {
        return;  // don't reset current fade-out
    }
    FadeOutSwitch = true;
    FadeInSwitch = false;
    FadeStartVol = LocalVolume;
    FadeTicks = 0;
    FadePeriod = Ticks;
    LastTick = SDL_GetTicks();
}

/*
 * Fade-out all music themes, except provided.
 */
void vw_FadeOutAllMusicWithException(const std::string &Name, uint32_t Ticks,
                                     float ExceptionFadeInEndVol, uint32_t ExceptionFadeInTicks)
{
    for (auto &tmpMusic : MusicMap) {
        if (tmpMusic.first != Name) {
            if (alIsSource(tmpMusic.second.Source) && CheckALSourceState(tmpMusic.second.Source, AL_PLAYING)) {
                tmpMusic.second.FadeOut(Ticks);
            }
        } else {
            // fade-in exception music theme in case we fade-out it
            if (tmpMusic.second.FadeOutSwitch && ExceptionFadeInEndVol > 0.0f) {
                tmpMusic.second.FadeIn(ExceptionFadeInEndVol, ExceptionFadeInTicks);
            }
        }
    }
}

/*
 * Check, is music theme playing.
 */
bool vw_IsMusicPlaying(const std::string &Name)
{
    auto tmpMusic = MusicMap.find(Name);
    if (tmpMusic != MusicMap.end()
        && alIsSource((*tmpMusic).second.Source)
        && CheckALSourceState((*tmpMusic).second.Source, AL_PLAYING))
        return true;

    return false;
}

/*
 * Check, is any music theme playing.
 */
bool vw_IsAnyMusicPlaying()
{
    for (auto &tmpMusic : MusicMap) {
        if (alIsSource(tmpMusic.second.Source) && CheckALSourceState(tmpMusic.second.Source, AL_PLAYING)) {
            return true;
        }
    }

    return false;
}

/*
 * Release particular music theme by name. Also could be used for "stop" playing.
 */
void vw_ReleaseMusic(const std::string &Name)
{
    if (Name.empty()) {
        return;
    }

    MusicMap.erase(Name);
}

/*
 * Release all music. Also could be used for "stop" playing all music themes.
 */
void vw_ReleaseAllMusic()
{
    MusicMap.clear();
}

/*
 * Update all music themes status and calculate effects.
 */
void vw_UpdateMusic(uint32_t CurrentTick)
{
    // NOTE use std::erase_if here (since C++20)
    for (auto iter = MusicMap.begin(); iter != MusicMap.end();) {
        if (!iter->second.Update(CurrentTick)) {
            iter = MusicMap.erase(iter);
        } else {
            ++iter;
        }
    }
}

/*
 * Set music fade-in.
 */
void vw_SetMusicFadeIn(const std::string &Name, float EndVol, uint32_t Ticks)
{
    auto tmpMusic = MusicMap.find(Name);
    if (tmpMusic != MusicMap.end()) {
        tmpMusic->second.FadeIn(EndVol, Ticks);
    }
}

/*
 * Set global music volume.
 */
void vw_SetMusicGlobalVolume(float NewGlobalVolume)
{
    for (auto &tmpMusic : MusicMap) {
        tmpMusic.second.SetGlobalVolume(NewGlobalVolume);
    }
}

} // viewizard namespace
