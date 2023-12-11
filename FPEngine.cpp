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
                                960, 960,
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
                if(_goodShaderProgram) {
                    _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.lightType, _lightType );
                }
                if(_goofyShaderProgram) {
                    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.lightType, _lightType );
                }
                break;

            case GLFW_KEY_H:
                if(_goofyShaderProgram && canShoot){
                    cueState++;

                    if(cueState == 2){
                        glm::vec3 camPos = _pArcballCam->getPosition();
                        camPos.y = 0;
                        glm::vec3 lookPos = _pArcballCam->getLookAtPoint();
                        lookPos.y = 0;
                        glm::vec3 hitVec = camPos - lookPos;
                        _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.hitVector, glm::normalize(hitVec));
                        glfwSetTime(0);

                        hitVec = glm::normalize(hitVec) * (float(meterHeight*2.5));
                        balls[0]->vx = -hitVec.x;
                        balls[0]->vy = -hitVec.z;
                        // make it so you can't shoot until balls have stopped moving
                        canShoot = false;
                        currentTurn++;
                    }


                    if(cueState >1) cueState = 0;
                }
                break;
            case GLFW_KEY_R:
                // reset the game
                resetGame();

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
            if(cueState != 1) {
                // rotate the camera by the distance the mouse moved
                _pArcballCam->rotate((currMousePosition.x - _mousePosition.x) * 0.005f,
                                     (_mousePosition.y - currMousePosition.y) * -0.005f);
            }
        }

        // ensure shader program is not null
        if(_goodShaderProgram) {
            // set the eye position - needed for specular reflection
            _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.eyePos, _pArcballCam->getPosition());
        }
        if(_goofyShaderProgram) {
            // set the eye position - needed for specular reflection
            _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.eyePos, _pArcballCam->getPosition());
        }
    }

    // update the mouse position
    _mousePosition = currMousePosition;
}

void Lab08Engine::handleScrollEvent(glm::vec2 offset) {
    // update the camera radius in/out
    GLfloat totChgSq = offset.y;
    _pArcballCam->moveForward(totChgSq * 1.0f );
}

//*************************************************************************************
//
// Engine Setup

void Lab08Engine::mSetupGLFW(){
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
    _goofyShaderProgramUniformLocations.spotLightPos        = _goofyShaderProgram->getUniformLocation("spotLightPos");
    _goofyShaderProgramUniformLocations.spotLightDir        = _goofyShaderProgram->getUniformLocation("spotLightDir");
    _goofyShaderProgramUniformLocations.dirLightDir         = _goofyShaderProgram->getUniformLocation("dirLightDir");
    _goofyShaderProgramUniformLocations.lightCutoff         = _goofyShaderProgram->getUniformLocation("lightCutoff");
    _goofyShaderProgramUniformLocations.lightColor          = _goofyShaderProgram->getUniformLocation("lightColor");
    _goofyShaderProgramUniformLocations.materialDiffColor   = _goofyShaderProgram->getUniformLocation("materialDiffColor");
    _goofyShaderProgramUniformLocations.materialSpecColor   = _goofyShaderProgram->getUniformLocation("materialSpecColor");
    _goofyShaderProgramUniformLocations.materialShininess   = _goofyShaderProgram->getUniformLocation("materialShininess");
    _goofyShaderProgramUniformLocations.materialAmbColor    = _goofyShaderProgram->getUniformLocation("materialAmbColor");
    _goofyShaderProgramUniformLocations.diffuseMap          = _goofyShaderProgram->getUniformLocation("diffuseMap");
    // get attribute locations
    _goofyShaderProgramAttributeLocations.vPos              = _goofyShaderProgram->getAttributeLocation("vPos");
    _goofyShaderProgramAttributeLocations.vNormal           = _goofyShaderProgram->getAttributeLocation("vNormal");
    _goofyShaderProgramAttributeLocations.vTexCoord         = _goofyShaderProgram->getAttributeLocation("inTexCoord");

    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.hitVector, glm::vec3(0));
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.timeSince, 1);

    //***************************************************************************
    // Setup Gouraud Shader Program

    _goodShaderProgram = new CSCI441::ShaderProgram( "shaders/goodShader.v.glsl", "shaders/goodShader.f.glsl" );
    // get uniform locations
    _goodShaderProgramUniformLocations.mvpMatrix           = _goodShaderProgram->getUniformLocation("mvpMatrix");
    _goodShaderProgramUniformLocations.modelMatrix         = _goodShaderProgram->getUniformLocation("modelMatrix");
    _goodShaderProgramUniformLocations.normalMatrix        = _goodShaderProgram->getUniformLocation("normalMtx");
    _goodShaderProgramUniformLocations.eyePos              = _goodShaderProgram->getUniformLocation("eyePos");
    _goodShaderProgramUniformLocations.spotLightPos        = _goodShaderProgram->getUniformLocation("spotLightPos");
    _goodShaderProgramUniformLocations.spotLightDir        = _goodShaderProgram->getUniformLocation("spotLightDir");
    _goodShaderProgramUniformLocations.dirLightDir         = _goodShaderProgram->getUniformLocation("dirLightDir");
    _goodShaderProgramUniformLocations.lightCutoff         = _goodShaderProgram->getUniformLocation("lightCutoff");
    _goodShaderProgramUniformLocations.lightColor          = _goodShaderProgram->getUniformLocation("lightColor");
    _goodShaderProgramUniformLocations.materialDiffColor   = _goodShaderProgram->getUniformLocation("materialDiffColor");
    _goodShaderProgramUniformLocations.materialSpecColor   = _goodShaderProgram->getUniformLocation("materialSpecColor");
    _goodShaderProgramUniformLocations.materialShininess   = _goodShaderProgram->getUniformLocation("materialShininess");
    _goodShaderProgramUniformLocations.materialAmbColor    = _goodShaderProgram->getUniformLocation("materialAmbColor");
    // get attribute locations
    _goodShaderProgramAttributeLocations.vPos              = _goodShaderProgram->getAttributeLocation("vPos");
    _goodShaderProgramAttributeLocations.vNormal           = _goodShaderProgram->getAttributeLocation("vNormal");


    //***************************************************************************
    // Setup Flat Shader Program

    _flatShaderProgram = new CSCI441::ShaderProgram( "shaders/flatShader.v.glsl", "shaders/flatShader.f.glsl" );
    // get uniform locations
    _flatShaderProgramUniformLocations.mvpMatrix             = _flatShaderProgram->getUniformLocation("mvpMatrix");
    _flatShaderProgramUniformLocations.color                 = _flatShaderProgram->getUniformLocation("color");
    // NOTE: we do not query an attribute locations because in our shader we have set the locations to be the same as

}

