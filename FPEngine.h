#ifndef LAB08_LAB08_ENGINE_H
#define LAB08_LAB08_ENGINE_H

#include <CSCI441/ArcballCam.hpp>
#include <CSCI441/materials.hpp>
#include <CSCI441/objects.hpp>
#include <CSCI441/OpenGLEngine.hpp>
#include <CSCI441/ShaderProgram.hpp>
#include <CSCI441/ModelLoader.hpp>
#include <CSCI441/TextureUtils.hpp>

class Lab08Engine final : public CSCI441::OpenGLEngine {
public:
    //***************************************************************************
    // Engine Interface

    Lab08Engine();

    void run() final;

    //***************************************************************************
    // Event Handlers

    /// \desc handle any key events inside the engine
    /// \param key key as represented by GLFW_KEY_ macros
    /// \param action key event action as represented by GLFW_ macros
    void handleKeyEvent(GLint key, GLint action);

    /// \desc handle any mouse button events inside the engine
    /// \param button mouse button as represented by GLFW_MOUSE_BUTTON_ macros
    /// \param action mouse event as represented by GLFW_ macros
    void handleMouseButtonEvent(GLint button, GLint action);

    /// \desc handle any cursor movement events inside the engine
    /// \param currMousePosition the current cursor position
    void handleCursorPositionEvent(glm::vec2 currMousePosition);

    /// \desc handle any scroll events inside the engine
    /// \param offset the current scroll offset
    void handleScrollEvent(glm::vec2 offset);

    /// \desc value off-screen to represent mouse has not begun interacting with window yet
    static constexpr GLfloat MOUSE_UNINITIALIZED = -9999.0f;

private:
    //Model Things
    CSCI441::ModelLoader *testModel;
    CSCI441::ModelLoader *poolGrass;
    CSCI441::ModelLoader *poolWood;
    CSCI441::ModelLoader *poolHoles;
    CSCI441::ModelLoader *mines;
    CSCI441::ModelLoader *hand;
    CSCI441::ModelLoader *stick;

    //***************************************************************************
    // Engine Setup

    void mSetupGLFW() final;
    void mSetupOpenGL() final;
    void mSetupShaders() final;
    void mSetupBuffers() final;
    void mSetupTextures() final;
    void mSetupScene() final;

    //***************************************************************************
    // Engine Cleanup

    void mCleanupScene() final;
    void mCleanupTextures() final;
    void mCleanupBuffers() final;
    void mCleanupShaders() final;

    //***************************************************************************
    // Engine Rendering & Updating

    /// \desc draws everything to the scene from a particular point of view
    /// \param viewMtx the current view matrix for our camera
    /// \param projMtx the current projection matrix for our camera
    void _renderScene(glm::mat4 viewMtx, glm::mat4 projMtx) const;
    /// \desc handles moving our FreeCam as determined by keyboard input
    void _updateScene();

    //***************************************************************************
    // Input Tracking (Keyboard & Mouse)

    /// \desc tracks the number of different keys that can be present as determined by GLFW
    static constexpr GLuint NUM_KEYS = GLFW_KEY_LAST;
    /// \desc boolean array tracking each key state.  if true, then the key is in a pressed or held
    /// down state.  if false, then the key is in a released state and not being interacted with
    GLboolean _keys[NUM_KEYS];

    /// \desc last location of the mouse in window coordinates
    glm::vec2 _mousePosition;
    /// \desc current state of the left mouse button
    GLint _leftMouseButtonState;

    //***************************************************************************
    // Camera Information

    /// \desc the arcball camera in our world
    CSCI441::ArcballCam* _pArcballCam;

    //***************************************************************************
    // Illumination Information

    /// \desc type of the light
    /// \desc 0 - point
    /// \desc 1 - directional
    /// \desc 2 - spot
    GLuint _lightType;
    /// \desc position of the light for point or spotlight
    float spotLightSwingAngle = 0;
    float spotLightDTheta = 0.01;
    glm::vec3 _spotLightPos;
    glm::vec3 _spotLightDir;
    /// \desc direction of the light for directional or spotlight
    glm::vec3 _lightDir;
    /// \desc angle of our spotlight
    GLfloat _lightAngle;

