#include "Project.h"
#include "Logger.h"

Project::Project() {
    createNew();
}

Project::~Project() {
    if (hasUnsavedChanges()) {
        LOG_WARNING("Project closed with unsaved changes");
    }
}

void Project::createNew() {
    // Initialize metadata
    metadata.name = "New Project";
    metadata.created = juce::Time::getCurrentTime();
    metadata.modified = metadata.created;
    
    // Create master track
    masterTrack = std::make_unique<Track>(Track::Type::Master);
    masterTrack->setName("Master");
    
    // Clear all collections
    tracks.clear();
    buses.clear();
    audioFiles.clear();
    midiFiles.clear();
    samples.clear();
    presets.clear();
    
    // Reset history
    clearHistory();
    
    // Reset file path and saved state
    projectFile = juce::File();
    unsavedChanges = false;
    
    notifyProjectChanged();
    LOG_INFO("Created new project: %s", metadata.name.toRawUTF8());
}

bool Project::save(const juce::File& file) {
    try {
        // Create project state
        auto state = getState();
        
        // Create JSON object
        juce::DynamicObject::Ptr json = new juce::DynamicObject();
        
        // Add version info
        json->setProperty("version", "1.0.0");
        
        // Add metadata
        auto metadataObj = new juce::DynamicObject();
        metadataObj->setProperty("name", metadata.name);
        metadataObj->setProperty("author", metadata.author);
        metadataObj->setProperty("created", metadata.created.toISO8601(true));
        metadataObj->setProperty("modified", metadata.modified.toISO8601(true));
        metadataObj->setProperty("description", metadata.description);
        metadataObj->setProperty("tags", metadata.tags.joinIntoString(","));
        metadataObj->setProperty("category", metadata.category);
        json->setProperty("metadata", metadataObj);
        
        // Add settings
        auto settingsObj = new juce::DynamicObject();
        settingsObj->setProperty("tempo", settings.tempo);
        settingsObj->setProperty("timeSignature", 
            juce::String(settings.timeSignature.numerator) + "/" + 
            juce::String(settings.timeSignature.denominator));
        settingsObj->setProperty("key", settings.key);
        settingsObj->setProperty("scale", settings.scale);
        settingsObj->setProperty("length", settings.length);
        settingsObj->setProperty("sampleRate", settings.sampleRate);
        settingsObj->setProperty("bitDepth", settings.bitDepth);
        json->setProperty("settings", settingsObj);
        
        // Add transport state
        auto transportObj = new juce::DynamicObject();
        transportObj->setProperty("loopEnabled", transportState.loopEnabled);
        transportObj->setProperty("loopStart", transportState.loopStart);
        transportObj->setProperty("loopEnd", transportState.loopEnd);
        transportObj->setProperty("timeRulerOffset", transportState.timeRulerOffset);
        transportObj->setProperty("snapToGrid", transportState.snapToGrid);
        transportObj->setProperty("gridSize", transportState.gridSize);
        
        // Add markers
        juce::Array<juce::var> markersArray;
        for (const auto& marker : transportState.markers) {
            auto markerObj = new juce::DynamicObject();
            markerObj->setProperty("time", marker.first);
            markerObj->setProperty("name", marker.second);
            markersArray.add(markerObj);
        }
        transportObj->setProperty("markers", markersArray);
        json->setProperty("transport", transportObj);
        
        // Add tracks
        juce::Array<juce::var> tracksArray;
        for (auto* track : tracks) {
            tracksArray.add(track->getState());
        }
        json->setProperty("tracks", tracksArray);
        
        // Add buses
        juce::Array<juce::var> busesArray;
        for (auto* bus : buses) {
            busesArray.add(bus->getState());
        }
        json->setProperty("buses", busesArray);
        
        // Add master track
        json->setProperty("masterTrack", masterTrack->getState());
        
        // Add resources
        auto resourcesObj = new juce::DynamicObject();
        resourcesObj->setProperty("audioFiles", getRelativeFilePaths(audioFiles));
        resourcesObj->setProperty("midiFiles", getRelativeFilePaths(midiFiles));
        resourcesObj->setProperty("samples", getRelativeFilePaths(samples));
        resourcesObj->setProperty("presets", getRelativeFilePaths(presets));
        json->setProperty("resources", resourcesObj);
        
        // Write to file
        if (auto fileStream = std::unique_ptr<juce::FileOutputStream>(file.createOutputStream())) {
            const auto jsonString = juce::JSON::toString(json.get(), true);
            fileStream->writeText(jsonString, false, false, "\n");
            fileStream->flush();
            
            projectFile = file;
            unsavedChanges = false;
            updateModifiedTime();
            
            LOG_INFO("Project saved: %s", file.getFullPathName().toRawUTF8());
            return true;
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to save project: %s", e.what());
    }
    
    return false;
}

bool Project::load(const juce::File& file) {
    try {
        if (auto fileStream = std::unique_ptr<juce::FileInputStream>(file.createInputStream())) {
            const auto jsonString = fileStream->readEntireStreamAsString();
            auto json = juce::JSON::parse(jsonString);
            
            if (json.isObject()) {
                // Clear current project
                createNew();
                
                // Load metadata
                if (auto* metadataObj = json.getProperty("metadata", {}).getDynamicObject()) {
                    metadata.name = metadataObj->getProperty("name").toString();
                    metadata.author = metadataObj->getProperty("author").toString();
                    metadata.created = juce::Time::fromISO8601(metadataObj->getProperty("created").toString());
                    metadata.modified = juce::Time::fromISO8601(metadataObj->getProperty("modified").toString());
                    metadata.description = metadataObj->getProperty("description").toString();
                    metadata.tags.addTokens(metadataObj->getProperty("tags").toString(), ",", "");
                    metadata.category = metadataObj->getProperty("category").toString();
                }
                
                // Load settings
                if (auto* settingsObj = json.getProperty("settings", {}).getDynamicObject()) {
                    settings.tempo = settingsObj->getProperty("tempo");
                    const auto timeSignature = settingsObj->getProperty("timeSignature").toString();
                    settings.timeSignature.numerator = timeSignature.upToFirstOccurrenceOf("/", false, true).getIntValue();
                    settings.timeSignature.denominator = timeSignature.fromLastOccurrenceOf("/", false, true).getIntValue();
                    settings.key = settingsObj->getProperty("key").toString();
                    settings.scale = settingsObj->getProperty("scale").toString();
                    settings.length = settingsObj->getProperty("length");
                    settings.sampleRate = settingsObj->getProperty("sampleRate");
                    settings.bitDepth = settingsObj->getProperty("bitDepth");
                }
                
                // Load transport state
                if (auto* transportObj = json.getProperty("transport", {}).getDynamicObject()) {
                    transportState.loopEnabled = transportObj->getProperty("loopEnabled");
                    transportState.loopStart = transportObj->getProperty("loopStart");
                    transportState.loopEnd = transportObj->getProperty("loopEnd");
                    transportState.timeRulerOffset = transportObj->getProperty("timeRulerOffset");
                    transportState.snapToGrid = transportObj->getProperty("snapToGrid");
                    transportState.gridSize = transportObj->getProperty("gridSize");
                    
                    // Load markers
                    if (auto* markersArray = transportObj->getProperty("markers").getArray()) {
                        for (const auto& marker : *markersArray) {
                            if (auto* markerObj = marker.getDynamicObject()) {
                                transportState.markers.add({
                                    markerObj->getProperty("time"),
                                    markerObj->getProperty("name").toString()
                                });
                            }
                        }
                    }
                }
                
                // Load tracks
                if (auto* tracksArray = json.getProperty("tracks", {}).getArray()) {
                    for (const auto& trackState : *tracksArray) {
                        if (auto* track = addTrack(static_cast<Track::Type>(
                            trackState.getProperty("type", 0).toString().getIntValue()))) {
                            track->restoreState(trackState);
                        }
                    }
                }
                
                // Load buses
                if (auto* busesArray = json.getProperty("buses", {}).getArray()) {
                    for (const auto& busState : *busesArray) {
                        if (auto* bus = addBus(busState.getProperty("name", "Bus").toString())) {
                            bus->restoreState(busState);
                        }
                    }
                }
                
                // Load master track
                if (auto masterState = json.getProperty("masterTrack", {})) {
                    masterTrack->restoreState(masterState);
                }
                
                // Load resources
                if (auto* resourcesObj = json.getProperty("resources", {}).getDynamicObject()) {
                    loadResourceFiles(audioFiles, resourcesObj->getProperty("audioFiles"), file.getParentDirectory());
                    loadResourceFiles(midiFiles, resourcesObj->getProperty("midiFiles"), file.getParentDirectory());
                    loadResourceFiles(samples, resourcesObj->getProperty("samples"), file.getParentDirectory());
                    loadResourceFiles(presets, resourcesObj->getProperty("presets"), file.getParentDirectory());
                }
                
                projectFile = file;
                unsavedChanges = false;
                
                notifyProjectChanged();
                LOG_INFO("Project loaded: %s", file.getFullPathName().toRawUTF8());
                return true;
            }
        }
    } catch (const std::exception& e) {
        LOG_ERROR("Failed to load project: %s", e.what());
    }
    
    return false;
}

void Project::setMetadata(const Metadata& newMetadata) {
    metadata = newMetadata;
    markAsUnsaved();
    notifyProjectChanged();
}

void Project::setSettings(const Settings& newSettings) {
    settings = newSettings;
    markAsUnsaved();
    notifyProjectChanged();
}

void Project::setTransportState(const TransportState& newState) {
    transportState = newState;
    markAsUnsaved();
    notifyProjectChanged();
}

Track* Project::addTrack(Track::Type type) {
    auto track = new Track(type);
    tracks.add(track);
    markAsUnsaved();
    notifyProjectChanged();
    return track;
}

void Project::removeTrack(Track* track) {
    if (tracks.removeObject(track)) {
        markAsUnsaved();
        notifyProjectChanged();
    }
}

void Project::moveTrack(int fromIndex, int toIndex) {
    if (tracks.move(fromIndex, toIndex)) {
        markAsUnsaved();
        notifyProjectChanged();
    }
}

Track* Project::getTrackByID(const juce::String& id) const {
    for (auto* track : tracks) {
        if (track->getID() == id) {
            return track;
        }
    }
    return nullptr;
}

Track* Project::addBus(const juce::String& name) {
    auto bus = new Track(Track::Type::Bus);
    bus->setName(name);
    buses.add(bus);
    markAsUnsaved();
    notifyProjectChanged();
    return bus;
}

void Project::removeBus(Track* bus) {
    if (buses.removeObject(bus)) {
        markAsUnsaved();
        notifyProjectChanged();
    }
}

Track* Project::getBusByID(const juce::String& id) const {
    for (auto* bus : buses) {
        if (bus->getID() == id) {
            return bus;
        }
    }
    return nullptr;
}

void Project::addPluginToTrack(Track* track, const juce::String& pluginID) {
    if (track != nullptr) {
        track->addPlugin(pluginID);
        markAsUnsaved();
        notifyProjectChanged();
    }
}

void Project::removePluginFromTrack(Track* track, int index) {
    if (track != nullptr) {
        track->removePlugin(index);
        markAsUnsaved();
        notifyProjectChanged();
    }
}

void Project::movePlugin(Track* track, int fromIndex, int toIndex) {
    if (track != nullptr) {
        track->movePlugin(fromIndex, toIndex);
        markAsUnsaved();
        notifyProjectChanged();
    }
}

void Project::addAudioFile(const juce::File& file) {
    if (!audioFiles.contains(file)) {
        audioFiles.add(file);
        markAsUnsaved();
        notifyProjectChanged();
    }
}

void Project::addMIDIFile(const juce::File& file) {
    if (!midiFiles.contains(file)) {
        midiFiles.add(file);
        markAsUnsaved();
        notifyProjectChanged();
    }
}

void Project::addSample(const juce::File& file) {
    if (!samples.contains(file)) {
        samples.add(file);
        markAsUnsaved();
        notifyProjectChanged();
    }
}

void Project::addPreset(const juce::File& file) {
    if (!presets.contains(file)) {
        presets.add(file);
        markAsUnsaved();
        notifyProjectChanged();
    }
}

void Project::undo() {
    if (canUndo()) {
        redoHistory.add({getState(), "Undo"});
        restoreState(undoHistory.getLast().state);
        undoHistory.removeLast();
        notifyProjectChanged();
    }
}

void Project::redo() {
    if (canRedo()) {
        undoHistory.add({getState(), "Redo"});
        restoreState(redoHistory.getLast().state);
        redoHistory.removeLast();
        notifyProjectChanged();
    }
}

bool Project::canUndo() const {
    return !undoHistory.isEmpty();
}

bool Project::canRedo() const {
    return !redoHistory.isEmpty();
}

void Project::clearHistory() {
    undoHistory.clear();
    redoHistory.clear();
}

void Project::saveState() {
    addToHistory("Save state");
}

void Project::restoreState(const juce::ValueTree& state) {
    // TODO: Implement state restoration
    markAsUnsaved();
    notifyProjectChanged();
}

juce::ValueTree Project::getState() const {
    juce::ValueTree state("Project");
    // TODO: Implement state saving
    return state;
}

void Project::addToHistory(const juce::String& description) {
    undoHistory.add({getState(), description});
    if (undoHistory.size() > maxHistorySize) {
        undoHistory.remove(0);
    }
    redoHistory.clear();
}

void Project::updateModifiedTime() {
    metadata.modified = juce::Time::getCurrentTime();
}

void Project::notifyProjectChanged() {
    sendChangeMessage();
}

juce::StringArray Project::getRelativeFilePaths(const juce::Array<juce::File>& files) const {
    juce::StringArray paths;
    if (projectFile != juce::File()) {
        const auto projectDir = projectFile.getParentDirectory();
        for (const auto& file : files) {
            paths.add(file.getRelativePathFrom(projectDir));
        }
    }
    return paths;
}

void Project::loadResourceFiles(juce::Array<juce::File>& files,
                              const juce::var& paths,
                              const juce::File& projectDir) {
    files.clear();
    if (auto* pathsArray = paths.getArray()) {
        for (const auto& path : *pathsArray) {
            files.add(projectDir.getChildFile(path.toString()));
        }
    }
}