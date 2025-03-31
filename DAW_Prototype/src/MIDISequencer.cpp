#include "MIDISequencer.h"
#include "Project.h"
#include "Track.h"
#include "Logger.h"

//==============================================================================
// MIDISequencer Implementation
//==============================================================================

MIDISequencer::MIDISequencer() {
    // Initialize default settings
    recordSettings.activeChannels.setRange(0, 16, true);  // All channels active
    recordSettings.activeNotes.setRange(0, 128, true);    // All notes active
}

MIDISequencer::~MIDISequencer() {
    stopRecording();
}

void MIDISequencer::setProject(Project* project) {
    if (recording) {
        stopRecording();
    }
    
    currentProject = project;
    sendChangeMessage();
}

void MIDISequencer::setRecordingSettings(const RecordingSettings& settings) {
    recordSettings = settings;
    sendChangeMessage();
}

void MIDISequencer::setQuantizeInput(bool shouldQuantize) {
    if (recordSettings.quantizeInput != shouldQuantize) {
        recordSettings.quantizeInput = shouldQuantize;
        sendChangeMessage();
    }
}

void MIDISequencer::setQuantizeGrid(double grid) {
    if (recordSettings.quantizeGrid != grid) {
        recordSettings.quantizeGrid = grid;
        sendChangeMessage();
    }
}

void MIDISequencer::setAutoQuantize(bool shouldAutoQuantize) {
    if (recordSettings.autoQuantize != shouldAutoQuantize) {
        recordSettings.autoQuantize = shouldAutoQuantize;
        sendChangeMessage();
    }
}

void MIDISequencer::setReplaceMode(bool shouldReplace) {
    if (recordSettings.replaceMode != shouldReplace) {
        recordSettings.replaceMode = shouldReplace;
        sendChangeMessage();
    }
}

void MIDISequencer::setOverdubMode(bool shouldOverdub) {
    if (recordSettings.overdubMode != shouldOverdub) {
        recordSettings.overdubMode = shouldOverdub;
        sendChangeMessage();
    }
}

void MIDISequencer::setVelocityMode(int mode) {
    if (recordSettings.velocityMode != mode) {
        recordSettings.velocityMode = mode;
        sendChangeMessage();
    }
}

void MIDISequencer::setVelocityValue(float value) {
    if (recordSettings.velocityValue != value) {
        recordSettings.velocityValue = value;
        sendChangeMessage();
    }
}

void MIDISequencer::setVelocityScale(float scale) {
    if (recordSettings.velocityScale != scale) {
        recordSettings.velocityScale = scale;
        sendChangeMessage();
    }
}

void MIDISequencer::setActiveChannels(const juce::BigInteger& channels) {
    if (recordSettings.activeChannels != channels) {
        recordSettings.activeChannels = channels;
        sendChangeMessage();
    }
}

void MIDISequencer::setActiveNotes(const juce::BigInteger& notes) {
    if (recordSettings.activeNotes != notes) {
        recordSettings.activeNotes = notes;
        sendChangeMessage();
    }
}

void MIDISequencer::setPlaybackSettings(const PlaybackSettings& settings) {
    playbackSettings = settings;
    sendChangeMessage();
}

void MIDISequencer::setMidiThru(bool shouldThru) {
    if (playbackSettings.midiThru != shouldThru) {
        playbackSettings.midiThru = shouldThru;
        sendChangeMessage();
    }
}

void MIDISequencer::setSendClock(bool shouldSend) {
    if (playbackSettings.sendClock != shouldSend) {
        playbackSettings.sendClock = shouldSend;
        sendChangeMessage();
    }
}

void MIDISequencer::setSendMTC(bool shouldSend) {
    if (playbackSettings.sendMTC != shouldSend) {
        playbackSettings.sendMTC = shouldSend;
        sendChangeMessage();
    }
}

