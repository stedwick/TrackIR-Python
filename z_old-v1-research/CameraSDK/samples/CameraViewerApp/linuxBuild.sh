#!/bin/bash
# Usage: ./linuxBuild.sh /path/to/camera/sdk [--ffmpeg]

if [ -z "$1" ]; then
    echo "Usage: $0 <CAMERA_SDK_PATH> [--ffmpeg]"
    exit 1
fi

CAMERA_SDK_PATH="$1"
shift

BUILD_DIR=build
ENABLE_FFMPEG=OFF

# Parse extra args
while [[ $# -gt 0 ]]; do
    case "$1" in
        --ffmpeg)
            ENABLE_FFMPEG=ON
            ;;
        *)
            echo "Unknown option: $1"
            echo "Usage: $0 <CAMERA_SDK_PATH> [--ffmpeg]"
            exit 1
            ;;
    esac
    shift
done

# Check if build directory exists
if [ -d "$BUILD_DIR" ]; then
    echo "Warning: This will delete the existing '$BUILD_DIR' directory and all its contents."
    read -p "Are you sure you want to continue? [y/N] " confirm
    if [[ ! "$confirm" =~ ^[Yy]$ ]]; then
        echo "Aborted."
        exit 1
    fi
    rm -rf "$BUILD_DIR"
fi

# Create new build directory
mkdir "$BUILD_DIR"
cd "$BUILD_DIR"

# Run CMake to configure and build
cmake -DCAMERA_SDK_PATH=$CAMERA_SDK_PATH -DENABLE_FFMPEG=$ENABLE_FFMPEG ..
make -j$(nproc)

# Run the executable if successful
if [ $? -eq 0 ]; then
    echo "Build successful!"
else
    echo "Build failed!"
fi
