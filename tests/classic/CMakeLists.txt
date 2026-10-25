find_package(Python3 COMPONENTS Interpreter REQUIRED)

# Gameplay fixtures use deterministic time and a separate score directory.
set(TEST_CORE_SOURCES ${GAME_COMMON} ${GAME_SDL})

add_library(xfrog-classic-test-core STATIC ${TEST_CORE_SOURCES})
target_compile_features(xfrog-classic-test-core PUBLIC cxx_std_11)
target_compile_definitions(xfrog-classic-test-core PUBLIC XFROG_SDL XFROG_TESTING)
target_include_directories(xfrog-classic-test-core PUBLIC "${GAME_SRC}")
target_compile_options(xfrog-classic-test-core PRIVATE -Wno-write-strings)
target_link_libraries(xfrog-classic-test-core PUBLIC SDL3::SDL3 ${XFROG_ZLIB})
add_executable(xfrog-classic-gameplay-test gameplay.cpp)
target_link_libraries(xfrog-classic-gameplay-test PRIVATE xfrog-classic-test-core)
set_target_properties(xfrog-classic-gameplay-test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
add_dependencies(xfrog-classic-gameplay-test xfrog-classic)
add_test(NAME linux-menu-smoke COMMAND xfrog-classic --smoke-test)
add_test(NAME linux-gameplay COMMAND xfrog-classic-gameplay-test)
set_tests_properties(linux-menu-smoke linux-gameplay PROPERTIES
    TIMEOUT 45 LABELS integration
    ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy;XDG_DATA_HOME=${CMAKE_BINARY_DIR}/test-data"
    RUN_SERIAL TRUE)

file(GLOB_RECURSE ASSET_FILES RELATIVE "${PROJECT_SOURCE_DIR}/src/game/gamedata"
    "${PROJECT_SOURCE_DIR}/src/game/gamedata/*.tga" "${PROJECT_SOURCE_DIR}/src/game/gamedata/*.wav")
string(REPLACE ";" "\n" ASSET_MANIFEST "${ASSET_FILES}")
file(WRITE "${CMAKE_BINARY_DIR}/assets.txt" "${ASSET_MANIFEST}\n")
add_executable(xfrog-classic-asset-test assets.cpp)
target_compile_definitions(xfrog-classic-asset-test PRIVATE
    XFROG_ASSET_MANIFEST="${CMAKE_BINARY_DIR}/assets.txt"
    XFROG_ASSET_ROOT="${PROJECT_SOURCE_DIR}/src/game/gamedata")
target_link_libraries(xfrog-classic-asset-test PRIVATE xfrog-classic-core)
add_test(NAME original-assets COMMAND xfrog-classic-asset-test "${PROJECT_SOURCE_DIR}/bin/xfrog.cfs")
add_test(NAME malformed-packs COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/invalid_packs.py"
    "$<TARGET_FILE:xfrog-classic-asset-test>" "${PROJECT_SOURCE_DIR}/bin/xfrog.cfs")
set_tests_properties(original-assets malformed-packs PROPERTIES LABELS integration TIMEOUT 20)

add_executable(xfrog-classic-unit-test unit.cpp)
target_link_libraries(xfrog-classic-unit-test PRIVATE xfrog-classic-core)
foreach(SUITE input surfaces timer cfs audio)
    add_test(NAME unit-${SUITE} COMMAND xfrog-classic-unit-test ${SUITE} "${PROJECT_SOURCE_DIR}/bin/xfrog.cfs" "${CMAKE_CURRENT_BINARY_DIR}")
    set_tests_properties(unit-${SUITE} PROPERTIES LABELS unit TIMEOUT 15
        ENVIRONMENT "SDL_VIDEODRIVER=dummy;SDL_AUDIODRIVER=dummy")
endforeach()

option(XFROG_ENABLE_E2E "Run normal executable through an isolated X11 display" OFF)
if(XFROG_ENABLE_E2E)
    find_program(XFROG_XVFB Xvfb REQUIRED)
    find_program(XFROG_XDOTOOL xdotool REQUIRED)
    find_program(XFROG_CAPTURE import REQUIRED)
    add_test(NAME e2e-linux-executable COMMAND "${Python3_EXECUTABLE}" "${CMAKE_CURRENT_SOURCE_DIR}/e2e.py"
        --game "$<TARGET_FILE:xfrog-classic>" --assets "${PROJECT_SOURCE_DIR}/src/game/gamedata"
        --xvfb "${XFROG_XVFB}" --xdotool "${XFROG_XDOTOOL}" --capture "${XFROG_CAPTURE}"
        --artifacts "${CMAKE_BINARY_DIR}/e2e-artifacts")
    set_tests_properties(e2e-linux-executable PROPERTIES LABELS e2e TIMEOUT 60 RUN_SERIAL TRUE)
endif()
