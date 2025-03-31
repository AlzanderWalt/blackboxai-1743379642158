#include "MainComponent.h"
#include "Logger.h"
#include "CustomLookAndFeel.h"

//==============================================================================
// TransportComponent Implementation
//==============================================================================

MainComponent::TransportComponent::TransportComponent() {
    setupControls();
}

void MainComponent::TransportComponent::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    g.fillAll(lf.getTransportBackground());
}

void MainComponent::TransportComponent::resized() {
    juce::Grid grid;
    grid.setGap(juce::Grid::Px(4));
    
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = {
        Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),  // Buttons
        Track(Fr(2)),                                            // Time display
        Track(Fr(1)), Track(Fr(2))                              // Tempo controls
    };
    
    grid.templateRows = { Track(Fr(1)) };
    
    juce::Array<juce::GridItem> items;
    items.add(playButton);
    items.add(stopButton);
    items.add(recordButton);
    items.add(loopButton);
    items.add(timeDisplay);
    items.add(tempoDisplay);
    items.add(tempoSlider);
    
    grid.items = items;
    grid.performLayout(getLocalBounds());
}

void MainComponent::TransportComponent::updateFromTransport() {
    // TODO: Update transport controls from engine state
}

void MainComponent::TransportComponent::setupControls() {
    // Play button
    addAndMakeVisible(playButton);
    playButton.setButtonText("Play");
    playButton.onClick = [this] { handlePlayClick(); };
    
    // Stop button
    addAndMakeVisible(stopButton);
    stopButton.setButtonText("Stop");
    stopButton.onClick = [this] { handleStopClick(); };
    
    // Record button
    addAndMakeVisible(recordButton);
    recordButton.setButtonText("Record");
    recordButton.setClickingTogglesState(true);
    recordButton.onClick = [this] { handleRecordClick(); };
    
    // Loop button
    addAndMakeVisible(loopButton);
    loopButton.setButtonText("Loop");
    loopButton.setClickingTogglesState(true);
    loopButton.onClick = [this] { handleLoopClick(); };
    
    // Time display
    addAndMakeVisible(timeDisplay);
    timeDisplay.setJustificationType(juce::Justification::centred);
    timeDisplay.setText("00:00:00.000", juce::dontSendNotification);
    
    // Tempo display
    addAndMakeVisible(tempoDisplay);
    tempoDisplay.setJustificationType(juce::Justification::centred);
    tempoDisplay.setText("120 BPM", juce::dontSendNotification);
    
    // Tempo slider
    addAndMakeVisible(tempoSlider);
    tempoSlider.setRange(20.0, 300.0, 0.1);
    tempoSlider.setValue(120.0);
    tempoSlider.onValueChange = [this] { handleTempoChange(); };
}

void MainComponent::TransportComponent::handlePlayClick() {
    // TODO: Handle play button click
}

void MainComponent::TransportComponent::handleStopClick() {
    // TODO: Handle stop button click
}

void MainComponent::TransportComponent::handleRecordClick() {
    // TODO: Handle record button click
}

void MainComponent::TransportComponent::handleLoopClick() {
    // TODO: Handle loop button click
}

void MainComponent::TransportComponent::handleTempoChange() {
    // TODO: Handle tempo change
}

//==============================================================================
// ToolBarComponent Implementation
//==============================================================================

MainComponent::ToolBarComponent::ToolBarComponent() {
    setupControls();
}

void MainComponent::ToolBarComponent::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    g.fillAll(lf.getToolBarBackground());
}

void MainComponent::ToolBarComponent::resized() {
    juce::Grid grid;
    grid.setGap(juce::Grid::Px(4));
    
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateColumns = {
        Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1)),  // Buttons
        Track(Fr(2))                                                           // Tool selector
    };
    
    grid.templateRows = { Track(Fr(1)) };
    
    juce::Array<juce::GridItem> items;
    items.add(newTrackButton);
    items.add(deleteButton);
    items.add(splitButton);
    items.add(mergeButton);
    items.add(quantizeButton);
    items.add(toolSelector);
    
    grid.items = items;
    grid.performLayout(getLocalBounds());
}

void MainComponent::ToolBarComponent::setupControls() {
    // New track button
    addAndMakeVisible(newTrackButton);
    newTrackButton.setButtonText("New Track");
    newTrackButton.onClick = [this] { handleNewTrackClick(); };
    
    // Delete button
    addAndMakeVisible(deleteButton);
    deleteButton.setButtonText("Delete");
    deleteButton.onClick = [this] { handleDeleteClick(); };
    
    // Split button
    addAndMakeVisible(splitButton);
    splitButton.setButtonText("Split");
    splitButton.onClick = [this] { handleSplitClick(); };
    
    // Merge button
    addAndMakeVisible(mergeButton);
    mergeButton.setButtonText("Merge");
    mergeButton.onClick = [this] { handleMergeClick(); };
    
    // Quantize button
    addAndMakeVisible(quantizeButton);
    quantizeButton.setButtonText("Quantize");
    quantizeButton.onClick = [this] { handleQuantizeClick(); };
    
    // Tool selector
    addAndMakeVisible(toolSelector);
    toolSelector.addItem("Select", 1);
    toolSelector.addItem("Draw", 2);
    toolSelector.addItem("Erase", 3);
    toolSelector.setSelectedId(1);
    toolSelector.onChange = [this] { handleToolChange(); };
}

