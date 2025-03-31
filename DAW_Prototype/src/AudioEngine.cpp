#include "AudioEngine.h"
#include "Project.h"
#include "Logger.h"

//==============================================================================
// AudioEngine Implementation
//==============================================================================

AudioEngine::AudioEngine()
    : deviceManager(std::make_unique<juce::AudioDeviceManager>()) {
}

AudioEngine::~AudioEngine() {
    shutdown();
}

AudioEngine& AudioEngine::getInstance() {
    static AudioEngine instance;
    return instance;
}

bool AudioEngine::initialize() {
    if (initialized) {
        return true;
    }

    LOG_INFO("Initializing audio engine");
    
    try {
        if (!setupAudioDevice()) {
            LOG_ERROR("Failed to setup audio device");
            return false;
        }
        
        initializeBuffers();
        initialized = true;
        
        LOG_INFO("Audio engine initialized successfully");
        return true;
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception during audio engine initialization: %s", e.what());
        return false;
    }
}

void AudioEngine::shutdown() {
    if (!initialized) {
        return;
    }

    LOG_INFO("Shutting down audio engine");
    
    stop();
    cleanupAudioDevice();
    clearBuffers();
    initialized = false;
}

bool AudioEngine::applySettings(const Settings& newSettings) {
    if (newSettings.sampleRate == settings.sampleRate &&
        newSettings.bufferSize == settings.bufferSize &&
        newSettings.inputChannels == settings.inputChannels &&
        newSettings.outputChannels == settings.outputChannels &&
        newSettings.inputDevice == settings.inputDevice &&
        newSettings.outputDevice == settings.outputDevice &&
        newSettings.useAsioDriver == settings.useAsioDriver) {
        return true;
    }

    LOG_INFO("Applying new audio settings");
    
    const bool wasPlaying = transport.isPlaying;
    if (wasPlaying) {
        stop();
    }
    
    settings = newSettings;
    
    if (!setupAudioDevice()) {
        LOG_ERROR("Failed to apply new audio settings");
        return false;
    }
    
    initializeBuffers();
    
    if (wasPlaying) {
        play();
    }
    
    sendChangeMessage();
    return true;
}

juce::StringArray AudioEngine::getAvailableDevices() const {
    juce::StringArray devices;
    
    if (auto* currentDevice = deviceManager->getCurrentAudioDevice()) {
        for (const auto& type : deviceManager->getAvailableDeviceTypes()) {
            type->scanForDevices();
            devices.addArray(type->getDeviceNames());
        }
    }
    
    return devices;
}

juce::String AudioEngine::getCurrentDeviceName() const {
    if (auto* device = deviceManager->getCurrentAudioDevice()) {
        return device->getName();
    }
    return {};
}

void AudioEngine::setProject(Project* project) {
    const bool wasPlaying = transport.isPlaying;
    if (wasPlaying) {
        stop();
    }
    
    currentProject = project;
    
    if (currentProject != nullptr) {
        transport.bpm = currentProject->getSettings().tempo;
        const auto& ts = currentProject->getSettings().timeSignature;
        transport.timeSignature.numerator = ts.numerator;
        transport.timeSignature.denominator = ts.denominator;
    }
    
    if (wasPlaying) {
        play();
    }
}

void AudioEngine::play() {
    if (!initialized || transport.isPlaying) {
        return;
    }

    transport.isPlaying = true;
    sendChangeMessage();
    
    LOG_INFO("Transport: Play (position: %.2f s)", transport.position);
}

void AudioEngine::stop() {
    if (!initialized || !transport.isPlaying) {
        return;
    }

    transport.isPlaying = false;
    transport.isRecording = false;
    sendChangeMessage();
    
    LOG_INFO("Transport: Stop (position: %.2f s)", transport.position);
}

void AudioEngine::record() {
    if (!initialized) {
        return;
    }

    transport.isRecording = !transport.isRecording;
    if (transport.isRecording && !transport.isPlaying) {
        play();
    }
    sendChangeMessage();
    
    LOG_INFO("Transport: Record %s", transport.isRecording ? "on" : "off");
}

