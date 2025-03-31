#include "PluginManager.h"
#include "Logger.h"
#include "Configuration.h"

//==============================================================================
// PluginManager Implementation
//==============================================================================

PluginManager::PluginManager() {
    initializeFormats();
    loadPluginCache();
}

PluginManager::~PluginManager() {
    if (activeScanner != nullptr) {
        activeScanner->signalThreadShouldExit();
        activeScanner->waitForThreadToExit(5000);
    }
    savePluginCache();
}

PluginManager& PluginManager::getInstance() {
    static PluginManager instance;
    return instance;
}

void PluginManager::scanForPlugins(bool async) {
    if (scanning) {
        LOG_WARNING("Plugin scan already in progress");
        return;
    }

    if (async) {
        activeScanner = std::make_unique<Scanner>(*this);
        activeScanner->startThread();
        scanning = true;
        LOG_INFO("Started asynchronous plugin scan");
    } else {
        scanning = true;
        scanProgress = 0.0f;
        lastScanErrors.clear();
        
        LOG_INFO("Starting synchronous plugin scan");
        
        for (const auto& path : pluginPaths) {
            juce::File dir(path);
            if (dir.exists() && dir.isDirectory()) {
                juce::Array<juce::File> files;
                dir.findChildFiles(files, juce::File::findFiles, true,
                                 "*.vst3;*.component");
                
                float progressPerFile = 1.0f / files.size();
                
                for (const auto& file : files) {
                    auto result = scanPlugin(file.getFullPathName());
                    handlePluginScanResult(result);
                    scanProgress += progressPerFile;
                    sendChangeMessage();
                }
            }
        }
        
        scanning = false;
        scanProgress = 1.0f;
        sendChangeMessage();
        
        LOG_INFO("Completed plugin scan. Found %d plugins with %d errors",
                 getNumPlugins(), lastScanErrors.size());
    }
}

int PluginManager::getNumPlugins() const {
    return static_cast<int>(pluginCache.size());
}

const PluginManager::PluginInfo& PluginManager::getPluginInfo(int index) const {
    auto it = std::next(pluginCache.begin(), index);
    return it->second.info;
}

const PluginManager::PluginInfo& PluginManager::getPluginInfo(const juce::String& identifier) const {
    auto it = pluginCache.find(identifier);
    jassert(it != pluginCache.end());
    return it->second.info;
}

juce::StringArray PluginManager::getPluginNames() const {
    juce::StringArray names;
    for (const auto& plugin : pluginCache) {
        names.add(plugin.second.info.name);
    }
    return names;
}

juce::StringArray PluginManager::getPluginCategories() const {
    juce::StringArray categories;
    for (const auto& plugin : pluginCache) {
        juce::String category = PluginManagerUtils::getPluginCategory(plugin.second.info);
        if (!categories.contains(category)) {
            categories.add(category);
        }
    }
    return categories;
}

juce::StringArray PluginManager::getPluginsInCategory(const juce::String& category) const {
    juce::StringArray plugins;
    for (const auto& plugin : pluginCache) {
        if (PluginManagerUtils::getPluginCategory(plugin.second.info) == category) {
            plugins.add(plugin.first);
        }
    }
    return plugins;
}

bool PluginManager::isPluginAvailable(const juce::String& identifier) const {
    return pluginCache.find(identifier) != pluginCache.end();
}

bool PluginManager::isPluginValid(const juce::String& identifier) const {
    auto it = pluginCache.find(identifier);
    if (it != pluginCache.end()) {
        return validatePluginCache(identifier);
    }
    return false;
}

bool PluginManager::isPluginBlacklisted(const juce::String& identifier) const {
    return blacklist.contains(identifier);
}