void MIDISequencer::setMTCFormat(int format) {
    if (playbackSettings.mtcFormat != format) {
        playbackSettings.mtcFormat = format;
        sendChangeMessage();
    }
}

void MIDISequencer::setSendMMC(bool shouldSend) {
    if (playbackSettings.sendMMC != shouldSend) {
        playbackSettings.sendMMC = shouldSend;
        sendChangeMessage();
    }
}

void MIDISequencer::setSendProgramChanges(bool shouldSend) {
    if (playbackSettings.sendProgramChanges != shouldSend) {
        playbackSettings.sendProgramChanges = shouldSend;
        sendChangeMessage();
    }
}

void MIDISequencer::setSendControlChanges(bool shouldSend) {
    if (playbackSettings.sendControlChanges != shouldSend) {
        playbackSettings.sendControlChanges = shouldSend;
        sendChangeMessage();
    }
}

void MIDISequencer::setSendSysEx(bool shouldSend) {
    if (playbackSettings.sendSysEx != shouldSend) {
        playbackSettings.sendSysEx = shouldSend;
        sendChangeMessage();
    }
}

void MIDISequencer::startRecording(Track* track) {
    if (track == nullptr || track->getType() != Track::Type::MIDI) {
        return;
    }

    stopRecording();  // Stop any existing recording
    
    recordingTrack = track;
    recordingStartTime = currentProject != nullptr ? currentProject->getTransportPosition() : 0.0;
    recordingSequence.clear();
    recording = true;
    
    LOG_INFO("Started MIDI recording on track: %s", track->getName());
}

void MIDISequencer::stopRecording() {
    if (!recording) {
        return;
    }

    finalizeRecording();
    clearRecordingState();
    
    LOG_INFO("Stopped MIDI recording");
}

void MIDISequencer::handleIncomingMidiMessage(const juce::MidiMessage& message) {
    const juce::ScopedLock lock(midiLock);
    
    if (!shouldProcessMessage(message)) {
        return;
    }
    
    // Handle MIDI thru
    if (playbackSettings.midiThru) {
        inputBuffer.addEvent(message, 0);
    }
    
    // Handle recording
    if (recording && recordingTrack != nullptr) {
        const double messageTime = message.getTimeStamp() - recordingStartTime;
        processRecordedMessage(message, messageTime);
    }
}

void MIDISequencer::processInputBuffer(juce::MidiBuffer& buffer) {
    const juce::ScopedLock lock(midiLock);
    
    if (!inputBuffer.isEmpty()) {
        buffer.addEvents(inputBuffer, 0, -1, 0);
        inputBuffer.clear();
    }
}

void MIDISequencer::processOutputBuffer(juce::MidiBuffer& buffer, double position) {
    if (currentProject == nullptr) {
        return;
    }

    // Send MIDI clock
    if (playbackSettings.sendClock) {
        sendMidiClock(position);
    }
    
    // Send MTC
    if (playbackSettings.sendMTC) {
        sendMidiTimeCode(position);
    }
    
    // Process track MIDI
    // TODO: Process MIDI from tracks
    
    buffer.addEvents(outputBuffer, 0, -1, 0);
    outputBuffer.clear();
}

void MIDISequencer::sendMidiClock(double position) {
    const double clockInterval = 60.0 / (currentProject->getSettings().tempo * 24.0);
    
    while (lastClockTime <= position) {
        outputBuffer.addEvent(juce::MidiMessage::midiClock(),
                            static_cast<int>((lastClockTime - position) * 44100.0));
        lastClockTime += clockInterval;
    }
}

void MIDISequencer::sendMidiTimeCode(double position) {
    const int frameRate = getMTCFrameRate(playbackSettings.mtcFormat);
    const double frameTime = 1.0 / frameRate;
    
    while (lastMTCTime <= position) {
        const int totalFrames = static_cast<int>(lastMTCTime * frameRate);
        if (totalFrames != mtcFrameCount) {
            mtcFrameCount = totalFrames;
            
            // Send MTC quarter-frame messages
            const juce::String mtcString = getMTCString(lastMTCTime,
                                                      playbackSettings.mtcFormat);
            // TODO: Generate and send MTC messages
        }
        
        lastMTCTime += frameTime;
    }
}