void Lab08Engine::mSetupBuffers() {

    //flat with texture for skybox
    _textureShader = new CSCI441::ShaderProgram("shaders/textureShader.v.glsl", "shaders/textureShader.f.glsl" );

    _textureShaderProgramUniform.mvpMatrix    = _textureShader->getUniformLocation("mvpMatrix");
    _textureShaderProgramUniform.diffuseMap   = _textureShader->getUniformLocation("diffuseMap");
    _textureShaderProgramUniform.colorTint    = _textureShader->getUniformLocation("colorTint");

    _textureShaderProgramAttribute.vPos       = _textureShader->getAttributeLocation("vPos");
    _textureShaderProgramAttribute.vNormal       = _textureShader->getAttributeLocation("vNormal");
    _textureShaderProgramAttribute.vTexCoord  = _textureShader->getAttributeLocation("inTexCoord");

    _textureShader->setProgramUniform( _textureShaderProgramUniform.diffuseMap, 0 );

    glEnableVertexAttribArray( _textureShaderProgramAttribute.vPos );
    glVertexAttribPointer( _textureShaderProgramAttribute.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) 0 );

    glEnableVertexAttribArray( _textureShaderProgramAttribute.vTexCoord );
    glVertexAttribPointer( _textureShaderProgramAttribute.vTexCoord, 2, GL_FLOAT, GL_FALSE, sizeof(VertexTextured), (void*) (sizeof(GLfloat) * 3) );

    //models
    testModel = new CSCI441::ModelLoader("models/shoe.obj");
    testModel->setAttributeLocations(_goodShaderProgramAttributeLocations.vPos, _goodShaderProgramAttributeLocations.vNormal);

    poolGrass = new CSCI441::ModelLoader("models/poolgrass.obj");
    poolGrass->setAttributeLocations(_goodShaderProgramAttributeLocations.vPos, _goodShaderProgramAttributeLocations.vNormal);

    poolWood = new CSCI441::ModelLoader("models/poolwood.obj");
    poolWood->setAttributeLocations(_goodShaderProgramAttributeLocations.vPos, _goodShaderProgramAttributeLocations.vNormal);

    poolHoles = new CSCI441::ModelLoader("models/poolholes.obj");
    poolHoles->setAttributeLocations(_goodShaderProgramAttributeLocations.vPos, _goodShaderProgramAttributeLocations.vNormal);

    mines = new CSCI441::ModelLoader();
    mines->loadModelFile("models/maps2.obj");
    mines->setAttributeLocations(_textureShaderProgramAttribute.vPos, _textureShaderProgramAttribute.vNormal, _textureShaderProgramAttribute.vTexCoord);

    hand = new CSCI441::ModelLoader("models/hand.obj");
    hand->setAttributeLocations(_goodShaderProgramAttributeLocations.vPos, _goodShaderProgramAttributeLocations.vNormal);

    stick = new CSCI441::ModelLoader("models/stick.obj");
    stick->setAttributeLocations(_goodShaderProgramAttributeLocations.vPos, _goodShaderProgramAttributeLocations.vNormal);

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

    glEnableVertexAttribArray( _goodShaderProgramAttributeLocations.vPos );
    glVertexAttribPointer(_goodShaderProgramAttributeLocations.vPos, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalCoordinate), nullptr );

    glEnableVertexAttribArray( _goodShaderProgramAttributeLocations.vNormal );
    glVertexAttribPointer(_goodShaderProgramAttributeLocations.vNormal, 3, GL_FLOAT, GL_FALSE, sizeof(VertexNormalCoordinate), (void*) (sizeof(GLfloat) * 3) );

    glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, ibo );
    glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( platformIndices ), platformIndices, GL_STATIC_DRAW );

    fprintf( stdout, "[INFO]: platform read in with VAO/VBO/IBO %d/%d/%d & %d points\n", vao, vbo, ibo, numVAOPoints );
}

