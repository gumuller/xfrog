# Xfrog 2.0 — The Pizza Run

A full-color, widescreen reimagining of Gustavo Simi Muller's original pizza
courier arcade game. Deliver hot pizzas across Frog City's traffic in a sunny
cartoon town, with a smoothly animated courier, a mischievous pizza thief,
eight frog-driven vehicles at consistent scale, responsive movement,
an original three-minute console-style stereo score, and a complete shift-based game loop.

## Play on Linux

Requires a C++17 compiler, CMake 3.24+, Ninja, SDL3 3.2+, and Python 3 for tests.
On Arch/Omarchy:

```sh
sudo pacman -S --needed base-devel cmake ninja sdl3 python
./Play.sh
```

Manual build:

```sh
cmake --preset linux
cmake --build --preset linux
./build/linux/bin/xfrog
```

The renderer draws to a 1920x1080 full-color framebuffer and scales to the window
or fullscreen display, preserving 16:9. Textures use 24-bit RGB with an additional
alpha channel for transparency. This is new full-color artwork and rendering,
rather than an enlarged indexed framebuffer. Keyboard movement simulates at a
fixed 120 Hz with render interpolation. Courier running uses a layered 2D rig
with alternating contact, compression, push-off and airborne poses. Joint
constraints keep the knees connected, and the gait follows distance travelled.
Idle and running share the same artwork and blend as movement slows. The thief
keeps its separate eight-pose animation. Both characters use an 88-pixel gameplay
height, 20% smaller than the previous 110-pixel presentation, including reactions
and courier dash trails.
Sprite loading rejects detached atlas marks and preserves broad running feet.


## Play on Windows

Install Visual Studio 2022 with Desktop development with C++, the Windows SDK,
and CMake tools, then run:

```powershell
.\build.ps1 -Run
```

The x86 build is staged at `build/windows-x86/bin/Release/xfrog.exe`. SDL 3.4.18
is downloaded from its official release with a pinned SHA-256 and linked
statically alongside the Visual C++ runtime. The same HD implementation is used
on Windows and Linux. The HD game does not require DirectDraw or DDrawCompat.
Debug builds use `-Configuration Debug`.

## Controls and the delivery loop

| Action | Keyboard | Gamepad |
| --- | --- | --- |
| Move | Arrows / WASD | Left stick |
| Pick up / deliver | Space | South / A |
| Dash | Shift | East / B |
| Pause | Escape / P | Start |
| Help | H | Menu button with mouse |
| Fullscreen | F11 / Alt+Enter | — |
| Sound toggle | M | Menu button with mouse |

Start at the pizza shop and press Space. Walk around the shop, cross the three
traffic lanes, and deliver at the glowing door with Space. Hotter pizzas and
consecutive deliveries earn better tips. Dash has a cooldown. Collisions cost a
heart and drop the pizza, then give a short respawn grace period.

Clear six deliveries to finish your first shift; subsequent shifts increase the
goal and traffic speed. Traffic also advances within a shift after every two
deliveries and every 50 seconds, including Relaxed mode. At traffic level 3,
cars can signal for 1.2 seconds before a smooth two-second lane change between
the same-direction outer lanes. Projected gaps in the opposing lane and a safe
distance from the courier are required. Faster cars signal to overtake when a
safe same-direction route is available. They brake and follow when it is blocked.
Whole queues keep visible bumper gaps, including across the screen-wrap boundary.
Merge speed is held steady, and the path is rechecked before steering starts.
All vehicle classes share a 170-pixel
collision footprint and a consistent on-screen width.

The thief always runs along the upper sidewalk beside the delivery addresses, warns before entering,
and can snatch a carried pizza. Theft breaks the delivery streak without taking
a heart; the thief carries the stolen box away and cannot steal another until
a later pass. Dash and careful spacing help avoid him.

Relaxed mode offers five hearts, fewer cars and a
three-minute shift. Arcade has three hearts and denser traffic over two and a
half minutes. Help, pause and loss of focus stop simulation and sound.

Records, completed delivery totals, sound and difficulty are stored using SDL's
per-user `Xfrog 2` data directory. They are separate from the original 1.03
scores. Saves are replaced atomically and support Unicode paths and backup
recovery. The game locates its assets beside the executable, independently of
the current working directory.

