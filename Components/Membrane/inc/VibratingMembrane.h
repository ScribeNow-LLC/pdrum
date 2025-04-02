#ifndef VIBRATING_MEMBRANE_H
#define VIBRATING_MEMBRANE_H

#include <cassert>
#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include "VibratingMembraneModel.h"

/**
 * @brief Class representing a vibrating membrane simulation with physical
 * dimensions, optimized for performance by using contiguous memory.
 */
class VibratingMembrane final : public juce::Component, juce::Timer {
public:
    /**
     * @brief Constructor for the VibratingMembrane class.
     */
    explicit VibratingMembrane(VibratingMembraneModel &membraneModel);

    /**
     * @brief Paint the component.
     * @param g The graphics context used for painting.
     */
    void paint(juce::Graphics &g) override;

    /**
     * @brief Handle mouse down events.
     * @param e The mouse event.
     */
    void mouseDown(const juce::MouseEvent &e) override;

private:
    /**
     * @brief Timer callback function to update the simulation.
     */
    void timerCallback() override { repaint(); }

    /** A reference to the vibrating membrane model. */
    VibratingMembraneModel &membraneModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VibratingMembrane)
};

#endif // VIBRATING_MEMBRANE_H
