// SPDX-License-Identifier: GPL-2.0-or-later
#include "timer.h"
#include <cstdlib>
#include <ctime>
Timer* g_timer=nullptr;
bool TimerInit() { g_timer=new Timer; std::srand(unsigned(std::time(nullptr))); return true; }
void TimerShutdown() { delete g_timer; g_timer=nullptr; }
