#include "FPEngine.h"



#include <cmath>
#include <iostream>
//*************************************************************************************
//
// Helper Functions

#ifndef M_PI
#define M_PI 3.14159265
#endif

//*************************************************************************************
//
// Public Interface

Lab08Engine::Lab08Engine()
         : CSCI441::OpenGLEngine(4, 1,
                                 640, 480,
                                 "FP: POOL HALL") {

    for(auto& _key : _keys) _key = GL_FALSE;

    _mousePosition = glm::vec2(MOUSE_UNINITIALIZED, MOUSE_UNINITIALIZED );
    _leftMouseButtonState = GLFW_RELEASE;
}

void Lab08Engine::handleKeyEvent(GLint key, GLint action) {
    if(key != GLFW_KEY_UNKNOWN)
        _keys[key] = ((action == GLFW_PRESS) || (action == GLFW_REPEAT));

    if(action == GLFW_PRESS) {
        switch( key ) {
            // quit!
            case GLFW_KEY_Q:
            case GLFW_KEY_ESCAPE:
                setWindowShouldClose();
                break;

            // toggle between light types
            case GLFW_KEY_1:    // point light
            case GLFW_KEY_2:    // directional light
            case GLFW_KEY_3:    // spotlight
                _lightType = key - GLFW_KEY_1; // GLFW_KEY_1 is 49.  they go in sequence from there

                // ensure shader program is not null
                if(_gouraudShaderProgram) {
                    _gouraudShaderProgram->setProgramUniform(_gouraudShaderProgramUniformLocations.lightType, _lightType );
                }
                if(_goofyShaderProgram) {
                    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.lightType, _lightType );
                }
                break;
            
            case GLFW_KEY_H:
                if(_goofyShaderProgram){
                    glm::vec3 hitVec = _pArcballCam->getPosition() - _pArcballCam->getLookAtPoint();
                    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.hitVector, glm::normalize(hitVec));
                    glfwSetTime(0);
                }
                break;
            default: break; // suppress CLion warning
        }
    }
}

void Lab08Engine::handleMouseButtonEvent(GLint button, GLint action) {
    // if the event is for the left mouse button
    if( button == GLFW_MOUSE_BUTTON_LEFT ) {
        // update the left mouse button's state
        _leftMouseButtonState = action;
    }
}

void Lab08Engine::handleCursorPositionEvent(glm::vec2 currMousePosition) {
    // if mouse hasn't moved in the window, prevent camera from flipping out
    if(fabs(_mousePosition.x - MOUSE_UNINITIALIZED) <= 0.000001f) {
        _mousePosition = currMousePosition;
    }

    // active motion - if the left mouse button is being held down while the mouse is moving
    if(_leftMouseButtonState == GLFW_PRESS) {
        // if shift is held down, update our camera radius
        if( _keys[GLFW_KEY_LEFT_SHIFT] || _keys[GLFW_KEY_RIGHT_SHIFT] ) {
            GLfloat totChgSq = (currMousePosition.x - _mousePosition.x) + (currMousePosition.y - _mousePosition.y);
            _pArcballCam->moveForward(totChgSq * 0.01f );
        }
        // otherwise, update our camera angles theta & phi
        else {
            // rotate the camera by the distance the mouse moved
            _pArcballCam->rotate((currMousePosition.x - _mousePosition.x) * 0.005f,
                                (_mousePosition.y - currMousePosition.y) * 0.005f);
        }

        // ensure shader program is not null
        if(_gouraudShaderProgram) {
            // set the eye position - needed for specular reflection
            _gouraudShaderProgram->setProgramUniform(_gouraudShaderProgramUniformLocations.eyePos, _pArcballCam->getPosition());
        }
        if(_goofyShaderProgram) {
            // set the eye position - needed for specular reflection
            _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.eyePos, _pArcballCam->getPosition());
        }
    }
    // passive motion
    else {

    }

    // update the mouse position
    _mousePosition = currMousePosition;
}

