#include "PDrumEditor.h"

/**
 * @brief Constructor for the NBandParametricEQEditor class.
 * @param p A reference to the NBandParametricEQ processor that this editor
 * is associated with.
 */
PDrumEditor::PDrumEditor(PDrum &p) :
    AudioProcessorEditor(p), processor(p), membrane(p.getModel()),
    membraneSizeKnob(p.getParameters(), "membraneSize", "Size"),
    membraneTensionKnob(p.getParameters(), "membraneTension", "Tension"),
    depthKnob(p.getParameters(), "depth", "Depth"),
    randomnessKnob(p.getParameters(), "randomness", "Randomness") {
    addAndMakeVisible(midiKeyboardComponent);
    addAndMakeVisible(membrane);
    addAndMakeVisible(membraneSizeKnob);
    addAndMakeVisible(membraneTensionKnob);
    addAndMakeVisible(depthKnob);
    addAndMakeVisible(randomnessKnob);
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

    auto drumArea =
            area.removeFromTop(area.getHeight() * 3 / 5).reduced(8);
    membrane.setBounds(drumArea.removeFromLeft(drumArea.getWidth() / 2).reduced(8));

    auto knobArea = area.reduced(8);
    const auto sizeKnobArea = knobArea.removeFromLeft(knobArea.getWidth() / 4);
    membraneSizeKnob.setBounds(sizeKnobArea);

    const auto tensionArea = knobArea.removeFromLeft(knobArea.getWidth() / 3);
    membraneTensionKnob.setBounds(tensionArea);

    const auto depthArea = knobArea.removeFromLeft(knobArea.getWidth() / 2);
    depthKnob.setBounds(depthArea);

    const auto randomnessArea = knobArea;
    randomnessKnob.setBounds(randomnessArea);

    /// TODO - create a Component to draw a 3D cylinder to represent the drum
}

/**
 * @brief Timer callback function to update the editor.
 */
void PDrumEditor::timerCallback() { repaint(); }
