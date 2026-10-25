# Xfrog 1.02

Original Xfrog game by Gustavo Simi Muller, recovered from the March 10, 2000 release archive, with modern Windows and native Linux builds.

## Downloads

Published builds are available on the [GitHub releases page](https://github.com/gumuller/xfrog/releases).
The Linux tarball includes a launcher and game data and requires SDL3 3.2+, zlib,
and compatible glibc/C++ runtime libraries. The Windows ZIP includes the pinned
DDrawCompat wrapper and its license. Extract the whole package before running.
The source archive remains available with each release.

## Build and run on Linux

Requires a C++ compiler, CMake 3.24+, Ninja, SDL **3.2+**, and zlib development files.
On Arch Linux / Omarchy:

```sh
sudo pacman -S --needed base-devel cmake ninja sdl3 zlib
```

From the repository, run:

```sh
./Play.sh
```

The launcher configures and builds incrementally, then starts the native 64-bit
executable at `build/linux/bin/xfrog`. You can also build manually:

```sh
cmake --preset linux
cmake --build --preset linux
./build/linux/bin/xfrog
```

The game finds `xfrog.cfs` beside the executable, so it can run from any working
directory. High scores are saved separately in
`${XDG_DATA_HOME:-~/.local/share}/Gustavo Muller/Xfrog/hs.dat`, using SDL's user data directory.
The original Windows assets and high-score format are retained. Linux starts
with a zero high score when there is no saved file.

Graphics retain the original 640x480, 256-color artwork. The resizable window
uses nearest-neighbor scaling and a 4:3 viewport. **F11** or **Alt+Enter** toggles
fullscreen. Losing focus pauses the game and audio and clears held keys.
The frame rate is capped at 60; long frame delays are bounded to avoid large
movement jumps after interruptions.

## Automated tests

Python 3 is needed for the test runners. To build and run the unit and integration
suites with dummy display/audio drivers:

```sh
cmake --preset linux
cmake --build build/linux
ctest --test-dir build/linux --output-on-failure
```

The five unit suites exercise keyboard transitions and bounds, clipped and
transparent blits, mirrored sprites, palette conversion, monotonic timing and
stall limits, CFS seeks and read bounds, and sound channel priorities/looping.
The integration suites cover all 36 original packed assets, malformed pack
rejection, startup/shutdown, and gameplay including pizza delivery, traffic
death/respawn, focus handling, menu navigation, and earned score persistence.
The delivery integration check uses a delivery-zone fixture and deterministic
time; it is separate from executable E2E coverage.

For the full suite, install the virtual display and OS input/capture tools:

```sh
# Arch Linux / Omarchy
sudo pacman -S --needed python xorg-server-xvfb xdotool imagemagick
# Ubuntu / Debian (in addition to the compiler, SDL3, zlib and build tools)
sudo apt-get install python3 xvfb xdotool imagemagick

cmake --preset linux-tests
cmake --build --preset linux-tests
ctest --preset linux-tests
```

The executable E2E test runs the normal game on its own Xvfb display. It sends
keyboard events with xdotool and compares actual window pixels with the original
artwork/font. It checks a fresh profile, instructions, gameplay, player movement,
return/resume, exit/cancel, and score loading across process restarts, from an
unrelated working directory. It uses temporary saves and dummy audio and does
not touch your desktop or your own scores. Screenshots, process logs, and a JSON
report are kept under `build/linux-tests/e2e-artifacts/`, including on failure.

CTest labels let you select a layer:

```sh
ctest --preset linux-tests -L unit
ctest --preset linux-tests -L integration
ctest --preset linux-tests -L e2e
```

To run the same full suite with memory and undefined-behavior checks:

```sh
cmake --preset linux-sanitizers
cmake --build --preset linux-sanitizers
ctest --preset linux-sanitizers
```

For a real display/audio gameplay check (including fullscreen), run:

```sh
./build/linux/bin/xfrog-gameplay-test
```

For the normal executable's startup/render/shutdown check:

```sh
./Play.sh --smoke-test
```

## Build and run on Windows 11

Install Visual Studio 2022 with **Desktop development with C++**, including the Windows SDK and CMake tools. The PowerShell helper finds the bundled CMake automatically. No old DirectX SDK, NASM, or DJGPP installation is needed for the game.

From PowerShell in this repository:

```powershell
.\build.ps1 -WithDDrawCompat -Run
```

This builds the **32-bit Release** executable at `build/windows-x86/bin/Release/xfrog.exe`, stages its data and licenses, installs the pinned DDrawCompat wrapper locally, and launches with the correct working directory. The original `bin/xfrog.exe` is untouched. Close the running game before rebuilding.

The first build downloads zlib 1.3.2 from its official site; `-WithDDrawCompat` downloads DDrawCompat v0.7.2 from its official GitHub release. Both downloads are pinned and SHA-256 checked. DDrawCompat v0.7.2 is marked prerelease upstream. No system DLLs or registry settings are changed. A different existing `ddraw.dll` will not be overwritten.

For a Debug build and a short startup/render/shutdown check:

```powershell
.\build.ps1 -Configuration Debug -WithDDrawCompat -SmokeTest
```

The smoke test opens the game, runs 120 menu frames, requires 120 successful DirectDraw flips, shuts down, and writes `xfrog-smoke.log` beside the executable. It does not verify visual appearance, keyboard controls, or audible sound. Keep the game focused during this test.

To build without downloading the optional wrapper:

```powershell
.\build.ps1
```

This leaves any previously installed wrapper in place. For native DirectDraw testing, use a fresh build directory or move `ddraw.dll` out of the output directory. If CMake is on PATH, the equivalent build commands are:

```powershell
cmake --preset windows-x86
cmake --build --preset release
```

You can also open the repository folder in Visual Studio and select the `windows-x86` configure preset. Run the new executable from its output directory; `xfrog.cfs` and `hs.dat` must be present. Existing high scores in the output directory survive rebuilds.

## Controls

Use Up/Down and Enter in the menu. Use the arrow keys to move, Space to continue past the introduction and deliver pizza, and Escape to return to the menu. To quit, choose Exit and press Y at the confirmation screen.

## Compatibility changes

- CMake x86 builds with the current MSVC compiler and Windows SDK; zlib and the C/C++ runtime are linked statically.
- DirectInput 8 replaces the old DirectInput 7 initialization and missing `dinput.lib` dependency.
- Standard C memory routines replace the NASM dependency, including safe zero-length operations.
- Loop variables use standard C++ scopes, and Windows painting is correctly completed.
- Screen flipping returns to the Windows message loop after a failed flip instead of retrying forever.
- The CFS reader decodes the original 80-byte disk records independently of native pointers and uses the correct zlib length type. Linux builds as 64-bit; the existing Windows build remains x86.

On Windows, graphics still use DirectDraw 7 at 640x480 with a 256-color palette. DDrawCompat addresses compatibility with current Windows graphics drivers. See its [upstream project](https://github.com/narzoul/DDrawCompat).

## Original files

- `src/game/src/`: game source and preserved legacy NASM file (unused by CMake).
- `src/game/project/xfrog.dsw`: original Visual C++ 6 workspace, retained as historical reference; use CMake for the modified source.
- `src/game/gamedata/`: 31 TGA images and 5 WAV sounds.
- `src/cfspack/`: original DJGPP packer source and executable, not built by CMake. The existing pack is sufficient to run the game.
- `bin/`: untouched original executable, asset pack, and seed high-score file.
- `docs/`: original player/developer instructions and GPL license.
- `archive/xfrog102.zip`: untouched release archive containing all 96 original files.

All 36 packed assets have been decompressed and verified against the loose assets. The historical source is preserved in the ZIP and initial Git commit.

## Platform implementation

The gameplay, menus, sprites, palette, and asset pack are shared. CMake selects
`src/game/src/sdl/` for Linux windowing, graphics, input, audio, and timing;
Windows retains the existing Win32 / DirectDraw / DirectInput / DirectSound
implementation. Shared headers select the backend and expose game key and
palette types. Linux requires neither Wine nor DDrawCompat.

The GitHub Actions workflow builds Windows x86 in Release and Debug, and runs
the full Linux unit/integration/executable E2E suite both normally and with
AddressSanitizer/UndefinedBehaviorSanitizer. CI uploads test logs and screenshots
and creates the Linux tarball and Windows ZIP used for releases.

## Validation

Linux: built with GCC 16.2 and SDL 3.4.18. All ten CTest checks pass, including
under AddressSanitizer and UndefinedBehaviorSanitizer. The gameplay test also
passes on the real Wayland display and PipeWire audio device; fullscreen toggles,
rendered menu/gameplay captures, and nonzero speaker-monitor audio were checked.
Windows: the DirectX backend cross-compiles to a 32-bit Windows executable with
LLVM-MinGW. The modified version has not been run on a Windows machine; the
following Windows runtime results describe the earlier Windows-only revision.


Built with Visual Studio 2022/MSVC 19.44 and Windows SDK 10.0.26100.0 on Windows 11 x64, build 26300. Both Release and Debug compile without warnings and pass the 120-frame smoke test with DDrawCompat v0.7.2. Automated window inspection was unavailable, so a visual/gameplay check remains necessary.

## License

The game source is GNU GPL version 2 or later; see source headers and `docs/gnu.txt`. Build outputs include the game and zlib license texts. DDrawCompat uses the BSD Zero Clause license, copied into the output when installed by the helper. Its binary is downloaded on demand and is not checked into this repository.
