// SPDX-License-Identifier: GPL-2.0-or-later
#include "audio.hpp"
#include <algorithm>
#include <cmath>
namespace xfrog {
static constexpr float Pi = 3.14159265358979323846f;
Audio::Audio() {
  SDL_AudioSpec spec{SDL_AUDIO_F32, 2, 48000};
  stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,
                                     nullptr, nullptr);
  if (stream)
    SDL_ResumeAudioStreamDevice(stream);
  else
    SDL_Log("Audio unavailable: %s", SDL_GetError());
}
Audio::~Audio() { SDL_DestroyAudioStream(stream); }
float Audio::musicSample(uint64_t sample) {
  // Original generative score: warm marimba arpeggios over a soft bass line.
  // A deterministic eight-bar progression, never a downloaded audio loop.
  double time = double(sample) / 48000;
  int beat = int(time * 2.5);
  float phase = float(std::fmod(time, .4));
  const int roots[8] = {60, 65, 57, 67, 60, 65, 62, 67};
  int root = roots[(beat / 8) % 8];
  const int notes[8] = {0, 7, 12, 4, 7, 12, 16, 7};
  float frequency = 440 * std::pow(2.f, (root + notes[beat % 8] - 69) / 12.f);
  float note = std::sin(2 * Pi * frequency * phase) * std::exp(-phase * 13) +
               .2f * std::sin(2 * Pi * frequency * 2.01f * phase) *
                   std::exp(-phase * 22);
  float bassFrequency = 440 * std::pow(2.f, (root - 24 - 69) / 12.f);
  float bass = std::sin(float(time) * 2 * Pi * bassFrequency) *
               (.5f + .5f * std::cos(phase * Pi / .4f));
  float shimmer = std::sin(float(time) * 2 * Pi * 220) * .008f;
  return note * .045f + bass * .022f + shimmer;
}
void Audio::update(bool enabled, bool paused, const std::vector<Cue> &cues) {
  if (!stream)
    return;
  if (enabled != wasEnabled || paused) {
    SDL_ClearAudioStream(stream);
    voices.clear();
    wasEnabled = enabled;
  }
  if (!enabled || paused)
    return;
  for (Cue cue : cues) {
    if (voices.size() < 12)
      voices.push_back({cue, 0,
                        cue == Cue::Win       ? 48000
                        : cue == Cue::Deliver ? 22000
                                              : 13000});
  }
  int queued = SDL_GetAudioStreamQueued(stream);
  if (queued < 0 || queued > 48000)
    return;
  int frames = std::clamp(2048 - queued / int(sizeof(float) * 2), 0, 2048);
  if (frames == 0)
    return;
  std::vector<float> output(frames * 2);
  for (int i = 0; i < frames; ++i) {
    float value = musicSample(cursor++);
    for (auto &voice : voices) {
      float t = voice.age / 48000.f, life = float(voice.age) / voice.duration;
      float frequency = 660, volume = .17f;
      switch (voice.cue) {
      case Cue::Pickup:
        frequency = 600 + 500 * t;
        break;
      case Cue::Deliver:
        frequency = 880 * std::pow(1.25f, float(int(t * 12)));
        break;
      case Cue::Win:
        frequency = 523.25f * std::pow(1.2599f, float(int(t * 9) % 5));
        volume = .13f;
        break;
      case Cue::Hit:
        frequency = 160 - 180 * t;
        break;
      case Cue::Dash:
        frequency = 320 + 2000 * t;
        volume = .055f;
        break;
      case Cue::NearMiss:
        frequency = 1100;
        volume = .07f;
        break;
      case Cue::Expired:
        frequency = 500 - 650 * t;
        volume = .1f;
        break;
      case Cue::Click:
        frequency = 780;
        volume = .05f;
        break;
      }
      float envelope =
          std::min(t * 90, 1.f) * std::pow(std::max(0.f, 1 - life), 3.f);
      value += std::sin(2 * Pi * frequency * t) * envelope * volume;
      ++voice.age;
    }
    voices.erase(
        std::remove_if(voices.begin(), voices.end(),
                       [](const Voice &v) { return v.age >= v.duration; }),
        voices.end());
    value = std::clamp(value, -.8f, .8f);
    output[i * 2] = value;
    output[i * 2 + 1] = value;
  }
  SDL_PutAudioStreamData(stream, output.data(),
                         int(output.size() * sizeof(float)));
}
} // namespace xfrog
