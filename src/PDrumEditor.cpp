#include <PDrum/PDrumEditor.h>

/**
 * @brief Constructor for the NBandParametricEQEditor class.
 * @param p A reference to the NBandParametricEQ processor that this editor
 * is associated with.
 */
PDrumEditor::PDrumEditor(PDrum &p) : AudioProcessorEditor(p), processor(p),
                                     membrane(p.getModel()),
                                     membraneSizeKnob(
                                         p.getParameters(), "membraneSize",
                                         "Size"),
                                     membraneTensionKnob(
                                         p.getParameters(), "membraneTension",
                                         "Tension"),
                                    depthKnob(
                                        p.getParameters(), "depth", "Depth") {
    addAndMakeVisible(midiKeyboardComponent);
    addAndMakeVisible(membrane);
    addAndMakeVisible(membraneSizeKnob);
    addAndMakeVisible(membraneTensionKnob);
    addAndMakeVisible(depthKnob);
    midiKeyboardComponent.setMidiChannel(2);
    midiKeyboardState.addListener(&processor.getMidiMessageCollector());
    setSize(500, 400);
    setResizable(true, true);
    setResizeLimits(300, 150, 1000, 600);
    startTimerHz(60);
}

/**
 * @brief Paint the editor's background.
 * @param g The graphics context used for painting.
 */
void PDrumEditor::paint(juce::Graphics &g) {
    g.fillAll(getLookAndFeel().findColour(
        juce::ResizableWindow::backgroundColourId));
}

/**
 * @brief Resize the editor.
 */
void PDrumEditor::resized() {
    juce::Rectangle<int> area = getLocalBounds();

    const auto keyboardArea = area.removeFromBottom(80).reduced(8);
    midiKeyboardComponent.setBounds(keyboardArea);

    const auto drumArea = area.removeFromTop(area.getHeight() * 3 / 5).reduced(8);
    membrane.setBounds(drumArea.reduced(8));

    auto knobArea = area.reduced(8);
    const auto sizeKnobArea = knobArea.removeFromLeft(knobArea.getWidth() / 3);
    membraneSizeKnob.setBounds(sizeKnobArea);

    const auto tensionArea = knobArea.removeFromLeft(knobArea.getWidth() / 2);
    membraneTensionKnob.setBounds(tensionArea);

    const auto depthArea = knobArea;
    depthKnob.setBounds(depthArea);
}

/**
 * @brief Timer callback function to update the editor.
 */
void PDrumEditor::timerCallback() { repaint(); }
