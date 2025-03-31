#pragma once
#include <JuceHeader.h>
#include "Track.h"

class Project;
class Clip;

class TrackEditorComponent : public juce::Component,
                           public juce::ChangeListener {
public:
    // Track header component
    class TrackHeader : public juce::Component,
                       public juce::ChangeListener {
    public:
        TrackHeader(TrackEditorComponent& owner, Track& track);
        ~TrackHeader() override;
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;
        
        void updateFromTrack();
        
    private:
        TrackEditorComponent& owner;
        Track& track;
        
        juce::Label nameLabel;
        juce::TextButton muteButton;
        juce::TextButton soloButton;
        juce::TextButton recordButton;
        juce::TextButton automationButton;
        juce::Slider heightSlider;
        
        void setupControls();
        void handleNameChange();
        void handleMuteClick();
        void handleSoloClick();
        void handleRecordClick();
        void handleAutomationClick();
        void handleHeightChange();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackHeader)
    };

    // Track content component
    class TrackContent : public juce::Component,
                        public juce::ChangeListener,
                        public juce::DragAndDropTarget {
    public:
        TrackContent(TrackEditorComponent& owner, Track& track);
        ~TrackContent() override;
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void changeListenerCallback(juce::ChangeBroadcaster* source) override;
        
        // DragAndDropTarget interface
        bool isInterestedInDragSource(const SourceDetails& source) override;
        void itemDragEnter(const SourceDetails& source) override;
        void itemDragMove(const SourceDetails& source) override;
        void itemDragExit(const SourceDetails& source) override;
        void itemDropped(const SourceDetails& source) override;
        
        void updateFromTrack();
        void setTimeRange(double start, double end);
        
    private:
        TrackEditorComponent& owner;
        Track& track;
        
        double timeStart{0.0};
        double timeEnd{60.0};
        double pixelsPerSecond{100.0};
        
        bool dragging{false};
        double dragStartTime{0.0};
        int draggedClipIndex{-1};
        
        struct ClipView {
            std::unique_ptr<juce::Component> component;
            double startTime;
            double endTime;
            bool selected;
        };
        std::vector<ClipView> clipViews;
        
        void createClipViews();
        void updateClipPositions();
        double timeToX(double time) const;
        double xToTime(int x) const;
        int findClipAt(juce::Point<int> position) const;
        void selectClip(int index, bool deselectOthers);
        void deleteSelectedClips();
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackContent)
    };

    // Constructor/Destructor
    TrackEditorComponent();
    ~TrackEditorComponent() override;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ChangeListener interface
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // Project handling
    void setProject(Project* project);
    Project* getProject() const { return currentProject; }

    // Time range
    void setTimeRange(double start, double end);
    void setVisibleRange(double start, double end);
    double getVisibleStart() const { return visibleTimeStart; }
    double getVisibleEnd() const { return visibleTimeEnd; }
    
    // Track management
    void addTrack(Track::Type type);
    void removeTrack(int index);
    void moveTrack(int fromIndex, int toIndex);
    
    // Selection
    void selectTrack(int index, bool deselectOthers = true);
    void selectClip(Track* track, int clipIndex, bool deselectOthers = true);
    void clearSelection();
    
    // Editing
    void deleteSelected();
    void splitSelectedClips();
    void mergeSelectedClips();
    void duplicateSelectedClips();
    
    // Automation
    void showAutomation(const juce::String& paramId);
    void hideAutomation();

private:
    Project* currentProject{nullptr};
    
    struct TrackView {
        std::unique_ptr<TrackHeader> header;
        std::unique_ptr<TrackContent> content;
        bool selected;
    };
    std::vector<TrackView> trackViews;
    
    juce::Viewport headerViewport;
    juce::Viewport contentViewport;
    juce::Component headerContainer;
    juce::Component contentContainer;
    
    double timeStart{0.0};
    double timeEnd{60.0};
    double visibleTimeStart{0.0};
    double visibleTimeEnd{60.0};
    double pixelsPerSecond{100.0};
    
    void updateTrackViews();
    void updateTimeRange();
    void updateLayout();
    
    static constexpr int headerWidth = 200;
    static constexpr int minTrackHeight = 60;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(TrackEditorComponent)
};