void MIDISequencer::sendMidiMachineControl(int command) {
    if (!playbackSettings.sendMMC) {
        return;
    }

    // Standard MMC commands
    switch (command) {
        case 1:  // Stop
            outputBuffer.addEvent(juce::MidiMessage::midiMachineControlCommand(0x01), 0);
            break;
        case 2:  // Play
            outputBuffer.addEvent(juce::MidiMessage::midiMachineControlCommand(0x02), 0);
            break;
        case 3:  // Record
            outputBuffer.addEvent(juce::MidiMessage::midiMachineControlCommand(0x06), 0);
            break;
        case 4:  // Pause
            outputBuffer.addEvent(juce::MidiMessage::midiMachineControlCommand(0x09), 0);
            break;
    }
}

void MIDISequencer::quantizeEvents(juce::MidiMessageSequence& sequence,
                                 double grid) {
    for (int i = 0; i < sequence.getNumEvents(); ++i) {
        auto* event = sequence.getEventPointer(i);
        if (event->message.isNoteOn()) {
            const double quantizedTime = std::round(event->message.getTimeStamp() / grid) * grid;
            const double timeDiff = quantizedTime - event->message.getTimeStamp();
            
            event->message.setTimeStamp(quantizedTime);
            
            // Move corresponding note-off
            if (event->noteOffObject != nullptr) {
                event->noteOffObject->message.setTimeStamp(
                    event->noteOffObject->message.getTimeStamp() + timeDiff);
            }
        }
    }
    
    sequence.updateMatchedPairs();
}

void MIDISequencer::transposeEvents(juce::MidiMessageSequence& sequence,
                                  int semitones) {
    for (int i = 0; i < sequence.getNumEvents(); ++i) {
        auto* event = sequence.getEventPointer(i);
        if (event->message.isNoteOnOrOff()) {
            const int newNote = juce::jlimit(0, 127,
                                           event->message.getNoteNumber() + semitones);
            event->message.setNoteNumber(newNote);
        }
    }
}

void MIDISequencer::scaleVelocities(juce::MidiMessageSequence& sequence,
                                  float scale) {
    for (int i = 0; i < sequence.getNumEvents(); ++i) {
        auto* event = sequence.getEventPointer(i);
        if (event->message.isNoteOn()) {
            const float newVelocity = juce::jlimit(0.0f, 127.0f,
                                                 event->message.getVelocity() * scale);
            event->message.setVelocity(newVelocity);
        }
    }
}

void MIDISequencer::filterEvents(juce::MidiMessageSequence& sequence,
                               const juce::BigInteger& channels,
                               const juce::BigInteger& notes) {
    for (int i = sequence.getNumEvents() - 1; i >= 0; --i) {
        auto* event = sequence.getEventPointer(i);
        const bool keepEvent = event->message.isNoteOnOrOff() ?
            (channels[event->message.getChannel() - 1] &&
             notes[event->message.getNoteNumber()]) :
            channels[event->message.getChannel() - 1];
             
        if (!keepEvent) {
            sequence.deleteEvent(i, true);
        }
    }
    
    sequence.updateMatchedPairs();
}

