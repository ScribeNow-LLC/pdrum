#include "PDrumEditor.h"

/**
 * @brief Constructor for the NBandParametricEQEditor class.
 * @param p A reference to the NBandParametricEQ processor that this editor
 * is associated with.
 */
PDrumEditor::PDrumEditor(PDrum &p) :
    AudioProcessorEditor(p), processor(p), /*membrane(p.getModel()),*/
    resonator(p.getParameters(), p.getModel()),
    membraneSizeKnob(p.getParameters(), "membraneSize", "Size"),
    membraneTensionKnob(p.getParameters(), "membraneTension", "Tension"),
    depthKnob(p.getParameters(), "depth", "Depth"),
    randomnessKnob(p.getParameters(), "randomness", "Randomness") {
    addAndMakeVisible(midiKeyboardComponent);
    //addAndMakeVisible(membrane);
    addAndMakeVisible(resonator);
    addAndMakeVisible(membraneSizeKnob);
    addAndMakeVisible(membraneTensionKnob);
    addAndMakeVisible(depthKnob);
    addAndMakeVisible(randomnessKnob);
    midiKeyboardComponent.setMidiChannel(2);
    midiKeyboardState.addListener(&processor.getMidiMessageCollector());
    setSize(300, 400);
    setResizable(true, true);
    setResizeLimits(300, 400, 1000, 600);
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

    DBG(area.getWidth() << " " << area.getHeight());

    const auto keyboardArea = area.removeFromBottom(80).reduced(8);
    midiKeyboardComponent.setBounds(keyboardArea);

    constexpr int knobWidth = 75;

    const auto drumArea = area.removeFromLeft(area.getWidth() - knobWidth);
    resonator.setBounds(drumArea.reduced(8));

    auto knobArea = area;
    const auto sizeKnobArea = knobArea.removeFromTop(knobWidth);
    membraneSizeKnob.setBounds(sizeKnobArea.reduced(8));

    const auto depthArea = knobArea.removeFromTop(knobWidth);
    depthKnob.setBounds(depthArea.reduced(8));

    const auto tensionArea = knobArea.removeFromTop(knobWidth);
    membraneTensionKnob.setBounds(tensionArea.reduced(8));

    const auto randomnessArea = knobArea.removeFromTop(knobWidth);
    randomnessKnob.setBounds(randomnessArea.reduced(8));

    /// TODO - create a Component to draw a 3D cylinder to represent the drum
}

/**
 * @brief Timer callback function to update the editor.
 */
void PDrumEditor::timerCallback() { repaint(); }
