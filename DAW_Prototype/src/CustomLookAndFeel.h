#pragma once
#include <JuceHeader.h>

class CustomLookAndFeel : public juce::LookAndFeel_V4 {
public:
    CustomLookAndFeel();
    ~CustomLookAndFeel() override = default;

    // Color scheme
    void setDarkMode(bool dark);
    bool isDarkMode() const { return darkMode; }
    void setAccentColor(juce::Colour color);
    juce::Colour getAccentColor() const { return accentColor; }

    // Window colors
    juce::Colour getWindowBackgroundColour() const;
    juce::Colour getDialogBackgroundColour() const;
    
    // Text colors
    juce::Colour getTextColour() const;
    juce::Colour getDisabledTextColour() const;
    juce::Colour getHighlightedTextColour() const;
    
    // Control colors
    juce::Colour getButtonBackgroundColour() const;
    juce::Colour getButtonHoverColour() const;
    juce::Colour getButtonDownColour() const;
    juce::Colour getToggleButtonBackgroundColour(bool isToggled) const;
    juce::Colour getSliderBackgroundColour() const;
    juce::Colour getSliderThumbColour() const;
    juce::Colour getSliderTrackColour() const;
    
    // Mixer colors
    juce::Colour getChannelStripBackground() const;
    juce::Colour getChannelStripBorder() const;
    juce::Colour getBusStripBackground() const;
    juce::Colour getBusStripBorder() const;
    juce::Colour getMasterStripBackground() const;
    juce::Colour getMasterStripBorder() const;
    juce::Colour getMeterBackground() const;
    juce::Colour getMeterRMSColour() const;
    juce::Colour getMeterPeakColour() const;
    
    // Track editor colors
    juce::Colour getTrackHeaderBackground() const;
    juce::Colour getTrackHeaderBorder() const;
    juce::Colour getTrackContentBackground() const;
    juce::Colour getTrackContentGrid() const;
    juce::Colour getTrackContentBorder() const;
    juce::Colour getClipBackground() const;
    juce::Colour getClipBorder() const;
    juce::Colour getSelectedClipBackground() const;
    juce::Colour getSelectedClipBorder() const;
    
    // Piano roll colors
    juce::Colour getPianoRollBackground() const;
    juce::Colour getPianoRollGrid() const;
    juce::Colour getPianoRollBarLine() const;
    juce::Colour getWhiteKeyColour() const;
    juce::Colour getBlackKeyColour() const;
    juce::Colour getWhiteKeyDownColour() const;
    juce::Colour getBlackKeyDownColour() const;
    juce::Colour getKeyBorderColour() const;
    juce::Colour getKeyTextColour() const;
    juce::Colour getNoteColour() const;
    juce::Colour getSelectedNoteColour() const;
    juce::Colour getNoteBorderColour() const;
    juce::Colour getSelectionRectColour() const;
    
    // Transport colors
    juce::Colour getTransportBackground() const;
    juce::Colour getTransportBorder() const;
    juce::Colour getTransportTextColour() const;
    
    // Velocity editor colors
    juce::Colour getVelocityEditorBackground() const;
    juce::Colour getVelocityEditorGrid() const;
    juce::Colour getVelocityColour() const;
    juce::Colour getSelectedVelocityColour() const;
    juce::Colour getVelocityBorderColour() const;

    // Component drawing
    void drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                         juce::Slider& slider) override;
                         
    void drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                         float sliderPos, float minSliderPos, float maxSliderPos,
                         const juce::Slider::SliderStyle style, juce::Slider& slider) override;
                         
    void drawButtonBackground(juce::Graphics& g, juce::Button& button,
                            const juce::Colour& backgroundColour,
                            bool shouldDrawButtonAsHighlighted,
                            bool shouldDrawButtonAsDown) override;
                            
    void drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                         bool shouldDrawButtonAsHighlighted,
                         bool shouldDrawButtonAsDown) override;
                         
    void drawComboBox(juce::Graphics& g, int width, int height,
                     bool isButtonDown, int buttonX, int buttonY,
                     int buttonW, int buttonH,
                     juce::ComboBox& box) override;
                     
    void drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                          bool isSeparator, bool isActive, bool isHighlighted,
                          bool isTicked, bool hasSubMenu,
                          const juce::String& text,
                          const juce::String& shortcutKeyText,
                          const juce::Drawable* icon,
                          const juce::Colour* textColour) override;

    // Font settings
    juce::Font getTextButtonFont(juce::TextButton&, int buttonHeight) override;
    juce::Font getComboBoxFont(juce::ComboBox&) override;
    juce::Font getLabelFont(juce::Label&) override;
    juce::Font getPopupMenuFont() override;
    juce::Font getSliderPopupFont(juce::Slider&) override;

private:
    bool darkMode{false};
    juce::Colour accentColor;
    
    // Helper functions
    juce::Colour getBaseColour() const;
    juce::Colour getContrastColour() const;
    juce::Colour getHighlightColour() const;
    juce::Colour getShadowColour() const;
    
    // Color palette
    struct ColorPalette {
        juce::Colour background;
        juce::Colour foreground;
        juce::Colour highlight;
        juce::Colour shadow;
        juce::Colour accent;
        juce::Colour error;
        juce::Colour warning;
        juce::Colour success;
    };
    
    ColorPalette lightPalette;
    ColorPalette darkPalette;
    
    void initializePalettes();
    const ColorPalette& getCurrentPalette() const;
    
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(CustomLookAndFeel)
};