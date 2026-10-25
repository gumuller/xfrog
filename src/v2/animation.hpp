// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <algorithm>
#include <cmath>
namespace xfrog {
constexpr int RunFrames = 8;
inline int runFrame(float cycle) {
  if (!std::isfinite(cycle) || cycle < 0)
    return 0;
  return int(std::fmod(cycle, 1.f) * RunFrames) % RunFrames;
}
struct RunSpritePose {
  int index;
  bool flip;
};
inline RunSpritePose courierRunPose(int facing, float cycle) {
  facing = std::clamp(facing, 0, 3);
  int phase = runFrame(cycle);
  // The side sheet's opposite wide contact and recoil were authored in
  // reversed slots. Put contact before recoil, then passing and knee drive.
  constexpr int sideOrder[8] = {0, 1, 2, 3, 5, 4, 6, 7};
  bool side = facing == 1 || facing == 3;
  int row = facing == 3 ? 1 : facing;
  return {row * RunFrames + (side ? sideOrder[phase] : phase), facing == 3};
}
} // namespace xfrog
