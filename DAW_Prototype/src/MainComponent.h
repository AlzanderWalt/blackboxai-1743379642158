#pragma once
#include <JuceHeader.h>
#include "Project.h"
#include "MixerComponent.h"
#include "TrackEditorComponent.h"
#include "PianoRollComponent.h"

class MainComponent : public juce::Component,
                     public juce::ChangeListener {
public:
    // Transport control component
    class TransportComponent : public juce::Component {
    public:
        TransportComponent();
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        
        void updateFromTransport();
        
    private:
        juce::TextButton playButton;
        juce::TextButton stopButton;
        juce::TextButton recordButton;
        juce::TextButton loopButton;
        juce::Label timeDisplay;
        juce::Label tempoDisplay;
        juce::Slider tempoSlider;
        
        void setupControls();
        void handlePlayClick();
        void handleStopClick();
        void handleRecordClick();
        void handleLoopClick();
        void handleTempoChange();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TransportComponent)
    };

    // Tool bar component
    class ToolBarComponent : public juce::Component {
    public:
        ToolBarComponent();
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        
    private:
        juce::TextButton newTrackButton;
        juce::TextButton deleteButton;
        juce::TextButton splitButton;
        juce::TextButton mergeButton;
        juce::TextButton quantizeButton;
        juce::ComboBox toolSelector;
        
        void setupControls();
        void handleNewTrackClick();
        void handleDeleteClick();
        void handleSplitClick();
        void handleMergeClick();
        void handleQuantizeClick();
        void handleToolChange();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ToolBarComponent)
    };

    // Constructor/Destructor
    MainComponent();
    ~MainComponent() override;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ChangeListener interface
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // Project handling
    void setProject(std::unique_ptr<Project> newProject);
    void createNewProject();
    void loadProject(const juce::File& file);
    void saveProject(const juce::File& file);
    Project* getProject() const { return currentProject.get(); }

    // View management
    void showMixer(bool show);
    void showPianoRoll(bool show);
    void updateViews();

private:
    std::unique_ptr<Project> currentProject;
    
    std::unique_ptr<TransportComponent> transport;
    std::unique_ptr<ToolBarComponent> toolbar;
    std::unique_ptr<MixerComponent> mixer;
    std::unique_ptr<TrackEditorComponent> trackEditor;
    std::unique_ptr<PianoRollComponent> pianoRoll;
    
    juce::StretchableLayoutManager verticalLayout;
    juce::StretchableLayoutManager horizontalLayout;
    
    bool mixerVisible{true};
    bool pianoRollVisible{false};
    
    void setupLayout();
    void updateLayout();
    
    static constexpr int transportHeight = 40;
    static constexpr int toolbarHeight = 40;
    static constexpr int mixerHeight = 200;
    static constexpr int pianoRollHeight = 300;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainComponent)
};