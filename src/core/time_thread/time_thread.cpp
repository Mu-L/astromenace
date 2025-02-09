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

/*
The idea of this code - time manipulations, what we need in the game. For example,
we could "pause" game by stop the time flowing, or just "slowdown" some objects.

This code provide "time thread" mechanism that allow manipulate time inside time
thread or even "stop" time inside time thread.

In common cases, if time manipulations feature not a goal, SDL_GetTicks() provide
same functionality.
*/

// TODO the main idea of time calculation should be revised for discrete time
//      Since we don't know how expensive SDL_GetTicks() is (on different platforms),
//      no reason call SDL_GetTicks() all the time for animation/gfx. One time update
//      for thread peer loop cycle should be enough.
//      * see UpdateAllObject3D() - we already use this idea for all objects update.
//      (!) make sure, all vw_GetTimeThread() usage changed to SDL_GetTicks() where
//      it is necessary.

// TODO (?) change concept of TimeThreads from 'time' in float to 'ticks' in uint32_t, so,
//      this will looks like SDL_GetTicks() work, but with time manipulations. From one side,
//      we don't need float at all (with specific accuracy and other float-related issues),
//      from another side, ticks in milliseconds that all we really need.

#include "../base.h"
#include "../math/math.h"
#include "SDL2/SDL.h"

namespace viewizard {

namespace {

struct sTimeThread {
    // current time thread pause status
    bool Paused{true};
    // last ticks, in order to care about time thread pause
    uint32_t LastGetTicks{0};
    // "time point", ticks of last speed change
    uint32_t DiffGetTicks{0};
    // current time thread speed
    float Speed{1.0f};
    // all "previous time" on last speed change
    float Buffer{0.0f};
};

// std::unordered_map - caller could use any integer thread number (negative also
// allowed), we need container with operator [], that create new element on access
// and could use any integer number as index.
std::unordered_map<int, sTimeThread> TimeThreadsMap;

} // unnamed namespace


/*
 * Initialize or re-initialize time for particular thread.
 */
void vw_InitTimeThread(int TimeThread)
{
    TimeThreadsMap[TimeThread].Paused = false;
    TimeThreadsMap[TimeThread].LastGetTicks = 0;
    TimeThreadsMap[TimeThread].DiffGetTicks = 0;
    TimeThreadsMap[TimeThread].Speed = 1.0f;
    TimeThreadsMap[TimeThread].Buffer = 0.0f;
}

/*
 * Release all threads.
 */
void vw_ReleaseAllTimeThread()
{
    TimeThreadsMap.clear();
}

/*
 * Get time for particular thread.
 */
float vw_GetTimeThread(int TimeThread)
{
    if (TimeThreadsMap.find(TimeThread) == TimeThreadsMap.end()) {
        std::cerr << __func__ << "(): " << "TimeThread was not initialized: " << TimeThread << "\n";
        return 0.0f;
    }

    assert(!TimeThreadsMap[TimeThread].Paused);

    // time manipulations
    if (TimeThreadsMap[TimeThread].Speed != 1.0f) {
        // calculate time from "time point" (DiffGetTicks), when speed was changed last time, till now
        // FIXME static_cast, we could have an issue with float type value
        float RealTimeThread = (static_cast<float>(SDL_GetTicks() - TimeThreadsMap[TimeThread].DiffGetTicks) *
                                TimeThreadsMap[TimeThread].Speed) / 1000.0f;
        // add "previous time" from time buffer
        return RealTimeThread + TimeThreadsMap[TimeThread].Buffer;
    }

    // FIXME static_cast, we could have an issue with float type value
    return TimeThreadsMap[TimeThread].Buffer +
           static_cast<float>(SDL_GetTicks() - TimeThreadsMap[TimeThread].DiffGetTicks) / 1000.0f;
}

/*
 * Pause all time threads.
 */
void vw_PauseTimeThreads()
{
    for (auto &TimeThread : TimeThreadsMap) {
        if (!TimeThread.second.Paused) {
            TimeThread.second.LastGetTicks = SDL_GetTicks();
            TimeThread.second.Paused = true;
        }
    }
}

/*
 * Resume all previously paused time threads.
 */
void vw_ResumeTimeThreads()
{
    for (auto &TimeThread : TimeThreadsMap) {
        if (TimeThread.second.Paused) {
            TimeThread.second.DiffGetTicks += SDL_GetTicks() - TimeThread.second.LastGetTicks;
            TimeThread.second.Paused = false;
        }
    }
}

/*
 * Set time thread speed for particular thread.
 */
void vw_SetTimeThreadSpeed(int TimeThread, float NewSpeed)
{
    if (TimeThreadsMap.find(TimeThread) == TimeThreadsMap.end()) {
        std::cerr << __func__ << "(): " << "TimeThread was not initialized: " << TimeThread << "\n";
        return;
    }

    // store "previous time" in the time buffer
    // FIXME static_cast, we could have an issue with float type value
    TimeThreadsMap[TimeThread].Buffer += (static_cast<float>(SDL_GetTicks() - TimeThreadsMap[TimeThread].DiffGetTicks) *
                                          TimeThreadsMap[TimeThread].Speed) / 1000.0f;
    // store "time point", when speed was changed
    TimeThreadsMap[TimeThread].DiffGetTicks = SDL_GetTicks();
    TimeThreadsMap[TimeThread].Speed = NewSpeed;
}

} // viewizard namespace
