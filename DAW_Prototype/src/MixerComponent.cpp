#include "MixerComponent.h"
#include "Project.h"
#include "Track.h"
#include "Plugin.h"
#include "Logger.h"
#include "CustomLookAndFeel.h"

//==============================================================================
// ChannelStrip Implementation
//==============================================================================

MixerComponent::ChannelStrip::ChannelStrip(MixerComponent& owner, int index)
    : owner(owner)
    , channelIndex(index) {
    setupControls();
    updateFromTrack();
}

MixerComponent::ChannelStrip::~ChannelStrip() {
    if (auto* mixer = owner.getMixer()) {
        mixer->removeChangeListener(this);
    }
}

void MixerComponent::ChannelStrip::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    
    // Draw background
    g.fillAll(lf.getChannelStripBackground());
    
    // Draw border
    g.setColour(lf.getChannelStripBorder());
    g.drawRect(getLocalBounds());
}

void MixerComponent::ChannelStrip::resized() {
    juce::Grid grid;
    grid.setGap(juce::Grid::Px(4));
    
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateRows = {
        Track(Fr(1)),    // Name
        Track(Fr(8)),    // Meter
        Track(Fr(1)),    // Pan
        Track(Fr(1)),    // Buttons
        Track(Fr(12))    // Fader
    };
    
    grid.templateColumns = { Track(Fr(1)) };
    
    juce::Array<juce::GridItem> items;
    items.add(juce::GridItem(nameLabel));
    items.add(juce::GridItem(meter));
    items.add(juce::GridItem(pan));
    
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
    buttonItems.add(editButton);
    buttonGrid.items = buttonItems;
    
    items.add(juce::GridItem(buttonGrid));
    items.add(juce::GridItem(fader));
    
    grid.items = items;
    grid.performLayout(getLocalBounds());
}

void MixerComponent::ChannelStrip::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == owner.getMixer()) {
        updateFromTrack();
    }
}

void MixerComponent::ChannelStrip::updateFromTrack() {
    if (auto* mixer = owner.getMixer()) {
        const auto& channel = mixer->getChannel(channelIndex);
        
        fader.setValue(channel.volume, juce::dontSendNotification);
        pan.setValue(channel.pan, juce::dontSendNotification);
        muteButton.setToggleState(channel.mute, juce::dontSendNotification);
        soloButton.setToggleState(channel.solo, juce::dontSendNotification);
        
        if (auto* project = owner.getProject()) {
            const auto& tracks = project->getTracks();
            if (channelIndex < tracks.size()) {
                nameLabel.setText(tracks[channelIndex]->getName(),
                                juce::dontSendNotification);
                recordButton.setToggleState(tracks[channelIndex]->getParameters().record,
                                          juce::dontSendNotification);
            }
        }
    }
}

void MixerComponent::ChannelStrip::updateMeters(float peak, float rms) {
    meter.peak = peak;
    meter.rms = rms;
    meter.repaint();
}

void MixerComponent::ChannelStrip::setupControls() {
    // Name label
    addAndMakeVisible(nameLabel);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(juce::Font(12.0f));
    
    // Fader
    addAndMakeVisible(fader);
    fader.setSliderStyle(juce::Slider::LinearVertical);
    fader.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    fader.setRange(0.0, 2.0, 0.01);
    fader.setValue(1.0);
    fader.onValueChange = [this] { handleFaderChange(); };
    
    // Pan
    addAndMakeVisible(pan);
    pan.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pan.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pan.setRange(-1.0, 1.0, 0.01);
    pan.setValue(0.0);
    pan.onValueChange = [this] { handlePanChange(); };
    
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
    
    addAndMakeVisible(editButton);
    editButton.setButtonText("E");
    editButton.onClick = [this] { handleEditClick(); };
    
    // Meter
    addAndMakeVisible(meter);
    
    if (auto* mixer = owner.getMixer()) {
        mixer->addChangeListener(this);
    }
}

