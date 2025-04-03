#include "ModalResonator.h"

/**
 * @brief Constructor for the ModalResonator class.
 * @param state Reference to the AudioProcessorValueTreeState for parameter
 * @param membraneModel Reference to the VibratingMembraneModel for
 * management.
 */
ModalResonator::ModalResonator(juce::AudioProcessorValueTreeState &state,
                               VibratingMembraneModel &membraneModel) :
    parameters(state), m_membraneModel(membraneModel) {
    widthParam = dynamic_cast<juce::AudioParameterFloat *>(
            parameters.getParameter("membraneSize"));
    depthParam = dynamic_cast<juce::AudioParameterFloat *>(
            parameters.getParameter("depth"));
    setSize(400, 400);
    openGLContext.setContinuousRepainting(true);
    openGLContext.setSwapInterval(60);
}

/**
 * @brief Initialise the OpenGL context.
 */
void ModalResonator::initialise() {
    juce::gl::glEnable(juce::gl::GL_DEPTH_TEST);
    juce::gl::glShadeModel(juce::gl::GL_SMOOTH);
}

/**
 * @brief Render the OpenGL content.
 */
void ModalResonator::render() {
    if (!juce::OpenGLHelpers::isContextActive())
        return;
    /// Compute time delta
    const uint32_t currentTime = juce::Time::getMillisecondCounter();
    const float deltaTime =
            (lastFrameTime > 0)
                    ? static_cast<float>(currentTime - lastFrameTime) / 1000.0f
                    : 0.0f;
    lastFrameTime = currentTime;
    /// Update rotation angle (degrees per second)
    rotationAngle += 10.0f * deltaTime; // e.g., 45 degrees per second
    if (rotationAngle >= 360.0f)
        rotationAngle -= 360.0f;
    /// Retrieve parameter values
    const float widthValue = widthParam ? widthParam->get() / 10.0f : 0.5f;
    const float depthValue = depthParam ? depthParam->get() / 10.0f : 0.5f;

    /// Map parameter values to cylinder dimensions
    const float radius = juce::jmap(widthValue, 0.0f, 1.0f, 0.2f, 1.0f);
    const float height = juce::jmap(depthValue, 0.0f, 1.0f, 0.2f, 1.5f);

    /// Clear screen and enable depth test
    juce::OpenGLHelpers::clear(juce::Colours::black);
    juce::gl::glEnable(juce::gl::GL_BLEND);
    juce::gl::glBlendFunc(juce::gl::GL_SRC_ALPHA, juce::gl::GL_ONE_MINUS_SRC_ALPHA);


    /// Compute viewport dimensions in pixels
    const auto viewportWidth =
            static_cast<GLint>(getWidth() * openGLContext.getRenderingScale());
    const auto viewportHeight =
            static_cast<GLint>(getHeight() * openGLContext.getRenderingScale());
    juce::gl::glViewport(0, 0, viewportWidth, viewportHeight);

    /// Set up the projection matrix
    juce::gl::glMatrixMode(juce::gl::GL_PROJECTION);
    juce::gl::glLoadIdentity();
    setPerspective(45.0f,
                   static_cast<float>(viewportWidth) /
                           static_cast<float>(viewportHeight),
                   0.1f, 10.0f);

    /// Set up modelview matrix
    juce::gl::glMatrixMode(juce::gl::GL_MODELVIEW);
    juce::gl::glLoadIdentity();
    juce::gl::glTranslatef(0.0f, 0.0f, -3.0f);
    juce::gl::glRotatef(30.0f, 1.0f, 0.0f, 0.0f); // tilt
    juce::gl::glRotatef(rotationAngle, 0.0f, 1.0f, 0.0f); // spin

    /// Draw the cylinder
    drawCylinder(radius, height, 32);
    drawMembraneMesh(radius, height);
}

/**
 * @brief Draw a cylinder with the specified radius, height, and number of
 * segments.
 * @param radius The radius of the cylinder.
 * @param height The height of the cylinder.
 * @param segments The number of segments to approximate the cylinder.
 */
