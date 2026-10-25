#!/usr/bin/env python3
"""Drive the unmodified game executable through X11 and verify displayed pixels.

No engine symbols, deterministic-time builds, injected delivery zones, or in-game
input hooks are used here. The separate integration suite covers those fixtures.
"""
import argparse
import json
import os
import re
from pathlib import Path
import select
import shutil
import struct
import subprocess
import tempfile
import time


def require(condition, message):
    if not condition:
        raise AssertionError(message)
    print("PASS:", message, flush=True)


def ppm(data):
    """Decode ImageMagick's 8-bit P6 capture without a Python imaging dependency."""
    offset = 0
    tokens = []
    while len(tokens) < 4:
        while data[offset:offset + 1].isspace():
            offset += 1
        if data[offset:offset + 1] == b"#":
            offset = data.index(b"\n", offset) + 1
            continue
        end = offset
        while not data[end:end + 1].isspace():
            end += 1
        tokens.append(data[offset:end])
        offset = end
    require(tokens[0] == b"P6" and tokens[3] == b"255", "capture has RGB pixels")
    width, height = map(int, tokens[1:3])
    offset += 2 if data[offset:offset + 2] == b"\r\n" else 1
    pixels = data[offset:]
    require(len(pixels) == width * height * 3, "capture is complete")
    return width, height, pixels


def tga(path):
    data = path.read_bytes()
    require(data[2] == 1 and data[16] == 8, "reference asset is indexed TGA")
    width, height = struct.unpack_from("<HH", data, 12)
    colors = struct.unpack_from("<H", data, 5)[0]
    start = 18 + data[0]
    palette = [bytes((data[start + i * 3 + 2], data[start + i * 3 + 1], data[start + i * 3]))
               for i in range(colors)]
    start += colors * data[7] // 8
    rows = [data[start + y * width:start + (y + 1) * width] for y in range(height)]
    if not data[17] & 32:
        rows.reverse()
    if data[17] & 16:
        rows = [row[::-1] for row in rows]
    return width, height, b"".join(rows), palette


class Game:
    def __init__(self, args, env, directory, label):
        self.args, self.env, self.directory = args, env, directory
        self.log = open(directory / (label + ".log"), "wb")
        self.process = subprocess.Popen([args.game], cwd=directory, env=env,
                                        stdout=self.log, stderr=subprocess.STDOUT)
        try:
            self.window = None
            deadline = time.monotonic() + 8
            while time.monotonic() < deadline:
                found = self.command("search", "--onlyvisible", "--pid", str(self.process.pid),
                                     "--name", "^Xfrog$", check=False)
                if found.returncode == 0 and found.stdout.strip():
                    self.window = found.stdout.splitlines()[0].decode()
                    break
                require(self.process.poll() is None, "game stays alive during startup")
                time.sleep(0.05)
            require(self.window is not None, "normal executable opens a visible window")
            self.command("windowsize", "--sync", self.window, "640", "480")
            self.command("windowmove", "--sync", self.window, "0", "0")
            self.command("windowfocus", "--sync", self.window)
            time.sleep(0.2)
        except BaseException:
            self.close()
            raise

    def command(self, *arguments, check=True):
        return subprocess.run([self.args.xdotool, *arguments], env=self.env,
                              stdout=subprocess.PIPE, stderr=subprocess.PIPE,
                              check=check, timeout=5)

    def key(self, key, duration=0.08):
        self.command("keydown", key)
        time.sleep(duration)
        self.command("keyup", key)

    def capture(self, name):
        capture = subprocess.run([self.args.capture, "-window", self.window, "-depth", "8", "ppm:-"],
                                 env=self.env, capture_output=True, check=True, timeout=5)
        (self.directory / (name + ".ppm")).write_bytes(capture.stdout)
        width, height, pixels = ppm(capture.stdout)
        require((width, height) == (640, 480), "game viewport is 640x480")
        return pixels

    def wait(self, name, predicate):
        deadline = time.monotonic() + 5
        while time.monotonic() < deadline:
            pixels = self.capture(name)
            if predicate(pixels):
                require(True, name)
                return pixels
            require(self.process.poll() is None, "game stays alive while awaiting " + name)
            time.sleep(0.05)
        raise AssertionError("screen did not reach " + name)

    def close(self):
        if self.process.poll() is None:
            self.process.terminate()
            try:
                self.process.wait(timeout=3)
            except subprocess.TimeoutExpired:
                self.process.kill()
                self.process.wait(timeout=3)
        self.log.close()


