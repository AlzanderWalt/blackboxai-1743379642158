#include "CustomLookAndFeel.h"

CustomLookAndFeel::CustomLookAndFeel() {
    initializePalettes();
    setColour(juce::ResizableWindow::backgroundColourId, getWindowBackgroundColour());
}

void CustomLookAndFeel::initializePalettes() {
    // Light theme palette
    lightPalette.background = juce::Colour(0xFFF5F5F5);
    lightPalette.foreground = juce::Colour(0xFF2C2C2C);
    lightPalette.highlight = juce::Colour(0xFFE0E0E0);
    lightPalette.shadow = juce::Colour(0xFFAAAAAA);
    lightPalette.accent = juce::Colour(0xFF007AFF);
    lightPalette.error = juce::Colour(0xFFFF3B30);
    lightPalette.warning = juce::Colour(0xFFFF9500);
    lightPalette.success = juce::Colour(0xFF34C759);

    // Dark theme palette
    darkPalette.background = juce::Colour(0xFF1C1C1E);
    darkPalette.foreground = juce::Colour(0xFFF5F5F5);
    darkPalette.highlight = juce::Colour(0xFF2C2C2E);
    darkPalette.shadow = juce::Colour(0xFF000000);
    darkPalette.accent = juce::Colour(0xFF0A84FF);
    darkPalette.error = juce::Colour(0xFFFF453A);
    darkPalette.warning = juce::Colour(0xFFFF9F0A);
    darkPalette.success = juce::Colour(0xFF30D158);

    accentColor = getCurrentPalette().accent;
}

const CustomLookAndFeel::ColorPalette& CustomLookAndFeel::getCurrentPalette() const {
    return darkMode ? darkPalette : lightPalette;
}

void CustomLookAndFeel::setDarkMode(bool dark) {
    if (darkMode != dark) {
        darkMode = dark;
        setColour(juce::ResizableWindow::backgroundColourId, getWindowBackgroundColour());
    }
}

void CustomLookAndFeel::setAccentColor(juce::Colour color) {
    accentColor = color;
}

juce::Colour CustomLookAndFeel::getWindowBackgroundColour() const {
    return getCurrentPalette().background;
}

juce::Colour CustomLookAndFeel::getDialogBackgroundColour() const {
    return getCurrentPalette().background.brighter(0.1f);
}

juce::Colour CustomLookAndFeel::getTextColour() const {
    return getCurrentPalette().foreground;
}

juce::Colour CustomLookAndFeel::getDisabledTextColour() const {
    return getCurrentPalette().foreground.withAlpha(0.4f);
}

juce::Colour CustomLookAndFeel::getHighlightedTextColour() const {
    return getCurrentPalette().background;
}

juce::Colour CustomLookAndFeel::getButtonBackgroundColour() const {
    return getCurrentPalette().highlight;
}

juce::Colour CustomLookAndFeel::getButtonHoverColour() const {
    return getCurrentPalette().highlight.brighter(0.1f);
}

juce::Colour CustomLookAndFeel::getButtonDownColour() const {
    return getCurrentPalette().accent;
}

juce::Colour CustomLookAndFeel::getToggleButtonBackgroundColour(bool isToggled) const {
    return isToggled ? getCurrentPalette().accent : getCurrentPalette().highlight;
}

juce::Colour CustomLookAndFeel::getSliderBackgroundColour() const {
    return getCurrentPalette().highlight;
}

juce::Colour CustomLookAndFeel::getSliderThumbColour() const {
    return getCurrentPalette().accent;
}

juce::Colour CustomLookAndFeel::getSliderTrackColour() const {
    return getCurrentPalette().highlight.darker(0.2f);
}

juce::Colour CustomLookAndFeel::getChannelStripBackground() const {
    return getCurrentPalette().background.brighter(0.05f);
}

juce::Colour CustomLookAndFeel::getChannelStripBorder() const {
    return getCurrentPalette().shadow;
}

juce::Colour CustomLookAndFeel::getBusStripBackground() const {
    return getCurrentPalette().background.brighter(0.1f);
}

juce::Colour CustomLookAndFeel::getBusStripBorder() const {
    return getCurrentPalette().shadow;
}

juce::Colour CustomLookAndFeel::getMasterStripBackground() const {
    return getCurrentPalette().background.brighter(0.15f);
}

juce::Colour CustomLookAndFeel::getMasterStripBorder() const {
    return getCurrentPalette().accent.withAlpha(0.5f);
}

juce::Colour CustomLookAndFeel::getMeterBackground() const {
    return getCurrentPalette().background.darker(0.2f);
}

juce::Colour CustomLookAndFeel::getMeterRMSColour() const {
    return getCurrentPalette().accent;
}

juce::Colour CustomLookAndFeel::getMeterPeakColour() const {
    return getCurrentPalette().warning;
}

