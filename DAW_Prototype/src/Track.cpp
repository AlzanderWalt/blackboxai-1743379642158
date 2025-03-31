#include "Track.h"
#include "Logger.h"

Track::Track(Type trackType)
    : type(trackType) {
    generateID();
    name = "New " + getTypeString(type) + " Track";
    LOG_INFO("Created track: %s (%s)", name.toRawUTF8(), id.toRawUTF8());
}

Track::~Track() {
    LOG_INFO("Destroyed track: %s (%s)", name.toRawUTF8(), id.toRawUTF8());
}

void Track::setName(const juce::String& newName) {
    if (name != newName) {
        name = newName;
        notifyTrackChanged();
        LOG_INFO("Renamed track to: %s (%s)", name.toRawUTF8(), id.toRawUTF8());
    }
}

void Track::setParameters(const Parameters& newParams) {
    parameters = newParams;
    notifyTrackChanged();
}

void Track::setVolume(float newVolume) {
    if (parameters.volume != newVolume) {
        parameters.volume = newVolume;
        notifyTrackChanged();
    }
}

void Track::setPan(float newPan) {
    if (parameters.pan != newPan) {
        parameters.pan = newPan;
        notifyTrackChanged();
    }
}

void Track::setMute(bool shouldMute) {
    if (parameters.mute != shouldMute) {
        parameters.mute = shouldMute;
        notifyTrackChanged();
    }
}

void Track::setSolo(bool shouldSolo) {
    if (parameters.solo != shouldSolo) {
        parameters.solo = shouldSolo;
        notifyTrackChanged();
    }
}

void Track::setRecord(bool shouldRecord) {
    if (parameters.record != shouldRecord) {
        parameters.record = shouldRecord;
        notifyTrackChanged();
    }
}

void Track::setMonitoring(bool shouldMonitor) {
    if (parameters.monitoring != shouldMonitor) {
        parameters.monitoring = shouldMonitor;
        notifyTrackChanged();
    }
}

void Track::setHeight(int newHeight) {
    if (parameters.height != newHeight) {
        parameters.height = newHeight;
        notifyTrackChanged();
    }
}

void Track::setInputDevice(const juce::String& device) {
    if (parameters.input.device != device) {
        parameters.input.device = device;
        notifyTrackChanged();
    }
}

void Track::setInputChannel(int channel) {
    if (parameters.input.channel != channel) {
        parameters.input.channel = channel;
        notifyTrackChanged();
    }
}

void Track::setOutputBus(const juce::String& bus) {
    if (parameters.output.bus != bus) {
        parameters.output.bus = bus;
        notifyTrackChanged();
    }
}

void Track::setOutputChannel(int channel) {
    if (parameters.output.channel != channel) {
        parameters.output.channel = channel;
        notifyTrackChanged();
    }
}

void Track::addPlugin(const juce::String& pluginID) {
    auto plugin = std::make_unique<Plugin>(pluginID);
    plugins.add(plugin.release());
    notifyTrackChanged();
    LOG_INFO("Added plugin %s to track %s", pluginID.toRawUTF8(), name.toRawUTF8());
}

void Track::removePlugin(int index) {
    if (isPositiveAndBelow(index, plugins.size())) {
        plugins.remove(index);
        notifyTrackChanged();
        LOG_INFO("Removed plugin at index %d from track %s", index, name.toRawUTF8());
    }
}

void Track::movePlugin(int fromIndex, int toIndex) {
    if (isPositiveAndBelow(fromIndex, plugins.size()) &&
        isPositiveAndBelow(toIndex, plugins.size())) {
        plugins.move(fromIndex, toIndex);
        notifyTrackChanged();
    }
}

void Track::bypassPlugin(int index, bool bypass) {
    if (auto* plugin = getPlugin(index)) {
        plugin->bypass(bypass);
        notifyTrackChanged();
    }
}

Plugin* Track::getPlugin(int index) const {
    return plugins[index];
}

int Track::getNumPlugins() const {
    return plugins.size();
}

void Track::addClip(std::unique_ptr<Clip> clip) {
    clips.add(clip.release());
    notifyTrackChanged();
    LOG_INFO("Added clip to track %s", name.toRawUTF8());
}

void Track::removeClip(Clip* clip) {
    if (clips.removeObject(clip)) {
        notifyTrackChanged();
        LOG_INFO("Removed clip from track %s", name.toRawUTF8());
    }
}

void Track::moveClip(Clip* clip, double newStartTime) {
    if (clip != nullptr) {
        clip->setStartTime(newStartTime);
        notifyTrackChanged();
    }
}

Clip* Track::getClipAt(double time) const {
    for (auto* clip : clips) {
        if (clip->containsTime(time)) {
            return clip;
        }
    }
    return nullptr;
}

