#include "Clip.h"
#include "Track.h"
#include "Logger.h"
#include "AudioUtils.h"
#include "MIDIUtils.h"

//==============================================================================
// Base Clip Implementation
//==============================================================================

Clip::Clip(Track& track, double startTime)
    : track(track)
    , startTime(startTime)
    , length(0.0)
    , colour(track.getColor()) {
}

void Clip::setStartTime(double newStartTime) {
    if (startTime != newStartTime) {
        startTime = newStartTime;
        sendChangeMessage();
    }
}

void Clip::setLength(double newLength) {
    if (length != newLength) {
        length = std::max(0.0, newLength);
        sendChangeMessage();
    }
}

void Clip::setName(const juce::String& newName) {
    if (name != newName) {
        name = newName;
        sendChangeMessage();
    }
}

void Clip::setColour(juce::Colour newColour) {
    if (colour != newColour) {
        colour = newColour;
        sendChangeMessage();
    }
}

void Clip::setSelected(bool shouldBeSelected) {
    if (selected != shouldBeSelected) {
        selected = shouldBeSelected;
        sendChangeMessage();
    }
}

void Clip::setMuted(bool shouldBeMuted) {
    if (muted != shouldBeMuted) {
        muted = shouldBeMuted;
        sendChangeMessage();
    }
}

void Clip::saveState(juce::ValueTree& state) const {
    state.setProperty("startTime", startTime, nullptr);
    state.setProperty("length", length, nullptr);
    state.setProperty("name", name, nullptr);
    state.setProperty("colour", colour.toString(), nullptr);
    state.setProperty("selected", selected, nullptr);
    state.setProperty("muted", muted, nullptr);
}

void Clip::loadState(const juce::ValueTree& state) {
    startTime = state.getProperty("startTime", startTime);
    length = state.getProperty("length", length);
    name = state.getProperty("name", name);
    colour = juce::Colour::fromString(state.getProperty("colour", colour.toString()));
    selected = state.getProperty("selected", selected);
    muted = state.getProperty("muted", muted);
    
    sendChangeMessage();
}

//==============================================================================
// AudioClip Implementation
//==============================================================================

AudioClip::AudioClip(Track& track, double startTime, const juce::File& file)
    : Clip(track, startTime) {
    loadAudioFile(file);
}

AudioClip::~AudioClip() {
    releaseResources();
}

bool AudioClip::loadAudioFile(const juce::File& file) {
    if (!file.existsAsFile()) {
        LOG_ERROR("Audio file does not exist: %s", file.getFullPathName());
        return false;
    }

    audioFile = file;
    
    juce::AudioFormatManager formatManager;
    formatManager.registerBasicFormats();
    
    reader.reset(formatManager.createReaderFor(file));
    
    if (reader != nullptr) {
        sourceLength = reader->lengthInSamples / reader->sampleRate;
        length = sourceLength;
        updateAudioData();
        
        LOG_INFO("Loaded audio file: %s (%.2f seconds, %.0f Hz, %d channels)",
                 file.getFullPathName(),
                 sourceLength,
                 reader->sampleRate,
                 reader->numChannels);
        return true;
    }
    
    LOG_ERROR("Failed to load audio file: %s", file.getFullPathName());
    return false;
}

void AudioClip::setSourceStartTime(double newStartTime) {
    if (sourceStartTime != newStartTime) {
        sourceStartTime = std::max(0.0, newStartTime);
        updateAudioData();
        sendChangeMessage();
    }
}

void AudioClip::setSourceLength(double newLength) {
    if (sourceLength != newLength) {
        sourceLength = std::max(0.0, newLength);
        updateAudioData();
        sendChangeMessage();
    }
}

void AudioClip::setLooping(bool shouldLoop) {
    if (looping != shouldLoop) {
        looping = shouldLoop;
        sendChangeMessage();
    }
}

void AudioClip::setGain(float newGain) {
    if (gain != newGain) {
        gain = juce::jlimit(0.0f, 10.0f, newGain);
        sendChangeMessage();
    }
}

