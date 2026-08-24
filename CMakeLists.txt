cmake_minimum_required(VERSION 3.20)

project(
    UltraEdgeX
    VERSION 0.1.0
    LANGUAGES CXX
)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(
    UltraEdgeX
    WIN32

    src/app/Main.cpp
    src/app/Application.cpp

    src/roui/core/UIElement.cpp
    src/roui/controls/Text.cpp
    src/roui/controls/Panel.cpp
    src/roui/controls/Button.cpp
    src/roui/controls/Card.cpp
    src/roui/layout/StackPanel.cpp
    src/features/dashboard/DashboardPage.cpp
    src/roui/window/Window.cpp
    src/roui/rendering/Renderer.cpp
    src/system/SystemInfo.cpp
    src/system/SystemAnalyzer.cpp
    src/system/TemporaryFilesScanner.cpp
    src/system/TemporaryFilesCleaner.cpp
    src/system/StartupManager.cpp
)

target_include_directories(
    UltraEdgeX
    PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)

target_link_libraries(
    UltraEdgeX
    PRIVATE
    d2d1
    dwrite
)

if(MSVC)

    target_compile_options(
        UltraEdgeX
        PRIVATE
        /W4
        /permissive-
        /EHsc
        /utf-8
    )

    target_compile_definitions(
        UltraEdgeX
        PRIVATE
        UNICODE
        _UNICODE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )

else()

    target_compile_options(
        UltraEdgeX
        PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )

    target_compile_definitions(
        UltraEdgeX
        PRIVATE
        UNICODE
        _UNICODE
        NOMINMAX
        WIN32_LEAN_AND_MEAN
    )

endif()