"""Validate checked-in full-color art, alpha atlases, font files and licenses."""
import hashlib
import json
from pathlib import Path
import struct
import sys
import zlib
import wave
root = Path(sys.argv[1])
manifest = json.loads((root / "manifest.json").read_text())
for name, expected in manifest["files"].items():
    data = (root / name).read_bytes()
    assert hashlib.sha256(data).hexdigest() == expected["sha256"], name + " checksum differs"
    if name.endswith(".png"):
        assert data[:8] == b"\x89PNG\r\n\x1a\n"
        width, height, depth, color = struct.unpack_from(">IIBB", data, 16)
        assert width >= (1600 if name == "city.png" else 1024) and height >= 800 and depth == 8
        assert width == expected["width"] and height == expected["height"]
        assert color in (2, 6), "indexed art is not permitted in HD assets"
        if name != "city.png":
            assert color == 6, "sprite atlas must preserve alpha"
        offset = 8
        while offset < len(data):
            length = struct.unpack_from(">I", data, offset)[0]
            chunk = data[offset + 4:offset + 8 + length]
            crc = struct.unpack_from(">I", data, offset + 8 + length)[0]
            assert zlib.crc32(chunk) & 0xffffffff == crc, name + " corrupt PNG chunk"
            offset += length + 12
    elif name.endswith(".wav"):
        with wave.open(str(root/name)) as score:
            assert score.getnchannels() == 2 and score.getsampwidth() == 2
            assert score.getframerate() == 32000
            assert score.getnframes()/score.getframerate() > 180
    elif name.endswith(".motion"):
        assert data[:8] == b"XFMOTION"
        assert struct.unpack_from("<5I", data, 8) == (64, 6, 3, 200, 230)
        assert len(data) == 28 + 3 * 6 * 2 * 64 * 64 * 4
        source = name.replace(".motion", ".png")
        assert expected["source_sha256"] == hashlib.sha256((root/source).read_bytes()).hexdigest(), "motion data belongs to different artwork"
        samples = struct.unpack_from("<" + "h"*((len(data)-28)//2), data, 28)
        assert max(samples) <= 96*256 and min(samples) >= -96*256
        assert any(abs(value) > 256 for value in samples), "missing motion vectors"
    elif name.endswith(".ttf"):
        assert data[:4] in (b"\x00\x01\x00\x00", b"OTTO"), "invalid font"
assert manifest["courier_grid"] == [8, 4] and manifest["traffic_grid"] == [4, 2]
assert manifest["poses_grid"] == [4, 3]
assert manifest["carry_walk_grid"] == [8, 4] and manifest["carry_poses_grid"] == [4, 2]
assert manifest["thief_grid"] == [2, 2]
assert manifest["run_inbetweens_grid"] == [4, 4]
assert manifest["carry_inbetweens_grid"] == [4, 4]
assert manifest["thief_run_grid"] == [4, 4]
print("PASS: RGB city, original RGBA courier and matching motion data, eight RGBA vehicles, fonts and license checksums")