std::unique_ptr<Plugin> PluginManager::createPlugin(Track& track, const juce::String& identifier) {
    if (!isPluginAvailable(identifier) || isPluginBlacklisted(identifier)) {
        return nullptr;
    }

    const auto& cache = pluginCache.at(identifier);
    
    try {
        juce::AudioPluginFormat* format = nullptr;
        
        switch (cache.info.type) {
            case Plugin::Type::VST3:
                format = formatManager->getFormat(0);  // VST3 format
                break;
                
            case Plugin::Type::AudioUnit:
                #if JUCE_PLUGINHOST_AU && JUCE_MAC
                format = formatManager->getFormat(1);  // AU format
                #endif
                break;
                
            default:
                break;
        }
        
        if (format != nullptr) {
            juce::String error;
            std::unique_ptr<juce::AudioPluginInstance> instance(
                format->createInstanceFromDescription(
                    knownPluginList->getTypeForFile(cache.file.getFullPathName()),
                    44100.0, 512, error));
                    
            if (instance != nullptr) {
                // TODO: Create wrapper plugin class
                LOG_INFO("Created plugin instance: %s", cache.info.name);
                return nullptr;  // Replace with actual plugin instance
            }
            
            LOG_ERROR("Failed to create plugin instance: %s (%s)",
                     cache.info.name, error);
        }
    }
    catch (const std::exception& e) {
        LOG_ERROR("Exception creating plugin instance: %s (%s)",
                 cache.info.name, e.what());
    }
    
    return nullptr;
}

void PluginManager::releasePlugin(Plugin* plugin) {
    if (plugin != nullptr) {
        cleanupPlugin(plugin);
    }
}

void PluginManager::addPluginPath(const juce::String& path) {
    if (!pluginPaths.contains(path)) {
        pluginPaths.add(path);
        sendChangeMessage();
    }
}

void PluginManager::removePluginPath(const juce::String& path) {
    pluginPaths.removeString(path);
    sendChangeMessage();
}

juce::StringArray PluginManager::getPluginPaths() const {
    return pluginPaths;
}

void PluginManager::addToBlacklist(const juce::String& identifier) {
    if (!blacklist.contains(identifier)) {
        blacklist.add(identifier);
        sendChangeMessage();
    }
}

void PluginManager::removeFromBlacklist(const juce::String& identifier) {
    blacklist.removeString(identifier);
    sendChangeMessage();
}

juce::StringArray PluginManager::getBlacklist() const {
    return blacklist;
}

void PluginManager::clearBlacklist() {
    blacklist.clear();
    sendChangeMessage();
}

void PluginManager::saveState(juce::ValueTree& state) const {
    // Save plugin paths
    auto pathsNode = state.getOrCreateChildWithName("pluginPaths", nullptr);
    pathsNode.removeAllChildren(nullptr);
    for (const auto& path : pluginPaths) {
        auto child = pathsNode.createChild("path");
        child.setProperty("value", path, nullptr);
    }
    
    // Save blacklist
    auto blacklistNode = state.getOrCreateChildWithName("blacklist", nullptr);
    blacklistNode.removeAllChildren(nullptr);
    for (const auto& identifier : blacklist) {
        auto child = blacklistNode.createChild("plugin");
        child.setProperty("identifier", identifier, nullptr);
    }
}

void PluginManager::loadState(const juce::ValueTree& state) {
    // Load plugin paths
    pluginPaths.clear();
    if (auto pathsNode = state.getChildWithName("pluginPaths")) {
        for (auto child : pathsNode) {
            pluginPaths.add(child.getProperty("value").toString());
        }
    }
    
    // Load blacklist
    blacklist.clear();
    if (auto blacklistNode = state.getChildWithName("blacklist")) {
        for (auto child : blacklistNode) {
            blacklist.add(child.getProperty("identifier").toString());
        }
    }
    
    sendChangeMessage();
}

void PluginManager::initializeFormats() {
    formatManager = std::make_unique<juce::AudioPluginFormatManager>();
    formatManager->addDefaultFormats();
    
    knownPluginList = std::make_unique<juce::KnownPluginList>();
    
    // Add default plugin paths
    pluginPaths = PluginManagerUtils::getDefaultPluginPaths();
}

void PluginManager::loadPluginCache() {
    juce::File cacheFile = PluginManagerUtils::getPluginCacheFile();
    if (cacheFile.exists()) {
        juce::XmlDocument doc(cacheFile);
        if (auto xml = doc.getDocumentElement()) {
            knownPluginList->recreateFromXml(*xml);
        }
    }
    
    // Load blacklist
    juce::File blacklistFile = PluginManagerUtils::getPluginBlacklistFile();
    if (blacklistFile.exists()) {
        blacklist.addLines(blacklistFile.loadFileAsString());
    }
}

