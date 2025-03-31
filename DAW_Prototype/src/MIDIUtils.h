#pragma once
#include <JuceHeader.h>

class MIDIUtils {
public:
    // MIDI message creation
    static juce::MidiMessage createNoteOn(int channel, int noteNumber,
                                        uint8_t velocity);
    static juce::MidiMessage createNoteOff(int channel, int noteNumber,
                                         uint8_t velocity = 0);
    static juce::MidiMessage createPitchBend(int channel, int value);
    static juce::MidiMessage createControlChange(int channel, int controller,
                                               int value);
    static juce::MidiMessage createProgramChange(int channel, int program);
    static juce::MidiMessage createAftertouch(int channel, int noteNumber,
                                            int pressure);
    static juce::MidiMessage createChannelPressure(int channel, int pressure);
    
    // MIDI message analysis
    static bool isNoteMessage(const juce::MidiMessage& message);
    static bool isControllerMessage(const juce::MidiMessage& message);
    static bool isProgramChangeMessage(const juce::MidiMessage& message);
    static bool isPitchBendMessage(const juce::MidiMessage& message);
    static bool isAftertouchMessage(const juce::MidiMessage& message);
    static bool isChannelPressureMessage(const juce::MidiMessage& message);
    static bool isSystemMessage(const juce::MidiMessage& message);
    
    // MIDI file utilities
    static bool saveMidiFile(const juce::File& file,
                           const juce::MidiMessageSequence& sequence,
                           int format = 1,
                           short timeFormat = 960);
    static bool loadMidiFile(const juce::File& file,
                           juce::MidiMessageSequence& sequence);
    
    // MIDI data manipulation
    static void transposeNotes(juce::MidiMessageSequence& sequence,
                             int semitones);
    static void quantizeNotes(juce::MidiMessageSequence& sequence,
                            double gridSize,
                            float amount = 1.0f);
    static void adjustVelocities(juce::MidiMessageSequence& sequence,
                               float multiplier,
                               float offset = 0.0f);
    static void adjustTimings(juce::MidiMessageSequence& sequence,
                            double factor);
    
    // MIDI event filtering
    static void filterEvents(juce::MidiMessageSequence& sequence,
                           const std::function<bool(const juce::MidiMessage&)>& predicate);
    static void filterChannel(juce::MidiMessageSequence& sequence,
                            int channel);
    static void filterNoteRange(juce::MidiMessageSequence& sequence,
                              int lowestNote,
                              int highestNote);
    static void filterControllers(juce::MidiMessageSequence& sequence,
                                const juce::Array<int>& controllers);
    
    // MIDI time conversion
    static double ticksToBeats(long ticks, int ppq);
    static long beatsToTicks(double beats, int ppq);
    static double ticksToSeconds(long ticks, int ppq, double tempo);
    static long secondsToTicks(double seconds, int ppq, double tempo);
    
    // MIDI note utilities
    static juce::String getMidiNoteName(int noteNumber,
                                      bool includeOctave = true,
                                      bool sharps = true);
    static int getMidiNoteFromName(const juce::String& noteName);
    static bool isBlackNote(int noteNumber);
    static int getNoteOctave(int noteNumber);
    
    // Controller names and mappings
    static juce::String getControllerName(int controller);
    static int getControllerNumber(const juce::String& name);
    static bool isValidController(int controller);
    
    // MIDI device utilities
    static juce::StringArray getAvailableInputDevices();
    static juce::StringArray getAvailableOutputDevices();
    static bool openMidiInput(juce::MidiInput* input, int index,
                            juce::MidiInputCallback* callback);
    static bool openMidiOutput(juce::MidiOutput* output, int index);
    
    // MIDI thru handling
    static void processMidiThru(const juce::MidiMessage& message,
                              juce::MidiOutput* output,
                              bool filterChannel = false,
                              int channel = 1);
    
    // MIDI clock utilities
    static juce::MidiMessage createMidiClock();
    static juce::MidiMessage createMidiStart();
    static juce::MidiMessage createMidiStop();
    static juce::MidiMessage createMidiContinue();
    static bool isMidiClockMessage(const juce::MidiMessage& message);
    static bool isMidiStartMessage(const juce::MidiMessage& message);
    static bool isMidiStopMessage(const juce::MidiMessage& message);
    static bool isMidiContinueMessage(const juce::MidiMessage& message);
    
    // MIDI time code (MTC) utilities
    static juce::MidiMessage createFullFrameMessage(int hours, int minutes,
                                                  int seconds, int frames,
                                                  int frameRate);
    static void parseMTCFullFrame(const juce::MidiMessage& message,
                                int& hours, int& minutes,
                                int& seconds, int& frames,
                                int& frameRate);
    static bool isMTCMessage(const juce::MidiMessage& message);
    static bool isMTCFullFrameMessage(const juce::MidiMessage& message);

private:
    // Constants
    static constexpr int MIDI_CLOCK = 0xF8;
    static constexpr int MIDI_START = 0xFA;
    static constexpr int MIDI_CONTINUE = 0xFB;
    static constexpr int MIDI_STOP = 0xFC;
    static constexpr int MIDI_MTC_QUARTER_FRAME = 0xF1;
    static constexpr int MIDI_MTC_FULL_FRAME = 0xF0;
    
    // Controller numbers
    static constexpr int BANK_SELECT_MSB = 0;
    static constexpr int MODULATION = 1;
    static constexpr int BREATH_CONTROLLER = 2;
    static constexpr int FOOT_CONTROLLER = 4;
    static constexpr int PORTAMENTO_TIME = 5;
    static constexpr int VOLUME = 7;
    static constexpr int BALANCE = 8;
    static constexpr int PAN = 10;
    static constexpr int EXPRESSION = 11;
    static constexpr int SUSTAIN_PEDAL = 64;
    static constexpr int PORTAMENTO_SWITCH = 65;
    static constexpr int SOSTENUTO_PEDAL = 66;
    static constexpr int SOFT_PEDAL = 67;
    static constexpr int BANK_SELECT_LSB = 32;
    
    // Note names
    static const char* const NOTE_NAMES[];
    static const char* const NOTE_NAMES_FLAT[];
    
    // Controller names
    struct ControllerInfo {
        int number;
        const char* name;
    };
    static const ControllerInfo CONTROLLER_INFO[];
    
    // Make constructor private to prevent instantiation
    MIDIUtils() = delete;
};