"""Check native CFS rejection of malformed headers and records."""
import pathlib
import struct
import subprocess
import sys
import tempfile

executable, original = sys.argv[1:]
source = pathlib.Path(original).read_bytes()
with tempfile.TemporaryDirectory(prefix="xfrog-cfs-") as directory:
    cases = {"truncated": source[:10], "bad-id": b"BAD!" + source[4:]}
    def changed(offset, value):
        data = bytearray(source)
        struct.pack_into("<I", data, offset, value)
        return data
    cases["file-count"] = changed(4, 0xFFFFFFFF)
    cases["record-offset"] = changed(12 + 64, 0xFFFFFFFF)
    cases["compressed-size"] = changed(12 + 68, 0xFFFFFFFF)
    cases["expanded-size"] = changed(12 + 72, 0xFFFFFFFF)
    for name, data in cases.items():
        path = pathlib.Path(directory) / (name + ".cfs")
        path.write_bytes(data)
        subprocess.run([executable, str(path), "--invalid"], check=True)