void AudioEngine::setPosition(double timeInSeconds) {
    nextPosition = timeInSeconds;
    sendChangeMessage();
    
    LOG_INFO("Transport: Set position to %.2f s", timeInSeconds);
}

void AudioEngine::setLoopPoints(double startTime, double endTime) {
    transport.loopStart = startTime;
    transport.loopEnd = endTime;
    sendChangeMessage();
    
    LOG_INFO("Transport: Set loop points (%.2f s - %.2f s)",
             startTime, endTime);
}

void AudioEngine::setLooping(bool shouldLoop) {
    transport.isLooping = shouldLoop;
    sendChangeMessage();
    
    LOG_INFO("Transport: Loop %s", shouldLoop ? "on" : "off");
}

void AudioEngine::setBpm(double newBpm) {
    transport.bpm = newBpm;
    sendChangeMessage();
    
    LOG_INFO("Transport: Set tempo to %.1f BPM", newBpm);
}

void AudioEngine::setTimeSignature(int numerator, int denominator) {
    transport.timeSignature.numerator = numerator;
    transport.timeSignature.denominator = denominator;
    sendChangeMessage();
    
    LOG_INFO("Transport: Set time signature to %d/%d",
             numerator, denominator);
}

void AudioEngine::addMidiInputDevice(const juce::String& deviceName) {
    const juce::ScopedLock lock(midiLock);
    
    for (auto* device : midiInputs) {
        if (device->getName() == deviceName) {
            return;
        }
    }
    
    if (auto midiInput = juce::MidiInput::openDevice(deviceName, this)) {
        midiInputs.add(midiInput.release());
        LOG_INFO("Added MIDI input device: %s", deviceName);
    }
}

void AudioEngine::removeMidiInputDevice(const juce::String& deviceName) {
    const juce::ScopedLock lock(midiLock);
    
    for (int i = midiInputs.size() - 1; i >= 0; --i) {
        if (midiInputs[i]->getName() == deviceName) {
            midiInputs.remove(i);
            LOG_INFO("Removed MIDI input device: %s", deviceName);
            break;
        }
    }
}

juce::StringArray AudioEngine::getMidiInputDevices() const {
    juce::StringArray devices;
    for (auto* device : midiInputs) {
        devices.add(device->getName());
    }
    return devices;
}

void AudioEngine::handleIncomingMidiMessage(juce::MidiInput* source,
                                          const juce::MidiMessage& message) {
    const juce::ScopedLock lock(midiLock);
    incomingMidi.addEvent(message, 0);
}

void AudioEngine::audioDeviceIOCallback(const float** inputChannelData,
                                      int numInputChannels,
                                      float** outputChannelData,
                                      int numOutputChannels,
                                      int numSamples) {
    const juce::Time processStartTime = juce::Time::getHighResolutionTicks();
    
    // Process audio
    processAudioBlock(inputChannelData, numInputChannels,
                     outputChannelData, numOutputChannels,
                     numSamples);
    
    // Process MIDI
    processMidiBlock(numSamples);
    
    // Update transport
    updateTransportPosition(numSamples);
    
    // Update CPU info
    const double processTimeMs = juce::Time::highResolutionTicksToSeconds(
        juce::Time::getHighResolutionTicks() - processStartTime) * 1000.0;
    updateCPUInfo(processTimeMs);
}

void AudioEngine::audioDeviceAboutToStart(juce::AudioIODevice* device) {
    const double sampleRate = device->getCurrentSampleRate();
    const int bufferSize = device->getCurrentBufferSizeSamples();
    
    LOG_INFO("Audio device starting (%.1f Hz, %d samples)",
             sampleRate, bufferSize);
    
    settings.sampleRate = sampleRate;
    settings.bufferSize = bufferSize;
    
    initializeBuffers();
}

void AudioEngine::audioDeviceStopped() {
    LOG_INFO("Audio device stopped");
    clearBuffers();
}

