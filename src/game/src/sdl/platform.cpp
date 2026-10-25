// SPDX-License-Identifier: GPL-2.0-or-later
#include "platform.h"
#include "input.h"
#include "timer.h"
#include "sound.h"
#include "video.h"
#include <string>
#include <stdexcept>
bool g_app_focus=true;
bool g_quit_requested=false;
const char* HighScorePath() {
    static std::string path=[] {
#ifdef XFROG_TESTING
        char* pref=SDL_GetPrefPath("Gustavo Muller","Xfrog Tests");
#else
        char* pref=SDL_GetPrefPath("Gustavo Muller","Xfrog");
#endif
        if(!pref) throw std::runtime_error(SDL_GetError());
        std::string result=std::string(pref)+"hs.dat";
        SDL_free(pref); return result;
    }();
    return path.c_str();
}
void HandlePlatformEvent(const SDL_Event& event) {
    if(event.type==SDL_EVENT_QUIT || event.type==SDL_EVENT_WINDOW_CLOSE_REQUESTED) PostQuitMessage(0);
    if(event.type==SDL_EVENT_WINDOW_FOCUS_LOST || event.type==SDL_EVENT_WINDOW_FOCUS_GAINED) {
        g_app_focus=event.type==SDL_EVENT_WINDOW_FOCUS_GAINED;
        g_input->FocusManager(); g_timer->Reset();
        if(g_app_focus) SDL_ResumeAudioDevice(g_sound->Device());
        else SDL_PauseAudioDevice(g_sound->Device());
    }
    if(event.type==SDL_EVENT_KEY_DOWN && !event.key.repeat &&
       (event.key.scancode==SDL_SCANCODE_F11 ||
        (event.key.scancode==SDL_SCANCODE_RETURN && (event.key.mod&SDL_KMOD_ALT)))) {
        bool fullscreen=(SDL_GetWindowFlags(g_video->Window())&SDL_WINDOW_FULLSCREEN)!=0;
        if(!SDL_SetWindowFullscreen(g_video->Window(),!fullscreen)) SDL_Log("Fullscreen: %s",SDL_GetError());
    } else g_input->HandleEvent(event);
}
