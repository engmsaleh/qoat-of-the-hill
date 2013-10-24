/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef MYGAMEWINDOW_H
#define MYGAMEWINDOW_H

#include <QVector3D>

#include "audioout.h"
#include "audiomixer.h"


#include <QWidget>
#include <QApplication>
#include "qgameopengles2.h"


#define SELECT_PLAYER_DISTANCE 3.0f
#define BACKGROUND_LAYER_COUNT 9

/*
#define SURFACE_WIDTH  640
#define SURFACE_HEIGHT 360
*/
#define SURFACE_WIDTH  854
#define SURFACE_HEIGHT 480


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





class MyGameApplication : public QApplication
{
    Q_OBJECT

public:
    explicit MyGameApplication(int &argc, char **argv, int = QT_VERSION);
    virtual ~MyGameApplication();

public slots:
    void idleTimer();

public:
        // Called by the eventfilter
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);

    // for porting test. To be changed
    bool isProfileSilent() { return false; }
    int width() { return SURFACE_WIDTH; }
    int height() { return SURFACE_HEIGHT; }
    bool stopAudio();
    bool startAudio();


    void updateGame(const float fFrameDelta);
    void renderFrame();
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


protected:
    void toggleMute();

    void coordsToScreen(QMouseEvent *event, float &x, float &y);
    void screenToWorld(float mx, float my, QVector3D &target);

    virtual int onCreate();
    virtual void onDestroy();

protected: // Data
    GE::AudioOut *m_audioOut;
    GE::AudioMixer m_mixer;

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