void AudioEngine::audioDeviceError(const juce::String& errorMessage) {
    LOG_ERROR("Audio device error: %s", errorMessage);
    audioDeviceErrorCallback(errorMessage);
}

void AudioEngine::processAudioBlock(const float** inputChannelData,
                                  int numInputChannels,
                                  float** outputChannelData,
                                  int numOutputChannels,
                                  int numSamples) {
    // Clear output
    for (int channel = 0; channel < numOutputChannels; ++channel) {
        juce::FloatVectorOperations::clear(outputChannelData[channel], numSamples);
    }
    
    if (!transport.isPlaying || currentProject == nullptr) {
        return;
    }
    
    // Copy input
    for (int channel = 0; channel < numInputChannels; ++channel) {
        inputBuffer.copyFrom(channel, 0, inputChannelData[channel], numSamples);
    }
    
    // Process tracks
    if (currentProject != nullptr) {
        // TODO: Process tracks
    }
    
    // Copy to output
    for (int channel = 0; channel < numOutputChannels; ++channel) {
        juce::FloatVectorOperations::copy(outputChannelData[channel],
                                        outputBuffer.getReadPointer(channel),
                                        numSamples);
    }
}

void AudioEngine::processMidiBlock(int numSamples) {
    if (!transport.isPlaying) {
        return;
    }
    
    // Add incoming MIDI
    {
        const juce::ScopedLock lock(midiLock);
        if (!incomingMidi.isEmpty()) {
            midiBuffer.addEvents(incomingMidi, 0, numSamples, 0);
            incomingMidi.clear();
        }
    }
    
    if (currentProject != nullptr) {
        // TODO: Process MIDI tracks
    }
    
    midiBuffer.clear();
}

void AudioEngine::updateTransportPosition(int numSamples) {
    if (!transport.isPlaying) {
        return;
    }
    
    // Handle position changes
    if (nextPosition.load() >= 0.0) {
        transport.position = nextPosition.exchange(-1.0);
    }
    
    // Update position
    const double timeIncrement = numSamples / settings.sampleRate;
    transport.position += timeIncrement;
    
    // Handle looping
    if (transport.isLooping &&
        transport.position >= transport.loopEnd) {
        transport.position = transport.loopStart;
    }
}

void AudioEngine::handleXRun() {
    cpuInfo.xruns++;
    LOG_WARNING("Audio dropout detected (total xruns: %d)", cpuInfo.xruns);
}

void AudioEngine::updateCPUInfo(double processingTimeMs) {
    const double bufferTimeMs = (settings.bufferSize / settings.sampleRate) * 1000.0;
    const float load = static_cast<float>(processingTimeMs / bufferTimeMs);
    
    cpuInfo.currentLoad = load;
    cpuInfo.averageLoad = cpuInfo.averageLoad * 0.9f + load * 0.1f;
    cpuInfo.peakLoad = std::max(cpuInfo.peakLoad * 0.99f, load);
    
    if (processingTimeMs > bufferTimeMs) {
        handleXRun();
    }
}

void AudioEngine::initializeBuffers() {
    const int numChannels = std::max(settings.inputChannels,
                                   settings.outputChannels);
    
    inputBuffer.setSize(numChannels, settings.bufferSize);
    outputBuffer.setSize(numChannels, settings.bufferSize);
    
    clearBuffers();
}

void AudioEngine::clearBuffers() {
    inputBuffer.clear();
    outputBuffer.clear();
    midiBuffer.clear();
    incomingMidi.clear();
}

