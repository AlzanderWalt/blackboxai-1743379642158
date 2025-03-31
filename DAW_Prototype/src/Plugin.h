#pragma once
#include <JuceHeader.h>

class Track;

class Plugin : public juce::ChangeBroadcaster {
public:
    // Plugin types
    enum class Type {
        VST3,
        AudioUnit,
        Internal
    };

    // Plugin format info
    struct Format {
        Type type;
        juce::String name;
        juce::String manufacturer;
        juce::String version;
        juce::String identifier;
        bool isInstrument;
        int numInputChannels;
        int numOutputChannels;
        juce::Array<juce::AudioProcessorParameter*> parameters;
    };

    // Constructor/Destructor
    Plugin(Track& track);
    virtual ~Plugin();

    // Basic properties
    Track& getTrack() const { return track; }
    virtual const Format& getFormat() const = 0;
    virtual juce::String getName() const = 0;
    
    bool isBypassed() const { return bypassed; }
    void bypass(bool shouldBypass);
    
    bool isEnabled() const { return enabled; }
    void enable(bool shouldBeEnabled);

    // GUI
    virtual bool hasEditor() const = 0;
    virtual juce::AudioProcessorEditor* createEditor() = 0;
    
    // State management
    virtual void saveState(juce::MemoryBlock& destData) const = 0;
    virtual void loadState(const void* data, size_t sizeInBytes) = 0;
    
    // Preset management
    virtual void savePreset(const juce::File& file) const = 0;
    virtual void loadPreset(const juce::File& file) = 0;
    virtual juce::StringArray getPresetNames() const = 0;
    virtual void setCurrentPreset(int index) = 0;
    virtual int getCurrentPreset() const = 0;

    // Parameter management
    virtual int getNumParameters() const = 0;
    virtual float getParameter(int index) const = 0;
    virtual void setParameter(int index, float value) = 0;
    virtual juce::String getParameterName(int index) const = 0;
    virtual juce::String getParameterText(int index) const = 0;
    virtual juce::NormalisableRange<float> getParameterRange(int index) const = 0;

    // Processing setup
    virtual void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) = 0;
    virtual void releaseResources() = 0;
    virtual void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) = 0;

    // Latency handling
    virtual int getLatencySamples() const = 0;
    virtual double getTailLengthSeconds() const = 0;

    // Program handling
    virtual int getNumPrograms() const = 0;
    virtual int getCurrentProgram() const = 0;
    virtual void setCurrentProgram(int index) = 0;
    virtual juce::String getProgramName(int index) const = 0;

protected:
    Track& track;
    bool bypassed{false};
    bool enabled{true};
    
    // Internal helpers
    virtual void bypassProcessing(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    virtual void handleBypassChange();
    virtual void handleEnableChange();

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Plugin)
};

// Plugin factory interface
class PluginFactory {
public:
    virtual ~PluginFactory() = default;
    
    virtual juce::StringArray getPluginNames() const = 0;
    virtual std::unique_ptr<Plugin> createPlugin(Track& track, const juce::String& name) = 0;
    virtual const Plugin::Format& getPluginFormat(const juce::String& name) const = 0;
};

// Plugin utilities
namespace PluginUtils {
    // Plugin type conversion
    juce::String typeToString(Plugin::Type type);
    Plugin::Type stringToType(const juce::String& str);
    
    // Parameter conversion
    float normalizeParameter(float value, const juce::NormalisableRange<float>& range);
    float denormalizeParameter(float normalized, const juce::NormalisableRange<float>& range);
    
    // State management
    void savePluginState(const Plugin& plugin, juce::ValueTree& state);
    void loadPluginState(Plugin& plugin, const juce::ValueTree& state);
    
    // Preset management
    juce::File getPresetDirectory();
    juce::Array<juce::File> findPresetFiles(const juce::String& formatName);
    bool isPresetFile(const juce::File& file);
    
    // Plugin scanning
    juce::StringArray getAvailablePlugins(Plugin::Type type);
    bool validatePlugin(const juce::String& path);
    juce::String getPluginArchitecture(const juce::String& path);
}

// RAII helper for plugin processing
class ScopedPluginProcess {
public:
    ScopedPluginProcess(Plugin& p, juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midi)
        : plugin(p)
        , audioBuffer(buffer)
        , midiBuffer(midi) {
        // Store original buffers
        originalAudio.makeCopyOf(buffer);
        originalMidi = midi;
    }
    
    ~ScopedPluginProcess() {
        // Restore original buffers if processing failed
        if (!succeeded) {
            audioBuffer.makeCopyOf(originalAudio);
            midiBuffer = originalMidi;
        }
    }
    
    void success() { succeeded = true; }

private:
    Plugin& plugin;
    juce::AudioBuffer<float>& audioBuffer;
    juce::MidiBuffer& midiBuffer;
    juce::AudioBuffer<float> originalAudio;
    juce::MidiBuffer originalMidi;
    bool succeeded{false};
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ScopedPluginProcess)
};