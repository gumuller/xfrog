// SPDX-License-Identifier: GPL-2.0-or-later
#include "sound.h"
#include "../cfs.h"
#include <algorithm>
#include <stdexcept>
static std::vector<SoundBuffer*> buffers;
Sound* g_sound=nullptr;
static void Check(bool ok) { if(!ok) throw std::runtime_error(SDL_GetError()); }
Sound::Sound() {
    device=SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,nullptr);
    if(!device) throw std::runtime_error(SDL_GetError());
    SDL_Log("Audio: %s",SDL_GetAudioDeviceName(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK));
}
Sound::~Sound() { SDL_CloseAudioDevice(device); }
bool SoundInit() { g_sound=new Sound; return true; }
void SoundShutdown() { delete g_sound; g_sound=nullptr; }
void SoundUpdate() { for(auto* b:buffers) b->Update(); }
SoundBuffer::~SoundBuffer() {
    buffers.erase(std::remove(buffers.begin(),buffers.end(),this),buffers.end());
    SDL_DestroyAudioStream(stream);
}
bool SoundBuffer::LoadWAV(char filename[]) {
    CFS_File file(filename);
    if(!file.Open()) return false;
    std::vector<byte> wav(file.Size());
    if(!file.Read(wav.data(),1,file.Size())) return false;
    SDL_AudioSpec spec;
    Uint8* decoded=nullptr;
    Uint32 length=0;
    Check(SDL_LoadWAV_IO(SDL_IOFromConstMem(wav.data(),wav.size()),true,&spec,&decoded,&length));
    data.assign(decoded,decoded+length);
    SDL_free(decoded);
    stream=SDL_CreateAudioStream(&spec,nullptr);
    if(!stream) throw std::runtime_error(SDL_GetError());
    Check(SDL_BindAudioStream(g_sound->Device(),stream));
    buffers.push_back(this);
    return true;
}
bool SoundBuffer::Play(bool loop,sound_priority_t p) {
    if(!stream || IsPlaying()) return false;
    int active=0;
    SoundBuffer* victim=nullptr;
    for(auto* b:buffers) if(b->IsPlaying()) {
        ++active;
        if(b->Priority()<p && (!victim || b->Priority()<victim->Priority())) victim=b;
    }
    if(active>=MAX_CHANNELS) {
        if(!victim) return false;
        victim->Stop();
    }
    priority=p; looping=loop;
    Check(SDL_ClearAudioStream(stream));
    Check(SDL_PutAudioStreamData(stream,data.data(),int(data.size())));
    if(loop) Check(SDL_PutAudioStreamData(stream,data.data(),int(data.size())));
    else Check(SDL_FlushAudioStream(stream));
    return true;
}
void SoundBuffer::Stop() { looping=false; if(stream) Check(SDL_ClearAudioStream(stream)); }
bool SoundBuffer::IsPlaying() {
    return stream && (looping || SDL_GetAudioStreamQueued(stream)>0 || SDL_GetAudioStreamAvailable(stream)>0);
}
void SoundBuffer::Update() {
    if(looping && SDL_GetAudioStreamQueued(stream)<int(data.size()))
        Check(SDL_PutAudioStreamData(stream,data.data(),int(data.size())));
}
