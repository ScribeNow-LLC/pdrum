#ifndef MODAL_RESONATOR_H
#define MODAL_RESONATOR_H

#include <juce_audio_basics/juce_audio_basics.h>

class ModalResonator final : public juce::AudioProcessorValueTreeState::Listener
{
public:
    explicit ModalResonator(juce::AudioProcessorValueTreeState& state)
        : state(state)
    {
        state.addParameterListener("membraneSize", this);
        state.addParameterListener("depth", this);
    }

    void setParameters(const float radiusMeters, const float depthMeters,
                       const float sampleRate)
    {
        modes.clear();
        m_sampleRate = sampleRate;
        for (const std::vector<float> besselZeros = {
                 2.405f, 3.832f, 5.520f, 7.016f, 8.417f
             }; const float alpha : besselZeros)
        {
            constexpr int numAxialModes = 3;
            for (int n = 0; n < numAxialModes; ++n)
            {
                constexpr float c = 343.0f;
                float freq = (c / (2.0f * juce::MathConstants<float>::pi)) *
                    std::sqrt(
                        std::pow(alpha / radiusMeters, 2.0f) +
                        std::pow(
                            static_cast<float>(n) * juce::MathConstants
                            <float>::pi / depthMeters, 2.0f));
                float q = 10.0f;
                modes.emplace_back(freq, q, sampleRate);
            }
        }
    }

    float process(const float input)
    {
        float output = 0.0f;
        for (auto& mode : modes)
            output += mode.process(input);
        return output;
    }

private:
    void parameterChanged(const juce::String& parameterID, const float newValue) override
    {
        if (parameterID == "membraneSize")
        {
            const float depth = state.getRawParameterValue("depth")->load();
            setParameters(newValue, depth, m_sampleRate);
        }
        else if (parameterID == "depth")
        {
            const float size = state.getRawParameterValue("membraneSize")->load();
            setParameters(size, newValue, m_sampleRate);
        }
    }

    struct BiquadMode
    {
        BiquadMode(const float freq, const float q, const float sampleRate)
        {
            setCoefficients(freq, q, sampleRate);
        }

        void setCoefficients(const float freq, const float q,
                             const float sampleRate)
        {
            const float omega = 2.0f * juce::MathConstants<float>::pi * freq /
                sampleRate;
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

        float process(const float input)
        {
            const float output = b0 * input + b1 * x1 + b2 * x2
                - a1 * y1 - a2 * y2;
            x2 = x1;
            x1 = input;
            y2 = y1;
            y1 = output;
            return output;
        }

        float b0 = 0, b1 = 0, b2 = 0;
        float a1 = 0, a2 = 0;
        float x1 = 0, x2 = 0;
        float y1 = 0, y2 = 0;
    };

    std::vector<BiquadMode> modes;
    juce::AudioProcessorValueTreeState& state;

    float m_sampleRate = 44100.0f;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModalResonator)
};

#endif //MODAL_RESONATOR_H