    //***************************************************************************
    // VAO & Object Information

    float meterHeight = 0.1;
    float meterStep = -0.01;
    int cueState = 0; // 0: nothing happening, 1: angle chosen, 2: FIRE!

    static constexpr GLuint NUM_VAOS = 3;
    /// \desc used to index through our VAO/VBO/IBO array to give named access
    enum VAO_ID {
        /// \desc the platform that represents our ground for everything to appear on
        PLATFORM = 0
    };
    /// \desc VAO for our objects
    GLuint _vaos[NUM_VAOS];
    /// \desc VBO for our objects
    GLuint _vbos[NUM_VAOS];
    /// \desc IBO for our objects
    GLuint _ibos[NUM_VAOS];
    /// \desc the number of points that make up our VAO
    GLsizei _numVAOPoints[NUM_VAOS];

    /// \desc creates the platform object
    /// \param [in] vao VAO descriptor to bind
    /// \param [in] vbo VBO descriptor to bind
    /// \param [in] ibo IBO descriptor to bind
    /// \param [out] numVAOPoints sets the number of vertices that make up the IBO array
    void _createPlatform(GLuint vao, GLuint vbo, GLuint ibo, GLsizei &numVAOPoints) const;

    //***************************************************************************
    // Shader Program Information

    /// \desc shader program for cue ball weirdness
    CSCI441::ShaderProgram* _goofyShaderProgram;
    /// \desc stores the locations of all of our shader uniforms
    struct goofyShaderProgramUniformLocations {
        /// \desc vector in the direction of the incoming hit
        GLfloat hitVector;
        /// \desc time since hit. Used for animated movement
        GLfloat timeSince;
        /// \desc precomputed MVP matrix location
        GLint mvpMatrix;
        /// \desc model matrix location
        GLint modelMatrix;
        /// \desc normal matrix location
        GLint normalMatrix;
        /// \desc camera position location
        GLint eyePos;
        /// \desc light position location - used for point/spot
        GLint spotLightPos;
        /// \desc light position location - used for point/spot
        GLint spotLightDir;
        /// \desc light direction location - used for directional/spot
        GLint dirLightDir;
        /// \desc light cone angle location - used for spot
        GLint lightCutoff;
        /// \desc color of the light location
        GLint lightColor;
        /// \desc type of the light location - 0 point 1 directional 2 spot
        GLint lightType;
        /// \desc material diffuse color location
        GLint materialDiffColor;
        /// \desc material specular color location
        GLint materialSpecColor;
        /// \desc material shininess factor location
        GLint materialShininess;
        /// \desc material ambient color location
        GLint materialAmbColor;
    } _goofyShaderProgramUniformLocations;
    /// \desc stores the locations of all of our shader attributes
    struct goofyShaderProgramAttributeLocations {
        /// \desc vertex position location
        GLint vPos;
        /// \desc vertex normal location
        GLint vNormal;
    } _goofyShaderProgramAttributeLocations;

    /// \desc shader program that performs Gouraud Shading with the Phong Illumination Model
    CSCI441::ShaderProgram* _goodShaderProgram;
    /// \desc stores the locations of all of our shader uniforms
    struct GouraudShaderProgramUniformLocations {
        /// \desc precomputed MVP matrix location
        GLint mvpMatrix;
        /// \desc model matrix location
        GLint modelMatrix;
        /// \desc normal matrix location
        GLint normalMatrix;
        /// \desc camera position location
        GLint eyePos;
        /// \desc light position location - used for point/spot
        GLint spotLightPos;
        /// \desc light position location - used for point/spot
        GLint spotLightDir;
        /// \desc light direction location - used for directional/spot
        GLint dirLightDir;
        /// \desc light cone angle location - used for spot
        GLint lightCutoff;
        /// \desc color of the light location
        GLint lightColor;
        /// \desc type of the light location - 0 point 1 directional 2 spot
        GLint lightType;
        /// \desc material diffuse color location
        GLint materialDiffColor;
        /// \desc material specular color location
        GLint materialSpecColor;
        /// \desc material shininess factor location
        GLint materialShininess;
        /// \desc material ambient color location
        GLint materialAmbColor;
    } _goodShaderProgramUniformLocations;
    /// \desc stores the locations of all of our shader attributes
    struct GouraudShaderProgramAttributeLocations {
        /// \desc vertex position location
        GLint vPos;
        /// \desc vertex normal location
        GLint vNormal;
    } _goodShaderProgramAttributeLocations;

