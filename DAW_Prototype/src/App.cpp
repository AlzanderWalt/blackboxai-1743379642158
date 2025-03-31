#include "App.h"
#include "Logger.h"

//==============================================================================
// MainWindow Implementation
//==============================================================================

App::MainWindow::MainWindow(const juce::String& name)
    : DocumentWindow(name,
                    juce::Desktop::getInstance().getDefaultLookAndFeel()
                        .findColour(juce::ResizableWindow::backgroundColourId),
                    DocumentWindow::allButtons) {
    setUsingNativeTitleBar(true);
    setContentOwned(new MainComponent(), true);
    setResizable(true, true);
    
    // Center window and set minimum size
    centreWithSize(1200, 800);
    setMinimumSize(800, 600);
    
    // Set up menu bar
    setMenuBar(App::getCommandManager().getMenuBarComponent());
    
    // Make window visible
    setVisible(true);
}

App::MainWindow::~MainWindow() {
    setMenuBar(nullptr);
}

void App::MainWindow::closeButtonPressed() {
    JUCEApplication::getInstance()->systemRequestedQuit();
}

//==============================================================================
// CommandManager Implementation
//==============================================================================

App::CommandManager::CommandManager() {
    // Register commands
    juce::JUCEApplication::getInstance()->setApplicationCommandManagerToWatch(this);
}

void App::CommandManager::getAllCommands(juce::Array<juce::CommandID>& commands) {
    const juce::CommandID ids[] = {
        NewProject,
        OpenProject,
        SaveProject,
        SaveProjectAs,
        
        Undo,
        Redo,
        Cut,
        Copy,
        Paste,
        Delete,
        SelectAll,
        
        AddAudioTrack,
        AddMIDITrack,
        DeleteSelectedTracks,
        
        ShowMixer,
        ShowPianoRoll,
        
        Play,
        Stop,
        Record,
        ToggleLoop,
        
        ShowPluginManager,
        ShowSettings,
        ShowAbout
    };
    
    commands.addArray(ids, juce::numElementsInArray(ids));
}

void App::CommandManager::getCommandInfo(juce::CommandID commandID,
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
            break;
            
        case SaveProjectAs:
            result.setInfo("Save Project As...", "Save the project with a new name", "File", 0);
            result.addDefaultKeypress('s', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
            break;
            
        case Undo:
            result.setInfo("Undo", "Undo the last action", "Edit", 0);
            result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier);
            break;
            
        case Redo:
            result.setInfo("Redo", "Redo the last undone action", "Edit", 0);
            result.addDefaultKeypress('z', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
            break;
            
        case Cut:
            result.setInfo("Cut", "Cut the selected items", "Edit", 0);
            result.addDefaultKeypress('x', juce::ModifierKeys::commandModifier);
            break;
            
        case Copy:
            result.setInfo("Copy", "Copy the selected items", "Edit", 0);
            result.addDefaultKeypress('c', juce::ModifierKeys::commandModifier);
            break;
            
        case Paste:
            result.setInfo("Paste", "Paste the clipboard contents", "Edit", 0);
            result.addDefaultKeypress('v', juce::ModifierKeys::commandModifier);
            break;
            
        case Delete:
            result.setInfo("Delete", "Delete the selected items", "Edit", 0);
            result.addDefaultKeypress(juce::KeyPress::deleteKey, 0);
            break;
            
        case SelectAll:
            result.setInfo("Select All", "Select all items", "Edit", 0);
            result.addDefaultKeypress('a', juce::ModifierKeys::commandModifier);
            break;
            
        case AddAudioTrack:
            result.setInfo("Add Audio Track", "Add a new audio track", "Track", 0);
            result.addDefaultKeypress('t', juce::ModifierKeys::commandModifier);
            break;
            
        case AddMIDITrack:
            result.setInfo("Add MIDI Track", "Add a new MIDI track", "Track", 0);
            result.addDefaultKeypress('t', juce::ModifierKeys::commandModifier | juce::ModifierKeys::shiftModifier);
            break;
            
        case DeleteSelectedTracks:
            result.setInfo("Delete Selected Tracks", "Delete the selected tracks", "Track", 0);
            break;
            
        case ShowMixer:
            result.setInfo("Show/Hide Mixer", "Toggle mixer visibility", "View", 0);
            result.addDefaultKeypress('m', juce::ModifierKeys::commandModifier);
            break;
            
        case ShowPianoRoll:
            result.setInfo("Show/Hide Piano Roll", "Toggle piano roll visibility", "View", 0);
            result.addDefaultKeypress('p', juce::ModifierKeys::commandModifier);
            break;
            
        case Play:
            result.setInfo("Play/Pause", "Start or pause playback", "Transport", 0);
            result.addDefaultKeypress(juce::KeyPress::spaceKey, 0);
            break;
            
        case Stop:
            result.setInfo("Stop", "Stop playback", "Transport", 0);
            result.addDefaultKeypress('.', juce::ModifierKeys::commandModifier);
            break;
            
        case Record:
            result.setInfo("Record", "Toggle recording", "Transport", 0);
            result.addDefaultKeypress('r', juce::ModifierKeys::commandModifier);
            break;
            
        case ToggleLoop:
            result.setInfo("Toggle Loop", "Toggle loop mode", "Transport", 0);
            result.addDefaultKeypress('l', juce::ModifierKeys::commandModifier);
            break;
            
        case ShowPluginManager:
            result.setInfo("Plugin Manager...", "Show the plugin manager", "Tools", 0);
            break;
            
        case ShowSettings:
            result.setInfo("Settings...", "Show application settings", "Tools", 0);
            result.addDefaultKeypress(',', juce::ModifierKeys::commandModifier);
            break;
            
        case ShowAbout:
            result.setInfo("About...", "Show application information", "Help", 0);
            break;
            
        default:
            break;
    }
}

