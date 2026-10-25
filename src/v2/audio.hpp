// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "game.hpp"
#include <SDL3/SDL.h>
#include <vector>
namespace xfrog {
class Audio {
public:
  explicit Audio(const std::string &assets = {});
  ~Audio();
  void update(bool enabled, bool paused, const std::vector<Cue> &cues);
  static bool loadMusic(const std::string &path);
  static uint64_t musicFrames();
  static float musicSample(uint64_t sample, int channel = 0);
  static float soundSample(Cue cue, uint64_t sample);
  static int soundSamples(Cue cue);

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
