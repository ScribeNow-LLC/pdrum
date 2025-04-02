#ifndef VIBRATING_MEMBRANE_MODEL_H
#define VIBRATING_MEMBRANE_MODEL_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>

class VibratingMembraneModel final : public
        juce::AudioProcessorValueTreeState::Listener {
public:
    /**
     * @brief Constructor for the VibratingMembraneModel class.
     */
    explicit VibratingMembraneModel(
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
        state.addParameterListener("membraneSize", this);
        state.addParameterListener("membraneTension", this);
    }

    void initialize() {
        /// CFL condition: c*dt/dx = 1/sqrt(2)
        dx = physicalSize / static_cast<float>(gridResolution);
        dt = dx / (c * std::sqrt(2.0f));
    }

    void excite(const float amplitude, const int x, const int y) {
        // Only add impulse if within valid region and inside circle.
        if (const int index = y * gridResolution + x;
            x > 1 && x < gridResolution - 1 && y > 1 && y < gridResolution - 1
            && isInside[index]) {
            current[index] = amplitude;
            previous[index] = amplitude * 0.5f;
        }
    }

    /**
     * @brief Timer callback function to update the simulation.
     */
    float processSample() {
        dt = 1.0f / 44100.0f;
        const float factor = (c * dt / dx);
        const float c2 = factor * factor;
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
        /// Return the sample at the center:
        const int center = gridResolution / 2;
        const int index = center * gridResolution + center;
        return current[index];
    }

    std::vector<float> &getCurrent() { return current; }

    [[nodiscard]] int getGridResolution() const {
        return gridResolution;
    }

    std::vector<bool> &getIsInside() { return isInside; }

private:
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

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VibratingMembraneModel)
};

#endif //VIBRATING_MEMBRANE_MODEL_H