void MixerComponent::ChannelStrip::handleFaderChange() {
    if (auto* mixer = owner.getMixer()) {
        mixer->setChannelVolume(channelIndex, static_cast<float>(fader.getValue()));
    }
}

void MixerComponent::ChannelStrip::handlePanChange() {
    if (auto* mixer = owner.getMixer()) {
        mixer->setChannelPan(channelIndex, static_cast<float>(pan.getValue()));
    }
}

void MixerComponent::ChannelStrip::handleMuteClick() {
    if (auto* mixer = owner.getMixer()) {
        mixer->setChannelMute(channelIndex, muteButton.getToggleState());
    }
}

void MixerComponent::ChannelStrip::handleSoloClick() {
    if (auto* mixer = owner.getMixer()) {
        mixer->setChannelSolo(channelIndex, soloButton.getToggleState());
    }
}

void MixerComponent::ChannelStrip::handleRecordClick() {
    if (auto* project = owner.getProject()) {
        const auto& tracks = project->getTracks();
        if (channelIndex < tracks.size()) {
            auto params = tracks[channelIndex]->getParameters();
            params.record = recordButton.getToggleState();
            tracks[channelIndex]->setParameters(params);
        }
    }
}

void MixerComponent::ChannelStrip::handleEditClick() {
    // TODO: Show channel edit dialog
}

void MixerComponent::ChannelStrip::MeterBar::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    const auto bounds = getLocalBounds().toFloat();
    
    // Draw background
    g.setColour(lf.getMeterBackground());
    g.fillRect(bounds);
    
    // Draw RMS level
    const float rmsHeight = bounds.getHeight() * rms;
    g.setColour(lf.getMeterRMSColour());
    g.fillRect(bounds.withHeight(rmsHeight).withY(bounds.getBottom() - rmsHeight));
    
    // Draw peak level
    const float peakHeight = bounds.getHeight() * peak;
    g.setColour(lf.getMeterPeakColour());
    g.fillRect(bounds.withHeight(2.0f).withY(bounds.getBottom() - peakHeight));
}

//==============================================================================
// BusStrip Implementation
//==============================================================================

MixerComponent::BusStrip::BusStrip(MixerComponent& owner, int index)
    : owner(owner)
    , busIndex(index) {
    setupControls();
    updateFromBus();
}

MixerComponent::BusStrip::~BusStrip() {
    if (auto* mixer = owner.getMixer()) {
        mixer->removeChangeListener(this);
    }
}

void MixerComponent::BusStrip::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    
    // Draw background
    g.fillAll(lf.getBusStripBackground());
    
    // Draw border
    g.setColour(lf.getBusStripBorder());
    g.drawRect(getLocalBounds());
}

void MixerComponent::BusStrip::resized() {
    juce::Grid grid;
    grid.setGap(juce::Grid::Px(4));
    
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateRows = {
        Track(Fr(1)),    // Name
        Track(Fr(1)),    // Output
        Track(Fr(8)),    // Meter
        Track(Fr(1)),    // Pan
        Track(Fr(1)),    // Buttons
        Track(Fr(12))    // Fader
    };
    
    grid.templateColumns = { Track(Fr(1)) };
    
    juce::Array<juce::GridItem> items;
    items.add(juce::GridItem(nameLabel));
    items.add(juce::GridItem(outputSelector));
    items.add(juce::GridItem(meter));
    items.add(juce::GridItem(pan));
    
    // Buttons panel
    juce::Grid buttonGrid;
    buttonGrid.setGap(juce::Grid::Px(2));
    buttonGrid.templateColumns = { Track(Fr(1)), Track(Fr(1)) };
    buttonGrid.templateRows = { Track(Fr(1)) };
    
    juce::Array<juce::GridItem> buttonItems;
    buttonItems.add(muteButton);
    buttonItems.add(editButton);
    buttonGrid.items = buttonItems;
    
    items.add(juce::GridItem(buttonGrid));
    items.add(juce::GridItem(fader));
    
    grid.items = items;
    grid.performLayout(getLocalBounds());
}

