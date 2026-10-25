// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <SDL3/SDL.h>
extern bool g_app_focus;
extern bool g_quit_requested;
inline void PostQuitMessage(int) { g_quit_requested = true; }
const char* HighScorePath();
void HandlePlatformEvent(const SDL_Event& event);
