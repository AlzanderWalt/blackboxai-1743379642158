#include "Mixer.h"
#include "Project.h"
#include "Track.h"
#include "Plugin.h"
#include "Logger.h"

//==============================================================================
// Mixer Implementation
//==============================================================================

Mixer::Mixer() {
    masterChannel.volume = 1.0f;
}

Mixer::~Mixer() {
    releaseResources();
}

void Mixer::setProject(Project* project) {
    if (currentProject == project) {
        return;
    }

    releaseResources();
    currentProject = project;
    
    if (currentProject != nullptr) {
        // Initialize channels for all tracks
        const int numTracks = currentProject->getTracks().size();
        channels.resize(numTracks);
        channelSoloBuffer.resize(numTracks);
        updateProcessingBuffers();
    } else {
        channels.clear();
        channelSoloBuffer.clear();
        buses.clear();
    }
    
    sendChangeMessage();
}

Mixer::Channel& Mixer::getChannel(int index) {
    jassert(index >= 0 && index < channels.size());
    return channels[index];
}

const Mixer::Channel& Mixer::getChannel(int index) const {
    jassert(index >= 0 && index < channels.size());
    return channels[index];
}

void Mixer::setChannelVolume(int index, float volume) {
    if (index >= 0 && index < channels.size()) {
        channels[index].volume = juce::jlimit(0.0f, 2.0f, volume);
        sendChangeMessage();
    }
}

void Mixer::setChannelPan(int index, float pan) {
    if (index >= 0 && index < channels.size()) {
        channels[index].pan = juce::jlimit(-1.0f, 1.0f, pan);
        sendChangeMessage();
    }
}

void Mixer::setChannelMute(int index, bool mute) {
    if (index >= 0 && index < channels.size()) {
        channels[index].mute = mute;
        sendChangeMessage();
    }
}

void Mixer::setChannelSolo(int index, bool solo) {
    if (index >= 0 && index < channels.size()) {
        channels[index].solo = solo;
        updateSoloStates();
        sendChangeMessage();
    }
}

void Mixer::setChannelBypass(int index, bool bypass) {
    if (index >= 0 && index < channels.size()) {
        channels[index].bypass = bypass;
        sendChangeMessage();
    }
}

float Mixer::getChannelPeakLevel(int index) const {
    if (index >= 0 && index < channels.size()) {
        return channels[index].peakLevel;
    }
    return 0.0f;
}

float Mixer::getChannelRMSLevel(int index) const {
    if (index >= 0 && index < channels.size()) {
        return channels[index].rmsLevel;
    }
    return 0.0f;
}

void Mixer::addSend(int channelIndex, int busIndex, float level) {
    if (channelIndex >= 0 && channelIndex < channels.size() &&
        busIndex >= 0 && busIndex < buses.size()) {
        channels[channelIndex].sends.push_back({busIndex, level});
        sendChangeMessage();
    }
}

void Mixer::removeSend(int channelIndex, int busIndex) {
    if (channelIndex >= 0 && channelIndex < channels.size()) {
        auto& sends = channels[channelIndex].sends;
        sends.erase(std::remove_if(sends.begin(), sends.end(),
                                 [busIndex](const auto& send) {
                                     return send.first == busIndex;
                                 }),
                   sends.end());
        sendChangeMessage();
    }
}

void Mixer::setSendLevel(int channelIndex, int busIndex, float level) {
    if (channelIndex >= 0 && channelIndex < channels.size()) {
        for (auto& send : channels[channelIndex].sends) {
            if (send.first == busIndex) {
                send.second = juce::jlimit(0.0f, 1.0f, level);
                sendChangeMessage();
                break;
            }
        }
    }
}

int Mixer::addBus(BusType type, const juce::String& name) {
    Bus bus;
    bus.type = type;
    bus.name = name;
    bus.outputBus = -1;  // Output to master by default
    
    buses.push_back(std::move(bus));
    updateProcessingBuffers();
    sendChangeMessage();
    
    return static_cast<int>(buses.size() - 1);
}

void Mixer::removeBus(int index) {
    if (index >= 0 && index < buses.size()) {
        // Remove sends to this bus
        for (auto& channel : channels) {
            removeSend(static_cast<int>(&channel - &channels[0]), index);
        }
        
        // Remove bus
        buses.erase(buses.begin() + index);
        updateProcessingBuffers();
        sendChangeMessage();
    }
}

Mixer::Bus& Mixer::getBus(int index) {
    jassert(index >= 0 && index < buses.size());
    return buses[index];
}

