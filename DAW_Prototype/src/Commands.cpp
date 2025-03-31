#include "Commands.h"
#include "Project.h"
#include "Logger.h"

Commands::Commands() {
    // Register as command target
    juce::JUCEApplication::getInstance()->setApplicationCommandManagerToWatch(
        juce::JUCEApplication::getInstance()->getCommandManager());
}

Commands::~Commands() {
    // Nothing to clean up
}

void Commands::setProject(Project* newProject) {
    project = newProject;
}

bool Commands::canUndo() const {
    return project != nullptr && project->canUndo();
}

bool Commands::canRedo() const {
    return project != nullptr && project->canRedo();
}

bool Commands::hasSelection() const {
    // TODO: Implement selection check
    return false;
}

bool Commands::hasClipboard() const {
    // TODO: Implement clipboard check
    return false;
}

bool Commands::isPlaying() const {
    // TODO: Implement playback check
    return false;
}

bool Commands::isRecording() const {
    // TODO: Implement recording check
    return false;
}

bool Commands::isLooping() const {
    return project != nullptr && project->getTransportState().loopEnabled;
}

juce::ApplicationCommandTarget* Commands::getNextCommandTarget() {
    return nullptr;
}

void Commands::getAllCommands(juce::Array<juce::CommandID>& commands) {
    const juce::CommandID ids[] = {
        NewProject,
        OpenProject,
        SaveProject,
        SaveProjectAs,
        ExportAudio,
        ExportMIDI,
        ImportAudio,
        ImportMIDI,
        
        Undo,
        Redo,
        Cut,
        Copy,
        Paste,
        Delete,
        SelectAll,
        SelectNone,
        DuplicateSelection,
        SplitAtCursor,
        Merge,
        
        AddAudioTrack,
        AddMIDITrack,
        AddBusTrack,
        DeleteSelectedTracks,
        DuplicateSelectedTracks,
        MuteSelectedTracks,
        SoloSelectedTracks,
        ArmSelectedTracks,
        FreezeSelectedTracks,
        UnfreezeSelectedTracks,
        
        Play,
        Stop,
        Record,
        FastForward,
        Rewind,
        ToggleLoop,
        ToggleMetronome,
        ToggleCountIn,
        SetTempo,
        SetTimeSignature,
        
        ToggleMixer,
        TogglePianoRoll,
        ToggleAutomation,
        ToggleGrid,
        ZoomIn,
        ZoomOut,
        ZoomToSelection,
        ZoomToFit,
        
        ShowPluginManager,
        ShowPluginBrowser,
        ScanForPlugins,
        EnableSelectedPlugins,
        DisableSelectedPlugins,
        
        SelectTool,
        DrawTool,
        EraseTool,
        SplitTool,
        GlueTool,
        FadeTool,
        
        QuantizeSelection,
        TransposeSelection,
        VelocityAdjust,
        LengthAdjust,
        LegateNotes,
        ChordMode,
        ArpeggiateMode,
        
        NormalizeAudio,
        ReverseAudio,
        FadeIn,
        FadeOut,
        CrossFade,
        TimeStretch,
        PitchShift,
        
        ShowPreferences,
        ShowAudioSettings,
        ShowMIDISettings,
        ShowKeyboardShortcuts,
        
        ShowHelp,
        ShowTutorial,
        ShowAbout
    };

    commands.addArray(ids, juce::numElementsInArray(ids));
}

void Commands::getCommandInfo(juce::CommandID commandID,
                            juce::ApplicationCommandInfo& result) {
    if (commandID >= 0x1000 && commandID < 0x2000)
        addFileCommandInfo(commandID, result);
    else if (commandID >= 0x2000 && commandID < 0x3000)
        addEditCommandInfo(commandID, result);
    else if (commandID >= 0x3000 && commandID < 0x4000)
        addTrackCommandInfo(commandID, result);
    else if (commandID >= 0x4000 && commandID < 0x5000)
        addTransportCommandInfo(commandID, result);
    else if (commandID >= 0x5000 && commandID < 0x6000)
        addViewCommandInfo(commandID, result);
    else if (commandID >= 0x6000 && commandID < 0x7000)
        addPluginCommandInfo(commandID, result);
    else if (commandID >= 0x7000 && commandID < 0x8000)
        addToolCommandInfo(commandID, result);
    else if (commandID >= 0x8000 && commandID < 0x9000)
        addMIDICommandInfo(commandID, result);
    else if (commandID >= 0x9000 && commandID < 0xA000)
        addAudioCommandInfo(commandID, result);
    else if (commandID >= 0xA000 && commandID < 0xB000)
        addSettingsCommandInfo(commandID, result);
    else if (commandID >= 0xB000 && commandID < 0xC000)
        addHelpCommandInfo(commandID, result);
}

