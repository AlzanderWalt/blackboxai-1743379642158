#include "PianoRollComponent.h"
#include "Track.h"
#include "Clip.h"
#include "Logger.h"
#include "CustomLookAndFeel.h"

//==============================================================================
// KeyboardComponent Implementation
//==============================================================================

PianoRollComponent::KeyboardComponent::KeyboardComponent() {
    setWantsKeyboardFocus(true);
}

void PianoRollComponent::KeyboardComponent::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    
    // Draw white keys first
    for (int note = lowestNote; note <= highestNote; ++note) {
        if (!isBlackKey(note)) {
            drawWhiteKey(g, note, note == playingNote);
        }
    }
    
    // Draw black keys on top
    for (int note = lowestNote; note <= highestNote; ++note) {
        if (isBlackKey(note)) {
            drawBlackKey(g, note, note == playingNote);
        }
    }
}

void PianoRollComponent::KeyboardComponent::resized() {
    // Nothing to do here
}

void PianoRollComponent::KeyboardComponent::mouseDown(const juce::MouseEvent& e) {
    const int note = getNoteForY(e.y);
    if (note >= lowestNote && note <= highestNote) {
        playingNote = note;
        // TODO: Send MIDI note on message
        repaint();
    }
}

void PianoRollComponent::KeyboardComponent::mouseUp(const juce::MouseEvent&) {
    if (playingNote >= 0) {
        // TODO: Send MIDI note off message
        playingNote = -1;
        repaint();
    }
}

void PianoRollComponent::KeyboardComponent::setKeyRange(int lowest, int highest) {
    lowestNote = lowest;
    highestNote = highest;
    repaint();
}

int PianoRollComponent::KeyboardComponent::getNoteForY(int y) const {
    const float keyHeight = static_cast<float>(getHeight()) / (highestNote - lowestNote + 1);
    return highestNote - static_cast<int>(y / keyHeight);
}

int PianoRollComponent::KeyboardComponent::getYForNote(int note) const {
    const float keyHeight = static_cast<float>(getHeight()) / (highestNote - lowestNote + 1);
    return static_cast<int>((highestNote - note) * keyHeight);
}

void PianoRollComponent::KeyboardComponent::drawWhiteKey(juce::Graphics& g,
                                                       int note,
                                                       bool isPlaying) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    const int y = getYForNote(note);
    const float keyHeight = static_cast<float>(getHeight()) / (highestNote - lowestNote + 1);
    
    // Draw key background
    g.setColour(isPlaying ? lf.getWhiteKeyDownColour() : lf.getWhiteKeyColour());
    g.fillRect(0, y, keyWidth, static_cast<int>(keyHeight));
    
    // Draw key border
    g.setColour(lf.getKeyBorderColour());
    g.drawRect(0, y, keyWidth, static_cast<int>(keyHeight));
    
    // Draw note name for C notes
    if (note % 12 == 0) {
        g.setColour(lf.getKeyTextColour());
        g.setFont(11.0f);
        g.drawText("C" + juce::String(note / 12 - 1),
                  2, y + 2, keyWidth - 4, static_cast<int>(keyHeight) - 4,
                  juce::Justification::bottomLeft);
    }
}

void PianoRollComponent::KeyboardComponent::drawBlackKey(juce::Graphics& g,
                                                       int note,
                                                       bool isPlaying) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    const int y = getYForNote(note);
    const float keyHeight = static_cast<float>(getHeight()) / (highestNote - lowestNote + 1);
    
    // Draw key background
    g.setColour(isPlaying ? lf.getBlackKeyDownColour() : lf.getBlackKeyColour());
    g.fillRect(0, y, static_cast<int>(keyWidth * 0.6f), static_cast<int>(keyHeight));
    
    // Draw key border
    g.setColour(lf.getKeyBorderColour());
    g.drawRect(0, y, static_cast<int>(keyWidth * 0.6f), static_cast<int>(keyHeight));
}

bool PianoRollComponent::KeyboardComponent::isBlackKey(int note) const {
    static const bool blackKeys[] = { false, true, false, true, false, false, true, false, true, false, true, false };
    return blackKeys[note % 12];
}

//==============================================================================
// GridComponent Implementation
//==============================================================================

PianoRollComponent::GridComponent::GridComponent(PianoRollComponent& owner)
    : owner(owner) {
    setWantsKeyboardFocus(true);
}

