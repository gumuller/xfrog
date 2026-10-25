#!/usr/bin/env bash
set -euo pipefail
cd -- "$(dirname -- "${BASH_SOURCE[0]}")"
cmake --preset linux >&2
cmake --build --preset linux >&2
exec ./build/linux/bin/xfrog "$@"
