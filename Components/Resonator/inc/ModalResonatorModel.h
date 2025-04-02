#ifndef MODAL_RESONATOR_MODEL_H
#define MODAL_RESONATOR_MODEL_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>

/**
 * @brief Modal resonator class.
 */
class ModalResonatorModel final
    : public juce::AudioProcessorValueTreeState::Listener {
public:
    /**
     * @brief Constructor for ModalResonatorModel.
     * @param state The AudioProcessorValueTreeState to use for parameter
     */
    explicit ModalResonatorModel(juce::AudioProcessorValueTreeState &state);

    /**
     * @brief Set the physical parameters of the resonator.
     * @param radiusMeters The radius of the resonator in meters.
     * @param depthMeters The depth of the resonator in meters.
     * @param sampleRate The sample rate of the audio processor.
     */
    void setParameters(float radiusMeters, float depthMeters, float sampleRate);

    /**
     * @brief Process the input signal through the resonator.
     * @param input The input signal to process.
     * @return The processed output signal.
     */
    float process(float input);

private:
    /**
     * @brief Callback for when a parameter changes.
     * @param parameterID The ID of the parameter that changed.
     * @param newValue The new value of the parameter.
     */
    void parameterChanged(const juce::String &parameterID,
                          float newValue) override;

    /**
     * @brief Biquad filter mode structure.
     */
    struct BiquadMode {
        /**
         * @brief Constructor for BiquadMode.
         * @param freq The frequency of the mode.
         * @param q The Q factor of the mode.
         * @param sampleRate
         */
        BiquadMode(float freq, float q, float sampleRate);

        /**
         * @brief Set the coefficients for the Biquad filter.
         * @param freq The frequency of the mode.
         * @param q The Q factor of the mode.
         * @param sampleRate The sample rate of the audio processor.
         */
        void setCoefficients(float freq, float q, float sampleRate);

        /**
         * @brief Process the input signal through the Biquad filter.
         * @param input The input signal to process.
         * @return The processed output signal.
         */
        float process(float input);

        /** Biquad filter coefficients */
        float b0 = 0, b1 = 0, b2 = 0;
        float a1 = 0, a2 = 0;
        float x1 = 0, x2 = 0;
        float y1 = 0, y2 = 0;
    };

    /** List of Biquad modes */
    std::vector<BiquadMode> modes;

    /** AudioProcessorValueTreeState reference */
    juce::AudioProcessorValueTreeState &state;

    /** Sample rate of the audio processor */
    float m_sampleRate = 44100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModalResonatorModel)
};

#endif // MODAL_RESONATOR_MODEL_H