void PianoRollComponent::GridComponent::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    
    // Draw background
    g.fillAll(lf.getPianoRollBackground());
    
    // Draw horizontal grid lines (note divisions)
    g.setColour(lf.getPianoRollGrid());
    for (int note = owner.keyboard->getNoteForY(0); note <= owner.keyboard->getNoteForY(getHeight()); ++note) {
        const int y = owner.keyboard->getYForNote(note);
        g.drawHorizontalLine(y, 0.0f, static_cast<float>(getWidth()));
    }
    
    // Draw vertical grid lines (beat divisions)
    const double pixelsPerBeat = getWidth() / (timeEnd - timeStart);
    for (double beat = timeStart; beat <= timeEnd; beat += gridSize) {
        const int x = static_cast<int>((beat - timeStart) * pixelsPerBeat);
        g.setColour((static_cast<int>(beat) % 4) == 0 ? lf.getPianoRollBarLine() : lf.getPianoRollGrid());
        g.drawVerticalLine(x, 0.0f, static_cast<float>(getHeight()));
    }
    
    // Draw notes
    for (const auto& noteRect : noteRects) {
        g.setColour(noteRect.selected ? lf.getSelectedNoteColour() : lf.getNoteColour());
        g.fillRect(noteRect.bounds);
        g.setColour(lf.getNoteBorderColour());
        g.drawRect(noteRect.bounds);
    }
    
    // Draw selection rectangle
    if (dragging) {
        g.setColour(lf.getSelectionRectColour());
        g.drawRect(selectionRect);
    }
}

void PianoRollComponent::GridComponent::resized() {
    updateNoteRects();
}

void PianoRollComponent::GridComponent::mouseDown(const juce::MouseEvent& e) {
    dragging = true;
    dragStart = e.getPosition();
    selectionRect = juce::Rectangle<int>(dragStart.x, dragStart.y, 0, 0);
    
    // Check if clicking on a note
    for (size_t i = 0; i < noteRects.size(); ++i) {
        if (noteRects[i].bounds.contains(e.getPosition())) {
            if (!e.mods.isShiftDown()) {
                // Deselect all other notes if shift is not held
                for (auto& noteRect : noteRects) {
                    noteRect.selected = false;
                }
            }
            noteRects[i].selected = true;
            repaint();
            return;
        }
    }
    
    // Start selection rectangle if not clicking on a note
    if (!e.mods.isShiftDown()) {
        for (auto& noteRect : noteRects) {
            noteRect.selected = false;
        }
    }
    repaint();
}

void PianoRollComponent::GridComponent::mouseDrag(const juce::MouseEvent& e) {
    if (dragging) {
        selectionRect.setWidth(e.x - dragStart.x);
        selectionRect.setHeight(e.y - dragStart.y);
        selectNotesInRect(selectionRect, e.mods.isShiftDown());
        repaint();
    }
}

void PianoRollComponent::GridComponent::mouseUp(const juce::MouseEvent&) {
    dragging = false;
    repaint();
}

void PianoRollComponent::GridComponent::mouseDoubleClick(const juce::MouseEvent& e) {
    // Add new note at double click position
    const double time = xToTime(e.x);
    const int note = yToNote(e.y);
    
    if (auto* clip = owner.getClip()) {
        owner.addNote(note, time, gridSize, 100);
    }
}

void PianoRollComponent::GridComponent::setTimeRange(double start, double end) {
    timeStart = start;
    timeEnd = end;
    updateNoteRects();
    repaint();
}

void PianoRollComponent::GridComponent::setKeyRange(int lowest, int highest) {
    // Update via owner's keyboard component
    owner.keyboard->setKeyRange(lowest, highest);
    updateNoteRects();
    repaint();
}

void PianoRollComponent::GridComponent::setGridSize(double beats) {
    gridSize = beats;
    repaint();
}

void PianoRollComponent::GridComponent::setSnapToGrid(bool shouldSnap) {
    snapToGrid = shouldSnap;
}

double PianoRollComponent::GridComponent::timeToX(double time) const {
    return (time - timeStart) * getWidth() / (timeEnd - timeStart);
}

double PianoRollComponent::GridComponent::xToTime(int x) const {
    const double time = timeStart + (x * (timeEnd - timeStart)) / getWidth();
    if (snapToGrid) {
        return std::round(time / gridSize) * gridSize;
    }
    return time;
}

int PianoRollComponent::GridComponent::yToNote(int y) const {
    return owner.keyboard->getNoteForY(y);
}

void PianoRollComponent::GridComponent::updateNoteRects() {
    noteRects.clear();
    
    if (auto* clip = owner.getClip()) {
        // TODO: Create note rectangles from MIDI clip data
    }
}