void Lab08Engine::handleScrollEvent(glm::vec2 offset) {
    // update the camera radius in/out
    GLfloat totChgSq = offset.y;
    _pArcballCam->moveForward(totChgSq * 0.2f );
}

//*************************************************************************************
//
// Engine Setup

void Lab08Engine::mSetupGLFW() {
    CSCI441::OpenGLEngine::mSetupGLFW();

    // set our callbacks
    glfwSetKeyCallback(mpWindow, lab08_keyboard_callback);
    glfwSetMouseButtonCallback(mpWindow, lab08_mouse_button_callback);
    glfwSetCursorPosCallback(mpWindow, lab08_cursor_callback);
    glfwSetScrollCallback(mpWindow, lab08_scroll_callback);
}

void Lab08Engine::mSetupOpenGL() {
    glEnable( GL_DEPTH_TEST );					                    // enable depth testing
    glDepthFunc( GL_LESS );							                // use less than depth test

    glEnable(GL_BLEND);									            // enable blending
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);	            // use one minus blending equation

    glClearColor( 0.0f, 0.0f, 0.0f, 1.0f );	// clear the frame buffer to black

    glFrontFace(GL_CCW);                                            // the front faces are CCW
    glCullFace(GL_BACK);                                            // cull the back faces
}

void Lab08Engine::mSetupShaders() {
    //Setup goofy ball shader
    _goofyShaderProgram = new CSCI441::ShaderProgram( "shaders/goofyShader.v.glsl", "shaders/goofyShader.f.glsl" );
    // get uniform locations
    _goofyShaderProgramUniformLocations.hitVector           = _goofyShaderProgram->getUniformLocation("hitVec");
    _goofyShaderProgramUniformLocations.timeSince           = _goofyShaderProgram->getUniformLocation("timeSince");
    _goofyShaderProgramUniformLocations.mvpMatrix           = _goofyShaderProgram->getUniformLocation("mvpMatrix");
    _goofyShaderProgramUniformLocations.modelMatrix         = _goofyShaderProgram->getUniformLocation("modelMatrix");
    _goofyShaderProgramUniformLocations.normalMatrix        = _goofyShaderProgram->getUniformLocation("normalMtx");
    _goofyShaderProgramUniformLocations.eyePos              = _goofyShaderProgram->getUniformLocation("eyePos");
    _goofyShaderProgramUniformLocations.spotLightPos            = _goofyShaderProgram->getUniformLocation("spotLightPos");
    _goofyShaderProgramUniformLocations.spotLightDir            = _goofyShaderProgram->getUniformLocation("spotLightDir");
    _goofyShaderProgramUniformLocations.dirLightDir            = _goofyShaderProgram->getUniformLocation("dirLightDir");
    _goofyShaderProgramUniformLocations.lightCutoff         = _goofyShaderProgram->getUniformLocation("lightCutoff");
    _goofyShaderProgramUniformLocations.lightColor          = _goofyShaderProgram->getUniformLocation("lightColor");
    _goofyShaderProgramUniformLocations.materialDiffColor   = _goofyShaderProgram->getUniformLocation("materialDiffColor");
    _goofyShaderProgramUniformLocations.materialSpecColor   = _goofyShaderProgram->getUniformLocation("materialSpecColor");
    _goofyShaderProgramUniformLocations.materialShininess   = _goofyShaderProgram->getUniformLocation("materialShininess");
    _goofyShaderProgramUniformLocations.materialAmbColor    = _goofyShaderProgram->getUniformLocation("materialAmbColor");
    // get attribute locations
    _goofyShaderProgramAttributeLocations.vPos              = _goofyShaderProgram->getAttributeLocation("vPos");
    _goofyShaderProgramAttributeLocations.vNormal           = _goofyShaderProgram->getAttributeLocation("vNormal");

    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.hitVector, glm::vec3(0));
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.timeSince, 1);

    //***************************************************************************
    // Setup Gouraud Shader Program

    _gouraudShaderProgram = new CSCI441::ShaderProgram( "shaders/gouraudShader.v.glsl", "shaders/gouraudShader.f.glsl" );
    // get uniform locations
    _gouraudShaderProgramUniformLocations.mvpMatrix           = _gouraudShaderProgram->getUniformLocation("mvpMatrix");
    _gouraudShaderProgramUniformLocations.modelMatrix         = _gouraudShaderProgram->getUniformLocation("modelMatrix");
    _gouraudShaderProgramUniformLocations.normalMatrix        = _gouraudShaderProgram->getUniformLocation("normalMtx");
    _gouraudShaderProgramUniformLocations.eyePos              = _gouraudShaderProgram->getUniformLocation("eyePos");
    _gouraudShaderProgramUniformLocations.lightPos            = _gouraudShaderProgram->getUniformLocation("lightPos");
    _gouraudShaderProgramUniformLocations.lightDir            = _gouraudShaderProgram->getUniformLocation("lightDir");
    _gouraudShaderProgramUniformLocations.lightCutoff         = _gouraudShaderProgram->getUniformLocation("lightCutoff");
    _gouraudShaderProgramUniformLocations.lightColor          = _gouraudShaderProgram->getUniformLocation("lightColor");
    _gouraudShaderProgramUniformLocations.lightType           = _gouraudShaderProgram->getUniformLocation("lightType");
    _gouraudShaderProgramUniformLocations.materialDiffColor   = _gouraudShaderProgram->getUniformLocation("materialDiffColor");
    _gouraudShaderProgramUniformLocations.materialSpecColor   = _gouraudShaderProgram->getUniformLocation("materialSpecColor");
    _gouraudShaderProgramUniformLocations.materialShininess   = _gouraudShaderProgram->getUniformLocation("materialShininess");
    _gouraudShaderProgramUniformLocations.materialAmbColor    = _gouraudShaderProgram->getUniformLocation("materialAmbColor");
    // get attribute locations
    _gouraudShaderProgramAttributeLocations.vPos              = _gouraudShaderProgram->getAttributeLocation("vPos");
    _gouraudShaderProgramAttributeLocations.vNormal           = _gouraudShaderProgram->getAttributeLocation("vNormal");

    //***************************************************************************
    // Setup Flat Shader Program

    _flatShaderProgram = new CSCI441::ShaderProgram( "shaders/flatShader.v.glsl", "shaders/flatShader.f.glsl" );
    // get uniform locations
    _flatShaderProgramUniformLocations.mvpMatrix             = _flatShaderProgram->getUniformLocation("mvpMatrix");
    _flatShaderProgramUniformLocations.color                 = _flatShaderProgram->getUniformLocation("color");
    // NOTE: we do not query an attribute locations because in our shader we have set the locations to be the same as
    // the Gouraud Shader attribute locations

    // hook up the CSCI441 object library to our shader program - MUST be done after the shader is used and before the objects are drawn
    // if we have multiple shaders the flow would be:
    //      1) shader->useProgram()
    //      2) CSCI441::setVertexAttributeLocations()
    //      3) CSCI441::draw*()
    // OR the alternative is to ensure that all of your shader programs use the
    // same attribute locations for the vertex position, normal, and texture coordinate
    // as this lab is doing
    CSCI441::setVertexAttributeLocations(_gouraudShaderProgramAttributeLocations.vPos,
                                         _gouraudShaderProgramAttributeLocations.vNormal,
                                         -1 );
}

