#pragma once
#include <JuceHeader.h>
#include <vector>
#include <memory>

class Track;
class Project;
class Plugin;

class Mixer : public juce::ChangeBroadcaster {
public:
    // Mixer channel strip
    struct Channel {
        float volume{1.0f};
        float pan{0.0f};
        bool mute{false};
        bool solo{false};
        bool bypass{false};
        float peakLevel{0.0f};
        float rmsLevel{0.0f};
        std::vector<std::pair<int, float>> sends;  // bus index, level
        std::vector<std::unique_ptr<Plugin>> plugins;
    };

    // Bus types
    enum class BusType {
        Aux,
        Group,
        Master
    };

    // Bus configuration
    struct Bus {
        BusType type;
        juce::String name;
        Channel channel;
        std::vector<int> sources;  // Track/bus indices
        int outputBus{-1};  // -1 = master
    };

    // Constructor/Destructor
    Mixer();
    ~Mixer() override;

    // Project handling
    void setProject(Project* project);
    Project* getProject() const { return currentProject; }

    // Channel management
    Channel& getChannel(int index);
    const Channel& getChannel(int index) const;
    Channel& getMasterChannel() { return masterChannel; }
    const Channel& getMasterChannel() const { return masterChannel; }
    
    void setChannelVolume(int index, float volume);
    void setChannelPan(int index, float pan);
    void setChannelMute(int index, bool mute);
    void setChannelSolo(int index, bool solo);
    void setChannelBypass(int index, bool bypass);
    
    float getChannelPeakLevel(int index) const;
    float getChannelRMSLevel(int index) const;
    
    void addSend(int channelIndex, int busIndex, float level);
    void removeSend(int channelIndex, int busIndex);
    void setSendLevel(int channelIndex, int busIndex, float level);

    // Bus management
    int addBus(BusType type, const juce::String& name);
    void removeBus(int index);
    Bus& getBus(int index);
    const Bus& getBus(int index) const;
    int getNumBuses() const { return static_cast<int>(buses.size()); }
    
    void setBusName(int index, const juce::String& name);
    void setBusOutput(int index, int outputBus);
    void addBusSource(int busIndex, int sourceIndex);
    void removeBusSource(int busIndex, int sourceIndex);
    
    juce::StringArray getBusNames() const;
    std::vector<int> getBusSources(int index) const;
    int getBusOutput(int index) const;

    // Plugin management
    void addPlugin(int channelIndex, std::unique_ptr<Plugin> plugin);
    void removePlugin(int channelIndex, int pluginIndex);
    void movePlugin(int channelIndex, int fromIndex, int toIndex);
    Plugin* getPlugin(int channelIndex, int pluginIndex);
    int getNumPlugins(int channelIndex) const;

    // Processing
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages);
    void releaseResources();

    // State management
    void saveState(juce::ValueTree& state) const;
    void loadState(const juce::ValueTree& state);

private:
    // Project reference
    Project* currentProject{nullptr};
    
    // Channels and buses
    std::vector<Channel> channels;
    std::vector<Bus> buses;
    Channel masterChannel;
    
    // Processing buffers
    std::vector<juce::AudioBuffer<float>> channelBuffers;
    std::vector<juce::AudioBuffer<float>> busBuffers;
    juce::AudioBuffer<float> masterBuffer;
    
    // Processing state
    double currentSampleRate{44100.0};
    int currentBlockSize{512};
    bool processingPrepared{false};
    
    // Solo state
    bool soloActive{false};
    std::vector<bool> channelSoloBuffer;
    
    // Internal helpers
    void updateProcessingBuffers();
    void clearAllBuffers();
    void processChannels(juce::AudioBuffer<float>& buffer,
                        juce::MidiBuffer& midiMessages);
    void processBuses();
    void processMaster(juce::AudioBuffer<float>& buffer);
    
    void updatePeakAndRMSLevels(const juce::AudioBuffer<float>& buffer,
                               float& peak, float& rms);
    void applyChannelSettings(juce::AudioBuffer<float>& buffer,
                            const Channel& channel);
    void processSends(const juce::AudioBuffer<float>& source,
                     const Channel& channel);
    
    void updateSoloStates();
    bool isChannelActive(int index) const;
    
    static float calculatePanGain(float pan, bool leftChannel);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Mixer)
};

// Mixer utilities
namespace MixerUtils {
    // Level conversion
    float dbToGain(float db);
    float gainToDb(float gain);
    float velocityToGain(int velocity);
    
    // Pan law
    float panToGain(float pan, bool leftChannel);
    juce::Array<float> calculatePanLaw(int numSteps);
    
    // Level measurement
    float calculateRMSLevel(const float* data, int numSamples);
    float calculatePeakLevel(const float* data, int numSamples);
    
    // Buffer operations
    void applyGainRamp(juce::AudioBuffer<float>& buffer,
                      int startSample,
                      int numSamples,
                      float startGain,
                      float endGain);
                      
    void mixBuffers(const juce::AudioBuffer<float>& source,
                   juce::AudioBuffer<float>& destination,
                   float gain = 1.0f);
                   
    void copyWithGain(const juce::AudioBuffer<float>& source,
                     juce::AudioBuffer<float>& destination,
                     float gain);
}