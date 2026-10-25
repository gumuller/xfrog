#!/usr/bin/env python3
"""Exercise the normal HD game with OS input, screenshots and independent OCR."""
import argparse
import json
import os
from pathlib import Path
import select
import re
import subprocess
import tempfile
import time


def require(value, message):
    if not value:
        raise AssertionError(message)
    print("PASS:", message, flush=True)


def main():
    parser = argparse.ArgumentParser()
    for name in ("game", "xvfb", "xdotool", "capture", "ocr", "artifacts"):
        parser.add_argument("--" + name, required=True)
    parser.add_argument("--delivery", action="store_true")
    args = parser.parse_args()
    game = str(Path(args.game).resolve())
    output = Path(args.artifacts).resolve()
    output.mkdir(parents=True, exist_ok=True)
    (output / "report.json").write_text('{"status":"running"}\n')
    process = None
    server = None
    with tempfile.TemporaryDirectory(prefix="xfrog2-e2e-") as temp:
        work = Path(temp)
        env = os.environ.copy()
        read_fd, write_fd = os.pipe()
        with open(output / "xvfb.log", "wb") as log:
            try:
                server = subprocess.Popen([args.xvfb, "-displayfd", str(write_fd), "-screen", "0",
                                           "1920x1080x24", "-nolisten", "tcp", "-ac", "-noreset"],
                                          pass_fds=(write_fd,), stdout=log, stderr=log)
            finally:
                os.close(write_fd)
            try:
                require(select.select([read_fd], [], [], 8)[0], "virtual HD display starts")
                display = os.read(read_fd, 64).decode().strip()
                os.close(read_fd)
                require(display.isdigit(), "virtual display reports a valid number")
                env.update(DISPLAY=":" + display, SDL_VIDEODRIVER="x11", SDL_RENDER_DRIVER="software",
                           SDL_AUDIODRIVER="dummy", XDG_DATA_HOME=str(work / "data"), OMP_THREAD_LIMIT="1")

                def command(*arguments, check=True):
                    return subprocess.run([args.xdotool, *arguments], env=env, check=check,
                                          capture_output=True, timeout=5)

                def launch(label):
                    nonlocal process
                    handle = open(output / (label + ".log"), "wb")
                    process = subprocess.Popen([game, "--seed", "7"], cwd=work, env=env,
                                               stdout=handle, stderr=subprocess.STDOUT)
                    handle.close()
                    deadline = time.monotonic() + 8
                    while time.monotonic() < deadline:
                        found = command("search", "--onlyvisible", "--pid", str(process.pid),
                                        "--name", "Xfrog 2.0", check=False)
                        if found.returncode == 0 and found.stdout.strip():
                            window = found.stdout.splitlines()[0].decode()
                            command("windowsize", "--sync", window, "1920", "1080")
                            command("windowmove", "--sync", window, "0", "0")
                            command("windowfocus", "--sync", window)
                            time.sleep(.2)
                            return window
                        require(process.poll() is None, "game survives startup")
                        time.sleep(.05)
                    raise AssertionError("HD game window did not appear")

                def key(name, duration=.09):
                    command("keydown", name)
                    time.sleep(duration)
                    command("keyup", name)

                def click(x, y):
                    command("mousemove", "--sync", str(x), str(y))
                    command("click", "1")

                def snapshot(name):
                    path = output / (name + ".png")
                    subprocess.run([args.capture, "-window", window, str(path)], env=env, check=True,
                                   capture_output=True, timeout=5)
                    # Tesseract is independent of the renderer/font implementation.
                    result = subprocess.run([args.ocr, str(path), "stdout", "--psm", "11"],
                                            env=env, check=True, capture_output=True, timeout=8)
                    text = result.stdout.decode().upper()
                    (output / (name + ".txt")).write_text(text)
                    return text

                def screen(name, phrase, extra=()):
                    deadline = time.monotonic() + 7
                    while time.monotonic() < deadline:
                        text = snapshot(name)
                        if phrase in text and all(part in text for part in extra):
                            require(True, name + " / " + phrase)
                            return text
                        require(process.poll() is None, "game survives while awaiting " + name)
                    raise AssertionError("screen did not show " + phrase)

                window = launch("first-run")
                if args.delivery:
                    screen("01-menu", "CHOOSE YOUR PACE")
                    key("Return")
                    key("space")
                    # Ordinary OS input navigates around the physical pizzeria,
                    # across live traffic and into seed 7's first delivery zone.
                    key("Left", 1.15)
                    key("Up", 2.4)
                    key("space")
                    text = screen("02-delivered", "DELIVERIES")
                    require(re.search(r"1\s*/\s*6\s+DELIVERIES", text), "ordinary keyboard route completes a real delivery")
                    key("Escape")
                    screen("03-paused", "TAKE A BREATHER")
                    click(960, 690)
                    screen("04-menu", "CHOOSE YOUR PACE")
                    key("Escape")
                    require(process.wait(timeout=5) == 0, "delivery run exits cleanly")
                    profile = next((work / "data").rglob("profile.dat")).read_text()
                    values = dict(line.split() for line in profile.splitlines())
                    require(int(values["best"]) >= 100 and int(values["deliveries"]) == 1,
                            "earned score and completed delivery persist to disk")
                    (output / "report.json").write_text(json.dumps({"status": "passed", "checks": ["normal executable", "OS keyboard navigation", "live traffic", "real pizza delivery", "earned progress save"]}, indent=2) + "\n")
                    return
                menu = screen("01-menu", "CHOOSE YOUR PACE", ("HOW TO PLAY", "RELAXED", "ARCADE"))
                require("HOW TO PLAY" in menu and "RELAXED" in menu and "ARCADE" in menu,
                        "menu exposes help and difficulty choices")
                key("h")
                screen("02-help", "YOUR NEXT GREAT DELIVERY")
                key("Return")
                screen("03-menu", "CHOOSE YOUR PACE")
                key("Right")
                screen("04-arcade", "BIGGER CHALLENGE")
                key("Left")
                key("Return")
                screen("05-gameplay", "FRESH PIZZA")
                key("space")
                order = screen("06-order", "PIZZA HEAT")
                require(any(name in order for name in ("SUNRISE BAKERY", "THE GREEN GROCER", "HOTEL AZURE", "THE WORKSHOP", "CORAL CAFE")),
                        "pickup assigns a visible named destination")
                def player_pixels():
                    return subprocess.check_output([args.capture, "-window", window, "-crop", "360x150+870+900", "-depth", "8", "rgb:-"], env=env, timeout=5)
                before_pixels = player_pixels()
                key("Right", .45)
                screen("07-moved", "PIZZA HEAT")
                after_pixels = player_pixels()
                require(len(before_pixels) == len(after_pixels) and sum(a != b for a, b in zip(before_pixels, after_pixels)) > 2400,
                        "OS movement changes the rendered courier position")
                # Pause freezes the real process's simulation; resume and help use
                # normal keyboard/mouse handlers, with no test-time clock hooks.
                key("Escape")
                screen("08-paused", "TAKE A BREATHER")
                key("h")
                screen("09-paused-help", "YOUR NEXT GREAT DELIVERY")
                key("Return")
                screen("10-return-to-pause", "TAKE A BREATHER")
                click(960, 485)
                screen("11-resumed", "PIZZA HEAT")
                # Removing focus must pause rather than silently letting traffic hit.
                command("windowunmap", window)
                time.sleep(.15)
                command("windowmap", "--sync", window)
                command("windowfocus", "--sync", window)
                screen("12-focus-paused", "TAKE A BREATHER")
                click(960, 690)
                screen("13-menu", "CHOOSE YOUR PACE")
                key("m")
                screen("14-sound-off", "SOUND OFF")
                key("Right")
                key("Escape")
                require(process.wait(timeout=5) == 0, "normal process exits cleanly")
                profiles = list((work / "data").rglob("profile.dat"))
                require(len(profiles) == 1, "profile is separate from game installation")
                content = profiles[0].read_text()
                require("sound 0" in content and "difficulty 1" in content, "settings save to disk")
                # Seed a prior record, then observe it through the ordinary menu.
                profiles[0].write_text(content.replace("best 0", "best 321"))
                window = launch("second-run")
                text = screen("15-restored", "SOUND OFF", ("321", "BIGGER CHALLENGE"))
                require("321" in text and "BIGGER CHALLENGE" in text, "record and difficulty reload in a new process")
                key("Escape")
                require(process.wait(timeout=5) == 0, "relaunched process exits cleanly")
                (output / "report.json").write_text(json.dumps({"status": "passed", "executable": game,
                    "resolution": "1920x1080", "checks": ["real OS input", "HD menu", "help", "difficulty",
                    "pizza pickup", "delivery marker", "movement", "pause/resume", "focus pause",
                    "mouse UI", "sound toggle", "settings and record persistence", "clean restart/exit"]}, indent=2) + "\n")
            except BaseException as error:
                (output / "report.json").write_text(json.dumps({"status": "failed", "error": str(error)}, indent=2) + "\n")
                raise
            finally:
                if process is not None and process.poll() is None:
                    process.terminate()
                    try:
                        process.wait(timeout=3)
                    except subprocess.TimeoutExpired:
                        process.kill()
                        process.wait(timeout=3)
                if server is not None:
                    server.terminate()
                    server.wait(timeout=5)


if __name__ == "__main__":
    main()