juce::Colour CustomLookAndFeel::getTrackHeaderBackground() const {
    return getCurrentPalette().background.brighter(0.05f);
}

juce::Colour CustomLookAndFeel::getTrackHeaderBorder() const {
    return getCurrentPalette().shadow;
}

juce::Colour CustomLookAndFeel::getTrackContentBackground() const {
    return getCurrentPalette().background;
}

juce::Colour CustomLookAndFeel::getTrackContentGrid() const {
    return getCurrentPalette().foreground.withAlpha(0.1f);
}

juce::Colour CustomLookAndFeel::getTrackContentBorder() const {
    return getCurrentPalette().shadow;
}

juce::Colour CustomLookAndFeel::getClipBackground() const {
    return getCurrentPalette().accent.withAlpha(0.7f);
}

juce::Colour CustomLookAndFeel::getClipBorder() const {
    return getCurrentPalette().accent;
}

juce::Colour CustomLookAndFeel::getSelectedClipBackground() const {
    return getCurrentPalette().accent;
}

juce::Colour CustomLookAndFeel::getSelectedClipBorder() const {
    return getCurrentPalette().accent.brighter(0.2f);
}

juce::Colour CustomLookAndFeel::getPianoRollBackground() const {
    return getCurrentPalette().background;
}

juce::Colour CustomLookAndFeel::getPianoRollGrid() const {
    return getCurrentPalette().foreground.withAlpha(0.1f);
}

juce::Colour CustomLookAndFeel::getPianoRollBarLine() const {
    return getCurrentPalette().foreground.withAlpha(0.2f);
}

juce::Colour CustomLookAndFeel::getWhiteKeyColour() const {
    return getCurrentPalette().background.brighter(0.2f);
}

juce::Colour CustomLookAndFeel::getBlackKeyColour() const {
    return getCurrentPalette().background.darker(0.2f);
}

juce::Colour CustomLookAndFeel::getWhiteKeyDownColour() const {
    return getCurrentPalette().accent.withAlpha(0.7f);
}

juce::Colour CustomLookAndFeel::getBlackKeyDownColour() const {
    return getCurrentPalette().accent.withAlpha(0.8f);
}

juce::Colour CustomLookAndFeel::getKeyBorderColour() const {
    return getCurrentPalette().shadow;
}

juce::Colour CustomLookAndFeel::getKeyTextColour() const {
    return getCurrentPalette().foreground.withAlpha(0.7f);
}

juce::Colour CustomLookAndFeel::getNoteColour() const {
    return getCurrentPalette().accent.withAlpha(0.7f);
}

juce::Colour CustomLookAndFeel::getSelectedNoteColour() const {
    return getCurrentPalette().accent;
}

juce::Colour CustomLookAndFeel::getNoteBorderColour() const {
    return getCurrentPalette().accent.brighter(0.2f);
}

juce::Colour CustomLookAndFeel::getSelectionRectColour() const {
    return getCurrentPalette().accent.withAlpha(0.3f);
}

juce::Colour CustomLookAndFeel::getTransportBackground() const {
    return getCurrentPalette().background.brighter(0.1f);
}

juce::Colour CustomLookAndFeel::getTransportBorder() const {
    return getCurrentPalette().shadow;
}

juce::Colour CustomLookAndFeel::getTransportTextColour() const {
    return getCurrentPalette().foreground;
}

juce::Colour CustomLookAndFeel::getVelocityEditorBackground() const {
    return getCurrentPalette().background;
}

juce::Colour CustomLookAndFeel::getVelocityEditorGrid() const {
    return getCurrentPalette().foreground.withAlpha(0.1f);
}

juce::Colour CustomLookAndFeel::getVelocityColour() const {
    return getCurrentPalette().accent.withAlpha(0.7f);
}

juce::Colour CustomLookAndFeel::getSelectedVelocityColour() const {
    return getCurrentPalette().accent;
}

juce::Colour CustomLookAndFeel::getVelocityBorderColour() const {
    return getCurrentPalette().accent.brighter(0.2f);
}

