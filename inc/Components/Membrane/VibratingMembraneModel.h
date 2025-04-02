#ifndef VIBRATING_MEMBRANE_MODEL_H
#define VIBRATING_MEMBRANE_MODEL_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>
#include <cmath>
#include <algorithm>
#include <cassert>
#ifdef _OPENMP
#include <omp.h>
#endif

class VibratingMembraneModel final : public juce::AudioProcessorValueTreeState::Listener {
public:
    explicit VibratingMembraneModel(juce::AudioProcessorValueTreeState& state)
        : state(state) {
        initialize();

        const int totalCells = gridResolution * gridResolution;
        bufferA.resize(totalCells, 0.0f);
        bufferB.resize(totalCells, 0.0f);
        bufferC.resize(totalCells, 0.0f);
        isInside.resize(totalCells, 0);

        current = bufferA.data();
        previous = bufferB.data();
        next = bufferC.data();

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

    void exciteCenter(const float amplitude) {
        const int centerX = gridResolution / 2;
        const int centerY = gridResolution / 2;
        const int index = centerY * gridResolution + centerX + 1;
        if (isInside[index]) {
            current[index] = amplitude;
            previous[index] = amplitude * 0.5f;
        }
    }

    /// TODO - track the position of the mouse used to excite the membrane and measure from there
    float processSample(const float timeStep) {
        static constexpr int updateInterval = 10;
        static int counter = 0;
        if (++counter < updateInterval)
            return current[(gridResolution / 2) * gridResolution + (gridResolution / 2)];
        counter = 0;

        constexpr float smoothingFactor = 0.005f;

        dx += (targetDx - dx) * smoothingFactor;
        c += (targetC - c) * smoothingFactor;

        dt = timeStep;

        const float newC2 = c * dt / dx;
        const float clampedC2 = std::min(newC2 * newC2, 0.49f);

        #pragma omp parallel for schedule(static)
        for (int i = 0; i < static_cast<int>(activeIndices.size()); ++i) {
            const int index = activeIndices[i];

            const float laplacian = current[index - gridResolution] + current[index + gridResolution] +
                                    current[index - 1] + current[index + 1] - 4.0f * current[index];

            next[index] = damping * (2.0f * current[index] - previous[index] + clampedC2 * laplacian);
        }

        std::swap(previous, current);
        std::swap(current, next);

        const int centerIndex = (gridResolution / 2) * gridResolution + (gridResolution / 2);
        return current[centerIndex];
    }

    std::vector<float>& getCurrentBuffer() { return bufferA; }
    std::vector<uint8_t>& getIsInsideMask() { return isInside; }
    int getGridResolution() const { return gridResolution; }

private:
    void parameterChanged(const juce::String& parameterID, const float newValue) override {
        if (parameterID == "membraneSize") {
            targetDx = newValue / static_cast<float>(gridResolution);
        } else if (parameterID == "membraneTension") {
            // targetC = 100.0f * newValue;
            /// if newValue is from 0 to 1, add or subtract a very small value from damping:
            damping = 0.996f + (newValue - 0.5f) * 2.0f * 0.0035f;
        }
    }

    const int gridResolution = 150;
    float physicalSize = 1.0f;
    float c = 100.0f;
    float dx = 0.0f;
    float dt = 0.0f;
    float damping = 0.996f;

    float targetC = 100.0f;
    float targetDx = 0.0f;

    std::vector<float> bufferA, bufferB, bufferC;
    std::vector<uint8_t> isInside;
    std::vector<int> activeIndices;

    float* current = nullptr;
    float* previous = nullptr;
    float* next = nullptr;

    juce::AudioProcessorValueTreeState& state;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VibratingMembraneModel)
};

#endif // VIBRATING_MEMBRANE_MODEL_H
