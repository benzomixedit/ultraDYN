# ultraDYN - Dynamic Compressor Plugin

ultraDYN is a professional dynamic compressor plugin featuring both downwards and upwards compression with advanced controls and metering.

## Features

- **Dual Compression**: Downwards and upwards compression in one plugin
- **Advanced Controls**: Threshold, ratio, attack, release, knee, and mix controls for each compressor
- **Real-time Metering**: Input, output, gain reduction, and upwards gain meters
- **Vocal and Drumbus Modes**: Specialized EQ curves for different applications
- **Cross-Platform**: Compatible with macOS and Windows

## Installation

### System Requirements

- **macOS**: 10.13 (High Sierra) or later
- **Windows**: Windows 10 or later
- **Architecture**: Universal binary (Intel x86_64 + Apple Silicon arm64) on macOS, x64 on Windows
- **Plugin Formats**: Audio Unit (AU) and VST3 on macOS, VST3 on Windows

### macOS Installation

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

### Windows Installation

#### VST3 Plugin
Copy the `ultraDYN.vst3` folder to one of these locations:
```
C:\Program Files\Common Files\VST3\
C:\Program Files\VST3\
```

**Note**: Some DAWs may require the plugin to be in a specific VST3 folder. Check your DAW's documentation for the preferred location.

### Installation Steps

#### macOS
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

#### Windows
1. **Download the plugin files** from the releases section
2. **Extract the ZIP file** to a temporary location
3. **Copy the VST3 plugin**:
   - Navigate to `C:\Program Files\Common Files\VST3\` (or your preferred VST3 location)
   - Copy the entire `ultraDYN.vst3` folder to this location
4. **Restart your DAW** to scan for new plugins

### Alternative Installation (System-wide)

#### macOS
If you want to install the plugins system-wide (available to all users):

##### Audio Unit (AU) Plugin
```
/Library/Audio/Plug-Ins/Components/
```

##### VST3 Plugin
```
/Library/Audio/Plug-Ins/VST3/
```

**Note**: System-wide installation requires administrator privileges.

#### Windows
For system-wide installation on Windows, use:
```
C:\Program Files\Common Files\VST3\
```

**Note**: System-wide installation requires administrator privileges.

## Supported DAWs

The plugin has been tested and works with:

### macOS
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

### Windows
- **Ableton Live** (VST3)
- **FL Studio** (VST3)
- **Reaper** (VST3)
- **Cubase** (VST3)
- **Studio One** (VST3)
- **Pro Tools** (VST3)
- **Bitwig Studio** (VST3)
- **Waveform** (VST3)

## Troubleshooting

### Plugin Not Appearing in DAW

#### macOS
1. **Verify installation location**: Make sure the plugin is in the correct folder
2. **Check file permissions**: Ensure the plugin files are readable
3. **Rescan plugins**: Most DAWs have a "Rescan Plugins" option in preferences
4. **Restart DAW**: Some DAWs require a restart to detect new plugins

#### Windows
1. **Verify installation location**: Make sure the plugin is in the correct VST3 folder
2. **Check file permissions**: Ensure the plugin files are readable
3. **Rescan plugins**: Most DAWs have a "Rescan Plugins" option in preferences
4. **Restart DAW**: Some DAWs require a restart to detect new plugins
5. **Check VST3 path**: Ensure your DAW is configured to scan the correct VST3 folder

### "Not a Compatible 64-bit Plugin" Error

#### macOS
This plugin is built as a universal binary supporting both Intel and Apple Silicon architectures. If you encounter this error:

1. **Ensure you're using the latest version** of the plugin
2. **Check your DAW's architecture**: Make sure your DAW is running in the correct mode
3. **Verify plugin format**: Some DAWs prefer AU over VST3 or vice versa

#### Windows
This plugin is built for 64-bit Windows systems. If you encounter this error:

1. **Ensure you're using the latest version** of the plugin
2. **Check your DAW's architecture**: Make sure your DAW is running in 64-bit mode
3. **Verify Windows version**: Ensure you're running Windows 10 or later

### Plugin Crashes or Performance Issues

1. **Update your DAW**: Ensure you're using the latest version
2. **Check system resources**: Ensure adequate CPU and memory
3. **Try different plugin format**: If AU crashes on macOS, try VST3 or vice versa

## Technical Specifications

- **Sample Rates**: 44.1 kHz to 192 kHz
- **Bit Depth**: 32-bit float
- **Architectures**: Universal binary (x86_64 + arm64) on macOS, x64 on Windows
- **Plugin Formats**: Audio Unit (AU) and VST3 on macOS, VST3 on Windows
- **GUI**: Native interface with high DPI support

## Support

For technical support or bug reports, please visit the project repository or contact the development team.

## License

[Add your license information here]

---

**Version**: 1.0.0  
**Build Date**: August 29, 2025  
**Architecture**: Universal Binary (Intel x86_64 + Apple Silicon arm64) on macOS, x64 on Windows