bool AudioEngine::setupAudioDevice() {
    juce::String error;
    
    auto* currentDevice = deviceManager->getCurrentAudioDevice();
    if (currentDevice != nullptr &&
        currentDevice->getCurrentSampleRate() == settings.sampleRate &&
        currentDevice->getCurrentBufferSizeSamples() == settings.bufferSize) {
        return true;
    }
    
    juce::AudioDeviceManager::AudioDeviceSetup config;
    deviceManager->getAudioDeviceSetup(config);
    
    config.sampleRate = settings.sampleRate;
    config.bufferSize = settings.bufferSize;
    config.inputChannels = settings.inputChannels;
    config.outputChannels = settings.outputChannels;
    config.useDefaultInputChannels = true;
    config.useDefaultOutputChannels = true;
    
    if (settings.inputDevice.isNotEmpty()) {
        config.inputDeviceName = settings.inputDevice;
    }
    if (settings.outputDevice.isNotEmpty()) {
        config.outputDeviceName = settings.outputDevice;
    }
    
    error = deviceManager->setAudioDeviceSetup(config, true);
    
    if (error.isNotEmpty()) {
        LOG_ERROR("Failed to setup audio device: %s", error);
        return false;
    }
    
    deviceManager->addAudioCallback(this);
    return true;
}

void AudioEngine::cleanupAudioDevice() {
    deviceManager->removeAudioCallback(this);
    deviceManager->closeAudioDevice();
}

void AudioEngine::audioDeviceErrorCallback(const juce::String& errorMessage) {
    juce::AlertWindow::showMessageBoxAsync(juce::AlertWindow::WarningIcon,
                                         "Audio Device Error",
                                         errorMessage);
}

//==============================================================================
// AudioEngineUtils Implementation
//==============================================================================

namespace AudioEngineUtils {
    double samplesToTime(int64_t samples, double sampleRate) {
        return static_cast<double>(samples) / sampleRate;
    }

    int64_t timeToSamples(double timeInSeconds, double sampleRate) {
        return static_cast<int64_t>(timeInSeconds * sampleRate);
    }

    double ppqToTime(double ppq, double bpm) {
        return (60.0 * ppq) / (bpm * 4.0);
    }

    double timeToPpq(double timeInSeconds, double bpm) {
        return (timeInSeconds * bpm * 4.0) / 60.0;
    }

    void applyGainRamp(juce::AudioBuffer<float>& buffer,
                      int startSample,
                      int numSamples,
                      float startGain,
                      float endGain) {
        const float increment = (endGain - startGain) / numSamples;
        
        for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
            float* data = buffer.getWritePointer(channel, startSample);
            float gain = startGain;
            
            for (int i = 0; i < numSamples; ++i) {
                data[i] *= gain;
                gain += increment;
            }
        }
    }

    void crossfadeBuffers(juce::AudioBuffer<float>& buffer1,
                         juce::AudioBuffer<float>& buffer2,
                         int crossfadeLength) {
        const int numChannels = std::min(buffer1.getNumChannels(),
                                       buffer2.getNumChannels());
                                       
        for (int channel = 0; channel < numChannels; ++channel) {
            float* data1 = buffer1.getWritePointer(channel);
            float* data2 = buffer2.getWritePointer(channel);
            
            for (int i = 0; i < crossfadeLength; ++i) {
                const float alpha = static_cast<float>(i) / crossfadeLength;
                data1[i] = data1[i] * (1.0f - alpha) + data2[i] * alpha;
            }
        }
    }

    float calculateCPULoad(double processingTimeMs, double bufferTimeMs) {
        return static_cast<float>(processingTimeMs / bufferTimeMs);
    }

    void detectXRun(double processingTimeMs, double bufferTimeMs) {
        if (processingTimeMs > bufferTimeMs) {
            LOG_WARNING("Audio dropout detected (processing: %.2f ms, buffer: %.2f ms)",
                       processingTimeMs, bufferTimeMs);
        }
    }

    juce::String getDefaultDeviceName() {
        return juce::AudioDeviceManager::getDefaultAudioDeviceName(false, true);
    }

    int getDefaultBufferSize() {
        #if JUCE_WINDOWS
        return 512;
        #else
        return 256;
        #endif
    }

    bool isAsioAvailable() {
        #if JUCE_WINDOWS
        return juce::SystemStats::getOperatingSystemType() >= 
               juce::SystemStats::WinVista;
        #else
        return false;
        #endif
    }
}