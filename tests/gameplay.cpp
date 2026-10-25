// SPDX-License-Identifier: GPL-2.0-or-later
#include "sdl/platform.h"
#include "engine.h"
#include "game.h"
#include "player.h"
#include "input.h"
#include "timer.h"
#include "highscore.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>
static void Require(bool ok,const char* message) {
    if(!ok) throw std::runtime_error(message);
    printf("PASS: %s\n",message);
}
static void Pump() { SDL_Event event; while(SDL_PollEvent(&event)) HandlePlatformEvent(event); }
static void Key(SDL_Scancode code,bool down) {
    SDL_Event event={}; event.type=down ? SDL_EVENT_KEY_DOWN : SDL_EVENT_KEY_UP;
    event.key.windowID=SDL_GetWindowID(g_video->Window());
    event.key.scancode=code; event.key.down=down;
    if(!SDL_PushEvent(&event)) throw std::runtime_error(SDL_GetError());
    Pump();
}
static void Frames(int count) { for(int i=0;i<count;++i) { Pump(); EngineFrame(); } }
static void Tap(SDL_Scancode code) { Key(code,true); Frames(1); Key(code,false); Frames(1); }
static void Capture(const char* name) {
    const char* directory=SDL_getenv("XFROG_TEST_CAPTURE_DIR");
    if(directory) g_video->Screenshot((std::string(directory)+"/"+name+".bmp").c_str());
}
int main() {
    if(!SDL_Init(SDL_INIT_VIDEO|SDL_INIT_AUDIO)) { fprintf(stderr,"%s\n",SDL_GetError()); return 1; }
    try {
        std::remove(HighScorePath()); // The test build uses its own Xfrog Tests directory.
        Require(EngineInit(),"engine initialization");
        std::srand(1);
        SDL_SyncWindow(g_video->Window()); SDL_Delay(400); Pump();
        Frames(5); SDL_Delay(400); Pump(); Capture("menu");
        Require(g_engine_state==MENU,"menu renders");
        if(strcmp(SDL_GetCurrentVideoDriver(),"dummy")) {
            Tap(SDL_SCANCODE_F11); SDL_SyncWindow(g_video->Window()); Pump();
            Require(SDL_GetWindowFlags(g_video->Window())&SDL_WINDOW_FULLSCREEN,"F11 enters fullscreen");
            Tap(SDL_SCANCODE_F11); SDL_SyncWindow(g_video->Window()); Pump();
            Require(!(SDL_GetWindowFlags(g_video->Window())&SDL_WINDOW_FULLSCREEN),"F11 restores windowed mode");
            Frames(5); SDL_Delay(300); Pump();
        }
        Tap(SDL_SCANCODE_RETURN);
        Require(g_engine_state==GAME && g_game_state==STARTING,"New Game opens instructions");
        Capture("instructions");
        Tap(SDL_SCANCODE_SPACE);
        Require(g_game_state==PLAYING && g_is_playing,"Space starts gameplay");
        float x=g_player->X();
        Key(SDL_SCANCODE_RIGHT,true); Frames(20); Key(SDL_SCANCODE_RIGHT,false); Frames(1);
        Require(g_player->X()>x+20,"arrow key moves player");
        Key(SDL_SCANCODE_LEFT,true); Frames(20); Key(SDL_SCANCODE_LEFT,false); Frames(1);
        Key(SDL_SCANCODE_DOWN,true); Frames(30); Key(SDL_SCANCODE_DOWN,false); Frames(1);
        Require(g_player->HasPizza(),"pizzeria grants pizza through movement and collision");
        // Place a delivery-zone fixture over the current player. This exercises
        // overlap, Space input, score, sound and pizza removal without teleport APIs.
        pizza_spot_t delivery={int(g_player->X()),int(g_player->Y()),40,35};
        g_player->DeliverPizzaTo(&delivery);
        int score=g_player->Score();
        Tap(SDL_SCANCODE_SPACE);
        Require(g_player->Score()==score+15 && !g_player->HasPizza(),"delivery awards 15 points and consumes pizza");
        Frames(1); Capture("gameplay");
        // Holding Up crosses traffic. Test the real collision/death/respawn path.
        int lives=g_player->Lives();
        Key(SDL_SCANCODE_UP,true);
        for(int i=0;i<900 && g_player->Lives()==lives;++i) Frames(1);
        Key(SDL_SCANCODE_UP,false);
        Require(g_player->Lives()<lives,"traffic collision plays death animation and costs a life");
        Frames(1); Capture("respawn");
        Key(SDL_SCANCODE_RIGHT,true);
        SDL_Event focus={}; focus.type=SDL_EVENT_WINDOW_FOCUS_LOST;
        SDL_PushEvent(&focus); Pump();
        Require(!g_app_focus && InputCheckKey(KEY_RIGHT,UP),"focus loss clears held keys");
        Require(SDL_AudioDevicePaused(g_sound->Device()),"focus loss pauses audio");
        focus.type=SDL_EVENT_WINDOW_FOCUS_GAINED; SDL_PushEvent(&focus); Pump();
        Require(g_app_focus && !SDL_AudioDevicePaused(g_sound->Device()),"focus return resumes audio");
        Tap(SDL_SCANCODE_ESCAPE);
        Require(g_engine_state==MENU,"Escape returns to menu");
        Tap(SDL_SCANCODE_DOWN); Tap(SDL_SCANCODE_RETURN);
        Require(g_engine_state==GAME && g_game_state==PLAYING,"Return resumes game");
        Tap(SDL_SCANCODE_ESCAPE);
        Tap(SDL_SCANCODE_DOWN); Tap(SDL_SCANCODE_RETURN);
        Require(g_engine_state==EXIT,"Exit opens confirmation");
        Tap(SDL_SCANCODE_N);
        Require(g_engine_state==MENU,"N cancels exit");
        Tap(SDL_SCANCODE_RETURN); Tap(SDL_SCANCODE_Y);
        Require(g_quit_requested,"Y requests shutdown");
        uint32 presented=g_video->PresentedFrames();
        EngineShutdown();
        // Reopen the saved file through the game's reader; don't merely inspect memory.
        Require(HighScoreInit(),"saved score reloads after shutdown");
        HighScoreShutdown();
        FILE* fp=fopen(HighScorePath(),"rb"); int32 saved=0;
        Require(fp && fread(&saved,sizeof(saved),1,fp)==1,"score file contains four bytes");
        fclose(fp);
        Require((saved^2125)>=15,"earned high score persists");
        printf("PASS: %u frames presented; clean gameplay shutdown\n",presented);
        SDL_Quit(); return 0;
    } catch(const std::exception& e) { fprintf(stderr,"FAIL: %s\n",e.what()); SDL_Quit(); return 1; }
}
