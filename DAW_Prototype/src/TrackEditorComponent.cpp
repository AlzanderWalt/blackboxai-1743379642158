#include "TrackEditorComponent.h"
#include "Project.h"
#include "Track.h"
#include "Clip.h"
#include "Logger.h"
#include "CustomLookAndFeel.h"

//==============================================================================
// TrackHeader Implementation
//==============================================================================

TrackEditorComponent::TrackHeader::TrackHeader(TrackEditorComponent& owner, Track& track)
    : owner(owner)
    , track(track) {
    setupControls();
    updateFromTrack();
}

TrackEditorComponent::TrackHeader::~TrackHeader() {
    track.removeChangeListener(this);
}

void TrackEditorComponent::TrackHeader::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    
    // Draw background
    g.fillAll(lf.getTrackHeaderBackground());
    
    // Draw border
    g.setColour(lf.getTrackHeaderBorder());
    g.drawRect(getLocalBounds());
}

void TrackEditorComponent::TrackHeader::resized() {
    juce::Grid grid;
    grid.setGap(juce::Grid::Px(4));
    
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateRows = {
        Track(Fr(1)),    // Name
        Track(Fr(1)),    // Buttons
        Track(Fr(1))     // Height
    };
    
    grid.templateColumns = { Track(Fr(1)) };
    
    juce::Array<juce::GridItem> items;
    items.add(juce::GridItem(nameLabel));
    
    // Buttons panel
    juce::Grid buttonGrid;
    buttonGrid.setGap(juce::Grid::Px(2));
    buttonGrid.templateColumns = {
        Track(Fr(1)), Track(Fr(1)), Track(Fr(1)), Track(Fr(1))
    };
    buttonGrid.templateRows = { Track(Fr(1)) };
    
    juce::Array<juce::GridItem> buttonItems;
    buttonItems.add(muteButton);
    buttonItems.add(soloButton);
    buttonItems.add(recordButton);
    buttonItems.add(automationButton);
    buttonGrid.items = buttonItems;
    
    items.add(juce::GridItem(buttonGrid));
    items.add(juce::GridItem(heightSlider));
    
    grid.items = items;
    grid.performLayout(getLocalBounds());
}

void TrackEditorComponent::TrackHeader::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &track) {
        updateFromTrack();
    }
}

void TrackEditorComponent::TrackHeader::updateFromTrack() {
    nameLabel.setText(track.getName(), juce::dontSendNotification);
    
    const auto& params = track.getParameters();
    muteButton.setToggleState(params.mute, juce::dontSendNotification);
    soloButton.setToggleState(params.solo, juce::dontSendNotification);
    recordButton.setToggleState(params.record, juce::dontSendNotification);
    heightSlider.setValue(params.height, juce::dontSendNotification);
}

void TrackEditorComponent::TrackHeader::setupControls() {
    // Name label
    addAndMakeVisible(nameLabel);
    nameLabel.setEditable(true);
    nameLabel.onTextChange = [this] { handleNameChange(); };
    
    // Buttons
    addAndMakeVisible(muteButton);
    muteButton.setButtonText("M");
    muteButton.setClickingTogglesState(true);
    muteButton.onClick = [this] { handleMuteClick(); };
    
    addAndMakeVisible(soloButton);
    soloButton.setButtonText("S");
    soloButton.setClickingTogglesState(true);
    soloButton.onClick = [this] { handleSoloClick(); };
    
    addAndMakeVisible(recordButton);
    recordButton.setButtonText("R");
    recordButton.setClickingTogglesState(true);
    recordButton.onClick = [this] { handleRecordClick(); };
    
    addAndMakeVisible(automationButton);
    automationButton.setButtonText("A");
    automationButton.setClickingTogglesState(true);
    automationButton.onClick = [this] { handleAutomationClick(); };
    
    // Height slider
    addAndMakeVisible(heightSlider);
    heightSlider.setSliderStyle(juce::Slider::LinearHorizontal);
    heightSlider.setRange(60, 300, 1);
    heightSlider.setValue(100);
    heightSlider.onValueChange = [this] { handleHeightChange(); };
    
    track.addChangeListener(this);
}

void TrackEditorComponent::TrackHeader::handleNameChange() {
    track.setName(nameLabel.getText());
}

void TrackEditorComponent::TrackHeader::handleMuteClick() {
    auto params = track.getParameters();
    params.mute = muteButton.getToggleState();
    track.setParameters(params);
}

void TrackEditorComponent::TrackHeader::handleSoloClick() {
    auto params = track.getParameters();
    params.solo = soloButton.getToggleState();
    track.setParameters(params);
}

void TrackEditorComponent::TrackHeader::handleRecordClick() {
    auto params = track.getParameters();
    params.record = recordButton.getToggleState();
    track.setParameters(params);
}

void TrackEditorComponent::TrackHeader::handleAutomationClick() {
    // TODO: Show automation lanes
}