void Lab08Engine::mSetupBuffers() {
    testModel = new CSCI441::ModelLoader("models/shoe.obj");
    testModel->setAttributeLocations(_gouraudShaderProgramAttributeLocations.vPos, _gouraudShaderProgramAttributeLocations.vNormal);


    // ------------------------------------------------------------------------------------------------------
    // generate all of our VAO/VBO/IBO descriptors
    glGenVertexArrays( NUM_VAOS, _vaos );
    glGenBuffers( NUM_VAOS, _vbos );
    glGenBuffers( NUM_VAOS, _ibos );

    // ------------------------------------------------------------------------------------------------------
    // create the platform
    _createPlatform(_vaos[VAO_ID::PLATFORM], _vbos[VAO_ID::PLATFORM], _ibos[VAO_ID::PLATFORM], _numVAOPoints[VAO_ID::PLATFORM]);

}

void Lab08Engine::_createPlatform(GLuint vao, GLuint vbo, GLuint ibo, GLsizei &numVAOPoints) const {
    // container for our vertex data
    struct VertexNormalCoordinate {
        GLfloat x, y, z;        // x. y, z position
        GLfloat nx, ny, nz;     // x, y, z normal vector
    };

    // create our platform
    VertexNormalCoordinate platformVertices[4] = {
            { -0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f }, // 0 - BL
            {  0.5f, 0.0f, -0.5f,  0.0f, 1.0f, 0.0f }, // 1 - BR
            { -0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f }, // 2 - TL
            {  0.5f, 0.0f,  0.5f,  0.0f, 1.0f, 0.0f }  // 3 - TR
    };

    // order to connect the vertices in
    GLushort platformIndices[4] = { 0, 2, 1, 3 };
    numVAOPoints = 4;

    glBindVertexArray(vao);

    glBindBuffer( GL_ARRAY_BUFFER, vbo );
    glBufferData( GL_ARRAY_BUFFER, sizeof( platformVertices ), platformVertices, GL_STATIC_DRAW );

    glEnableVertexAttribArray( _gouraudShaderProgramAttributeLocations.vPos );
    glVertexAttribPointer(_gouraudShaderProgramAttributeLocations.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalCoordinate), nullptr );

    glEnableVertexAttribArray( _gouraudShaderProgramAttributeLocations.vNormal );
    glVertexAttribPointer(_gouraudShaderProgramAttributeLocations.vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalCoordinate), (void*) (sizeof(GLfloat) * 3) );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

    fprintf( stdout, "[INFO]: platform read in with VAO/VBO/IBO %d/%d/%d & %d points\n", vao, vbo, ibo, numVAOPoints );
}


