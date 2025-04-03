#ifndef MODAL_RESONATOR_H
#define MODAL_RESONATOR_H

#include <juce_audio_processors/juce_audio_processors.h>
#include <juce_opengl/juce_opengl.h>

/**
 * @brief Class representing a 3D modal resonator simulation GUI.
 */
class ModalResonator final : public juce::OpenGLAppComponent {
public:
    /**
     * @brief Constructor for the ModalResonator class.
     * @param state Reference to the AudioProcessorValueTreeState for parameter
     * management.
     */
    explicit ModalResonator(juce::AudioProcessorValueTreeState &state);

    /**
     * @brief Destructor for the ModalResonator class.
     */
    ~ModalResonator() override { openGLContext.detach(); }

    /**
     * @brief Initialise the OpenGL context.
     */
    void initialise() override;

    /**
     * @brief Shutdown the OpenGL context.
     */
    void shutdown() override {}

    /**
     * @brief Render the OpenGL content.
     */
    void render() override;

    /**
     * @brief Draw a cylinder with the specified radius, height, and number of
     * segments.
     * @param radius The radius of the cylinder.
     * @param height The height of the cylinder.
     * @param segments The number of segments to approximate the cylinder.
     */
    static void drawCylinder(float radius, float height, int segments);

    /**
     * @brief Set the perspective projection matrix.
     * @param fovY Field of view in the Y direction (in degrees).
     * @param aspect Aspect ratio (width / height).
     * @param zNear Near clipping plane distance.
     * @param zFar Far clipping plane distance.
     */
    static void setPerspective(float fovY, float aspect, float zNear,
                               float zFar);

private:
    /** Reference to the processor's parameter tree */
    juce::AudioProcessorValueTreeState &parameters;

    /** Width and depth parameters */
    juce::AudioParameterFloat *widthParam = nullptr;
    juce::AudioParameterFloat *depthParam = nullptr;

    /** Rotation parameters */
    float rotationAngle = 0.0f;
    juce::int64 lastFrameTime = 0;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModalResonator)
};

#endif // MODAL_RESONATOR_H
