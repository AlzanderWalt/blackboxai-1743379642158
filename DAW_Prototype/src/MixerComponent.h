#pragma once
#include <JuceHeader.h>
#include "Mixer.h"

class Project;
class Track;

class MixerComponent : public juce::Component,
                      public juce::ChangeListener {
public:
    // Channel strip component
    class ChannelStrip : public juce::Component,
                        public juce::ChangeListener {
    public:
        ChannelStrip(MixerComponent& owner, int index);
        ~ChannelStrip() override;
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;
        
        void updateFromTrack();
        void updateMeters(float peak, float rms);
        
    private:
        MixerComponent& owner;
        int channelIndex;
        
        juce::Label nameLabel;
        juce::Slider fader;
        juce::Slider pan;
        juce::TextButton muteButton;
        juce::TextButton soloButton;
        juce::TextButton recordButton;
        juce::TextButton editButton;
        
        struct MeterBar : public juce::Component {
            void paint(juce::Graphics& g) override;
            float peak{0.0f}, rms{0.0f};
        } meter;
        
        juce::OwnedArray<juce::Component> plugins;
        juce::OwnedArray<juce::Component> sends;
        
        void setupControls();
        void handleFaderChange();
        void handlePanChange();
        void handleMuteClick();
        void handleSoloClick();
        void handleRecordClick();
        void handleEditClick();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ChannelStrip)
    };

    // Bus strip component
    class BusStrip : public juce::Component,
                    public juce::ChangeListener {
    public:
        BusStrip(MixerComponent& owner, int index);
        ~BusStrip() override;
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;
        
        void updateFromBus();
        void updateMeters(float peak, float rms);
        
    private:
        MixerComponent& owner;
        int busIndex;
        
        juce::Label nameLabel;
        juce::Slider fader;
        juce::Slider pan;
        juce::TextButton muteButton;
        juce::TextButton editButton;
        juce::ComboBox outputSelector;
        
        struct MeterBar : public juce::Component {
            void paint(juce::Graphics& g) override;
            float peak{0.0f}, rms{0.0f};
        } meter;
        
        juce::OwnedArray<juce::Component> plugins;
        
        void setupControls();
        void handleFaderChange();
        void handlePanChange();
        void handleMuteClick();
        void handleEditClick();
        void handleOutputChange();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(BusStrip)
    };

    // Master strip component
    class MasterStrip : public juce::Component,
                       public juce::ChangeListener {
    public:
        MasterStrip(MixerComponent& owner);
        ~MasterStrip() override;
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;
        
        void updateFromMaster();
        void updateMeters(float peak, float rms);
        
    private:
        MixerComponent& owner;
        
        juce::Label nameLabel;
        juce::Slider fader;
        juce::Slider pan;
        juce::TextButton muteButton;
        juce::TextButton editButton;
        
        struct MeterBar : public juce::Component {
            void paint(juce::Graphics& g) override;
            float peak{0.0f}, rms{0.0f};
        } meter;
        
        juce::OwnedArray<juce::Component> plugins;
        
        void setupControls();
        void handleFaderChange();
        void handlePanChange();
        void handleMuteClick();
        void handleEditClick();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MasterStrip)
    };

    // Constructor/Destructor
    MixerComponent();
    ~MixerComponent() override;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ChangeListener interface
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // Project handling
    void setProject(Project* project);
    Project* getProject() const { return currentProject; }

    // Mixer access
    Mixer* getMixer() const;
    
    // Update handling
    void updateChannelStrips();
    void updateBusStrips();
    void updateMasterStrip();
    void updateMeters();

private:
    Project* currentProject{nullptr};
    
    juce::OwnedArray<ChannelStrip> channelStrips;
    juce::OwnedArray<BusStrip> busStrips;
    std::unique_ptr<MasterStrip> masterStrip;
    
    juce::Viewport viewport;
    juce::StretchableLayoutManager layout;
    
    juce::TextButton addBusButton;
    juce::ComboBox busTypeSelector;
    
    void setupLayout();
    void handleAddBusClick();
    void handleBusTypeChange();
    
    static constexpr int stripWidth = 100;
    static constexpr int minHeight = 400;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(MixerComponent)
};