## Automated tests

```sh
cmake --preset linux
cmake --build build/linux
ctest --test-dir build/linux --output-on-failure
```

Unit suites cover movement, collision bounds, dash cooldown, pizza heat, tips,
streaks, shift progression, traffic bonuses, save recovery and Unicode paths.
Automated playthroughs drive ordinary controls across live traffic through whole
shifts. Renderer tests draw menu, gameplay, order, pause, help and results scenes,
check full HD output and thousands of distinct colors, and verify bounded music.
The asset contract checks actual RGB/RGBA PNGs, transparent atlases, fonts and
checksums. Separate suites cover thief warnings, theft, escape, difficulty
progression, safe/blocked lane changes and consistent vehicle footprints.
The audio suite checks all 184.6 seconds of the stereo score for clipping and DC
bias, checks section variation, stereo width, smooth loop boundaries and nine
distinct effects, and exports the theme plus the cashier effect.

For independent executable E2E tests, install the virtual display, OS input,
image capture and OCR tools:

```sh
# Arch / Omarchy
sudo pacman -S --needed xorg-server-xvfb xdotool imagemagick tesseract tesseract-data-eng
# Ubuntu / Debian, in addition to the build dependencies
sudo apt-get install xvfb xdotool imagemagick tesseract-ocr

cmake --preset linux-tests
cmake --build --preset linux-tests
ctest --preset linux-tests
```

These launch the normal executable on isolated 1920x1080 displays and use real
OS mouse/keyboard events, captured images and independent OCR. They cover menus,
help, difficulty, movement, pickup, named destinations, pause, focus handling,
sound, settings and records across process restarts, plus a real delivery across
traffic and persistence of its earned score. No engine-state access or forced
score is used in the executable tests. Artifacts are saved under
`build/linux-tests/e2e-artifacts/`, `delivery-evidence/` and `render-evidence/`.

Sanitizers:

```sh
cmake --preset linux-sanitizers
cmake --build --preset linux-sanitizers
ctest --preset linux-sanitizers
```

CTest labels `unit`, `integration` and `e2e` select each layer. Windows CI also
runs the model and dummy-driver renderer/startup tests in Release and Debug.
The normal executable supports `--smoke-test`, `--fullscreen`, `--seed N`,
`--version`, and `--help`.

## Art, design and future improvements

See [the art direction and production prompts](docs/v2/art-direction.md) for the
built-in image-generation workflow and consistent asset palette, and
[the game design notes](docs/v2/design.md) for implemented improvements and
suggested future districts, cosmetics and daily challenges. Fonts retain their
included SIL Open Font Licenses; SDL and stb license texts are staged with builds.
The original town score is an 80-bar, 104 BPM arrangement with an intro,
contrasting melodies, quiet bridge and reprise. Soft flute, electric piano,
vibraphone, strings, bass, percussion and stereo echo evoke sampled console music.
It contains no borrowed recordings or melodies. `scripts/compose_theme.py`
regenerates `assets/v2/town-theme.wav` with Python and numpy; those tools are
unnecessary at runtime. The game loads the included 32 kHz stereo PCM score.
Delivery plays a synthesized cashier drawer and bell; theft has its own cue.

## Original game and classic build

The recovered source, original sprites/sounds, CFS archive and untouched release
ZIP remain in `src/game/`, `src/cfspack/`, `bin/`, `docs/` and `archive/`.
Version 1.03 is preserved at tag `v1.03` and its published GitHub release.
To also build the original implementation alongside the HD game:

```sh
cmake --preset linux -DXFROG_BUILD_CLASSIC=ON
cmake --build build/linux --target xfrog-classic
./build/linux/bin/xfrog-classic
```

Its regression tests remain under `tests/classic/`. The optional Windows classic
target retains the Win32/DirectDraw/DirectInput/DirectSound backend and requires
x86. The HD game remains the default target on both platforms.

## License

The game source is GPL version 2 or later; see `docs/gnu.txt`. Third-party fonts,
SDL and stb retain their individual included licenses.
