#pragma once
#include <JuceHeader.h>

class Track;
class MIDIClip;

class PianoRollComponent : public juce::Component,
                          public juce::ChangeListener {
public:
    // Piano keyboard component
    class KeyboardComponent : public juce::Component {
    public:
        KeyboardComponent();
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        
        void setKeyRange(int lowest, int highest);
        int getNoteForY(int y) const;
        int getYForNote(int note) const;
        
    private:
        int lowestNote{36};  // C2
        int highestNote{96}; // C7
        int keyWidth{60};
        int playingNote{-1};
        
        void drawWhiteKey(juce::Graphics& g, int note, bool isPlaying);
        void drawBlackKey(juce::Graphics& g, int note, bool isPlaying);
        bool isBlackKey(int note) const;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KeyboardComponent)
    };

    // Grid component for note editing
    class GridComponent : public juce::Component {
    public:
        GridComponent(PianoRollComponent& owner);
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        void mouseDoubleClick(const juce::MouseEvent& e) override;
        
        void setTimeRange(double start, double end);
        void setKeyRange(int lowest, int highest);
        void setGridSize(double beats);
        void setSnapToGrid(bool shouldSnap);
        
    private:
        PianoRollComponent& owner;
        
        double timeStart{0.0};
        double timeEnd{4.0};
        double gridSize{0.25};  // Quarter note
        bool snapToGrid{true};
        
        struct NoteRect {
            juce::Rectangle<int> bounds;
            int noteNumber;
            bool selected;
        };
        std::vector<NoteRect> noteRects;
        
        bool dragging{false};
        juce::Point<int> dragStart;
        juce::Rectangle<int> selectionRect;
        
        double timeToX(double time) const;
        double xToTime(int x) const;
        int yToNote(int y) const;
        void updateNoteRects();
        void selectNotesInRect(const juce::Rectangle<int>& rect, bool addToSelection);
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(GridComponent)
    };

    // Velocity editor component
    class VelocityComponent : public juce::Component {
    public:
        VelocityComponent(PianoRollComponent& owner);
        
        void paint(juce::Graphics& g) override;
        void resized() override;
        void mouseDown(const juce::MouseEvent& e) override;
        void mouseDrag(const juce::MouseEvent& e) override;
        void mouseUp(const juce::MouseEvent& e) override;
        
        void setTimeRange(double start, double end);
        
    private:
        PianoRollComponent& owner;
        
        double timeStart{0.0};
        double timeEnd{4.0};
        
        struct VelocityBar {
            juce::Rectangle<int> bounds;
            int velocity;
            bool selected;
        };
        std::vector<VelocityBar> velocityBars;
        
        bool dragging{false};
        int draggedBarIndex{-1};
        
        double timeToX(double time) const;
        void updateVelocityBars();
        int findBarAt(juce::Point<int> position) const;
        
        JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VelocityComponent)
    };

    // Constructor/Destructor
    PianoRollComponent();
    ~PianoRollComponent() override;

    // Component interface
    void paint(juce::Graphics& g) override;
    void resized() override;
    
    // ChangeListener interface
    void changeListenerCallback(juce::ChangeBroadcaster* source) override;

    // Clip handling
    void setClip(MIDIClip* clip);
    MIDIClip* getClip() const { return currentClip; }

    // Time range
    void setTimeRange(double start, double end);
    void setVisibleRange(double start, double end);
    double getVisibleStart() const { return visibleTimeStart; }
    double getVisibleEnd() const { return visibleTimeEnd; }
    
    // Grid settings
    void setGridSize(double beats);
    void setSnapToGrid(bool shouldSnap);
    
    // Note editing
    void addNote(int noteNumber, double startTime, double duration, int velocity);
    void removeNote(int noteNumber, double startTime);
    void updateNote(int noteNumber, double startTime,
                   double newStartTime, double newDuration, int newVelocity);
    void selectNote(int noteNumber, double startTime, bool deselectOthers = true);
    void selectNotesInRange(double startTime, double endTime,
                          int lowestNote, int highestNote,
                          bool addToSelection = false);
    void clearNoteSelection();
    
    // Note manipulation
    void deleteSelectedNotes();
    void transposeSelectedNotes(int semitones);
    void moveSelectedNotes(double timeDelta);
    void resizeSelectedNotes(double durationMultiplier);
    void setSelectedNotesVelocity(int velocity);
    
    // Quantization
    void quantizeSelectedNotes(double gridSize);
    void quantizeSelectedNotesStart(double gridSize);
    void quantizeSelectedNotesEnd(double gridSize);
    void quantizeSelectedNotesDuration(double gridSize);

private:
    MIDIClip* currentClip{nullptr};
    
    std::unique_ptr<KeyboardComponent> keyboard;
    std::unique_ptr<GridComponent> grid;
    std::unique_ptr<VelocityComponent> velocityEditor;
    
    juce::Viewport gridViewport;
    juce::Viewport velocityViewport;
    
    double timeStart{0.0};
    double timeEnd{4.0};
    double visibleTimeStart{0.0};
    double visibleTimeEnd{4.0};
    double gridSize{0.25};
    bool snapToGrid{true};
    
    void updateComponents();
    void synchronizeViewports();
    
    static constexpr int keyboardWidth = 60;
    static constexpr int velocityHeight = 100;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PianoRollComponent)
};