file(COPY_FILE "${SOURCE_DIR}/bin/xfrog.cfs" "${OUTPUT_DIR}/xfrog.cfs" ONLY_IF_DIFFERENT)
# Seed a new build directory without resetting the player's saved score.
if(NOT EXISTS "${OUTPUT_DIR}/hs.dat")
    file(COPY_FILE "${SOURCE_DIR}/bin/hs.dat" "${OUTPUT_DIR}/hs.dat")
endif()
file(COPY_FILE "${SOURCE_DIR}/docs/gnu.txt" "${OUTPUT_DIR}/COPYING.txt" ONLY_IF_DIFFERENT)
file(COPY_FILE "${ZLIB_SOURCE_DIR}/LICENSE" "${OUTPUT_DIR}/zlib-LICENSE.txt" ONLY_IF_DIFFERENT)