void ModalResonator::drawCylinder(const float radius, const float height,
                                  const int segments) {
    const float halfHeight = height / 2.0f;

    // Set color for wireframe
    juce::gl::glColor3f(0.0f, 0.5f, 0.0f);

    // Draw vertical lines (side edges)
    juce::gl::glBegin(juce::gl::GL_LINES);
    for (int i = 0; i <= segments; ++i) {
        const float angle = juce::MathConstants<float>::twoPi *
                            static_cast<float>(i) /
                            static_cast<float>(segments);
        const float x = std::cos(angle) * radius;
        const float z = std::sin(angle) * radius;
        juce::gl::glVertex3f(x, -halfHeight, z);
        juce::gl::glVertex3f(x, halfHeight, z);
    }
    juce::gl::glEnd();

    // Draw top circle
    juce::gl::glBegin(juce::gl::GL_LINE_LOOP);
    for (int i = 0; i <= segments; ++i) {
        const float angle = juce::MathConstants<float>::twoPi *
                            static_cast<float>(i) /
                            static_cast<float>(segments);
        const float x = std::cos(angle) * radius;
        const float z = std::sin(angle) * radius;
        juce::gl::glVertex3f(x, halfHeight, z);
    }
    juce::gl::glEnd();

    // Draw bottom circle
    juce::gl::glBegin(juce::gl::GL_LINE_LOOP);
    for (int i = 0; i <= segments; ++i) {
        const float angle = juce::MathConstants<float>::twoPi *
                            static_cast<float>(i) /
                            static_cast<float>(segments);
        const float x = std::cos(angle) * radius;
        const float z = std::sin(angle) * radius;
        juce::gl::glVertex3f(x, -halfHeight, z);
    }
    juce::gl::glEnd();
}

void ModalResonator::drawMembraneMesh(const float radius,
                                      const float height) const {
    const auto &current = m_membraneModel.getCurrentBuffer();
    const auto &isInside = m_membraneModel.getIsInsideMask();
    const int gridResolution = m_membraneModel.getGridResolution();

    const float halfHeight = height / 2.0f;
    const float logDenom = std::log10(101.0f);

    juce::gl::glPointSize(2.0f); // Optional: bigger point size
    juce::gl::glBegin(juce::gl::GL_POINTS);

    for (int y = 0; y < gridResolution; ++y) {
        for (int x = 0; x < gridResolution; ++x) {
            const int idx = y * gridResolution + x;
            if (!isInside[idx])
                continue;

            const float value = current[idx];
            const float logValue =
                    std::log10(1.0f + std::abs(value) * 300.0f) / logDenom;
            const float scaled = juce::jlimit(0.0f, 1.0f, logValue);

            const float r = static_cast<float>(x) /
                                    static_cast<float>(gridResolution - 1) *
                                    2.0f -
                            1.0f;
            const float s = static_cast<float>(y) /
                                    static_cast<float>(gridResolution - 1) *
                                    2.0f -
                            1.0f;

            // Convert to polar and clamp to unit circle
            const float d = std::sqrt(r * r + s * s);
            if (d > 1.0f)
                continue; // outside drum head

            // Map to circle in XZ plane
            const float theta = std::atan2(s, r);
            const float radial = d * radius;
            const float x3D = std::cos(theta) * radial;
            const float z3D = std::sin(theta) * radial;
            const float y3D = halfHeight + value * 0.1f; // height scaling

            const float alpha = scaled; // or pow(scaled, 2.0f)

            if (value >= 0.0f)
                juce::gl::glColor4f(0.0f, scaled, 0.0f, alpha);
            else
                juce::gl::glColor4f(scaled, 0.0f, 0.0f, alpha);

            juce::gl::glVertex3f(x3D, y3D, z3D);
        }
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
void ModalResonator::setPerspective(const float fovY, const float aspect,
                                    const float zNear, const float zFar) {
    const float fH =
            std::tan(fovY * juce::MathConstants<float>::pi / 360.0f) * zNear;
    const float fW = fH * aspect;
    juce::gl::glFrustum(-fW, fW, -fH, fH, zNear, zFar);
}