void MixerComponent::BusStrip::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == owner.getMixer()) {
        updateFromBus();
    }
}

void MixerComponent::BusStrip::updateFromBus() {
    if (auto* mixer = owner.getMixer()) {
        const auto& bus = mixer->getBus(busIndex);
        
        nameLabel.setText(bus.name, juce::dontSendNotification);
        fader.setValue(bus.channel.volume, juce::dontSendNotification);
        pan.setValue(bus.channel.pan, juce::dontSendNotification);
        muteButton.setToggleState(bus.channel.mute, juce::dontSendNotification);
        outputSelector.setSelectedId(bus.outputBus + 2, juce::dontSendNotification);
    }
}

void MixerComponent::BusStrip::updateMeters(float peak, float rms) {
    meter.peak = peak;
    meter.rms = rms;
    meter.repaint();
}

void MixerComponent::BusStrip::setupControls() {
    // Name label
    addAndMakeVisible(nameLabel);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(juce::Font(12.0f));
    
    // Output selector
    addAndMakeVisible(outputSelector);
    outputSelector.addItem("Master", 1);
    if (auto* mixer = owner.getMixer()) {
        const auto busNames = mixer->getBusNames();
        for (int i = 0; i < busNames.size(); ++i) {
            if (i != busIndex) {
                outputSelector.addItem(busNames[i], i + 2);
            }
        }
    }
    outputSelector.onChange = [this] { handleOutputChange(); };
    
    // Fader
    addAndMakeVisible(fader);
    fader.setSliderStyle(juce::Slider::LinearVertical);
    fader.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    fader.setRange(0.0, 2.0, 0.01);
    fader.setValue(1.0);
    fader.onValueChange = [this] { handleFaderChange(); };
    
    // Pan
    addAndMakeVisible(pan);
    pan.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pan.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pan.setRange(-1.0, 1.0, 0.01);
    pan.setValue(0.0);
    pan.onValueChange = [this] { handlePanChange(); };
    
    // Buttons
    addAndMakeVisible(muteButton);
    muteButton.setButtonText("M");
    muteButton.setClickingTogglesState(true);
    muteButton.onClick = [this] { handleMuteClick(); };
    
    addAndMakeVisible(editButton);
    editButton.setButtonText("E");
    editButton.onClick = [this] { handleEditClick(); };
    
    // Meter
    addAndMakeVisible(meter);
    
    if (auto* mixer = owner.getMixer()) {
        mixer->addChangeListener(this);
    }
}

void MixerComponent::BusStrip::handleFaderChange() {
    if (auto* mixer = owner.getMixer()) {
        auto& bus = mixer->getBus(busIndex);
        bus.channel.volume = static_cast<float>(fader.getValue());
    }
}

void MixerComponent::BusStrip::handlePanChange() {
    if (auto* mixer = owner.getMixer()) {
        auto& bus = mixer->getBus(busIndex);
        bus.channel.pan = static_cast<float>(pan.getValue());
    }
}

void MixerComponent::BusStrip::handleMuteClick() {
    if (auto* mixer = owner.getMixer()) {
        auto& bus = mixer->getBus(busIndex);
        bus.channel.mute = muteButton.getToggleState();
    }
}

void MixerComponent::BusStrip::handleEditClick() {
    // TODO: Show bus edit dialog
}

void MixerComponent::BusStrip::handleOutputChange() {
    if (auto* mixer = owner.getMixer()) {
        const int selectedId = outputSelector.getSelectedId();
        mixer->setBusOutput(busIndex, selectedId > 1 ? selectedId - 2 : -1);
    }
}

void MixerComponent::BusStrip::MeterBar::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    const auto bounds = getLocalBounds().toFloat();
    
    // Draw background
    g.setColour(lf.getMeterBackground());
    g.fillRect(bounds);
    
    // Draw RMS level
    const float rmsHeight = bounds.getHeight() * rms;
    g.setColour(lf.getMeterRMSColour());
    g.fillRect(bounds.withHeight(rmsHeight).withY(bounds.getBottom() - rmsHeight));
    
    // Draw peak level
    const float peakHeight = bounds.getHeight() * peak;
    g.setColour(lf.getMeterPeakColour());
    g.fillRect(bounds.withHeight(2.0f).withY(bounds.getBottom() - peakHeight));
}

