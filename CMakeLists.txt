cmake_minimum_required(VERSION 3.25)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Use an out-of-source build: cmake -S . -B build")
endif()

project(CavernBloom VERSION 0.1.0 LANGUAGES C CXX)

# A Windows ZIP must not require a separately installed MSVC runtime.
if(MSVC)
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
    set(USE_MSVC_RUNTIME_LIBRARY_DLL OFF CACHE BOOL "Use the static runtime in GLFW" FORCE)
    # Keep linker debug metadata out of the distributed Release executable.
    add_link_options("$<$<CONFIG:Release>:/DEBUG:NONE>")
endif()

include(FetchContent)
set(BUILD_SHARED_LIBS OFF CACHE BOOL "Build static dependencies")
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
    SYSTEM
)
FetchContent_Declare(glad
    GIT_REPOSITORY https://github.com/Dav1dde/glad.git
    GIT_TAG v2.0.8
    GIT_SHALLOW TRUE
    SOURCE_SUBDIR cmake
    SYSTEM
)
FetchContent_Declare(glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
    SYSTEM
)
FetchContent_Declare(miniaudio
    GIT_REPOSITORY https://github.com/mackron/miniaudio.git
    GIT_TAG 0.11.23
    GIT_SHALLOW TRUE
    GIT_SUBMODULES ""
    # Use our Ogg-enabled implementation target, not upstream's optional tools.
    SOURCE_SUBDIR cavernbloom-unused
    SYSTEM
)

find_package(Python 3 COMPONENTS Interpreter REQUIRED)
execute_process(COMMAND "${Python_EXECUTABLE}" -c "import jinja2"
    RESULT_VARIABLE CAVERNBLOOM_JINJA_RESULT OUTPUT_QUIET ERROR_QUIET)
if(NOT CAVERNBLOOM_JINJA_RESULT EQUAL 0)
    message(FATAL_ERROR
        "GLAD requires Jinja2. Install it with: \"${Python_EXECUTABLE}\" -m pip install Jinja2==3.1.6")
endif()

FetchContent_MakeAvailable(glfw glad glm miniaudio)
glad_add_library(cavernbloom_glad STATIC REPRODUCIBLE API gl:core=3.3 EXTENSIONS NONE)

find_package(Threads REQUIRED)
add_library(cavernbloom_audio_backend STATIC src/MiniaudioBackend.c)
target_include_directories(cavernbloom_audio_backend SYSTEM PUBLIC "${miniaudio_SOURCE_DIR}")
target_compile_definitions(cavernbloom_audio_backend PUBLIC MA_NO_NULL)
target_link_libraries(cavernbloom_audio_backend PRIVATE Threads::Threads ${CMAKE_DL_LIBS})
if(MSVC)
    target_compile_definitions(cavernbloom_audio_backend PRIVATE _CRT_SECURE_NO_WARNINGS)
elseif(UNIX AND NOT APPLE)
    target_link_libraries(cavernbloom_audio_backend PRIVATE m)
endif()

add_executable(CavernBloom
    src/main.cpp src/Game.cpp src/Renderer.cpp src/Shader.cpp src/Player.cpp src/Collision.cpp
    src/JumpReachability.cpp src/LevelGenerator.cpp src/Camera2D.cpp src/Progression.cpp
    include/Game.hpp include/Renderer.hpp include/Shader.hpp include/Player.hpp
    include/SimulationConfig.hpp include/Platform.hpp include/Collision.hpp
    include/JumpReachability.hpp include/LevelGenerator.hpp include/Camera2D.hpp include/Bounds2D.hpp
    src/EnemySystem.cpp src/Gameplay.cpp
    include/Enemy.hpp include/EnemySystem.hpp include/Hazard.hpp include/EncounterConfig.hpp include/Gameplay.hpp
    src/GamePresentation.cpp src/PresentationMath.cpp
    include/GamePresentation.hpp include/VisualTheme.hpp include/PresentationMath.hpp
    include/Collectible.hpp include/Progression.hpp
    src/AudioSystem.cpp src/AudioCues.cpp include/AudioSystem.hpp include/AudioCues.hpp
    src/RuntimePaths.cpp include/RuntimePaths.hpp
)
target_compile_features(CavernBloom PRIVATE cxx_std_20)
set_target_properties(CavernBloom PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(CavernBloom PRIVATE include)
target_link_libraries(CavernBloom PRIVATE glfw cavernbloom_glad glm::glm cavernbloom_audio_backend)
target_compile_definitions(CavernBloom PRIVATE GLFW_INCLUDE_NONE)
if(MSVC)
    target_compile_options(CavernBloom PRIVATE /W4 /permissive- /Zc:__cplusplus)
else()
    target_compile_options(CavernBloom PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

# Explicit lists also define the complete install/package manifest.
set(CAVERNBLOOM_SHADERS shaders/basic.vert shaders/basic.frag)
set(CAVERNBLOOM_AUDIO assets/audio/jump.ogg assets/audio/collect.ogg
    assets/audio/damage.ogg assets/audio/win.ogg)
add_custom_target(cavernbloom_resources ALL
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CavernBloom>/shaders"
    COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CavernBloom>/assets/audio"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CAVERNBLOOM_SHADERS} "$<TARGET_FILE_DIR:CavernBloom>/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${CAVERNBLOOM_AUDIO} "$<TARGET_FILE_DIR:CavernBloom>/assets/audio"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different THIRD_PARTY_ASSETS.md "$<TARGET_FILE_DIR:CavernBloom>"
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}"
    COMMENT "Copying runtime resources beside CavernBloom"
    VERBATIM
)
add_dependencies(CavernBloom cavernbloom_resources)

install(TARGETS CavernBloom RUNTIME DESTINATION .)
install(FILES ${CAVERNBLOOM_SHADERS} DESTINATION shaders)
install(FILES ${CAVERNBLOOM_AUDIO} DESTINATION assets/audio)
install(FILES THIRD_PARTY_ASSETS.md DESTINATION .)

set(CPACK_GENERATOR ZIP)
set(CPACK_PACKAGE_NAME CavernBloom)
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_DIRECTORY "${CMAKE_BINARY_DIR}/package")
set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY ON)
if(WIN32)
    if(CMAKE_GENERATOR_PLATFORM MATCHES "[Aa][Rr][Mm]64" OR CMAKE_SYSTEM_PROCESSOR MATCHES "[Aa][Rr][Mm]64")
        set(CAVERNBLOOM_PACKAGE_ARCH arm64)
    elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
        set(CAVERNBLOOM_PACKAGE_ARCH x64)
    else()
        set(CAVERNBLOOM_PACKAGE_ARCH x86)
    endif()
    set(CPACK_PACKAGE_FILE_NAME "CavernBloom-${PROJECT_VERSION}-Windows-${CAVERNBLOOM_PACKAGE_ARCH}")
