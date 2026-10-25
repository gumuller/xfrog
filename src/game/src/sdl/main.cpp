// SPDX-License-Identifier: GPL-2.0-or-later
#include "platform.h"
#include "../engine.h"
#include "video.h"
#include "input.h"
#include "timer.h"
#include "sound.h"
#include <cstdio>
#include <cstring>
#include <stdexcept>
#include <string>
int main(int argc,char** argv) {
    bool smoke=false;
    for(int i=1;i<argc;++i) {
        if(!strcmp(argv[i],"--smoke-test")) smoke=true;
        else if(!strcmp(argv[i],"--help")) {
            puts("Xfrog: arrows to move, Space to deliver, Escape for menu.\nF11 or Alt+Enter toggles fullscreen.\nOptions: --smoke-test (120 menu frames then exit)"); return 0;
        } else { fprintf(stderr,"Unknown option: %s\n",argv[i]); return 1; }
    }
    if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)) { fprintf(stderr,"SDL: %s\n",SDL_GetError()); return 1; }
    int result=0;
    try {
        if(!EngineInit()) throw std::runtime_error("Engine initialization failed");
        Uint64 next=SDL_GetTicksNS();
        while(!g_quit_requested) {
            SDL_Event event;
            while(SDL_PollEvent(&event)) {
                HandlePlatformEvent(event);
            }
            if(g_quit_requested) break;
            if(!g_app_focus && !smoke) { SDL_Delay(20); continue; }
            EngineFrame();
            if(smoke && g_video->PresentedFrames()==120) {
                const char* capture=SDL_getenv("XFROG_SCREENSHOT");
                if(capture) g_video->Screenshot(capture);
                puts("PASS: presented 120 menu frames"); break;
            }
            // Bound frame rate even when VSync is unavailable or the monitor is faster.
            if(!g_app_focus) next=SDL_GetTicksNS();
            next+=1000000000/60;
            Uint64 now=SDL_GetTicksNS();
            if(next>now) SDL_DelayNS(next-now);
            else next=now;
        }
        EngineShutdown();
        puts("Engine shut down cleanly");
    } catch(const std::exception& error) { fprintf(stderr,"Xfrog: %s\n",error.what()); result=1; }
    SDL_Quit(); return result;
}
