// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "game.hpp"
#include <SDL3/SDL.h>
#include <vector>
namespace xfrog {
class Audio {
public:
  Audio();
  ~Audio();
  void update(bool enabled, bool paused, const std::vector<Cue> &cues);
  static float musicSample(uint64_t sample);

private:
  struct Voice {
    Cue cue;
    int age = 0, duration = 0;
  };
  SDL_AudioStream *stream = nullptr;
  std::vector<Voice> voices;
  uint64_t cursor = 0;
  bool wasEnabled = true;
};
} // namespace xfrog