void AudioClip::setPitch(float newPitch) {
    if (pitch != newPitch) {
        pitch = juce::jlimit(0.25f, 4.0f, newPitch);
        applyPitchShift();
        sendChangeMessage();
    }
}

void AudioClip::setReversed(bool shouldBeReversed) {
    if (reversed != shouldBeReversed) {
        reversed = shouldBeReversed;
        reverseAudio();
        sendChangeMessage();
    }
}

void AudioClip::setStretching(bool shouldStretch) {
    if (timeStretchEnabled != shouldStretch) {
        timeStretchEnabled = shouldStretch;
        applyTimeStretch();
        sendChangeMessage();
    }
}

void AudioClip::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = maximumExpectedSamplesPerBlock;
}

void AudioClip::processBlock(juce::AudioBuffer<float>& buffer, int numSamples, double position) {
    if (muted || reader == nullptr) {
        return;
    }

    const double clipPosition = position - startTime;
    if (clipPosition < 0 || (!looping && clipPosition >= length)) {
        return;
    }

    // Calculate read position
    double readPosition = clipPosition;
    if (looping) {
        readPosition = std::fmod(readPosition, length);
    }
    readPosition += sourceStartTime;

    // Convert to samples
    const int readOffset = static_cast<int>(readPosition * currentSampleRate);
    const int numChannels = std::min(buffer.getNumChannels(), audioData.getNumChannels());

    // Copy audio data
    for (int channel = 0; channel < numChannels; ++channel) {
        buffer.addFrom(channel, 0, audioData, channel, readOffset, numSamples, gain);
    }
}

void AudioClip::releaseResources() {
    reader = nullptr;
    audioData.setSize(0, 0);
}

void AudioClip::saveState(juce::ValueTree& state) const {
    Clip::saveState(state);
    
    state.setProperty("audioFile", audioFile.getFullPathName(), nullptr);
    state.setProperty("sourceStartTime", sourceStartTime, nullptr);
    state.setProperty("sourceLength", sourceLength, nullptr);
    state.setProperty("looping", looping, nullptr);
    state.setProperty("gain", gain, nullptr);
    state.setProperty("pitch", pitch, nullptr);
    state.setProperty("reversed", reversed, nullptr);
    state.setProperty("timeStretchEnabled", timeStretchEnabled, nullptr);
}

void AudioClip::loadState(const juce::ValueTree& state) {
    Clip::loadState(state);
    
    juce::File file(state.getProperty("audioFile").toString());
    if (file.existsAsFile()) {
        loadAudioFile(file);
    }
    
    sourceStartTime = state.getProperty("sourceStartTime", sourceStartTime);
    sourceLength = state.getProperty("sourceLength", sourceLength);
    looping = state.getProperty("looping", looping);
    gain = state.getProperty("gain", gain);
    pitch = state.getProperty("pitch", pitch);
    reversed = state.getProperty("reversed", reversed);
    timeStretchEnabled = state.getProperty("timeStretchEnabled", timeStretchEnabled);
    
    updateAudioData();
}

void AudioClip::updateAudioData() {
    if (reader != nullptr) {
        const int numSamples = static_cast<int>(sourceLength * reader->sampleRate);
        audioData.setSize(reader->numChannels, numSamples);
        reader->read(&audioData, 0, numSamples, static_cast<int>(sourceStartTime * reader->sampleRate), true, true);
    }
}

void AudioClip::applyTimeStretch() {
    // TODO: Implement time stretching
}

void AudioClip::applyPitchShift() {
    // TODO: Implement pitch shifting
}

void AudioClip::reverseAudio() {
    if (audioData.getNumSamples() > 0) {
        for (int channel = 0; channel < audioData.getNumChannels(); ++channel) {
            auto* data = audioData.getWritePointer(channel);
            std::reverse(data, data + audioData.getNumSamples());
        }
    }
}

//==============================================================================
// MIDIClip Implementation
//==============================================================================

MIDIClip::MIDIClip(Track& track, double startTime)
    : Clip(track, startTime) {
}

MIDIClip::~MIDIClip() {
    releaseResources();
}

void MIDIClip::setSequence(const juce::MidiMessageSequence& newSequence) {
    sequence = newSequence;
    if (quantized) {
        quantizeSequence();
    }
    sendChangeMessage();
}

