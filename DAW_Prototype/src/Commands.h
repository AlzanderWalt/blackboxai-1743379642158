#pragma once
#include <JuceHeader.h>

class Project;

class Commands : public juce::ApplicationCommandTarget {
public:
    // Command IDs
    enum CommandIDs {
        // File commands
        NewProject = 0x1000,
        OpenProject,
        SaveProject,
        SaveProjectAs,
        ExportAudio,
        ExportMIDI,
        ImportAudio,
        ImportMIDI,
        
        // Edit commands
        Undo = 0x2000,
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
        
        // Track commands
        AddAudioTrack = 0x3000,
        AddMIDITrack,
        AddBusTrack,
        DeleteSelectedTracks,
        DuplicateSelectedTracks,
        MuteSelectedTracks,
        SoloSelectedTracks,
        ArmSelectedTracks,
        FreezeSelectedTracks,
        UnfreezeSelectedTracks,
        
        // Transport commands
        Play = 0x4000,
        Stop,
        Record,
        FastForward,
        Rewind,
        ToggleLoop,
        ToggleMetronome,
        ToggleCountIn,
        SetTempo,
        SetTimeSignature,
        
        // View commands
        ToggleMixer = 0x5000,
        TogglePianoRoll,
        ToggleAutomation,
        ToggleGrid,
        ZoomIn,
        ZoomOut,
        ZoomToSelection,
        ZoomToFit,
        
        // Plugin commands
        ShowPluginManager = 0x6000,
        ShowPluginBrowser,
        ScanForPlugins,
        EnableSelectedPlugins,
        DisableSelectedPlugins,
        
        // Tools commands
        SelectTool = 0x7000,
        DrawTool,
        EraseTool,
        SplitTool,
        GlueTool,
        FadeTool,
        
        // MIDI commands
        QuantizeSelection = 0x8000,
        TransposeSelection,
        VelocityAdjust,
        LengthAdjust,
        LegateNotes,
        ChordMode,
        ArpeggiateMode,
        
        // Audio commands
        NormalizeAudio = 0x9000,
        ReverseAudio,
        FadeIn,
        FadeOut,
        CrossFade,
        TimeStretch,
        PitchShift,
        
        // Settings commands
        ShowPreferences = 0xA000,
        ShowAudioSettings,
        ShowMIDISettings,
        ShowKeyboardShortcuts,
        
        // Help commands
        ShowHelp = 0xB000,
        ShowTutorial,
        ShowAbout
    };

    // Constructor/Destructor
    Commands();
    ~Commands() override;

    // Command handling
    void getCommandInfo(juce::CommandID commandID,
                       juce::ApplicationCommandInfo& result);
    bool perform(const juce::ApplicationCommandTarget::InvocationInfo& info);

    // Command target handling
    juce::ApplicationCommandTarget* getNextCommandTarget() override;
    void getAllCommands(juce::Array<juce::CommandID>& commands) override;
    void getCommandInfo(juce::CommandID commandID,
                       juce::ApplicationCommandInfo& result) override;
    bool perform(const juce::ApplicationCommandTarget::InvocationInfo& info) override;

    // Project handling
    void setProject(Project* newProject);
    Project* getProject() const { return project; }

    // Command state
    bool canUndo() const;
    bool canRedo() const;
    bool hasSelection() const;
    bool hasClipboard() const;
    bool isPlaying() const;
    bool isRecording() const;
    bool isLooping() const;

private:
    Project* project{nullptr};
    
    // Command implementations
    void handleFileCommand(juce::CommandID commandID);
    void handleEditCommand(juce::CommandID commandID);
    void handleTrackCommand(juce::CommandID commandID);
    void handleTransportCommand(juce::CommandID commandID);
    void handleViewCommand(juce::CommandID commandID);
    void handlePluginCommand(juce::CommandID commandID);
    void handleToolCommand(juce::CommandID commandID);
    void handleMIDICommand(juce::CommandID commandID);
    void handleAudioCommand(juce::CommandID commandID);
    void handleSettingsCommand(juce::CommandID commandID);
    void handleHelpCommand(juce::CommandID commandID);
    
    // Command info helpers
    void addFileCommandInfo(juce::CommandID commandID,
                          juce::ApplicationCommandInfo& result);
    void addEditCommandInfo(juce::CommandID commandID,
                          juce::ApplicationCommandInfo& result);
    void addTrackCommandInfo(juce::CommandID commandID,
                           juce::ApplicationCommandInfo& result);
    void addTransportCommandInfo(juce::CommandID commandID,
                               juce::ApplicationCommandInfo& result);
    void addViewCommandInfo(juce::CommandID commandID,
                          juce::ApplicationCommandInfo& result);
    void addPluginCommandInfo(juce::CommandID commandID,
                            juce::ApplicationCommandInfo& result);
    void addToolCommandInfo(juce::CommandID commandID,
                          juce::ApplicationCommandInfo& result);
    void addMIDICommandInfo(juce::CommandID commandID,
                          juce::ApplicationCommandInfo& result);
    void addAudioCommandInfo(juce::CommandID commandID,
                           juce::ApplicationCommandInfo& result);
    void addSettingsCommandInfo(juce::CommandID commandID,
                              juce::ApplicationCommandInfo& result);
    void addHelpCommandInfo(juce::CommandID commandID,
                          juce::ApplicationCommandInfo& result);
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(Commands)
};