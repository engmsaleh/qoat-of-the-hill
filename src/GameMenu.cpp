/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "GameMenu.h"

#include <QApplication>
#include <math.h>

#include "GameInstance.h"
#include "GameObject.h"
#include "TextureManager.h"


/*!
  \class GameMenu
  \brief -
*/


/*!
  Constructor.
*/
GameMenu::GameMenu(GameInstance *gameInstance,
                   int logoIndex,
                   int button1Index,
                   int button2Index)
    : m_gameInstance(gameInstance),
      m_logoIndex(logoIndex),
      m_button1Index(button1Index),
      m_button2Index(button2Index),
      m_selected(-1),
      m_selectedCounter(0.0f),
      m_counter(0.0f),
      m_finished(false)
{
    m_textTexture = gameInstance->getTextureManager()->getTexture(":/texts.png");
}


/*!
  Destructor.
*/
GameMenu::~GameMenu()
{
}


/*!
*/
bool GameMenu::run(float frameTime)
{
    m_counter += frameTime;
    if (m_selected != -1)
        m_selectedCounter += frameTime * 6.0f;

    if (m_selectedCounter > 1.0f) {
        // Apply the action
        int action = m_button1Index;

        if (m_selected == 1)
            action = m_button2Index;

        switch (action) {
            case 1: // Start
                m_gameInstance->restartGame();
                break;
            case 2: // Exit
                QApplication::exit();
                break;
            case 3: // Resume, does nothing
                break;
            case 4: // End game, go to the main menu
                m_gameInstance->toMainMenu();
                break;
        }

        return false;
    }

    if (m_finished)
        return false; // Die

    return true; // Do we want to continue running
}


/*!
*/
void GameMenu::render()
{
    GLuint program = m_gameInstance->getObjectManager()->m_program;

    glUseProgram(program);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"),
                       1, GL_FALSE, m_gameInstance->getProjectionMatrix());
    glUniform1i(glGetUniformLocation(program, "sampler2d"), 0);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindTexture(GL_TEXTURE_2D, m_textTexture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    GLfloat col[4];
    col[0] = 1.0f;
    col[1] = 1.0f;
    col[2] = 1.0f;
    col[3] = m_counter * 2.0f;

    if (col[3] > 1.0f)
        col[3] = 1.0f;

    col[3] -= m_selectedCounter;
    glUniform4fv(glGetUniformLocation(program, "pcol"), 1, col);

    // Vertex coordinates
    GLfloat vertices[] = { -4.0f, -1.0f, 0.0f, 4.0f, -1.0f, 0.0f,
                           4.0f, 1.0f, 0.0f, -4.0f, 1.0f, 0.0f };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    float ftemp = 1.0f + 10.0f / (m_counter * 200.0f + 1.0f);
    drawText(program, 0.0f, 1.0f, ftemp * 1.4f,
             (ftemp - 1.0f) / 2.0f, m_logoIndex);

    float bs;

    if (m_selected == 0)
        bs = -0.2f + m_selectedCounter;
    else
        bs = 0.0f;

    drawText(program, -4.0f, -2.0f, ftemp + bs,
             -(ftemp - 1.0f) / 2.0f, m_button1Index);

    if (m_selected == 1)
        bs = -0.2f + m_selectedCounter;
    else
        bs = 0.0f;

    drawText(program, 4.0f, -2.0f, ftemp + bs,
             -(ftemp - 1.0f) / 2.0f, m_button2Index);
}


/*!
*/
void GameMenu::select(int button)
{
    m_selected = button;
    m_selectedCounter = 0.0f;
}


/*!
*/
void GameMenu::drawText(GLuint program,
                        float x, float y,
                        float scale, float angle, int index)
{
    // UV coordinates
    float v1 = (float)index / 8.0f;
    float v2 = (float)(index + 1) / 8.0f;
    GLfloat texCoords[] = {0.0f, v2, 1.0f, v2, 1.0f, v1, 0.0f, v1};
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texCoords);

    // Setup a matrix for the text
    float m[16];
    memset(m, 0, sizeof(float) * 16);
    m[0] = cosf(angle) * scale;
    m[1] = sinf(angle) * scale;
    m[4] = -m[1];
    m[5] = m[0];
    m[10] = 1.0f;
    m[15] = 1.0f;
    m[3] = x;
    m[7] = y;
    m[11] = -10.0f;
    glUniformMatrix4fv(glGetUniformLocation(program, "transMatrix"),
                       1, GL_FALSE, m);

    // Render the quad
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}