//==============================================================================
// MasterStrip Implementation
//==============================================================================

MixerComponent::MasterStrip::MasterStrip(MixerComponent& owner)
    : owner(owner) {
    setupControls();
    updateFromMaster();
}

MixerComponent::MasterStrip::~MasterStrip() {
    if (auto* mixer = owner.getMixer()) {
        mixer->removeChangeListener(this);
    }
}

void MixerComponent::MasterStrip::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    
    // Draw background
    g.fillAll(lf.getMasterStripBackground());
    
    // Draw border
    g.setColour(lf.getMasterStripBorder());
    g.drawRect(getLocalBounds());
}

void MixerComponent::MasterStrip::resized() {
    juce::Grid grid;
    grid.setGap(juce::Grid::Px(4));
    
    using Track = juce::Grid::TrackInfo;
    using Fr = juce::Grid::Fr;
    
    grid.templateRows = {
        Track(Fr(1)),    // Name
        Track(Fr(8)),    // Meter
        Track(Fr(1)),    // Pan
        Track(Fr(1)),    // Buttons
        Track(Fr(12))    // Fader
    };
    
    grid.templateColumns = { Track(Fr(1)) };
    
    juce::Array<juce::GridItem> items;
    items.add(juce::GridItem(nameLabel));
    items.add(juce::GridItem(meter));
    items.add(juce::GridItem(pan));
    
    // Buttons panel
    juce::Grid buttonGrid;
    buttonGrid.setGap(juce::Grid::Px(2));
    buttonGrid.templateColumns = { Track(Fr(1)), Track(Fr(1)) };
    buttonGrid.templateRows = { Track(Fr(1)) };
    
    juce::Array<juce::GridItem> buttonItems;
    buttonItems.add(muteButton);
    buttonItems.add(editButton);
    buttonGrid.items = buttonItems;
    
    items.add(juce::GridItem(buttonGrid));
    items.add(juce::GridItem(fader));
    
    grid.items = items;
    grid.performLayout(getLocalBounds());
}

void MixerComponent::MasterStrip::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == owner.getMixer()) {
        updateFromMaster();
    }
}

void MixerComponent::MasterStrip::updateFromMaster() {
    if (auto* mixer = owner.getMixer()) {
        const auto& master = mixer->getMasterChannel();
        
        fader.setValue(master.volume, juce::dontSendNotification);
        pan.setValue(master.pan, juce::dontSendNotification);
        muteButton.setToggleState(master.mute, juce::dontSendNotification);
    }
}

void MixerComponent::MasterStrip::updateMeters(float peak, float rms) {
    meter.peak = peak;
    meter.rms = rms;
    meter.repaint();
}

void MixerComponent::MasterStrip::setupControls() {
    // Name label
    addAndMakeVisible(nameLabel);
    nameLabel.setText("Master", juce::dontSendNotification);
    nameLabel.setJustificationType(juce::Justification::centred);
    nameLabel.setFont(juce::Font(12.0f));
    
    // Fader
    addAndMakeVisible(fader);
    fader.setSliderStyle(juce::Slider::LinearVertical);
    fader.setTextBoxStyle(juce::Slider::TextBoxBelow, false, 50, 15);
    fader.setRange(0.0, 2.0, 0.01);
    fader.setValue(1.0);
    fader.onValueChange = [this] { handleFaderChange(); };
    
    // Pan
    addAndMakeVisible(pan);
    pan.setSliderStyle(juce::Slider::RotaryHorizontalVerticalDrag);
    pan.setTextBoxStyle(juce::Slider::NoTextBox, false, 0, 0);
    pan.setRange(-1.0, 1.0, 0.01);
    pan.setValue(0.0);
    pan.onValueChange = [this] { handlePanChange(); };
    
    // Buttons
    addAndMakeVisible(muteButton);
    muteButton.setButtonText("M");
    muteButton.setClickingTogglesState(true);
    muteButton.onClick = [this] { handleMuteClick(); };
    
    addAndMakeVisible(editButton);
    editButton.setButtonText("E");
    editButton.onClick = [this] { handleEditClick(); };
    
    // Meter
    addAndMakeVisible(meter);
    
    if (auto* mixer = owner.getMixer()) {
        mixer->addChangeListener(this);
    }
}

