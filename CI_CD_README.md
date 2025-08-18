# UltraDYN CI/CD Pipeline

This repository includes a comprehensive CI/CD pipeline for building, testing, and releasing the UltraDYN audio plugin across multiple platforms.

## Overview

The CI/CD pipeline consists of three main workflows:

1. **CI Pipeline** (`ci.yml`) - Automated builds and testing on every push/PR
2. **Release Pipeline** (`release.yml`) - Automated releases when tags are pushed
3. **Development Pipeline** (`development.yml`) - Manual development builds

## Features

### ✅ Multi-Platform Support
- **macOS**: x86_64 and ARM64 architectures
- **Windows**: x64 architecture  
- **Linux**: x64 architecture

### ✅ Automated Plugin Validation
- Uses [pluginval](https://github.com/Tracktion/pluginval) for comprehensive plugin testing
- Validates VST3, AU, and Standalone formats
- Generates detailed validation reports

### ✅ Build Artifact Caching
- Caches JUCE framework downloads
- Caches build artifacts for faster incremental builds
- Uses ccache for compilation speed improvements

### ✅ Automatic DMG Creation (macOS)
- Creates professional DMG installers
- Includes proper volume naming and window positioning
- Automatically generated for releases

### ✅ Self-Signing Certificates
- Creates development certificates for macOS builds
- Enables code signing for all plugin formats
- Supports both development and release signing

## Workflows

### 1. CI Pipeline (`ci.yml`)

**Triggers**: Push to `main`/`develop` branches, Pull Requests

**Features**:
- Multi-platform matrix builds (macOS, Windows, Linux)
- Debug and Release configurations
- Plugin validation with pluginval
- Build artifact uploads
- Caching for faster builds

**Output**: Build artifacts available for download

### 2. Release Pipeline (`release.yml`)

**Triggers**: Push tags starting with `v*` (e.g., `v1.0.0`)

**Features**:
- Universal binary creation (x86_64 + ARM64 for macOS)
- Code signing with self-signed certificates
- DMG installer creation
- Automatic GitHub release creation
- Release notes generation

**Output**: GitHub release with installers and plugins

### 3. Development Pipeline (`development.yml`)

**Triggers**: Manual workflow dispatch

**Features**:
- Platform selection (macOS, Windows, Linux)
- Configuration selection (Debug, Release)
- Optional development release creation
- Code signing for development builds

**Output**: Development builds or releases

## Usage

### For Regular Development

1. **Push to develop branch**: Triggers CI pipeline for testing
2. **Create Pull Request**: Runs full CI suite
3. **Manual development builds**: Use the Development workflow

### For Releases

1. **Create and push a tag**:
   ```bash
   git tag v1.0.0
   git push origin v1.0.0
   ```

2. **Automatic release creation**: The release pipeline will:
   - Build universal binaries
   - Create DMG installer
   - Sign all plugins
   - Create GitHub release

### For Development Builds

1. **Go to Actions tab** in GitHub
2. **Select "Development Build"** workflow
3. **Click "Run workflow"**
4. **Configure options**:
   - Platform: macOS, Windows, or Linux
   - Configuration: Debug or Release
   - Create development release: Yes/No

## Build Artifacts

### macOS
- `ultraDYN.component` (Audio Unit)
- `ultraDYN.vst3` (VST3 Plugin)
- `ultraDYN.app` (Standalone Application)
- `ultraDYN-{version}-macOS.dmg` (Installer)

### Windows
- `ultraDYN.vst3` (VST3 Plugin)
- `ultraDYN.exe` (Standalone Application)

### Linux
- `ultraDYN.so` (VST3 Plugin)
- `ultraDYN` (Standalone Application)

## Caching Strategy

### JUCE Framework
- Cached per platform and version
- Automatically downloaded if not cached
- Reduces build time significantly

### Build Artifacts
- Cached based on source code hash
- Includes intermediate build files
- Platform and configuration specific

### Pluginval
- Cached per platform and version
- Validation tool for plugin testing

## Code Signing

### Development Builds
- Self-signed certificates created automatically
- Valid for development and testing
- No Apple Developer account required

### Release Builds
- Uses same self-signed certificates
- For production releases, replace with proper certificates

## Plugin Validation

The pipeline uses pluginval to validate plugins:

- **VST3**: Full VST3 specification compliance
- **Audio Unit**: AU validation and testing
- **Standalone**: Application functionality testing
- **Performance**: CPU and memory usage testing
- **Stability**: Crash and stability testing

## Environment Variables

| Variable | Description | Default |
|----------|-------------|---------|
| `JUCE_VERSION` | JUCE framework version | `8.0.8` |
| `PLUGINVAL_VERSION` | pluginval version | `1.0.0` |

## Troubleshooting

### Build Failures

1. **Check build logs** in GitHub Actions
2. **Verify JUCE path** in `.jucer` file
3. **Check platform-specific dependencies**

### Validation Failures

1. **Download validation results** from failed builds
2. **Review pluginval output** for specific issues
3. **Test locally** with pluginval

### Release Issues

1. **Verify tag format** (must start with `v`)
2. **Check permissions** for repository
3. **Review release workflow** logs

## Local Development

### Prerequisites

- **macOS**: Xcode, JUCE framework
- **Windows**: Visual Studio 2022, JUCE framework
- **Linux**: Build tools, JUCE framework

### Building Locally

1. **Clone repository**:
   ```bash
   git clone <repository-url>
   cd UltraDYN
   ```

2. **Open in Projucer**:
   - Open `ultraDYN.jucer`
   - Set JUCE path
   - Save and open in IDE

3. **Build**:
   - Use your IDE's build system
   - Or use command line tools

### Testing with pluginval

1. **Download pluginval** for your platform
2. **Run validation**:
   ```bash
   pluginval --validate-in-process --timeout-ms 60000 your-plugin.vst3
   ```

## Contributing

1. **Fork the repository**
2. **Create feature branch**
3. **Make changes**
4. **Push to your fork**
5. **Create Pull Request**

The CI pipeline will automatically test your changes.

## Support

For issues with the CI/CD pipeline:

1. **Check existing issues** in the repository
2. **Review workflow logs** for specific errors
3. **Create new issue** with detailed information

## License

This CI/CD pipeline is part of the UltraDYN project and follows the same license terms.
