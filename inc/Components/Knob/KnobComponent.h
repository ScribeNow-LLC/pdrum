#ifndef KNOB_COMPONENT_H
#define KNOB_COMPONENT_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_gui_basics/juce_gui_basics.h>

/**
 * @brief KnobComponent class that represents a knob UI component.
 */
class KnobComponent final : public juce::Component {
public:
    /**
     * @brief Constructor for the KnobComponent.
     * @param state A reference to the AudioProcessorValueTreeState object.
     * @param paramID The ID of the parameter to attach to the knob.
     * @param titleText The title text to display above the knob.
     */
    KnobComponent(juce::AudioProcessorValueTreeState &state,
                  const juce::String &paramID, const juce::String &titleText);

    /**
     * @brief Method to resize the component.
     */
    void resized() override;

private:
    /** Slider for the knob */
    juce::Slider slider;

    /** Label for the title */
    juce::Label title;

    /** Attachment for the slider */
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(KnobComponent)
};

#endif // KNOB_COMPONENT_H
