#pragma once
#include <JuceHeader.h>
#include "Track.h"
#include "Mixer.h"

class Project : public juce::ChangeBroadcaster {
public:
    // Project metadata
    struct Metadata {
        juce::String name;
        juce::String author;
        juce::Time created;
        juce::Time modified;
        juce::String description;
        juce::StringArray tags;
        juce::String category;
    };

    // Project settings
    struct Settings {
        double tempo{120.0};
        struct TimeSignature {
            int numerator{4};
            int denominator{4};
        } timeSignature;
        juce::String key{"C"};
        juce::String scale{"major"};
        double length{240.0};  // In seconds
        double sampleRate{44100.0};
        int bitDepth{32};
    };

    // Transport state
    struct TransportState {
        bool loopEnabled{false};
        double loopStart{0.0};
        double loopEnd{4.0};
        juce::Array<std::pair<double, juce::String>> markers;
        double timeRulerOffset{0.0};
        bool snapToGrid{true};
        double gridSize{0.25};  // In beats
    };

    // Constructor/Destructor
    Project();
    ~Project() override;

    // File operations
    bool save(const juce::File& file);
    bool load(const juce::File& file);
    void createNew();
    
    // Project info
    const Metadata& getMetadata() const { return metadata; }
    void setMetadata(const Metadata& newMetadata);
    
    const Settings& getSettings() const { return settings; }
    void setSettings(const Settings& newSettings);
    
    const TransportState& getTransportState() const { return transportState; }
    void setTransportState(const TransportState& newState);
    
    juce::File getProjectFile() const { return projectFile; }
    bool hasUnsavedChanges() const { return unsavedChanges; }

    // Track management
    Track* addTrack(Track::Type type);
    void removeTrack(Track* track);
    void moveTrack(int fromIndex, int toIndex);
    const juce::OwnedArray<Track>& getTracks() const { return tracks; }
    Track* getTrackByID(const juce::String& id) const;
    Track* getMasterTrack() const { return masterTrack.get(); }

    // Bus management
    Track* addBus(const juce::String& name);
    void removeBus(Track* bus);
    const juce::OwnedArray<Track>& getBuses() const { return buses; }
    Track* getBusByID(const juce::String& id) const;

    // Plugin management
    void addPluginToTrack(Track* track, const juce::String& pluginID);
    void removePluginFromTrack(Track* track, int index);
    void movePlugin(Track* track, int fromIndex, int toIndex);
    
    // Resource management
    void addAudioFile(const juce::File& file);
    void addMIDIFile(const juce::File& file);
    void addSample(const juce::File& file);
    void addPreset(const juce::File& file);
    const juce::Array<juce::File>& getAudioFiles() const { return audioFiles; }
    const juce::Array<juce::File>& getMIDIFiles() const { return midiFiles; }
    const juce::Array<juce::File>& getSamples() const { return samples; }
    const juce::Array<juce::File>& getPresets() const { return presets; }

    // History management
    void undo();
    void redo();
    bool canUndo() const;
    bool canRedo() const;
    void clearHistory();

    // Project state
    void saveState();
    void restoreState(const juce::ValueTree& state);
    juce::ValueTree getState() const;

private:
    Metadata metadata;
    Settings settings;
    TransportState transportState;
    
    juce::File projectFile;
    bool unsavedChanges{false};
    
    std::unique_ptr<Track> masterTrack;
    juce::OwnedArray<Track> tracks;
    juce::OwnedArray<Track> buses;
    
    juce::Array<juce::File> audioFiles;
    juce::Array<juce::File> midiFiles;
    juce::Array<juce::File> samples;
    juce::Array<juce::File> presets;
    
    struct HistoryState {
        juce::ValueTree state;
        juce::String description;
    };
    
    juce::Array<HistoryState> undoHistory;
    juce::Array<HistoryState> redoHistory;
    int maxHistorySize{100};
    
    void markAsUnsaved() { unsavedChanges = true; }
    void addToHistory(const juce::String& description);
    void updateModifiedTime();
    void notifyProjectChanged();
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Project)
};