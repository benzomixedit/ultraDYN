# ultraDYN - Dynamic Compressor Plugin

ultraDYN is a professional dynamic compressor plugin featuring both downwards and upwards compression with advanced controls and metering.

## Features

- **Dual Compression**: Downwards and upwards compression in one plugin
- **Advanced Controls**: Threshold, ratio, attack, release, knee, and mix controls for each compressor
- **Real-time Metering**: Input, output, gain reduction, and upwards gain meters
- **Vocal and Drumbus Modes**: Specialized EQ curves for different applications
- **Universal Binary**: Compatible with both Intel (x86_64) and Apple Silicon (arm64) Macs

## CI/CD Pipeline

This project includes automated CI/CD pipelines for building and testing the plugin across multiple platforms:

### Automated Builds
- **CI Pipeline**: Runs on every push and pull request
- **Release Pipeline**: Automatically creates releases when tags are pushed
- **Development Pipeline**: Manual builds for testing

### Supported Platforms
- **macOS**: Universal binary (x86_64 + arm64)
- **Windows**: x64 architecture

### Build Artifacts
- VST3 plugins
- Audio Unit plugins (macOS)
- Standalone applications
- Release packages (ZIP files)

For detailed information about the CI/CD setup, see [CI_CD_README.md](CI_CD_README.md).

## Installation

### System Requirements

- **macOS**: 10.13 (High Sierra) or later
- **Architecture**: Universal binary (Intel x86_64 + Apple Silicon arm64)
- **Plugin Formats**: Audio Unit (AU) and VST3

### Plugin Installation Locations

#### Audio Unit (AU) Plugin
Copy the `ultraDYN.component` file to:
```
~/Library/Audio/Plug-Ins/Components/
```

#### VST3 Plugin
Copy the `ultraDYN.vst3` folder to:
```
~/Library/Audio/Plug-Ins/VST3/
```

### Installation Steps

1. **Download the plugin files** from the releases section
2. **Locate your user Library folder**:
   - Open Finder
   - Press `Cmd + Shift + G` (Go to Folder)
   - Type: `~/Library/Audio/Plug-Ins/`
   - Press Enter

3. **Install Audio Unit**:
   - Navigate to the `Components` folder
   - Copy `ultraDYN.component` to this location

4. **Install VST3**:
   - Navigate to the `VST3` folder
   - Copy the entire `ultraDYN.vst3` folder to this location

5. **Restart your DAW** to scan for new plugins

### Alternative Installation (System-wide)

If you want to install the plugins system-wide (available to all users):

#### Audio Unit (AU) Plugin
```
/Library/Audio/Plug-Ins/Components/
```

#### VST3 Plugin
```
/Library/Audio/Plug-Ins/VST3/
```

**Note**: System-wide installation requires administrator privileges.

## Supported DAWs

The plugin has been tested and works with:
- **Pro Tools** (AU, VST3)
- **Logic Pro** (AU)
- **Ableton Live** (AU, VST3)
- **FL Studio** (VST3)
- **Reaper** (AU, VST3)
- **Cubase** (VST3)
- **Studio One** (AU, VST3)
- **GarageBand** (AU)
- **DDMF Metaplugin** (AU, VST3)
- **Blue Cat Patchworks** (AU, VST3)

## Troubleshooting

### Plugin Not Appearing in DAW

1. **Verify installation location**: Make sure the plugin is in the correct folder
2. **Check file permissions**: Ensure the plugin files are readable
3. **Rescan plugins**: Most DAWs have a "Rescan Plugins" option in preferences
4. **Restart DAW**: Some DAWs require a restart to detect new plugins

### "Not a Compatible 64-bit Plugin" Error

This plugin is built as a universal binary supporting both Intel and Apple Silicon architectures. If you encounter this error:

1. **Ensure you're using the latest version** of the plugin
2. **Check your DAW's architecture**: Make sure your DAW is running in the correct mode
3. **Verify plugin format**: Some DAWs prefer AU over VST3 or vice versa

### Plugin Crashes or Performance Issues

1. **Update your DAW**: Ensure you're using the latest version
2. **Check system resources**: Ensure adequate CPU and memory
3. **Try different plugin format**: If AU crashes, try VST3 or vice versa

## Technical Specifications

- **Sample Rates**: 44.1 kHz to 192 kHz
- **Bit Depth**: 32-bit float
- **Architectures**: Universal binary (x86_64 + arm64)
- **Plugin Formats**: Audio Unit (AU) and VST3
- **GUI**: Native macOS interface with high DPI support

## Support

For technical support or bug reports, please visit the project repository or contact the development team.

## License

[Add your license information here]

---

**Version**: 1.0.0  
**Build Date**: [Current Date]  
**Architecture**: Universal Binary (Intel x86_64 + Apple Silicon arm64)
