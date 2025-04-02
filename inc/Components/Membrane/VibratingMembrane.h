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
class VibratingMembrane final : public juce::Component, juce::Timer,
                                public
                                juce::AudioProcessorValueTreeState::Listener {
public:
    /**
     * @brief Constructor for the VibratingMembrane class.
     */
    explicit VibratingMembrane(
        juce::AudioProcessorValueTreeState &state) : state(state) {
        initialize();
        /// Allocate contiguous arrays.
        const int totalCells = gridResolution * gridResolution;
        previous.resize(totalCells, 0.0f);
        current.resize(totalCells, 0.0f);
        next.resize(totalCells, 0.0f);
        isInside.resize(totalCells, false);
        /// Precompute the "inside circle" mask.
        const int center = gridResolution / 2;
        const int radius = gridResolution / 2 - 1;
        for (int y = 0; y < gridResolution; ++y) {
            for (int x = 0; x < gridResolution; ++x) {
                const int index = y * gridResolution + x;
                const int dxCell = x - center;
                const int dyCell = y - center;
                isInside[index] =
                        (dxCell * dxCell + dyCell * dyCell) <= (
                            radius * radius);
            }
        }
        startTimerHz(60);
        state.addParameterListener("membraneSize", this);
        state.addParameterListener("membraneTension", this);
    }

    void initialize() {
        /// CFL condition: c*dt/dx = 1/sqrt(2)
        dx = physicalSize / static_cast<float>(gridResolution);
        dt = dx / (c * std::sqrt(2.0f));
        constexpr float frameTime = 1.0f / 60.0f;
        timeStepsPerFrame = static_cast<int>(std::ceil(frameTime / dt));
    }

    /**
     * @brief Paint the component.
     * @param g The graphics context used for painting.
     */
    void paint(juce::Graphics &g) override {
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

        // Only add impulse if within valid region and inside circle.
        if (const int index = y * gridResolution + x;
            x > 1 && x < gridResolution - 1 && y > 1 && y < gridResolution - 1
            && isInside[index]) {
            current[index] = 1.0f;
            previous[index] = 0.5f;
        }
    }

private:
    /**
     * @brief Timer callback function to update the simulation.
     */
    void timerCallback() override {
        const float factor = (c * dt / dx);
        const float c2 = factor * factor;
        /// Loop over sub-steps per frame.
        for (int step = 0; step < timeStepsPerFrame; ++step) {
            /// Update inner grid cells (skip border cells).
            for (int y = 1; y < gridResolution - 1; ++y) {
                for (int x = 1; x < gridResolution - 1; ++x) {
                    const int index = y * gridResolution + x;
                    if (!isInside[index]) {
                        next[index] = 0.0f;
                        continue;
                    }
                    /// Compute laplacian using contiguous memory indices.
                    const int indexUp = (y - 1) * gridResolution + x;
                    const int indexDown = (y + 1) * gridResolution + x;
                    const int indexLeft = y * gridResolution + (x - 1);
                    const int indexRight = y * gridResolution + (x + 1);
                    const float laplacian = current[indexUp] +
                                            current[indexDown] +
                                            current[indexLeft] +
                                            current[indexRight] -
                                            4.0f * current[index];
                    /// Use a fixed damping value.
                    constexpr float damping = 0.996f;
                    next[index] =
                            damping * (
                                2.0f * current[index] - previous[index] + c2 *
                                laplacian);
                }
            }
            // Swap buffers.
            std::swap(previous, current);
            std::swap(current, next);
        }
        repaint();
    }

    /**
     * @brief Handle parameter changes.
     * @param parameterID The ID of the parameter that changed.
     * @param newValue The new value of the parameter.
     */
    void parameterChanged(const juce::String &parameterID,
                          const float newValue) override {
        if (parameterID == "membraneSize") {
            physicalSize = newValue;
            initialize();
        }
        if (parameterID == "membraneTension") {
            c = newValue;
            initialize();
        }
    }

    /** Grid resolution (number of cells along one side). */
    const int gridResolution = 50;

    /** Physical size of the membrane in meters. */
    float physicalSize = 1.0f;

    /** Wave speed in meters per second. */
    float c = 100.0f;

    /** Spacial step size in meters, computed as physicalSize / gridResolution */
    float dx = 0.0f;

    /** Time step size in seconds, computed as dx / (c * sqrt(2)) */
    float dt = 0.0f;

    /** Simulation time steps per frame, computed as ceil(frameTime / dt) */
    int timeStepsPerFrame = 1;

    /** Contiguous memory for the previous frame simulation. */
    std::vector<float> previous;

    /** Contiguous memory for the current frame simulation. */
    std::vector<float> current;

    /** Contiguous memory for the next frame simulation. */
    std::vector<float> next;

    /** Precomputed mask for "inside circle" cells. */
    std::vector<bool> isInside;

    /** Reference to the AudioProcessorValueTreeState for parameter management. */
    juce::AudioProcessorValueTreeState &state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VibratingMembrane)
};

#endif //VIBRATING_MEMBRANE_H
