// SPDX-License-Identifier: GPL-2.0-or-later
#include "audio.hpp"
#include <array>
#include <cmath>
#include <cstdio>
#include <fstream>
#include <set>
#include <stdexcept>
using namespace xfrog;
static void check(bool ok, const char *message) {
  if (!ok)
    throw std::runtime_error(message);
}
int main(int argc, char **argv) {
  try {
    check(argc > 2 &&
              Audio::loadMusic(std::string(argv[2]) + "/town-theme.wav"),
          "full town score loads");
    check(Audio::musicFrames() > 48000 * 180,
          "score exceeds three minutes before looping");
    const int samples = int(Audio::musicFrames());
    std::ofstream wav;
    auto word = [&](uint32_t n, int bytes) {
      for (int i = 0; i < bytes; ++i)
        wav.put(char(n >> (8 * i)));
    };
    if (argc > 1) {
      wav.open(argv[1], std::ios::binary);
      check(bool(wav), "audio evidence writable");
      wav.write("RIFF", 4);
      word(36 + samples * 2, 4);
      wav.write("WAVEfmt ", 8);
      word(16, 4);
      word(1, 2);
      word(1, 2);
      word(48000, 4);
      word(96000, 4);
      word(2, 2);
      word(16, 2);
      wav.write("data", 4);
      word(samples * 2, 4);
    }
    double sum = 0, energy = 0;
    float peak = 0;
    for (int i = 0; i < samples; ++i) {
      float value = Audio::musicSample(i);
      check(std::isfinite(value), "music is finite");
      sum += value;
      energy += value * value;
      peak = std::max(peak, std::abs(value));
      if (wav)
        word(uint16_t(int16_t(value * 32767)), 2);
    }
    double rms = std::sqrt(energy / samples);
    check(rms > .015 && rms < .12 && peak < .3f,
          "music audible without clipping");
    check(std::abs(sum / samples) < .003, "music has no significant DC bias");
    double contrast = 0, stereo = 0;
    for (int i = 0; i < 48000; ++i) {
      contrast += std::abs(Audio::musicSample(i + 48000 * 23) -
                           Audio::musicSample(i + 48000 * 70));
      stereo += std::abs(Audio::musicSample(i + 48000 * 30, 0) -
                         Audio::musicSample(i + 48000 * 30, 1));
    }
    check(contrast > 500 && stereo > 100,
          "sections contrast and mix has stereo width");
    check(std::abs(Audio::musicSample(0) - Audio::musicSample(samples - 1)) <
              .002,
          "music loop boundary has no click");
    if (wav) {
      wav.close();
      wav.open(std::string(argv[1]) + ".cashier.wav", std::ios::binary);
      int frames = Audio::soundSamples(Cue::Deliver);
      wav.write("RIFF", 4);
      word(36 + frames * 2, 4);
      wav.write("WAVEfmt ", 8);
      word(16, 4);
      word(1, 2);
      word(1, 2);
      word(48000, 4);
      word(96000, 4);
      word(2, 2);
      word(16, 2);
      wav.write("data", 4);
      word(frames * 2, 4);
      for (int i = 0; i < frames; ++i)
        word(uint16_t(int16_t(Audio::soundSample(Cue::Deliver, i) * 32767)), 2);
      wav.close();
    }
    {
      wav.open(std::string(argv[1]) + ".horn.wav", std::ios::binary);
      int frames = Audio::soundSamples(Cue::Horn);
      wav.write("RIFF", 4);
      word(36 + frames * 2, 4);
      wav.write("WAVEfmt ", 8);
      word(16, 4);
      word(1, 2);
      word(1, 2);
      word(48000, 4);
      word(96000, 4);
      word(2, 2);
      word(16, 2);
      wav.write("data", 4);
      word(frames * 2, 4);
      for (int i = 0; i < frames; ++i)
        word(uint16_t(int16_t(Audio::soundSample(Cue::Horn, i) * 32767)), 2);
      wav.close();
      check(Audio::soundSample(Cue::Horn, 0) == 0 &&
                std::abs(Audio::soundSample(Cue::Horn, frames - 1)) < .001f,
            "horn soft attack and release avoid clicks");
    }
    std::set<uint64_t> signatures;
    for (Cue cue :
         {Cue::Pickup, Cue::Deliver, Cue::Hit, Cue::Dash, Cue::NearMiss,
          Cue::Expired, Cue::Win, Cue::Click, Cue::Steal, Cue::Horn}) {
      uint64_t signature = 1469598103934665603ull;
      double energy = 0;
      int count = Audio::soundSamples(cue);
      for (int i = 0; i < count; ++i) {
        float value = Audio::soundSample(cue, i);
        check(std::isfinite(value) && std::abs(value) < .2f,
              "effect is finite and bounded");
        energy += value * value;
        signature =
            (signature ^ uint16_t(int16_t(value * 32767))) * 1099511628211ull;
      }
      check(energy > .1 && Audio::soundSample(cue, count) == 0,
            "effect audible and terminates");
      signatures.insert(signature);
    }
    check(signatures.size() == 10, "all ten effects have distinct waveforms");
    printf("PASS: original 80-bar stereo score RMS %.4f peak %.4f, zero DC and "
           "ten distinct SFX\n",
           rms, peak);
  } catch (const std::exception &e) {
    fprintf(stderr, "FAIL: %s\n", e.what());
    return 1;
  }
}
