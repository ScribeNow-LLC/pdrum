#include "ModalResonatorModel.h"

/**
 * @brief Constructor for ModalResonator.
 * @param state The AudioProcessorValueTreeState to use for parameter
 */
ModalResonatorModel::ModalResonatorModel(
        juce::AudioProcessorValueTreeState &state) : state(state) {
    state.addParameterListener("membraneSize", this);
    state.addParameterListener("depth", this);
}

/**
 * @brief Set the physical parameters of the resonator.
 * @param radiusMeters The radius of the resonator in meters.
 * @param depthMeters The depth of the resonator in meters.
 * @param sampleRate The sample rate of the audio processor.
 */
void ModalResonatorModel::setParameters(const float radiusMeters,
                                        const float depthMeters,
                                        const float sampleRate) {
    m_sampleRate = sampleRate;

    std::vector<BiquadMode> newModes;

    for (const std::vector besselZeros = {2.405f, 3.832f, 5.520f, 7.016f,
                                          8.417f};
         const float alpha: besselZeros) {
        constexpr int numAxialModes = 3;
        for (int n = 0; n < numAxialModes; ++n) {
            constexpr float c = 343.0f;
            float freq =
                    (c / (2.0f * juce::MathConstants<float>::pi)) *
                    std::sqrt(std::pow(alpha / radiusMeters, 2.0f) +
                              std::pow(static_cast<float>(n) *
                                               juce::MathConstants<float>::pi /
                                               depthMeters,
                                       2.0f));
            float q = 10.0f;
            newModes.emplace_back(freq, q, sampleRate);
        }
    }
    // Begin crossfade: move current modes to oldModes and swap in newModes
    oldModes = std::move(modes);
    modes = std::move(newModes);
    crossfadeCounter = 0;
    isCrossfading = true;
}

/**
 * @brief Process the input signal through the resonator.
 * @param input The input signal to process.
 * @return The processed output signal.
 */
float ModalResonatorModel::process(const float input) {
    float newOutput = 0.0f;
    for (auto &mode: modes)
        newOutput += mode.process(input);
    if (isCrossfading) {
        float oldOutput = 0.0f;
        for (auto &oldMode: oldModes)
            oldOutput += oldMode.process(input);
        const float alpha = static_cast<float>(crossfadeCounter) /
                            static_cast<float>(crossfadeDuration);
        const float output = (1.0f - alpha) * oldOutput + alpha * newOutput;
        if (++crossfadeCounter >= crossfadeDuration) {
            isCrossfading = false;
            oldModes.clear();
        }
        return output;
    }
    return newOutput;
}

/**
 * @brief Callback for when a parameter changes.
 * @param parameterID The ID of the parameter that changed.
 * @param newValue The new value of the parameter.
 */
void ModalResonatorModel::parameterChanged(const juce::String &parameterID,
                                           const float newValue) {
    if (parameterID == "membraneSize") {
        const float depth = state.getRawParameterValue("depth")->load();
        setParameters(newValue, depth, m_sampleRate);
    } else if (parameterID == "depth") {
        const float size = state.getRawParameterValue("membraneSize")->load();
        setParameters(size, newValue, m_sampleRate);
    }
}

/**
 * @brief Constructor for BiquadMode.
 * @param freq The frequency of the mode.
 * @param q The Q factor of the mode.
 * @param sampleRate
 */
ModalResonatorModel::BiquadMode::BiquadMode(const float freq, const float q,
                                            const float sampleRate) {
    setCoefficients(freq, q, sampleRate);
}

/**
 * @brief Set the coefficients for the Biquad filter.
 * @param freq The frequency of the mode.
 * @param q The Q factor of the mode.
 * @param sampleRate The sample rate of the audio processor.
 */
void ModalResonatorModel::BiquadMode::setCoefficients(const float freq,
                                                      const float q,
                                                      const float sampleRate) {
    const float omega =
            2.0f * juce::MathConstants<float>::pi * freq / sampleRate;
    const float alpha = std::sin(omega) / (2.0f * q);
    const float cos_omega = std::cos(omega);

    b0 = alpha;
    b1 = 0.0f;
    b2 = -alpha;
    const float a0 = 1.0f + alpha;
    a1 = -2.0f * cos_omega;
    a2 = 1.0f - alpha;

    // Normalize
    b0 /= a0;
    b1 /= a0;
    b2 /= a0;
    a1 /= a0;
    a2 /= a0;
}

/**
 * @brief Process the input signal through the Biquad filter.
 * @param input The input signal to process.
 * @return The processed output signal.
 */
float ModalResonatorModel::BiquadMode::process(const float input) {
    const float output = b0 * input + b1 * x1 + b2 * x2 - a1 * y1 - a2 * y2;
    x2 = x1;
    x1 = input;
    y2 = y1;
    y1 = output;
    return output;
}
