cmake_minimum_required(VERSION 3.10)
project(CameraViewerApp)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Options
option(ENABLE_FFMPEG "Build with FFmpeg video decoding" OFF)

# Default to a local 'ffmpeg' folder (Windows); can be overridden with -DFFMPEG_ROOT=...
set(FFMPEG_ROOT "${CMAKE_SOURCE_DIR}/ffmpeg" CACHE PATH "Path to FFmpeg install prefix (bin/include/lib layout)")

# Camera SDK
if(NOT DEFINED CAMERA_SDK_PATH)
    message(FATAL_ERROR "ERROR: CAMERA_SDK_PATH is not set! Please provide it when running CMake.")
endif()
find_package(CameraLibrary REQUIRED HINTS ${CAMERA_SDK_PATH})

# Qt & OpenGL
find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGLWidgets)
find_package(OpenGL REQUIRED)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Sources
set(APP_SOURCES
    main.cpp
    BitmapPool.cpp
    QtCameraViewer.cpp
    QtCameraPicker.cpp
    QtCameraConnectionManager.cpp
    QtVideoWidget.cpp
    QtCameraControlPanel.cpp
)

# Only compile the decoder if FFmpeg is enabled
if(ENABLE_FFMPEG)
    list(APPEND APP_SOURCES videodecoder.cpp)
    add_compile_definitions(HAVE_FFMPEG=1)
endif()

qt6_wrap_cpp(MOC_SOURCES QtCameraViewer.h QtCameraConnectionManager.h QtCameraPicker.h QtVideoWidget.h QtCameraControlPanel.h)
add_executable(CameraViewerApp ${APP_SOURCES} ${MOC_SOURCES})

# Link basics
target_link_libraries(CameraViewerApp PRIVATE Qt6::Core Qt6::Gui Qt6::OpenGLWidgets OpenGL::GL)

# FFmpeg if included
if(ENABLE_FFMPEG)
    if(WIN32)
        set(FFMPEG_INCLUDE_DIR "${FFMPEG_ROOT}/include")

        set(_ff_lib_candidates
            "${FFMPEG_ROOT}/lib/x64"
            "${FFMPEG_ROOT}/lib/win64"
            "${FFMPEG_ROOT}/lib/amd64"
            "${FFMPEG_ROOT}/lib"
        )
        set(FFMPEG_LIB_DIR "")
        foreach(_cand IN LISTS _ff_lib_candidates)
            if(EXISTS "${_cand}")
                set(FFMPEG_LIB_DIR "${_cand}")
                break()
            endif()
        endforeach()

        if(NOT EXISTS "${FFMPEG_INCLUDE_DIR}")
            message(FATAL_ERROR "FFmpeg include dir not found: ${FFMPEG_INCLUDE_DIR}. Set FFMPEG_ROOT.")
        endif()
        if(NOT FFMPEG_LIB_DIR)
            message(FATAL_ERROR "FFmpeg lib dir not found under ${FFMPEG_ROOT} (tried lib/x64, lib/win64, lib/amd64, lib). Set FFMPEG_ROOT.")
        endif()

        find_library(AVCODEC_LIB
            NAMES avcodec avcodec-61 avcodec-60 avcodec-59
            HINTS "${FFMPEG_LIB_DIR}"
        )
        find_library(AVUTIL_LIB
            NAMES avutil avutil-59 avutil-58
            HINTS "${FFMPEG_LIB_DIR}"
        )
        find_library(SWSCALE_LIB
            NAMES swscale swscale-8 swscale-7
            HINTS "${FFMPEG_LIB_DIR}"
        )

        if(NOT AVCODEC_LIB OR NOT AVUTIL_LIB OR NOT SWSCALE_LIB)
            message(FATAL_ERROR
                "Could not find FFmpeg import libs in ${FFMPEG_LIB_DIR}.\n"
                "Expected avcodec(.lib), avutil(.lib), swscale(.lib) (versioned names also supported)."
            )
        endif()

        target_include_directories(CameraViewerApp PRIVATE "${FFMPEG_INCLUDE_DIR}")
        target_link_libraries(CameraViewerApp PRIVATE ${AVCODEC_LIB} ${AVUTIL_LIB} ${SWSCALE_LIB})

        set(_ff_bin_candidates
            "${FFMPEG_ROOT}/bin/x64"
            "${FFMPEG_ROOT}/bin"
        )
        foreach(_bindir IN LISTS _ff_bin_candidates)
            if(EXISTS "${_bindir}")
                add_custom_command(
                    TARGET CameraViewerApp POST_BUILD
                    COMMAND ${CMAKE_COMMAND} -E echo "Copying FFmpeg DLLs from ${_bindir}"
                    COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "${_bindir}"
                        "$<TARGET_FILE_DIR:CameraViewerApp>"
                )
                break()
            endif()
        endforeach()

    elseif(UNIX)
        find_package(PkgConfig QUIET)
        if(PkgConfig_FOUND)
            pkg_check_modules(FFMPEG_PKG QUIET libavcodec libavutil libswscale)
        endif()
        if(FFMPEG_PKG_FOUND)
            target_include_directories(CameraViewerApp PRIVATE ${FFMPEG_PKG_INCLUDE_DIRS})
            target_link_libraries(CameraViewerApp PRIVATE ${FFMPEG_PKG_LIBRARIES})
        else()
            find_library(AVCODEC_LIB avcodec)
            find_library(AVUTIL_LIB  avutil)
            find_library(SWSCALE_LIB swscale)
            if(NOT AVCODEC_LIB OR NOT AVUTIL_LIB OR NOT SWSCALE_LIB)
                message(FATAL_ERROR "System FFmpeg not found. Install libavcodec-dev libavutil-dev libswscale-dev.")
            endif()
            target_link_libraries(CameraViewerApp PRIVATE ${AVCODEC_LIB} ${AVUTIL_LIB} ${SWSCALE_LIB})
        endif()
    endif()
endif()

# CameraLibrary + platform libs
if(WIN32)
    target_link_libraries(
        CameraViewerApp
        PRIVATE
        CameraLibrary::CameraLibrary
        opengl32
        glu32
        ws2_32
        setupapi
    )

    add_custom_command(
        TARGET CameraViewerApp POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "Copying: $<TARGET_FILE:CameraLibrary::CameraLibrary> -> $<TARGET_FILE_DIR:CameraViewerApp>"
        COMMAND ${CMAKE_COMMAND} -E make_directory "$<TARGET_FILE_DIR:CameraViewerApp>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "$<TARGET_FILE:CameraLibrary::CameraLibrary>"
                "$<TARGET_FILE_DIR:CameraViewerApp>"
        VERBATIM
    )
elseif(UNIX)
    target_link_libraries(CameraViewerApp
        PRIVATE
        CameraLibrary::CameraLibrary
        rt
        stdc++
        pthread
        jpeg
    )
endif()