void Lab08Engine::addBall(float x, float y, ballStyle s){
    Ball* newBall = new Ball(x, y, 0.25, 0, s);
    newBall->rot = randomRotationMatrix();
    balls.emplace_back(newBall);
}

void Lab08Engine::addHole(float x, float y){
    Hole* newHole = new Hole(x, y, 0.5);
    holes.emplace_back(newHole);
}

void Lab08Engine::sinkBalls(){
    for(int i = balls.size()-1; i >= 0; i--){
        Ball* ball = balls[i];

        for (int j=0; j < holes.size(); j++){
            Hole* hole = holes[j];
            float dist = glm::distance(glm::vec2(ball->x,ball->y),glm::vec2(hole->x,hole->y));
            if(dist < hole->r){
                // store values of sunk balls before deletion
                switch(ball->s) {
                    case striped:
                        sunkStriped++;
                        sunkThisTurnStriped++;
                        break;
                    case regular:
                        sunkRegular++;
                        sunkThisTurnRegular++;
                        break;
                    case eight:
                        sunkEight++;
                        break;
                    case cue:
                        break;
                    default:
                        break;
                }
                delete ball;
                balls.erase(balls.begin() + i);
            }
        }
    }
}

bool Lab08Engine::ballNearHole(int i) const{
    Ball* ball = balls[i];

    for (int j=0; j < holes.size(); j++){
        Hole* hole = holes[j];
        float dist = glm::distance(glm::vec2(ball->x,ball->y),glm::vec2(hole->x,hole->y));
        if(dist < (hole->r + ball->r)){
            glm::vec2 vel = glm::vec2(ball->vx,ball->vy);
            float speed = glm::length(vel);

            glm::vec2 newVel = glm::normalize(glm::vec2(hole->x,hole->y) - glm::vec2(ball->x,ball->y));

            if(glm::acos(glm::dot(newVel,vel)) < glm::radians(65.0)) {
                newVel *= speed;
                ball->vx = newVel.x;
                ball->vy = newVel.y;
                return true;
            }else{
                return false;
            }
        }
    }
    return false;
}

void Lab08Engine::physics(float delta) {

    delta = 0.1;
    float w = 8.8;
    float h = 4.4;
    float left = -w;
    float right = w;
    float down = -h;
    float up = h;

    for(int i = 0; i<balls.size(); i++){
        Ball* ball = balls[i];

        ball->x += ball->vx * delta;
        ball->y += ball->vy * delta;

        float fric = 1 - (0.5 * delta);
        fric = 0.99;
        ball->vx *= fric;
        ball->vy *= fric;

        float speed = glm::length(glm::vec2(ball->vx,ball->vy));
        if(speed > 0.001){
            ball->moving = true;

            //rotate ball matrix
            glm::vec3 dir = glm::normalize(glm::vec3(ball->vx,0,ball->vy));
            glm::vec3 rotAxis = glm::cross(dir,glm::vec3(0,1,0));
            ball->rot = glm::rotate(ball->rot, speed*-0.25f, rotAxis);
        }else {
            ball->vx = 0;
            ball->vy = 0;
            ball->moving = false;
        }

        for(int j = i; j<balls.size(); j++){
            Ball* ball2 = balls[j];

            float collideRadius = ball->r + ball2->r;

            if(i<j && (ball->moving || ball2->moving) && glm::distance(glm::vec2(ball->x, ball->y), glm::vec2(ball2->x, ball2->y)) < collideRadius){
                hit(i,j);
                // std::cout << "hit" << std::endl;
                myBallsHaveBeenHit = true;
            }
        }

        if(ball->moving && !ballNearHole(i)){
            float bounce = -0.9;
            if(ball->x < (left + ball->r)){
                ball->vx *= bounce;
                ball->x = (left + ball->r);
            }else if(ball->x > (right - ball->r)){
                ball->vx *= bounce;
                ball->x = (right - ball->r);
            }

            if(ball->y < (down + ball->r)){
                ball->vy *= bounce;
                ball->y = (down + ball->r);
            }else if(ball->y > (up - ball->r)){
                ball->vy *= bounce;
                ball->y = (up - ball->r);
            }
        }

    }
}

