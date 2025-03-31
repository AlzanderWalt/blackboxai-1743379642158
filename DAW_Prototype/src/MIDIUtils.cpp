#include "MIDIUtils.h"
#include "Logger.h"

// Static member initialization
const char* const MIDIUtils::NOTE_NAMES[] = {
    "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B"
};

const char* const MIDIUtils::NOTE_NAMES_FLAT[] = {
    "C", "Db", "D", "Eb", "E", "F", "Gb", "G", "Ab", "A", "Bb", "B"
};

const MIDIUtils::ControllerInfo MIDIUtils::CONTROLLER_INFO[] = {
    { BANK_SELECT_MSB, "Bank Select (MSB)" },
    { MODULATION, "Modulation" },
    { BREATH_CONTROLLER, "Breath Controller" },
    { FOOT_CONTROLLER, "Foot Controller" },
    { PORTAMENTO_TIME, "Portamento Time" },
    { VOLUME, "Volume" },
    { BALANCE, "Balance" },
    { PAN, "Pan" },
    { EXPRESSION, "Expression" },
    { SUSTAIN_PEDAL, "Sustain Pedal" },
    { PORTAMENTO_SWITCH, "Portamento Switch" },
    { SOSTENUTO_PEDAL, "Sostenuto Pedal" },
    { SOFT_PEDAL, "Soft Pedal" },
    { BANK_SELECT_LSB, "Bank Select (LSB)" }
};

juce::MidiMessage MIDIUtils::createNoteOn(int channel, int noteNumber,
                                         uint8_t velocity) {
    return juce::MidiMessage::noteOn(channel, noteNumber, velocity);
}

juce::MidiMessage MIDIUtils::createNoteOff(int channel, int noteNumber,
                                          uint8_t velocity) {
    return juce::MidiMessage::noteOff(channel, noteNumber, velocity);
}

juce::MidiMessage MIDIUtils::createPitchBend(int channel, int value) {
    return juce::MidiMessage::pitchWheel(channel, value);
}

juce::MidiMessage MIDIUtils::createControlChange(int channel, int controller,
                                                int value) {
    return juce::MidiMessage::controllerEvent(channel, controller, value);
}

juce::MidiMessage MIDIUtils::createProgramChange(int channel, int program) {
    return juce::MidiMessage::programChange(channel, program);
}

juce::MidiMessage MIDIUtils::createAftertouch(int channel, int noteNumber,
                                             int pressure) {
    return juce::MidiMessage::aftertouchChange(channel, noteNumber, pressure);
}

juce::MidiMessage MIDIUtils::createChannelPressure(int channel, int pressure) {
    return juce::MidiMessage::channelPressureChange(channel, pressure);
}

bool MIDIUtils::isNoteMessage(const juce::MidiMessage& message) {
    return message.isNoteOn() || message.isNoteOff();
}

bool MIDIUtils::isControllerMessage(const juce::MidiMessage& message) {
    return message.isController();
}

bool MIDIUtils::isProgramChangeMessage(const juce::MidiMessage& message) {
    return message.isProgramChange();
}

bool MIDIUtils::isPitchBendMessage(const juce::MidiMessage& message) {
    return message.isPitchWheel();
}

bool MIDIUtils::isAftertouchMessage(const juce::MidiMessage& message) {
    return message.isAftertouch();
}

bool MIDIUtils::isChannelPressureMessage(const juce::MidiMessage& message) {
    return message.isChannelPressure();
}

bool MIDIUtils::isSystemMessage(const juce::MidiMessage& message) {
    return message.isSystemMessage();
}

bool MIDIUtils::saveMidiFile(const juce::File& file,
                            const juce::MidiMessageSequence& sequence,
                            int format,
                            short timeFormat) {
    juce::MidiFile midiFile;
    midiFile.setTicksPerQuarterNote(timeFormat);
    midiFile.addTrack(sequence);
    
    if (auto stream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream())) {
        midiFile.writeTo(*stream);
        return true;
    }
    return false;
}

bool MIDIUtils::loadMidiFile(const juce::File& file,
                            juce::MidiMessageSequence& sequence) {
    juce::MidiFile midiFile;
    if (auto stream = std::unique_ptr<juce::FileInputStream>(file.createInputStream())) {
        if (midiFile.readFrom(*stream)) {
            sequence.clear();
            if (midiFile.getNumTracks() > 0) {
                sequence = *midiFile.getTrack(0);
                return true;
            }
        }
    }
    return false;
}