    /// \desc shader program that performs flat shading of a textiir
    CSCI441::ShaderProgram* _flatShaderProgram;
    struct FlatShaderProgramUniformLocations {
        /// \desc precomputed MVP matrix location
        GLint mvpMatrix;
        /// \desc the color to apply location
        GLint color;
    } _flatShaderProgramUniformLocations;

    CSCI441::ShaderProgram* _textureShader;
    struct TextureShaderProgramUniformLocations {
        GLint mvpMatrix;
        GLint diffuseMap;
        GLint colorTint;
    } _textureShaderProgramUniform;

    struct TextureShaderProgramAttributeLocations {
        GLint vPos;
        GLint vNormal;
        GLint vTexCoord;
    } _textureShaderProgramAttribute;

    struct VertexTextured {
        GLfloat x, y, z;
        GLfloat s, t;
    };

    GLuint _skyHandle;
    GLuint _minesHandle;
    enum ballStyle { regular, striped, cue, eight };

    class Ball {
    public:
        Ball(float x, float y, float r, int tex, ballStyle s);
        ballStyle s;
        float x;
        float y;
        float vx;
        float vy;
        float r;
        glm::mat4 rot;
        int tex;
        bool moving;
    };
    /// \desc All the balls. Index 0 is the cue ball
    std::vector<Ball*> balls;

    void addBall(float x, float y, ballStyle s);

    void physics(float delta) const;

    void drawBalls(glm::mat4 viewMtx, glm::mat4 projMtx) const;

    void drawStick(glm::mat4 viewMtx, glm::mat4 projMtx) const;

    void hit(int i, int j) const;

    class Hole {
    public:
        Hole(float x, float y, float r);
        float x;
        float y;
        float r;
    };
    /// \desc information list of all the buildings to draw
    std::vector<Hole*> holes;

    void sinkBalls();

    void addHole(float x, float y);

    void setupTable();

    //***************************************************************************
    // Private Helper Functions

    /// \desc sends the material uniforms to the Gouraud Shader Program
    /// \param material material properties to set
    void _setMaterialProperties(CSCI441::Materials::Material material) const;

    /// \desc sends the matrix uniforms to a given shader program
    /// \param shaderProgram shader program to send matrices to
    /// \param modelMatrix model matrix
    /// \param viewMatrix view matrix
    /// \param projectionMatrix projection matrix
    /// \param mvpMtxLocation location of the MVP matrix within the shaderProgram
    /// \param modelMtxLocation location of the Model matrix within the shaderProgram (-1 by default)
    /// \param normalMtxLocation location of the Normal matrix within the shaderProgram (-1 by default)
    static void _computeAndSendTransformationMatrices(CSCI441::ShaderProgram* shaderProgram,
                                                      glm::mat4 modelMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix,
                                                      GLint mvpMtxLocation, GLint modelMtxLocation = -1, GLint normalMtxLocation = -1);
};

void lab08_keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods );
void lab08_cursor_callback(GLFWwindow *window, double x, double y );
void lab08_mouse_button_callback(GLFWwindow *window, int button, int action, int mods );
void lab08_scroll_callback(GLFWwindow *window, double xOffset, double yOffset);

#endif // LAB08_LAB08_ENGINE_H
