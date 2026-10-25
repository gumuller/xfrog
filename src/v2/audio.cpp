// SPDX-License-Identifier: GPL-2.0-or-later
#include "audio.hpp"
#include <algorithm>
#include <array>
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
namespace {
constexpr double Rate = 48000, Eighth = 60.0 / 132 / 2;
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
float Audio::musicSample(uint64_t sample) {
  // Original 132 BPM, eight-bar chiptune: pulse lead, arpeggio, triangle
  // bass and clocked-noise drums. No external recordings or licensed tunes.
  static constexpr int melody[64] = {
      72, 76, 79, -1, 76, 74, 72, 0,  77, 81, 84, -1, 81, 79, 77, 0,
      81, 76, 81, 83, 84, 83, 81, 0,  79, 83, 86, -1, 84, 83, 79, 0,
      84, 83, 81, 79, 76, 79, 84, -1, 74, 77, 81, -1, 77, 76, 74, 0,
      77, 81, 84, 81, 79, 81, 77, 74, 79, 83, 86, 83, 79, 76, 72, -1};
  static constexpr int roots[8] = {60, 65, 57, 67, 60, 62, 65, 67};
  double time = std::fmod(double(sample) / Rate, Eighth * 64);
  int step = std::min(63, int(time / Eighth)), bar = step / 8;
  double age = time - step * Eighth;
  int noteStep = step;
  if (melody[step] == -1)
    --noteStep;
  int note = melody[noteStep];
  double noteAge = time - noteStep * Eighth;
  double duration =
      Eighth * ((noteStep < 63 && melody[noteStep + 1] == -1) ? 2 : 1);
  float lead = note > 0 ? pulse(noteAge, frequency(note)) *
                              envelope(noteAge, duration) * .052f
                        : 0;
  // Alternating root/fifth bass gives the delivery loop a buoyant walking
  // pulse.
  int bassNote = roots[bar] - 24 + ((step % 4 == 2) ? 7 : 0);
  float bass =
      triangle(age, frequency(bassNote)) * envelope(age, Eighth) * .052f;
  int sixteenth = int(time / (Eighth / 2));
  static constexpr int arps[4] = {0, 7, 12, 7};
  int third = (bar == 2 || bar == 5) ? 3 : 4;
  int offset = sixteenth % 4 == 1 ? third : arps[sixteenth % 4];
  double arpAge = std::fmod(time, Eighth / 2);
  float arp = pulse(arpAge, frequency(roots[bar] + 12 + offset), .125) *
              envelope(arpAge, Eighth / 2) * .013f;
  float hat = noise(sample) * std::exp(float(-arpAge * 110)) * .013f;
  double beatAge = std::fmod(time, Eighth * 2);
  float drum;
  if ((step / 2) % 2 == 0) {
    // Integrate the exponential pitch sweep, avoiding phase discontinuities.
    double phase = 48 * beatAge + 95 * (1 - std::exp(-beatAge * 32)) / 32;
    drum = std::sin(float(phase * 2 * Pi)) * std::exp(float(-beatAge * 23)) *
           .046f;
  } else {
    drum = (noise(sample) * .032f + triangle(beatAge, 170) * .014f) *
           std::exp(float(-beatAge * 28));
  }
  return lead + bass + arp + hat + drum;
}
int Audio::soundSamples(Cue cue) {
  switch (cue) {
  case Cue::Win:
    return 48000;
  case Cue::Deliver:
    return 19200;
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
    const int notes[] = {84, 88, 91, 96};
    wave =
        pulse(std::fmod(t, .1), frequency(notes[std::min(3, int(t / .1))]), .5);
    break;
  }
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
    float value = musicSample(cursor++);
    for (auto &voice : voices) {
      value += soundSample(voice.cue, voice.age);
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