bool Commands::perform(const juce::ApplicationCommandTarget::InvocationInfo& info) {
    if (info.commandID >= 0x1000 && info.commandID < 0x2000)
        handleFileCommand(info.commandID);
    else if (info.commandID >= 0x2000 && info.commandID < 0x3000)
        handleEditCommand(info.commandID);
    else if (info.commandID >= 0x3000 && info.commandID < 0x4000)
        handleTrackCommand(info.commandID);
    else if (info.commandID >= 0x4000 && info.commandID < 0x5000)
        handleTransportCommand(info.commandID);
    else if (info.commandID >= 0x5000 && info.commandID < 0x6000)
        handleViewCommand(info.commandID);
    else if (info.commandID >= 0x6000 && info.commandID < 0x7000)
        handlePluginCommand(info.commandID);
    else if (info.commandID >= 0x7000 && info.commandID < 0x8000)
        handleToolCommand(info.commandID);
    else if (info.commandID >= 0x8000 && info.commandID < 0x9000)
        handleMIDICommand(info.commandID);
    else if (info.commandID >= 0x9000 && info.commandID < 0xA000)
        handleAudioCommand(info.commandID);
    else if (info.commandID >= 0xA000 && info.commandID < 0xB000)
        handleSettingsCommand(info.commandID);
    else if (info.commandID >= 0xB000 && info.commandID < 0xC000)
        handleHelpCommand(info.commandID);
    else
        return false;

    return true;
}

