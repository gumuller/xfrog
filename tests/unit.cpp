// SPDX-License-Identifier: GPL-2.0-or-later
#include "input.h"
#include "timer.h"
#include "video.h"
#include "sound.h"
#include "cfs.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <functional>
#include <memory>
#include <stdexcept>
#include <string>
#include <vector>
static int checks=0;
static void Check(bool ok,const char* message) {
    ++checks;
    if(!ok) throw std::runtime_error(message);
}
static void Throws(const std::function<void()>& action) {
    bool threw=false;
    try { action(); } catch(const std::exception&) { threw=true; }
    Check(threw,"invalid operation must report an error");
}
static void InputTests() {
    TimerInit(); InputInit();
    Check(InputCheckKey(KEY_RIGHT,UP),"keys start released");
    Check(!InputCheckKey(-1,DOWN) && !InputCheckKey(NUM_OF_KEYS,UP),"invalid key queries are bounded");
    SDL_Event event={}; event.type=SDL_EVENT_KEY_DOWN; event.key.scancode=SDL_SCANCODE_RIGHT; event.key.down=true;
    g_input->HandleEvent(event);
    Check(InputCheckKey(KEY_RIGHT,DOWN),"keydown sets held state");
    Check(InputCheckKey(KEY_RIGHT,TICK),"keydown emits a tick");
    Check(!InputCheckKey(KEY_RIGHT,TICK),"tick is consumed exactly once");
    event.key.repeat=true; g_input->HandleEvent(event);
    Check(!InputCheckKey(KEY_RIGHT,TICK),"OS repeat cannot duplicate game ticks");
    event.key.repeat=false; event.type=SDL_EVENT_KEY_UP; event.key.down=false; g_input->HandleEvent(event);
    Check(InputCheckKey(KEY_RIGHT,UP) && !InputCheckKey(KEY_RIGHT,TICK),"keyup releases key and clears tick");
    event.type=SDL_EVENT_KEY_DOWN; event.key.down=true; g_input->HandleEvent(event);
    g_input->FocusManager();
    Check(InputCheckKey(KEY_RIGHT,UP) && !InputCheckKey(KEY_RIGHT,TICK),"focus reset removes held keys and pending ticks");
    event.key.scancode=SDL_Scancode(NUM_OF_KEYS); g_input->HandleEvent(event);
    Check(InputCheckKey(KEY_RIGHT,UP),"invalid keyboard event is ignored");
    InputShutdown(); TimerShutdown();
}
static void SurfaceTests(const char* directory) {
    g_video=new Video;
    Check(g_video->SetMode(6,4) && g_video->CreateFlipSystem(),"small framebuffer initializes");
    Surface source;
    Check(!source.Create(0,2,MEM_SYSTEM) && !source.Create(3,-1,MEM_SYSTEM),"invalid surface dimensions rejected");
    Check(source.Create(3,2,MEM_SYSTEM),"surface creates");
    byte pattern[]={0,1,2,3,4,5}; std::copy(pattern,pattern+6,source.Lock()); source.Unlock();
    for(bool transparent:{false,true}) for(int y:{-3,-1,0,3,4}) for(int x:{-4,-1,0,4,6}) {
        g_video->ClearBackSurface(9); source.Blit(x,y,transparent);
        for(int dy=0;dy<4;++dy) for(int dx=0;dx<6;++dx) {
            byte expected=9;
            if(dx>=x && dx<x+3 && dy>=y && dy<y+2) {
                byte pixel=pattern[(dy-y)*3+dx-x];
                if(!transparent || pixel) expected=pixel;
            }
            Check(g_video->BackBuffer()[dy*6+dx]==expected,"clipping and transparent blits preserve expected pixels");
        }
    }
    source.Mirror(); byte mirrored[]={2,1,0,5,4,3};
    Check(std::equal(mirrored,mirrored+6,source.Lock()),"mirror reverses rows independently"); source.Unlock();
    source.Mirror(); Check(std::equal(pattern,pattern+6,source.Lock()),"two mirrors restore source"); source.Unlock();
    g_video->ClearBackSurface(7);
    g_video->PutPixel(-1,0,99); g_video->PutPixel(6,0,99); g_video->PutPixel(0,-1,99); g_video->PutPixel(0,4,99);
    Check(std::all_of(g_video->BackBuffer(),g_video->BackBuffer()+24,[](byte p){return p==7;}),"out of bounds pixels cannot overwrite framebuffer");
    g_actual_palette[7]={17,53,201,0}; g_video->Flip();
    Check(g_video->PresentedFrames()==1,"successful presentation increments counter");
    std::string path=std::string(directory)+"/unit-palette.bmp";
    g_video->Screenshot(path.c_str());
    SDL_Surface* screenshot=SDL_LoadBMP(path.c_str()); Check(screenshot!=nullptr,"presented frame can be reloaded");
    Uint8 r=0,g=0,b=0,a=0; Check(SDL_ReadSurfacePixel(screenshot,0,0,&r,&g,&b,&a),"read screenshot pixel");
    Check(r==17 && g==53 && b==201,"indexed palette expands to correct RGB channels");
    SDL_DestroySurface(screenshot); std::remove(path.c_str()); VideoShutdown();
}
static void TimerTests() {
    Timer timer; Check(timer.GetDelta()==0 && timer.GetFPS()==0,"initial timer avoids division by zero");
    float before=timer.GetTime(); SDL_Delay(80); timer.Time();
    Check(timer.GetTime()>before,"monotonic clock advances");
    Check(timer.GetDelta()>0 && timer.GetDelta()<=0.05f,"long stalls clamp movement delta");
    Check(std::isfinite(timer.GetFPS()),"FPS remains finite");
    timer.Reset(); Check(timer.GetDelta()==0,"reset discards paused duration");
    timer.Time(); Check(timer.GetDelta()<0.05f,"resumed frame does not inherit long stall");
}
static void CFSTests(const char* pack) {
    Check(CFS_Init(pack),"CFS initializes");
    CFS_File file(const_cast<char*>("gfx/palette.tga")); Check(file.Open(),"known packed asset opens");
    std::vector<byte> expected(file.Size()); Check(file.Read(expected.data(),1,file.Size()),"complete read succeeds");
    byte value=0;
    Check(!file.Read(&value,1,1),"EOF read is rejected");
    Check(!file.Read(&value,-1,1) && !file.Read(&value,1,-1),"negative read dimensions rejected");
    file.Seek(0,SEEK_SET); Check(file.Read(&value,0,1),"zero size read succeeds without advancing");
    Check(file.Read(&value,1,1) && value==expected[0],"SEEK_SET addresses first byte");
    file.Seek(2,SEEK_CUR); Check(file.Read(&value,1,1) && value==expected[3],"SEEK_CUR advances from current position");
    file.Seek(-1,SEEK_END); Check(file.Read(&value,1,1) && value==expected.back(),"SEEK_END addresses final byte");
    Throws([&]{file.Seek(-1,SEEK_SET);});
    Throws([&]{file.Seek(file.Size()+1,SEEK_SET);});
    Throws([&]{file.Seek(0,999);});
    Check(file.Open() && file.Read(&value,1,1) && value==expected[0],"reopening resets read cursor");
    CFS_Shutdown(); Check(CFS_Init(pack),"shutdown allows pack reinitialization"); CFS_Shutdown();
}
static void AudioTests(const char* pack) {
    Check(CFS_Init(pack),"sound assets initialize"); SoundInit();
    SDL_PauseAudioDevice(g_sound->Device());
    {
        std::vector<std::unique_ptr<SoundBuffer>> sounds;
        for(int i=0;i<MAX_CHANNELS+2;++i) {
            sounds.emplace_back(new SoundBuffer);
            Check(sounds.back()->LoadWAV(const_cast<char*>("sounds/frog/frog.wav")),"WAV loads from compressed pack");
        }
        for(int i=0;i<MAX_CHANNELS;++i) Check(sounds[i]->Play(true,LOW),"free audio channel accepts looping sound");
        Check(!sounds[0]->Play(true,HIGH),"already playing buffer cannot overlap itself");
        Check(!sounds[MAX_CHANNELS]->Play(true,LOW),"equal priority sound cannot steal a full channel");
        Check(sounds[MAX_CHANNELS]->Play(true,HIGH),"high priority sound preempts lower priority");
        Check(!sounds[0]->IsPlaying() && sounds[MAX_CHANNELS]->IsPlaying(),"preempted sound stops");
        SoundUpdate();
        for(auto& sound:sounds) sound->Stop();
        Check(std::none_of(sounds.begin(),sounds.end(),[](const std::unique_ptr<SoundBuffer>& p){return p->IsPlaying();}),"stop clears all queued and looping sound");
        Check(sounds[0]->Play(false,NORMAL),"stopped buffer can play again as one-shot");
        SDL_ResumeAudioDevice(g_sound->Device());
        Uint64 deadline=SDL_GetTicks()+3000;
        while(sounds[0]->IsPlaying() && SDL_GetTicks()<deadline) SDL_Delay(10);
        Check(!sounds[0]->IsPlaying(),"one-shot audio drains without looping");
    }
    SoundUpdate(); // Destroyed buffers must no longer appear in the update list.
    SoundShutdown(); CFS_Shutdown();
}
int main(int argc,char** argv) {
    if(argc<4) return 2;
    if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)) { fprintf(stderr,"%s\n",SDL_GetError()); return 1; }
    try {
        std::string name=argv[1];
        if(name=="input") InputTests();
        else if(name=="surfaces") SurfaceTests(argv[3]);
        else if(name=="timer") TimerTests();
        else if(name=="cfs") CFSTests(argv[2]);
        else if(name=="audio") AudioTests(argv[2]);
        else throw std::runtime_error("unknown unit suite");
        printf("PASS: %s (%d checks)\n",name.c_str(),checks); SDL_Quit(); return 0;
    } catch(const std::exception& e) { fprintf(stderr,"FAIL: %s\n",e.what()); SDL_Quit(); return 1; }
}
