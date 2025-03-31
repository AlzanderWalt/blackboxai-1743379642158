# DAW Prototype

A Digital Audio Workstation (DAW) prototype built with JUCE framework, featuring audio/MIDI recording, mixing, and plugin support.

## Features

- Multi-track audio and MIDI recording/playback
- Mixer with channel strips, buses, and effects sends
- Piano roll editor for MIDI note editing
- Plugin support (VST3, AU)
- Transport controls with tempo and time signature settings
- Project save/load functionality
- Customizable UI with dark/light mode support

## Building from Source

### Prerequisites

- CMake 3.15 or higher
- C++17 compatible compiler
- JUCE framework (automatically downloaded by CMake)

### Build Instructions

1. Clone the repository:
```bash
git clone https://github.com/yourusername/DAW_Prototype.git
cd DAW_Prototype
```

2. Clone JUCE (if not already present):
```bash
git clone https://github.com/juce-framework/JUCE.git
```

3. Create and enter build directory:
```bash
mkdir build
cd build
```

4. Configure with CMake:
```bash
cmake ..
```

5. Build the project:
```bash
cmake --build .
```

The built application will be in the `build/bin` directory.

## Project Structure

- `src/` - Source files
  - `App.*` - Application initialization and management
  - `MainComponent.*` - Main window content
  - `AudioEngine.*` - Audio processing and device management
  - `MIDISequencer.*` - MIDI recording and playback
  - `Mixer.*` - Audio mixing and routing
  - `Track.*` - Track management
  - `Plugin.*` - Plugin hosting
  - Various UI components and utilities

- `resources/` - Application resources
  - `config.json` - Default configuration
  - `project.json` - Project template

## Development

### Code Style

- Use JUCE naming conventions
- Class names in PascalCase
- Method names in camelCase
- Member variables prefixed with 'm_'
- Constants in UPPER_CASE

### Adding New Features

1. Create new class files in `src/`
2. Add files to `CMakeLists.txt`
3. Update relevant manager classes
4. Add UI components if needed
5. Update documentation

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Contributing

1. Fork the repository
2. Create a feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## Acknowledgments

- JUCE framework team
- Contributors and testers
- Open source audio community

## Contact

Your Name - your.email@example.com
Project Link: https://github.com/yourusername/DAW_Prototype