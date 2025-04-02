#ifndef VIBRATING_MEMBRANE_MODEL_H
#define VIBRATING_MEMBRANE_MODEL_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>

class VibratingMembraneModel final : public juce::AudioProcessorValueTreeState::Listener {
public:
    explicit VibratingMembraneModel(juce::AudioProcessorValueTreeState& state)
        : state(state) {
        initialize();

        const int totalCells = gridResolution * gridResolution;
        bufferA.resize(totalCells, 0.0f);
        bufferB.resize(totalCells, 0.0f);
        bufferC.resize(totalCells, 0.0f);
        isInside.resize(totalCells, false);

        current = bufferA.data();
        previous = bufferB.data();
        next = bufferC.data();

        const int center = gridResolution / 2;
        const int radius = center - 1;
        for (int y = 0; y < gridResolution; ++y) {
            for (int x = 0; x < gridResolution; ++x) {
                const int index = y * gridResolution + x;
                const int dx = x - center;
                const int dy = y - center;
                isInside[index] = (dx * dx + dy * dy <= radius * radius);
            }
        }

        state.addParameterListener("membraneSize", this);
        state.addParameterListener("membraneTension", this);
    }

    void initialize() {
        dx = physicalSize / static_cast<float>(gridResolution);
        dt = dx / (c * std::sqrt(2.0f));
        targetDx = dx;
        targetC = c;
    }

    void excite(const float amplitude, const int x, const int y) {
        if (x > 1 && x < gridResolution - 1 && y > 1 && y < gridResolution - 1) {
            const int index = y * gridResolution + x;
            if (isInside[index]) {
                current[index] = amplitude;
                previous[index] = amplitude * 0.5f;
            }
        }
    }

    /// TODO - add a midi key to slightly change the size
    void exciteCenter(const float amplitude) {
        const int centerX = gridResolution / 2;
        const int centerY = gridResolution / 2;
        if (const int index = centerY * gridResolution + centerX; isInside[index]) {
            current[index + 1] = amplitude;
            previous[index + 1] = amplitude * 0.5f;
        }
    }

    float processSample() {
        static constexpr int updateInterval = 10;
        static int counter = 0;
        if (++counter < updateInterval)
            return current[(gridResolution / 2) * gridResolution + (gridResolution / 2)];
        counter = 0;

        constexpr float smoothingFactor = 0.005f; // slower smoothing for stability

        dx = dx + (targetDx - dx) * smoothingFactor;
        c = c + (targetC - c) * smoothingFactor;

        dt = 1.0f / 44100.0f;
        float c2 = std::pow(c * dt / dx, 2.0f);

        // Clamp c2 to ensure it remains stable under the CFL condition
        const float maxC2 = 0.49f; // Just under 0.5 for 2D grid stability
        c2 = std::min(c2, maxC2);

        for (int y = 1; y < gridResolution - 1; ++y) {
            for (int x = 1; x < gridResolution - 1; ++x) {
                const int index = y * gridResolution + x;
                if (!isInside[index]) {
                    next[index] = 0.0f;
                    continue;
                }
                const float laplacian = current[index - gridResolution] + current[index + gridResolution] +
                                        current[index - 1] + current[index + 1] - 4.0f * current[index];
                next[index] = damping * (2.0f * current[index] - previous[index] + c2 * laplacian);
            }
        }

        std::swap(previous, current);
        std::swap(current, next);

        const int centerIndex = (gridResolution / 2) * gridResolution + (gridResolution / 2);
        return current[centerIndex];
    }

    std::vector<float>& getCurrentBuffer() { return bufferA; }
    std::vector<bool>& getIsInsideMask() { return isInside; }
    int getGridResolution() const { return gridResolution; }

private:
    void parameterChanged(const juce::String& parameterID, float newValue) override {
        if (parameterID == "membraneSize") {
            targetDx = newValue / static_cast<float>(gridResolution);
        } else if (parameterID == "membraneTension") {
            /// TODO - implement tension?
        }
    }

    const int gridResolution = 128;
    float physicalSize = 1.0f;
    float c = 100.0f;
    float dx = 0.0f;
    float dt = 0.0f;
    float damping = 0.996f;

    float targetC = 100.0f;
    float targetDx = 0.0f;

    std::vector<float> bufferA, bufferB, bufferC;
    std::vector<bool> isInside;

    float* current = nullptr;
    float* previous = nullptr;
    float* next = nullptr;

    juce::AudioProcessorValueTreeState& state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VibratingMembraneModel)
};

#endif // VIBRATING_MEMBRANE_MODEL_H