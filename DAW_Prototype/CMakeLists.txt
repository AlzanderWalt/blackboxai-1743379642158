cmake_minimum_required(VERSION 3.15)

project(DAW_PROTOTYPE VERSION 1.0.0)

# Add JUCE as a subdirectory
add_subdirectory(JUCE)

# Create the main application target
juce_add_gui_app(DAW_PROTOTYPE
    PRODUCT_NAME "DAW Prototype"
    VERSION "1.0.0"
    COMPANY_NAME "Your Company"
    COMPANY_WEBSITE "www.yourcompany.com"
    COMPANY_EMAIL "info@yourcompany.com"
    PLUGIN_MANUFACTURER_CODE "Ycom"
    PLUGIN_CODE "Daw1"
    FORMATS AU VST3 Standalone
    MICROPHONE_PERMISSION_ENABLED TRUE
    NEEDS_CURL TRUE
    NEEDS_WEB_BROWSER TRUE
)

# Add source files
target_sources(DAW_PROTOTYPE
    PRIVATE
        src/main.cpp
        src/App.cpp
        src/MainComponent.cpp
        src/MixerComponent.cpp
        src/TrackEditorComponent.cpp
        src/PianoRollComponent.cpp
        src/AudioEngine.cpp
        src/MIDISequencer.cpp
        src/Mixer.cpp
        src/Track.cpp
        src/Clip.cpp
        src/Plugin.cpp
        src/PluginManager.cpp
        src/Project.cpp
        src/Commands.cpp
        src/Configuration.cpp
        src/Logger.cpp
        src/AudioUtils.cpp
        src/MIDIUtils.cpp
        src/CustomLookAndFeel.cpp
)

# Set C++ standard
target_compile_features(DAW_PROTOTYPE PRIVATE cxx_std_17)

# Add include directories
target_include_directories(DAW_PROTOTYPE
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Link with JUCE modules
target_link_libraries(DAW_PROTOTYPE
    PRIVATE
        juce::juce_audio_basics
        juce::juce_audio_devices
        juce::juce_audio_formats
        juce::juce_audio_processors
        juce::juce_audio_utils
        juce::juce_core
        juce::juce_data_structures
        juce::juce_dsp
        juce::juce_events
        juce::juce_graphics
        juce::juce_gui_basics
        juce::juce_gui_extra
    PUBLIC
        juce::juce_recommended_config_flags
        juce::juce_recommended_lto_flags
        juce::juce_recommended_warning_flags
)

# Set binary output directories
set_target_properties(DAW_PROTOTYPE PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib"
)

# Copy resources to build directory
add_custom_command(TARGET DAW_PROTOTYPE POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/resources"
        "${CMAKE_BINARY_DIR}/bin/resources"
)

# Install targets
install(TARGETS DAW_PROTOTYPE
    RUNTIME DESTINATION bin
    LIBRARY DESTINATION lib
    ARCHIVE DESTINATION lib
)

# Install resources
install(DIRECTORY resources/
    DESTINATION bin/resources
)