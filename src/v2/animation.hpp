// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <algorithm>
#include <cmath>
namespace xfrog {
constexpr int RunFrames = 8;
constexpr float CharacterHeight = 88.f;
constexpr int CourierKeys = 6, CourierSamples = 96;
inline int courierKeyCount(int row) { return row == 1 ? 6 : 2; }
inline int courierKey(int row, int key) {
  // Contact -> down -> passing on each foot. Redundant extended poses at the
  // old last/first boundary are excluded, rather than held for two frames.
  constexpr int order[3][CourierKeys] = {
      {0, 1, 0, 1, 0, 1}, {0, 2, 1, 3, 6, 4}, {0, 1, 0, 1, 0, 1}};
  return row * RunFrames + order[row][key % CourierKeys];
}
inline int courierSample(float cycle) {
  if (!std::isfinite(cycle) || cycle < 0)
    return 0;
  return int(std::fmod(cycle, 1.f) * CourierSamples + .0001f) % CourierSamples;
}
inline int runFrame(float cycle) {
  if (!std::isfinite(cycle) || cycle < 0)
    return 0;
  return int(std::fmod(cycle, 1.f) * RunFrames) % RunFrames;
}
inline float courierCycleBetween(float previous, float current, float blend) {
  if (current < previous)
    current += 1;
  float cycle = previous * (1 - blend) + current * blend;
  return cycle - std::floor(cycle);
}
} // namespace xfrog