void MixerComponent::MasterStrip::handleFaderChange() {
    if (auto* mixer = owner.getMixer()) {
        auto& master = mixer->getMasterChannel();
        master.volume = static_cast<float>(fader.getValue());
    }
}

void MixerComponent::MasterStrip::handlePanChange() {
    if (auto* mixer = owner.getMixer()) {
        auto& master = mixer->getMasterChannel();
        master.pan = static_cast<float>(pan.getValue());
    }
}

void MixerComponent::MasterStrip::handleMuteClick() {
    if (auto* mixer = owner.getMixer()) {
        auto& master = mixer->getMasterChannel();
        master.mute = muteButton.getToggleState();
    }
}

void MixerComponent::MasterStrip::handleEditClick() {
    // TODO: Show master edit dialog
}

void MixerComponent::MasterStrip::MeterBar::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    const auto bounds = getLocalBounds().toFloat();
    
    // Draw background
    g.setColour(lf.getMeterBackground());
    g.fillRect(bounds);
    
    // Draw RMS level
    const float rmsHeight = bounds.getHeight() * rms;
    g.setColour(lf.getMeterRMSColour());
    g.fillRect(bounds.withHeight(rmsHeight).withY(bounds.getBottom() - rmsHeight));
    
    // Draw peak level
    const float peakHeight = bounds.getHeight() * peak;
    g.setColour(lf.getMeterPeakColour());
    g.fillRect(bounds.withHeight(2.0f).withY(bounds.getBottom() - peakHeight));
}

//==============================================================================
// MixerComponent Implementation
//==============================================================================

MixerComponent::MixerComponent() {
    setupLayout();
    startTimerHz(30);  // Update meters at 30Hz
}

MixerComponent::~MixerComponent() {
    if (auto* mixer = getMixer()) {
        mixer->removeChangeListener(this);
    }
}

void MixerComponent::paint(juce::Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void MixerComponent::resized() {
    auto bounds = getLocalBounds();
    
    // Layout controls at the top
    auto controlsArea = bounds.removeFromTop(30);
    addBusButton.setBounds(controlsArea.removeFromLeft(100));
    busTypeSelector.setBounds(controlsArea.removeFromLeft(100));
    
    // Layout strips
    auto stripsArea = bounds;
    const int totalWidth = (channelStrips.size() + busStrips.size() + 1) * stripWidth;
    
    if (totalWidth > stripsArea.getWidth()) {
        viewport.setBounds(stripsArea);
        stripsArea = { 0, 0, totalWidth, stripsArea.getHeight() };
    }
    
    int x = 0;
    
    for (auto* strip : channelStrips) {
        strip->setBounds(x, 0, stripWidth, stripsArea.getHeight());
        x += stripWidth;
    }
    
    for (auto* strip : busStrips) {
        strip->setBounds(x, 0, stripWidth, stripsArea.getHeight());
        x += stripWidth;
    }
    
    if (masterStrip != nullptr) {
        masterStrip->setBounds(x, 0, stripWidth, stripsArea.getHeight());
    }
}

void MixerComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == getMixer()) {
        updateChannelStrips();
        updateBusStrips();
        updateMasterStrip();
    }
}

