#ifndef VIBRATING_MEMBRANE_H
#define VIBRATING_MEMBRANE_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>

#include "VibratingMembraneModel.h"

/**
 * @brief Class representing a vibrating membrane simulation with physical dimensions,
 *        optimized for performance by using contiguous memory.
 */
class VibratingMembrane final : public juce::Component, juce::Timer {
public:
    /**
     * @brief Constructor for the VibratingMembrane class.
     */
    explicit
    VibratingMembrane(VibratingMembraneModel &membraneModel) : membraneModel(
        membraneModel) {
        startTimerHz(60);
    }

    /**
     * @brief Paint the component.
     * @param g The graphics context used for painting.
     */
    void paint(juce::Graphics &g) override {
        const auto gridResolution = membraneModel.getGridResolution();
        const auto isInside = membraneModel.getIsInsideMask();
        const std::vector<float> current = membraneModel.getCurrentBuffer();

        const auto bounds = getLocalBounds().toFloat();
        const float side = std::min(bounds.getWidth(), bounds.getHeight());
        const float offsetX = (bounds.getWidth() - side) * 0.5f;
        const float offsetY = (bounds.getHeight() - side) * 0.5f;
        const juce::Rectangle squareBounds(offsetX, offsetY, side, side);
        const float cellWidth = squareBounds.getWidth() / static_cast<float>(
                                    gridResolution);
        const float cellHeight =
                squareBounds.getHeight() / static_cast<float>(gridResolution);
        /// Draw each cell (only if it's inside the circle).
        for (int y = 0; y < gridResolution; ++y) {
            for (int x = 0; x < gridResolution; ++x) {
                const int index = y * gridResolution + x;
                if (!isInside[index])
                    continue;
                const float value = current[index];
                const float logValue = std::log10(
                                           1.0f + std::abs(value) * 100.0f) /
                                       std::log10(101.0f);
                const float scaled = juce::jlimit(0.0f, 1.0f, logValue);
                const juce::Colour cellColour = (value >= 0.0f)
                                                    ? juce::Colour::fromFloatRGBA(
                                                        0.0f, scaled, 0.0f,
                                                        1.0f)
                                                    : juce::Colour::fromFloatRGBA(
                                                        scaled, 0.0f, 0.0f,
                                                        1.0f);
                g.setColour(cellColour);
                const float cellX =
                        squareBounds.getX() + static_cast<float>(x) * cellWidth
                        - cellWidth / 2.0f;
                const float cellY =
                        squareBounds.getY() + static_cast<float>(y) * cellHeight
                        - cellWidth / 2.0f;
                g.fillRect(cellX, cellY, cellWidth, cellHeight);
            }
        }
        g.setColour(juce::Colours::white);
        g.drawRect(getLocalBounds().toFloat(), 2.0f);
        g.drawEllipse(squareBounds.toFloat().reduced(8), 10.0f);
    }

    /**
     * @brief Handle mouse down events.
     * @param e The mouse event.
     */
    void mouseDown(const juce::MouseEvent &e) override {
        const auto gridResolution = membraneModel.getGridResolution();

        const auto bounds = getLocalBounds().toFloat();
        const float side = std::min(bounds.getWidth(), bounds.getHeight());
        const float offsetX = (bounds.getWidth() - side) * 0.5f;
        const float offsetY = (bounds.getHeight() - side) * 0.5f;
        const juce::Rectangle squareBounds(offsetX, offsetY, side, side);

        const float relativeX = static_cast<float>(e.x) - squareBounds.getX();
        const float relativeY = static_cast<float>(e.y) - squareBounds.getY();
        const int x = static_cast<int>(
            (relativeX / squareBounds.getWidth()) * static_cast<float>(
                gridResolution));
        const int y = static_cast<int>(
            (relativeY / squareBounds.getHeight()) * static_cast<float>(
                gridResolution));

        membraneModel.excite(0.9f, x, y);
    }

private:
    /**
     * @brief Timer callback function to update the simulation.
     */
    void timerCallback() override {
        repaint();
    }

    VibratingMembraneModel &membraneModel;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VibratingMembrane)
};

#endif //VIBRATING_MEMBRANE_H
