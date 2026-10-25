# Changelog

## 2.0.0 — HD reimagining

- Replace the indexed primary renderer with a shared SDL3 full-color 1080p
  widescreen implementation on Linux and Windows.
- Add consistent cartoon city artwork, 84 courier animation frames, eight
  vehicles, scalable typography, alpha sprites, shadows, particles and dash trails.
- Add heat-dependent tips, delivery streaks, successive shifts, relaxed/arcade
  difficulty, dash cooldowns and forgiving feet/bumper collision bounds.
- Add a complete menu/help/pause/results flow, mouse and gamepad support, original
  console-style music and nine effects, focus pause and recoverable per-user progress.
- Add HD asset/render contracts, whole-shift simulations, independent GUI E2E
  checks and a real delivery through live traffic using ordinary OS input.
- Add visible pizza-carrying walks and idles in all directions, pickup/handoff
  sequences, interpolated movement and animated result transitions.
- Preserve the original implementation as an optional classic target.

### Playtest revisions

- Restore the original wide-stride running sprites, adding four recoil/passing
  drawings per courier direction and a dedicated eight-pose thief run.
- Keep all thief appearances on the delivery-address sidewalk, with a repeated
  patrol regression test.
- Remove disconnected atlas fragments and avoid torso/leg seams.
- Replace the short chiptune with a 184.6-second stereo, sampled-console-style
  arrangement, contrasting themes and quiet bridge; add cashier delivery audio.
- Restore the pizza thief, warning, theft, carrying/escape state and cooldown.
- Add within-shift difficulty progression in Relaxed and Arcade modes, signaled
  gap-checked lane changes, and consistent scale across all vehicles.
- Replace interaction subtitles with outlined, shadowed speech balloons.
- Add thief/progression regression suites, directional walk proofs, music variation
  and loop checks, and vehicle footprint assertions.

### Animation and traffic corrections

- Keep courier and thief artwork separate; use the courier's original eight-frame
  cycles with corrected contact/recoil ordering and no mid-cycle flips.
- Cap gait speed and interpolate animation phase; add five-second tick-driven
  playback and frame-continuity checks at 30, 60 and 120 FPS.
- Reduce the thief to the courier's 110-pixel display height.
- Add braking, following distances and free-lane overtaking. Preserve queue gaps
  across wraps, recheck merges, and validate whole swept paths between frames.

### Loop continuity and character scale

- Remove repeated end/contact poses and interpolate the original courier artwork
  through 96 samples per cycle, including its last-to-first transition.
- Keep the gait clock bounded and interpolate forward across phase zero.
- Reduce both gameplay characters from 110 to 88 pixels (20%), including courier
  reactions and dash trails; preserve the thief's animation.
- Validate every rendered sample in all four directions with and without pizza,
  exact loop closure, source-art/motion-data checksums, and repeated live loops.

### Anatomical run-cycle correction

- Replace whole-image motion interpolation with separately painted courier parts
  and continuous two-bone leg animation in all four directions, with/without pizza.
- Author alternating contact, push-off, flight and recovery phases with matching
  endpoint velocities; prevent knee locking and keep planted feet from skating.
- Blend the same rig into idle, avoiding artwork/proportion changes at each stop.
- Preserve the thief's existing animation and both characters' reduced size.
- Add bone-length, reach, ground-contact and joint-continuity regression tests,
  alongside full-cycle renderer captures and native executable E2E coverage.
- Remove obsolete optical-flow data and its authoring dependency.

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
