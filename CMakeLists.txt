cmake_minimum_required(VERSION 3.16)
project(DisasterRescueCommander VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_GUI "Build SFML graphical interface" ON)
option(BUILD_TESTS "Build unit tests" ON)

# Core engine library (no SFML dependency - fully testable headless).
add_library(DisasterCore STATIC
    src/map/DisasterMap.cpp
    src/vehicles/Vehicle.cpp
    src/algorithms/Dijkstra.cpp
    src/algorithms/RouteStrategy.cpp
    src/managers/GameEngine.cpp
    src/managers/SaveManager.cpp
    src/ui/AudioManager.cpp
)
target_include_directories(DisasterCore PUBLIC include)

# Optional SFML GUI layer.
set(DRC_HAS_SFML OFF)
if(BUILD_GUI)
    find_package(SFML 2.5 COMPONENTS graphics window system audio QUIET)
    if(SFML_FOUND)
        set(DRC_HAS_SFML ON)
        message(STATUS "SFML found - building graphical interface")
    else()
        message(WARNING "SFML not found - building headless simulation mode. Install SFML for the GUI.")
    endif()
endif()

if(DRC_HAS_SFML)
    add_executable(DisasterRescueCommander
        src/main.cpp
        src/ui/GameRenderer.cpp
        src/ui/MapRenderer.cpp
        src/ui/Button.cpp
        src/ui/Panel.cpp
        src/ui/ParticleSystem.cpp
    )
    target_compile_definitions(DisasterRescueCommander PRIVATE DRC_HAS_SFML)
    target_link_libraries(DisasterRescueCommander PRIVATE DisasterCore sfml-graphics sfml-window sfml-system sfml-audio)
else()
    add_executable(DisasterRescueCommander src/main.cpp)
    target_link_libraries(DisasterRescueCommander PRIVATE DisasterCore)
endif()

if(BUILD_TESTS)
    enable_testing()
    add_executable(DisasterTests tests/test_main.cpp)
    target_link_libraries(DisasterTests PRIVATE DisasterCore)
    add_test(NAME DisasterTests COMMAND DisasterTests)
endif()

# Copy runtime assets next to the executable.
add_custom_command(TARGET DisasterRescueCommander POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/assets ${CMAKE_BINARY_DIR}/assets
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/config ${CMAKE_BINARY_DIR}/config
    COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/saves
    COMMENT "Copying assets/config to build directory")
