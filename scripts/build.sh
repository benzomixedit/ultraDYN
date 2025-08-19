#!/bin/bash

# UltraDYN Build Script
# This script builds the UltraDYN plugin locally for testing

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Function to print colored output
print_status() {
    echo -e "${GREEN}[INFO]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Check if we're on macOS
if [[ "$OSTYPE" == "darwin"* ]]; then
    PLATFORM="macOS"
    print_status "Detected macOS platform"
elif [[ "$OSTYPE" == "msys" ]] || [[ "$OSTYPE" == "cygwin" ]]; then
    PLATFORM="Windows"
    print_status "Detected Windows platform"
else
    PLATFORM="Linux"
    print_status "Detected Linux platform"
fi

# Configuration
CONFIG=${1:-Release}
print_status "Building with configuration: $CONFIG"

# Check if JUCE is available
JUCE_DIR=""
if [[ "$PLATFORM" == "macOS" ]]; then
    # Check common JUCE locations on macOS
    JUCE_PATHS=(
        "$HOME/juce"
        "/usr/local/juce"
        "/opt/juce"
        "$HOME/Development/juce"
    )
    
    for path in "${JUCE_PATHS[@]}"; do
        if [[ -d "$path" && -d "$path/modules" ]]; then
            JUCE_DIR="$path"
            print_status "Found JUCE at: $JUCE_DIR"
            break
        fi
    done
fi

if [[ -z "$JUCE_DIR" ]]; then
    print_warning "JUCE not found in common locations"
    print_status "Please ensure JUCE is installed and set JUCE_DIR environment variable"
    print_status "Example: export JUCE_DIR=/path/to/juce"
    
    if [[ -n "$JUCE_DIR" ]]; then
        print_status "Using JUCE_DIR from environment: $JUCE_DIR"
    else
        print_error "JUCE_DIR not set and JUCE not found in common locations"
        exit 1
    fi
fi

# Build Projucer if needed
if [[ "$PLATFORM" == "macOS" ]]; then
    PROJUCER_BIN="$JUCE_DIR/extras/Projucer/Builds/MacOSX/build/Release/Projucer.app/Contents/MacOS/Projucer"
    
    if [[ ! -x "$PROJUCER_BIN" ]]; then
        print_status "Building Projucer..."
        cd "$JUCE_DIR/extras/Projucer/Builds/MacOSX"
        xcodebuild -project Projucer.xcodeproj -target "Projucer - App" -configuration Release
        cd - > /dev/null
    fi
    
    if [[ -x "$PROJUCER_BIN" ]]; then
        print_status "Generating project files with Projucer..."
        "$PROJUCER_BIN" --resave ultraDYN.jucer
    else
        print_error "Failed to build Projucer"
        exit 1
    fi
fi

# Build the project
if [[ "$PLATFORM" == "macOS" ]]; then
    print_status "Building macOS project..."
    
    # List available targets
    print_status "Available targets:"
    xcodebuild -project Builds/MacOSX/ultraDYN.xcodeproj -list
    
    # Build VST3 target
    print_status "Building VST3..."
    xcodebuild -project Builds/MacOSX/ultraDYN.xcodeproj -target "ultraDYN - VST3" -configuration "$CONFIG"
    
    # Build AU target
    print_status "Building AU..."
    xcodebuild -project Builds/MacOSX/ultraDYN.xcodeproj -target "ultraDYN - AU" -configuration "$CONFIG"
    
    print_status "Build completed successfully!"
    print_status "Check the following locations for build outputs:"
    print_status "  - build/Build/Products/$CONFIG/"
    print_status "  - Builds/MacOSX/build/Build/Products/$CONFIG/"
    
elif [[ "$PLATFORM" == "Windows" ]]; then
    print_status "Building Windows project..."
    
    # Find MSBuild
    MSBUILD_PATH=""
    MSBUILD_PATHS=(
        "msbuild"
        "C:/Program Files/Microsoft Visual Studio/2022/Enterprise/MSBuild/Current/Bin/MSBuild.exe"
        "C:/Program Files/Microsoft Visual Studio/2022/Professional/MSBuild/Current/Bin/MSBuild.exe"
        "C:/Program Files/Microsoft Visual Studio/2022/Community/MSBuild/Current/Bin/MSBuild.exe"
    )
    
    for path in "${MSBUILD_PATHS[@]}"; do
        if command -v "$path" >/dev/null 2>&1; then
            MSBUILD_PATH="$path"
            print_status "Found MSBuild at: $MSBUILD_PATH"
            break
        fi
    done
    
    if [[ -n "$MSBUILD_PATH" ]]; then
        "$MSBUILD_PATH" "Builds/VisualStudio2022/ultraDYN.sln" "/p:Configuration=$CONFIG" "/p:Platform=x64"
        print_status "Build completed successfully!"
    else
        print_error "MSBuild not found. Please ensure Visual Studio is installed."
        exit 1
    fi
else
    print_error "Unsupported platform: $PLATFORM"
    exit 1
fi

print_status "Build script completed!"
