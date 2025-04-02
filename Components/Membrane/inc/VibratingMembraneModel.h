#ifndef VIBRATING_MEMBRANE_MODEL_H
#define VIBRATING_MEMBRANE_MODEL_H

#include <algorithm>
#include <cmath>
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <random>
#include <vector>

/**
 * @brief Class to simulate a vibrating membrane using the wave equation.
 */
class VibratingMembraneModel final
    : public juce::AudioProcessorValueTreeState::Listener {
public:
    /**
     * @brief Constructs a VibratingMembraneModel object.
     * @param state Reference to the AudioProcessorValueTreeState object.
     * @param gridResolution Resolution of the grid for the membrane simulation.
     */
    explicit VibratingMembraneModel(juce::AudioProcessorValueTreeState &state,
                                    int gridResolution = 150);

    /**
     * @brief Initializes the simulation parameters.
     */
    void initialize();

    /**
     * @brief Excites the membrane at a specific position with a given
     * amplitude.
     * @param amplitude The amplitude of the excitation.
     * @param x The x-coordinate of the excitation position.
     * @param y The y-coordinate of the excitation position.
     */
    void excite(float amplitude, int x, int y);

    /**
     * @brief Excites the center of the membrane with a given amplitude. The
     * randomness parameter is used to add a random offset to the center
     * position.
     * @param amplitude The amplitude of the excitation.
     */
    void exciteCenter(float amplitude);

    /**
     * @brief Processes a single sample of the membrane simulation.
     * @param timeStep The time step for the simulation.
     * @return The current value of the membrane at the measurement index.
     */
    float processSample(float timeStep);

    /**
     * @brief Returns the current buffer for the membrane simulation.
     * @return Reference to the current buffer.
     */
    std::vector<float> &getCurrentBuffer() { return bufferA; }

    /**
     * @brief Gets the mask indicating the inside region of the membrane.
     * @return Reference to the mask vector.
     */
    std::vector<uint8_t> &getIsInsideMask() { return isInside; }

    /**
     * @brief Gets the grid resolution.
     * @return The grid resolution.
     */
    [[nodiscard]] int getGridResolution() const { return gridResolution; }

private:
    /**
     * @brief Handles parameter changes from the AudioProcessorValueTreeState.
     * @param parameterID The ID of the parameter that changed.
     * @param newValue The new value of the parameter.
     */
    void parameterChanged(const juce::String &parameterID,
                          float newValue) override;

    /** The resolution of the grid for the membrane simulation. */
    const int gridResolution;

    /** The physical size of the membrane. */
    float physicalSize = 1.0f;

    /** The speed of sound in the membrane. */
    float c = 100.0f;

    /** The position step size for the simulation. */
    float dx = 0.0f;

    /** The time step for the simulation. */
    float dt = 0.0f;

    /** The damping factor for the simulation. */
    float damping = 0.996f;

    /** The target speed of sound in the membrane. */
    float targetC = 100.0f;

    /** The target position step size for the simulation. */
    float targetDx = 0.0f;

    /** State buffers */
    std::vector<float> bufferA, bufferB, bufferC;

    /** Mask for the inside region of the membrane */
    std::vector<uint8_t> isInside;

    /** Indices of the active cells in the membrane */
    std::vector<int> activeIndices;

    /** Buffers for the current, previous, and next states of the membrane */
    float *current = nullptr;
    float *previous = nullptr;
    float *next = nullptr;

    /** Index for the measurement point in the membrane */
    int measureIndex = 0;

    /** Reference to the AudioProcessorValueTreeState object */
    juce::AudioProcessorValueTreeState &state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VibratingMembraneModel)
};

#endif // VIBRATING_MEMBRANE_MODEL_H
