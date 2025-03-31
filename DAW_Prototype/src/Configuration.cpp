#include "Configuration.h"
#include "Logger.h"

Configuration::Configuration() {
    loadDefaults();
    createDefaultDirectories();
    loadFromFile();
}

Configuration::~Configuration() {
    saveToFile();
}

Configuration& Configuration::getInstance() {
    static Configuration instance;
    return instance;
}

void Configuration::loadFromFile() {
    const auto configFile = getConfigFile();
    
    if (!configFile.existsAsFile()) {
        LOG_INFO("Configuration file not found, using defaults");
        return;
    }

    try {
        auto json = juce::JSON::parse(configFile);
        
        if (auto* audioObj = json.getProperty("audio", nullptr).getDynamicObject()) {
            audioSettings.outputDevice = audioObj->getProperty("defaultOutputDevice").toString();
            audioSettings.inputDevice = audioObj->getProperty("defaultInputDevice").toString();
            audioSettings.sampleRate = audioObj->getProperty("sampleRate", 44100.0);
            audioSettings.bufferSize = audioObj->getProperty("bufferSize", 512);
            audioSettings.inputChannels = audioObj->getProperty("inputChannels", 2);
            audioSettings.outputChannels = audioObj->getProperty("outputChannels", 2);
            audioSettings.bitDepth = audioObj->getProperty("bitDepth", 32);
            audioSettings.dithering = audioObj->getProperty("dithering", true);
            audioSettings.autoConnectInputs = audioObj->getProperty("autoConnectInputs", true);
            audioSettings.autoConnectOutputs = audioObj->getProperty("autoConnectOutputs", true);
        }
        
        if (auto* midiObj = json.getProperty("midi", nullptr).getDynamicObject()) {
            midiSettings.thruEnabled = midiObj->getProperty("thruEnabled", true);
            midiSettings.clockEnabled = midiObj->getProperty("clockEnabled", false);
            midiSettings.mtcEnabled = midiObj->getProperty("mtcEnabled", false);
            midiSettings.mtcFormat = midiObj->getProperty("mtcFormat", 0);
            midiSettings.sendMMC = midiObj->getProperty("sendMMC", false);
            midiSettings.receiveMMC = midiObj->getProperty("receiveMMC", false);
            midiSettings.velocityScale = midiObj->getProperty("velocityScale", 1.0f);
            midiSettings.velocityOffset = midiObj->getProperty("velocityOffset", 0.0f);
            
            if (auto* devicesArray = midiObj->getProperty("defaultInputDevices").getArray()) {
                for (const auto& device : *devicesArray) {
                    midiSettings.inputDevices.add(device.toString());
                }
            }
        }
        
        if (auto* uiObj = json.getProperty("ui", nullptr).getDynamicObject()) {
            if (auto* themeObj = uiObj->getProperty("theme", nullptr).getDynamicObject()) {
                uiSettings.theme.darkMode = themeObj->getProperty("darkMode", false);
                uiSettings.theme.accentColor = juce::Colour::fromString(
                    themeObj->getProperty("accentColor", "#007AFF").toString());
                uiSettings.theme.fontSize = themeObj->getProperty("fontSize", 14);
                uiSettings.theme.fontName = themeObj->getProperty("fontName", "Default");
                uiSettings.theme.showTooltips = themeObj->getProperty("showTooltips", true);
                uiSettings.theme.showStatusBar = themeObj->getProperty("showStatusBar", true);
                uiSettings.theme.showMeterBridges = themeObj->getProperty("showMeterBridges", true);
            }
            
            if (auto* layoutObj = uiObj->getProperty("layout", nullptr).getDynamicObject()) {
                uiSettings.layout.mixerVisible = layoutObj->getProperty("mixerVisible", true);
                uiSettings.layout.pianoRollVisible = layoutObj->getProperty("pianoRollVisible", false);
                uiSettings.layout.mixerHeight = layoutObj->getProperty("mixerHeight", 200);
                uiSettings.layout.pianoRollHeight = layoutObj->getProperty("pianoRollHeight", 300);
                uiSettings.layout.trackHeight = layoutObj->getProperty("trackHeight", 100);
                uiSettings.layout.minimumTrackHeight = layoutObj->getProperty("minimumTrackHeight", 60);
                uiSettings.layout.maximumTrackHeight = layoutObj->getProperty("maximumTrackHeight", 300);
            }
            
            if (auto* gridObj = uiObj->getProperty("grid", nullptr).getDynamicObject()) {
                uiSettings.grid.snapToGrid = gridObj->getProperty("snapToGrid", true);
                uiSettings.grid.showGrid = gridObj->getProperty("showGrid", true);
                uiSettings.grid.gridColor = juce::Colour::fromString(
                    gridObj->getProperty("gridColor", "#404040").toString());
                uiSettings.grid.gridOpacity = gridObj->getProperty("gridOpacity", 0.5f);
                uiSettings.grid.majorGridInterval = gridObj->getProperty("majorGridInterval", 4);
                uiSettings.grid.minorGridInterval = gridObj->getProperty("minorGridInterval", 1);
            }
            
            if (auto* metersObj = uiObj->getProperty("meters", nullptr).getDynamicObject()) {
                uiSettings.meters.meterStyle = metersObj->getProperty("meterStyle", "gradient");
                uiSettings.meters.meterFallback = metersObj->getProperty("meterFallback", 1.5f);
                uiSettings.meters.peakHoldTime = metersObj->getProperty("peakHoldTime", 2000);
                uiSettings.meters.rmsWindowSize = metersObj->getProperty("rmsWindowSize", 50);
                uiSettings.meters.showPeakMarkers = metersObj->getProperty("showPeakMarkers", true);
                uiSettings.meters.showClipIndicators = metersObj->getProperty("showClipIndicators", true);
            }
        }
        
        if (auto* pluginsObj = json.getProperty("plugins", nullptr).getDynamicObject()) {
            if (auto* pathsArray = pluginsObj->getProperty("scanPaths").getArray()) {
                for (const auto& path : *pathsArray) {
                    pluginSettings.scanPaths.add(path.toString());
                }
            }
            
            if (auto* blacklistArray = pluginsObj->getProperty("blacklist").getArray()) {
                for (const auto& plugin : *blacklistArray) {
                    pluginSettings.blacklist.add(plugin.toString());
                }
            }
            
            if (auto* favoritesArray = pluginsObj->getProperty("favoritePlugins").getArray()) {
                for (const auto& plugin : *favoritesArray) {
                    pluginSettings.favoritePlugins.add(plugin.toString());
                }
            }
            
            if (auto* formatObj = pluginsObj->getProperty("pluginFormat", nullptr).getDynamicObject()) {
                pluginSettings.format.vst3 = formatObj->getProperty("vst3", true);
                pluginSettings.format.au = formatObj->getProperty("au", true);
                pluginSettings.format.lv2 = formatObj->getProperty("lv2", false);
            }
            
            if (auto* windowObj = pluginsObj->getProperty("pluginWindowBehavior", nullptr).getDynamicObject()) {
                pluginSettings.windowBehavior.alwaysOnTop = windowObj->getProperty("alwaysOnTop", false);
                pluginSettings.windowBehavior.hideWithHost = windowObj->getProperty("hideWithHost", true);
                pluginSettings.windowBehavior.rememberPosition = windowObj->getProperty("rememberPosition", true);
            }
        }
        
        if (auto* performanceObj = json.getProperty("performance", nullptr).getDynamicObject()) {
            performanceSettings.maxVoices = performanceObj->getProperty("maxVoices", 256);
            performanceSettings.diskCacheSize = performanceObj->getProperty("diskCacheSize", 1024);
            performanceSettings.ramCacheSize = performanceObj->getProperty("ramCacheSize", 512);
            performanceSettings.processingThreads = performanceObj->getProperty("processingThreads", 0);
            performanceSettings.pluginThreadPool = performanceObj->getProperty("pluginThreadPool", 4);
            performanceSettings.realTimeProcessing = performanceObj->getProperty("realTimeProcessing", true);
            performanceSettings.useMMCSS = performanceObj->getProperty("useMMCSS", true);
            performanceSettings.guardAgainstDenormals = performanceObj->getProperty("guardAgainstDenormals", true);
        }
        
        if (auto* recordingObj = json.getProperty("recording", nullptr).getDynamicObject()) {
            recordingSettings.prerollTime = recordingObj->getProperty("prerollTime", 2.0);
            recordingSettings.postrollTime = recordingObj->getProperty("postrollTime", 2.0);
            recordingSettings.countInEnabled = recordingObj->getProperty("countInEnabled", true);
            recordingSettings.countInBars = recordingObj->getProperty("countInBars", 1);
            recordingSettings.punchInEnabled = recordingObj->getProperty("punchInEnabled", false);
            recordingSettings.punchOutEnabled = recordingObj->getProperty("punchOutEnabled", false);
            recordingSettings.recordFileFormat = recordingObj->getProperty("recordFileFormat", "wav");
            recordingSettings.recordBitDepth = recordingObj->getProperty("recordBitDepth", 32);
            recordingSettings.recordingPath = recordingObj->getProperty("recordingPath").toString();
            recordingSettings.createTakeFolder = recordingObj->getProperty("createTakeFolder", true);
            recordingSettings.autoQuantize = recordingObj->getProperty("autoQuantize", false);
            recordingSettings.autoQuantizeAmount = recordingObj->getProperty("autoQuantizeAmount", 0.5f);
        }
        
        if (auto* exportObj = json.getProperty("export", nullptr).getDynamicObject()) {
            exportSettings.defaultFormat = exportObj->getProperty("defaultFormat", "wav");
            exportSettings.defaultBitDepth = exportObj->getProperty("defaultBitDepth", 24);
            exportSettings.defaultSampleRate = exportObj->getProperty("defaultSampleRate", 44100);
            exportSettings.normalizeOutput = exportObj->getProperty("normalizeOutput", false);
            exportSettings.normalizationLevel = exportObj->getProperty("normalizationLevel", -1.0f);
            exportSettings.addDithering = exportObj->getProperty("addDithering", true);
            exportSettings.exportMarkers = exportObj->getProperty("exportMarkers", true);
            exportSettings.splitStereoFiles = exportObj->getProperty("splitStereoFiles", false);
            exportSettings.includePluginLatency = exportObj->getProperty("includePluginLatency", true);
        }
        
        LOG_INFO("Configuration loaded successfully");
        
    } catch (const std::exception& e) {
        LOG_ERROR("Error loading configuration: %s", e.what());
    }
}