void TrackEditorComponent::TrackHeader::handleHeightChange() {
    auto params = track.getParameters();
    params.height = static_cast<int>(heightSlider.getValue());
    track.setParameters(params);
}

//==============================================================================
// TrackContent Implementation
//==============================================================================

TrackEditorComponent::TrackContent::TrackContent(TrackEditorComponent& owner, Track& track)
    : owner(owner)
    , track(track) {
    setWantsKeyboardFocus(true);
    createClipViews();
    track.addChangeListener(this);
}

TrackEditorComponent::TrackContent::~TrackContent() {
    track.removeChangeListener(this);
}

void TrackEditorComponent::TrackContent::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    
    // Draw background
    g.fillAll(lf.getTrackContentBackground());
    
    // Draw grid
    g.setColour(lf.getTrackContentGrid());
    const double gridInterval = 1.0;  // 1 second
    for (double t = timeStart; t <= timeEnd; t += gridInterval) {
        const int x = static_cast<int>(timeToX(t));
        g.drawVerticalLine(x, 0.0f, static_cast<float>(getHeight()));
    }
}

void TrackEditorComponent::TrackContent::resized() {
    updateClipPositions();
}

void TrackEditorComponent::TrackContent::mouseDown(const juce::MouseEvent& e) {
    if (e.mods.isLeftButtonDown()) {
        draggedClipIndex = findClipAt(e.getPosition());
        if (draggedClipIndex >= 0) {
            dragging = true;
            dragStartTime = xToTime(e.x);
            selectClip(draggedClipIndex, !e.mods.isShiftDown());
        } else {
            owner.selectTrack(&track, !e.mods.isShiftDown());
        }
    }
}

void TrackEditorComponent::TrackContent::mouseDrag(const juce::MouseEvent& e) {
    if (dragging && draggedClipIndex >= 0) {
        const double dragTime = xToTime(e.x);
        const double timeDelta = dragTime - dragStartTime;
        
        // TODO: Update clip position
        updateClipPositions();
    }
}

void TrackEditorComponent::TrackContent::mouseUp(const juce::MouseEvent& e) {
    dragging = false;
    draggedClipIndex = -1;
}

void TrackEditorComponent::TrackContent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == &track) {
        updateFromTrack();
    }
}

bool TrackEditorComponent::TrackContent::isInterestedInDragSource(const SourceDetails& source) {
    // TODO: Implement drag and drop handling
    return false;
}

void TrackEditorComponent::TrackContent::itemDragEnter(const SourceDetails& source) {
    // TODO: Implement drag and drop handling
}

void TrackEditorComponent::TrackContent::itemDragMove(const SourceDetails& source) {
    // TODO: Implement drag and drop handling
}

void TrackEditorComponent::TrackContent::itemDragExit(const SourceDetails& source) {
    // TODO: Implement drag and drop handling
}

void TrackEditorComponent::TrackContent::itemDropped(const SourceDetails& source) {
    // TODO: Implement drag and drop handling
}

void TrackEditorComponent::TrackContent::updateFromTrack() {
    createClipViews();
    updateClipPositions();
    repaint();
}

void TrackEditorComponent::TrackContent::setTimeRange(double start, double end) {
    timeStart = start;
    timeEnd = end;
    pixelsPerSecond = getWidth() / (timeEnd - timeStart);
    updateClipPositions();
    repaint();
}

void TrackEditorComponent::TrackContent::createClipViews() {
    clipViews.clear();
    
    // TODO: Create clip views based on track type
}

void TrackEditorComponent::TrackContent::updateClipPositions() {
    for (auto& clipView : clipViews) {
        const int x = static_cast<int>(timeToX(clipView.startTime));
        const int width = static_cast<int>(timeToX(clipView.endTime) - x);
        clipView.component->setBounds(x, 0, width, getHeight());
    }
}

double TrackEditorComponent::TrackContent::timeToX(double time) const {
    return (time - timeStart) * pixelsPerSecond;
}

double TrackEditorComponent::TrackContent::xToTime(int x) const {
    return timeStart + x / pixelsPerSecond;
}

int TrackEditorComponent::TrackContent::findClipAt(juce::Point<int> position) const {
    for (int i = static_cast<int>(clipViews.size()) - 1; i >= 0; --i) {
        if (clipViews[i].component->getBounds().contains(position)) {
            return i;
        }
    }
    return -1;
}

void TrackEditorComponent::TrackContent::selectClip(int index, bool deselectOthers) {
    if (deselectOthers) {
        for (auto& clipView : clipViews) {
            clipView.selected = false;
        }
    }
    
    if (index >= 0 && index < clipViews.size()) {
        clipViews[index].selected = true;
    }
    
    repaint();
}

void TrackEditorComponent::TrackContent::deleteSelectedClips() {
    // TODO: Implement clip deletion
}

//==============================================================================
// TrackEditorComponent Implementation
//==============================================================================