void CustomLookAndFeel::drawRotarySlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider& slider) {
    const auto outline = slider.findColour(juce::Slider::rotarySliderOutlineColourId);
    const auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);
    const auto thumb = slider.findColour(juce::Slider::thumbColourId);

    const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    const auto radius = juce::jmin(bounds.getWidth(), bounds.getHeight()) * 0.5f;
    const auto toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto lineW = radius * 0.1f;
    const auto arcRadius = radius - lineW * 0.5f;

    // Draw outline
    g.setColour(outline);
    g.drawEllipse(bounds.reduced(lineW * 0.5f), lineW);

    // Draw value arc
    juce::Path valueArc;
    valueArc.addArc(bounds.getX() + lineW, bounds.getY() + lineW,
                    bounds.getWidth() - lineW * 2.0f, bounds.getHeight() - lineW * 2.0f,
                    rotaryStartAngle, toAngle, true);
    g.setColour(fill);
    g.strokePath(valueArc, juce::PathStrokeType(lineW, juce::PathStrokeType::curved, juce::PathStrokeType::rounded));

    // Draw thumb
    const auto thumbWidth = lineW * 2.0f;
    const auto thumbAngle = toAngle - juce::MathConstants<float>::halfPi;
    const juce::Point<float> thumbPoint(bounds.getCentreX() + arcRadius * std::cos(thumbAngle),
                                      bounds.getCentreY() + arcRadius * std::sin(thumbAngle));
    g.setColour(thumb);
    g.fillEllipse(juce::Rectangle<float>(thumbWidth, thumbWidth).withCentre(thumbPoint));
}

void CustomLookAndFeel::drawLinearSlider(juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float minSliderPos, float maxSliderPos,
                                        const juce::Slider::SliderStyle style, juce::Slider& slider) {
    const auto bounds = juce::Rectangle<int>(x, y, width, height).toFloat();
    const auto track = slider.findColour(juce::Slider::trackColourId);
    const auto fill = slider.findColour(juce::Slider::rotarySliderFillColourId);
    const auto thumb = slider.findColour(juce::Slider::thumbColourId);

    if (style == juce::Slider::LinearVertical) {
        const auto trackWidth = juce::jmin(6.0f, bounds.getWidth() * 0.25f);
        const auto trackX = bounds.getCentreX() - trackWidth * 0.5f;
        const auto thumbHeight = trackWidth * 1.5f;

        // Draw track
        g.setColour(track);
        g.fillRoundedRectangle(trackX, bounds.getY(), trackWidth, bounds.getHeight(), trackWidth * 0.5f);

        // Draw fill
        g.setColour(fill);
        g.fillRoundedRectangle(trackX, sliderPos, trackWidth, bounds.getBottom() - sliderPos, trackWidth * 0.5f);

        // Draw thumb
        g.setColour(thumb);
        g.fillRoundedRectangle(trackX - trackWidth * 0.25f, sliderPos - thumbHeight * 0.5f,
                              trackWidth * 1.5f, thumbHeight, thumbHeight * 0.5f);
    } else {
        const auto trackHeight = juce::jmin(6.0f, bounds.getHeight() * 0.25f);
        const auto trackY = bounds.getCentreY() - trackHeight * 0.5f;
        const auto thumbWidth = trackHeight * 1.5f;

        // Draw track
        g.setColour(track);
        g.fillRoundedRectangle(bounds.getX(), trackY, bounds.getWidth(), trackHeight, trackHeight * 0.5f);

        // Draw fill
        g.setColour(fill);
        g.fillRoundedRectangle(bounds.getX(), trackY, sliderPos - bounds.getX(), trackHeight, trackHeight * 0.5f);

        // Draw thumb
        g.setColour(thumb);
        g.fillRoundedRectangle(sliderPos - thumbWidth * 0.5f, trackY - trackHeight * 0.25f,
                              thumbWidth, trackHeight * 1.5f, thumbWidth * 0.5f);
    }
}

void CustomLookAndFeel::drawButtonBackground(juce::Graphics& g, juce::Button& button,
                                           const juce::Colour& backgroundColour,
                                           bool shouldDrawButtonAsHighlighted,
                                           bool shouldDrawButtonAsDown) {
    const auto bounds = button.getLocalBounds().toFloat();
    const auto cornerSize = 4.0f;
    
    if (shouldDrawButtonAsDown) {
        g.setColour(getButtonDownColour());
    } else if (shouldDrawButtonAsHighlighted) {
        g.setColour(getButtonHoverColour());
    } else {
        g.setColour(backgroundColour);
    }
    
    g.fillRoundedRectangle(bounds, cornerSize);
}

void CustomLookAndFeel::drawToggleButton(juce::Graphics& g, juce::ToggleButton& button,
                                        bool shouldDrawButtonAsHighlighted,
                                        bool shouldDrawButtonAsDown) {
    const auto bounds = button.getLocalBounds().toFloat();
    const auto cornerSize = 4.0f;
    
    g.setColour(getToggleButtonBackgroundColour(button.getToggleState()));
    g.fillRoundedRectangle(bounds, cornerSize);
    
    if (shouldDrawButtonAsHighlighted) {
        g.setColour(getCurrentPalette().foreground.withAlpha(0.1f));
        g.fillRoundedRectangle(bounds, cornerSize);
    }
}

