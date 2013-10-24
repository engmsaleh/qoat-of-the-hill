/**
 * Copyright (c) 2011 Nokia Corporation.
 * Programmed by Tuomo Hirvonen.
 */

#include "MyGameWindow.h"

#include <QApplication>
#include <QKeyEvent>
#include <QMatrix4x4>
#include <QTime>
#include <math.h>

#include "audiobuffer.h"
#include "audioout.h"
#include "trace.h"

#include "GameInstance.h"
#include "GameLevel.h"
#include "GameMenu.h"
#include "GamePlayer.h"
#include "ParticleEngine.h"
#include "TextureManager.h"


    // Shaders for clouds at the top and the bottom.
const char* strFragmentShader =
    "uniform sampler2D sampler2d;\n"
    "uniform mediump vec4 pcol;\n"
    "varying mediump vec2 texCoord;\n"
    "void main (void)\n"
    "{\n"
     "    mediump float amount = texture2D(sampler2d, texCoord + pcol.xx).r;\n"
    "    amount = amount+clamp(texture2D(sampler2d, texCoord*2.0 + pcol.yy).r, 0.0, 1.0);\n"
    "    amount = amount*clamp((1.0 - (texCoord[0]*texCoord[0] + texCoord[1]*texCoord[1])), 0.0, 1.0);\n"
    "    gl_FragColor = vec4(0.46, 0.58, 0.87, 0.0)*(1.0-amount) + vec4(amount, amount, amount, 0.0);\n"
    "}";

const char* strVertexShader =
    "attribute highp vec3 vertex;\n"
    "uniform mediump mat4 transMatrix;\n"
    "uniform mediump mat4 projMatrix;\n"
    "varying mediump vec2 texCoord;\n"
    "void main(void)\n"
    "{\n"
         "highp vec4 transVertex = vec4(vertex,1.0)*transMatrix;\n"
         "gl_Position = transVertex * projMatrix;\n"
         "texCoord = vertex.xz*0.02;\n"
    "}";


/*!
  \class MyGameWindow
  \brief The game window.
*/


/*!
  Constructor.
*/
MyGameWindow::MyGameWindow(QWidget *parent)
    : GE::GameWindow(parent),
      m_gameInstance(0),
      m_beat1(0),
      m_shootObject(0),
      m_followObject(0),
      m_playerTurn(0),
      m_turnState(eSHOW_PLAYER),
      m_showResultsCounter(),
      m_mousePressTime(),
      m_mouseOn(false),
      m_bgAngle(),
      m_cloudPos(0.0f),
      m_cameraXPos(0.0f),
      m_cameraZPos(10.0f),
      m_cameraYTarget(0.0f),
      m_cameraXAngle(0.0f),
      m_cameraYOffset(0.0f),
      m_muted(false),
      m_currentButton(1),
      m_prevButton(-1),
      m_buttonFade(1.0f)
{
    srand(QTime::currentTime().msec());
    m_muted = isProfileSilent();
}


/*!
  Destructor.
*/
MyGameWindow::~MyGameWindow()
{

}


/*!
  Starts a new game.
*/
void MyGameWindow::startNewGame()
{
    m_followObject = 0;
    m_shootObject  = 0;
    m_playerTurn = 0;
    m_turnState = eSHOW_PLAYER;
    m_followObject = m_gameInstance->getPlayer(0);
}


/*!
*/
void MyGameWindow::coordsToScreen(QMouseEvent *event, float &x, float &y)
{
    x = (float)event->pos().x() / (float)width() - 0.5f;
    y = (float)event->pos().y() / (float)height() - 0.5f;
}


/*!
*/
void MyGameWindow::screenToWorld(float mx, float my, QVector3D &target)
{
    Q_UNUSED(mx);
    Q_UNUSED(my);

    float *m = m_gameInstance->getCameraMatrix();
    float aspect = (float)width() / (float)height();
    float zmul = m[14] * 0.779f;
    target = QVector3D(m[12] + m_mousePressPos[0] * zmul * aspect,
                       m[13] - (m_mousePressPos[1] + 0.025f) * zmul, 0.0f);
}


/*!
  Toggles mute on and off.
*/
void MyGameWindow::toggleMute()
{
    if (isProfileSilent()) {
        // Will not turn mute off in silent profile.
        m_muted = true;
        stopAudio();
        return;
    }

    if (m_muted)
        m_muted = false;
    else
        m_muted = true;

    // Apply the actual setting.
    if (m_muted) {
        stopAudio();
    }
    else {
        startAudio();
    }
}


