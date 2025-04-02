#ifndef MODAL_RESONATOR_H
#define MODAL_RESONATOR_H

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
    explicit ModalResonator(juce::AudioProcessorValueTreeState &state) :
        parameters(state) {
        widthParam = dynamic_cast<juce::AudioParameterFloat *>(
                parameters.getParameter("membraneSize"));
        depthParam = dynamic_cast<juce::AudioParameterFloat *>(
                parameters.getParameter("depth"));

        setSize(400, 400);
        openGLContext.setContinuousRepainting(true);
    }

    /**
     * @brief Destructor for the ModalResonator class.
     */
    ~ModalResonator() override { openGLContext.detach(); }

    /**
     * @brief Initialise the OpenGL context.
     */
    void initialise() override {
        juce::gl::glEnable(juce::gl::GL_DEPTH_TEST);
        juce::gl::glShadeModel(juce::gl::GL_SMOOTH);
    }

    /**
     * @brief Shutdown the OpenGL context.
     */
    void shutdown() override {}

    /**
     * @brief Render the OpenGL content.
     */
    void render() override {
        if (!juce::OpenGLHelpers::isContextActive())
            return;

        /// Retrieve parameter values
        const float widthValue = widthParam ? widthParam->get() / 10.0f : 0.5f;
        const float depthValue = depthParam ? depthParam->get() / 10.0f : 0.5f;

        /// Map parameter values to cylinder dimensions
        const float radius = juce::jmap(widthValue, 0.0f, 1.0f, 0.2f, 1.0f);
        const float height = juce::jmap(depthValue, 0.0f, 1.0f, 0.2f, 1.5f);

        /// Clear screen and enable depth test
        juce::OpenGLHelpers::clear(juce::Colours::black);
        juce::gl::glEnable(juce::gl::GL_DEPTH_TEST);

        /// Compute viewport dimensions in pixels
        const auto viewportWidth = static_cast<GLint>(
                getWidth() * openGLContext.getRenderingScale());
        const auto viewportHeight = static_cast<GLint>(
                getHeight() * openGLContext.getRenderingScale());
        juce::gl::glViewport(0, 0, viewportWidth, viewportHeight);

        /// Set up the projection matrix based on the viewport dimensions.
        juce::gl::glMatrixMode(juce::gl::GL_PROJECTION);
        juce::gl::glLoadIdentity();
        setPerspective(45.0f,
                       static_cast<float>(viewportWidth) /
                               static_cast<float>(viewportHeight),
                       0.1f, 10.0f);

        /// Set up the modelview matrix.
        juce::gl::glMatrixMode(juce::gl::GL_MODELVIEW);
        juce::gl::glLoadIdentity();
        juce::gl::glTranslatef(0.0f, 0.0f, -3.0f);

        /// Apply transformations to the modelview matrix.
        juce::gl::glRotatef(30.0f, 1.0f, 0.0f, 0.0f);
        juce::gl::glRotatef(
                static_cast<float>(juce::Time::getMillisecondCounter()) * 0.03f,
                0.0f, 1.0f, 0.0f);

        // Draw the cylinder.
        drawCylinder(radius, height, 32);
    }

    /**
     * @brief Draw a cylinder with the specified radius, height, and number of
     * segments.
     * @param radius The radius of the cylinder.
     * @param height The height of the cylinder.
     * @param segments The number of segments to approximate the cylinder.
     */
    static void drawCylinder(const float radius, const float height,
                             const int segments) {
        const float halfHeight = height / 2.0f;
        /// Draw cylinder sides.
        juce::gl::glColor3f(0.2f, 0.6f, 1.0f);
        juce::gl::glBegin(juce::gl::GL_QUAD_STRIP);
        for (int i = 0; i <= segments; ++i) {
            const float angle = juce::MathConstants<float>::twoPi *
                                static_cast<float>(i) /
                                static_cast<float>(segments);
            const float x = std::cos(angle);
            const float z = std::sin(angle);
            juce::gl::glVertex3f(radius * x, -halfHeight, radius * z);
            juce::gl::glVertex3f(radius * x, halfHeight, radius * z);
        }
        juce::gl::glEnd();

        /// Draw top cap.
        juce::gl::glBegin(juce::gl::GL_TRIANGLE_FAN);
        juce::gl::glColor3f(0.4f, 0.8f, 1.0f);
        juce::gl::glVertex3f(0.0f, halfHeight, 0.0f);
        for (int i = 0; i <= segments; ++i) {
            const float angle = juce::MathConstants<float>::twoPi *
                                static_cast<float>(i) /
                                static_cast<float>(segments);
            const float x = std::cos(angle);
            const float z = std::sin(angle);
            juce::gl::glVertex3f(radius * x, halfHeight, radius * z);
        }
        juce::gl::glEnd();

        /// Draw bottom cap.
        juce::gl::glBegin(juce::gl::GL_TRIANGLE_FAN);
        juce::gl::glColor3f(0.1f, 0.3f, 0.6f);
        juce::gl::glVertex3f(0.0f, -halfHeight, 0.0f);
        for (int i = 0; i <= segments; ++i) {
            const float angle = juce::MathConstants<float>::twoPi *
                                static_cast<float>(i) /
                                static_cast<float>(segments);
            const float x = std::cos(angle);
            const float z = std::sin(angle);
            juce::gl::glVertex3f(radius * x, -halfHeight, radius * z);
        }
        juce::gl::glEnd();
    }

    /**
     * @brief Set the perspective projection matrix.
     * @param fovY Field of view in the Y direction (in degrees).
     * @param aspect Aspect ratio (width / height).
     * @param zNear Near clipping plane distance.
     * @param zFar Far clipping plane distance.
     */
    static void setPerspective(const float fovY, const float aspect,
                               const float zNear, const float zFar) {
        const float fH =
                std::tan(fovY * juce::MathConstants<float>::pi / 360.0f) *
                zNear;
        const float fW = fH * aspect;
        juce::gl::glFrustum(-fW, fW, -fH, fH, zNear, zFar);
    }

private:
    /** Reference to the processor's parameter tree */
    juce::AudioProcessorValueTreeState &parameters;

    /** Width and depth parameters */
    juce::AudioParameterFloat *widthParam = nullptr;
    juce::AudioParameterFloat *depthParam = nullptr;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR(ModalResonator)
};

#endif // MODAL_RESONATOR_H
