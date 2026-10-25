// SPDX-License-Identifier: GPL-2.0-or-later
#include "audio.hpp"
#include <algorithm>
#include <array>
#include <cmath>
#include <stdexcept>
namespace xfrog {
static constexpr float Pi = 3.14159265358979323846f;
namespace {
std::vector<float> score;
}
Audio::Audio(const std::string &assets) {
  if (!assets.empty() && !loadMusic(assets + "/town-theme.wav"))
    throw std::runtime_error("Unable to load town soundtrack: " +
                             std::string(SDL_GetError()));
  SDL_AudioSpec spec{SDL_AUDIO_F32, 2, 48000};
  stream = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, &spec,
                                     nullptr, nullptr);
  if (stream)
    SDL_ResumeAudioStreamDevice(stream);
  else
    SDL_Log("Audio unavailable: %s", SDL_GetError());
}
Audio::~Audio() { SDL_DestroyAudioStream(stream); }
namespace {
constexpr double Rate = 48000;
float frequency(int midi) {
  static const auto table = [] {
    std::array<float, 128> values{};
    for (int i = 0; i < 128; ++i)
      values[i] = 440.f * std::pow(2.f, (i - 69) / 12.f);
    return values;
  }();
  return table[std::clamp(midi, 0, 127)];
}
float blep(double phase, double increment) {
  if (phase < increment) {
    float x = float(phase / increment);
    return 2 * x - x * x - 1;
  }
  if (phase > 1 - increment) {
    float x = float((phase - 1) / increment);
    return x * x + 2 * x + 1;
  }
  return 0;
}
float pulse(double time, float hz, double duty = .25) {
  double phase = std::fmod(time * hz, 1.0), increment = hz / Rate;
  float value = (phase < duty ? 1.f : -1.f) + blep(phase, increment) -
                blep(std::fmod(phase - duty + 1, 1.0), increment);
  // Remove the pulse wave's duty-dependent DC offset before quantization.
  return std::round((value - float(2 * duty - 1)) * 127) / 127;
}
float triangle(double time, float hz) {
  return float(1 - 4 * std::abs(std::fmod(time * hz, 1.0) - .5));
}
float envelope(double age, double duration) {
  return float(std::clamp(age / .003, 0.0, 1.0) *
               std::clamp((duration - age) / .025, 0.0, 1.0));
}
float noise(uint64_t sample) {
  uint32_t n = uint32_t(sample / 6) * 747796405u + 2891336453u;
  n = ((n >> ((n >> 28) + 4)) ^ n) * 277803737u;
  return float((n >> 22) ^ n) / 2147483648.f - 1;
}
} // namespace
bool Audio::loadMusic(const std::string &path) {
  SDL_AudioSpec input{}, output{SDL_AUDIO_F32, 2, 48000};
  Uint8 *bytes = nullptr, *converted = nullptr;
  Uint32 length = 0;
  int convertedLength = 0;
  if (!SDL_LoadWAV(path.c_str(), &input, &bytes, &length))
    return false;
  bool ok = SDL_ConvertAudioSamples(&input, bytes, int(length), &output,
                                    &converted, &convertedLength);
  SDL_free(bytes);
  if (!ok)
    return false;
  const float *data = reinterpret_cast<const float *>(converted);
  score.assign(data, data + convertedLength / sizeof(float));
  SDL_free(converted);
  return !score.empty();
}
uint64_t Audio::musicFrames() { return score.size() / 2; }
float Audio::musicSample(uint64_t sample, int channel) {
  if (score.empty())
    return 0;
  return score[(sample % musicFrames()) * 2 + (channel & 1)];
}
int Audio::soundSamples(Cue cue) {
  switch (cue) {
  case Cue::Win:
    return 48000;
  case Cue::Deliver:
    return 33600;
  case Cue::Hit:
    return 16800;
  case Cue::Expired:
    return 21600;
  case Cue::Pickup:
    return 11520;
  case Cue::Dash:
    return 8640;
  case Cue::NearMiss:
    return 7200;
  case Cue::Click:
    return 2400;
  case Cue::Steal:
    return 19200;
  }
  return 0;
}
float Audio::soundSample(Cue cue, uint64_t sample) {
  int duration = soundSamples(cue);
  if (sample >= uint64_t(duration))
    return 0;
  double t = double(sample) / Rate, end = duration / Rate;
  float wave = 0, volume = .095f;
  switch (cue) {
  case Cue::Pickup: {
    const int notes[] = {72, 76, 79};
    wave = pulse(std::fmod(t, .08), frequency(notes[std::min(2, int(t / .08))]),
                 .5);
    break;
  }
  case Cue::Deliver: {
    // Cash drawer clack, followed by the bright two-part cashier "ka-ching".
    wave = noise(sample) * std::exp(float(-t * 65)) * .55f;
    for (double onset : {.065, .16}) {
      double u = t - onset;
      if (u >= 0)
        wave += float((std::sin(2 * Pi * 2100 * u) +
                       .35 * std::sin(2 * Pi * 4217 * u) +
                       .18 * std::sin(2 * Pi * 6350 * u)) *
                      std::exp(-u * 8)) *
                .65f;
    }
    volume = .11f;
    break;
  }
  case Cue::Steal:
    wave = triangle(700 * t - 480 * t * t, 1) *
           (.75f + .25f * std::sin(float(t) * 70));
    break;
  case Cue::Win: {
    const int notes[] = {72, 76, 79, 84, 79, 84, 88, 91};
    int n = std::min(7, int(t * 8));
    wave = pulse(std::fmod(t, .125), frequency(notes[n]), .5) *
           envelope(std::fmod(t, .125), .125);
    break;
  }
  case Cue::Hit:
    wave = triangle(0.0 + (260 * t - 220 * t * t), 1) * .8f +
           noise(sample) * std::exp(float(-t * 35)) * .25f;
    break;
  case Cue::Dash:
    wave = pulse(360 * t + 3200 * t * t, 1, .5) * .55f + noise(sample) * .3f;
    volume = .055f;
    break;
  case Cue::NearMiss:
    wave = pulse(t, frequency(t < .055 ? 88 : 96), .5);
    volume = .065f;
    break;
  case Cue::Expired: {
    const int notes[] = {67, 64, 60};
    wave = pulse(std::fmod(t, .15), frequency(notes[std::min(2, int(t / .15))]),
                 .5);
    break;
  }
  case Cue::Click:
    wave = pulse(t, frequency(79), .5);
    volume = .045f;
    break;
  }
  return wave * envelope(t, end) * float(std::exp(-t / end * 1.7)) * volume;
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
      voices.push_back({cue, 0, soundSamples(cue)});
  }
  int queued = SDL_GetAudioStreamQueued(stream);
  if (queued < 0 || queued > 48000)
    return;
  int frames = std::clamp(2048 - queued / int(sizeof(float) * 2), 0, 2048);
  if (frames == 0)
    return;
  std::vector<float> output(frames * 2);
  for (int i = 0; i < frames; ++i) {
    float left = musicSample(cursor, 0), right = musicSample(cursor++, 1);
    float value = 0;
    for (auto &voice : voices) {
      value += soundSample(voice.cue, voice.age);
      ++voice.age;
    }
    voices.erase(
        std::remove_if(voices.begin(), voices.end(),
                       [](const Voice &v) { return v.age >= v.duration; }),
        voices.end());
    value = std::clamp(value, -.8f, .8f);
    output[i * 2] = std::clamp(left + value, -.8f, .8f);
    output[i * 2 + 1] = std::clamp(right + value, -.8f, .8f);
  }
  SDL_PutAudioStreamData(stream, output.data(),
                         int(output.size() * sizeof(float)));
}
} // namespace xfrog