void MIDISequencer::saveState(juce::ValueTree& state) const {
    // Save recording settings
    auto recordNode = state.getOrCreateChildWithName("recordSettings", nullptr);
    recordNode.setProperty("quantizeInput", recordSettings.quantizeInput, nullptr);
    recordNode.setProperty("quantizeGrid", recordSettings.quantizeGrid, nullptr);
    recordNode.setProperty("autoQuantize", recordSettings.autoQuantize, nullptr);
    recordNode.setProperty("replaceMode", recordSettings.replaceMode, nullptr);
    recordNode.setProperty("overdubMode", recordSettings.overdubMode, nullptr);
    recordNode.setProperty("velocityMode", recordSettings.velocityMode, nullptr);
    recordNode.setProperty("velocityValue", recordSettings.velocityValue, nullptr);
    recordNode.setProperty("velocityScale", recordSettings.velocityScale, nullptr);
    recordNode.setProperty("filterChannels", recordSettings.filterChannels, nullptr);
    recordNode.setProperty("activeChannels", recordSettings.activeChannels.toString(), nullptr);
    recordNode.setProperty("filterNotes", recordSettings.filterNotes, nullptr);
    recordNode.setProperty("activeNotes", recordSettings.activeNotes.toString(), nullptr);
    
    // Save playback settings
    auto playbackNode = state.getOrCreateChildWithName("playbackSettings", nullptr);
    playbackNode.setProperty("midiThru", playbackSettings.midiThru, nullptr);
    playbackNode.setProperty("sendClock", playbackSettings.sendClock, nullptr);
    playbackNode.setProperty("sendMTC", playbackSettings.sendMTC, nullptr);
    playbackNode.setProperty("mtcFormat", playbackSettings.mtcFormat, nullptr);
    playbackNode.setProperty("sendMMC", playbackSettings.sendMMC, nullptr);
    playbackNode.setProperty("sendProgramChanges", playbackSettings.sendProgramChanges, nullptr);
    playbackNode.setProperty("sendControlChanges", playbackSettings.sendControlChanges, nullptr);
    playbackNode.setProperty("sendSysEx", playbackSettings.sendSysEx, nullptr);
}

void MIDISequencer::loadState(const juce::ValueTree& state) {
    // Load recording settings
    if (auto recordNode = state.getChildWithName("recordSettings")) {
        recordSettings.quantizeInput = recordNode.getProperty("quantizeInput", recordSettings.quantizeInput);
        recordSettings.quantizeGrid = recordNode.getProperty("quantizeGrid", recordSettings.quantizeGrid);
        recordSettings.autoQuantize = recordNode.getProperty("autoQuantize", recordSettings.autoQuantize);
        recordSettings.replaceMode = recordNode.getProperty("replaceMode", recordSettings.replaceMode);
        recordSettings.overdubMode = recordNode.getProperty("overdubMode", recordSettings.overdubMode);
        recordSettings.velocityMode = recordNode.getProperty("velocityMode", recordSettings.velocityMode);
        recordSettings.velocityValue = recordNode.getProperty("velocityValue", recordSettings.velocityValue);
        recordSettings.velocityScale = recordNode.getProperty("velocityScale", recordSettings.velocityScale);
        recordSettings.filterChannels = recordNode.getProperty("filterChannels", recordSettings.filterChannels);
        recordSettings.activeChannels.parseString(recordNode.getProperty("activeChannels", "").toString(), 2);
        recordSettings.filterNotes = recordNode.getProperty("filterNotes", recordSettings.filterNotes);
        recordSettings.activeNotes.parseString(recordNode.getProperty("activeNotes", "").toString(), 2);
    }
    
    // Load playback settings
    if (auto playbackNode = state.getChildWithName("playbackSettings")) {
        playbackSettings.midiThru = playbackNode.getProperty("midiThru", playbackSettings.midiThru);
        playbackSettings.sendClock = playbackNode.getProperty("sendClock", playbackSettings.sendClock);
        playbackSettings.sendMTC = playbackNode.getProperty("sendMTC", playbackSettings.sendMTC);
        playbackSettings.mtcFormat = playbackNode.getProperty("mtcFormat", playbackSettings.mtcFormat);
        playbackSettings.sendMMC = playbackNode.getProperty("sendMMC", playbackSettings.sendMMC);
        playbackSettings.sendProgramChanges = playbackNode.getProperty("sendProgramChanges", playbackSettings.sendProgramChanges);
        playbackSettings.sendControlChanges = playbackNode.getProperty("sendControlChanges", playbackSettings.sendControlChanges);
        playbackSettings.sendSysEx = playbackNode.getProperty("sendSysEx", playbackSettings.sendSysEx);
    }
    
    sendChangeMessage();
}

