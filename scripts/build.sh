#!/bin/bash

# UltraDYN Build Script
# This script provides local build functionality similar to the CI pipeline

set -e

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
NC='\033[0m' # No Color

# Default values
PLATFORM="macos"
CONFIGURATION="Release"
ARCHITECTURE="universal"
CLEAN=false
VALIDATE=false
CREATE_DMG=false
SIGN=false

# Function to print colored output
print_status() {
    echo -e "${BLUE}[INFO]${NC} $1"
}

print_success() {
    echo -e "${GREEN}[SUCCESS]${NC} $1"
}

print_warning() {
    echo -e "${YELLOW}[WARNING]${NC} $1"
}

print_error() {
    echo -e "${RED}[ERROR]${NC} $1"
}

# Function to show usage
show_usage() {
    echo "Usage: $0 [OPTIONS]"
    echo ""
    echo "Options:"
    echo "  -p, --platform PLATFORM     Target platform (macos, windows, linux) [default: macos]"
    echo "  -c, --config CONFIG         Build configuration (Debug, Release) [default: Release]"
    echo "  -a, --arch ARCH             Architecture (x86_64, arm64, universal) [default: universal]"
    echo "  -v, --validate              Run plugin validation after build"
    echo "  -d, --dmg                   Create DMG installer (macOS only)"
    echo "  -s, --sign                  Code sign plugins (macOS only)"
    echo "  --clean                     Clean build directory before building"
    echo "  -h, --help                  Show this help message"
    echo ""
    echo "Examples:"
    echo "  $0                                    # Build for macOS (universal, Release)"
    echo "  $0 -p windows -c Debug               # Build for Windows (Debug)"
    echo "  $0 -p macos -a arm64 -v -s           # Build for macOS ARM64 with validation and signing"
    echo "  $0 -p linux --clean                  # Clean build for Linux"
}

# Function to check dependencies
check_dependencies() {
    print_status "Checking dependencies..."
    
    case $PLATFORM in
        "macos")
            if ! command -v xcodebuild &> /dev/null; then
                print_error "Xcode not found. Please install Xcode."
                exit 1
            fi
            if ! command -v lipo &> /dev/null; then
                print_error "lipo not found. Please install Xcode command line tools."
                exit 1
            fi
            ;;
        "windows")
            if ! command -v msbuild &> /dev/null; then
                print_error "MSBuild not found. Please install Visual Studio."
                exit 1
            fi
            ;;
        "linux")
            if ! command -v make &> /dev/null; then
                print_error "make not found. Please install build tools."
                exit 1
            fi
            if ! command -v g++ &> /dev/null; then
                print_error "g++ not found. Please install gcc/g++."
                exit 1
            fi
            ;;
    esac
    
    print_success "Dependencies check passed"
}

# Function to setup JUCE
setup_juce() {
    print_status "Setting up JUCE framework..."
    
    JUCE_VERSION="8.0.8"
    JUCE_PATH=""
    
    case $PLATFORM in
        "macos")
            JUCE_PATH="$HOME/JUCE"
            ;;
        "windows")
            JUCE_PATH="C:\\JUCE"
            ;;
        "linux")
            JUCE_PATH="$HOME/JUCE"
            ;;
    esac
    
    if [ ! -d "$JUCE_PATH" ]; then
        print_warning "JUCE not found at $JUCE_PATH"
        print_status "Please download JUCE $JUCE_VERSION and place it at $JUCE_PATH"
        print_status "Or update the JUCE path in your .jucer file"
    else
        print_success "JUCE found at $JUCE_PATH"
    fi
}

# Function to clean build directory
clean_build() {
    if [ "$CLEAN" = true ]; then
        print_status "Cleaning build directory..."
        
        case $PLATFORM in
            "macos")
                rm -rf Builds/MacOSX/build
                ;;
            "windows")
                rm -rf Builds/VisualStudio2022/x64
                ;;
            "linux")
                rm -rf Builds/LinuxMakefile/build
                ;;
        esac
        
        print_success "Build directory cleaned"
    fi
}

