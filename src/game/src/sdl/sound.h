// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <SDL3/SDL.h>
#include <vector>
#include "../types.h"
const int MAX_CHANNELS=16;
enum sound_priority_t { LOW, NORMAL, HIGH };
class Sound {
public:
    Sound();
    ~Sound();
    SDL_AudioDeviceID Device() const { return device; }
private:
    SDL_AudioDeviceID device=0;
};
class SoundBuffer {
public:
    ~SoundBuffer();
    bool LoadWAV(char filename[]);
    bool Play(bool loop,sound_priority_t priority);
    void Stop();
    bool IsPlaying();
    sound_priority_t Priority() const { return priority; }
    void Update();
private:
    SDL_AudioStream* stream=nullptr;
    std::vector<byte> data;
    bool looping=false;
    sound_priority_t priority=NORMAL;
};
extern Sound* g_sound;
bool SoundInit();
void SoundShutdown();
void SoundUpdate();
