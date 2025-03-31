#pragma once
#include <JuceHeader.h>

class Configuration : public juce::ChangeBroadcaster {
public:
    // Audio settings
    struct AudioSettings {
        juce::String outputDevice;
        juce::String inputDevice;
        double sampleRate{44100.0};
        int bufferSize{512};
        int inputChannels{2};
        int outputChannels{2};
        int bitDepth{32};
        bool dithering{true};
        bool autoConnectInputs{true};
        bool autoConnectOutputs{true};
    };

    // MIDI settings
    struct MIDISettings {
        juce::StringArray inputDevices;
        bool thruEnabled{true};
        bool clockEnabled{false};
        bool mtcEnabled{false};
        int mtcFormat{0};
        bool sendMMC{false};
        bool receiveMMC{false};
        float velocityScale{1.0f};
        float velocityOffset{0.0f};
    };

    // UI settings
    struct UISettings {
        struct Theme {
            bool darkMode{false};
            juce::Colour accentColor{juce::Colour::fromString("#007AFF")};
            int fontSize{14};
            juce::String fontName{"Default"};
            bool showTooltips{true};
            bool showStatusBar{true};
            bool showMeterBridges{true};
        } theme;

        struct Layout {
            bool mixerVisible{true};
            bool pianoRollVisible{false};
            int mixerHeight{200};
            int pianoRollHeight{300};
            int trackHeight{100};
            int minimumTrackHeight{60};
            int maximumTrackHeight{300};
        } layout;

        struct Grid {
            bool snapToGrid{true};
            bool showGrid{true};
            juce::Colour gridColor{juce::Colour::fromString("#404040")};
            float gridOpacity{0.5f};
            int majorGridInterval{4};
            int minorGridInterval{1};
        } grid;

        struct Meters {
            juce::String meterStyle{"gradient"};
            float meterFallback{1.5f};
            int peakHoldTime{2000};
            int rmsWindowSize{50};
            bool showPeakMarkers{true};
            bool showClipIndicators{true};
        } meters;
    };

    // Plugin settings
    struct PluginSettings {
        juce::StringArray scanPaths;
        juce::StringArray blacklist;
        juce::StringArray favoritePlugins;
        
        struct Format {
            bool vst3{true};
            bool au{true};
            bool lv2{false};
        } format;
        
        struct WindowBehavior {
            bool alwaysOnTop{false};
            bool hideWithHost{true};
            bool rememberPosition{true};
        } windowBehavior;
    };

    // Performance settings
    struct PerformanceSettings {
        int maxVoices{256};
        int diskCacheSize{1024};
        int ramCacheSize{512};
        int processingThreads{0};
        int pluginThreadPool{4};
        bool realTimeProcessing{true};
        bool useMMCSS{true};
        bool guardAgainstDenormals{true};
    };

    // Recording settings
    struct RecordingSettings {
        double prerollTime{2.0};
        double postrollTime{2.0};
        bool countInEnabled{true};
        int countInBars{1};
        bool punchInEnabled{false};
        bool punchOutEnabled{false};
        juce::String recordFileFormat{"wav"};
        int recordBitDepth{32};
        juce::String recordingPath;
        bool createTakeFolder{true};
        bool autoQuantize{false};
        float autoQuantizeAmount{0.5f};
    };

    // Export settings
    struct ExportSettings {
        juce::String defaultFormat{"wav"};
        int defaultBitDepth{24};
        int defaultSampleRate{44100};
        bool normalizeOutput{false};
        float normalizationLevel{-1.0f};
        bool addDithering{true};
        bool exportMarkers{true};
        bool splitStereoFiles{false};
        bool includePluginLatency{true};
    };

    // Constructor/Destructor
    Configuration();
    ~Configuration() override;

    // Singleton access
    static Configuration& getInstance();

    // Settings access
    AudioSettings& getAudioSettings() { return audioSettings; }
    MIDISettings& getMIDISettings() { return midiSettings; }
    UISettings& getUISettings() { return uiSettings; }
    PluginSettings& getPluginSettings() { return pluginSettings; }
    PerformanceSettings& getPerformanceSettings() { return performanceSettings; }
    RecordingSettings& getRecordingSettings() { return recordingSettings; }
    ExportSettings& getExportSettings() { return exportSettings; }

    const AudioSettings& getAudioSettings() const { return audioSettings; }
    const MIDISettings& getMIDISettings() const { return midiSettings; }
    const UISettings& getUISettings() const { return uiSettings; }
    const PluginSettings& getPluginSettings() const { return pluginSettings; }
    const PerformanceSettings& getPerformanceSettings() const { return performanceSettings; }
    const RecordingSettings& getRecordingSettings() const { return recordingSettings; }
    const ExportSettings& getExportSettings() const { return exportSettings; }

    // File operations
    void loadFromFile();
    void saveToFile();
    void resetToDefaults();

    // Path handling
    juce::File getConfigDirectory() const;
    juce::File getConfigFile() const;
    juce::File getDefaultProjectDirectory() const;
    juce::File getDefaultRecordingDirectory() const;

private:
    AudioSettings audioSettings;
    MIDISettings midiSettings;
    UISettings uiSettings;
    PluginSettings pluginSettings;
    PerformanceSettings performanceSettings;
    RecordingSettings recordingSettings;
    ExportSettings exportSettings;

    void loadDefaults();
    void createDefaultDirectories();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Configuration)
};