void MIDISequencer::processRecordedMessage(const juce::MidiMessage& message,
                                         double time) {
    if (!message.isNoteOnOrOff() && !message.isController() &&
        !message.isProgramChange() && !message.isPitchWheel()) {
        return;
    }
    
    juce::MidiMessage recordedMessage(message);
    
    // Apply velocity processing
    if (message.isNoteOn()) {
        recordedMessage.setVelocity(processVelocity(message.getVelocity()));
    }
    
    // Apply quantization
    const double messageTime = recordSettings.quantizeInput ?
        quantizeTime(time) : time;
    recordedMessage.setTimeStamp(messageTime);
    
    recordingSequence.addEvent(recordedMessage);
    recordingSequence.updateMatchedPairs();
}

void MIDISequencer::finalizeRecording() {
    if (recordingTrack == nullptr || recordingSequence.getNumEvents() == 0) {
        return;
    }

    // Apply auto-quantize if enabled
    if (recordSettings.autoQuantize) {
        quantizeEvents(recordingSequence, recordSettings.quantizeGrid);
    }
    
    // TODO: Add recorded sequence to track
}

void MIDISequencer::clearRecordingState() {
    recording = false;
    recordingTrack = nullptr;
    recordingSequence.clear();
    recordingStartTime = 0.0;
}

double MIDISequencer::quantizeTime(double time) const {
    return std::round(time / recordSettings.quantizeGrid) * recordSettings.quantizeGrid;
}

int MIDISequencer::processVelocity(int velocity) const {
    switch (recordSettings.velocityMode) {
        case 0:  // As played
            return velocity;
            
        case 1:  // Fixed
            return static_cast<int>(recordSettings.velocityValue);
            
        case 2:  // Scaled
            return static_cast<int>(velocity * recordSettings.velocityScale);
            
        default:
            return velocity;
    }
}

bool MIDISequencer::shouldProcessMessage(const juce::MidiMessage& message) const {
    if (!message.isNoteOnOrOff() && !message.isController() &&
        !message.isProgramChange() && !message.isPitchWheel()) {
        return false;
    }
    
    if (recordSettings.filterChannels &&
        !recordSettings.activeChannels[message.getChannel() - 1]) {
        return false;
    }
    
    if (recordSettings.filterNotes && message.isNoteOnOrOff() &&
        !recordSettings.activeNotes[message.getNoteNumber()]) {
        return false;
    }
    
    return true;
}

juce::String MIDISequencer::getMTCString(double time, int format) {
    const int frameRate = getMTCFrameRate(format);
    const int totalFrames = static_cast<int>(time * frameRate);
    
    const int hours = totalFrames / (frameRate * 3600);
    const int minutes = (totalFrames % (frameRate * 3600)) / (frameRate * 60);
    const int seconds = (totalFrames % (frameRate * 60)) / frameRate;
    const int frames = totalFrames % frameRate;
    
    return juce::String::formatted("%02d:%02d:%02d:%02d",
                                 hours, minutes, seconds, frames);
}

int MIDISequencer::getMTCFrameRate(int format) {
    switch (format) {
        case 0: return 24;  // 24 fps
        case 1: return 25;  // 25 fps
        case 2: return 30;  // 30 fps drop
        case 3: return 30;  // 30 fps
        default: return 25;
    }
}

//==============================================================================
// MIDISequencerUtils Implementation
//==============================================================================