void MIDIUtils::transposeNotes(juce::MidiMessageSequence& sequence,
                              int semitones) {
    for (int i = 0; i < sequence.getNumEvents(); ++i) {
        auto event = sequence.getEventPointer(i);
        if (event->message.isNoteOnOrOff()) {
            const int newNote = juce::jlimit(0, 127,
                event->message.getNoteNumber() + semitones);
            event->message.setNoteNumber(newNote);
        }
    }
}

void MIDIUtils::quantizeNotes(juce::MidiMessageSequence& sequence,
                             double gridSize,
                             float amount) {
    for (int i = 0; i < sequence.getNumEvents(); ++i) {
        auto event = sequence.getEventPointer(i);
        if (event->message.isNoteOn()) {
            const double time = event->message.getTimeStamp();
            const double quantizedTime = std::round(time / gridSize) * gridSize;
            const double newTime = time + (quantizedTime - time) * amount;
            
            event->message.setTimeStamp(newTime);
            if (auto noteOff = event->noteOffObject) {
                const double duration = noteOff->message.getTimeStamp() - time;
                noteOff->message.setTimeStamp(newTime + duration);
            }
        }
    }
    sequence.updateMatchedPairs();
}

void MIDIUtils::adjustVelocities(juce::MidiMessageSequence& sequence,
                                float multiplier,
                                float offset) {
    for (int i = 0; i < sequence.getNumEvents(); ++i) {
        auto event = sequence.getEventPointer(i);
        if (event->message.isNoteOn()) {
            const float velocity = event->message.getVelocity() * multiplier + offset;
            event->message.setVelocity(juce::jlimit(0.0f, 127.0f, velocity));
        }
    }
}

void MIDIUtils::adjustTimings(juce::MidiMessageSequence& sequence,
                             double factor) {
    for (int i = 0; i < sequence.getNumEvents(); ++i) {
        auto event = sequence.getEventPointer(i);
        event->message.setTimeStamp(event->message.getTimeStamp() * factor);
    }
}

void MIDIUtils::filterEvents(juce::MidiMessageSequence& sequence,
                            const std::function<bool(const juce::MidiMessage&)>& predicate) {
    sequence.deleteEvent(predicate, true);
}

void MIDIUtils::filterChannel(juce::MidiMessageSequence& sequence,
                             int channel) {
    filterEvents(sequence, [channel](const juce::MidiMessage& m) {
        return m.getChannel() != channel;
    });
}

void MIDIUtils::filterNoteRange(juce::MidiMessageSequence& sequence,
                               int lowestNote,
                               int highestNote) {
    filterEvents(sequence, [lowestNote, highestNote](const juce::MidiMessage& m) {
        return m.isNoteOnOrOff() &&
               (m.getNoteNumber() < lowestNote || m.getNoteNumber() > highestNote);
    });
}

void MIDIUtils::filterControllers(juce::MidiMessageSequence& sequence,
                                 const juce::Array<int>& controllers) {
    filterEvents(sequence, [&controllers](const juce::MidiMessage& m) {
        return m.isController() &&
               !controllers.contains(m.getControllerNumber());
    });
}

double MIDIUtils::ticksToBeats(long ticks, int ppq) {
    return static_cast<double>(ticks) / ppq;
}

long MIDIUtils::beatsToTicks(double beats, int ppq) {
    return static_cast<long>(beats * ppq);
}

double MIDIUtils::ticksToSeconds(long ticks, int ppq, double tempo) {
    return (static_cast<double>(ticks) * 60.0) / (ppq * tempo);
}

long MIDIUtils::secondsToTicks(double seconds, int ppq, double tempo) {
    return static_cast<long>((seconds * ppq * tempo) / 60.0);
}

juce::String MIDIUtils::getMidiNoteName(int noteNumber,
                                       bool includeOctave,
                                       bool sharps) {
    if (noteNumber < 0 || noteNumber > 127) {
        return "Invalid";
    }

    const int octave = (noteNumber / 12) - 1;
    const int note = noteNumber % 12;
    const auto& names = sharps ? NOTE_NAMES : NOTE_NAMES_FLAT;
    
    return includeOctave
        ? names[note] + juce::String(octave)
        : names[note];
}

int MIDIUtils::getMidiNoteFromName(const juce::String& noteName) {
    // Parse note name (e.g., "C4", "F#3", "Bb5")
    if (noteName.length() < 2) return -1;
    
    const juce::String notePart = noteName.substring(0, noteName.length() - 1);
    const int octave = noteName.getLastCharacter().getNumericValue();
    
    if (octave < -1 || octave > 9) return -1;
    
    int noteNumber = -1;
    for (int i = 0; i < 12; ++i) {
        if (notePart == NOTE_NAMES[i] || notePart == NOTE_NAMES_FLAT[i]) {
            noteNumber = i;
            break;
        }
    }
    
    if (noteNumber == -1) return -1;
    
    return noteNumber + ((octave + 1) * 12);
}

