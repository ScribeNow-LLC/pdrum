#ifndef ADSR_COMPONENT_H
#define ADSR_COMPONENT_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <juce_gui_basics/juce_gui_basics.h>

class ADSRComponent final : public juce::Component, public juce::Timer {
public:
    explicit ADSRComponent(juce::AudioProcessorValueTreeState &state) :
        attackAttachment(state, "attack", attackSlider),
        decayAttachment(state, "decay", decaySlider),
        sustainAttachment(state, "sustain", sustainSlider),
        releaseAttachment(state, "release", releaseSlider) {
        setupSlider(attackSlider, "A");
        setupSlider(decaySlider, "D");
        setupSlider(sustainSlider, "S");
        setupSlider(releaseSlider, "R");
        startTimerHz(60);
    }

    void resized() override {
        auto bounds = getLocalBounds().reduced(10);
        graphArea = bounds.removeFromTop(bounds.getHeight() / 3);
        auto sliderArea = bounds;
        const int sliderWidth = sliderArea.getWidth() / 4;

        for (int i = 0; i < 4; ++i) {
            auto sliderBounds = sliderArea.removeFromLeft(sliderWidth);
            auto *slider = getSliderByIndex(i);
            auto *label = labels[i];
            const int knobHeight = static_cast<int>(static_cast<float>(sliderBounds.getHeight()) * 0.75f);
            const int labelHeight = static_cast<int>(static_cast<float>(sliderBounds.getHeight()) * 0.25f);
            slider->setBounds(sliderBounds.withHeight(knobHeight));
            label->setBounds(
                    sliderBounds.withTop(sliderBounds.getY() + knobHeight)
                            .withHeight(labelHeight));
        }
    }

    void paint(juce::Graphics& g) override {
        g.fillAll(juce::Colours::black);

        g.setColour(juce::Colours::white);
        g.drawRect(graphArea);

        const auto plotArea = graphArea.reduced(10);

        // Get ADSR parameters
        const auto a = static_cast<float>(attackSlider.getValue());
        const auto d = static_cast<float>(decaySlider.getValue());
        const auto s = static_cast<float>(sustainSlider.getValue());
        const auto r = static_cast<float>(releaseSlider.getValue());
        constexpr float holdTime = 0.25f;

        const float envelopeDuration = a + d + holdTime + r + 0.0001f;

        constexpr int numPoints = 256;
        juce::Path env;
        env.startNewSubPath(static_cast<float>(plotArea.getX()),
                            static_cast<float>(plotArea.getBottom()));
        for (int i = 0; i < numPoints; ++i) {
            constexpr float visualDuration = 2.0f;
            const float tNorm = static_cast<float>(i) / (numPoints - 1);
            const float t = tNorm * visualDuration;
            float value = 0.0f;

            if (t <= a) {
                value = t / a; // Attack
            } else if (t <= a + d) {
                value = 1.0f - ((t - a) / d) * (1.0f - s); // Decay
            } else if (t <= a + d + holdTime) {
                value = s; // Sustain
            } else if (t <= envelopeDuration) {
                value = s * (1.0f - (t - a - d - holdTime) / r); // Release
            } else {
                value = 0.0f;
            }
            const float x = static_cast<float>(plotArea.getX()) + tNorm * static_cast<float>(plotArea.getWidth());
            const float y = static_cast<float>(plotArea.getY()) + (1.0f - value) * static_cast<float>(plotArea.getHeight());
            env.lineTo(x, y);
        }
        g.setColour(juce::Colours::green);
        g.strokePath(env, juce::PathStrokeType(2.0f));
    }

    void timerCallback() override { repaint(); }

private:
    void setupSlider(juce::Slider &slider, const juce::String &name) {
        slider.setSliderStyle(juce::Slider::RotaryVerticalDrag);
        slider.setTextBoxStyle(juce::Slider::NoTextBox, false, 40, 20);
        addAndMakeVisible(slider);

        auto *label = new juce::Label();
        labels.add(label);
        label->setText(name, juce::dontSendNotification);
        label->setJustificationType(juce::Justification::centred);
        addAndMakeVisible(label);
    }

    juce::Slider *getSliderByIndex(const int index) {
        switch (index) {
            case 0:
                return &attackSlider;
            case 1:
                return &decaySlider;
            case 2:
                return &sustainSlider;
            case 3:
                return &releaseSlider;
            default:
                return nullptr;
        }
    }

    juce::Slider attackSlider, decaySlider, sustainSlider, releaseSlider;
    juce::OwnedArray<juce::Label> labels;

    juce::AudioProcessorValueTreeState::SliderAttachment attackAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment decayAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment sustainAttachment;
    juce::AudioProcessorValueTreeState::SliderAttachment releaseAttachment;

    juce::Rectangle<int> graphArea;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ADSRComponent)
};


#endif // ADSR_COMPONENT_H