/*!
  From QWidget.
*/
void MyGameWindow::mousePressEvent(QMouseEvent *event)
{
    m_followObject = 0;
    coordsToScreen(event, m_mousePressPos[0], m_mousePressPos[1]);

    if (m_mousePressPos[0] > 0.35f && m_mousePressPos[1] < -0.35f) {
        if (m_gameInstance->getCurrentMenu())
            toggleMute();
        else
            onPause();

        return;
    }

    if (m_gameInstance->getCurrentMenu()) {
        return;
    }

    m_mouseOn = true;
    m_mousePressTime = 0.0f;
    QVector3D worldPos;
    screenToWorld(m_mousePressPos[0], m_mousePressPos[1], worldPos);

    if (m_turnState == eSHOW_PLAYER) {
        QVector3D playerPos = m_gameInstance->getPlayer(m_playerTurn)->pos();
        QVector3D delta = playerPos - worldPos;

        if (delta.length() < SELECT_PLAYER_DISTANCE) {
            // Select player distance
            m_turnState = eSHOOTING;
        }
    }
}


/*!
  From QWidget.
*/
void MyGameWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_gameInstance->getCurrentMenu() || !m_mouseOn)
        return;

    float mx;
    float my;
    coordsToScreen(event, mx, my);
    float dx,dy;
    dx = (m_mousePressPos[0] - mx);
    dy = (m_mousePressPos[1] - my);

    m_mousePressPos[0] = mx;
    m_mousePressPos[1] = my;

    QVector3D worldPos;
    screenToWorld(mx, my, worldPos);

    if (m_turnState == eSHOOTING) {
        m_gameInstance->resetShowHelpTimer();
        m_gameInstance->getPlayer(m_playerTurn)->aimTo(worldPos);
        QVector3D shooterPos =
                m_gameInstance->getPlayer(m_playerTurn)->pos();

        QVector3D aim =  QVector3D(
                    -(worldPos.x() - shooterPos.x()),
                    -(worldPos.y() - shooterPos.y()),
                    0.0f);

        // Rotate the indicator arrow accordingly
        GameObject *ia = m_gameInstance->indicatorArrow();
        ia->pos() = shooterPos + aim;
        float dragLength = aim.length();

        // Auto zoom out.
        float camzoom = m_cameraZPos - 15.0f;
        float tarzoom = dragLength * 2.0f;

        if (camzoom < tarzoom)
            camzoom = tarzoom;

        m_cameraZPos = 15.0f + camzoom;

        aim.normalize();
        ia->setUpVector(aim.x(), -aim.y());
        return;
    }

    // Camera movement
    m_cameraXPos += dx * fabsf(GAME_LEVEL_ZBASE - m_cameraZPos);
    m_cameraXAngle -= dx * 40.0f;
    m_cameraZPos -= dy * fabsf(GAME_LEVEL_ZBASE - m_cameraZPos);
}


/*!
  From QWidget.
*/
void MyGameWindow::mouseReleaseEvent(QMouseEvent *event)
{
    float mx;
    float my;
    coordsToScreen(event, mx, my);

    if (m_gameInstance->getCurrentMenu()) {
        if (my > 0.1f && my < 0.3f) {
            if (mx < -0.17f && mx > -0.4f) {
                m_gameInstance->getCurrentMenu()->select(0);
            }
            else if (mx > 0.17f && mx < 0.4f) {
                m_gameInstance->getCurrentMenu()->select(1);
            }
        }

        return;
    }

    m_mouseOn = false;

    QVector3D worldPos;
    screenToWorld(mx, my, worldPos);

    if (m_turnState == eSHOOTING) {
        m_gameInstance->m_sampleShoot->playWithMixer(getMixer());

        // Create new ammunition
        m_shootObject = m_gameInstance->getObjectManager()->addObject(
                    new GameAmmunition(m_gameInstance));

        m_shootObject->pos() =
                m_gameInstance->getPlayer(m_playerTurn)->gunPos();

        QVector3D shooterPos =
                m_gameInstance->getPlayer(m_playerTurn)->pos();

        m_gameInstance->getPlayer(m_playerTurn)->setShootVector(
                    shooterPos - worldPos);

        m_shootObject->dir() = QVector3D(
                    -(worldPos.x() - shooterPos.x())*10.0f,
                    -(worldPos.y() - shooterPos.y())*10.0f,
                    0.0f);

        GameObject *gun = m_gameInstance->getPlayer(m_playerTurn)->gun();
        float flipm = -0.45f;

        if (gun->getFlipX())
            flipm = 0.45f;

        QVector3D d = m_shootObject->dir();
        gun->pos() -= d * 5.0f;
        m_gameInstance->getPlayer(m_playerTurn)->dir() -= d / 2.0f;
        d /= 2.0f;

        m_gameInstance->getPlayer(m_playerTurn)->stopAiming();
        m_shootObject->setOnGround(false);
        m_followObject = m_shootObject;
        m_turnState = eSHOW_RESULTS;
        m_shootObject = 0;
        m_gameInstance->killHelp();

        return;
    }
}


