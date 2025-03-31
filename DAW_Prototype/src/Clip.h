#pragma once
#include <JuceHeader.h>

class Clip : public juce::ChangeBroadcaster {
public:
    enum class Type {
        Audio,
        MIDI
    };

    // Constructor/Destructor
    Clip(Type type = Type::Audio);
    ~Clip() override;

    // Basic properties
    const juce::String& getID() const { return id; }
    const juce::String& getName() const { return name; }
    void setName(const juce::String& newName);
    Type getType() const { return type; }

    // Time properties
    double getStartTime() const { return startTime; }
    void setStartTime(double newTime);
    double getLength() const { return length; }
    void setLength(double newLength);
    double getOffset() const { return offset; }
    void setOffset(double newOffset);
    bool containsTime(double time) const;

    // Audio specific
    void setAudioFile(const juce::File& file);
    const juce::File& getAudioFile() const { return audioFile; }
    void setGain(float newGain);
    float getGain() const { return gain; }
    void setPitch(float newPitch);
    float getPitch() const { return pitch; }
    void setStretch(float newStretch);
    float getStretch() const { return stretch; }
    void setFadeIn(double length);
    double getFadeIn() const { return fadeIn; }
    void setFadeOut(double length);
    double getFadeOut() const { return fadeOut; }

    // MIDI specific
    void addMIDINote(int note, int velocity, double startBeat, double lengthInBeats);
    void removeMIDINote(int note, double startBeat);
    void clearMIDINotes();
    const juce::MidiMessageSequence& getMIDISequence() const { return midiSequence; }
    void setMIDIChannel(int channel);
    int getMIDIChannel() const { return midiChannel; }
    void setVelocityOffset(int offset);
    int getVelocityOffset() const { return velocityOffset; }
    void transposeNotes(int semitones);
    void quantizeNotes(double gridSize, float amount = 1.0f);

    // Processing
    void prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock);
    void processBlock(juce::AudioBuffer<float>& buffer,
                     juce::MidiBuffer& midiMessages,
                     double currentTime,
                     double blockDuration);
    void releaseResources();

    // State management
    void saveState(juce::ValueTree& state) const;
    void restoreState(const juce::ValueTree& state);
    juce::ValueTree getState() const;

    // Color
    void setColor(juce::Colour newColor);
    juce::Colour getColor() const { return color; }

private:
    Type type;
    juce::String id;
    juce::String name;
    juce::Colour color;
    
    double startTime{0.0};
    double length{0.0};
    double offset{0.0};
    
    // Audio properties
    juce::File audioFile;
    std::unique_ptr<juce::AudioFormatReader> audioReader;
    float gain{1.0f};
    float pitch{0.0f};
    float stretch{1.0f};
    double fadeIn{0.0};
    double fadeOut{0.0};
    
    // MIDI properties
    juce::MidiMessageSequence midiSequence;
    int midiChannel{1};
    int velocityOffset{0};
    
    // Processing
    double sampleRate{44100.0};
    int blockSize{512};
    juce::AudioBuffer<float> processBuffer;
    
    void generateID();
    void notifyClipChanged();
    float calculateGainAt(double time) const;
    void readAudioBlock(juce::AudioBuffer<float>& buffer,
                       double clipRelativeTime,
                       int numSamples);
    void processMIDIBlock(juce::MidiBuffer& midiMessages,
                         double clipRelativeTime,
                         double blockDuration);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Clip)
};