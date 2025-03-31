#pragma once
#include <JuceHeader.h>
#include "Plugin.h"
#include "Clip.h"

class Track : public juce::ChangeBroadcaster {
public:
    enum class Type {
        Audio,
        MIDI,
        Bus,
        Master
    };

    struct Parameters {
        float volume{1.0f};
        float pan{0.0f};
        bool mute{false};
        bool solo{false};
        bool record{false};
        bool monitoring{false};
        int height{100};
        
        struct Input {
            juce::String device;
            int channel{1};
        } input;
        
        struct Output {
            juce::String bus{"master"};
            int channel{1};
        } output;
    };

    // Constructor/Destructor
    Track(Type type);
    ~Track() override;

    // Basic properties
    const juce::String& getID() const { return id; }
    const juce::String& getName() const { return name; }
    void setName(const juce::String& newName);
    Type getType() const { return type; }

    // Parameters
    const Parameters& getParameters() const { return parameters; }
    void setParameters(const Parameters& newParams);
    void setVolume(float newVolume);
    void setPan(float newPan);
    void setMute(bool shouldMute);
    void setSolo(bool shouldSolo);
    void setRecord(bool shouldRecord);
    void setMonitoring(bool shouldMonitor);
    void setHeight(int newHeight);
    void setInputDevice(const juce::String& device);
    void setInputChannel(int channel);
    void setOutputBus(const juce::String& bus);
    void setOutputChannel(int channel);

    // Plugin management
    void addPlugin(const juce::String& pluginID);
    void removePlugin(int index);
    void movePlugin(int fromIndex, int toIndex);
    void bypassPlugin(int index, bool bypass);
    Plugin* getPlugin(int index) const;
    int getNumPlugins() const;
    const juce::OwnedArray<Plugin>& getPlugins() const { return plugins; }

    // Clip management
    void addClip(std::unique_ptr<Clip> clip);
    void removeClip(Clip* clip);
    void moveClip(Clip* clip, double newStartTime);
    Clip* getClipAt(double time) const;
    const juce::OwnedArray<Clip>& getClips() const { return clips; }

    // Automation
    void addAutomation(const juce::String& paramID);
    void removeAutomation(const juce::String& paramID);
    bool hasAutomation(const juce::String& paramID) const;
    void setAutomationValue(const juce::String& paramID, double time, float value);
    float getAutomationValue(const juce::String& paramID, double time) const;

    // Processing
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer,
                     juce::MidiBuffer& midiMessages);
    void releaseResources();

    // State management
    void saveState(juce::ValueTree& state) const;
    void restoreState(const juce::ValueTree& state);
    juce::ValueTree getState() const;

    // Freezing
    void freeze();
    void unfreeze();
    bool isFrozen() const { return frozen; }

private:
    Type type;
    juce::String id;
    juce::String name;
    Parameters parameters;
    
    juce::OwnedArray<Plugin> plugins;
    juce::OwnedArray<Clip> clips;
    
    struct AutomationData {
        juce::Array<double> times;
        juce::Array<float> values;
    };
    
    juce::HashMap<juce::String, AutomationData> automation;
    
    bool frozen{false};
    juce::AudioBuffer<float> frozenBuffer;
    juce::MidiBuffer frozenMidi;
    
    double sampleRate{44100.0};
    int blockSize{512};
    
    void generateID();
    void updateAutomation(double time);
    void notifyTrackChanged();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Track)
};