/*!
  From QWidget.
*/
void MyGameWindow::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
        case Qt::Key_P: {
            static bool paused = false;

            if (!paused)
                pause();
            else
                resume();

            paused = !paused;
            break;
        }
    case Qt::Key_Q: {
            QApplication::quit();
            break;
        }
    }
}


/*!
*/
void MyGameWindow::onUpdate(const float fFrameDeltaOld)
{
#ifdef QOTH_MEASURE_FPS
    static float lpft = 0.01f;
    lpft = lpft*0.98f + fFrameDeltaOld * 0.02f;
    static int fc = 0;
    fc++;
    if (fc>50) {
        qDebug() << "avg fps: " << (1.0f / lpft);
        fc = 0;
    }

#endif
    //Q_UNUSED(fFrameDeltaOld);
    float frameTime = fFrameDeltaOld*0.33f;         // little slowdown
    //if (frameTime>0.1f) frameTime = 0.1f;
    //qDebug() << frameTime;

    /*
    // Static frame times for different operating systems were decided to be
    // used due to the very low accuracy of the timer in the Symbian platform.
 #ifndef Q_OS_WIN
    float frameTime  = 1.0f / 40.0f;
#else
    float frameTime  = 1.0f / 200.0f;
#endif
*/
    //qDebug() << "update ft: " << fFrameDeltaOld;

    m_cloudPos += frameTime;
    m_bgAngle += frameTime;
    m_cameraXAngle -= m_cameraXAngle * frameTime * 5.0f;

    m_buttonFade -= (m_buttonFade * frameTime * 5.0f);

    if (m_gameInstance) {
        m_gameInstance->run(frameTime, m_playerTurn);

        if (m_gameInstance->isRestarted()) {
            startNewGame();
        }
    }

    if (m_mouseOn) {
        m_mousePressTime += frameTime;
    }

    if (m_gameInstance->getCurrentMenu())
        m_cameraYOffset += (30.0f - m_cameraYOffset) * frameTime * 5.0f;
    else
        m_cameraYOffset += (0.0f - m_cameraYOffset) * frameTime * 5.0f;

    if (m_followObject) {
        // Follow object is set, follow it with the camera.
        if (m_followObject->isDying()) {
            m_followObject = 0;
            m_showResultsCounter = 0.0f;
        } else {
            m_cameraXPos += (m_followObject->pos().x()
                             - m_cameraXPos) * frameTime * 20.0f;
            m_cameraYTarget += ((m_followObject->pos().y() - 2.5f)
                                - m_cameraYTarget) * frameTime * 20.0f;
        }
    }
    else {
        // No object to follow; target the level's height.
        float height = 0.0f;

        if (m_gameInstance->getLevel()) {
            height = m_gameInstance->getLevel()
                     ->getHeightAndNormalAt(m_cameraXPos, 0);
        }

        if (height < 0.0f)
            height = 0.0f;

        height -= 3.0f;
        m_cameraYTarget += (height - m_cameraYTarget) * frameTime * 5.0f;
    }

    switch (m_turnState) {
        case eSHOW_PLAYER:
            break;
        case eSHOOTING:
            break;
        case eSHOW_RESULTS:
            m_gameInstance->resetShowHelpTimer();

            if (!m_followObject) {
                m_showResultsCounter += frameTime;

                if (m_showResultsCounter > 1.0f) {
                    if (m_playerTurn == 0)
                        m_playerTurn = 1;
                    else
                        m_playerTurn = 0;

                    m_turnState = eSHOW_PLAYER;
                    m_followObject = m_gameInstance->getPlayer(m_playerTurn);
                }
            }

            break;
    }


    // Camera limits
    if (m_cameraZPos < 15.0f)
        m_cameraZPos = 15.0f;

    if (m_cameraZPos > 50.0f)
        m_cameraZPos = 50.0f;

    if (m_cameraYTarget>10.0f) {
        float rover = (m_cameraYTarget - 10.0f) / 10.0f;
        m_cameraYTarget = 10.0f + (rover / (1.0f + rover)) * 10.0f;
    }

    float xsee = m_cameraZPos * 0.4f;
    float xlimit = GAME_LEVEL_END_X-xsee;

    if (xlimit < 0.0f)
        xlimit = 0.0f;

    if (m_cameraXPos < -xlimit)
        m_cameraXPos = -xlimit;

    if (m_cameraXPos > xlimit)
        m_cameraXPos = xlimit;

    if (m_shootObject) {
        m_shootObject->pos() =  m_gameInstance->getPlayer(m_playerTurn)->gunPos();
        QVector3D d = QVector3D(0.0f, 0.0f, 0.0f);
        m_shootObject->dir() = d;
    }
}


