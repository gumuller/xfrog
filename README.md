# Xfrog 1.02

Original Xfrog game by Gustavo Simi Muller, recovered from the March 10, 2000 release archive.

## Contents

- `src/game/src/`: C++ game source and NASM memory routines.
- `src/game/project/xfrog.dsw`: original Visual C++ workspace.
- `src/game/gamedata/`: 31 TGA images and 5 WAV sounds.
- `src/cfspack/`: compressed file system packer source and original executable.
- `bin/`: original Windows executable, `xfrog.cfs` asset pack, and high score file.
- `docs/`: original player instructions, developer notes, and GPL license.
- `archive/xfrog102.zip`: untouched original release archive.

## Run the original executable

From PowerShell in this repository:

```powershell
Push-Location .\bin
.\xfrog.exe
Pop-Location
```

The working directory must contain `xfrog.cfs`. The original game uses DirectDraw 7, DirectInput, and DirectSound, with a fullscreen 640x480 8-bit display. Compatibility with current Windows has not yet been tested.

Use the arrow keys to move, Space to deliver pizza, and Escape for the menu.

## Original build

The developer notes specify Visual C++ 6.0, DirectX 7+, zlib (listed as v1.13), and NASM 0.98. The CFS packer originally used DJGPP 2.02. See `docs/devreadme.txt`.

No modernization changes have been made. All 96 release files are preserved byte-for-byte. All 36 assets in `bin/xfrog.cfs` were decompressed and verified against the loose assets.

## License

The original source is licensed under GNU GPL version 2 or later. See the source headers and `docs/gnu.txt`.
