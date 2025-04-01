#ifndef VIBRATING_MEMBRANE_H
#define VIBRATING_MEMBRANE_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>

/**
 * @brief Class representing a vibrating membrane simulation.
 */
class VibratingMembrane final : public juce::Component, juce::Timer {
public:
    /**
     * @brief Constructor for the VibratingMembrane class.
     */
    VibratingMembrane() {
        resizeGrid(gridSize);
        startTimerHz(60);
    }

    /**
     * @brief Paint the component.
     * @param g The graphics context used for painting.
     */
    void paint(juce::Graphics &g) override {
        const auto bounds = getLocalBounds().toFloat();
        /// Ensure square bounds, centered
        const float side = std::min(bounds.getWidth(), bounds.getHeight());
        const float offsetX = (bounds.getWidth() - side) * 0.5f;
        const float offsetY = (bounds.getHeight() - side) * 0.5f;
        const juce::Rectangle squareBounds(offsetX, offsetY, side, side);

        const float cellWidth = squareBounds.getWidth() / static_cast<float>(
                                    gridSize);
        const float cellHeight = squareBounds.getHeight() / static_cast<float>(
                                     gridSize);

        const int center = gridSize / 2;
        const int radius = gridSize / 2 - 1;

        for (int y = 0; y < gridSize; ++y) {
            for (int x = 0; x < gridSize; ++x) {
                if (!isInsideCircle(x, y, center, center, radius))
                    continue;
                const float value = current[y][x];
                const float logValue =
                        std::log10(1.0f + std::abs(value) * 100.0f) /
                        std::log10(101.0f);
                const float scaled = 0.5f + 0.5f * std::copysign(
                                         logValue, value);
                const auto brightness = static_cast<uint8_t>(juce::jlimit(
                        0.0f, 1.0f, 0.5f + 0.5f * scaled) * 255.0f);
                g.setColour(juce::Colour(brightness, brightness, brightness));
                g.fillRect(static_cast<float>(x) * cellWidth - 1,
                           static_cast<float>(y) * cellHeight - 1,
                           cellWidth + 1,
                           cellHeight + 1);
            }
        }
    }

    /**
     * @brief Handle mouse down events.
     * @param e The mouse event.
     */
    void mouseDown(const juce::MouseEvent &e) override {
        const auto bounds = getLocalBounds().toFloat();
        // Ensure square bounds, centered
        const float side = std::min(bounds.getWidth(), bounds.getHeight());
        const float offsetX = (bounds.getWidth() - side) * 0.5f;
        const float offsetY = (bounds.getHeight() - side) * 0.5f;
        const juce::Rectangle squareBounds(offsetX, offsetY, side, side);

        const int x = static_cast<int>(
            (static_cast<float>(e.x) / squareBounds.getWidth()) * static_cast<
                float>(gridSize));
        const int y = static_cast<int>(
            (static_cast<float>(e.y) / squareBounds.getHeight()) * static_cast<
                float>(gridSize));

        const int center = gridSize / 2;

        /// Impulse at the mouse position
        if (const int radius = gridSize / 2 - 1;
            x > 1 && x < gridSize - 1 && y > 1 && y < gridSize - 1 &&
            isInsideCircle(x, y, center, center, radius)) {
            current[y][x] = 1.0f;
            previous[y][x] = 0.5f;
        }
    }

private:
    /**
     * @brief Timer callback function to update the simulation.
     */
    void timerCallback() override {
        constexpr int timeStepsPerFrame = 10;

        for (int step = 0; step < timeStepsPerFrame; ++step) {
            const int center = gridSize / 2;
            const int radius = gridSize / 2 - 1;
            for (int y = 1; y < gridSize - 1; ++y) {
                for (int x = 1; x < gridSize - 1; ++x) {
                    constexpr float c2 = 0.25f;
                    constexpr float damping = 0.99f;
                    // constexpr float damping = 0.996f;
                    if (!isInsideCircle(x, y, center, center, radius)) {
                        next[y][x] = 0.0f;
                        continue;
                    }

                    const float laplacian =
                            current[y + 1][x] + current[y - 1][x]
                            + current[y][x + 1] + current[y][x - 1]
                            - 4.0f * current[y][x];

                    next[y][x] = damping * (
                                     2.0f * current[y][x]
                                     - previous[y][x]
                                     + c2 * laplacian
                                 );
                }
            }

            std::swap(previous, current);
            std::swap(current, next);
        }

        repaint();
    }

    /**
     * @brief Resize the grid to the specified size.
     * @param size The new size of the grid.
     */
    void resizeGrid(const int size) {
        previous = std::vector(size, std::vector(size, 0.0f));
        current = std::vector(size, std::vector(size, 0.0f));
        next = std::vector(size, std::vector(size, 0.0f));
    }

    /**
     * @brief Check if a point is inside a circle.
     * @param x The x-coordinate of the point.
     * @param y The y-coordinate of the point.
     * @param centerX The x-coordinate of the circle's center.
     * @param centerY The y-coordinate of the circle's center.
     * @param radius The radius of the circle.
     * @return True if the point is inside the circle, false otherwise.
     */
    static bool isInsideCircle(const int x, const int y, const int centerX,
                               const int centerY,
                               const int radius) {
        const int dx = x - centerX;
        const int dy = y - centerY;
        return dx * dx + dy * dy <= radius * radius;
    }

    /** The size of the grid */
    const int gridSize = 100;

    /** States of the membrane */
    std::vector<std::vector<float> > previous, current, next;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VibratingMembrane)
};


#endif //VIBRATING_MEMBRANE_H
