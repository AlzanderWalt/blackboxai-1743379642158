#pragma once
#include <JuceHeader.h>
#include "Track.h"
#include "Plugin.h"

class Project;

class AudioEngine : public juce::AudioIODeviceCallback,
                   public juce::MidiInputCallback,
                   public juce::ChangeBroadcaster {
public:
    // Audio settings
    struct Settings {
        double sampleRate{44100.0};
        int bufferSize{512};
        int inputChannels{2};
        int outputChannels{2};
        juce::String inputDevice;
        juce::String outputDevice;
        bool useAsioDriver{false};
    };

    // Transport state
    struct TransportState {
        bool isPlaying{false};
        bool isRecording{false};
        bool isLooping{false};
        double bpm{120.0};
        double position{0.0};
        double loopStart{0.0};
        double loopEnd{0.0};
        juce::AudioPlayHead::TimeSignature timeSignature{4, 4};
    };

    // CPU usage info
    struct CPUInfo {
        float averageLoad{0.0f};
        float peakLoad{0.0f};
        float currentLoad{0.0f};
        int xruns{0};
    };

    // Constructor/Destructor
    AudioEngine();
    ~AudioEngine() override;

    // Singleton access
    static AudioEngine& getInstance();

    // Device management
    bool initialize();
    void shutdown();
    bool isInitialized() const { return initialized; }
    
    const Settings& getSettings() const { return settings; }
    bool applySettings(const Settings& newSettings);
    
    juce::StringArray getAvailableDevices() const;
    juce::String getCurrentDeviceName() const;
    
    const CPUInfo& getCPUInfo() const { return cpuInfo; }

    // Project handling
    void setProject(Project* project);
    Project* getProject() const { return currentProject; }

    // Transport control
    void play();
    void stop();
    void record();
    void setPosition(double timeInSeconds);
    void setLoopPoints(double startTime, double endTime);
    void setLooping(bool shouldLoop);
    void setBpm(double newBpm);
    void setTimeSignature(int numerator, int denominator);
    
    const TransportState& getTransportState() const { return transport; }
    double getCurrentPosition() const { return transport.position; }

    // MIDI handling
    void addMidiInputDevice(const juce::String& deviceName);
    void removeMidiInputDevice(const juce::String& deviceName);
    juce::StringArray getMidiInputDevices() const;
    
    void handleIncomingMidiMessage(juce::MidiInput* source,
                                 const juce::MidiMessage& message) override;

    // AudioIODeviceCallback interface
    void audioDeviceIOCallback(const float** inputChannelData,
                             int numInputChannels,
                             float** outputChannelData,
                             int numOutputChannels,
                             int numSamples) override;
                             
    void audioDeviceAboutToStart(juce::AudioIODevice* device) override;
    void audioDeviceStopped() override;
    void audioDeviceError(const juce::String& errorMessage) override;

private:
    // Device management
    std::unique_ptr<juce::AudioDeviceManager> deviceManager;
    Settings settings;
    bool initialized{false};
    
    // Project reference
    Project* currentProject{nullptr};
    
    // Transport state
    TransportState transport;
    std::atomic<double> nextPosition{0.0};
    
    // Processing state
    juce::AudioBuffer<float> inputBuffer;
    juce::AudioBuffer<float> outputBuffer;
    juce::MidiBuffer midiBuffer;
    
    // Performance monitoring
    CPUInfo cpuInfo;
    juce::Time lastProcessTime;
    
    // MIDI devices
    juce::OwnedArray<juce::MidiInput> midiInputs;
    juce::MidiBuffer incomingMidi;
    juce::CriticalSection midiLock;
    
    // Internal helpers
    void processAudioBlock(const float** inputChannelData,
                         int numInputChannels,
                         float** outputChannelData,
                         int numOutputChannels,
                         int numSamples);
                         
    void processMidiBlock(int numSamples);
    void updateTransportPosition(int numSamples);
    void handleXRun();
    void updateCPUInfo(double processingTimeMs);
    
    void initializeBuffers();
    void clearBuffers();
    bool setupAudioDevice();
    void cleanupAudioDevice();
    
    static void audioDeviceErrorCallback(const juce::String& errorMessage);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(AudioEngine)
};

// Audio engine utilities
namespace AudioEngineUtils {
    // Time conversion
    double samplesToTime(int64_t samples, double sampleRate);
    int64_t timeToSamples(double timeInSeconds, double sampleRate);
    double ppqToTime(double ppq, double bpm);
    double timeToPpq(double timeInSeconds, double bpm);
    
    // Buffer utilities
    void applyGainRamp(juce::AudioBuffer<float>& buffer,
                      int startSample,
                      int numSamples,
                      float startGain,
                      float endGain);
                      
    void crossfadeBuffers(juce::AudioBuffer<float>& buffer1,
                         juce::AudioBuffer<float>& buffer2,
                         int crossfadeLength);
                         
    // Performance monitoring
    float calculateCPULoad(double processingTimeMs, double bufferTimeMs);
    void detectXRun(double processingTimeMs, double bufferTimeMs);
    
    // Device configuration
    juce::String getDefaultDeviceName();
    int getDefaultBufferSize();
    bool isAsioAvailable();
}