def main():
    parser = argparse.ArgumentParser()
    for flag in ("game", "assets", "xvfb", "xdotool", "capture", "artifacts"):
        parser.add_argument("--" + flag, required=True)
    args = parser.parse_args()
    args.game = str(Path(args.game).resolve())
    output = Path(args.artifacts).resolve()
    output.mkdir(parents=True, exist_ok=True)
    (output / "report.json").write_text(json.dumps({"status": "running"}) + "\n")
    assets = Path(args.assets)
    palette = tga(assets / "gfx/palette.tga")[3]
    palette[0], palette[255] = bytes((0, 0, 0)), bytes((255, 255, 255))
    logo_w, logo_h, logo, _ = tga(assets / "gfx/ui/xfroglogo.tga")
    _, _, buildings, _ = tga(assets / "gfx/env/buildings.tga")
    font_w, _, font, _ = tga(assets / "gfx/ui/gfont.tga")

    def logo_matches(pixels):
        left = 320 - logo_w // 2
        for y in range(logo_h):
            expected = b"".join(palette[p] for p in logo[y * logo_w:(y + 1) * logo_w])
            start = ((y + 90) * 640 + left) * 3
            if pixels[start:start + logo_w * 3] != expected:
                return False
        return True

    version_header = assets.parent / "src/version.h"
    version = re.search(r'#define\s+XFROG_VERSION\s+"([^"]+)"', version_header.read_text()).group(1)

    def text_matches(pixels, text, y, left=None):
        if left is None:
            left = 320 - len(text) * 4
        for i, character in enumerate(text):
            code = ord(character) - 32
            fx, fy = (code % 16) * 8, (code // 16) * 8
            for row in range(8):
                for col in range(8):
                    color = font[(fy + row) * font_w + fx + col]
                    if color:  # text is transparent over the game background
                        start = ((y + row) * 640 + left + i * 8 + col) * 3
                        if pixels[start:start + 3] != palette[color]:
                            return False
        return True

    def gameplay_matches(pixels):
        # This strip avoids the HUD and moving delivery balloon, but verifies
        # actual game scenery using the original assets as an independent oracle.
        expected = b"".join(palette[p] for p in buildings[40 * 640:100 * 640])
        return pixels[40 * 640 * 3:100 * 640 * 3] == expected

    def changed(a, b, x, y, w, h):
        return sum(a[(row * 640 + col) * 3:(row * 640 + col + 1) * 3] !=
                   b[(row * 640 + col) * 3:(row * 640 + col + 1) * 3]
                   for row in range(y, y + h) for col in range(x, x + w))

    xvfb = None
    games = []
    with tempfile.TemporaryDirectory(prefix="xfrog-e2e-") as scratch:
        work = Path(scratch)
        read_fd, write_fd = os.pipe()
        with open(output / "xvfb.log", "wb") as log:
            try:
                xvfb = subprocess.Popen([args.xvfb, "-displayfd", str(write_fd), "-screen", "0",
                                         "1024x768x24", "-nolisten", "tcp", "-ac", "-noreset"],
                                        pass_fds=(write_fd,), stdout=log, stderr=log)
            finally:
                os.close(write_fd)
            try:
                require(bool(select.select([read_fd], [], [], 8)[0]), "isolated virtual display starts")
                display = os.read(read_fd, 64).decode().strip()
                require(display.isdigit(), "virtual display reports its number")
                os.close(read_fd)
                env = os.environ.copy()
                env.update(DISPLAY=":" + display, SDL_VIDEODRIVER="x11", SDL_RENDER_DRIVER="software",
                           SDL_AUDIODRIVER="dummy", XDG_DATA_HOME=str(work / "data"))
                # Bootstrap a fresh profile through the real binary, then discover
                # SDL's preference directory instead of assuming its path layout.
                bootstrap = subprocess.run([args.game, "--smoke-test"], cwd=work, env=env,
                                           capture_output=True, timeout=12)
                (work / "bootstrap.log").write_bytes(bootstrap.stdout + bootstrap.stderr)
                require(bootstrap.returncode == 0, "fresh-profile startup and shutdown succeeds")
                scores = list((work / "data").rglob("hs.dat"))
                require(len(scores) == 1, "fresh profile creates one score file")
                score_path = scores[0]
                require(score_path.read_bytes() == struct.pack("<i", 2125), "fresh profile starts at score zero")
                seeded_score = struct.pack("<i", 321 ^ 2125)
                score_path.write_bytes(seeded_score)
                game = Game(args, env, work, "first-run")
                games.append(game)
                menu = game.wait("01-menu", logo_matches)
                require(text_matches(menu, "v" + version, 472, 640 - (len(version) + 2) * 8),
                        "menu displays the source version " + version)
                game.key("Return")
                game.wait("02-instructions", lambda p: text_matches(p, "BE WELCOME TO FROG CITY!", 10))
                game.key("space")
                before = game.wait("03-gameplay", gameplay_matches)
                require(text_matches(before, "HIGH SCORE: 321", 0), "normal executable loads seeded high score")
                game.key("Right", 0.45)
                moved = game.wait("04-moved", gameplay_matches)
                require(changed(before, moved, 304, 370, 32, 32) > 30 and
                        changed(before, moved, 340, 370, 55, 32) > 30,
                        "OS keyboard input moves the rendered player across the parking area")
                game.key("Escape")
                game.wait("05-menu-return", logo_matches)
                game.key("Down"); game.key("Return")
                game.wait("06-resumed", gameplay_matches)
                game.key("Escape")
                game.wait("07-menu", logo_matches)
                game.key("Down"); game.key("Return")
                game.wait("08-exit-confirmation", lambda p: text_matches(p, "Do you want to exit ? (Y/N)", 236))
                game.key("n")
                game.wait("09-exit-cancelled", logo_matches)
                game.key("Return")
                game.wait("10-exit-confirmation", lambda p: text_matches(p, "Do you want to exit ? (Y/N)", 236))
                game.key("y")
                require(game.process.wait(timeout=5) == 0, "Y exits the real process successfully")
                require(score_path.read_bytes() == seeded_score, "existing high score survives a normal session")
                # Relaunch the same executable and observe the score through its HUD.
                game = Game(args, env, work, "second-run")
                games.append(game)
                game.wait("11-relaunched-menu", logo_matches)
                game.key("Return")
                game.wait("12-relaunched-instructions", lambda p: text_matches(p, "BE WELCOME TO FROG CITY!", 10))
                game.key("space")
                frame = game.wait("13-relaunched-game", gameplay_matches)
                require(text_matches(frame, "HIGH SCORE: 321", 0), "high score reloads in a new process")
                game.key("Escape")
                game.wait("14-final-menu", logo_matches)
                game.key("Down"); game.key("Down"); game.key("Return")
                game.wait("15-final-confirmation", lambda p: text_matches(p, "Do you want to exit ? (Y/N)", 236))
                game.key("y")
                require(game.process.wait(timeout=5) == 0, "relaunched process shuts down cleanly")
                (output / "report.json").write_text(json.dumps({"status": "passed", "executable": args.game,
                    "checks": ["asset discovery from unrelated working directory", "menu", "instructions",
                               "gameplay scenery", "OS keyboard movement", "menu and resume",
                               "exit cancellation", "clean exit", "score persistence across processes"]}, indent=2) + "\n")
            except BaseException as error:
                (output / "report.json").write_text(json.dumps({"status": "failed", "error": str(error)}, indent=2) + "\n")
                raise
            finally:
                for game in games:
                    game.close()
                for path in work.glob("*.ppm"):
                    shutil.copy2(path, output / path.name)
                for path in work.glob("*.log"):
                    shutil.copy2(path, output / path.name)
                xvfb.terminate()
                xvfb.wait(timeout=5)


if __name__ == "__main__":
    main()