void Lab08Engine::hit(int i, int j) const{
    Ball* ball = balls[i];
    Ball* ball2 = balls[j];

    float diffX = ball2->x - ball->x;
    float diffY = ball2->y - ball->y;

    float dist = glm::distance(glm::vec2(ball->x, ball->y), glm::vec2(ball2->x, ball2->y));

    float normalX = diffX / dist;
    float normalY = diffY / dist;

    float velDeltaX = ball->vx-ball2->vx;
    float velDeltaY = ball->vy-ball2->vy;
    float velDelta = sqrt(velDeltaX * velDeltaX + velDeltaY * velDeltaY);

    float dot = velDeltaX * normalX + velDeltaY * normalY;

    if(dot > 0){
        float coefficient = -0.5;
        float impulseStrength = (1 + coefficient) * dot * 2;
        float impulseX = impulseStrength * normalX;
        float impulseY = impulseStrength * normalY;
        ball->vx = ball->vx - impulseX;
        ball->vy = ball->vy - impulseY;
        ball2->vx = ball2->vx + impulseX;
        ball2->vy = ball2->vy + impulseY;
        ball->moving = true;
        ball2->moving = true;
    }
}

void Lab08Engine::drawBalls(glm::mat4 viewMtx, glm::mat4 projMtx) const{

    for( const Ball* ball : balls ){
        //_setMaterialProperties(CSCI441::Materials::WHITE_PLASTIC);
        _goofyShaderProgram->useProgram();
        _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.materialAmbColor, glm::vec3(1,1,1));
        _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.materialDiffColor, glm::vec3(1,1,1));
        _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.materialSpecColor, glm::vec3(1,1,1));
        glm::mat4 modelMatrix;
        modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(ball->x, ball->r, ball->y));
        modelMatrix = modelMatrix * ball->rot;
        _computeAndSendTransformationMatrices( _goofyShaderProgram,
                                               modelMatrix, viewMtx, projMtx,
                                               _goofyShaderProgramUniformLocations.mvpMatrix,
                                               _goofyShaderProgramUniformLocations.modelMatrix,
                                               _goofyShaderProgramUniformLocations.normalMatrix);
        CSCI441::setVertexAttributeLocations(_goofyShaderProgramAttributeLocations.vPos, _goofyShaderProgramAttributeLocations.vNormal, _goofyShaderProgramAttributeLocations.vTexCoord);

        if(ball->s == regular) {
            glBindTexture(GL_TEXTURE_2D, _solidBallHandle);
        }else if(ball->s == striped){
            glBindTexture(GL_TEXTURE_2D, _stripeBallHandle);
        }else if(ball->s == eight){
            glBindTexture(GL_TEXTURE_2D, _8BallHandle);
        }else{
            glBindTexture(GL_TEXTURE_2D, _cueBallHandle);
        }

        CSCI441::drawSolidSphere(ball->r,20,20);

        //test draw rotation axis
//        glm::vec3 dir = glm::normalize(glm::vec3(ball->vx,0,ball->vy));
//        glm::vec3 rotAxis = glm::cross(dir,glm::vec3(0,1,0));
//        modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(ball->x, ball->r*2.0f, ball->y) + (rotAxis));
//        _computeAndSendTransformationMatrices( _goofyShaderProgram,
//                                               modelMatrix, viewMtx, projMtx,
//                                               _goofyShaderProgramUniformLocations.mvpMatrix,
//                                               _goofyShaderProgramUniformLocations.modelMatrix,
//                                               _goofyShaderProgramUniformLocations.normalMatrix);
//        CSCI441::drawSolidSphere(0.1f,4,4);

    }

    //draw holes for testing
//    for( const Hole* hole : holes ){
//        _setMaterialProperties(CSCI441::Materials::BLACK_RUBBER);
//        glm::mat4 modelMatrix;
//        modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(hole->x, 0, hole->y));
//        _computeAndSendTransformationMatrices( _goofyShaderProgram,
//                                               modelMatrix, viewMtx, projMtx,
//                                               _goofyShaderProgramUniformLocations.mvpMatrix,
//                                               _goofyShaderProgramUniformLocations.modelMatrix,
//                                               _goofyShaderProgramUniformLocations.normalMatrix);
//
//        CSCI441::drawSolidSphere(hole->r,20,20);
//    }
}