void Lab08Engine::mSetupTextures() {
    // unused in this lab
}

void Lab08Engine::mSetupScene() {
    // if either shader program is null, do not continue any further to prevent run time errors
    if(!_gouraudShaderProgram || !_flatShaderProgram) {
        return;
    }

    // set up camera
    _pArcballCam = new CSCI441::ArcballCam();
    _pArcballCam->setLookAtPoint(glm::vec3(0.0f, 0.0f, 0.0f));
    _pArcballCam->setTheta(3.52f);
    _pArcballCam->setPhi(1.9f);
    _pArcballCam->setRadius(15.0f);
    _pArcballCam->recomputeOrientation();
    _gouraudShaderProgram->setProgramUniform(_gouraudShaderProgramUniformLocations.eyePos, _pArcballCam->getPosition());

    // set up light info
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    _spotLightPos = glm::vec3(0.0f, 10.0f, 0.0f);
    _spotLightDir = glm::vec3(0,-1,0);
    _lightDir = glm::vec3(-1.0f, 0.0f, -1.0f);
    _lightAngle = glm::radians(27.5f);
    float lightCutoff = glm::cos(_lightAngle);
    _lightType = 0;

    _gouraudShaderProgram->setProgramUniform(_gouraudShaderProgramUniformLocations.lightColor, lightColor);
    _gouraudShaderProgram->setProgramUniform(_gouraudShaderProgramUniformLocations.lightPos, _spotLightPos);
    _gouraudShaderProgram->setProgramUniform(_gouraudShaderProgramUniformLocations.lightDir, _lightDir);
    _gouraudShaderProgram->setProgramUniform(_gouraudShaderProgramUniformLocations.lightCutoff, lightCutoff);
    _gouraudShaderProgram->setProgramUniform(_gouraudShaderProgramUniformLocations.lightType, _lightType);

    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.lightColor, lightColor);
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.spotLightPos, _spotLightPos);
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.spotLightPos, _spotLightDir);
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.dirLightDir, _lightDir);
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.lightCutoff, lightCutoff);
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.lightType, _lightType);

    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.eyePos, _pArcballCam->getPosition());


    // set flat shading color
    glm::vec3 flatColor(1.0f, 1.0f, 1.0f);
    _flatShaderProgram->setProgramUniform(_flatShaderProgramUniformLocations.color, flatColor);
}

