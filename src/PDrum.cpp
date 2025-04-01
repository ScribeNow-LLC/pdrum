#include <PDrum/PDrum.h>
#include <PDrum/PDrumEditor.h>

/**
 * @brief Constructor for the PDrum processor.
 */
PDrum::PDrum() :
    AudioProcessor(BusesProperties().withOutput(
            "Output", juce::AudioChannelSet::stereo(), true)) {}

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
    juce::ScopedNoDenormals noDenormals;
    const auto totalNumInputChannels = getTotalNumInputChannels();
    const auto totalNumOutputChannels = getTotalNumOutputChannels();

    midiMessageCollector.removeNextBlockOfMessages(midiMessages,
                                                   buffer.getNumSamples());

    for (int i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
        buffer.clear(i, 0, buffer.getNumSamples());

    /// DO PROCESSING HERE
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