bool App::CommandManager::perform(const juce::ApplicationCommandTarget::InvocationInfo& info) {
    switch (info.commandID) {
        case NewProject:
            // TODO: Handle new project command
            break;
            
        case OpenProject:
            // TODO: Handle open project command
            break;
            
        case SaveProject:
            // TODO: Handle save project command
            break;
            
        case SaveProjectAs:
            // TODO: Handle save project as command
            break;
            
        case Undo:
            // TODO: Handle undo command
            break;
            
        case Redo:
            // TODO: Handle redo command
            break;
            
        case Cut:
            // TODO: Handle cut command
            break;
            
        case Copy:
            // TODO: Handle copy command
            break;
            
        case Paste:
            // TODO: Handle paste command
            break;
            
        case Delete:
            // TODO: Handle delete command
            break;
            
        case SelectAll:
            // TODO: Handle select all command
            break;
            
        case AddAudioTrack:
            // TODO: Handle add audio track command
            break;
            
        case AddMIDITrack:
            // TODO: Handle add MIDI track command
            break;
            
        case DeleteSelectedTracks:
            // TODO: Handle delete selected tracks command
            break;
            
        case ShowMixer:
            // TODO: Handle show/hide mixer command
            break;
            
        case ShowPianoRoll:
            // TODO: Handle show/hide piano roll command
            break;
            
        case Play:
            // TODO: Handle play/pause command
            break;
            
        case Stop:
            // TODO: Handle stop command
            break;
            
        case Record:
            // TODO: Handle record command
            break;
            
        case ToggleLoop:
            // TODO: Handle toggle loop command
            break;
            
        case ShowPluginManager:
            // TODO: Handle show plugin manager command
            break;
            
        case ShowSettings:
            // TODO: Handle show settings command
            break;
            
        case ShowAbout:
            // TODO: Handle show about command
            break;
            
        default:
            return false;
    }
    
    return true;
}

//==============================================================================
// SettingsManager Implementation
//==============================================================================

App::SettingsManager::SettingsManager() {
    loadSettings();
}

App::SettingsManager::~SettingsManager() {
    saveSettings();
}

void App::SettingsManager::saveSettings() {
    juce::ValueTree state("Settings");
    
    // Save audio settings
    auto audioNode = state.getOrCreateChildWithName("Audio", nullptr);
    audioNode.setProperty("outputDevice", audioSettings.outputDevice, nullptr);
    audioNode.setProperty("inputDevice", audioSettings.inputDevice, nullptr);
    audioNode.setProperty("sampleRate", audioSettings.sampleRate, nullptr);
    audioNode.setProperty("bufferSize", audioSettings.bufferSize, nullptr);
    audioNode.setProperty("inputChannels", audioSettings.inputChannels, nullptr);
    audioNode.setProperty("outputChannels", audioSettings.outputChannels, nullptr);
    
    // Save MIDI settings
    auto midiNode = state.getOrCreateChildWithName("MIDI", nullptr);
    midiNode.setProperty("thruEnabled", midiSettings.thruEnabled, nullptr);
    midiNode.setProperty("clockEnabled", midiSettings.clockEnabled, nullptr);
    midiNode.setProperty("mtcEnabled", midiSettings.mtcEnabled, nullptr);
    
    auto inputDevicesNode = midiNode.getOrCreateChildWithName("InputDevices", nullptr);
    for (const auto& device : midiSettings.inputDevices) {
        auto deviceNode = inputDevicesNode.createChild("Device");
        deviceNode.setProperty("name", device, nullptr);
    }
    
    // Save UI settings
    auto uiNode = state.getOrCreateChildWithName("UI", nullptr);
    uiNode.setProperty("darkMode", uiSettings.darkMode, nullptr);
    uiNode.setProperty("fontSize", uiSettings.fontSize, nullptr);
    uiNode.setProperty("fontName", uiSettings.fontName, nullptr);
    uiNode.setProperty("showTooltips", uiSettings.showTooltips, nullptr);
    
    // Write to file
    if (auto xml = state.createXml()) {
        xml->writeTo(getSettingsFile());
    }
}

