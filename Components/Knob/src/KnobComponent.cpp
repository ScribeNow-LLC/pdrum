#include "KnobComponent.h"

/**
 * @brief Constructor for the KnobComponent.
 * @param state A reference to the AudioProcessorValueTreeState object.
 * @param paramID The ID of the parameter to attach to the knob.
 * @param titleText The title text to display above the knob.
 */
KnobComponent::KnobComponent(juce::AudioProcessorValueTreeState &state,
                             const juce::String &paramID,
                             const juce::String &titleText) :
    attachment(state, paramID, slider) {
    slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
    slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 50, 20);
    slider.setTooltip(titleText);
    addAndMakeVisible(slider);

    title.setText(titleText, juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centred);
    title.setFont(juce::Font(juce::FontOptions(14.0f, juce::Font::bold)));
    title.setInterceptsMouseClicks(false, false);
    // addAndMakeVisible(title);
}

/**
 * @brief Method to resize the component.
 */
void KnobComponent::resized() {
    const auto area = getLocalBounds();
    title.setBounds(area);
    slider.setBounds(area);
}