void Track::addAutomation(const juce::String& paramID) {
    if (!automation.contains(paramID)) {
        automation.set(paramID, AutomationData());
        notifyTrackChanged();
        LOG_INFO("Added automation for parameter %s on track %s",
                 paramID.toRawUTF8(), name.toRawUTF8());
    }
}

void Track::removeAutomation(const juce::String& paramID) {
    if (automation.remove(paramID)) {
        notifyTrackChanged();
        LOG_INFO("Removed automation for parameter %s on track %s",
                 paramID.toRawUTF8(), name.toRawUTF8());
    }
}

bool Track::hasAutomation(const juce::String& paramID) const {
    return automation.contains(paramID);
}

void Track::setAutomationValue(const juce::String& paramID, double time, float value) {
    if (auto* data = automation.getVarPointer(paramID)) {
        // Find insertion point
        int index = 0;
        while (index < data->times.size() && data->times[index] < time) {
            ++index;
        }
        
        // Update or insert value
        if (index < data->times.size() && data->times[index] == time) {
            data->values.set(index, value);
        } else {
            data->times.insert(index, time);
            data->values.insert(index, value);
        }
        
        notifyTrackChanged();
    }
}

float Track::getAutomationValue(const juce::String& paramID, double time) const {
    if (const auto* data = automation.getVarPointer(paramID)) {
        // Find surrounding points
        int index = 0;
        while (index < data->times.size() && data->times[index] < time) {
            ++index;
        }
        
        if (index == 0) {
            return data->values[0];
        }
        if (index >= data->times.size()) {
            return data->values.getLast();
        }
        
        // Linear interpolation
        const double t1 = data->times[index - 1];
        const double t2 = data->times[index];
        const float v1 = data->values[index - 1];
        const float v2 = data->values[index];
        
        const double alpha = (time - t1) / (t2 - t1);
        return v1 + (v2 - v1) * static_cast<float>(alpha);
    }
    return 0.0f;
}

void Track::prepareToPlay(double newSampleRate, int maximumExpectedSamplesPerBlock) {
    sampleRate = newSampleRate;
    blockSize = maximumExpectedSamplesPerBlock;
    
    // Prepare plugins
    for (auto* plugin : plugins) {
        plugin->prepareToPlay(sampleRate, blockSize);
    }
    
    // Prepare frozen buffer if needed
    if (frozen) {
        frozenBuffer.setSize(2, blockSize);
        frozenBuffer.clear();
    }
}

void Track::processBlock(juce::AudioBuffer<float>& buffer,
                        juce::MidiBuffer& midiMessages) {
    if (parameters.mute || (frozen && type != Type::Master)) {
        buffer.clear();
        midiMessages.clear();
        return;
    }
    
    // Apply volume and pan
    if (!parameters.mute && parameters.volume != 1.0f) {
        buffer.applyGain(parameters.volume);
    }
    
    if (parameters.pan != 0.0f && buffer.getNumChannels() == 2) {
        const float leftGain = std::cos((parameters.pan + 1.0f) * juce::MathConstants<float>::halfPi * 0.5f);
        const float rightGain = std::sin((parameters.pan + 1.0f) * juce::MathConstants<float>::halfPi * 0.5f);
        
        buffer.applyGain(0, 0, buffer.getNumSamples(), leftGain);
        buffer.applyGain(1, 0, buffer.getNumSamples(), rightGain);
    }
    
    // Process through plugins
    for (auto* plugin : plugins) {
        if (!plugin->isBypassed()) {
            plugin->processBlock(buffer, midiMessages);
        }
    }
}

void Track::releaseResources() {
    for (auto* plugin : plugins) {
        plugin->releaseResources();
    }
    
    frozenBuffer.setSize(0, 0);
    frozenMidi.clear();
}

void Track::freeze() {
    if (!frozen) {
        frozen = true;
        // TODO: Implement freezing logic
        notifyTrackChanged();
        LOG_INFO("Froze track: %s", name.toRawUTF8());
    }
}

void Track::unfreeze() {
    if (frozen) {
        frozen = false;
        frozenBuffer.setSize(0, 0);
        frozenMidi.clear();
        notifyTrackChanged();
        LOG_INFO("Unfroze track: %s", name.toRawUTF8());
    }
}