TrackEditorComponent::TrackEditorComponent() {
    addAndMakeVisible(headerViewport);
    addAndMakeVisible(contentViewport);
    
    headerViewport.setViewedComponent(&headerContainer, false);
    contentViewport.setViewedComponent(&contentContainer, false);
    
    // Synchronize vertical scrolling
    headerViewport.getVerticalScrollBar().addListener(this);
    contentViewport.getVerticalScrollBar().addListener(this);
}

TrackEditorComponent::~TrackEditorComponent() {
    if (currentProject != nullptr) {
        currentProject->removeChangeListener(this);
    }
}

void TrackEditorComponent::paint(juce::Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void TrackEditorComponent::resized() {
    auto bounds = getLocalBounds();
    
    // Layout viewports
    headerViewport.setBounds(bounds.removeFromLeft(headerWidth));
    contentViewport.setBounds(bounds);
    
    updateLayout();
}

void TrackEditorComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == currentProject) {
        updateTrackViews();
    }
}

void TrackEditorComponent::setProject(Project* project) {
    if (currentProject == project) {
        return;
    }

    if (currentProject != nullptr) {
        currentProject->removeChangeListener(this);
    }
    
    currentProject = project;
    
    if (currentProject != nullptr) {
        currentProject->addChangeListener(this);
    }
    
    updateTrackViews();
}

void TrackEditorComponent::setTimeRange(double start, double end) {
    timeStart = start;
    timeEnd = end;
    updateTimeRange();
}

void TrackEditorComponent::setVisibleRange(double start, double end) {
    visibleTimeStart = start;
    visibleTimeEnd = end;
    updateTimeRange();
}

void TrackEditorComponent::addTrack(Track::Type type) {
    if (currentProject != nullptr) {
        // TODO: Add track to project
        updateTrackViews();
    }
}

void TrackEditorComponent::removeTrack(int index) {
    if (currentProject != nullptr) {
        // TODO: Remove track from project
        updateTrackViews();
    }
}

void TrackEditorComponent::moveTrack(int fromIndex, int toIndex) {
    if (currentProject != nullptr) {
        // TODO: Move track in project
        updateTrackViews();
    }
}

void TrackEditorComponent::selectTrack(int index, bool deselectOthers) {
    if (deselectOthers) {
        for (auto& trackView : trackViews) {
            trackView.selected = false;
        }
    }
    
    if (index >= 0 && index < trackViews.size()) {
        trackViews[index].selected = true;
    }
    
    repaint();
}

void TrackEditorComponent::selectClip(Track* track, int clipIndex, bool deselectOthers) {
    // TODO: Implement clip selection
}

void TrackEditorComponent::clearSelection() {
    for (auto& trackView : trackViews) {
        trackView.selected = false;
        // TODO: Clear clip selection
    }
    repaint();
}

void TrackEditorComponent::deleteSelected() {
    // TODO: Implement deletion of selected tracks and clips
}

void TrackEditorComponent::splitSelectedClips() {
    // TODO: Implement clip splitting
}

void TrackEditorComponent::mergeSelectedClips() {
    // TODO: Implement clip merging
}

void TrackEditorComponent::duplicateSelectedClips() {
    // TODO: Implement clip duplication
}

void TrackEditorComponent::showAutomation(const juce::String& paramId) {
    // TODO: Show automation lanes
}

void TrackEditorComponent::hideAutomation() {
    // TODO: Hide automation lanes
}

void TrackEditorComponent::updateTrackViews() {
    if (currentProject == nullptr) {
        trackViews.clear();
        updateLayout();
        return;
    }

    const auto& tracks = currentProject->getTracks();
    
    // Add/remove track views
    while (trackViews.size() < tracks.size()) {
        TrackView view;
        view.header = std::make_unique<TrackHeader>(*this, *tracks[trackViews.size()]);
        view.content = std::make_unique<TrackContent>(*this, *tracks[trackViews.size()]);
        view.selected = false;
        
        headerContainer.addAndMakeVisible(view.header.get());
        contentContainer.addAndMakeVisible(view.content.get());
        
        trackViews.push_back(std::move(view));
    }
    
    while (trackViews.size() > tracks.size()) {
        trackViews.pop_back();
    }
    
    updateLayout();
}

void TrackEditorComponent::updateTimeRange() {
    for (auto& trackView : trackViews) {
        trackView.content->setTimeRange(timeStart, timeEnd);
    }
}

void TrackEditorComponent::updateLayout() {
    int y = 0;
    
    for (auto& trackView : trackViews) {
        const int height = trackView.header->getHeight();
        trackView.header->setBounds(0, y, headerWidth, height);
        trackView.content->setBounds(0, y, contentContainer.getWidth(), height);
        y += height;
    }
    
    headerContainer.setSize(headerWidth, y);
    contentContainer.setSize(
        static_cast<int>((timeEnd - timeStart) * pixelsPerSecond), y);
}