/*!
  Renders the clouds.
*/
void MyGameWindow::renderClouds()
{
    glDisable(GL_BLEND);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glUseProgram(m_program);
    glBindTexture(GL_TEXTURE_2D, m_cloudTexture);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "projMatrix"), 1,
                       GL_FALSE, m_gameInstance->getProjectionMatrix());
    glUniform1i(glGetUniformLocation(m_program, "sampler2d"), 0);
    glEnableVertexAttribArray(0);

    // Pass the vertex data
    GLfloat vertices[] = { -50.0f, 0.0f, -50.0f, 50.0f, 0.0f, -50.0f,
                           50.0f, 0.0f, 50.0f, -50.0f, 0.0f, 50.0f };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    float id[16];
    memset(id, 0, sizeof(float) * 16);
    id[0] = 1.0f;
    id[5] = 1.0f;
    id[10] = 1.0f;
    id[15] = 1.0f;
    id[7] = -2.0f;
    m_gameInstance->cameraTransform(id);

    glUniformMatrix4fv(glGetUniformLocation(m_program, "transMatrix"), 1,
                       GL_FALSE, id);

    GLfloat col[4];
    col[0] = m_cloudPos * 0.05f;
    col[1] = m_cloudPos * 0.2f;
    col[2] = 1.0f;
    col[3] = 1.0f;
    int colorLocation = glGetUniformLocation(m_program, "pcol");
    glUniform4fv(colorLocation, 1, col);
    glDrawArrays(GL_TRIANGLE_FAN, 0,4);

    // Upside
    memset(id, 0, sizeof(float) * 16);
    id[0] = 1.0f;
    id[5] = 1.0f;
    id[10] = 1.0f;
    id[15] = 1.0f;
    id[7] = 4.0f;
    m_gameInstance->cameraTransform(id, false);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "transMatrix"), 1,
                       GL_FALSE, id);

    glDrawArrays(GL_TRIANGLE_FAN, 0,4);
}


/*!
  Renders the background.
*/
void MyGameWindow::renderBg()
{
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    GLuint program = m_gameInstance->getParticleEngine()->normalProgram();
    glUseProgram(program);
    glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1,
                       GL_FALSE, m_gameInstance->getProjectionMatrix());
    glUniform1i(glGetUniformLocation(program, "sampler2d"), 0);
    glEnableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    GLfloat vertices[] = { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f,
                           1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    GLfloat col[4];
    col[0] = 1.0f;
    col[1] = 1.0f;
    col[2] = 1.0f;
    col[3] = 1.0f;
    int colorLocation = glGetUniformLocation(program, "pcol");
    glUniform4fv(colorLocation, 1, col);
    float id[16];

    for (int f = 0; f < BACKGROUND_LAYER_COUNT; f++) {
        glBindTexture(GL_TEXTURE_2D, m_bgLayers[f].texture);
        memset(id, 0, sizeof(float) * 16);
        id[0] = m_bgLayers[f].xsize;
        id[5] = m_bgLayers[f].ysize;
        id[10] = 1.0f;
        id[15] = 1.0f;
        id[3] = m_bgLayers[f].pos[0];
        id[7] = m_bgLayers[f].pos[1];
        id[11] = m_bgLayers[f].pos[2];

        if (f<3) { // cloud layer
            id[3] += sinf(m_bgAngle * 0.1f) * m_bgLayers[f].xsize
                    * cosf((float)f * m_bgAngle * 0.01f);
        }

        m_gameInstance->cameraTransform(id);
        glUniformMatrix4fv(glGetUniformLocation(program, "transMatrix"), 1,
                           GL_FALSE, id);
        glDrawArrays(GL_TRIANGLE_FAN, 0,4);
    }
}


