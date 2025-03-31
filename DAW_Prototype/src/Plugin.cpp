#include "Plugin.h"
#include "Track.h"
#include "Logger.h"

//==============================================================================
// Plugin Implementation
//==============================================================================

Plugin::Plugin(Track& track)
    : track(track) {
}

Plugin::~Plugin() {
    releaseResources();
}

void Plugin::bypass(bool shouldBypass) {
    if (bypassed != shouldBypass) {
        bypassed = shouldBypass;
        handleBypassChange();
        sendChangeMessage();
        
        LOG_INFO("Plugin %s on track %d: bypass %s",
                 getName(),
                 track.getId(),
                 bypassed ? "enabled" : "disabled");
    }
}

void Plugin::enable(bool shouldBeEnabled) {
    if (enabled != shouldBeEnabled) {
        enabled = shouldBeEnabled;
        handleEnableChange();
        sendChangeMessage();
        
        LOG_INFO("Plugin %s on track %d: %s",
                 getName(),
                 track.getId(),
                 enabled ? "enabled" : "disabled");
    }
}

void Plugin::bypassProcessing(juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages) {
    // Default bypass behavior - do nothing, letting audio pass through unchanged
    juce::ignoreUnused(buffer, midiMessages);
}

void Plugin::handleBypassChange() {
    // Default implementation - subclasses can override
}

void Plugin::handleEnableChange() {
    // Default implementation - subclasses can override
}

//==============================================================================
// PluginUtils Implementation
//==============================================================================

namespace PluginUtils {
    juce::String typeToString(Plugin::Type type) {
        switch (type) {
            case Plugin::Type::VST3: return "VST3";
            case Plugin::Type::AudioUnit: return "AudioUnit";
            case Plugin::Type::Internal: return "Internal";
            default: return "Unknown";
        }
    }

    Plugin::Type stringToType(const juce::String& str) {
        if (str == "VST3") return Plugin::Type::VST3;
        if (str == "AudioUnit") return Plugin::Type::AudioUnit;
        if (str == "Internal") return Plugin::Type::Internal;
        return Plugin::Type::Internal;  // Default to internal
    }

    float normalizeParameter(float value, const juce::NormalisableRange<float>& range) {
        return range.convertTo0to1(value);
    }

    float denormalizeParameter(float normalized, const juce::NormalisableRange<float>& range) {
        return range.convertFrom0to1(normalized);
    }

    void savePluginState(const Plugin& plugin, juce::ValueTree& state) {
        // Save basic properties
        state.setProperty("name", plugin.getName(), nullptr);
        state.setProperty("bypassed", plugin.isBypassed(), nullptr);
        state.setProperty("enabled", plugin.isEnabled(), nullptr);
        
        // Save plugin-specific state
        juce::MemoryBlock data;
        plugin.saveState(data);
        state.setProperty("pluginState", data.toBase64Encoding(), nullptr);
        
        // Save current program
        state.setProperty("currentProgram", plugin.getCurrentProgram(), nullptr);
        
        // Save parameter values
        auto params = state.getOrCreateChildWithName("parameters", nullptr);
        for (int i = 0; i < plugin.getNumParameters(); ++i) {
            auto param = params.createChild(i);
            param.setProperty("index", i, nullptr);
            param.setProperty("value", plugin.getParameter(i), nullptr);
        }
    }

    void loadPluginState(Plugin& plugin, const juce::ValueTree& state) {
        // Load basic properties
        plugin.bypass(state.getProperty("bypassed", plugin.isBypassed()));
        plugin.enable(state.getProperty("enabled", plugin.isEnabled()));
        
        // Load plugin-specific state
        if (state.hasProperty("pluginState")) {
            juce::MemoryBlock data;
            data.fromBase64Encoding(state.getProperty("pluginState").toString());
            plugin.loadState(data.getData(), data.getSize());
        }
        
        // Load current program
        if (state.hasProperty("currentProgram")) {
            plugin.setCurrentProgram(state.getProperty("currentProgram"));
        }
        
        // Load parameter values
        if (auto params = state.getChildWithName("parameters")) {
            for (auto param : params) {
                int index = param.getProperty("index");
                float value = param.getProperty("value");
                plugin.setParameter(index, value);
            }
        }
    }

    juce::File getPresetDirectory() {
        juce::File dir = juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
                            .getChildFile("DAW_Prototype")
                            .getChildFile("Presets");
        
        if (!dir.exists()) {
            dir.createDirectory();
        }
        
        return dir;
    }

    juce::Array<juce::File> findPresetFiles(const juce::String& formatName) {
        juce::Array<juce::File> presets;
        
        juce::File presetDir = getPresetDirectory().getChildFile(formatName);
        if (presetDir.exists()) {
            presetDir.findChildFiles(presets, juce::File::findFiles, true, "*.preset");
        }
        
        return presets;
    }

    bool isPresetFile(const juce::File& file) {
        return file.hasFileExtension(".preset");
    }

    juce::StringArray getAvailablePlugins(Plugin::Type type) {
        juce::StringArray plugins;
        
        switch (type) {
            case Plugin::Type::VST3: {
                juce::VST3PluginFormat format;
                juce::FileSearchPath path(format.getDefaultLocations());
                plugins.addArray(format.searchPathsForPlugins(path, true));
                break;
            }
            
            case Plugin::Type::AudioUnit: {
                #if JUCE_PLUGINHOST_AU && JUCE_MAC
                juce::AudioUnitPluginFormat format;
                plugins.addArray(format.searchPathsForPlugins(juce::FileSearchPath(), true));
                #endif
                break;
            }
            
            case Plugin::Type::Internal:
                // Add internal plugin names here
                plugins.add("Gain");
                plugins.add("Delay");
                plugins.add("Reverb");
                plugins.add("EQ");
                plugins.add("Compressor");
                break;
                
            default:
                break;
        }
        
        return plugins;
    }

    bool validatePlugin(const juce::String& path) {
        if (path.isEmpty()) {
            return false;
        }
        
        juce::File file(path);
        if (!file.exists()) {
            LOG_ERROR("Plugin file does not exist: %s", path);
            return false;
        }
        
        // Check file extension
        if (file.hasFileExtension(".vst3")) {
            juce::VST3PluginFormat format;
            return format.fileMightContainThisPluginType(file);
        }
        #if JUCE_PLUGINHOST_AU && JUCE_MAC
        else if (file.hasFileExtension(".component")) {
            juce::AudioUnitPluginFormat format;
            return format.fileMightContainThisPluginType(file);
        }
        #endif
        
        return false;
    }

    juce::String getPluginArchitecture(const juce::String& path) {
        juce::File file(path);
        if (!file.exists()) {
            return {};
        }
        
        // TODO: Implement proper architecture detection
        #if JUCE_64BIT
        return "x64";
        #else
        return "x86";
        #endif
    }
}