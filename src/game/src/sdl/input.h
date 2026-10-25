// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <SDL3/SDL.h>
#include "../types.h"
const int NUM_OF_KEYS = SDL_SCANCODE_COUNT;
const float KEY_RATE = 0.333f;
enum key_state_t { NONE, UP, DOWN, TICK };
enum { KEY_UP=SDL_SCANCODE_UP, KEY_DOWN=SDL_SCANCODE_DOWN,
       KEY_LEFT=SDL_SCANCODE_LEFT, KEY_RIGHT=SDL_SCANCODE_RIGHT,
       KEY_RETURN=SDL_SCANCODE_RETURN, KEY_SPACE=SDL_SCANCODE_SPACE,
       KEY_ESCAPE=SDL_SCANCODE_ESCAPE, KEY_Y=SDL_SCANCODE_Y, KEY_N=SDL_SCANCODE_N };
struct input_key_t { int32 code; key_state_t state; float time; bool tick; };
class Input {
public:
    void FlushBuffer() {}
    void CheckKeyEvents() {}
    void FocusManager();
    void HandleEvent(const SDL_Event& event);
};
extern Input* g_input;
bool InputInit();
void InputShutdown();
void InputUpdate();
bool InputCheckKey(int32 code,key_state_t state);
