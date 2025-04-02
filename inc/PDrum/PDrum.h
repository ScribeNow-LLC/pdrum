#ifndef P_DRUM_H
#define P_DRUM_H

#include <Components/Knob/KnobComponent.h>
#include <juce_audio_devices/juce_audio_devices.h>
#include <juce_audio_processors/juce_audio_processors.h>

/**
 * @brief Audio processor for the PDrum plugin.
 */
class PDrum final : public juce::AudioProcessor {
public:
    /**
     * @brief Constructor for the PDrum processor.
     */
    PDrum();

    /**
     * @brief Destructor for the PDrum processor.
     */
    ~PDrum() override = default;

    /**
     * @brief Prepare the processor for playback.
     * @param sampleRate The sample rate of the audio stream.
     * @param samplesPerBlock The number of samples per block to process.
     */
    void prepareToPlay(double sampleRate, int samplesPerBlock) override;

    /**
     * @brief Release any resources used by the processor.
     */
    void releaseResources() override {
    }

    /**
     * @brief Check if the processor supports the given bus layout.
     * @param layouts The bus layout to check for support.
     * @return True if the layout is supported, false otherwise.
     */
    bool isBusesLayoutSupported(const BusesLayout &layouts) const override;

    /**
     * @brief Process a block of audio and MIDI data.
     */
    void processBlock(juce::AudioBuffer<float> &, juce::MidiBuffer &) override;

    /**
     * @brief Create an editor for the processor.
     * @return A pointer to the created editor.
     */
    juce::AudioProcessorEditor *createEditor() override;

    /**
     * @brief Check if the processor has an editor.
     * @return True if the processor has an editor, false otherwise.
     */
    bool hasEditor() const override { return true; }

    /**
     * @brief Get the name of the processor.
     * @return The name of the processor as a string.
     */
    const juce::String getName() const override { return "PDrum"; }

    /**
     * @brief Check if the processor accepts MIDI input.
     * @return True if the processor accepts MIDI input, false otherwise.
     */
    bool acceptsMidi() const override { return false; }

    /**
     * @brief Check if the processor produces MIDI output.
     * @return True if the processor produces MIDI output, false otherwise.
     */
    bool producesMidi() const override { return false; }

    /**
     * @brief Get the tail length in seconds.
     * @return The tail length in seconds. For this processor, it is 0.0
     */
    double getTailLengthSeconds() const override { return 0.0; }

    /**
     * @brief Get the number of programs supported by the processor.
     * @return The number of programs supported. For this processor, it is 1.
     */
    int getNumPrograms() override { return 1; }

    /**
     * @brief Get the current program index.
     * @return The current program index. For this processor, it is always 0.
     */
    int getCurrentProgram() override { return 0; }

    /**
     * @brief Set the current program index.
     */
    void setCurrentProgram(int) override {
    }

    /**
     * @brief Get the name of a specific program.
     * @return The name of the program. For this processor, it returns an empty
     */
    const juce::String getProgramName(int) override { return {}; }

    /**
     * @brief Change the name of a specific program.
     */
    void changeProgramName(int, const juce::String &) override {
    }

    /**
     * @brief Get the state information of the processor.
     */
    void getStateInformation(juce::MemoryBlock &) override {
    }

    /**
     * @brief Set the state information of the processor.
     */
    void setStateInformation(const void *, int) override {
    }

    /**
     * @brief Get the MIDI message collector.
     * @return A reference to the MIDI message collector.
     */
    juce::MidiMessageCollector &getMidiMessageCollector() noexcept {
        return midiMessageCollector;
    }

    /**
     * @brief Gets the value tree state for the parameters.
     * @return A reference to the AudioProcessorValueTreeState object.
     */
    juce::AudioProcessorValueTreeState &getParameters() { return parameters; }

private:
    /** MIDI message collector to handle incoming MIDI messages. */
    juce::MidiMessageCollector midiMessageCollector{};

    /** Audio processor value tree state for managing parameters. */
    juce::AudioProcessorValueTreeState parameters;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(PDrum)
};


#endif // P_DRUM_H
