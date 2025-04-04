#include "PDrum.h"
#include "PDrumEditor.h"

/**
 * @brief Constructor for the PDrum processor.
 */
PDrum::PDrum() :
    AudioProcessor(BusesProperties().withOutput(
            "Output", juce::AudioChannelSet::stereo(), true)),
    parameters(*this, nullptr, "PARAMETERS",
               {
                       std::make_unique<juce::AudioParameterFloat>(
                               "membraneTension", "Tension", 0.01f, 1.0f, 0.5f),
                       std::make_unique<juce::AudioParameterFloat>(
                               "membraneSize", "Size", 0.75f, 10.0f, 3.0f),
                       std::make_unique<juce::AudioParameterFloat>(
                               "depth", "Depth", 0.75f, 10.0f, 3.0f),
                       std::make_unique<juce::AudioParameterFloat>(
                               "randomness", "Randomness", 0.0f, 50.0f, 5.0f),
               }),
#ifdef DEBUG
    membraneModel(parameters),
#else
    membraneModel(parameters, 256),
#endif
    resonatorModel(parameters) {
}

/**
 * @brief Prepare the processor for playback.
 * @param sampleRate The sample rate of the audio stream.
 * @param samplesPerBlock The number of samples per block to process.
 */
void PDrum::prepareToPlay(const double sampleRate, int samplesPerBlock) {
    midiMessageCollector.reset(sampleRate);
    resonatorModel.setParameters(1.0f, 1.0f, static_cast<float>(sampleRate));
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
    const int numSamples = buffer.getNumSamples();
    const int numChannels = buffer.getNumChannels();
    const float inverseSampleRate = 1.0f / static_cast<float>(getSampleRate());
    /// Clear output buffer
    buffer.clear();
    /// Process MIDI input
    midiMessageCollector.removeNextBlockOfMessages(midiMessages, numSamples);
    for (const auto &metadata: midiMessages) {
        if (const auto &message = metadata.getMessage(); message.isNoteOn())
                [[likely]] {
            membraneModel.exciteCenter(0.9f);
        }
    }
    /// Get write pointer for channel 0 (mono processing)
    float *out = buffer.getWritePointer(0);
    for (int i = 0; i < numSamples; ++i) {
        const float membraneOut =
                membraneModel.processSample(inverseSampleRate);
        const float resonatorOut = resonatorModel.process(membraneOut);
        out[i] = resonatorOut;
    }
    /// Duplicate mono output to remaining channels
    if (numChannels > 1) {
        for (int ch = 1; ch < numChannels; ++ch) {
            buffer.copyFrom(ch, 0, out, numSamples);
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