void Lab08Engine::drawStick(glm::mat4 viewMtx, glm::mat4 projMtx) const {

    if (cueState == 2) {
        return;
    }

    glm::vec3 hitVec = _pArcballCam->getPosition() - _pArcballCam->getLookAtPoint();
    hitVec = glm::normalize(hitVec);
    float cueRot = atan2(hitVec.x, hitVec.z) + (3.1415 * 0.5);
    //std::cout << cueRot << std::endl;
    // don't draw cue and hands while the balls are rolling
    if (canShoot) {
        glm::mat4 modelMatrix;
        modelMatrix = glm::translate(glm::mat4(1.0), glm::vec3(balls[0]->x, 0.25, balls[0]->y));
        modelMatrix = glm::rotate(modelMatrix, cueRot, glm::vec3(0, 1, 0));
        modelMatrix = glm::rotate(modelMatrix, -0.1f, glm::vec3(0, 0, 1));
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-1, 0, 0));
        glm::mat4 cueMtx = modelMatrix;
        if(cueState == 1){
            cueMtx = glm::translate(modelMatrix, glm::vec3(-meterHeight, 0, 0));
        }
        cueMtx = glm::scale(cueMtx, glm::vec3(0.1, 0.1, 0.1));

        _computeAndSendTransformationMatrices(_goodShaderProgram,
                                              cueMtx, viewMtx, projMtx,
                                              _goodShaderProgramUniformLocations.mvpMatrix,
                                              _goodShaderProgramUniformLocations.modelMatrix,
                                              _goodShaderProgramUniformLocations.normalMatrix);
        _setMaterialProperties(CSCI441::Materials::WHITE_PLASTIC);
        stick->draw(_goodShaderProgram->getShaderProgramHandle());

        glm::mat4 leftHandMtx = glm::translate(modelMatrix, glm::vec3(-2, 0, 0));
        leftHandMtx = glm::scale(leftHandMtx, glm::vec3(0.1, 0.1, 0.1));
        leftHandMtx = glm::scale(leftHandMtx, glm::vec3(1, 1, -1));

        if (currentPlayer == 1) {
            _goodShaderProgram->setProgramUniform("materialDiffColor", glm::vec3(0.8, 0.1, 0.1));
        } else {
            _goodShaderProgram->setProgramUniform("materialDiffColor", glm::vec3(0.1, 0.1, 0.8));
        }
        _computeAndSendTransformationMatrices(_goodShaderProgram,
                                              leftHandMtx, viewMtx, projMtx,
                                              _goodShaderProgramUniformLocations.mvpMatrix,
                                              _goodShaderProgramUniformLocations.modelMatrix,
                                              _goodShaderProgramUniformLocations.normalMatrix);
        hand->draw(_goodShaderProgram->getShaderProgramHandle());

        if (currentPlayer == 1) {
            _goodShaderProgram->setProgramUniform("materialDiffColor", glm::vec3(0.8, 0.1, 0.1));
        } else {
            _goodShaderProgram->setProgramUniform("materialDiffColor", glm::vec3(0.1, 0.1, 0.8));
        }
        glm::mat4 rightHandMtx = modelMatrix;
        if(cueState == 1){
            rightHandMtx = glm::translate(modelMatrix, glm::vec3(-5-meterHeight, 0, 0));
        }
        else{
            rightHandMtx = glm::translate(modelMatrix, glm::vec3(-5, 0, 0));
        }

        rightHandMtx = glm::scale(rightHandMtx, glm::vec3(0.1, 0.1, 0.1));
        _computeAndSendTransformationMatrices(_goodShaderProgram,
                                              rightHandMtx, viewMtx, projMtx,
                                              _goodShaderProgramUniformLocations.mvpMatrix,
                                              _goodShaderProgramUniformLocations.modelMatrix,
                                              _goodShaderProgramUniformLocations.normalMatrix);
        hand->draw(_goodShaderProgram->getShaderProgramHandle());
    }
}