void MainComponent::ToolBarComponent::handleNewTrackClick() {
    // TODO: Handle new track button click
}

void MainComponent::ToolBarComponent::handleDeleteClick() {
    // TODO: Handle delete button click
}

void MainComponent::ToolBarComponent::handleSplitClick() {
    // TODO: Handle split button click
}

void MainComponent::ToolBarComponent::handleMergeClick() {
    // TODO: Handle merge button click
}

void MainComponent::ToolBarComponent::handleQuantizeClick() {
    // TODO: Handle quantize button click
}

void MainComponent::ToolBarComponent::handleToolChange() {
    // TODO: Handle tool selection change
}

//==============================================================================
// MainComponent Implementation
//==============================================================================

MainComponent::MainComponent() {
    transport = std::make_unique<TransportComponent>();
    toolbar = std::make_unique<ToolBarComponent>();
    mixer = std::make_unique<MixerComponent>();
    trackEditor = std::make_unique<TrackEditorComponent>();
    pianoRoll = std::make_unique<PianoRollComponent>();
    
    addAndMakeVisible(transport.get());
    addAndMakeVisible(toolbar.get());
    addAndMakeVisible(mixer.get());
    addAndMakeVisible(trackEditor.get());
    addAndMakeVisible(pianoRoll.get());
    
    setupLayout();
    createNewProject();
}

MainComponent::~MainComponent() {
    if (currentProject != nullptr) {
        currentProject->removeChangeListener(this);
    }
}

void MainComponent::paint(juce::Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MainComponent::resized() {
    updateLayout();
}

void MainComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == currentProject.get()) {
        updateViews();
    }
}

void MainComponent::setProject(std::unique_ptr<Project> newProject) {
    if (currentProject != nullptr) {
        currentProject->removeChangeListener(this);
    }
    
    currentProject = std::move(newProject);
    
    if (currentProject != nullptr) {
        currentProject->addChangeListener(this);
        mixer->setProject(currentProject.get());
        trackEditor->setProject(currentProject.get());
        pianoRoll->setClip(nullptr);  // Clear piano roll
    }
    
    updateViews();
}

void MainComponent::createNewProject() {
    auto project = std::make_unique<Project>();
    // TODO: Initialize project with default settings
    setProject(std::move(project));
}

void MainComponent::loadProject(const juce::File& file) {
    // TODO: Implement project loading
}

void MainComponent::saveProject(const juce::File& file) {
    // TODO: Implement project saving
}

void MainComponent::showMixer(bool show) {
    if (mixerVisible != show) {
        mixerVisible = show;
        updateLayout();
    }
}

void MainComponent::showPianoRoll(bool show) {
    if (pianoRollVisible != show) {
        pianoRollVisible = show;
        updateLayout();
    }
}

void MainComponent::updateViews() {
    mixer->updateChannelStrips();
    trackEditor->updateTrackViews();
    transport->updateFromTransport();
}

void MainComponent::setupLayout() {
    // Set up vertical layout
    verticalLayout.setItemLayout(0, transportHeight, transportHeight, transportHeight);  // Transport
    verticalLayout.setItemLayout(1, toolbarHeight, toolbarHeight, toolbarHeight);       // Toolbar
    verticalLayout.setItemLayout(2, 200, -1.0, -1.0);                                   // Main area
    verticalLayout.setItemLayout(3, mixerHeight, mixerHeight, mixerHeight);             // Mixer
    verticalLayout.setItemLayout(4, pianoRollHeight, pianoRollHeight, pianoRollHeight); // Piano roll
    
    // Set up horizontal layout for main area
    horizontalLayout.setItemLayout(0, 200, -1.0, -1.0);  // Track editor
}

void MainComponent::updateLayout() {
    auto bounds = getLocalBounds();
    
    // Layout transport and toolbar
    transport->setBounds(bounds.removeFromTop(transportHeight));
    toolbar->setBounds(bounds.removeFromTop(toolbarHeight));
    
    // Layout mixer if visible
    if (mixerVisible) {
        mixer->setBounds(bounds.removeFromBottom(mixerHeight));
    }
    
    // Layout piano roll if visible
    if (pianoRollVisible) {
        pianoRoll->setBounds(bounds.removeFromBottom(pianoRollHeight));
    }
    
    // Layout track editor in remaining space
    trackEditor->setBounds(bounds);
}