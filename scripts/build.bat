@echo off
setlocal enabledelayedexpansion

REM UltraDYN Build Script for Windows
REM This script provides local build functionality similar to the CI pipeline

REM Default values
set PLATFORM=windows
set CONFIGURATION=Release
set ARCHITECTURE=x64
set CLEAN=false
set VALIDATE=false
set SIGN=false

REM Function to show usage
:show_usage
echo Usage: %0 [OPTIONS]
echo.
echo Options:
echo   -p, --platform PLATFORM     Target platform (windows only) [default: windows]
echo   -c, --config CONFIG         Build configuration (Debug, Release) [default: Release]
echo   -a, --arch ARCH             Architecture (x64 only) [default: x64]
echo   -v, --validate              Run plugin validation after build
echo   -s, --sign                  Code sign plugins
echo   --clean                     Clean build directory before building
echo   -h, --help                  Show this help message
echo.
echo Examples:
echo   %0                                    # Build for Windows (x64, Release)
echo   %0 -c Debug                          # Build for Windows (Debug)
echo   %0 -v -s                             # Build with validation and signing
echo   %0 --clean                           # Clean build
goto :eof

REM Function to print colored output
:print_status
echo [INFO] %~1
goto :eof

:print_success
echo [SUCCESS] %~1
goto :eof

:print_warning
echo [WARNING] %~1
goto :eof

:print_error
echo [ERROR] %~1
goto :eof

REM Function to check dependencies
:check_dependencies
call :print_status "Checking dependencies..."

where msbuild >nul 2>&1
if %errorlevel% neq 0 (
    call :print_error "MSBuild not found. Please install Visual Studio."
    exit /b 1
)

call :print_success "Dependencies check passed"
goto :eof

REM Function to setup JUCE
:setup_juce
call :print_status "Setting up JUCE framework..."

set JUCE_VERSION=8.0.8
set JUCE_PATH=C:\JUCE

if not exist "%JUCE_PATH%" (
    call :print_warning "JUCE not found at %JUCE_PATH%"
    call :print_status "Please download JUCE %JUCE_VERSION% and place it at %JUCE_PATH%"
    call :print_status "Or update the JUCE path in your .jucer file"
) else (
    call :print_success "JUCE found at %JUCE_PATH%"
)
goto :eof

REM Function to clean build directory
:clean_build
if "%CLEAN%"=="true" (
    call :print_status "Cleaning build directory..."
    
    if exist "Builds\VisualStudio2022\x64" (
        rmdir /s /q "Builds\VisualStudio2022\x64"
    )
    
    call :print_success "Build directory cleaned"
)
goto :eof

REM Function to build for Windows
:build_windows
call :print_status "Building for Windows (%CONFIGURATION%)..."

msbuild Builds\VisualStudio2022\ultraDYN.sln ^
    /p:Configuration=%CONFIGURATION% ^
    /p:Platform=x64 ^
    /p:OutDir=Builds\VisualStudio2022\x64\%CONFIGURATION%\

if %errorlevel% neq 0 (
    call :print_error "Build failed"
    exit /b 1
)

call :print_success "Windows build completed"
goto :eof

REM Function to code sign plugins
:sign_plugins
if "%SIGN%"=="true" (
    call :print_status "Code signing plugins..."
    
    REM Check if we have a certificate
    certutil -store My | findstr /i "ultraDYN" >nul
    if %errorlevel% neq 0 (
        call :print_warning "No ultraDYN certificate found. Creating self-signed certificate..."
        
        REM Create self-signed certificate (requires admin privileges)
        makecert -r -pe -n "CN=ultraDYN Development" -ss My -a sha256 -cy end -sky signature -sv ultraDYN.pvk ultraDYN.cer
        if %errorlevel% neq 0 (
            call :print_warning "Failed to create certificate. Please run as administrator or use existing certificate."
            goto :skip_signing
        )
        
        REM Import certificate
        certutil -importpfx ultraDYN.pfx
        if %errorlevel% neq 0 (
            call :print_warning "Failed to import certificate."
            goto :skip_signing
        )
        
        REM Clean up
        del ultraDYN.pvk ultraDYN.cer ultraDYN.pfx
    )
    
    REM Sign plugins (requires signtool)
    where signtool >nul 2>&1
    if %errorlevel% equ 0 (
        signtool sign /f ultraDYN.pfx /p password "Builds\VisualStudio2022\x64\%CONFIGURATION%\ultraDYN.vst3"
        signtool sign /f ultraDYN.pfx /p password "Builds\VisualStudio2022\x64\%CONFIGURATION%\ultraDYN.exe"
        call :print_success "Plugins signed"
    ) else (
        call :print_warning "signtool not found. Skipping code signing."
    )
)
:skip_signing
goto :eof