void Lab08Engine::setupTable(){

    holes.clear();
    balls.clear();

    float w = 8.8 * 2.0;
    float h = 4.4 * 2.0;
    w+= 0.4;
    h+= 0.4;
    float wh = w/2;
    float hh = h/2;
    addHole(-wh, -hh);
    addHole(0, -hh);
    addHole(wh, -hh);

    addHole(-wh, hh);
    addHole(0, hh);
    addHole(wh, hh);

    //setup balls

    w = 8.8 * 2.0;
    h = 4.4 * 2.0;
    wh = w/2;
    hh = h/2;

    float trix = (hh) + ((static_cast<float>(rand()) / RAND_MAX - 0.5) * 0.1);
    float triy = (0) + ((static_cast<float>(rand()) / RAND_MAX - 0.5) * 0.1);
    int ballcount = 1;
    int balltype = 2;

    addBall(-hh,0, ballStyle::cue);
    // balls[0]->vx = 1.0;

    for(int i = 0; i<5; i++){
        ballcount = ballcount + 1;

        for (int j = 0; j<(i+1); j++) {
            if(i == 3 && j == 2){
                balltype = 4;
            }else if((ballcount) % 2 == 0){
                balltype = 2 ;
                ballcount = ballcount + 1;
            }else{
                balltype = 3;
                ballcount = ballcount + 1;
            }

            if(i == 5 && j == 3){
                ballcount = ballcount - 1;
            }

            float xpos = trix + (0.45*(i-2));
            float ypos = triy + (0.51*(j)) - (0.255*(i));
            ballStyle style;
            if(ballcount == 8){
                style = eight;
            }
            else if(ballcount % 2 == 0){
                style = striped;
            }
            else{
                style = regular;
            }
            addBall(xpos,ypos,style);
        }
    }

}

void Lab08Engine::mSetupTextures() {
    // unused in this lab
}

void Lab08Engine::mSetupScene() {
    // if either shader program is null, do not continue any further to prevent run time errors
    if(!_goodShaderProgram || !_flatShaderProgram) {
        return;
    }

    // set up camera
    _pArcballCam = new CSCI441::ArcballCam();
    _pArcballCam->setLookAtPoint(glm::vec3(0.0f, 0.0f, 0.0f));
    _pArcballCam->setTheta(3.52f);
    _pArcballCam->setPhi(1.9f);
    _pArcballCam->setRadius(15.0f);
    _pArcballCam->recomputeOrientation();
    _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.eyePos, _pArcballCam->getPosition());

    // set up light info
    glm::vec3 lightColor(1.0f, 1.0f, 1.0f);
    _spotLightPos = glm::vec3(0.0f, 10.0f, 0.0f);
    _spotLightDir = glm::vec3(0,-1,0);
    _lightDir = glm::vec3(-1.0f, 0.0f, -1.0f);
    _lightAngle = glm::radians(27.5f);
    float lightCutoff = glm::cos(_lightAngle);
    _lightType = 0;

    _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.lightColor, lightColor);
    _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.spotLightPos, _spotLightPos);
    _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.spotLightPos, _spotLightDir);
    _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.dirLightDir, _lightDir);
    _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.lightCutoff, lightCutoff);
    _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.lightType, _lightType);

    _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.eyePos, _pArcballCam->getPosition());

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

    _skyHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/minesskyrev.png");
    _minesHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/mines.png");

    _cueBallHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/cue.png");
    _8BallHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/8ball.png");
    _solidBallHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/solid.png");
    _stripeBallHandle = CSCI441::TextureUtils::loadAndRegisterTexture("textures/stripe.png");

    //CUE BALL

    setupTable();
}

//*************************************************************************************
//
// Engine Cleanup