void PluginManager::savePluginCache() const {
    if (auto xml = knownPluginList->createXml()) {
        juce::File cacheFile = PluginManagerUtils::getPluginCacheFile();
        xml->writeTo(cacheFile);
    }
    
    // Save blacklist
    juce::File blacklistFile = PluginManagerUtils::getPluginBlacklistFile();
    blacklistFile.replaceWithText(blacklist.joinIntoString("\n"));
}

void PluginManager::updatePluginCache(const juce::String& identifier, const PluginInfo& info) {
    PluginCache cache;
    cache.file = juce::File(info.identifier);
    cache.lastModTime = cache.file.getLastModificationTime();
    cache.info = info;
    
    pluginCache[identifier] = std::move(cache);
}

bool PluginManager::validatePluginCache(const juce::String& identifier) const {
    auto it = pluginCache.find(identifier);
    if (it != pluginCache.end()) {
        const auto& cache = it->second;
        return cache.file.exists() &&
               cache.file.getLastModificationTime() == cache.lastModTime;
    }
    return false;
}

void PluginManager::clearPluginCache() {
    pluginCache.clear();
    knownPluginList->clear();
    juce::File cacheFile = PluginManagerUtils::getPluginCacheFile();
    cacheFile.deleteFile();
}

PluginManager::ScanResult PluginManager::scanPlugin(const juce::String& path) {
    ScanResult result;
    result.path = path;
    result.isValid = false;
    
    try {
        juce::File file(path);
        if (!file.exists()) {
            result.error = "File does not exist";
            return result;
        }
        
        // Determine plugin format
        juce::AudioPluginFormat* format = nullptr;
        if (file.hasFileExtension(".vst3")) {
            format = formatManager->getFormat(0);  // VST3 format
            result.type = Plugin::Type::VST3;
        }
        #if JUCE_PLUGINHOST_AU && JUCE_MAC
        else if (file.hasFileExtension(".component")) {
            format = formatManager->getFormat(1);  // AU format
            result.type = Plugin::Type::AudioUnit;
        }
        #endif
        
        if (format == nullptr) {
            result.error = "Unsupported plugin format";
            return result;
        }
        
        // Scan plugin
        juce::String error;
        auto types = format->findAllTypesForFile(file);
        
        if (types.isEmpty()) {
            result.error = "No plugin types found";
            return result;
        }
        
        // Get plugin info
        const auto& type = types.getReference(0);
        result.name = type.name;
        result.manufacturer = type.manufacturerName;
        result.version = type.version;
        result.isInstrument = type.isInstrument;
        result.architecture = PluginUtils::getPluginArchitecture(path);
        result.isValid = true;
        
        return result;
    }
    catch (const std::exception& e) {
        result.error = e.what();
        return result;
    }
}

void PluginManager::handlePluginScanResult(const ScanResult& result) {
    if (result.isValid) {
        PluginInfo info;
        info.identifier = result.path;
        info.name = result.name;
        info.manufacturer = result.manufacturer;
        info.version = result.version;
        info.type = result.type;
        info.isInstrument = result.isInstrument;
        
        updatePluginCache(result.path, info);
    } else {
        lastScanErrors.add(result.path + ": " + result.error);
    }
}

void PluginManager::updateScanProgress(float progress) {
    scanProgress = progress;
    sendChangeMessage();
}

juce::String PluginManager::generatePluginIdentifier(const juce::String& path,
                                                   const juce::String& name,
                                                   const juce::String& format) const {
    return path + "|" + name + "|" + format;
}

void PluginManager::cleanupPlugin(Plugin* plugin) {
    // TODO: Implement plugin cleanup
}

//==============================================================================
// Scanner Implementation
//==============================================================================

PluginManager::Scanner::Scanner(PluginManager& owner)
    : juce::Thread("PluginScanner")
    , owner(owner) {
}

PluginManager::Scanner::~Scanner() {
    stopThread(5000);
}

void PluginManager::Scanner::run() {
    progress = 0.0f;
    errors.clear();
    
    for (const auto& path : owner.getPluginPaths()) {
        if (threadShouldExit()) {
            break;
        }
        
        scanDirectory(juce::File(path));
    }
    
    owner.scanning = false;
    owner.scanProgress = 1.0f;
    owner.lastScanErrors = errors;
    owner.sendChangeMessage();
    
    LOG_INFO("Background plugin scan complete");
}