# Function to build for macOS
build_macos() {
    print_status "Building for macOS ($CONFIGURATION, $ARCHITECTURE)..."
    
    if [ "$ARCHITECTURE" = "universal" ]; then
        # Build for both architectures
        for arch in x86_64 arm64; do
            print_status "Building for $arch..."
            export ARCHFLAGS="-arch $arch"
            xcodebuild -project Builds/MacOSX/ultraDYN.xcodeproj \
                -scheme "ultraDYN - All" \
                -configuration $CONFIGURATION \
                -arch $arch \
                -derivedDataPath Builds/MacOSX/build-$arch \
                clean build
        done
        
        # Create universal binaries
        print_status "Creating universal binaries..."
        lipo -create \
            Builds/MacOSX/build-x86_64/$CONFIGURATION/ultraDYN.vst3/Contents/MacOS/ultraDYN \
            Builds/MacOSX/build-arm64/$CONFIGURATION/ultraDYN.vst3/Contents/MacOS/ultraDYN \
            -output Builds/MacOSX/build-x86_64/$CONFIGURATION/ultraDYN.vst3/Contents/MacOS/ultraDYN
        
        lipo -create \
            Builds/MacOSX/build-x86_64/$CONFIGURATION/ultraDYN.component/Contents/MacOS/ultraDYN \
            Builds/MacOSX/build-arm64/$CONFIGURATION/ultraDYN.component/Contents/MacOS/ultraDYN \
            -output Builds/MacOSX/build-x86_64/$CONFIGURATION/ultraDYN.component/Contents/MacOS/ultraDYN
        
        lipo -create \
            Builds/MacOSX/build-x86_64/$CONFIGURATION/ultraDYN.app/Contents/MacOS/ultraDYN \
            Builds/MacOSX/build-arm64/$CONFIGURATION/ultraDYN.app/Contents/MacOS/ultraDYN \
            -output Builds/MacOSX/build-x86_64/$CONFIGURATION/ultraDYN.app/Contents/MacOS/ultraDYN
        
        # Copy universal builds to main build directory
        cp -R Builds/MacOSX/build-x86_64/$CONFIGURATION/* Builds/MacOSX/build/$CONFIGURATION/
    else
        # Build for single architecture
        export ARCHFLAGS="-arch $ARCHITECTURE"
        xcodebuild -project Builds/MacOSX/ultraDYN.xcodeproj \
            -scheme "ultraDYN - All" \
            -configuration $CONFIGURATION \
            -arch $ARCHITECTURE \
            -derivedDataPath Builds/MacOSX/build \
            clean build
    fi
    
    print_success "macOS build completed"
}

# Function to build for Windows
build_windows() {
    print_status "Building for Windows ($CONFIGURATION)..."
    
    msbuild Builds\VisualStudio2022\ultraDYN.sln \
        /p:Configuration=$CONFIGURATION \
        /p:Platform=x64 \
        /p:OutDir=Builds\VisualStudio2022\x64\$CONFIGURATION\
    
    print_success "Windows build completed"
}

# Function to build for Linux
build_linux() {
    print_status "Building for Linux ($CONFIGURATION)..."
    
    export JUCE_PATH="$HOME/JUCE"
    make -C Builds/LinuxMakefile \
        CONFIG=$CONFIGURATION \
        VST3DIR=build/$CONFIGURATION
    
    print_success "Linux build completed"
}

# Function to code sign plugins (macOS only)
sign_plugins() {
    if [ "$PLATFORM" != "macos" ]; then
        print_warning "Code signing is only available for macOS"
        return
    fi
    
    if [ "$SIGN" = true ]; then
        print_status "Code signing plugins..."
        
        # Check if we have a certificate
        if ! security find-identity -v -p codesigning | grep -q "ultraDYN"; then
            print_warning "No ultraDYN certificate found. Creating self-signed certificate..."
            
            # Create self-signed certificate
            openssl req -x509 -newkey rsa:4096 -keyout key.pem -out cert.pem -days 365 -nodes \
                -subj "/C=US/ST=State/L=City/O=Benzo Audio/OU=Development/CN=ultraDYN Development"
            
            openssl pkcs12 -export -out certificate.p12 -inkey key.pem -in cert.pem \
                -passout pass:password -name "ultraDYN Development"
            
            # Import certificate
            security import certificate.p12 -k login.keychain -P password -T /usr/bin/codesign
            
            # Clean up
            rm key.pem cert.pem certificate.p12
        fi
        
        # Sign plugins
        codesign --force --deep --sign "ultraDYN Development" \
            Builds/MacOSX/build/$CONFIGURATION/ultraDYN.vst3
        
        codesign --force --deep --sign "ultraDYN Development" \
            Builds/MacOSX/build/$CONFIGURATION/ultraDYN.component
        
        codesign --force --deep --sign "ultraDYN Development" \
            Builds/MacOSX/build/$CONFIGURATION/ultraDYN.app
        
        print_success "Plugins signed"
    fi
}

# Function to validate plugins
validate_plugins() {
    if [ "$VALIDATE" = true ]; then
        print_status "Validating plugins with pluginval..."
        
        # Check if pluginval is available
        PLUGINVAL_PATH=""
        case $PLATFORM in
            "macos")
                PLUGINVAL_PATH="$HOME/pluginval/pluginval"
                ;;
            "windows")
                PLUGINVAL_PATH="C:\\pluginval\\pluginval.exe"
                ;;
            "linux")
                PLUGINVAL_PATH="$HOME/pluginval/pluginval"
                ;;
        esac
        
        if [ ! -f "$PLUGINVAL_PATH" ]; then
            print_warning "pluginval not found. Please download it from https://github.com/Tracktion/pluginval"
            return
        fi
        
        # Run validation
        case $PLATFORM in
            "macos")
                $PLUGINVAL_PATH --validate-in-process --timeout-ms 60000 \
                    Builds/MacOSX/build/$CONFIGURATION/ultraDYN.vst3
                ;;
            "windows")
                "$PLUGINVAL_PATH" --validate-in-process --timeout-ms 60000 \
                    "Builds\VisualStudio2022\x64\$CONFIGURATION\ultraDYN.vst3"
                ;;
            "linux")
                $PLUGINVAL_PATH --validate-in-process --timeout-ms 60000 \
                    Builds/LinuxMakefile/build/$CONFIGURATION/ultraDYN.so
                ;;
        esac
        
        print_success "Plugin validation completed"
    fi
}

# Function to create DMG
create_dmg() {
    if [ "$PLATFORM" != "macos" ]; then
        print_warning "DMG creation is only available for macOS"
        return
    fi
    
    if [ "$CREATE_DMG" = true ]; then
        print_status "Creating DMG installer..."
        
        # Check if create-dmg is available
        if ! command -v create-dmg &> /dev/null; then
            print_warning "create-dmg not found. Installing..."
            brew install create-dmg
        fi
        
        # Create DMG
        create-dmg \
            --volname "ultraDYN Development" \
            --window-pos 200 120 \
            --window-size 800 400 \
            --icon-size 100 \
            --icon "ultraDYN.app" 200 190 \
            --hide-extension "ultraDYN.app" \
            --app-drop-link 600 185 \
            "ultraDYN-development-$PLATFORM.dmg" \
            "Builds/MacOSX/build/$CONFIGURATION/"
        
        print_success "DMG created: ultraDYN-development-$PLATFORM.dmg"
    fi
}

# Function to show build summary
show_summary() {
    print_success "Build completed successfully!"
    echo ""
    echo "Build Summary:"
    echo "  Platform: $PLATFORM"
    echo "  Configuration: $CONFIGURATION"
    echo "  Architecture: $ARCHITECTURE"
    echo "  Validation: $VALIDATE"
    echo "  Code Signing: $SIGN"
    echo "  DMG Created: $CREATE_DMG"
    echo ""
    
    case $PLATFORM in
        "macos")
            echo "Build artifacts:"
            echo "  VST3: Builds/MacOSX/build/$CONFIGURATION/ultraDYN.vst3"
            echo "  AU: Builds/MacOSX/build/$CONFIGURATION/ultraDYN.component"
            echo "  Standalone: Builds/MacOSX/build/$CONFIGURATION/ultraDYN.app"
            if [ "$CREATE_DMG" = true ]; then
                echo "  DMG: ultraDYN-development-$PLATFORM.dmg"
            fi
            ;;
        "windows")
            echo "Build artifacts:"
            echo "  VST3: Builds/VisualStudio2022/x64/$CONFIGURATION/ultraDYN.vst3"
            echo "  Standalone: Builds/VisualStudio2022/x64/$CONFIGURATION/ultraDYN.exe"
            ;;
        "linux")
            echo "Build artifacts:"
            echo "  VST3: Builds/LinuxMakefile/build/$CONFIGURATION/ultraDYN.so"
            echo "  Standalone: Builds/LinuxMakefile/build/$CONFIGURATION/ultraDYN"
            ;;
    esac
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -p|--platform)
            PLATFORM="$2"
            shift 2
            ;;
        -c|--config)
            CONFIGURATION="$2"
            shift 2
            ;;
        -a|--arch)
            ARCHITECTURE="$2"
            shift 2
            ;;
        -v|--validate)
            VALIDATE=true
            shift
            ;;
        -d|--dmg)
            CREATE_DMG=true
            shift
            ;;
        -s|--sign)
            SIGN=true
            shift
            ;;
        --clean)
            CLEAN=true
            shift
            ;;
        -h|--help)
            show_usage
            exit 0
            ;;
        *)
            print_error "Unknown option: $1"
            show_usage
            exit 1
            ;;
    esac
done

# Validate arguments
case $PLATFORM in
    "macos"|"windows"|"linux")
        ;;
    *)
        print_error "Invalid platform: $PLATFORM"
        exit 1
        ;;
esac

case $CONFIGURATION in
    "Debug"|"Release")
        ;;
    *)
        print_error "Invalid configuration: $CONFIGURATION"
        exit 1
        ;;
esac

case $ARCHITECTURE in
    "x86_64"|"arm64"|"universal")
        ;;
    *)
        print_error "Invalid architecture: $ARCHITECTURE"
        exit 1
        ;;
esac

# Main build process
print_status "Starting UltraDYN build process..."

check_dependencies
setup_juce
clean_build

case $PLATFORM in
    "macos")
        build_macos
        sign_plugins
        ;;
    "windows")
        build_windows
        ;;
    "linux")
        build_linux
        ;;
esac

validate_plugins
create_dmg
show_summary

print_success "Build process completed!"
