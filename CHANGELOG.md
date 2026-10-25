# Changelog

## 2.0.0 — HD reimagining

- Replace the indexed primary renderer with a shared SDL3 full-color 1080p
  widescreen implementation on Linux and Windows.
- Add consistent cartoon city artwork, 32 directional courier frames, eight
  vehicles, scalable typography, alpha sprites, shadows, particles and dash trails.
- Add heat-dependent tips, delivery streaks, successive shifts, relaxed/arcade
  difficulty, dash cooldowns and forgiving feet/bumper collision bounds.
- Add a complete menu/help/pause/results flow, mouse and gamepad support, original
  synthesized music/cues, focus pause and recoverable per-user progress.
- Add HD asset/render contracts, whole-shift simulations, independent GUI E2E
  checks and a real delivery through live traffic using ordinary OS input.
- Preserve the original implementation as an optional classic target.

## 1.03 — 2026-10-25

- Add a native 64-bit Linux port using SDL3 for windowing, graphics, keyboard,
  sound, and timing; retain the Windows x86 DirectX backend and build helper.
- Preserve the original 640x480 indexed artwork, assets, and gameplay. Add a
  resizable Linux window, fullscreen toggle, focus pause, and per-user scores.
- Decode the historical CFS format independently of native pointer sizes and
  use the correct zlib length type. Check archive boundaries and invalid reads.
- Add five unit suites for input, rendering, timing, archive reads, and audio.
- Add four integration suites for original assets, malformed packs, startup,
  and gameplay including delivery, collision/respawn, focus, and saved scores.
- Add an executable E2E test using an isolated virtual display and OS keyboard
  events, with rendered-screen checks and score persistence across restarts.
- Run all ten Linux checks normally and with address/undefined-behavior
  sanitizers in CI. Build Windows x86 Release and Debug on Windows runners.
- Package Linux and Windows release downloads. Windows includes the pinned
  DDrawCompat wrapper; Linux requires SDL3 3.2+, zlib, and compatible runtime
  libraries. The source archive includes the original release for reference.

## 1.02 Windows build

- Recover and preserve the original March 10, 2000 game release and assets.
- Add a modern Windows x86 CMake/MSVC build, DirectInput 8 compatibility, and
  optional pinned DDrawCompat integration.