void PluginManager::Scanner::scanDirectory(const juce::File& dir) {
    if (!dir.exists() || !dir.isDirectory()) {
        return;
    }
    
    juce::Array<juce::File> files;
    dir.findChildFiles(files, juce::File::findFiles, true,
                      "*.vst3;*.component");
                      
    float progressPerFile = 1.0f / files.size();
    
    for (const auto& file : files) {
        if (threadShouldExit()) {
            return;
        }
        
        handlePluginFile(file);
        progress += progressPerFile;
        owner.updateScanProgress(progress);
    }
}

void PluginManager::Scanner::handlePluginFile(const juce::File& file) {
    auto result = owner.scanPlugin(file.getFullPathName());
    if (!result.isValid) {
        errors.add(result.path + ": " + result.error);
    }
    owner.handlePluginScanResult(result);
}

void PluginManager::Scanner::updateProgress(float newProgress) {
    progress = newProgress;
    owner.updateScanProgress(progress);
}

//==============================================================================
// PluginManagerUtils Implementation
//==============================================================================

namespace PluginManagerUtils {
    bool validatePluginFile(const juce::File& file) {
        return file.exists() && (file.hasFileExtension(".vst3")
            #if JUCE_PLUGINHOST_AU && JUCE_MAC
            || file.hasFileExtension(".component")
            #endif
        );
    }

    bool validatePluginFormat(const juce::String& format) {
        return format == "VST3"
            #if JUCE_PLUGINHOST_AU && JUCE_MAC
            || format == "AudioUnit"
            #endif
        ;
    }

    bool validatePluginArchitecture(const juce::String& architecture) {
        #if JUCE_64BIT
        return architecture == "x64";
        #else
        return architecture == "x86";
        #endif
    }

    juce::String getPluginCategory(const PluginManager::PluginInfo& info) {
        if (info.isInstrument) {
            return "Instruments";
        }
        // TODO: Implement more detailed categorization
        return "Effects";
    }

    bool isEffectPlugin(const PluginManager::PluginInfo& info) {
        return !info.isInstrument;
    }

    bool isInstrumentPlugin(const PluginManager::PluginInfo& info) {
        return info.isInstrument;
    }

    bool isMIDIPlugin(const PluginManager::PluginInfo& info) {
        // TODO: Implement MIDI plugin detection
        return false;
    }

    juce::String getPluginManufacturerId(const juce::String& manufacturer) {
        return manufacturer.toLowerCase().replaceCharacters(" .", "__");
    }

    juce::String getPluginFormatId(Plugin::Type type) {
        switch (type) {
            case Plugin::Type::VST3: return "VST3";
            case Plugin::Type::AudioUnit: return "AU";
            case Plugin::Type::Internal: return "Internal";
            default: return "Unknown";
        }
    }

    juce::String generatePluginId(const PluginManager::PluginInfo& info) {
        return getPluginManufacturerId(info.manufacturer) + "." +
               info.name.toLowerCase().replaceCharacters(" ", "_") + "." +
               getPluginFormatId(info.type);
    }

    juce::StringArray getDefaultPluginPaths() {
        juce::StringArray paths;
        
        #if JUCE_WINDOWS
        paths.add("C:\\Program Files\\Common Files\\VST3");
        paths.add("C:\\Program Files\\VSTPlugins");
        #elif JUCE_MAC
        paths.add("/Library/Audio/Plug-Ins/VST3");
        paths.add("/Library/Audio/Plug-Ins/Components");
        paths.add("~/Library/Audio/Plug-Ins/VST3");
        paths.add("~/Library/Audio/Plug-Ins/Components");
        #elif JUCE_LINUX
        paths.add("/usr/lib/vst3");
        paths.add("/usr/local/lib/vst3");
        paths.add("~/.vst3");
        #endif
        
        return paths;
    }

    juce::File getPluginCacheFile() {
        return juce::File::getSpecialLocation(
                   juce::File::userApplicationDataDirectory)
                   .getChildFile("DAW_Prototype")
                   .getChildFile("PluginCache.xml");
    }

    juce::File getPluginBlacklistFile() {
        return juce::File::getSpecialLocation(
                   juce::File::userApplicationDataDirectory)
                   .getChildFile("DAW_Prototype")
                   .getChildFile("PluginBlacklist.txt");
    }
}