const Mixer::Bus& Mixer::getBus(int index) const {
    jassert(index >= 0 && index < buses.size());
    return buses[index];
}

void Mixer::setBusName(int index, const juce::String& name) {
    if (index >= 0 && index < buses.size()) {
        buses[index].name = name;
        sendChangeMessage();
    }
}

void Mixer::setBusOutput(int index, int outputBus) {
    if (index >= 0 && index < buses.size()) {
        buses[index].outputBus = outputBus;
        sendChangeMessage();
    }
}

void Mixer::addBusSource(int busIndex, int sourceIndex) {
    if (busIndex >= 0 && busIndex < buses.size()) {
        buses[busIndex].sources.push_back(sourceIndex);
        sendChangeMessage();
    }
}

void Mixer::removeBusSource(int busIndex, int sourceIndex) {
    if (busIndex >= 0 && busIndex < buses.size()) {
        auto& sources = buses[busIndex].sources;
        sources.erase(std::remove(sources.begin(), sources.end(), sourceIndex),
                     sources.end());
        sendChangeMessage();
    }
}

juce::StringArray Mixer::getBusNames() const {
    juce::StringArray names;
    for (const auto& bus : buses) {
        names.add(bus.name);
    }
    return names;
}

std::vector<int> Mixer::getBusSources(int index) const {
    if (index >= 0 && index < buses.size()) {
        return buses[index].sources;
    }
    return {};
}

int Mixer::getBusOutput(int index) const {
    if (index >= 0 && index < buses.size()) {
        return buses[index].outputBus;
    }
    return -1;
}

void Mixer::addPlugin(int channelIndex, std::unique_ptr<Plugin> plugin) {
    if (channelIndex >= 0 && channelIndex < channels.size() && plugin != nullptr) {
        channels[channelIndex].plugins.push_back(std::move(plugin));
        sendChangeMessage();
    }
}

void Mixer::removePlugin(int channelIndex, int pluginIndex) {
    if (channelIndex >= 0 && channelIndex < channels.size()) {
        auto& plugins = channels[channelIndex].plugins;
        if (pluginIndex >= 0 && pluginIndex < plugins.size()) {
            plugins.erase(plugins.begin() + pluginIndex);
            sendChangeMessage();
        }
    }
}

void Mixer::movePlugin(int channelIndex, int fromIndex, int toIndex) {
    if (channelIndex >= 0 && channelIndex < channels.size()) {
        auto& plugins = channels[channelIndex].plugins;
        if (fromIndex >= 0 && fromIndex < plugins.size() &&
            toIndex >= 0 && toIndex < plugins.size()) {
            auto plugin = std::move(plugins[fromIndex]);
            plugins.erase(plugins.begin() + fromIndex);
            plugins.insert(plugins.begin() + toIndex, std::move(plugin));
            sendChangeMessage();
        }
    }
}

Plugin* Mixer::getPlugin(int channelIndex, int pluginIndex) {
    if (channelIndex >= 0 && channelIndex < channels.size()) {
        auto& plugins = channels[channelIndex].plugins;
        if (pluginIndex >= 0 && pluginIndex < plugins.size()) {
            return plugins[pluginIndex].get();
        }
    }
    return nullptr;
}

int Mixer::getNumPlugins(int channelIndex) const {
    if (channelIndex >= 0 && channelIndex < channels.size()) {
        return static_cast<int>(channels[channelIndex].plugins.size());
    }
    return 0;
}

void Mixer::prepareToPlay(double sampleRate, int maximumExpectedSamplesPerBlock) {
    currentSampleRate = sampleRate;
    currentBlockSize = maximumExpectedSamplesPerBlock;
    
    updateProcessingBuffers();
    
    // Prepare plugins
    for (auto& channel : channels) {
        for (auto& plugin : channel.plugins) {
            plugin->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
        }
    }
    
    for (auto& bus : buses) {
        for (auto& plugin : bus.channel.plugins) {
            plugin->prepareToPlay(sampleRate, maximumExpectedSamplesPerBlock);
        }
    }
    
    processingPrepared = true;
}

void Mixer::processBlock(juce::AudioBuffer<float>& buffer,
                        juce::MidiBuffer& midiMessages) {
    if (!processingPrepared || currentProject == nullptr) {
        return;
    }

    // Clear all buffers
    clearAllBuffers();
    
    // Process channels
    processChannels(buffer, midiMessages);
    
    // Process buses
    processBuses();
    
    // Process master
    processMaster(buffer);
}

