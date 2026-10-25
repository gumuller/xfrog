// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include "animation.hpp"
namespace xfrog {
// Complete transparent sprites baked from the approved gait. Level zero is a
// single neutral frame; the other levels retain every phase while slowing down.
constexpr int CourierActivitySteps = 8, CourierTile = 128, CourierColumns = 16;
constexpr int CourierSpriteCount = 1 + CourierActivitySteps * CourierSamples;
constexpr int CourierRows =
    (CourierSpriteCount + CourierColumns - 1) / CourierColumns;
constexpr float CourierBakeHeight = 96, CourierAnchorX = 64,
                CourierAnchorY = 112;
inline int courierSpriteIndex(float cycle, float activity) {
  int level = int(std::clamp(activity, 0.f, 1.f) * CourierActivitySteps + .5f);
  return level == 0 ? 0
                    : 1 + (level - 1) * CourierSamples + courierSample(cycle);
}
} // namespace xfrog
