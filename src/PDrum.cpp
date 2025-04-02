#include <PDrum/PDrum.h>
#include <PDrum/PDrumEditor.h>

/**
 * @brief Constructor for the PDrum processor.
 */
PDrum::PDrum() : AudioProcessor(BusesProperties().withOutput(
                     "Output", juce::AudioChannelSet::stereo(), true)),
                 parameters(*this, nullptr, "PARAMETERS",
                            {
                                std::make_unique<juce::AudioParameterFloat>(
                                    "membraneTension", "Tension", 10.0f,
                                    300.0f,
                                    100.0f),
                                std::make_unique<juce::AudioParameterFloat>(
                                    "membraneSize", "Size",
                                    0.2f, 10.0f, 1.0f),
                            }),
                 membraneModel(parameters) {
}

/**
 * @brief Prepare the processor for playback.
 * @param sampleRate The sample rate of the audio stream.
 * @param samplesPerBlock The number of samples per block to process.
 */
void PDrum::prepareToPlay(const double sampleRate, int samplesPerBlock) {
    midiMessageCollector.reset(sampleRate);
}

/**
 * @brief Check if the processor supports the given bus layout.
 * @param layouts The bus layout to check for support.
 * @return True if the layout is supported, false otherwise.
 */
bool PDrum::isBusesLayoutSupported(const BusesLayout &layouts) const {
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    return true;
}

/**
 * @brief Process a block of audio and MIDI data.
 */
void PDrum::processBlock(juce::AudioBuffer<float> &buffer,
                         juce::MidiBuffer &midiMessages) {
    auto numSamples = buffer.getNumSamples();
    auto numChannels = buffer.getNumChannels();

    float membraneSample = 0.0f;

    for (int sample = 0; sample < numSamples; ++sample) {
        // Update only if the model says to (e.g., every 10 samples internally)
        membraneSample = membraneModel.processSample();

        for (int channel = 0; channel < numChannels; ++channel) {
            buffer.setSample(channel, sample, membraneSample);
        }
    }
}

/**
 * @brief Create an editor for the processor.
 * @return A pointer to the created editor.
 */
juce::AudioProcessorEditor *PDrum::createEditor() {
    return new PDrumEditor(*this);
}

/**
 * @brief Factory function to create an instance of the PDrum
 * processor.
 * @return A pointer to the created processor instance.
 */
juce::AudioProcessor *JUCE_CALLTYPE createPluginFilter() { return new PDrum(); }