void Lab08Engine::mCleanupShaders() {
    fprintf( stdout, "[INFO]: ...deleting Shaders.\n" );
    delete _goodShaderProgram;
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
    if(!_goodShaderProgram || !_flatShaderProgram) {
        return;
    }

    //PANO
    glm::mat4 skyboxpos = glm::translate(glm::mat4(1.0), glm::vec3(0,0,0));
    skyboxpos = glm::rotate(skyboxpos, (float)(90.0 * (3.1415 / 180.0)), glm::vec3(0.0,1.0,0.0));
    glm::mat4 mvpMtx = projMtx * viewMtx * skyboxpos;
    _textureShader->useProgram();
    CSCI441::setVertexAttributeLocations(_textureShaderProgramAttribute.vPos, -1, _textureShaderProgramAttribute.vTexCoord);
    _textureShader->setProgramUniform(_textureShaderProgramUniform.mvpMatrix, mvpMtx);
    _textureShader->setProgramUniform(_textureShaderProgramUniform.colorTint, glm::vec3(1,1,1));
    glBindTexture( GL_TEXTURE_2D, _skyHandle);

    CSCI441::drawSolidSphere(800,64,32);

    //MINES
    glm::mat4 minesPos = glm::translate(glm::mat4(1.0), glm::vec3(0,-12,0));
    minesPos = glm::scale(minesPos, glm::vec3(0.5,0.5,0.5));
    mvpMtx = projMtx * viewMtx * minesPos;
    _textureShader->setProgramUniform(_textureShaderProgramUniform.mvpMatrix, mvpMtx);
    glBindTexture( GL_TEXTURE_2D, _minesHandle);
    mines->draw(_textureShader->getShaderProgramHandle());

    glm::mat4 modelMatrix = glm::mat4(1.0f);

    // use the gouraud shader
    _goodShaderProgram->useProgram();
    CSCI441::setVertexAttributeLocations(_goodShaderProgramAttributeLocations.vPos, _goodShaderProgramAttributeLocations.vNormal, -1);

    //draw pool table
    modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0, 0, 0));
    modelMatrix = glm::scale(modelMatrix, glm::vec3(0.1,0.1,0.1));
    _computeAndSendTransformationMatrices( _goodShaderProgram,
                                           modelMatrix, viewMtx, projMtx,
                                           _goodShaderProgramUniformLocations.mvpMatrix,
                                           _goodShaderProgramUniformLocations.modelMatrix,
                                           _goodShaderProgramUniformLocations.normalMatrix);

    _setMaterialProperties(CSCI441::Materials::GREEN_RUBBER);
    poolGrass->draw(_goodShaderProgram->getShaderProgramHandle());
    _setMaterialProperties(CSCI441::Materials::RUBY);
    poolWood->draw(_goodShaderProgram->getShaderProgramHandle());
    _setMaterialProperties(CSCI441::Materials::BLACK_RUBBER);
    poolHoles->draw(_goodShaderProgram->getShaderProgramHandle());

    //draw cue stick
    drawStick(viewMtx, projMtx);

    //DRAW THE CUE BALL

    drawBalls(viewMtx, projMtx);


    _flatShaderProgram->useProgram();

    if(cueState == 1){
        //draw power meter
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.85,-1,0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.4,meterHeight*2,1));

        _computeAndSendTransformationMatrices(_flatShaderProgram,modelMatrix,glm::mat4(1.f),glm::mat4(1.f),_flatShaderProgramUniformLocations.mvpMatrix);
        _flatShaderProgram->setProgramUniform(_flatShaderProgramUniformLocations.color, glm::vec3(meterHeight/2, 0.6/meterHeight,0.1));
        CSCI441::drawSolidCube(0.25);

        //draw meter background
        modelMatrix = glm::mat4(1.0f);
        modelMatrix = glm::translate(modelMatrix, glm::vec3(-0.85,-1,0));
        modelMatrix = glm::scale(modelMatrix, glm::vec3(0.45,4.05,1));

        _computeAndSendTransformationMatrices(_flatShaderProgram,modelMatrix,glm::mat4(1.f),glm::mat4(1.f),_flatShaderProgramUniformLocations.mvpMatrix);
        _flatShaderProgram->setProgramUniform(_flatShaderProgramUniformLocations.color, glm::vec3(0,0,0));
        CSCI441::drawSolidCube(0.25);
    }
}

void Lab08Engine::_updateScene() {
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.timeSince, (float)glfwGetTime());

    spotLightSwingAngle += spotLightDTheta;
    if(fabs(spotLightSwingAngle) > M_PI_4 ) spotLightDTheta = -spotLightDTheta;
    _spotLightPos.z = glm::sin(spotLightSwingAngle);
    _spotLightDir = glm::vec3(0,-glm::cos(spotLightSwingAngle),glm::sin(spotLightSwingAngle));
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.spotLightPos, _spotLightPos);
    _goofyShaderProgram->setProgramUniform(_goofyShaderProgramUniformLocations.spotLightDir, _spotLightDir);

    _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.spotLightPos, _spotLightPos);
    _goodShaderProgram->setProgramUniform(_goodShaderProgramUniformLocations.spotLightDir, _spotLightDir);

    meterHeight += meterStep;
    if(meterHeight > 2 || meterHeight < 0.1) meterStep = -meterStep;
    sinkBalls();
    if(gamesUnlimitedGames){
        eightBallDestructionMegaLoss();
        if(stripedPlayer == 0 && regularPlayer == 0){
            setPlayerBallTypes();
        }
        else{
            checkWin();
        }
    }
    physics(0.01);
    // check if balls are moving to update shoot status
    if(gamesUnlimitedGames){
        //std::cout << "games";
        if (areBallsMoving()) {
            canShoot = false;
        } else {
            if(!canShoot) {
                canShoot = true;
                // check who's turn it is
                checkSinkTurn();
            }
            // if after the balls stop moving after the first shot and no balls are hit, do the easter egg
            if (currentTurn == 1) {
                if (!myBallsHaveBeenHit) {
                    easterEgg();
                    // reset var so it only is called once
                    myBallsHaveBeenHit = false;
                }
            }
        }
    }
    if(canShoot){
        _pArcballCam->setLookAtPoint( glm::vec3(balls[0]->x,0,balls[0]->y));
        _pArcballCam->recomputeOrientation();
    }
}