void Mixer::releaseResources() {
    // Release plugins
    for (auto& channel : channels) {
        for (auto& plugin : channel.plugins) {
            plugin->releaseResources();
        }
    }
    
    for (auto& bus : buses) {
        for (auto& plugin : bus.channel.plugins) {
            plugin->releaseResources();
        }
    }
    
    processingPrepared = false;
}

void Mixer::saveState(juce::ValueTree& state) const {
    // Save channels
    auto channelsNode = state.getOrCreateChildWithName("channels", nullptr);
    channelsNode.removeAllChildren(nullptr);
    
    for (const auto& channel : channels) {
        auto channelNode = channelsNode.createChild("channel");
        channelNode.setProperty("volume", channel.volume, nullptr);
        channelNode.setProperty("pan", channel.pan, nullptr);
        channelNode.setProperty("mute", channel.mute, nullptr);
        channelNode.setProperty("solo", channel.solo, nullptr);
        channelNode.setProperty("bypass", channel.bypass, nullptr);
        
        // Save sends
        auto sendsNode = channelNode.getOrCreateChildWithName("sends", nullptr);
        for (const auto& send : channel.sends) {
            auto sendNode = sendsNode.createChild("send");
            sendNode.setProperty("bus", send.first, nullptr);
            sendNode.setProperty("level", send.second, nullptr);
        }
    }
    
    // Save buses
    auto busesNode = state.getOrCreateChildWithName("buses", nullptr);
    busesNode.removeAllChildren(nullptr);
    
    for (const auto& bus : buses) {
        auto busNode = busesNode.createChild("bus");
        busNode.setProperty("type", static_cast<int>(bus.type), nullptr);
        busNode.setProperty("name", bus.name, nullptr);
        busNode.setProperty("output", bus.outputBus, nullptr);
        
        // Save sources
        auto sourcesNode = busNode.getOrCreateChildWithName("sources", nullptr);
        for (int source : bus.sources) {
            auto sourceNode = sourcesNode.createChild("source");
            sourceNode.setProperty("index", source, nullptr);
        }
        
        // Save channel settings
        auto channelNode = busNode.getOrCreateChildWithName("channel", nullptr);
        channelNode.setProperty("volume", bus.channel.volume, nullptr);
        channelNode.setProperty("pan", bus.channel.pan, nullptr);
        channelNode.setProperty("mute", bus.channel.mute, nullptr);
        channelNode.setProperty("bypass", bus.channel.bypass, nullptr);
    }
    
    // Save master channel
    auto masterNode = state.getOrCreateChildWithName("master", nullptr);
    masterNode.setProperty("volume", masterChannel.volume, nullptr);
    masterNode.setProperty("pan", masterChannel.pan, nullptr);
    masterNode.setProperty("mute", masterChannel.mute, nullptr);
    masterNode.setProperty("bypass", masterChannel.bypass, nullptr);
}

void Mixer::loadState(const juce::ValueTree& state) {
    // Load channels
    if (auto channelsNode = state.getChildWithName("channels")) {
        channels.clear();
        
        for (auto channelNode : channelsNode) {
            Channel channel;
            channel.volume = channelNode.getProperty("volume", 1.0f);
            channel.pan = channelNode.getProperty("pan", 0.0f);
            channel.mute = channelNode.getProperty("mute", false);
            channel.solo = channelNode.getProperty("solo", false);
            channel.bypass = channelNode.getProperty("bypass", false);
            
            // Load sends
            if (auto sendsNode = channelNode.getChildWithName("sends")) {
                for (auto sendNode : sendsNode) {
                    int bus = sendNode.getProperty("bus");
                    float level = sendNode.getProperty("level");
                    channel.sends.push_back({bus, level});
                }
            }
            
            channels.push_back(std::move(channel));
        }
    }
    
    // Load buses
    if (auto busesNode = state.getChildWithName("buses")) {
        buses.clear();
        
        for (auto busNode : busesNode) {
            Bus bus;
            bus.type = static_cast<BusType>(static_cast<int>(busNode.getProperty("type")));
            bus.name = busNode.getProperty("name");
            bus.outputBus = busNode.getProperty("output");
            
            // Load sources
            if (auto sourcesNode = busNode.getChildWithName("sources")) {
                for (auto sourceNode : sourcesNode) {
                    bus.sources.push_back(sourceNode.getProperty("index"));
                }
            }
            
            // Load channel settings
            if (auto channelNode = busNode.getChildWithName("channel")) {
                bus.channel.volume = channelNode.getProperty("volume", 1.0f);
                bus.channel.pan = channelNode.getProperty("pan", 0.0f);
                bus.channel.mute = channelNode.getProperty("mute", false);
                bus.channel.bypass = channelNode.getProperty("bypass", false);
            }
            
            buses.push_back(std::move(bus));
        }
    }
    
    // Load master channel
    if (auto masterNode = state.getChildWithName("master")) {
        masterChannel.volume = masterNode.getProperty("volume", 1.0f);
        masterChannel.pan = masterNode.getProperty("pan", 0.0f);
        masterChannel.mute = masterNode.getProperty("mute", false);
        masterChannel.bypass = masterNode.getProperty("bypass", false);
    }
    
    updateProcessingBuffers();
    updateSoloStates();
    sendChangeMessage();
}