namespace MIDISequencerUtils {
    bool importMidiFile(const juce::File& file,
                       juce::MidiMessageSequence& sequence) {
        juce::FileInputStream stream(file);
        if (stream.openedOk()) {
            juce::MidiFile midiFile;
            if (midiFile.readFrom(stream)) {
                sequence.clear();
                sequence.addSequence(*midiFile.getTrack(0), 0.0);
                sequence.updateMatchedPairs();
                return true;
            }
        }
        return false;
    }

    bool exportMidiFile(const juce::File& file,
                       const juce::MidiMessageSequence& sequence) {
        juce::MidiFile midiFile;
        midiFile.setTicksPerQuarterNote(960);
        
        midiFile.addTrack(sequence);
        
        juce::FileOutputStream stream(file);
        if (stream.openedOk()) {
            midiFile.writeTo(stream);
            return true;
        }
        return false;
    }

    void mergeMidiSequences(juce::MidiMessageSequence& dest,
                          const juce::MidiMessageSequence& source) {
        dest.addSequence(source, 0.0);
        dest.updateMatchedPairs();
    }

    void splitMidiSequence(const juce::MidiMessageSequence& source,
                         double splitTime,
                         juce::MidiMessageSequence& left,
                         juce::MidiMessageSequence& right) {
        left.clear();
        right.clear();
        
        for (int i = 0; i < source.getNumEvents(); ++i) {
            auto* event = source.getEventPointer(i);
            if (event->message.getTimeStamp() < splitTime) {
                left.addEvent(event->message);
            } else {
                right.addEvent(event->message);
            }
        }
        
        left.updateMatchedPairs();
        right.updateMatchedPairs();
    }

    double ticksToTime(int ticks, int ppq, double bpm) {
        return (60.0 * ticks) / (bpm * ppq);
    }

    int timeToTicks(double time, int ppq, double bpm) {
        return static_cast<int>((time * bpm * ppq) / 60.0);
    }

    void filterChannelMessages(juce::MidiMessageSequence& sequence,
                             int channel) {
        for (int i = sequence.getNumEvents() - 1; i >= 0; --i) {
            auto* event = sequence.getEventPointer(i);
            if (event->message.getChannel() != channel) {
                sequence.deleteEvent(i, true);
            }
        }
        sequence.updateMatchedPairs();
    }

    void filterNoteMessages(juce::MidiMessageSequence& sequence,
                          const juce::Array<int>& notes) {
        for (int i = sequence.getNumEvents() - 1; i >= 0; --i) {
            auto* event = sequence.getEventPointer(i);
            if (event->message.isNoteOnOrOff() &&
                !notes.contains(event->message.getNoteNumber())) {
                sequence.deleteEvent(i, true);
            }
        }
        sequence.updateMatchedPairs();
    }

    void filterControllerMessages(juce::MidiMessageSequence& sequence,
                                const juce::Array<int>& controllers) {
        for (int i = sequence.getNumEvents() - 1; i >= 0; --i) {
            auto* event = sequence.getEventPointer(i);
            if (event->message.isController() &&
                !controllers.contains(event->message.getControllerNumber())) {
                sequence.deleteEvent(i, true);
            }
        }
        sequence.updateMatchedPairs();
    }

    void generateMidiClock(juce::MidiMessageSequence& sequence,
                         double duration,
                         double bpm) {
        const double clockInterval = 60.0 / (bpm * 24.0);
        double time = 0.0;
        
        while (time < duration) {
            sequence.addEvent(juce::MidiMessage::midiClock(), time);
            time += clockInterval;
        }
    }

    void generateMidiTimeCode(juce::MidiMessageSequence& sequence,
                           double duration,
                           int format) {
        const int frameRate = format == 1 ? 25 : 30;
        const double frameTime = 1.0 / frameRate;
        double time = 0.0;
        
        while (time < duration) {
            // TODO: Generate MTC quarter-frame messages
            time += frameTime;
        }
    }
}