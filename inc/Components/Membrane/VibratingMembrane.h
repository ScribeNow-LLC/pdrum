#ifndef VIBRATING_MEMBRANE_H
#define VIBRATING_MEMBRANE_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>

/**
 * @brief Class representing a vibrating membrane simulation with physical dimensions,
 *        optimized for performance by using contiguous memory.
 */
class VibratingMembrane final : public juce::Component, juce::Timer {
public:
    VibratingMembrane()
    {
        dx = physicalSize / static_cast<float>(gridResolution);
        dt = dx / (c * std::sqrt(2.0f));  // CFL condition: c*dt/dx = 1/sqrt(2)

        const float frameTime = 1.0f / 60.0f;
        timeStepsPerFrame = static_cast<int>(std::ceil(frameTime / dt));

        // Allocate contiguous arrays.
        const int totalCells = gridResolution * gridResolution;
        previous.resize(totalCells, 0.0f);
        current.resize(totalCells, 0.0f);
        next.resize(totalCells, 0.0f);
        isInside.resize(totalCells, false);

        // Precompute the "inside circle" mask.
        const int center = gridResolution / 2;
        const int radius = gridResolution / 2 - 1;
        for (int y = 0; y < gridResolution; ++y)
        {
            for (int x = 0; x < gridResolution; ++x)
            {
                const int index = y * gridResolution + x;
                const int dxCell = x - center;
                const int dyCell = y - center;
                isInside[index] = (dxCell * dxCell + dyCell * dyCell) <= (radius * radius);
            }
        }

        startTimerHz(60);
    }

    void paint(juce::Graphics& g) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const float side = std::min(bounds.getWidth(), bounds.getHeight());
        const float offsetX = (bounds.getWidth() - side) * 0.5f;
        const float offsetY = (bounds.getHeight() - side) * 0.5f;
        const juce::Rectangle<float> squareBounds(offsetX, offsetY, side, side);

        const float cellWidth  = squareBounds.getWidth() / static_cast<float>(gridResolution);
        const float cellHeight = squareBounds.getHeight() / static_cast<float>(gridResolution);

        // Draw each cell (only if it's inside the circle).
        for (int y = 0; y < gridResolution; ++y)
        {
            for (int x = 0; x < gridResolution; ++x)
            {
                const int index = y * gridResolution + x;
                if (!isInside[index])
                    continue;

                const float value = current[index];
                const float logValue = std::log10(1.0f + std::abs(value) * 100.0f) /
                                         std::log10(101.0f);
                const float scaled = juce::jlimit(0.0f, 1.0f, logValue);

                juce::Colour cellColour = (value >= 0.0f)
                    ? juce::Colour::fromFloatRGBA(0.0f, scaled, 0.0f, 1.0f)
                    : juce::Colour::fromFloatRGBA(scaled, 0.0f, 0.0f, 1.0f);
                g.setColour(cellColour);
                g.fillRect(static_cast<float>(x) * cellWidth - 1,
                           static_cast<float>(y) * cellHeight - 1,
                           cellWidth + 1,
                           cellHeight + 1);
            }
        }
    }

    void mouseDown(const juce::MouseEvent &e) override
    {
        const auto bounds = getLocalBounds().toFloat();
        const float side = std::min(bounds.getWidth(), bounds.getHeight());
        const float offsetX = (bounds.getWidth() - side) * 0.5f;
        const float offsetY = (bounds.getHeight() - side) * 0.5f;
        const juce::Rectangle<float> squareBounds(offsetX, offsetY, side, side);

        const int x = static_cast<int>(
            (static_cast<float>(e.x) / squareBounds.getWidth()) * static_cast<float>(gridResolution));
        const int y = static_cast<int>(
            (static_cast<float>(e.y) / squareBounds.getHeight()) * static_cast<float>(gridResolution));

        const int index = y * gridResolution + x;
        // Only add impulse if within valid region and inside circle.
        const int center = gridResolution / 2;
        if (x > 1 && x < gridResolution - 1 && y > 1 && y < gridResolution - 1 && isInside[index])
        {
            current[index]  = 1.0f;
            previous[index] = 0.5f;
        }
    }

private:
    void timerCallback() override
    {
        const float factor = (c * dt / dx);
        const float c2 = factor * factor; // should be <= 0.5 due to CFL

        // Loop over substeps per frame.
        for (int step = 0; step < timeStepsPerFrame; ++step)
        {
            // Update inner grid cells (skip border cells).
            for (int y = 1; y < gridResolution - 1; ++y)
            {
                for (int x = 1; x < gridResolution - 1; ++x)
                {
                    const int index = y * gridResolution + x;
                    if (!isInside[index])
                    {
                        next[index] = 0.0f;
                        continue;
                    }

                    // Compute laplacian using contiguous memory indices.
                    const int indexUp    = (y - 1) * gridResolution + x;
                    const int indexDown  = (y + 1) * gridResolution + x;
                    const int indexLeft  = y * gridResolution + (x - 1);
                    const int indexRight = y * gridResolution + (x + 1);

                    const float laplacian = current[indexUp] +
                                            current[indexDown] +
                                            current[indexLeft] +
                                            current[indexRight] -
                                            4.0f * current[index];

                    // Use a fixed damping value.
                    constexpr float damping = 0.996f;
                    next[index] = damping * (2.0f * current[index] - previous[index] + c2 * laplacian);
                }
            }
            // Swap buffers.
            std::swap(previous, current);
            std::swap(current, next);
        }

        repaint();
    }

    // Grid dimensions.
    const int gridResolution = 50;

    // Physical parameters.
    float physicalSize = 1.0f; // Membrane size in meters.
    float c  = 100.0f;         // Wave speed in m/s.

    // Spatial and time discretization.
    float dx = 0.0f; // Computed as physicalSize / gridResolution.
    float dt = 0.0f; // Set by the CFL condition.

    // Simulation substeps per frame.
    int timeStepsPerFrame = 1;

    // Contiguous arrays for grid states.
    std::vector<float> previous;
    std::vector<float> current;
    std::vector<float> next;

    // Precomputed mask: true for cells inside the circular membrane.
    std::vector<bool> isInside;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VibratingMembrane)
};

#endif //VIBRATING_MEMBRANE_H