void App::SettingsManager::loadSettings() {
    if (auto xml = juce::XmlDocument::parse(getSettingsFile())) {
        juce::ValueTree state = juce::ValueTree::fromXml(*xml);
        
        // Load audio settings
        if (auto audioNode = state.getChildWithName("Audio")) {
            audioSettings.outputDevice = audioNode.getProperty("outputDevice", audioSettings.outputDevice);
            audioSettings.inputDevice = audioNode.getProperty("inputDevice", audioSettings.inputDevice);
            audioSettings.sampleRate = audioNode.getProperty("sampleRate", audioSettings.sampleRate);
            audioSettings.bufferSize = audioNode.getProperty("bufferSize", audioSettings.bufferSize);
            audioSettings.inputChannels = audioNode.getProperty("inputChannels", audioSettings.inputChannels);
            audioSettings.outputChannels = audioNode.getProperty("outputChannels", audioSettings.outputChannels);
        }
        
        // Load MIDI settings
        if (auto midiNode = state.getChildWithName("MIDI")) {
            midiSettings.thruEnabled = midiNode.getProperty("thruEnabled", midiSettings.thruEnabled);
            midiSettings.clockEnabled = midiNode.getProperty("clockEnabled", midiSettings.clockEnabled);
            midiSettings.mtcEnabled = midiNode.getProperty("mtcEnabled", midiSettings.mtcEnabled);
            
            midiSettings.inputDevices.clear();
            if (auto inputDevicesNode = midiNode.getChildWithName("InputDevices")) {
                for (auto deviceNode : inputDevicesNode) {
                    midiSettings.inputDevices.add(deviceNode.getProperty("name").toString());
                }
            }
        }
        
        // Load UI settings
        if (auto uiNode = state.getChildWithName("UI")) {
            uiSettings.darkMode = uiNode.getProperty("darkMode", uiSettings.darkMode);
            uiSettings.fontSize = uiNode.getProperty("fontSize", uiSettings.fontSize);
            uiSettings.fontName = uiNode.getProperty("fontName", uiSettings.fontName);
            uiSettings.showTooltips = uiNode.getProperty("showTooltips", uiSettings.showTooltips);
        }
    }
}

juce::File App::SettingsManager::getSettingsFile() const {
    return juce::File::getSpecialLocation(juce::File::userApplicationDataDirectory)
        .getChildFile("DAW_Prototype")
        .getChildFile("settings.xml");
}

//==============================================================================
// App Implementation
//==============================================================================

void App::initialise(const juce::String& commandLine) {
    LOG_INFO("Initializing application");
    
    // Initialize managers
    commandManager = std::make_unique<CommandManager>();
    settingsManager = std::make_unique<SettingsManager>();
    lookAndFeel = std::make_unique<CustomLookAndFeel>();
    
    // Set up look and feel
    juce::LookAndFeel::setDefaultLookAndFeel(lookAndFeel.get());
    
    // Create main window
    mainWindow = std::make_unique<MainWindow>(getApplicationName());
    
    LOG_INFO("Application initialized successfully");
}

void App::shutdown() {
    LOG_INFO("Shutting down application");
    
    // Save settings
    settingsManager->saveSettings();
    
    // Clean up window
    mainWindow = nullptr;
    
    // Clean up managers
    commandManager = nullptr;
    settingsManager = nullptr;
    lookAndFeel = nullptr;
    
    LOG_INFO("Application shutdown complete");
}

void App::systemRequestedQuit() {
    // TODO: Check for unsaved changes
    quit();
}

void App::anotherInstanceStarted(const juce::String& commandLine) {
    // TODO: Handle command line arguments from other instance
}

// This creates the application instance
START_JUCE_APPLICATION(App)