void MIDIClip::addNote(int noteNumber, float velocity, double startTime, double duration) {
    sequence.addEvent(juce::MidiMessage::noteOn(1, noteNumber, velocity), startTime);
    sequence.addEvent(juce::MidiMessage::noteOff(1, noteNumber), startTime + duration);
    sequence.updateMatchedPairs();
    
    if (quantized) {
        quantizeSequence();
    }
    
    sendChangeMessage();
}

void MIDIClip::removeNote(int noteNumber, double startTime) {
    for (int i = sequence.getNumEvents() - 1; i >= 0; --i) {
        auto* event = sequence.getEventPointer(i);
        if (event->message.isNoteOn() &&
            event->message.getNoteNumber() == noteNumber &&
            std::abs(event->message.getTimeStamp() - startTime) < 0.0001) {
            sequence.deleteEvent(i, true);
        }
    }
    
    sequence.updateMatchedPairs();
    sendChangeMessage();
}

void MIDIClip::clearAllNotes() {
    sequence.clear();
    sendChangeMessage();
}

void MIDIClip::setQuantized(bool shouldQuantize) {
    if (quantized != shouldQuantize) {
        quantized = shouldQuantize;
        if (quantized) {
            quantizeSequence();
        }
        sendChangeMessage();
    }
}

void MIDIClip::setQuantizeGrid(double newGrid) {
    if (quantizeGrid != newGrid) {
        quantizeGrid = newGrid;
        if (quantized) {
            quantizeSequence();
        }
        sendChangeMessage();
    }
}

void MIDIClip::setVelocityMultiplier(float multiplier) {
    if (velocityMultiplier != multiplier) {
        velocityMultiplier = juce::jlimit(0.0f, 2.0f, multiplier);
        updateNoteVelocities();
        sendChangeMessage();
    }
}

void MIDIClip::setTranspose(int semitones) {
    if (transpose != semitones) {
        transpose = semitones;
        transposeNotes();
        sendChangeMessage();
    }
}

void MIDIClip::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = maximumExpectedSamplesPerBlock;
}

void MIDIClip::processBlock(juce::MidiBuffer& midiMessages, double position) {
    if (muted) {
        return;
    }

    const double clipPosition = position - startTime;
    if (clipPosition < 0 || clipPosition >= length) {
        return;
    }

    // Add MIDI messages that fall within this block
    for (int i = 0; i < sequence.getNumEvents(); ++i) {
        auto* event = sequence.getEventPointer(i);
        double eventTime = event->message.getTimeStamp();
        
        if (eventTime >= clipPosition && eventTime < clipPosition + (currentBlockSize / currentSampleRate)) {
            const int samplePosition = static_cast<int>((eventTime - clipPosition) * currentSampleRate);
            midiMessages.addEvent(event->message, samplePosition);
        }
    }
}

void MIDIClip::releaseResources() {
    // Nothing to release for MIDI
}

void MIDIClip::saveState(juce::ValueTree& state) const {
    Clip::saveState(state);
    
    juce::MemoryOutputStream stream;
    sequence.writeTo(stream);
    state.setProperty("midiData", stream.getMemoryBlock(), nullptr);
    
    state.setProperty("quantized", quantized, nullptr);
    state.setProperty("quantizeGrid", quantizeGrid, nullptr);
    state.setProperty("velocityMultiplier", velocityMultiplier, nullptr);
    state.setProperty("transpose", transpose, nullptr);
}

void MIDIClip::loadState(const juce::ValueTree& state) {
    Clip::loadState(state);
    
    if (auto* midiData = state.getProperty("midiData").getBinaryData()) {
        juce::MemoryInputStream stream(*midiData, false);
        sequence.readFrom(stream);
    }
    
    quantized = state.getProperty("quantized", quantized);
    quantizeGrid = state.getProperty("quantizeGrid", quantizeGrid);
    velocityMultiplier = state.getProperty("velocityMultiplier", velocityMultiplier);
    transpose = state.getProperty("transpose", transpose);
    
    if (quantized) {
        quantizeSequence();
    }
}

