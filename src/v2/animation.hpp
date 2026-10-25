// SPDX-License-Identifier: GPL-2.0-or-later
#pragma once
#include <algorithm>
#include <cmath>
namespace xfrog {
constexpr int RunFrames = 8, CourierSamples = 96;
constexpr float CharacterHeight = 88.f, CourierStrideLength = 1.4f;
constexpr float AnimationPi = 3.14159265358979323846f;
inline float cyclePhase(float cycle) {
  return std::isfinite(cycle) ? cycle - std::floor(cycle) : 0;
}
inline int courierSample(float cycle) {
  return int(cyclePhase(cycle) * CourierSamples + .0001f) % CourierSamples;
}
inline int runFrame(float cycle) {
  return int(cyclePhase(cycle) * RunFrames) % RunFrames;
}
inline float courierCycleBetween(float previous, float current, float blend) {
  if (current < previous)
    current += 1;
  return cyclePhase(previous * (1 - blend) + current * blend);
}
struct RigPoint {
  float x = 0, y = 0;
};
struct LegPose {
  RigPoint hip, knee, ankle;
  float footAngle = 0;
  bool planted = false;
};
struct CourierPose {
  LegPose legs[2];
  RigPoint shoulders[2], elbows[2], hands[2];
  float hipY = 0, lean = 0;
};
inline float hermite(float a, float b, float da, float db, float t,
                     float span) {
  return (2 * t * t * t - 3 * t * t + 1) * a +
         (t * t * t - 2 * t * t + t) * span * da +
         (-2 * t * t * t + 3 * t * t) * b + (t * t * t - t * t) * span * db;
}
inline LegPose runningLeg(float phase, float hipY) {
  phase = cyclePhase(phase);
  struct Key {
    float phase, x, y, dx, dy, angle;
  };
  // Contact, planted push-off, heel recovery, passing, landing, contact.
  // The endpoint has the same position AND velocity as the first key.
  constexpr Key keys[] = {
      {0, .21f, -.075f, -1.4f, 0, 0},     {.30f, -.21f, -.075f, -1.4f, 0, 0},
      {.45f, -.30f, -.20f, 0, -.8f, 35},  {.62f, -.16f, -.27f, 1.4f, 0, 45},
      {.80f, .17f, -.21f, 1.f, .8f, -12}, {1, .21f, -.075f, -1.4f, 0, 0}};
  int key = 0;
  while (key < 4 && phase >= keys[key + 1].phase)
    ++key;
  const auto &a = keys[key], &b = keys[key + 1];
  float span = b.phase - a.phase, t = (phase - a.phase) / span;
  LegPose result;
  result.hip = {0, hipY};
  result.ankle = {hermite(a.x, b.x, a.dx, b.dx, t, span),
                  hermite(a.y, b.y, a.dy, b.dy, t, span)};
  result.footAngle = a.angle + (b.angle - a.angle) * t * t * (3 - 2 * t);
  result.planted = phase <= .30f;
  // Two-bone IK: fixed bone lengths, knee always bends toward travel.
  constexpr float Thigh = .205f, Shin = .195f;
  float dx = result.ankle.x, dy = result.ankle.y - hipY;
  float distance = std::hypot(dx, dy);
  float along =
      (Thigh * Thigh - Shin * Shin + distance * distance) / (2 * distance);
  float bend = std::sqrt(std::max(0.f, Thigh * Thigh - along * along));
  result.knee = {dx * along / distance + dy * bend / distance,
                 hipY + dy * along / distance - dx * bend / distance};
  return result;
}
inline CourierPose courierPose(float cycle, int facing, float activity = 1) {
  float phase = cyclePhase(cycle);
  bool side = facing == 1 || facing == 3;
  CourierPose pose;
  pose.hipY = -.37f + .015f * std::cos(4 * AnimationPi * (phase - .08f));
  pose.lean = side ? 2.f + 1.2f * std::sin(2 * AnimationPi * phase) : 0;
  for (int leg = 0; leg < 2; ++leg) {
    float p = cyclePhase(phase + leg * .5f), outward = leg == 0 ? -1.f : 1.f;
    auto &limb = pose.legs[leg];
    limb = runningLeg(p, pose.hipY);
    if (!side) {
      float depth = facing == 0 ? 1.f : -1.f;
      limb.hip.x = outward * .075f;
      limb.knee = {outward * (.085f + .018f * std::sin(2 * AnimationPi * p)),
                   limb.knee.y + depth * limb.knee.x * .24f - .02f};
      limb.ankle = {outward * .10f,
                    limb.ankle.y + depth * limb.ankle.x * .24f - .055f};
      limb.footAngle = outward * 3 * std::sin(2 * AnimationPi * p);
    }
    // Arms counter the corresponding leg; carried pizza stays in both hands.
    float swing = -.75f * std::cos(2 * AnimationPi * p);
    if (side) {
      pose.shoulders[leg] = {leg == 0 ? -.025f : .045f, pose.hipY - .18f};
      pose.elbows[leg] = {pose.shoulders[leg].x + .14f * std::sin(swing),
                          pose.shoulders[leg].y + .14f * std::cos(swing)};
      pose.hands[leg] = {pose.elbows[leg].x + .13f * std::cos(swing),
                         pose.elbows[leg].y - .13f * std::sin(swing)};
    } else {
      pose.shoulders[leg] = {outward * .155f, pose.hipY - .17f};
      pose.elbows[leg] = {outward * .19f,
                          pose.hipY - .04f +
                              .025f * std::cos(2 * AnimationPi * p)};
      pose.hands[leg] = {outward * .16f,
                         pose.hipY + .02f -
                             .05f * std::cos(2 * AnimationPi * p)};
    }
  }
  // Settle the same painted parts into a neutral stance as movement slows.
  // No frame-sheet swap (and therefore no change of head/body proportions).
  activity = std::clamp(activity, 0.f, 1.f);
  auto settle = [activity](RigPoint &point, RigPoint rest) {
    point = {rest.x + (point.x - rest.x) * activity,
             rest.y + (point.y - rest.y) * activity};
  };
  pose.hipY = -.37f + (pose.hipY + .37f) * activity;
  pose.lean *= activity;
  for (int leg = 0; leg < 2; ++leg) {
    float outward = leg == 0 ? -1.f : 1.f;
    auto &p = pose.legs[leg];
    settle(p.hip, {side ? 0.f : outward * .075f, -.37f});
    settle(p.knee, {side ? .025f : outward * .085f, -.225f});
    settle(p.ankle, {side ? (leg == 0 ? .045f : -.065f) : outward * .10f,
                     side ? -.075f : -.13f});
    p.footAngle *= activity;
    settle(pose.shoulders[leg],
           {side ? (leg == 0 ? -.025f : .045f) : outward * .155f, -.55f});
    settle(pose.elbows[leg], {side ? -.015f : outward * .19f, -.41f});
    settle(pose.hands[leg], {side ? .015f : outward * .16f, -.29f});
  }
  return pose;
}
} // namespace xfrog