void MixerComponent::setProject(Project* project) {
    if (currentProject == project) {
        return;
    }

    if (auto* oldMixer = getMixer()) {
        oldMixer->removeChangeListener(this);
    }
    
    currentProject = project;
    
    if (auto* newMixer = getMixer()) {
        newMixer->addChangeListener(this);
    }
    
    updateChannelStrips();
    updateBusStrips();
    updateMasterStrip();
}

Mixer* MixerComponent::getMixer() const {
    return currentProject != nullptr ? &currentProject->getMixer() : nullptr;
}

void MixerComponent::updateChannelStrips() {
    const int numChannels = currentProject != nullptr ?
        static_cast<int>(currentProject->getTracks().size()) : 0;
    
    // Add/remove channel strips
    while (channelStrips.size() < numChannels) {
        auto* strip = new ChannelStrip(*this, channelStrips.size());
        channelStrips.add(strip);
        addAndMakeVisible(strip);
    }
    
    while (channelStrips.size() > numChannels) {
        channelStrips.removeLast();
    }
    
    // Update existing strips
    for (auto* strip : channelStrips) {
        strip->updateFromTrack();
    }
    
    resized();
}

void MixerComponent::updateBusStrips() {
    const int numBuses = getMixer() != nullptr ? getMixer()->getNumBuses() : 0;
    
    // Add/remove bus strips
    while (busStrips.size() < numBuses) {
        auto* strip = new BusStrip(*this, busStrips.size());
        busStrips.add(strip);
        addAndMakeVisible(strip);
    }
    
    while (busStrips.size() > numBuses) {
        busStrips.removeLast();
    }
    
    // Update existing strips
    for (auto* strip : busStrips) {
        strip->updateFromBus();
    }
    
    resized();
}

void MixerComponent::updateMasterStrip() {
    if (getMixer() != nullptr && masterStrip == nullptr) {
        masterStrip = std::make_unique<MasterStrip>(*this);
        addAndMakeVisible(masterStrip.get());
    } else if (getMixer() == nullptr) {
        masterStrip = nullptr;
    }
    
    if (masterStrip != nullptr) {
        masterStrip->updateFromMaster();
    }
    
    resized();
}

void MixerComponent::updateMeters() {
    if (auto* mixer = getMixer()) {
        // Update channel meters
        for (int i = 0; i < channelStrips.size(); ++i) {
            channelStrips[i]->updateMeters(
                mixer->getChannelPeakLevel(i),
                mixer->getChannelRMSLevel(i));
        }
        
        // Update bus meters
        for (int i = 0; i < busStrips.size(); ++i) {
            const auto& bus = mixer->getBus(i);
            busStrips[i]->updateMeters(
                bus.channel.peakLevel,
                bus.channel.rmsLevel);
        }
        
        // Update master meters
        if (masterStrip != nullptr) {
            const auto& master = mixer->getMasterChannel();
            masterStrip->updateMeters(
                master.peakLevel,
                master.rmsLevel);
        }
    }
}

void MixerComponent::setupLayout() {
    // Add bus button
    addAndMakeVisible(addBusButton);
    addBusButton.setButtonText("Add Bus");
    addBusButton.onClick = [this] { handleAddBusClick(); };
    
    // Bus type selector
    addAndMakeVisible(busTypeSelector);
    busTypeSelector.addItem("Aux Bus", 1);
    busTypeSelector.addItem("Group Bus", 2);
    busTypeSelector.setSelectedId(1);
    busTypeSelector.onChange = [this] { handleBusTypeChange(); };
    
    // Viewport for scrolling
    addAndMakeVisible(viewport);
    viewport.setViewedComponent(new juce::Component());
    viewport.setScrollBarsShown(true, false);
}

void MixerComponent::handleAddBusClick() {
    if (auto* mixer = getMixer()) {
        const auto type = busTypeSelector.getSelectedId() == 1 ?
            Mixer::BusType::Aux : Mixer::BusType::Group;
            
        mixer->addBus(type, "New Bus " + juce::String(mixer->getNumBuses() + 1));
    }
}

void MixerComponent::handleBusTypeChange() {
    // Nothing to do here - type is used when creating new buses
}