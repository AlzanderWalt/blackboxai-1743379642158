#pragma once
#include <JuceHeader.h>
#include "Plugin.h"

class PluginManager : public juce::ChangeBroadcaster {
public:
    // Plugin scan result
    struct ScanResult {
        juce::String path;
        juce::String name;
        juce::String manufacturer;
        juce::String version;
        Plugin::Type type;
        bool isInstrument;
        bool isValid;
        juce::String architecture;
        juce::String error;
    };

    // Plugin instance info
    struct PluginInfo {
        juce::String identifier;
        juce::String name;
        juce::String manufacturer;
        juce::String version;
        Plugin::Type type;
        bool isInstrument;
        int numInputChannels;
        int numOutputChannels;
        juce::Array<juce::AudioProcessorParameter*> parameters;
    };

    // Constructor/Destructor
    PluginManager();
    ~PluginManager() override;

    // Singleton access
    static PluginManager& getInstance();

    // Plugin scanning
    void scanForPlugins(bool async = true);
    bool isScanningPlugins() const { return scanning; }
    float getScanProgress() const { return scanProgress; }
    const juce::StringArray& getLastScanErrors() const { return lastScanErrors; }

    // Plugin management
    int getNumPlugins() const;
    const PluginInfo& getPluginInfo(int index) const;
    const PluginInfo& getPluginInfo(const juce::String& identifier) const;
    
    juce::StringArray getPluginNames() const;
    juce::StringArray getPluginCategories() const;
    juce::StringArray getPluginsInCategory(const juce::String& category) const;
    
    bool isPluginAvailable(const juce::String& identifier) const;
    bool isPluginValid(const juce::String& identifier) const;
    bool isPluginBlacklisted(const juce::String& identifier) const;

    // Plugin creation
    std::unique_ptr<Plugin> createPlugin(Track& track, const juce::String& identifier);
    void releasePlugin(Plugin* plugin);

    // Plugin paths
    void addPluginPath(const juce::String& path);
    void removePluginPath(const juce::String& path);
    juce::StringArray getPluginPaths() const;
    
    // Plugin blacklist
    void addToBlacklist(const juce::String& identifier);
    void removeFromBlacklist(const juce::String& identifier);
    juce::StringArray getBlacklist() const;
    void clearBlacklist();

    // State management
    void saveState(juce::ValueTree& state) const;
    void loadState(const juce::ValueTree& state);

private:
    // Plugin formats
    std::unique_ptr<juce::AudioPluginFormatManager> formatManager;
    std::unique_ptr<juce::KnownPluginList> knownPluginList;
    
    // Plugin paths
    juce::StringArray pluginPaths;
    juce::StringArray blacklist;
    
    // Scan state
    bool scanning{false};
    float scanProgress{0.0f};
    juce::StringArray lastScanErrors;
    
    // Plugin cache
    struct PluginCache {
        juce::File file;
        juce::Time lastModTime;
        PluginInfo info;
    };
    std::map<juce::String, PluginCache> pluginCache;
    
    // Background scanning
    class Scanner;
    std::unique_ptr<Scanner> activeScanner;
    
    // Internal helpers
    void initializeFormats();
    void loadPluginCache();
    void savePluginCache() const;
    void updatePluginCache(const juce::String& identifier, const PluginInfo& info);
    bool validatePluginCache(const juce::String& identifier) const;
    void clearPluginCache();
    
    ScanResult scanPlugin(const juce::String& path);
    void handlePluginScanResult(const ScanResult& result);
    void updateScanProgress(float progress);
    
    juce::String generatePluginIdentifier(const juce::String& path,
                                        const juce::String& name,
                                        const juce::String& format) const;
                                        
    static void cleanupPlugin(Plugin* plugin);

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PluginManager)
};

// Plugin scanner class for background scanning
class PluginManager::Scanner : public juce::Thread {
public:
    Scanner(PluginManager& owner);
    ~Scanner() override;
    
    void run() override;
    float getProgress() const { return progress; }
    
private:
    PluginManager& owner;
    float progress{0.0f};
    juce::StringArray errors;
    
    void scanDirectory(const juce::File& dir);
    void handlePluginFile(const juce::File& file);
    void updateProgress(float newProgress);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Scanner)
};

// Plugin manager utilities
namespace PluginManagerUtils {
    // Plugin validation
    bool validatePluginFile(const juce::File& file);
    bool validatePluginFormat(const juce::String& format);
    bool validatePluginArchitecture(const juce::String& architecture);
    
    // Plugin categorization
    juce::String getPluginCategory(const PluginManager::PluginInfo& info);
    bool isEffectPlugin(const PluginManager::PluginInfo& info);
    bool isInstrumentPlugin(const PluginManager::PluginInfo& info);
    bool isMIDIPlugin(const PluginManager::PluginInfo& info);
    
    // Plugin identification
    juce::String getPluginManufacturerId(const juce::String& manufacturer);
    juce::String getPluginFormatId(Plugin::Type type);
    juce::String generatePluginId(const PluginManager::PluginInfo& info);
    
    // Plugin paths
    juce::StringArray getDefaultPluginPaths();
    juce::File getPluginCacheFile();
    juce::File getPluginBlacklistFile();
}