void MIDIClip::quantizeSequence() {
    ClipUtils::quantizeNotes(sequence, quantizeGrid);
}

void MIDIClip::updateNoteVelocities() {
    ClipUtils::scaleVelocities(sequence, velocityMultiplier);
}

void MIDIClip::transposeNotes() {
    ClipUtils::transposeNotes(sequence, transpose);
}

//==============================================================================
// ClipUtils Implementation
//==============================================================================

namespace ClipUtils {
    double pixelsToTime(double pixels, double pixelsPerSecond) {
        return pixels / pixelsPerSecond;
    }

    double timeToPixels(double time, double pixelsPerSecond) {
        return time * pixelsPerSecond;
    }

    void quantizeNotes(juce::MidiMessageSequence& sequence, double grid) {
        for (int i = 0; i < sequence.getNumEvents(); ++i) {
            auto* event = sequence.getEventPointer(i);
            if (event->message.isNoteOn()) {
                double time = event->message.getTimeStamp();
                double quantizedTime = std::round(time / grid) * grid;
                
                double timeDiff = quantizedTime - time;
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

    void transposeNotes(juce::MidiMessageSequence& sequence, int semitones) {
        for (int i = 0; i < sequence.getNumEvents(); ++i) {
            auto* event = sequence.getEventPointer(i);
            if (event->message.isNoteOnOrOff()) {
                int note = event->message.getNoteNumber() + semitones;
                note = juce::jlimit(0, 127, note);
                event->message.setNoteNumber(note);
            }
        }
    }

    void scaleVelocities(juce::MidiMessageSequence& sequence, float factor) {
        for (int i = 0; i < sequence.getNumEvents(); ++i) {
            auto* event = sequence.getEventPointer(i);
            if (event->message.isNoteOn()) {
                float velocity = event->message.getVelocity() * factor;
                velocity = juce::jlimit(0.0f, 1.0f, velocity);
                event->message.setVelocity(velocity);
            }
        }
    }

    void normalizeAudio(juce::AudioBuffer<float>& buffer, float targetLevel) {
        float maxLevel = 0.0f;
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            maxLevel = std::max(maxLevel, buffer.getMagnitude(channel, 0, buffer.getNumSamples()));
        }
        
        if (maxLevel > 0.0f) {
            float gainFactor = targetLevel / maxLevel;
            buffer.applyGain(gainFactor);
        }
    }

    void fadeIn(juce::AudioBuffer<float>& buffer, int numSamples) {
        numSamples = std::min(numSamples, buffer.getNumSamples());
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            auto* data = buffer.getWritePointer(channel);
            for (int i = 0; i < numSamples; ++i) {
                float gain = static_cast<float>(i) / numSamples;
                data[i] *= gain;
            }
        }
    }

    void fadeOut(juce::AudioBuffer<float>& buffer, int numSamples) {
        numSamples = std::min(numSamples, buffer.getNumSamples());
        const int startSample = buffer.getNumSamples() - numSamples;
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            auto* data = buffer.getWritePointer(channel);
            for (int i = 0; i < numSamples; ++i) {
                float gain = static_cast<float>(numSamples - i) / numSamples;
                data[startSample + i] *= gain;
            }
        }
    }

    void crossfade(juce::AudioBuffer<float>& buffer1,
                  juce::AudioBuffer<float>& buffer2,
                  int crossfadeLength) {
        const int numChannels = std::min(buffer1.getNumChannels(), buffer2.getNumChannels());
        crossfadeLength = std::min(crossfadeLength,
                                 std::min(buffer1.getNumSamples(), buffer2.getNumSamples()));
        
        for (int channel = 0; channel < numChannels; ++channel) {
            auto* data1 = buffer1.getWritePointer(channel);
            auto* data2 = buffer2.getWritePointer(channel);
            
            for (int i = 0; i < crossfadeLength; ++i) {
                float gain1 = static_cast<float>(crossfadeLength - i) / crossfadeLength;
                float gain2 = static_cast<float>(i) / crossfadeLength;
                
                data1[i] = data1[i] * gain1 + data2[i] * gain2;
            }
        }
    }
}