/*!
*/
void MyGameWindow::renderStaticButtons()
{
    GLuint program = m_gameInstance->getObjectManager()->m_program;
    glUseProgram(program);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    float m[16];
    memset(m, 0, sizeof(float) * 16);
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
    glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"),
                       1, GL_FALSE, m);//m_gameInstance->getProjectionMatrix());
    glUniform1i(glGetUniformLocation(program, "sampler2d"), 0);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glBindTexture(GL_TEXTURE_2D, m_buttonsTexture);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    GLfloat col[4];
    col[0] = 1.0f;
    col[1] = 1.0f;
    col[2] = 1.0f;

    // Vertex coordinates.
    GLfloat vertices[] = { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f,
                           1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f };
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);

    int curbut = m_currentButton;
    m_currentButton = 0;

    if (m_gameInstance->getCurrentMenu()) {
        if (m_muted)
            m_currentButton = 2;
        else
            m_currentButton = 1;
    }

    // Button has been changed, update previous button
    if (curbut != m_currentButton) {
        m_prevButton = curbut;
        m_buttonFade = 1.0f;
    }

    // Setup a matrix
    memset(m, 0, sizeof(float) * 16);

    m[10] = 1.0f;
    m[15] = 1.0f;
    m[3] = 0.85f;
    m[7] = 0.85f;
    m[11] = -1.0f;

    if (m_prevButton >= 0 && m_buttonFade > 0.0f) {
        col[3] = m_buttonFade;
        m[0] = 0.15f + (1.0f-m_buttonFade) * 0.14f - (1.0f - m_buttonFade)
                * (1.0f - m_buttonFade) * 0.28f;
        m[1] = 0.0f;
        m[4] = -m[1];
        m[5] = m[0];

        glUniform4fv(glGetUniformLocation(program, "pcol"), 1, col);
        GLfloat texCoords[] = { 0.0f, (float)(m_prevButton + 1) / 3.0f,
                                1.0f, (float)(m_prevButton + 1) / 3.0f,
                                1.0f, (float)m_prevButton / 3.0f,
                                0.0f, (float)m_prevButton / 3.0f };

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
        glUniformMatrix4fv(glGetUniformLocation(program, "transMatrix"), 1, GL_FALSE, m);
        glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    }

    col[3] = 1.0f - m_buttonFade;
    m[0] = (1.0f - m_buttonFade) * 0.15f;
    m[1] = 0.0f;
    m[4] = -m[1];
    m[5] = m[0];
    glUniform4fv(glGetUniformLocation(program, "pcol"), 1, col);
    GLfloat texCoords[] = { 0.0f, (float)(m_currentButton + 1) / 3.0f,
                            1.0f, (float)(m_currentButton + 1) / 3.0f,
                            1.0f, (float)m_currentButton / 3.0f,
                            0.0f, (float)m_currentButton / 3.0f};

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, texCoords);
    glUniformMatrix4fv(glGetUniformLocation(program, "transMatrix"), 1, GL_FALSE, m);

    // Render the quad
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(1);
}


