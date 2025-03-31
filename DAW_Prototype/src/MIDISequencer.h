#pragma once
#include <JuceHeader.h>

class Track;
class Project;

class MIDISequencer : public juce::ChangeBroadcaster {
public:
    // MIDI recording settings
    struct RecordingSettings {
        bool quantizeInput{false};
        double quantizeGrid{0.25};  // Quarter note
        bool autoQuantize{false};
        bool replaceMode{false};
        bool overdubMode{false};
        int velocityMode{0};  // 0 = as played, 1 = fixed, 2 = scaled
        float velocityValue{100};
        float velocityScale{1.0f};
        bool filterChannels{false};
        juce::BigInteger activeChannels;
        bool filterNotes{false};
        juce::BigInteger activeNotes;
    };

    // MIDI playback settings
    struct PlaybackSettings {
        bool midiThru{true};
        bool sendClock{false};
        bool sendMTC{false};
        int mtcFormat{0};  // 0 = 24fps, 1 = 25fps, 2 = 30fps drop, 3 = 30fps
        bool sendMMC{false};
        bool sendProgramChanges{true};
        bool sendControlChanges{true};
        bool sendSysEx{true};
    };

    // Constructor/Destructor
    MIDISequencer();
    ~MIDISequencer() override;

    // Project handling
    void setProject(Project* project);
    Project* getProject() const { return currentProject; }

    // Recording settings
    const RecordingSettings& getRecordingSettings() const { return recordSettings; }
    void setRecordingSettings(const RecordingSettings& settings);
    
    void setQuantizeInput(bool shouldQuantize);
    void setQuantizeGrid(double grid);
    void setAutoQuantize(bool shouldAutoQuantize);
    void setReplaceMode(bool shouldReplace);
    void setOverdubMode(bool shouldOverdub);
    void setVelocityMode(int mode);
    void setVelocityValue(float value);
    void setVelocityScale(float scale);
    void setActiveChannels(const juce::BigInteger& channels);
    void setActiveNotes(const juce::BigInteger& notes);

    // Playback settings
    const PlaybackSettings& getPlaybackSettings() const { return playbackSettings; }
    void setPlaybackSettings(const PlaybackSettings& settings);
    
    void setMidiThru(bool shouldThru);
    void setSendClock(bool shouldSend);
    void setSendMTC(bool shouldSend);
    void setMTCFormat(int format);
    void setSendMMC(bool shouldSend);
    void setSendProgramChanges(bool shouldSend);
    void setSendControlChanges(bool shouldSend);
    void setSendSysEx(bool shouldSend);

    // Recording control
    void startRecording(Track* track);
    void stopRecording();
    bool isRecording() const { return recording; }
    Track* getRecordingTrack() const { return recordingTrack; }

    // MIDI input handling
    void handleIncomingMidiMessage(const juce::MidiMessage& message);
    void processInputBuffer(juce::MidiBuffer& buffer);

    // MIDI output handling
    void processOutputBuffer(juce::MidiBuffer& buffer, double position);
    void sendMidiClock(double position);
    void sendMidiTimeCode(double position);
    void sendMidiMachineControl(int command);

    // Event processing
    void quantizeEvents(juce::MidiMessageSequence& sequence, double grid);
    void transposeEvents(juce::MidiMessageSequence& sequence, int semitones);
    void scaleVelocities(juce::MidiMessageSequence& sequence, float scale);
    void filterEvents(juce::MidiMessageSequence& sequence,
                     const juce::BigInteger& channels,
                     const juce::BigInteger& notes);

    // State management
    void saveState(juce::ValueTree& state) const;
    void loadState(const juce::ValueTree& state);

private:
    // Project reference
    Project* currentProject{nullptr};
    
    // Settings
    RecordingSettings recordSettings;
    PlaybackSettings playbackSettings;
    
    // Recording state
    bool recording{false};
    Track* recordingTrack{nullptr};
    juce::MidiMessageSequence recordingSequence;
    double recordingStartTime{0.0};
    
    // MIDI processing
    juce::MidiBuffer inputBuffer;
    juce::MidiBuffer outputBuffer;
    juce::CriticalSection midiLock;
    
    // MIDI timing
    double lastClockTime{0.0};
    double lastMTCTime{0.0};
    int mtcFrameCount{0};
    
    // Internal helpers
    void processRecordedMessage(const juce::MidiMessage& message, double time);
    void finalizeRecording();
    void clearRecordingState();
    
    double quantizeTime(double time) const;
    int processVelocity(int velocity) const;
    bool shouldProcessMessage(const juce::MidiMessage& message) const;
    
    static juce::String getMTCString(double time, int format);
    static int getMTCFrameRate(int format);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MIDISequencer)
};

// MIDI sequencer utilities
namespace MIDISequencerUtils {
    // MIDI file handling
    bool importMidiFile(const juce::File& file, juce::MidiMessageSequence& sequence);
    bool exportMidiFile(const juce::File& file, const juce::MidiMessageSequence& sequence);
    
    // Event manipulation
    void mergeMidiSequences(juce::MidiMessageSequence& dest,
                          const juce::MidiMessageSequence& source);
    void splitMidiSequence(const juce::MidiMessageSequence& source,
                         double splitTime,
                         juce::MidiMessageSequence& left,
                         juce::MidiMessageSequence& right);
    
    // Time conversion
    double ticksToTime(int ticks, int ppq, double bpm);
    int timeToTicks(double time, int ppq, double bpm);
    
    // Event filtering
    void filterChannelMessages(juce::MidiMessageSequence& sequence, int channel);
    void filterNoteMessages(juce::MidiMessageSequence& sequence,
                          const juce::Array<int>& notes);
    void filterControllerMessages(juce::MidiMessageSequence& sequence,
                                const juce::Array<int>& controllers);
    
    // Event generation
    void generateMidiClock(juce::MidiMessageSequence& sequence,
                         double duration,
                         double bpm);
    void generateMidiTimeCode(juce::MidiMessageSequence& sequence,
                           double duration,
                           int format);
}