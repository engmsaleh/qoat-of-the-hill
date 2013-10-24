/**
 * Copyright (c) 2011 Nokia Corporation.
 */



/*!
    Just a very small part from GameEnabler's GameWindow for
    getting the glhelpCreateShader function.
*/
#include "qgameopengles2.h"

namespace GE {
    class GameWindow {
    public:
        static bool glhelpCreateShader( GLuint &vertexShader,
                                               GLuint &fragmentShader,
                                               GLuint &shaderProgram,
                                               const char *vertexShaderSrc,
                                               const char *fragmentShaderSrc );
    };
}

