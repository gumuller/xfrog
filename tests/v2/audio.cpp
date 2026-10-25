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
    constexpr int samples = 48000 * 16;
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
    check(rms > .025 && rms < .12 && peak < .3f,
          "music audible without clipping");
    check(std::abs(sum / samples) < .003, "music has no significant DC bias");
    std::set<uint64_t> signatures;
    for (Cue cue : {Cue::Pickup, Cue::Deliver, Cue::Hit, Cue::Dash,
                    Cue::NearMiss, Cue::Expired, Cue::Win, Cue::Click}) {
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
    check(signatures.size() == 8, "all eight effects have distinct waveforms");
    printf("PASS: original 132 BPM chiptune RMS %.4f peak %.4f, zero DC and "
           "eight distinct SFX\n",
           rms, peak);
  } catch (const std::exception &e) {
    fprintf(stderr, "FAIL: %s\n", e.what());
    return 1;
  }
}