void Mixer::updateProcessingBuffers() {
    // Channel buffers
    channelBuffers.resize(channels.size());
    for (auto& buffer : channelBuffers) {
        buffer.setSize(2, currentBlockSize);
    }
    
    // Bus buffers
    busBuffers.resize(buses.size());
    for (auto& buffer : busBuffers) {
        buffer.setSize(2, currentBlockSize);
    }
    
    // Master buffer
    masterBuffer.setSize(2, currentBlockSize);
}

void Mixer::clearAllBuffers() {
    for (auto& buffer : channelBuffers) {
        buffer.clear();
    }
    
    for (auto& buffer : busBuffers) {
        buffer.clear();
    }
    
    masterBuffer.clear();
}

void Mixer::processChannels(juce::AudioBuffer<float>& buffer,
                          juce::MidiBuffer& midiMessages) {
    const auto& tracks = currentProject->getTracks();
    
    for (size_t i = 0; i < channels.size(); ++i) {
        if (!isChannelActive(static_cast<int>(i))) {
            continue;
        }
        
        auto& channelBuffer = channelBuffers[i];
        auto& channel = channels[i];
        
        // Get audio from track
        if (i < tracks.size()) {
            tracks[i]->processBlock(channelBuffer, midiMessages);
        }
        
        // Process plugins
        if (!channel.bypass) {
            for (auto& plugin : channel.plugins) {
                if (!plugin->isBypassed()) {
                    plugin->processBlock(channelBuffer, midiMessages);
                }
            }
        }
        
        // Apply channel settings
        applyChannelSettings(channelBuffer, channel);
        
        // Update meters
        updatePeakAndRMSLevels(channelBuffer, channel.peakLevel, channel.rmsLevel);
        
        // Process sends
        processSends(channelBuffer, channel);
        
        // Mix to master
        MixerUtils::mixBuffers(channelBuffer, masterBuffer);
    }
}

void Mixer::processBuses() {
    for (size_t i = 0; i < buses.size(); ++i) {
        auto& bus = buses[i];
        auto& busBuffer = busBuffers[i];
        
        // Mix sources
        for (int source : bus.sources) {
            if (source >= 0 && source < channelBuffers.size()) {
                MixerUtils::mixBuffers(channelBuffers[source], busBuffer);
            }
        }
        
        // Process plugins
        if (!bus.channel.bypass) {
            for (auto& plugin : bus.channel.plugins) {
                if (!plugin->isBypassed()) {
                    plugin->processBlock(busBuffer, juce::MidiBuffer());
                }
            }
        }
        
        // Apply channel settings
        applyChannelSettings(busBuffer, bus.channel);
        
        // Route to output
        if (bus.outputBus >= 0 && bus.outputBus < busBuffers.size()) {
            MixerUtils::mixBuffers(busBuffer, busBuffers[bus.outputBus]);
        } else {
            MixerUtils::mixBuffers(busBuffer, masterBuffer);
        }
    }
}

void Mixer::processMaster(juce::AudioBuffer<float>& buffer) {
    // Process master plugins
    if (!masterChannel.bypass) {
        for (auto& plugin : masterChannel.plugins) {
            if (!plugin->isBypassed()) {
                plugin->processBlock(masterBuffer, juce::MidiBuffer());
            }
        }
    }
    
    // Apply master settings
    applyChannelSettings(masterBuffer, masterChannel);
    
    // Update meters
    updatePeakAndRMSLevels(masterBuffer, masterChannel.peakLevel, masterChannel.rmsLevel);
    
    // Copy to output
    buffer.makeCopyOf(masterBuffer);
}

void Mixer::updatePeakAndRMSLevels(const juce::AudioBuffer<float>& buffer,
                                  float& peak, float& rms) {
    peak = 0.0f;
    rms = 0.0f;
    
    for (int channel = 0; channel < buffer.getNumChannels(); ++channel) {
        peak = std::max(peak, buffer.getMagnitude(channel, 0, buffer.getNumSamples()));
        rms = std::max(rms, MixerUtils::calculateRMSLevel(buffer.getReadPointer(channel),
                                                        buffer.getNumSamples()));
    }
}