endif()
include(CPack)
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT CavernBloom)

include(CTest)
if(BUILD_TESTING)
    add_executable(CavernBloomPlayerTests tests/PlayerTests.cpp src/Player.cpp src/Collision.cpp)
    add_executable(CavernBloomCollisionTests tests/CollisionTests.cpp src/Player.cpp src/Collision.cpp)
    add_executable(CavernBloomJumpReachabilityTests tests/JumpReachabilityTests.cpp src/JumpReachability.cpp)
    add_executable(CavernBloomLevelGeneratorTests tests/LevelGeneratorTests.cpp
        src/LevelGenerator.cpp src/JumpReachability.cpp src/Player.cpp src/Collision.cpp src/EnemySystem.cpp)
    add_executable(CavernBloomCameraTests tests/CameraTests.cpp src/Camera2D.cpp
        src/Player.cpp src/Collision.cpp)
    add_executable(CavernBloomCollectibleTests tests/CollectibleTests.cpp src/Progression.cpp src/Collision.cpp)
    add_executable(CavernBloomProgressionTests tests/ProgressionTests.cpp src/Progression.cpp src/Collision.cpp
        src/LevelGenerator.cpp src/JumpReachability.cpp)
    add_executable(CavernBloomEnemyTests tests/EnemyTests.cpp src/EnemySystem.cpp src/Camera2D.cpp)
    add_executable(CavernBloomHazardTests tests/HazardTests.cpp src/Gameplay.cpp src/EnemySystem.cpp
        src/Player.cpp src/Collision.cpp src/Progression.cpp src/LevelGenerator.cpp src/JumpReachability.cpp)
    add_executable(CavernBloomPresentationTests tests/PresentationTests.cpp src/PresentationMath.cpp)
    add_executable(CavernBloomAudioCueTests tests/AudioCueTests.cpp src/AudioCues.cpp src/Gameplay.cpp
        src/EnemySystem.cpp src/Player.cpp src/Collision.cpp src/Progression.cpp)
    foreach(suite Player Collision JumpReachability LevelGenerator Camera Collectible Progression Enemy Hazard Presentation AudioCue)
        set(test_target "CavernBloom${suite}Tests")
        target_compile_features(${test_target} PRIVATE cxx_std_20)
        set_target_properties(${test_target} PROPERTIES CXX_EXTENSIONS OFF)
        target_include_directories(${test_target} PRIVATE include)
        target_link_libraries(${test_target} PRIVATE glm::glm)
        if(MSVC)
            target_compile_options(${test_target} PRIVATE /W4 /permissive- /Zc:__cplusplus)
        else()
            target_compile_options(${test_target} PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
        endif()
    endforeach()
    add_test(NAME PlayerPhysics COMMAND CavernBloomPlayerTests)
    add_test(NAME StaticPlatformCollision COMMAND CavernBloomCollisionTests)
    add_test(NAME JumpReachability COMMAND CavernBloomJumpReachabilityTests)
    add_test(NAME SeededLevelGeneration COMMAND CavernBloomLevelGeneratorTests)
    add_test(NAME Camera2D COMMAND CavernBloomCameraTests)
    add_test(NAME Collectibles COMMAND CavernBloomCollectibleTests)
    add_test(NAME Progression COMMAND CavernBloomProgressionTests)
    add_test(NAME EnemyPatrol COMMAND CavernBloomEnemyTests)
    add_test(NAME HazardContact COMMAND CavernBloomHazardTests)
    add_test(NAME PresentationLayout COMMAND CavernBloomPresentationTests)
    add_test(NAME AudioCueMapping COMMAND CavernBloomAudioCueTests)
endif()