void Configuration::saveToFile() {
    juce::DynamicObject::Ptr json = new juce::DynamicObject();
    
    // Audio settings
    auto audioObj = new juce::DynamicObject();
    audioObj->setProperty("defaultOutputDevice", audioSettings.outputDevice);
    audioObj->setProperty("defaultInputDevice", audioSettings.inputDevice);
    audioObj->setProperty("sampleRate", audioSettings.sampleRate);
    audioObj->setProperty("bufferSize", audioSettings.bufferSize);
    audioObj->setProperty("inputChannels", audioSettings.inputChannels);
    audioObj->setProperty("outputChannels", audioSettings.outputChannels);
    audioObj->setProperty("bitDepth", audioSettings.bitDepth);
    audioObj->setProperty("dithering", audioSettings.dithering);
    audioObj->setProperty("autoConnectInputs", audioSettings.autoConnectInputs);
    audioObj->setProperty("autoConnectOutputs", audioSettings.autoConnectOutputs);
    json->setProperty("audio", audioObj);
    
    // MIDI settings
    auto midiObj = new juce::DynamicObject();
    midiObj->setProperty("thruEnabled", midiSettings.thruEnabled);
    midiObj->setProperty("clockEnabled", midiSettings.clockEnabled);
    midiObj->setProperty("mtcEnabled", midiSettings.mtcEnabled);
    midiObj->setProperty("mtcFormat", midiSettings.mtcFormat);
    midiObj->setProperty("sendMMC", midiSettings.sendMMC);
    midiObj->setProperty("receiveMMC", midiSettings.receiveMMC);
    midiObj->setProperty("velocityScale", midiSettings.velocityScale);
    midiObj->setProperty("velocityOffset", midiSettings.velocityOffset);
    midiObj->setProperty("defaultInputDevices", midiSettings.inputDevices);
    json->setProperty("midi", midiObj);
    
    // UI settings
    auto uiObj = new juce::DynamicObject();
    
    auto themeObj = new juce::DynamicObject();
    themeObj->setProperty("darkMode", uiSettings.theme.darkMode);
    themeObj->setProperty("accentColor", uiSettings.theme.accentColor.toString());
    themeObj->setProperty("fontSize", uiSettings.theme.fontSize);
    themeObj->setProperty("fontName", uiSettings.theme.fontName);
    themeObj->setProperty("showTooltips", uiSettings.theme.showTooltips);
    themeObj->setProperty("showStatusBar", uiSettings.theme.showStatusBar);
    themeObj->setProperty("showMeterBridges", uiSettings.theme.showMeterBridges);
    uiObj->setProperty("theme", themeObj);
    
    auto layoutObj = new juce::DynamicObject();
    layoutObj->setProperty("mixerVisible", uiSettings.layout.mixerVisible);
    layoutObj->setProperty("pianoRollVisible", uiSettings.layout.pianoRollVisible);
    layoutObj->setProperty("mixerHeight", uiSettings.layout.mixerHeight);
    layoutObj->setProperty("pianoRollHeight", uiSettings.layout.pianoRollHeight);
    layoutObj->setProperty("trackHeight", uiSettings.layout.trackHeight);
    layoutObj->setProperty("minimumTrackHeight", uiSettings.layout.minimumTrackHeight);
    layoutObj->setProperty("maximumTrackHeight", uiSettings.layout.maximumTrackHeight);
    uiObj->setProperty("layout", layoutObj);
    
    auto gridObj = new juce::DynamicObject();
    gridObj->setProperty("snapToGrid", uiSettings.grid.snapToGrid);
    gridObj->setProperty("showGrid", uiSettings.grid.showGrid);
    gridObj->setProperty("gridColor", uiSettings.grid.gridColor.toString());
    gridObj->setProperty("gridOpacity", uiSettings.grid.gridOpacity);
    gridObj->setProperty("majorGridInterval", uiSettings.grid.majorGridInterval);
    gridObj->setProperty("minorGridInterval", uiSettings.grid.minorGridInterval);
    uiObj->setProperty("grid", gridObj);
    
    auto metersObj = new juce::DynamicObject();
    metersObj->setProperty("meterStyle", uiSettings.meters.meterStyle);
    metersObj->setProperty("meterFallback", uiSettings.meters.meterFallback);
    metersObj->setProperty("peakHoldTime", uiSettings.meters.peakHoldTime);
    metersObj->setProperty("rmsWindowSize", uiSettings.meters.rmsWindowSize);
    metersObj->setProperty("showPeakMarkers", uiSettings.meters.showPeakMarkers);
    metersObj->setProperty("showClipIndicators", uiSettings.meters.showClipIndicators);
    uiObj->setProperty("meters", metersObj);
    
    json->setProperty("ui", uiObj);
    
    // Plugin settings
    auto pluginsObj = new juce::DynamicObject();
    pluginsObj->setProperty("scanPaths", pluginSettings.scanPaths);
    pluginsObj->setProperty("blacklist", pluginSettings.blacklist);
    pluginsObj->setProperty("favoritePlugins", pluginSettings.favoritePlugins);
    
    auto formatObj = new juce::DynamicObject();
    formatObj->setProperty("vst3", pluginSettings.format.vst3);
    formatObj->setProperty("au", pluginSettings.format.au);
    formatObj->setProperty("lv2", pluginSettings.format.lv2);
    pluginsObj->setProperty("pluginFormat", formatObj);
    
    auto windowObj = new juce::DynamicObject();
    windowObj->setProperty("alwaysOnTop", pluginSettings.windowBehavior.alwaysOnTop);
    windowObj->setProperty("hideWithHost", pluginSettings.windowBehavior.hideWithHost);
    windowObj->setProperty("rememberPosition", pluginSettings.windowBehavior.rememberPosition);
    pluginsObj->setProperty("pluginWindowBehavior", windowObj);
    
    json->setProperty("plugins", pluginsObj);
    
    // Performance settings
    auto performanceObj = new juce::DynamicObject();
    performanceObj->setProperty("maxVoices", performanceSettings.maxVoices);
    performanceObj->setProperty("diskCacheSize", performanceSettings.diskCacheSize);
    performanceObj->setProperty("ramCacheSize", performanceSettings.ramCacheSize);
    performanceObj->setProperty("processingThreads", performanceSettings.processingThreads);
    performanceObj->setProperty("pluginThreadPool", performanceSettings.pluginThreadPool);
    performanceObj->setProperty("realTimeProcessing", performanceSettings.realTimeProcessing);
    performanceObj->setProperty("useMMCSS", performanceSettings.useMMCSS);
    performanceObj->setProperty("guardAgainstDenormals", performanceSettings.guardAgainstDenormals);
    json->setProperty("performance", performanceObj);
    
    // Recording settings
    auto recordingObj = new juce::DynamicObject();
    recordingObj->setProperty("prerollTime", recordingSettings.prerollTime);
    recordingObj->setProperty("postrollTime", recordingSettings.postrollTime);
    recordingObj->setProperty("countInEnabled", recordingSettings.countInEnabled);
    recordingObj->setProperty("countInBars", recordingSettings.countInBars);
    recordingObj->setProperty("punchInEnabled", recordingSettings.punchInEnabled);
    recordingObj->setProperty("punchOutEnabled", recordingSettings.punchOutEnabled);
    recordingObj->setProperty("recordFileFormat", recordingSettings.recordFileFormat);
    recordingObj->setProperty("recordBitDepth", recordingSettings.recordBitDepth);
    recordingObj->setProperty("recordingPath", recordingSettings.recordingPath);
    recordingObj->setProperty("createTakeFolder", recordingSettings.createTakeFolder);
    recordingObj->setProperty("autoQuantize", recordingSettings.autoQuantize);
    recordingObj->setProperty("autoQuantizeAmount", recordingSettings.autoQuantizeAmount);
    json->setProperty("recording", recordingObj);
    
    // Export settings
    auto exportObj = new juce::DynamicObject();
    exportObj->setProperty("defaultFormat", exportSettings.defaultFormat);
    exportObj->setProperty("defaultBitDepth", exportSettings.defaultBitDepth);
    exportObj->setProperty("defaultSampleRate", exportSettings.defaultSampleRate);
    exportObj->setProperty("normalizeOutput", exportSettings.normalizeOutput);
    exportObj->setProperty("normalizationLevel", exportSettings.normalizationLevel);
    exportObj->setProperty("addDithering", exportSettings.addDithering);
    exportObj->setProperty("exportMarkers", exportSettings.exportMarkers);
    exportObj->setProperty("splitStereoFiles", exportSettings.splitStereoFiles);
    exportObj->setProperty("includePluginLatency", exportSettings.includePluginLatency);
    json->setProperty("export", exportObj);
    
    // Write to file
    const auto configFile = getConfigFile();
    configFile.deleteFile();
    
    if (auto fileStream = std::unique_ptr<juce::FileOutputStream>(configFile.createOutputStream())) {
        const auto jsonString = JSON::toString(json.get(), true);
        if (fileStream->writeText(jsonString, false, false, "\n")) {
            LOG_INFO("Configuration saved successfully");
        } else {
            LOG_ERROR("Error saving configuration");
        }
    }
}

