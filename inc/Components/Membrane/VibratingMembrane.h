#ifndef VIBRATING_MEMBRANE_H
#define VIBRATING_MEMBRANE_H

#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_audio_utils/juce_audio_utils.h>
#include <vector>


class VibratingMembrane : public juce::Component, private juce::Timer
{
public:
    VibratingMembrane()
    {
        resizeGrid(gridSize);
        startTimerHz(60);
    }

    void paint(juce::Graphics& g) override
    {
        auto bounds = getLocalBounds().toFloat();
        float cellWidth = bounds.getWidth() / (float)gridSize;
        float cellHeight = bounds.getHeight() / (float)gridSize;

        for (int y = 0; y < gridSize; ++y)
        {
            for (int x = 0; x < gridSize; ++x)
            {
                float value = current[y][x];
                uint8_t brightness = juce::jlimit(0, 255, (int)(127.0f + 127.0f * value));
                g.setColour(juce::Colour(brightness, brightness, brightness));
                g.fillRect(x * cellWidth, y * cellHeight, cellWidth, cellHeight);
            }
        }
    }

    void mouseDown(const juce::MouseEvent& e) override
    {
        auto bounds = getLocalBounds();
        int x = (int)((float)e.x / bounds.getWidth() * gridSize);
        int y = (int)((float)e.y / bounds.getHeight() * gridSize);

        if (x > 1 && x < gridSize - 1 && y > 1 && y < gridSize - 1)
        {
            current[y][x] = 1.0f;
            previous[y][x] = 0.5f; // slightly different: initial "kick"
        }
    }

private:
    void timerCallback() override
    {
        float damping = 0.999f;
        float c2 = 0.25f; // must be <= 0.25 for stability

        for (int y = 1; y < gridSize - 1; ++y)
        {
            for (int x = 1; x < gridSize - 1; ++x)
            {
                next[y][x] = damping * (
                    2.0f * current[y][x] - previous[y][x]
                    + c2 * (current[y+1][x] + current[y-1][x] +
                            current[y][x+1] + current[y][x-1] -
                            4.0f * current[y][x]));
            }
        }

        std::swap(previous, current);
        std::swap(current, next);
        repaint();
    }

    void resizeGrid(int size)
    {
        previous = std::vector<std::vector<float>>(size, std::vector<float>(size, 0.0f));
        current  = std::vector<std::vector<float>>(size, std::vector<float>(size, 0.0f));
        next     = std::vector<std::vector<float>>(size, std::vector<float>(size, 0.0f));
    }

    int gridSize = 100;
    std::vector<std::vector<float>> previous, current, next;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(VibratingMembrane)
};



#endif //VIBRATING_MEMBRANE_H
