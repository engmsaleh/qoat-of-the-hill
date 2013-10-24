/**
 * Copyright (c) 2011 Nokia Corporation.
 * Programmed by Tuomo Hirvonen.
 */

#ifndef MYGAMEWINDOW_H
#define MYGAMEWINDOW_H

#include <GLES2/gl2.h>
#include <QVector3D>

#include "gamewindow.h"

#define SELECT_PLAYER_DISTANCE 3.0f
#define BACKGROUND_LAYER_COUNT 9

// Forward declarations
class GameInstance;
class GameObject;
class QKeyEvent;
class QMouseEvent;

namespace GE {
    class AudioBuffer;
}


enum eTURNSTATE {
    eSHOW_PLAYER,
    eSHOOTING,
    eSHOW_RESULTS
};


class MyGameWindow : public GE::GameWindow
{
    Q_OBJECT

public:
    explicit MyGameWindow(QWidget *parent = 0);
    virtual ~MyGameWindow();

public:
    void onUpdate(const float fFrameDelta);
    void onRender();
    void onPause();
    void onResume();
    void setSize(int w, int h);

    // Game specific
    void startNewGame();

protected: // Data types

    struct SBackGroundLayer {
        GLuint texture;
        float xsize,ysize;
        float pos[3];
    };

protected:
    void renderBg();
    void renderClouds();
    void renderStaticButtons();

protected: // From QWidget
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void keyPressEvent(QKeyEvent *event);

protected:
    void toggleMute();

    void coordsToScreen(QMouseEvent *event, float &x, float &y);
    void screenToWorld(float mx, float my, QVector3D &target);

    virtual void onCreate();
    virtual void onDestroy();

protected: // Data
    GameInstance *m_gameInstance; // Owned
    GE::AudioBuffer *m_beat1; // Owned

    // Game logic specific
    GameObject *m_shootObject;
    GameObject *m_followObject;
    int m_playerTurn;
    eTURNSTATE m_turnState;
    float m_showResultsCounter;

    float m_mousePressPos[2];
    float m_mousePressTime;
    bool m_mouseOn;

    SBackGroundLayer m_bgLayers[BACKGROUND_LAYER_COUNT];
    float m_bgAngle;

    // Clouds
    GLuint m_fragmentShader;
    GLuint m_vertexShader;
    GLuint m_program;
    GLuint m_cloudTexture;
    float m_cloudPos;

    float m_cameraXPos;
    float m_cameraZPos;
    float m_cameraYTarget;
    float m_cameraXAngle;
    float m_cameraYOffset;

    // Buttons
    bool m_muted;
    GLint m_buttonsTexture;
    int m_currentButton;
    int m_prevButton;
    float m_buttonFade;
};


#endif // MYGAMEWINDOW_H