void Lab08Engine::checkSinkTurn(){
    if(currentPlayer == stripedPlayer){
        if(sunkThisTurnStriped > 0 && sunkThisTurnRegular == 0){
            sunkThisTurnStriped = 0;
            return;
        }
        else{
            if (currentPlayer == 1) {
                currentPlayer = 2;
            } else {
                currentPlayer = 1;
            }
        }
    }
    else{
        if(sunkThisTurnRegular > 0 && sunkThisTurnStriped == 0){
            sunkThisTurnRegular = 0;
            return;
        }
        else{
            if (currentPlayer == 1) {
                currentPlayer = 2;
            } else {
                currentPlayer = 1;
            }
        }
    }
}
void Lab08Engine::endGame(){
    // TODO: signify the winner, give them dopamine somehow
    std::cout << "player " << winner << " win! " << std::endl;
    std::cout << "sunk balls standard " << sunkRegular << " striped " << sunkStriped << " eight " << sunkEight << std::endl;
    gamesUnlimitedGames = false;
    return;
}

void Lab08Engine::checkWin(){
    if(sunkStriped == 8){
        winner = stripedPlayer;
        endGame();
    }
    else if(sunkRegular == 8){
        winner = regularPlayer;
        endGame();
    }
}

void Lab08Engine::easterEgg(){
    // TODO: easter the egg
    gamesUnlimitedGames = false;
    return;
}

void Lab08Engine::resetGame(){
    sunkEight, sunkRegular, sunkStriped, winner, stripedPlayer,
    regularPlayer, sunkThisTurnStriped, sunkThisTurnRegular, currentTurn = 0;
    myBallsHaveBeenHit = false;
    currentPlayer = 1;
    canShoot = true;
    setupTable();
}

void Lab08Engine::eightBallDestructionMegaLoss(){
    if(sunkEight != 0){
        if(currentPlayer == 1){
            winner = 2;
        }
        else{
            winner = 1;
        }
        endGame();
    }
}
// checks until a player sinks a ball
void Lab08Engine::setPlayerBallTypes(){
    if(sunkRegular > 0){
        regularPlayer = currentPlayer;
        if (currentPlayer == 1) {
            stripedPlayer = 2;
        } else {
            stripedPlayer = 1;
        }
    }
    else if(sunkStriped > 0) {
        stripedPlayer = currentPlayer;
        if (currentPlayer == 1) {
            regularPlayer = 2;
        } else {
            regularPlayer = 1;
        }
    }
}
bool Lab08Engine::areBallsMoving(){
    for(Ball* b: balls){
        if (b->moving){
            return true;
        }
    }
    return false;
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
    if(_goodShaderProgram) {
        // set the D, S, A, & shininess components of the material to our Gouraud Shader
        glProgramUniform3fv( _goodShaderProgram->getShaderProgramHandle(), _goodShaderProgramUniformLocations.materialDiffColor, 1, material.diffuse   );
        glProgramUniform3fv( _goodShaderProgram->getShaderProgramHandle(), _goodShaderProgramUniformLocations.materialSpecColor, 1, material.specular  );
        glProgramUniform1f(  _goodShaderProgram->getShaderProgramHandle(), _goodShaderProgramUniformLocations.materialShininess,    material.shininess );
        glProgramUniform3fv( _goodShaderProgram->getShaderProgramHandle(), _goodShaderProgramUniformLocations.materialAmbColor,  1, material.ambient   );
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

glm::mat4 Lab08Engine::randomRotationMatrix() {

    float angleX = glm::radians(static_cast<float>(rand() % 360));
    float angleY = glm::radians(static_cast<float>(rand() % 360));
    float angleZ = glm::radians(static_cast<float>(rand() % 360));

    glm::mat4 rotationX = glm::rotate(glm::mat4(1.0f), angleX, glm::vec3(1.0f, 0.0f, 0.0f));
    glm::mat4 rotationY = glm::rotate(glm::mat4(1.0f), angleY, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 rotationZ = glm::rotate(glm::mat4(1.0f), angleZ, glm::vec3(0.0f, 0.0f, 1.0f));

    return rotationZ * rotationY * rotationX;
}

Lab08Engine::Ball::Ball(float x, float y, float r, int tex, ballStyle s) {
    this->s = s;
    this->x = x;
    this->y = y;
    this->vx = 0;
    this->vy = 0;
    this->r = r;
    this->rot = glm::mat4(0);
    this->tex = tex;
    this->moving = true;
}

Lab08Engine::Hole::Hole(float x, float y, float r) {
    this->x = x;
    this->y = y;
    this->r = r;
}