void CustomLookAndFeel::drawComboBox(juce::Graphics& g, int width, int height,
                                    bool isButtonDown, int buttonX, int buttonY,
                                    int buttonW, int buttonH, juce::ComboBox& box) {
    const auto cornerSize = 4.0f;
    const auto bounds = juce::Rectangle<int>(0, 0, width, height).toFloat();
    
    g.setColour(box.findColour(juce::ComboBox::backgroundColourId));
    g.fillRoundedRectangle(bounds, cornerSize);
    
    g.setColour(box.findColour(juce::ComboBox::outlineColourId));
    g.drawRoundedRectangle(bounds, cornerSize, 1.0f);
    
    const auto arrowZone = juce::Rectangle<float>(buttonX, buttonY, buttonW, buttonH);
    const auto arrowColour = box.findColour(juce::ComboBox::arrowColourId);
    
    if (box.isEnabled()) {
        juce::Path arrow;
        arrow.startNewSubPath(arrowZone.getX() + arrowZone.getWidth() * 0.3f, arrowZone.getCentreY() - 2.0f);
        arrow.lineTo(arrowZone.getCentreX(), arrowZone.getCentreY() + 2.0f);
        arrow.lineTo(arrowZone.getRight() - arrowZone.getWidth() * 0.3f, arrowZone.getCentreY() - 2.0f);
        
        g.setColour(arrowColour);
        g.strokePath(arrow, juce::PathStrokeType(1.0f));
    }
}

void CustomLookAndFeel::drawPopupMenuItem(juce::Graphics& g, const juce::Rectangle<int>& area,
                                         bool isSeparator, bool isActive, bool isHighlighted,
                                         bool isTicked, bool hasSubMenu, const juce::String& text,
                                         const juce::String& shortcutKeyText,
                                         const juce::Drawable* icon,
                                         const juce::Colour* textColour) {
    if (isSeparator) {
        const auto separatorHeight = 1.0f;
        g.setColour(getCurrentPalette().shadow);
        g.fillRect(area.reduced(5, area.getHeight() / 2 - static_cast<int>(separatorHeight / 2.0f),
                              5, area.getHeight() / 2 + static_cast<int>(separatorHeight / 2.0f)));
    } else {
        const auto textColourToUse = textColour != nullptr ? *textColour :
            (isActive ? getCurrentPalette().foreground : getDisabledTextColour());
        
        if (isHighlighted && isActive) {
            g.setColour(getCurrentPalette().accent.withAlpha(0.2f));
            g.fillRect(area);
        }
        
        if (isTicked) {
            const auto tickWidth = 8.0f;
            const auto tickHeight = 8.0f;
            const auto tickX = 5.0f;
            const auto tickY = area.getCentreY() - tickHeight * 0.5f;
            
            juce::Path tick;
            tick.startNewSubPath(tickX, tickY + tickHeight * 0.5f);
            tick.lineTo(tickX + tickWidth * 0.3f, tickY + tickHeight);
            tick.lineTo(tickX + tickWidth, tickY);
            
            g.setColour(getCurrentPalette().accent);
            g.strokePath(tick, juce::PathStrokeType(2.0f));
        }
        
        g.setColour(textColourToUse);
        g.setFont(getPopupMenuFont());
        
        const auto maxTextWidth = area.getWidth() - (hasSubMenu ? 20 : 0) -
                                (shortcutKeyText.isNotEmpty() ? 50 : 0);
        g.drawFittedText(text, area.getX() + 20, area.getY(), maxTextWidth, area.getHeight(),
                        juce::Justification::centredLeft, 1);
        
        if (shortcutKeyText.isNotEmpty()) {
            g.setFont(getPopupMenuFont().withHeight(12.0f));
            g.drawText(shortcutKeyText, area.getRight() - 50, area.getY(),
                      45, area.getHeight(), juce::Justification::centredRight);
        }
        
        if (hasSubMenu) {
            const auto arrowX = area.getRight() - 16;
            const auto arrowY = area.getCentreY();
            
            juce::Path arrow;
            arrow.startNewSubPath(arrowX, arrowY - 3.0f);
            arrow.lineTo(arrowX + 3.0f, arrowY);
            arrow.lineTo(arrowX, arrowY + 3.0f);
            
            g.setColour(textColourToUse);
            g.strokePath(arrow, juce::PathStrokeType(1.0f));
        }
    }
}

juce::Font CustomLookAndFeel::getTextButtonFont(juce::TextButton&, int buttonHeight) {
    return juce::Font(juce::jmin(16.0f, buttonHeight * 0.6f));
}

juce::Font CustomLookAndFeel::getComboBoxFont(juce::ComboBox&) {
    return juce::Font(14.0f);
}

juce::Font CustomLookAndFeel::getLabelFont(juce::Label&) {
    return juce::Font(14.0f);
}

juce::Font CustomLookAndFeel::getPopupMenuFont() {
    return juce::Font(14.0f);
}

juce::Font CustomLookAndFeel::getSliderPopupFont(juce::Slider&) {
    return juce::Font(14.0f);
}