//*************************************************************************************
//
// Engine Cleanup

void Lab08Engine::mCleanupShaders() {
    fprintf( stdout, "[INFO]: ...deleting Shaders.\n" );
    delete _gouraudShaderProgram;
    delete _flatShaderProgram;
}

void Lab08Engine::mCleanupBuffers() {
    fprintf( stdout, "[INFO]: ...deleting VAOs....\n" );
    CSCI441::deleteObjectVAOs();
    glDeleteVertexArrays( NUM_VAOS, _vaos );

    fprintf( stdout, "[INFO]: ...deleting VBOs....\n" );
    CSCI441::deleteObjectVBOs();
    glDeleteBuffers( NUM_VAOS, _vbos );

    fprintf( stdout, "[INFO]: ...deleting IBOs....\n" );
    glDeleteBuffers( NUM_VAOS, _ibos );

    fprintf( stdout, "[INFO]: ...deleting models..\n" );
}

void Lab08Engine::mCleanupTextures() {
    fprintf( stdout, "[INFO]: ...deleting Textures\n" );
}

void Lab08Engine::mCleanupScene() {
    fprintf( stdout, "[INFO]: ...deleting scene...\n" );
    delete _pArcballCam;
}

//*************************************************************************************
//
// Rendering / Drawing Functions - this is where the magic happens!

void Lab08Engine::_renderScene(glm::mat4 viewMtx, glm::mat4 projMtx) const {
    // if either shader program is null, do not continue any further to prevent run time errors
    if(!_gouraudShaderProgram || !_flatShaderProgram) {
        return;
    }

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // use the gouraud shader
    _gouraudShaderProgram->useProgram();

    // _setMaterialProperties(CSCI441::Materials::RUBY);
    // modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 10, -4));
    // _computeAndSendTransformationMatrices( _gouraudShaderProgram,
    //                                         modelMatrix, viewMtx, projMtx,
    //                                         _gouraudShaderProgramUniformLocations.mvpMatrix,
    //                                         _gouraudShaderProgramUniformLocations.modelMatrix,
    //                                         _gouraudShaderProgramUniformLocations.normalMatrix);

    // testModel->draw(_gouraudShaderProgram->getShaderProgramHandle());
    //***************************************************************************
    // draw the ground

    // use the emerald material
    // _setMaterialProperties(CSCI441::Materials::EMERALD);

    // // draw a larger ground plane by translating a single quad across a grid
    // const int GROUND_SIZE = 6;
    // for(int i = -GROUND_SIZE; i <= GROUND_SIZE; i++) {
    //     for(int j = -GROUND_SIZE; j <= GROUND_SIZE; j++) {
    //         modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(i, 0, j));
    //         _computeAndSendTransformationMatrices( _gouraudShaderProgram,
    //                                                modelMatrix, viewMtx, projMtx,
    //                                                _gouraudShaderProgramUniformLocations.mvpMatrix,
    //                                                _gouraudShaderProgramUniformLocations.modelMatrix,
    //                                                _gouraudShaderProgramUniformLocations.normalMatrix);
    //         glBindVertexArray( _vaos[VAO_ID::PLATFORM] );
    //         glDrawElements( GL_TRIANGLE_STRIP, _numVAOPoints[VAO_ID::PLATFORM], GL_UNSIGNED_SHORT, nullptr );
    //     }
    // }
    //DRAW THE CUE BALL
    _goofyShaderProgram->useProgram();

    _setMaterialProperties(CSCI441::Materials::WHITE_PLASTIC);
    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 2, 0));
    _computeAndSendTransformationMatrices( _goofyShaderProgram,
                                            modelMatrix, viewMtx, projMtx,
                                            _goofyShaderProgramUniformLocations.mvpMatrix,
                                            _goofyShaderProgramUniformLocations.modelMatrix,
                                            _goofyShaderProgramUniformLocations.normalMatrix);

    CSCI441::drawSolidSphere(2,20,20);

    _flatShaderProgram->useProgram();
    modelMatrix = glm::mat4(1.0f);
    _computeAndSendTransformationMatrices(_flatShaderProgram,
                                          modelMatrix, viewMtx, projMtx,
                                         _flatShaderProgramUniformLocations.mvpMatrix); 
    // draw a visual of where our point or spotlight is located

    // if using a point light
    if( _lightType == 0 ) {
        // move to the light location
        modelMatrix = glm::translate( glm::mat4(1.0f), _spotLightPos );
        _computeAndSendTransformationMatrices(_flatShaderProgram,
                                              modelMatrix, viewMtx, projMtx,
                                              _flatShaderProgramUniformLocations.mvpMatrix);
        // draw a sphere there
        CSCI441::drawSolidSphere(0.25f, 16, 16);
    }
    // if using a spotlight
    else if( _lightType == 2 ) {
        // move to the light location
        modelMatrix = glm::translate( glm::mat4(1.0f), _spotLightPos );

        // orient with the light direction
        glm::vec3 rotAxis = glm::normalize( glm::cross( CSCI441::Y_AXIS_NEG, glm::normalize(_lightDir) ) );
        GLfloat rotAngle = acosf( glm::dot( CSCI441::Y_AXIS_NEG, glm::normalize(_lightDir) ) );
        glm::mat4 rotMtx = glm::rotate( glm::mat4(1.0f), rotAngle, rotAxis );
        modelMatrix = modelMatrix * rotMtx;

        _computeAndSendTransformationMatrices(_flatShaderProgram,
                                              modelMatrix, viewMtx, projMtx,
                                              _flatShaderProgramUniformLocations.mvpMatrix);

        // draw a cone there
        CSCI441::drawSolidCone(_lightAngle, 1.0f, 16, 16);
    }
}