void Configuration::resetToDefaults() {
    loadDefaults();
    saveToFile();
    sendChangeMessage();
}

juce::File Configuration::getConfigDirectory() const {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("DAW_Prototype");
}

juce::File Configuration::getConfigFile() const {
    return getConfigDirectory().getChildFile("config.json");
}

juce::File Configuration::getDefaultProjectDirectory() const {
    return juce::File::getSpecialLocation(juce::File::userDocumentsDirectory)
        .getChildFile("DAW_Prototype Projects");
}

juce::File Configuration::getDefaultRecordingDirectory() const {
    return juce::File::getSpecialLocation(juce::File::userMusicDirectory)
        .getChildFile("DAW_Prototype Recordings");
}

void Configuration::loadDefaults() {
    // Load default settings from resources/config.json
    const auto defaultConfig = juce::File::getCurrentWorkingDirectory()
        .getChildFile("resources")
        .getChildFile("config.json");
        
    if (defaultConfig.existsAsFile()) {
        try {
            auto json = juce::JSON::parse(defaultConfig);
            // TODO: Parse default settings
        } catch (const std::exception& e) {
            LOG_ERROR("Error loading default configuration: %s", e.what());
        }
    }
}

void Configuration::createDefaultDirectories() {
    getConfigDirectory().createDirectory();
    getDefaultProjectDirectory().createDirectory();
    getDefaultRecordingDirectory().createDirectory();
}