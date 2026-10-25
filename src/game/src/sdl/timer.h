// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <SDL3/SDL.h>
#include <algorithm>
class Timer {
public:
    Timer() : start(SDL_GetTicksNS()), previous(start) {}
    float GetTime() { return float((SDL_GetTicksNS()-start)/1e9); }
    void Time() {
        Uint64 now=SDL_GetTicksNS();
        delta=std::min(float((now-previous)/1e9),0.05f);
        previous=now;
#ifdef XFROG_TESTING
        delta=testDelta;
#endif
    }
    float GetDelta() const { return delta; }
    float GetFPS() const { return delta>0 ? 1.0f/delta : 0; }
    void Reset() { previous=SDL_GetTicksNS(); delta=0; }
#ifdef XFROG_TESTING
    void SetTestDelta(float value) { testDelta=value; }
#endif
private:
    Uint64 start,previous;
    float delta=0;
#ifdef XFROG_TESTING
    float testDelta=1.0f/60;
#endif
};
extern Timer* g_timer;
bool TimerInit();
void TimerShutdown();