void Lab08Engine::_updateScene() {
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.timeSince, (float)glfwGetTime());

    // spotLightSwingAngle += spotLightDTheta;
    if(fabs(spotLightSwingAngle) > M_PI_4 ) spotLightDTheta = -spotLightDTheta;
    // _spotLightPos.z = glm::sin(spotLightSwingAngle);
    _spotLightDir = glm::vec3(0,-glm::cos(spotLightSwingAngle),glm::sin(spotLightSwingAngle));
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.spotLightPos, _spotLightPos);
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.spotLightPos, _spotLightDir);

}

void Lab08Engine::run() {
    //  This is our draw loop - all rendering is done here.  We use a loop to keep the window open
    //	until the user decides to close the window and quit the program.  Without a loop, the
    //	window will display once and then the program exits.
    while( !glfwWindowShouldClose(mpWindow) ) {	        // check if the window was instructed to be closed
        glDrawBuffer( GL_BACK );				        // work with our back frame buffer
        glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );	// clear the current color contents and depth buffer in the window

        // Get the size of our framebuffer.  Ideally this should be the same dimensions as our window, but
        // when using a Retina display the actual window can be larger than the requested window.  Therefore,
        // query what the actual size of the window we are rendering to is.
        GLint framebufferWidth, framebufferHeight;
        glfwGetFramebufferSize( mpWindow, &framebufferWidth, &framebufferHeight );

        // update the viewport - tell OpenGL we want to render to the whole window
        glViewport( 0, 0, framebufferWidth, framebufferHeight );

        // draw everything to the window
        _renderScene(_pArcballCam->getViewMatrix(), _pArcballCam->getProjectionMatrix());

        // animate the scene
        _updateScene();

        glfwSwapBuffers(mpWindow);                       // flush the OpenGL commands and make sure they get rendered!
        glfwPollEvents();				                // check for any events and signal to redraw screen
    }
}

//*************************************************************************************
//
// Private Helper FUnctions