REM Function to validate plugins
:validate_plugins
if "%VALIDATE%"=="true" (
    call :print_status "Validating plugins with pluginval..."
    
    set PLUGINVAL_PATH=C:\pluginval\pluginval.exe
    
    if not exist "%PLUGINVAL_PATH%" (
        call :print_warning "pluginval not found. Please download it from https://github.com/Tracktion/pluginval"
        goto :eof
    )
    
    "%PLUGINVAL_PATH%" --validate-in-process --timeout-ms 60000 "Builds\VisualStudio2022\x64\%CONFIGURATION%\ultraDYN.vst3"
    
    if %errorlevel% neq 0 (
        call :print_warning "Plugin validation failed"
    ) else (
        call :print_success "Plugin validation completed"
    )
)
goto :eof

REM Function to show build summary
:show_summary
call :print_success "Build completed successfully!"
echo.
echo Build Summary:
echo   Platform: %PLATFORM%
echo   Configuration: %CONFIGURATION%
echo   Architecture: %ARCHITECTURE%
echo   Validation: %VALIDATE%
echo   Code Signing: %SIGN%
echo.
echo Build artifacts:
echo   VST3: Builds\VisualStudio2022\x64\%CONFIGURATION%\ultraDYN.vst3
echo   Standalone: Builds\VisualStudio2022\x64\%CONFIGURATION%\ultraDYN.exe
goto :eof

REM Parse command line arguments
:parse_args
if "%~1"=="" goto :main
if "%~1"=="-p" (
    set PLATFORM=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--platform" (
    set PLATFORM=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="-c" (
    set CONFIGURATION=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--config" (
    set CONFIGURATION=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="-a" (
    set ARCHITECTURE=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="--arch" (
    set ARCHITECTURE=%~2
    shift
    shift
    goto :parse_args
)
if "%~1"=="-v" (
    set VALIDATE=true
    shift
    goto :parse_args
)
if "%~1"=="--validate" (
    set VALIDATE=true
    shift
    goto :parse_args
)
if "%~1"=="-s" (
    set SIGN=true
    shift
    goto :parse_args
)
if "%~1"=="--sign" (
    set SIGN=true
    shift
    goto :parse_args
)
if "%~1"=="--clean" (
    set CLEAN=true
    shift
    goto :parse_args
)
if "%~1"=="-h" (
    call :show_usage
    exit /b 0
)
if "%~1"=="--help" (
    call :show_usage
    exit /b 0
)
call :print_error "Unknown option: %~1"
call :show_usage
exit /b 1

REM Validate arguments
:validate_args
if not "%PLATFORM%"=="windows" (
    call :print_error "Invalid platform: %PLATFORM%"
    exit /b 1
)

if not "%CONFIGURATION%"=="Debug" if not "%CONFIGURATION%"=="Release" (
    call :print_error "Invalid configuration: %CONFIGURATION%"
    exit /b 1
)

if not "%ARCHITECTURE%"=="x64" (
    call :print_error "Invalid architecture: %ARCHITECTURE%"
    exit /b 1
)
goto :eof

REM Main build process
:main
call :print_status "Starting UltraDYN build process..."

call :check_dependencies
if %errorlevel% neq 0 exit /b %errorlevel%

call :setup_juce
call :clean_build
call :build_windows
if %errorlevel% neq 0 exit /b %errorlevel%

call :sign_plugins
call :validate_plugins
call :show_summary

call :print_success "Build process completed!"
exit /b 0
