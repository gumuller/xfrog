# Vendored dependencies

- `stb_image.h` and `stb_truetype.h`: upstream
  <https://github.com/nothings/stb>, copied from its master branch on 2026-10-25.
  The copies are checked into Git and do not change during builds. License:
  `stb-LICENSE.txt`. They decode PNGs and bake TrueType glyph atlases locally.
- SDL 3.4.18: Windows source builds use the official release tarball, pinned by
  SHA-256 in CMakeLists.txt. `SDL-LICENSE.txt` is copied from that release.
  Linux normally uses its installed SDL3 package.

Font licenses are in `assets/v2/`. Generated artwork and its prompt set are
recorded in `docs/v2/art-direction.md`.
