# Xfrog 1.02

Original Xfrog game by Gustavo Simi Muller, recovered from the March 10, 2000 release archive, with a modern Windows build.

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
- The original 80-byte CFS entry layout is checked at compile time. A 64-bit build is deliberately rejected because the original asset format contains a 32-bit pointer field.

The graphics still use DirectDraw 7 at 640x480 with a 256-color palette. DDrawCompat addresses compatibility with current Windows graphics drivers. See its [upstream project](https://github.com/narzoul/DDrawCompat).

## Original files

- `src/game/src/`: game source and preserved legacy NASM file (unused by CMake).
- `src/game/project/xfrog.dsw`: original Visual C++ 6 workspace, retained as historical reference; use CMake for the modified source.
- `src/game/gamedata/`: 31 TGA images and 5 WAV sounds.
- `src/cfspack/`: original DJGPP packer source and executable, not built by CMake. The existing pack is sufficient to run the game.
- `bin/`: untouched original executable, asset pack, and seed high-score file.
- `docs/`: original player/developer instructions and GPL license.
- `archive/xfrog102.zip`: untouched release archive containing all 96 original files.

All 36 packed assets have been decompressed and verified against the loose assets. The historical source is preserved in the ZIP and initial Git commit.

## Validation

Built with Visual Studio 2022/MSVC 19.44 and Windows SDK 10.0.26100.0 on Windows 11 x64, build 26300. Both Release and Debug compile without warnings and pass the 120-frame smoke test with DDrawCompat v0.7.2. Automated window inspection was unavailable, so a visual/gameplay check remains necessary.

## License

The game source is GNU GPL version 2 or later; see source headers and `docs/gnu.txt`. Build outputs include the game and zlib license texts. DDrawCompat uses the BSD Zero Clause license, copied into the output when installed by the helper. Its binary is downloaded on demand and is not checked into this repository.
