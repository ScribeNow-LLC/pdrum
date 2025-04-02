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
    explicit VibratingMembrane(VibratingMembraneModel &membraneModel)
        : membraneModel(membraneModel)
    {
        startTimerHz(60);
    }

    /**
     * @brief Paint the component.
     * @param g The graphics context used for painting.
     */
    void paint(juce::Graphics &g) override {
        const int gridResolution = membraneModel.getGridResolution();
        // Use const references to avoid copying large buffers.
        const auto& isInside = membraneModel.getIsInsideMask();
        const auto& current  = membraneModel.getCurrentBuffer();

        const auto bounds = getLocalBounds().toFloat();
        const float side = std::min(bounds.getWidth(), bounds.getHeight());
        const float offsetX = (bounds.getWidth() - side) * 0.5f;
        const float offsetY = (bounds.getHeight() - side) * 0.5f;
        const juce::Rectangle<float> squareBounds(offsetX, offsetY, side, side);
        const float cellWidth = squareBounds.getWidth() / gridResolution;
        const float cellHeight = squareBounds.getHeight() / gridResolution;
        const float halfCellWidth = cellWidth * 0.5f;
        const float logDenom = std::log10(101.0f);  // Precompute constant for logarithmic scaling

        // Loop over each grid cell.
        for (int y = 0; y < gridResolution; ++y) {
            for (int x = 0; x < gridResolution; ++x) {
                const int index = y * gridResolution + x;
                if (!isInside[index])
                    continue;

                const float value = current[index];
                // Logarithmic scaling calculation.
                const float logValue = std::log10(1.0f + std::abs(value) * 100.0f) / logDenom;
                const float scaled = juce::jlimit(0.0f, 1.0f, logValue);
                const juce::Colour cellColour = (value >= 0.0f)
                    ? juce::Colour::fromFloatRGBA(0.0f, scaled, 0.0f, 1.0f)
                    : juce::Colour::fromFloatRGBA(scaled, 0.0f, 0.0f, 1.0f);
                g.setColour(cellColour);

                // Calculate cell position.
                const float cellX = squareBounds.getX() + x * cellWidth - halfCellWidth;
                const float cellY = squareBounds.getY() + y * cellHeight - halfCellWidth;
                g.fillRect(cellX, cellY, cellWidth, cellHeight);
            }
        }
        // Draw border and ellipse once.
        g.setColour(juce::Colours::white);
        g.drawRect(bounds, 2.0f);
        g.drawEllipse(squareBounds.reduced(8), 10.0f);
    }

    /**
     * @brief Handle mouse down events.
     * @param e The mouse event.
     */
    void mouseDown(const juce::MouseEvent &e) override {
        const int gridResolution = membraneModel.getGridResolution();
        const auto bounds = getLocalBounds().toFloat();
        const float side = std::min(bounds.getWidth(), bounds.getHeight());
        const float offsetX = (bounds.getWidth() - side) * 0.5f;
        const float offsetY = (bounds.getHeight() - side) * 0.5f;
        const juce::Rectangle<float> squareBounds(offsetX, offsetY, side, side);

        const float relativeX = e.x - squareBounds.getX();
        const float relativeY = e.y - squareBounds.getY();
        const int x = static_cast<int>((relativeX / squareBounds.getWidth()) * gridResolution);
        const int y = static_cast<int>((relativeY / squareBounds.getHeight()) * gridResolution);

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
