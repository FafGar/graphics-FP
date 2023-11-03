
#include "FPEngine.h"

///*****************************************************************************
//
// Our main function
int main() {

    auto labEngine = new Lab08Engine();
    labEngine->initialize();
    if (labEngine->getError() == CSCI441::OpenGLEngine::OPENGL_ENGINE_ERROR_NO_ERROR) {
        labEngine->run();
    }
    labEngine->shutdown();
    delete labEngine;

    return EXIT_SUCCESS;
}