bool MIDIUtils::isBlackNote(int noteNumber) {
    static const bool isBlack[] = { false, true, false, true, false, false, true, false, true, false, true, false };
    return isBlack[noteNumber % 12];
}

int MIDIUtils::getNoteOctave(int noteNumber) {
    return (noteNumber / 12) - 1;
}

juce::String MIDIUtils::getControllerName(int controller) {
    for (const auto& info : CONTROLLER_INFO) {
        if (info.number == controller) {
            return info.name;
        }
    }
    return "CC " + juce::String(controller);
}

int MIDIUtils::getControllerNumber(const juce::String& name) {
    for (const auto& info : CONTROLLER_INFO) {
        if (name == info.name) {
            return info.number;
        }
    }
    return -1;
}

bool MIDIUtils::isValidController(int controller) {
    return controller >= 0 && controller <= 127;
}

juce::StringArray MIDIUtils::getAvailableInputDevices() {
    return juce::MidiInput::getDevices();
}

juce::StringArray MIDIUtils::getAvailableOutputDevices() {
    return juce::MidiOutput::getDevices();
}

bool MIDIUtils::openMidiInput(juce::MidiInput* input, int index,
                             juce::MidiInputCallback* callback) {
    if (input != nullptr && callback != nullptr) {
        input->stop();
        return input->start();
    }
    return false;
}

bool MIDIUtils::openMidiOutput(juce::MidiOutput* output, int index) {
    return output != nullptr;
}

void MIDIUtils::processMidiThru(const juce::MidiMessage& message,
                               juce::MidiOutput* output,
                               bool filterChannel,
                               int channel) {
    if (output != nullptr) {
        if (!filterChannel || message.getChannel() == channel) {
            output->sendMessageNow(message);
        }
    }
}

juce::MidiMessage MIDIUtils::createMidiClock() {
    return juce::MidiMessage(MIDI_CLOCK);
}

juce::MidiMessage MIDIUtils::createMidiStart() {
    return juce::MidiMessage(MIDI_START);
}

juce::MidiMessage MIDIUtils::createMidiStop() {
    return juce::MidiMessage(MIDI_STOP);
}

juce::MidiMessage MIDIUtils::createMidiContinue() {
    return juce::MidiMessage(MIDI_CONTINUE);
}

bool MIDIUtils::isMidiClockMessage(const juce::MidiMessage& message) {
    return message.getRawData()[0] == MIDI_CLOCK;
}

bool MIDIUtils::isMidiStartMessage(const juce::MidiMessage& message) {
    return message.getRawData()[0] == MIDI_START;
}

bool MIDIUtils::isMidiStopMessage(const juce::MidiMessage& message) {
    return message.getRawData()[0] == MIDI_STOP;
}

bool MIDIUtils::isMidiContinueMessage(const juce::MidiMessage& message) {
    return message.getRawData()[0] == MIDI_CONTINUE;
}

juce::MidiMessage MIDIUtils::createFullFrameMessage(int hours, int minutes,
                                                   int seconds, int frames,
                                                   int frameRate) {
    uint8_t data[10] = { MIDI_MTC_FULL_FRAME, 0x7F, 0x01, 0x01,
                         static_cast<uint8_t>(frameRate),
                         static_cast<uint8_t>(hours),
                         static_cast<uint8_t>(minutes),
                         static_cast<uint8_t>(seconds),
                         static_cast<uint8_t>(frames),
                         0xF7 };
    return juce::MidiMessage(data, 10);
}

void MIDIUtils::parseMTCFullFrame(const juce::MidiMessage& message,
                                 int& hours, int& minutes,
                                 int& seconds, int& frames,
                                 int& frameRate) {
    if (isMTCFullFrameMessage(message)) {
        const uint8_t* data = message.getRawData();
        frameRate = data[4];
        hours = data[5];
        minutes = data[6];
        seconds = data[7];
        frames = data[8];
    }
}

bool MIDIUtils::isMTCMessage(const juce::MidiMessage& message) {
    const uint8_t status = message.getRawData()[0];
    return status == MIDI_MTC_QUARTER_FRAME || status == MIDI_MTC_FULL_FRAME;
}

bool MIDIUtils::isMTCFullFrameMessage(const juce::MidiMessage& message) {
    return message.getRawData()[0] == MIDI_MTC_FULL_FRAME;
}