void Commands::addFileCommandInfo(juce::CommandID commandID,
                                juce::ApplicationCommandInfo& result) {
    switch (commandID) {
        case NewProject:
            result.setInfo("New Project", "Create a new project", "File", 0);
            result.addDefaultKeypress('n', juce::ModifierKeys::commandModifier);
            break;
            
        case OpenProject:
            result.setInfo("Open Project...", "Open an existing project", "File", 0);
            result.addDefaultKeypress('o', juce::ModifierKeys::commandModifier);
            break;
            
        case SaveProject:
            result.setInfo("Save Project", "Save the current project", "File", 0);
            result.addDefaultKeypress('s', juce::ModifierKeys::commandModifier);
            if (project == nullptr || !project->hasUnsavedChanges())
                result.setActive(false);
            break;
            
        case SaveProjectAs:
            result.setInfo("Save Project As...", "Save the project with a new name", "File", 0);
            result.addDefaultKeypress('s', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
            if (project == nullptr)
                result.setActive(false);
            break;
            
        case ExportAudio:
            result.setInfo("Export Audio...", "Export project as audio file", "File", 0);
            result.addDefaultKeypress('e', juce::ModifierKeys::commandModifier);
            if (project == nullptr)
                result.setActive(false);
            break;
            
        case ExportMIDI:
            result.setInfo("Export MIDI...", "Export project as MIDI file", "File", 0);
            result.addDefaultKeypress('e', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
            if (project == nullptr)
                result.setActive(false);
            break;
            
        case ImportAudio:
            result.setInfo("Import Audio...", "Import audio file", "File", 0);
            result.addDefaultKeypress('i', juce::ModifierKeys::commandModifier);
            if (project == nullptr)
                result.setActive(false);
            break;
            
        case ImportMIDI:
            result.setInfo("Import MIDI...", "Import MIDI file", "File", 0);
            result.addDefaultKeypress('i', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
            if (project == nullptr)
                result.setActive(false);
            break;
    }
}

void Commands::addEditCommandInfo(juce::CommandID commandID,
                                juce::ApplicationCommandInfo& result) {
    switch (commandID) {
        case Undo:
            result.setInfo("Undo", "Undo the last action", "Edit", 0);
            result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier);
            if (!canUndo())
                result.setActive(false);
            break;
            
        case Redo:
            result.setInfo("Redo", "Redo the last undone action", "Edit", 0);
            result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
            if (!canRedo())
                result.setActive(false);
            break;
            
        case Cut:
            result.setInfo("Cut", "Cut the selected items", "Edit", 0);
            result.addDefaultKeypress('x', juce::ModifierKeys::commandModifier);
            if (!hasSelection())
                result.setActive(false);
            break;
            
        case Copy:
            result.setInfo("Copy", "Copy the selected items", "Edit", 0);
            result.addDefaultKeypress('c', juce::ModifierKeys::commandModifier);
            if (!hasSelection())
                result.setActive(false);
            break;
            
        case Paste:
            result.setInfo("Paste", "Paste the clipboard contents", "Edit", 0);
            result.addDefaultKeypress('v', juce::ModifierKeys::commandModifier);
            if (!hasClipboard())
                result.setActive(false);
            break;
            
        case Delete:
            result.setInfo("Delete", "Delete the selected items", "Edit", 0);
            result.addDefaultKeypress(juce::KeyPress::deleteKey, 0);
            if (!hasSelection())
                result.setActive(false);
            break;
            
        case SelectAll:
            result.setInfo("Select All", "Select all items", "Edit", 0);
            result.addDefaultKeypress('a', juce::ModifierKeys::commandModifier);
            break;
            
        case SelectNone:
            result.setInfo("Select None", "Deselect all items", "Edit", 0);
            result.addDefaultKeypress('d', juce::ModifierKeys::commandModifier);
            if (!hasSelection())
                result.setActive(false);
            break;
    }
}

void Commands::addTrackCommandInfo(juce::CommandID commandID,
                                 juce::ApplicationCommandInfo& result) {
    switch (commandID) {
        case AddAudioTrack:
            result.setInfo("Add Audio Track", "Add a new audio track", "Track", 0);
            result.addDefaultKeypress('t', juce::ModifierKeys::commandModifier);
            if (project == nullptr)
                result.setActive(false);
            break;
            
        case AddMIDITrack:
            result.setInfo("Add MIDI Track", "Add a new MIDI track", "Track", 0);
            result.addDefaultKeypress('t', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
            if (project == nullptr)
                result.setActive(false);
            break;
            
        case AddBusTrack:
            result.setInfo("Add Bus Track", "Add a new bus track", "Track", 0);
            result.addDefaultKeypress('b', juce::ModifierKeys::commandModifier);
            if (project == nullptr)
                result.setActive(false);
            break;
    }
}

void Commands::addTransportCommandInfo(juce::CommandID commandID,
                                     juce::ApplicationCommandInfo& result) {
    switch (commandID) {
        case Play:
            result.setInfo("Play/Pause", "Start or pause playback", "Transport", 0);
            result.addDefaultKeypress(juce::KeyPress::spaceKey, 0);
            if (project == nullptr)
                result.setActive(false);
            break;
            
        case Stop:
            result.setInfo("Stop", "Stop playback", "Transport", 0);
            result.addDefaultKeypress('.', juce::ModifierKeys::commandModifier);
            if (!isPlaying())
                result.setActive(false);
            break;
            
        case Record:
            result.setInfo("Record", "Start/stop recording", "Transport", 0);
            result.addDefaultKeypress('r', juce::ModifierKeys::commandModifier);
            if (project == nullptr)
                result.setActive(false);
            break;
            
        case ToggleLoop:
            result.setInfo("Toggle Loop", "Toggle loop mode", "Transport", 0);
            result.addDefaultKeypress('l', juce::ModifierKeys::commandModifier);
            if (project == nullptr)
                result.setActive(false);
            result.setTicked(isLooping());
            break;
    }
}

void Commands::addViewCommandInfo(juce::CommandID commandID,
                                juce::ApplicationCommandInfo& result) {
    switch (commandID) {
        case ToggleMixer:
            result.setInfo("Show/Hide Mixer", "Toggle mixer visibility", "View", 0);
            result.addDefaultKeypress('m', juce::ModifierKeys::commandModifier);
            break;
            
        case TogglePianoRoll:
            result.setInfo("Show/Hide Piano Roll", "Toggle piano roll visibility", "View", 0);
            result.addDefaultKeypress('p', juce::ModifierKeys::commandModifier);
            break;
            
        case ZoomIn:
            result.setInfo("Zoom In", "Increase zoom level", "View", 0);
            result.addDefaultKeypress('=', juce::ModifierKeys::commandModifier);
            break;
            
        case ZoomOut:
            result.setInfo("Zoom Out", "Decrease zoom level", "View", 0);
            result.addDefaultKeypress('-', juce::ModifierKeys::commandModifier);
            break;
    }
}

void Commands::addPluginCommandInfo(juce::CommandID commandID,
                                  juce::ApplicationCommandInfo& result) {
    // TODO: Implement plugin command info
}

void Commands::addToolCommandInfo(juce::CommandID commandID,
                                juce::ApplicationCommandInfo& result) {
    // TODO: Implement tool command info
}

void Commands::addMIDICommandInfo(juce::CommandID commandID,
                                juce::ApplicationCommandInfo& result) {
    // TODO: Implement MIDI command info
}

void Commands::addAudioCommandInfo(juce::CommandID commandID,
                                 juce::ApplicationCommandInfo& result) {
    // TODO: Implement audio command info
}

void Commands::addSettingsCommandInfo(juce::CommandID commandID,
                                    juce::ApplicationCommandInfo& result) {
    // TODO: Implement settings command info
}

void Commands::addHelpCommandInfo(juce::CommandID commandID,
                                juce::ApplicationCommandInfo& result) {
    // TODO: Implement help command info
}

void Commands::handleFileCommand(juce::CommandID commandID) {
    // TODO: Implement file command handling
}

void Commands::handleEditCommand(juce::CommandID commandID) {
    // TODO: Implement edit command handling
}

void Commands::handleTrackCommand(juce::CommandID commandID) {
    // TODO: Implement track command handling
}

void Commands::handleTransportCommand(juce::CommandID commandID) {
    // TODO: Implement transport command handling
}

void Commands::handleViewCommand(juce::CommandID commandID) {
    // TODO: Implement view command handling
}

void Commands::handlePluginCommand(juce::CommandID commandID) {
    // TODO: Implement plugin command handling
}

void Commands::handleToolCommand(juce::CommandID commandID) {
    // TODO: Implement tool command handling
}

void Commands::handleMIDICommand(juce::CommandID commandID) {
    // TODO: Implement MIDI command handling
}

void Commands::handleAudioCommand(juce::CommandID commandID) {
    // TODO: Implement audio command handling
}

void Commands::handleSettingsCommand(juce::CommandID commandID) {
    // TODO: Implement settings command handling
}

void Commands::handleHelpCommand(juce::CommandID commandID) {
    // TODO: Implement help command handling
}