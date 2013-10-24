/**
 * Copyright (c) 2011 Nokia Corporation.
 */


#include "GameWindow.h"
#include "trace.h"

using namespace GE;

/*!
  Create vertex and fragment shaders from provided source code. Create a new shader
  for them to be attached. Fills the references with the object handles.
*/
bool GameWindow::glhelpCreateShader( GLuint &vertexShader,
                                     GLuint &fragmentShader,
                                     GLuint &shaderProgram,
                                     const char *vertexShaderSrc,
                                     const char *fragmentShaderSrc )
{
    bool rval = true;
    vertexShader = 0;
    fragmentShader = 0;
    shaderProgram = 0;
    GLint retval;

    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, (const char**)&fragmentShaderSrc, NULL);
    glCompileShader(fragmentShader);
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &retval);
    if (!retval)
    {
        DEBUG_INFO("GameWindow:glhelpCreateShader: error compiling fragment shader");
            // Failed to compile fragment shader
        rval = false;
    }


    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, (const char**)&vertexShaderSrc, NULL);
    glCompileShader(vertexShader);
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &retval);
    if (!retval)
    {
        DEBUG_INFO("GameWindow:glhelpCreateShader: error compiling vertex shader");
            // Failed to compile vertex shader
        rval = false;
    }

    shaderProgram = glCreateProgram();

        // Attach the fragment and vertex shaders to it
    glAttachShader(shaderProgram, fragmentShader);
    glAttachShader(shaderProgram, vertexShader);
    glLinkProgram(shaderProgram);

    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &retval);
    if (!retval)
    {
        DEBUG_INFO("GameWindow:glhelpCreateShader: error linking the program");
        rval = false;
    }

    return rval;
}