void PianoRollComponent::GridComponent::selectNotesInRect(const juce::Rectangle<int>& rect,
                                                        bool addToSelection) {
    if (!addToSelection) {
        for (auto& noteRect : noteRects) {
            noteRect.selected = false;
        }
    }
    
    for (auto& noteRect : noteRects) {
        if (rect.intersects(noteRect.bounds)) {
            noteRect.selected = true;
        }
    }
}

//==============================================================================
// VelocityComponent Implementation
//==============================================================================

PianoRollComponent::VelocityComponent::VelocityComponent(PianoRollComponent& owner)
    : owner(owner) {
    setWantsKeyboardFocus(true);
}

void PianoRollComponent::VelocityComponent::paint(juce::Graphics& g) {
    auto& lf = dynamic_cast<CustomLookAndFeel&>(getLookAndFeel());
    
    // Draw background
    g.fillAll(lf.getVelocityEditorBackground());
    
    // Draw grid lines
    g.setColour(lf.getVelocityEditorGrid());
    for (int v = 0; v <= 127; v += 16) {
        const float y = static_cast<float>(getHeight() * (127 - v) / 127);
        g.drawHorizontalLine(static_cast<int>(y), 0.0f, static_cast<float>(getWidth()));
    }
    
    // Draw velocity bars
    for (const auto& bar : velocityBars) {
        g.setColour(bar.selected ? lf.getSelectedVelocityColour() : lf.getVelocityColour());
        g.fillRect(bar.bounds);
        g.setColour(lf.getVelocityBorderColour());
        g.drawRect(bar.bounds);
    }
}

void PianoRollComponent::VelocityComponent::resized() {
    updateVelocityBars();
}

void PianoRollComponent::VelocityComponent::mouseDown(const juce::MouseEvent& e) {
    draggedBarIndex = findBarAt(e.getPosition());
    if (draggedBarIndex >= 0) {
        dragging = true;
        if (!e.mods.isShiftDown()) {
            // Deselect all other bars if shift is not held
            for (auto& bar : velocityBars) {
                bar.selected = false;
            }
        }
        velocityBars[draggedBarIndex].selected = true;
        repaint();
    }
}

void PianoRollComponent::VelocityComponent::mouseDrag(const juce::MouseEvent& e) {
    if (dragging && draggedBarIndex >= 0) {
        const int velocity = juce::jlimit(0, 127,
            127 - (e.y * 127 / getHeight()));
        velocityBars[draggedBarIndex].velocity = velocity;
        updateVelocityBars();
        repaint();
    }
}

void PianoRollComponent::VelocityComponent::mouseUp(const juce::MouseEvent&) {
    dragging = false;
    draggedBarIndex = -1;
}

void PianoRollComponent::VelocityComponent::setTimeRange(double start, double end) {
    timeStart = start;
    timeEnd = end;
    updateVelocityBars();
    repaint();
}

double PianoRollComponent::VelocityComponent::timeToX(double time) const {
    return (time - timeStart) * getWidth() / (timeEnd - timeStart);
}

void PianoRollComponent::VelocityComponent::updateVelocityBars() {
    velocityBars.clear();
    
    if (auto* clip = owner.getClip()) {
        // TODO: Create velocity bars from MIDI clip data
    }
}