void Lab08Engine::_setMaterialProperties(CSCI441::Materials::Material material) const {
    // ensure our shader program is not null
    if(_gouraudShaderProgram) {
        // set the D, S, A, & shininess components of the material to our Gouraud Shader
        glProgramUniform3fv( _gouraudShaderProgram->getShaderProgramHandle(), _gouraudShaderProgramUniformLocations.materialDiffColor, 1, material.diffuse   );
        glProgramUniform3fv( _gouraudShaderProgram->getShaderProgramHandle(), _gouraudShaderProgramUniformLocations.materialSpecColor, 1, material.specular  );
        glProgramUniform1f(  _gouraudShaderProgram->getShaderProgramHandle(), _gouraudShaderProgramUniformLocations.materialShininess,    material.shininess );
        glProgramUniform3fv( _gouraudShaderProgram->getShaderProgramHandle(), _gouraudShaderProgramUniformLocations.materialAmbColor,  1, material.ambient   );
    }
    if(_goofyShaderProgram) {
        // set the D, S, A, & shininess components of the material to our goofy Shader
        glProgramUniform3fv( _goofyShaderProgram->getShaderProgramHandle(), _goofyShaderProgramUniformLocations.materialDiffColor, 1, material.diffuse   );
        glProgramUniform3fv( _goofyShaderProgram->getShaderProgramHandle(), _goofyShaderProgramUniformLocations.materialSpecColor, 1, material.specular  );
        glProgramUniform1f(  _goofyShaderProgram->getShaderProgramHandle(), _goofyShaderProgramUniformLocations.materialShininess,    material.shininess );
        glProgramUniform3fv( _goofyShaderProgram->getShaderProgramHandle(), _goofyShaderProgramUniformLocations.materialAmbColor,  1, material.ambient   );
    }
}

void Lab08Engine::_computeAndSendTransformationMatrices(CSCI441::ShaderProgram* shaderProgram,
                                                        glm::mat4 modelMatrix, glm::mat4 viewMatrix, glm::mat4 projectionMatrix,
                                                        GLint mvpMtxLocation, GLint modelMtxLocation, GLint normalMtxLocation) {
    // ensure our shader program is not null
    if( shaderProgram ) {
        // precompute the MVP matrix CPU side
        glm::mat4 mvpMatrix = projectionMatrix * viewMatrix * modelMatrix;
        // precompute the Normal matrix CPU side
        glm::mat3 normalMatrix = glm::mat3(glm::transpose(glm::inverse(modelMatrix)));

        // send the matrices to the shader
        shaderProgram->setProgramUniform(mvpMtxLocation, mvpMatrix);
        shaderProgram->setProgramUniform(modelMtxLocation, modelMatrix);
        shaderProgram->setProgramUniform(normalMtxLocation, normalMatrix);
    }
}

//*************************************************************************************
//
// Callbacks

void lab08_keyboard_callback(GLFWwindow *window, int key, int scancode, int action, int mods ) {
    auto engine = (Lab08Engine*) glfwGetWindowUserPointer(window);

    // ensure our engine is not null
    if(engine) {
        // pass the key and action through to the engine
        engine->handleKeyEvent(key, action);
    }
}

void lab08_cursor_callback(GLFWwindow *window, double x, double y ) {
    auto engine = (Lab08Engine*) glfwGetWindowUserPointer(window);

    // ensure our engine is not null
    if(engine) {
        // pass the cursor position through to the engine
        engine->handleCursorPositionEvent(glm::vec2(x, y));
    }
}

void lab08_mouse_button_callback(GLFWwindow *window, int button, int action, int mods ) {
    auto engine = (Lab08Engine*) glfwGetWindowUserPointer(window);

    // ensure our engine is not null
    if(engine) {
        // pass the mouse button and action through to the engine
        engine->handleMouseButtonEvent(button, action);
    }
}

void lab08_scroll_callback(GLFWwindow *window, double xOffset, double yOffset) {
    auto engine = (Lab08Engine*) glfwGetWindowUserPointer(window);

    // ensure our engine is not null
    if(engine) {
        // pass the scroll offset through to the engine
        engine->handleScrollEvent(glm::vec2(xOffset, yOffset));
    }
}