void Mixer::applyChannelSettings(juce::AudioBuffer<float>& buffer,
                               const Channel& channel) {
    if (channel.mute || !buffer.hasBeenCleared()) {
        buffer.clear();
        return;
    }
    
    // Apply volume
    if (channel.volume != 1.0f) {
        buffer.applyGain(channel.volume);
    }
    
    // Apply pan
    if (channel.pan != 0.0f && buffer.getNumChannels() == 2) {
        const float leftGain = calculatePanGain(channel.pan, true);
        const float rightGain = calculatePanGain(channel.pan, false);
        
        buffer.applyGain(0, 0, buffer.getNumSamples(), leftGain);
        buffer.applyGain(1, 0, buffer.getNumSamples(), rightGain);
    }
}

void Mixer::processSends(const juce::AudioBuffer<float>& source,
                        const Channel& channel) {
    for (const auto& send : channel.sends) {
        if (send.first >= 0 && send.first < busBuffers.size()) {
            MixerUtils::mixBuffers(source, busBuffers[send.first], send.second);
        }
    }
}

void Mixer::updateSoloStates() {
    soloActive = false;
    
    // Check if any channel is soloed
    for (const auto& channel : channels) {
        if (channel.solo) {
            soloActive = true;
            break;
        }
    }
    
    // Update solo buffer
    for (size_t i = 0; i < channels.size(); ++i) {
        channelSoloBuffer[i] = !soloActive || channels[i].solo;
    }
}

bool Mixer::isChannelActive(int index) const {
    if (index >= 0 && index < channels.size()) {
        return !channels[index].mute && channelSoloBuffer[index];
    }
    return false;
}

float Mixer::calculatePanGain(float pan, bool leftChannel) {
    return leftChannel ? std::cos(pan * juce::MathConstants<float>::halfPi)
                      : std::sin(pan * juce::MathConstants<float>::halfPi);
}

//==============================================================================
// MixerUtils Implementation
//==============================================================================

namespace MixerUtils {
    float dbToGain(float db) {
        return std::pow(10.0f, db * 0.05f);
    }

    float gainToDb(float gain) {
        return 20.0f * std::log10(gain);
    }

    float velocityToGain(int velocity) {
        return velocity / 127.0f;
    }

    float panToGain(float pan, bool leftChannel) {
        return leftChannel ? std::cos(pan * juce::MathConstants<float>::halfPi)
                         : std::sin(pan * juce::MathConstants<float>::halfPi);
    }

    juce::Array<float> calculatePanLaw(int numSteps) {
        juce::Array<float> law;
        law.resize(numSteps);
        
        for (int i = 0; i < numSteps; ++i) {
            const float pan = (i / static_cast<float>(numSteps - 1)) * 2.0f - 1.0f;
            law.set(i, panToGain(pan, true));
        }
        
        return law;
    }

    float calculateRMSLevel(const float* data, int numSamples) {
        float sum = 0.0f;
        
        for (int i = 0; i < numSamples; ++i) {
            const float sample = data[i];
            sum += sample * sample;
        }
        
        return std::sqrt(sum / numSamples);
    }

    float calculatePeakLevel(const float* data, int numSamples) {
        float peak = 0.0f;
        
        for (int i = 0; i < numSamples; ++i) {
            peak = std::max(peak, std::abs(data[i]));
        }
        
        return peak;
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

    void mixBuffers(const juce::AudioBuffer<float>& source,
                   juce::AudioBuffer<float>& destination,
                   float gain) {
        const int numChannels = std::min(source.getNumChannels(),
                                       destination.getNumChannels());
        const int numSamples = std::min(source.getNumSamples(),
                                      destination.getNumSamples());
        
        for (int channel = 0; channel < numChannels; ++channel) {
            destination.addFrom(channel, 0,
                              source, channel, 0,
                              numSamples, gain);
        }
    }

    void copyWithGain(const juce::AudioBuffer<float>& source,
                     juce::AudioBuffer<float>& destination,
                     float gain) {
        const int numChannels = std::min(source.getNumChannels(),
                                       destination.getNumChannels());
        const int numSamples = std::min(source.getNumSamples(),
                                      destination.getNumSamples());
        
        for (int channel = 0; channel < numChannels; ++channel) {
            destination.copyFrom(channel, 0,
                               source, channel, 0,
                               numSamples);
        }
        
        if (gain != 1.0f) {
            destination.applyGain(gain);
        }
    }
}