/*!
*/
void MyGameWindow::onRender()
{
    // Set the camera matrix
    QMatrix4x4 cam;
    cam.setToIdentity();
    float t = (m_cameraZPos - 5.0f) / 5.0f;

    if (t < 0.0f)
        t = 0.0f;

    cam.translate(m_cameraXPos,
                  m_cameraYTarget + m_cameraYOffset + t,m_cameraZPos);
    cam.rotate(m_cameraXAngle, 0.0f, 1.0f, 0.0f);
    m_gameInstance->setCamera(cam);

    // Clear background and depth buffer
    glClearColor(0.46f, 0.58f, 0.87f, 0.0f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    renderClouds();

    if (m_gameInstance->getLevel()) {
        m_gameInstance->getLevel()->render();
    }

    renderBg();

    // Background objects
    if (m_gameInstance->getObjectManager())
        m_gameInstance->getObjectManager()->render(true);

    m_gameInstance->renderParticleTypes();

    // Foreground objects
    if (m_gameInstance->getObjectManager())
        m_gameInstance->getObjectManager()->render(false);

    // Audio on/off overlay
    renderStaticButtons();

    if (m_gameInstance->getCurrentMenu()) {
        m_gameInstance->getCurrentMenu()->render();
    }

    //renderHelp();

    glDepthMask(GL_TRUE);
}


/*!
*/
void MyGameWindow::onCreate()
{
    DEBUG_POINT;

    if (!glhelpCreateShader( m_vertexShader, m_fragmentShader, m_program, strVertexShader, strFragmentShader )) {
        DEBUG_INFO("MyGameWindow: Failed to create shaderprogram. ");
    }

    // Bind the custom vertex attributes
    glBindAttribLocation(m_program, 0, "vertex");
    glBindAttribLocation(m_program, 1, "uv");

    DEBUG_INFO("Finished!");

    m_gameInstance = new GameInstance(width(), height(), this);

    m_cloudTexture =
            m_gameInstance->getTextureManager()->getTexture(":/clouds.png");

    // Setup the background layers
    for (int f = 0; f < BACKGROUND_LAYER_COUNT; f++) {
        m_bgLayers[f].pos[0] =
                ((float)cosf((float)(f) / (BACKGROUND_LAYER_COUNT - 1)
                             * 3.14159f * 2.0f * 1.5f)
                * 1.4f + ((rand() & 255) / 255.0f - 0.5f) / 2.0f)
                * (float)((BACKGROUND_LAYER_COUNT - f) + 10) * 2.0f;

        m_bgLayers[f].pos[1] = -3.0f + (float)f
                + ((float)(rand() & 255) / 255.0f - 0.5f) * 1.0f;
        m_bgLayers[f].pos[2] = -40.0f + (float)f * 4.0f;

        if (f < 3) {
            m_bgLayers[f].texture =
                    m_gameInstance->getTextureManager()->getTexture(":/bg2.png");
            m_bgLayers[f].xsize = 30.0f;
            m_bgLayers[f].ysize = 15.0f;
            m_bgLayers[f].pos[1] += 10.0f;
        }
        else {
            if (f < 6) {
                m_bgLayers[f].texture =
                        m_gameInstance->getTextureManager()->getTexture(":/bg0.png");
                m_bgLayers[f].xsize = 20.0f;
                m_bgLayers[f].ysize = 10.0f;
           } else {
                m_bgLayers[f].texture =
                        m_gameInstance->getTextureManager()->getTexture(":/bg1.png");
                m_bgLayers[f].pos[1] -= 2.0f;
                m_bgLayers[f].xsize = 10.0f + (float)(rand() & 255) / 64.0f;
                m_bgLayers[f].ysize = 6.0f + (float)(rand() & 255) / 64.0f;
            }
        }

        if ((rand() & 255) < 128)
            m_bgLayers[f].xsize *= -1.0f;
    }

    m_buttonsTexture =
            m_gameInstance->getTextureManager()->getTexture(":/sound_onoff.png");


    if (isProfileSilent()) {
        m_muted = true;
    }
    else {
        startAudio();
        m_muted = false;
    }

    getMixer().setAbsoluteVolume(0.5f);
}


/*!
*/
void MyGameWindow::setSize(int width, int height)
{
    DEBUG_INFO(width << "x" << height);

    if (m_gameInstance)
        m_gameInstance->setSize(width, height);
}


/*!
  Displays the pause menu.
*/
void MyGameWindow::onPause()
{
    if (!m_gameInstance->getCurrentMenu()) {
        // Create pause menu
        m_gameInstance->setCurrentMenu(new GameMenu(m_gameInstance, 5, 3, 4));
    }
}


/*!
  Called when resuming the game. Checks the profile and mutes the game if
  necessary. If the game was running, sets the player who has the turn
  as the object to follow (with the camera).
*/
void MyGameWindow::onResume()
{
    m_muted = isProfileSilent();

    // If game is on, re-track the active player when returned from pause menu.
    if (m_gameInstance) {
        GamePlayer *player = m_gameInstance->getPlayer(m_playerTurn);

        if (player) {
            m_followObject = player;
        }
    }
}


/*!
  Called when the game window is about to be destroyed. Frees allcated
  resources.
*/
void MyGameWindow::onDestroy()
{
    glDeleteProgram(m_program);
    glDeleteShader(m_fragmentShader);
    glDeleteShader(m_vertexShader);

    delete m_beat1;
    delete m_gameInstance;
}