int PianoRollComponent::VelocityComponent::findBarAt(juce::Point<int> position) const {
    for (size_t i = 0; i < velocityBars.size(); ++i) {
        if (velocityBars[i].bounds.contains(position)) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

//==============================================================================
// PianoRollComponent Implementation
//==============================================================================

PianoRollComponent::PianoRollComponent() {
    keyboard = std::make_unique<KeyboardComponent>();
    grid = std::make_unique<GridComponent>(*this);
    velocityEditor = std::make_unique<VelocityComponent>(*this);
    
    addAndMakeVisible(keyboard.get());
    addAndMakeVisible(gridViewport);
    addAndMakeVisible(velocityViewport);
    
    gridViewport.setViewedComponent(grid.get(), false);
    velocityViewport.setViewedComponent(velocityEditor.get(), false);
    
    // Synchronize horizontal scrolling
    gridViewport.getHorizontalScrollBar().addListener(this);
    velocityViewport.getHorizontalScrollBar().addListener(this);
}

PianoRollComponent::~PianoRollComponent() {
    if (currentClip != nullptr) {
        currentClip->removeChangeListener(this);
    }
}

void PianoRollComponent::paint(juce::Graphics& g) {
    g.fillAll(getLookAndFeel().findColour(juce::ResizableWindow::backgroundColourId));
}

void PianoRollComponent::resized() {
    auto bounds = getLocalBounds();
    
    // Layout keyboard
    keyboard->setBounds(bounds.removeFromLeft(keyboardWidth));
    
    // Layout velocity editor at bottom
    velocityViewport.setBounds(bounds.removeFromBottom(velocityHeight));
    velocityEditor->setBounds(0, 0,
        static_cast<int>((timeEnd - timeStart) * 100), // 100 pixels per second
        velocityHeight);
    
    // Layout grid
    gridViewport.setBounds(bounds);
    grid->setBounds(0, 0,
        static_cast<int>((timeEnd - timeStart) * 100), // 100 pixels per second
        bounds.getHeight());
}

void PianoRollComponent::changeListenerCallback(juce::ChangeBroadcaster* source) {
    if (source == currentClip) {
        updateComponents();
    }
}

void PianoRollComponent::setClip(MIDIClip* clip) {
    if (currentClip == clip) {
        return;
    }

    if (currentClip != nullptr) {
        currentClip->removeChangeListener(this);
    }
    
    currentClip = clip;
    
    if (currentClip != nullptr) {
        currentClip->addChangeListener(this);
    }
    
    updateComponents();
}

void PianoRollComponent::setTimeRange(double start, double end) {
    timeStart = start;
    timeEnd = end;
    updateComponents();
}

void PianoRollComponent::setVisibleRange(double start, double end) {
    visibleTimeStart = start;
    visibleTimeEnd = end;
    updateComponents();
}

void PianoRollComponent::setGridSize(double beats) {
    gridSize = beats;
    grid->setGridSize(beats);
}

void PianoRollComponent::setSnapToGrid(bool shouldSnap) {
    snapToGrid = shouldSnap;
    grid->setSnapToGrid(shouldSnap);
}

void PianoRollComponent::addNote(int noteNumber, double startTime,
                               double duration, int velocity) {
    if (currentClip != nullptr) {
        currentClip->addNote(noteNumber, velocity, startTime, duration);
    }
}

void PianoRollComponent::removeNote(int noteNumber, double startTime) {
    if (currentClip != nullptr) {
        currentClip->removeNote(noteNumber, startTime);
    }
}

void PianoRollComponent::updateNote(int noteNumber, double startTime,
                                  double newStartTime, double newDuration,
                                  int newVelocity) {
    if (currentClip != nullptr) {
        // TODO: Implement note updating
    }
}

void PianoRollComponent::selectNote(int noteNumber, double startTime,
                                  bool deselectOthers) {
    // TODO: Implement note selection
}

void PianoRollComponent::selectNotesInRange(double startTime, double endTime,
                                          int lowestNote, int highestNote,
                                          bool addToSelection) {
    // TODO: Implement range selection
}

void PianoRollComponent::clearNoteSelection() {
    // TODO: Implement selection clearing
}

void PianoRollComponent::deleteSelectedNotes() {
    // TODO: Implement note deletion
}

void PianoRollComponent::transposeSelectedNotes(int semitones) {
    // TODO: Implement note transposition
}

void PianoRollComponent::moveSelectedNotes(double timeDelta) {
    // TODO: Implement note moving
}

void PianoRollComponent::resizeSelectedNotes(double durationMultiplier) {
    // TODO: Implement note resizing
}

void PianoRollComponent::setSelectedNotesVelocity(int velocity) {
    // TODO: Implement velocity setting
}

void PianoRollComponent::quantizeSelectedNotes(double gridSize) {
    // TODO: Implement note quantization
}

void PianoRollComponent::quantizeSelectedNotesStart(double gridSize) {
    // TODO: Implement start time quantization
}

void PianoRollComponent::quantizeSelectedNotesEnd(double gridSize) {
    // TODO: Implement end time quantization
}

void PianoRollComponent::quantizeSelectedNotesDuration(double gridSize) {
    // TODO: Implement duration quantization
}

void PianoRollComponent::updateComponents() {
    grid->setTimeRange(timeStart, timeEnd);
    velocityEditor->setTimeRange(timeStart, timeEnd);
    resized();
}

void PianoRollComponent::synchronizeViewports() {
    // Synchronize horizontal scrolling
    auto& gridScroll = gridViewport.getHorizontalScrollBar();
    auto& velocityScroll = velocityViewport.getHorizontalScrollBar();
    
    if (gridScroll.getCurrentRangeStart() != velocityScroll.getCurrentRangeStart()) {
        velocityScroll.setCurrentRangeStart(gridScroll.getCurrentRangeStart());
    }
}