void Track::saveState(juce::ValueTree& state) const {
    state.setProperty("id", id, nullptr);
    state.setProperty("name", name, nullptr);
    state.setProperty("type", static_cast<int>(type), nullptr);
    
    // Save parameters
    auto paramsState = state.getOrCreateChildWithName("parameters", nullptr);
    paramsState.setProperty("volume", parameters.volume, nullptr);
    paramsState.setProperty("pan", parameters.pan, nullptr);
    paramsState.setProperty("mute", parameters.mute, nullptr);
    paramsState.setProperty("solo", parameters.solo, nullptr);
    paramsState.setProperty("record", parameters.record, nullptr);
    paramsState.setProperty("monitoring", parameters.monitoring, nullptr);
    paramsState.setProperty("height", parameters.height, nullptr);
    paramsState.setProperty("inputDevice", parameters.input.device, nullptr);
    paramsState.setProperty("inputChannel", parameters.input.channel, nullptr);
    paramsState.setProperty("outputBus", parameters.output.bus, nullptr);
    paramsState.setProperty("outputChannel", parameters.output.channel, nullptr);
    
    // Save plugins
    auto pluginsState = state.getOrCreateChildWithName("plugins", nullptr);
    for (auto* plugin : plugins) {
        auto pluginState = juce::ValueTree("plugin");
        plugin->saveState(pluginState);
        pluginsState.addChild(pluginState, -1, nullptr);
    }
    
    // Save clips
    auto clipsState = state.getOrCreateChildWithName("clips", nullptr);
    for (auto* clip : clips) {
        auto clipState = juce::ValueTree("clip");
        clip->saveState(clipState);
        clipsState.addChild(clipState, -1, nullptr);
    }
    
    // Save automation
    auto automationState = state.getOrCreateChildWithName("automation", nullptr);
    for (auto it = automation.begin(); it != automation.end(); ++it) {
        auto paramState = juce::ValueTree("parameter");
        paramState.setProperty("id", it.getKey(), nullptr);
        
        const auto& data = it.getValue();
        paramState.setProperty("times", juce::Array<juce::var>(data.times.begin(), data.times.size()), nullptr);
        paramState.setProperty("values", juce::Array<juce::var>(data.values.begin(), data.values.size()), nullptr);
        
        automationState.addChild(paramState, -1, nullptr);
    }
}

void Track::restoreState(const juce::ValueTree& state) {
    id = state.getProperty("id", juce::Uuid().toString());
    name = state.getProperty("name", "Unnamed Track");
    type = static_cast<Type>(static_cast<int>(state.getProperty("type", 0)));
    
    // Restore parameters
    if (auto paramsState = state.getChildWithName("parameters")) {
        parameters.volume = paramsState.getProperty("volume", 1.0f);
        parameters.pan = paramsState.getProperty("pan", 0.0f);
        parameters.mute = paramsState.getProperty("mute", false);
        parameters.solo = paramsState.getProperty("solo", false);
        parameters.record = paramsState.getProperty("record", false);
        parameters.monitoring = paramsState.getProperty("monitoring", false);
        parameters.height = paramsState.getProperty("height", 100);
        parameters.input.device = paramsState.getProperty("inputDevice", "");
        parameters.input.channel = paramsState.getProperty("inputChannel", 1);
        parameters.output.bus = paramsState.getProperty("outputBus", "master");
        parameters.output.channel = paramsState.getProperty("outputChannel", 1);
    }
    
    // Restore plugins
    plugins.clear();
    if (auto pluginsState = state.getChildWithName("plugins")) {
        for (auto pluginState : pluginsState) {
            if (auto plugin = std::make_unique<Plugin>("")) {
                plugin->restoreState(pluginState);
                plugins.add(plugin.release());
            }
        }
    }
    
    // Restore clips
    clips.clear();
    if (auto clipsState = state.getChildWithName("clips")) {
        for (auto clipState : clipsState) {
            if (auto clip = std::make_unique<Clip>()) {
                clip->restoreState(clipState);
                clips.add(clip.release());
            }
        }
    }
    
    // Restore automation
    automation.clear();
    if (auto automationState = state.getChildWithName("automation")) {
        for (auto paramState : automationState) {
            const auto paramID = paramState.getProperty("id").toString();
            AutomationData data;
            
            if (auto timesVar = paramState.getProperty("times")) {
                if (auto* array = timesVar.getArray()) {
                    for (const auto& value : *array) {
                        data.times.add(value);
                    }
                }
            }
            
            if (auto valuesVar = paramState.getProperty("values")) {
                if (auto* array = valuesVar.getArray()) {
                    for (const auto& value : *array) {
                        data.values.add(value);
                    }
                }
            }
            
            automation.set(paramID, data);
        }
    }
    
    notifyTrackChanged();
}

juce::ValueTree Track::getState() const {
    juce::ValueTree state("Track");
    saveState(state);
    return state;
}

void Track::generateID() {
    id = juce::Uuid().toString();
}

void Track::updateAutomation(double time) {
    for (auto it = automation.begin(); it != automation.end(); ++it) {
        const auto value = getAutomationValue(it.getKey(), time);
        // TODO: Apply automation value to parameter
    }
}

void Track::notifyTrackChanged() {
    sendChangeMessage();
}

juce::String Track::getTypeString(Type trackType) {
    switch (trackType) {
        case Type::Audio: return "Audio";
        case Type::MIDI: return "MIDI";
        case Type::Bus: return "Bus";
        case Type::Master: return "Master";
        default: return "Unknown";
    }
}