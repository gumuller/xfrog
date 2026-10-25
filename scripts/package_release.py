#!/usr/bin/env python3
"""Package the built runtime, preserving Linux executable permissions."""
import argparse
from pathlib import Path
import re
import tarfile
import tempfile
import zipfile
import shutil


def package(platform, source, destination, version):
    suffix = "linux-x86_64" if platform == "linux" else "windows-x86"
    name = f"xfrog-{version}-{suffix}"
    files = ["xfrog" if platform == "linux" else "xfrog.exe", "COPYING.txt", "stb-LICENSE.txt", "SDL-LICENSE.txt"]
    if not (source / "assets/v2").is_dir():
        raise FileNotFoundError(source / "assets/v2")
    for filename in files:
        if not (source / filename).is_file():
            raise FileNotFoundError(source / filename)
    destination.mkdir(parents=True, exist_ok=True)
    with tempfile.TemporaryDirectory(prefix="xfrog-package-") as scratch:
        root = Path(scratch) / name
        root.mkdir()
        for filename in files:
            shutil.copy2(source / filename, root / filename)
        shutil.copytree(source / "assets/v2", root / "assets/v2")
        if platform == "linux":
            (root / "xfrog").chmod(0o755)
            (root / "Play.sh").write_text('#!/bin/sh\nset -eu\ncd -- "$(dirname -- "$0")"\nexec ./xfrog "$@"\n')
            (root / "Play.sh").chmod(0o755)
            instructions = """Native Linux x86_64 build. Requires SDL3 >= 3.2 and a C++ runtime.
On Arch/Omarchy: sudo pacman -S --needed sdl3
Built on Ubuntu 24.04; other distributions need compatible runtime libraries.
Extract the whole archive and run ./Play.sh (no compiler is needed).
High scores use SDL's user data directory, separate from the installation.
"""
        else:
            instructions = """Windows x86 build with statically linked SDL3 and Visual C++ runtime.
Extract the whole archive and run xfrog.exe from this folder.
No DirectDraw wrapper or extra runtime DLLs are required.
Progress is saved in the per-user Xfrog 2 data directory.
"""
        (root / "README.txt").write_text(f"Xfrog {version}\n\n" + instructions + """
Controls: arrows/WASD move, Space picks up/delivers, Shift dashes.
Escape pauses, H opens help, M toggles sound. F11 or Alt+Enter is fullscreen.
Source and GPL license: https://github.com/gumuller/xfrog
""")
        if platform == "linux":
            result = destination / (name + ".tar.gz")
            with tarfile.open(result, "w:gz") as archive:
                archive.add(root, arcname=name)
        else:
            result = destination / (name + ".zip")
            with zipfile.ZipFile(result, "w", zipfile.ZIP_DEFLATED) as archive:
                for path in sorted(root.rglob("*")):
                    if path.is_file():
                        archive.write(path, f"{name}/{path.relative_to(root).as_posix()}")
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--platform", choices=("linux", "windows"), required=True)
    parser.add_argument("--input", type=Path, required=True)
    parser.add_argument("--output", type=Path, required=True)
    args = parser.parse_args()
    version_header = Path(__file__).resolve().parents[1] / "src/v2/game.hpp"
    version = re.search(r'Version\s*=\s*"([^"]+)"', version_header.read_text()).group(1)
    print(package(args.platform, args.input, args.output, version))


if __name__ == "__main__":
    main()
