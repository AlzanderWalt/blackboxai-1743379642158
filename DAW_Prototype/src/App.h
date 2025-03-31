#pragma once
#include <JuceHeader.h>
#include "MainComponent.h"
#include "CustomLookAndFeel.h"

class App : public juce::JUCEApplication {
public:
    // Application information
    const juce::String getApplicationName() override { return "DAW Prototype"; }
    const juce::String getApplicationVersion() override { return "1.0.0"; }
    bool moreThanOneInstanceAllowed() override { return false; }

    // Application lifecycle
    void initialise(const juce::String& commandLine) override;
    void shutdown() override;
    void systemRequestedQuit() override;
    void anotherInstanceStarted(const juce::String& commandLine) override;

    // Main window class
    class MainWindow : public juce::DocumentWindow {
    public:
        MainWindow(const juce::String& name);
        ~MainWindow() override;
        
        void closeButtonPressed() override;
        
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MainWindow)
    };

    // Command manager for handling menu commands
    class CommandManager : public juce::ApplicationCommandManager {
    public:
        CommandManager();
        
        juce::ApplicationCommandTarget* getNextCommandTarget() override { return nullptr; }
        void getAllCommands(juce::Array<juce::CommandID>& commands) override;
        void getCommandInfo(juce::CommandID commandID, juce::ApplicationCommandInfo& result) override;
        bool perform(const juce::ApplicationCommandTarget::InvocationInfo& info) override;
        
        // Command IDs
        enum CommandIDs {
            NewProject = 0x2000,
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
        
    private:
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CommandManager)
    };

    // Settings manager for handling application settings
    class SettingsManager {
    public:
        SettingsManager();
        ~SettingsManager();
        
        void saveSettings();
        void loadSettings();
        
        // Audio settings
        struct AudioSettings {
            juce::String outputDevice;
            juce::String inputDevice;
            double sampleRate{44100.0};
            int bufferSize{512};
            int inputChannels{2};
            int outputChannels{2};
        };
        
        // MIDI settings
        struct MIDISettings {
            juce::StringArray inputDevices;
            bool thruEnabled{true};
            bool clockEnabled{false};
            bool mtcEnabled{false};
        };
        
        // UI settings
        struct UISettings {
            bool darkMode{false};
            int fontSize{14};
            juce::String fontName{"Default"};
            bool showTooltips{true};
        };
        
        AudioSettings audioSettings;
        MIDISettings midiSettings;
        UISettings uiSettings;
        
    private:
        juce::File getSettingsFile() const;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(SettingsManager)
    };

    // Global access
    static App& getApp() { return *dynamic_cast<App*>(JUCEApplication::getInstance()); }
    static CommandManager& getCommandManager() { return *getInstance()->commandManager; }
    static SettingsManager& getSettingsManager() { return *getInstance()->settingsManager; }
    static CustomLookAndFeel& getLookAndFeel() { return *getInstance()->lookAndFeel; }

private:
    std::unique_ptr<MainWindow> mainWindow;
    std::unique_ptr<CommandManager> commandManager;
    std::unique_ptr<SettingsManager> settingsManager;
    std::unique_ptr<CustomLookAndFeel> lookAndFeel;
    
    static App* getInstance() { return dynamic_cast<App*>(JUCEApplication::getInstance()); }
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(App)
};