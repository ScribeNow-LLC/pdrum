#include "VibratingMembrane.h"

#include <cassert>
#include <cmath>
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>

/**
 * @brief Class representing a vibrating membrane simulation with physical
 * dimensions, optimized for performance by using contiguous memory.
 */
VibratingMembrane::VibratingMembrane(VibratingMembraneModel &membraneModel) :
    membraneModel(membraneModel) {
    startTimerHz(60);
}

/**
 * @brief Paint the component.
 * @param g The graphics context used for painting.
 */
void VibratingMembrane::paint(juce::Graphics &g) {
    const int gridResolution = membraneModel.getGridResolution();
    // Use const references to avoid copying large buffers.
    const auto &isInside = membraneModel.getIsInsideMask();
    const auto &current = membraneModel.getCurrentBuffer();

    const auto bounds = getLocalBounds().toFloat();
    const float side = std::min(bounds.getWidth(), bounds.getHeight());
    const float offsetX = (bounds.getWidth() - side) * 0.5f;
    const float offsetY = (bounds.getHeight() - side) * 0.5f;
    const juce::Rectangle<float> squareBounds(offsetX, offsetY, side, side);
    const float cellWidth =
            squareBounds.getWidth() / static_cast<float>(gridResolution);
    const float cellHeight =
            squareBounds.getHeight() / static_cast<float>(gridResolution);
    const float halfCellWidth = cellWidth * 0.5f;
    const float logDenom =
            std::log10(101.0f); // Precompute constant for logarithmic scaling

    // Loop over each grid cell.
    for (int y = 0; y < gridResolution; ++y) {
        for (int x = 0; x < gridResolution; ++x) {
            const int index = y * gridResolution + x;
            if (!isInside[index])
                continue;

            const float value = current[index];
            // Logarithmic scaling calculation.
            const float logValue =
                    std::log10(1.0f + std::abs(value) * 300.0f) / logDenom;
            const float scaled = juce::jlimit(0.0f, 1.0f, logValue);
            const juce::Colour cellColour =
                    (value >= 0.0f) ? juce::Colour::fromFloatRGBA(0.0f, scaled,
                                                                  0.0f, 1.0f)
                                    : juce::Colour::fromFloatRGBA(scaled, 0.0f,
                                                                  0.0f, 1.0f);
            g.setColour(cellColour);

            // Calculate cell position.
            const float cellX = squareBounds.getX() +
                                static_cast<float>(x) * cellWidth -
                                halfCellWidth;
            const float cellY = squareBounds.getY() +
                                static_cast<float>(y) * cellHeight -
                                halfCellWidth;
            g.fillRect(cellX, cellY, cellWidth, cellHeight);
        }
    }
    // Draw border and ellipse once.
    // g.setColour(juce::Colours::white);
    // g.drawRect(bounds, 2.0f);
    // g.drawEllipse(squareBounds.reduced(8), 10.0f);
}

/**
 * @brief Handle mouse down events.
 * @param e The mouse event.
 */
void VibratingMembrane::mouseDown(const juce::MouseEvent &e) {
    const int gridResolution = membraneModel.getGridResolution();
    const auto bounds = getLocalBounds().toFloat();
    const float side = std::min(bounds.getWidth(), bounds.getHeight());
    const float offsetX = (bounds.getWidth() - side) * 0.5f;
    const float offsetY = (bounds.getHeight() - side) * 0.5f;
    const juce::Rectangle<float> squareBounds(offsetX, offsetY, side, side);

    const float relativeX = static_cast<float>(e.x) - squareBounds.getX();
    const float relativeY = static_cast<float>(e.y) - squareBounds.getY();
    const int x = static_cast<int>((relativeX / squareBounds.getWidth()) *
                                   static_cast<float>(gridResolution));
    const int y = static_cast<int>((relativeY / squareBounds.getHeight()) *
                                   static_cast<float>(gridResolution));

    membraneModel.excite(0.9f, x, y);
}
