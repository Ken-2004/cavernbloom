cmake_minimum_required(VERSION 3.25)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Use an out-of-source build: cmake -S . -B build")
endif()

project(CavernBloom VERSION 0.1.0 LANGUAGES C CXX)

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

find_package(Python 3 COMPONENTS Interpreter REQUIRED)
execute_process(COMMAND "${Python_EXECUTABLE}" -c "import jinja2"
    RESULT_VARIABLE CAVERNBLOOM_JINJA_RESULT OUTPUT_QUIET ERROR_QUIET)
if(NOT CAVERNBLOOM_JINJA_RESULT EQUAL 0)
    message(FATAL_ERROR
        "GLAD requires Jinja2. Install it with: \"${Python_EXECUTABLE}\" -m pip install Jinja2==3.1.6")
endif()

FetchContent_MakeAvailable(glfw glad glm)
glad_add_library(cavernbloom_glad STATIC REPRODUCIBLE API gl:core=3.3 EXTENSIONS NONE)

add_executable(CavernBloom
    src/main.cpp src/Game.cpp src/Renderer.cpp src/Shader.cpp src/Player.cpp
    include/Game.hpp include/Renderer.hpp include/Shader.hpp include/Player.hpp
    include/SimulationConfig.hpp
)
target_compile_features(CavernBloom PRIVATE cxx_std_20)
set_target_properties(CavernBloom PROPERTIES CXX_EXTENSIONS OFF)
target_include_directories(CavernBloom PRIVATE include)
target_link_libraries(CavernBloom PRIVATE glfw cavernbloom_glad glm::glm)
target_compile_definitions(CavernBloom PRIVATE
    GLFW_INCLUDE_NONE
    CAVERNBLOOM_SHADER_DIR="${CMAKE_CURRENT_BINARY_DIR}/shaders"
)
if(MSVC)
    target_compile_options(CavernBloom PRIVATE /W4 /permissive- /Zc:__cplusplus)
else()
    target_compile_options(CavernBloom PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
endif()

# configure_file also causes shader edits to be copied on the next build.
foreach(shader basic.vert basic.frag)
    configure_file("shaders/${shader}" "shaders/${shader}" COPYONLY)
endforeach()
set_property(DIRECTORY PROPERTY VS_STARTUP_PROJECT CavernBloom)

include(CTest)
if(BUILD_TESTING)
    add_executable(CavernBloomPlayerTests tests/PlayerTests.cpp src/Player.cpp)
    target_compile_features(CavernBloomPlayerTests PRIVATE cxx_std_20)
    set_target_properties(CavernBloomPlayerTests PROPERTIES CXX_EXTENSIONS OFF)
    target_include_directories(CavernBloomPlayerTests PRIVATE include)
    target_link_libraries(CavernBloomPlayerTests PRIVATE glm::glm)
    if(MSVC)
        target_compile_options(CavernBloomPlayerTests PRIVATE /W4 /permissive- /Zc:__cplusplus)
    else()
        target_compile_options(CavernBloomPlayerTests PRIVATE -Wall -Wextra -Wpedantic -Wconversion)
    endif()
    add_test(NAME PlayerPhysics COMMAND CavernBloomPlayerTests)
endif()
