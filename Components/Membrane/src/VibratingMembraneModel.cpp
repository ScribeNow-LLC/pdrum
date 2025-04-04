#include "VibratingMembraneModel.h"
#include <algorithm>
#include <cmath>
#include <juce_audio_utils/juce_audio_utils.h>
#include <random>
#include <vector>
#ifdef _OPENMP
#include <omp.h>
#endif

/**
 * @brief Constructs a VibratingMembraneModel object.
 * @param state Reference to the AudioProcessorValueTreeState object.
 * @param gridResolution Resolution of the grid for the membrane simulation.
 */
VibratingMembraneModel::VibratingMembraneModel(
        juce::AudioProcessorValueTreeState &state, const int gridResolution) :
    gridResolution(gridResolution), state(state) {
    initialize();
    /// Setup grid
    const int totalCells = gridResolution * gridResolution;
    bufferA.resize(totalCells, 0.0f);
    bufferB.resize(totalCells, 0.0f);
    bufferC.resize(totalCells, 0.0f);
    isInside.resize(totalCells, 0);
    /// Pre-allocate buffers
    current = bufferA.data();
    previous = bufferB.data();
    next = bufferC.data();
    /// Pre-calculate circle region
    const int center = gridResolution / 2;
    const int radius = center - 1;
    for (int y = 1; y < gridResolution - 1; ++y) {
        for (int x = 1; x < gridResolution - 1; ++x) {
            const int index = y * gridResolution + x;
            const int dx = x - center;
            const int dy = y - center;
            if (dx * dx + dy * dy <= radius * radius) {
                isInside[index] = 1;
                activeIndices.push_back(index);
            }
        }
    }
    /// Start listening to parameter changes
    state.addParameterListener("membraneSize", this);
    state.addParameterListener("membraneTension", this);
    state.addParameterListener("randomness", this);
}

/**
 * @brief Initializes the simulation parameters.
 */
void VibratingMembraneModel::initialize() {
    dx = physicalSize / static_cast<float>(gridResolution);
    dt = dx / (c * std::sqrt(2.0f));
    targetDx = dx;
    targetC = c;
}

/**
 * @brief Excites the membrane at a specific position with a given
 * amplitude.
 * @param amplitude The amplitude of the excitation.
 * @param x The x-coordinate of the excitation position.
 * @param y The y-coordinate of the excitation position.
 */
void VibratingMembraneModel::excite(const float amplitude, const int x,
                                    const int y) {
    if (x > 1 && x < gridResolution - 1 && y > 1 && y < gridResolution - 1) {
        if (const int index = y * gridResolution + x; isInside[index]) {
            current[index] = amplitude;
            previous[index] = amplitude * 0.5f;
            measureIndex = index;
            /// Calculate the distance from the center:
            const int centerX = gridResolution / 2;
            const int centerY = gridResolution / 2;
            const int dx = x - centerX;
            const int dy = y - centerY;
            const double distance = std::sqrt(dx * dx + dy * dy);
            const double normalizedDistance =
                    distance / (static_cast<float>(gridResolution) / 2);
            const double offsetDistance = normalizedDistance - 0.5;
            const double scaledDistance = offsetDistance * 0.5;
            /// Get the current value of the membrane tension parameter
            const float tension = std::max(
                    0.01f,
                    std::min(1.0f, state.getRawParameterValue("membraneTension")
                                                   ->load() +
                                           static_cast<float>(scaledDistance)));
            /// Use the distance to add an offset to the tension
            const float cOffset = (tension * 50.0f) - 25.0f;
            targetC = 100.0f + cOffset;
        }
    }
}

/**
 * @brief Excites the center of the membrane with a given amplitude. The
 * randomness parameter is used to add a random offset to the center
 * position.
 * @param amplitude The amplitude of the excitation.
 */
void VibratingMembraneModel::exciteCenter(const float amplitude) {
    const float randomness = state.getRawParameterValue("randomness")->load();
    std::random_device dev;
    std::mt19937 rng(dev());
    std::uniform_real_distribution<> dist(-randomness, randomness);
    const int offsetX = static_cast<int>(dist(rng));
    const int offsetY = static_cast<int>(dist(rng));
    const int centerX = gridResolution / 2 + offsetX;
    const int centerY = gridResolution / 2 + offsetY;
    if (const int index = centerY * gridResolution + centerX + 1;
        isInside[index]) {
        current[index] = amplitude;
        previous[index] = amplitude * 0.5f;
        measureIndex = index;
        /// Calculate the distance from the center:
        const double distance =
                std::sqrt(offsetX * offsetX + offsetY * offsetY);
        const double normalizedDistance =
                distance / (static_cast<float>(gridResolution) / 2);
        const double offsetDistance = normalizedDistance - 0.5;
        const double scaledDistance = offsetDistance * 0.5;
        /// Get the current value of the membrane tension parameter
        const float tension = std::max(
                0.01f,
                std::min(1.0f,
                         state.getRawParameterValue("membraneTension")->load() +
                                 static_cast<float>(scaledDistance)));
        /// Use the distance to add an offset to the tension
        const float cOffset = (tension * 50.0f) - 25.0f;
        targetC = 100.0f + cOffset;
    }
}

/**
 * @brief Processes a single sample of the membrane simulation.
 * @param timeStep The time step for the simulation.
 * @return The current value of the membrane at the measurement index.
 */
float VibratingMembraneModel::processSample(const float timeStep) {
    static constexpr int updateInterval = 10;
    static int counter = 0;
    if (++counter < updateInterval)
        return current[measureIndex];
    counter = 0;

    constexpr float smoothingFactor = 0.005f;

    dx += (targetDx - dx) * smoothingFactor;
    c += (targetC - c) * smoothingFactor;

    dt = timeStep;

    const float newC2 = c * dt / dx;
    const float clampedC2 = std::min(newC2 * newC2, 0.49f);

#pragma omp parallel for schedule(static)
    for (const int idx: activeIndices) {
        // Load neighbors only once
        const float u = current[idx];
        const float laplacian = current[idx - gridResolution] +
                                current[idx + gridResolution] +
                                current[idx - 1] + current[idx + 1] - 4.0f * u;
        next[idx] =
                damping * (2.0f * u - previous[idx] + clampedC2 * laplacian);
    }

    std::swap(previous, current);
    std::swap(current, next);

    return current[measureIndex];
}

/**
 * @brief Handles parameter changes from the AudioProcessorValueTreeState.
 * @param parameterID The ID of the parameter that changed.
 * @param newValue The new value of the parameter.
 */
void VibratingMembraneModel::parameterChanged(const juce::String &parameterID,
                                              const float newValue) {
    if (parameterID == "membraneSize") {
        targetDx = newValue / static_cast<float>(gridResolution);
    } else if (parameterID == "membraneTension") {
        const float cOffset = (newValue * 50.0f) - 25.0f;
        targetC = 100.0f + cOffset;
        damping = 0.996f + (newValue - 0.5f) * 2.0f * 0.0035f;
    }
}
