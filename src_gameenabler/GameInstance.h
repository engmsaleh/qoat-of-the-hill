/**
 * Copyright (c) 2011 Nokia Corporation.
 * Programmed by Tuomo Hirvonen.
 */

#ifndef GAMEINSTANCE_H
#define GAMEINSTANCE_H

#include <GLES2/gl2.h>
#include <QMatrix4x4>

#include "gamewindow.h"

#define GAME_NOF_PLAYERS 2
#define GAME_MAX_PARTICLES 512

// Forward declarations
class GameLevel;
class GameMenu;
class GameObject;
class GameObjectManager;
class GamePlayer;
class ParticleEngine;
class ParticleType;
class TextureManager;

namespace GE {
    class AudioBuffer;
    class AudioBufferPlayInstance;
    class AudioMixer;
}


class GameInstance
{
public:
    GameInstance(int width, int height, GE::GameWindow *gameWindow);
    ~GameInstance();

public:
    inline GE::AudioMixer *getMixer() { return m_mixer; }

    void restartGame();

    bool isRestarted()
    {
        bool rval = m_restarted;
        m_restarted = false;
        return rval;
    }

    void toMainMenu() { m_toMainMenu = true; }
    inline GameMenu *getCurrentMenu() { return m_currentMenu; }

    void setCurrentMenu(GameMenu *newMenu);

    inline GameLevel *getLevel() { return m_level; }
    inline ParticleEngine *getParticleEngine() { return m_particleEngine; }
    inline GameObjectManager *getObjectManager() { return m_objManager; }
    inline GamePlayer *getPlayer(int index) { return m_players[index]; }
    inline TextureManager *getTextureManager() { return m_textureManager; }
    inline void markFireBurning() { m_fireTargetVolume = 1.0f; }

    int run(float frameTime, int m_playerTurn);
    void setCamera(QMatrix4x4 &camera);

    inline float *getCameraMatrix() { return m_cameraMatrix; }
    inline float *getProjectionMatrix() { return m_projectionMatrix; }

    void setSize(int width, int height);
    void cameraTransform(float *m, bool transformPosition = true);
    static GLint loadGLTexture(QString fileName);
    void renderParticleTypes();

    inline GameObject *indicatorArrow() { return m_indicatorArrow; }
    bool audioEnabled() { return m_gameWindow->audioEnabled(); }

    void killHelp();

    void resetShowHelpTimer() { m_showHelpTimer = 0.0f; }

protected:
    void initParticles();
    void initSamples();
    void recreateHelp();
    GameObject *placeTree();

public: // Data
    // Particle types for free use
    ParticleType *m_basicFireParticle; // Owned
    ParticleType *m_explosionFlareParticle; // Owned
    ParticleType *m_smokeParticle; // Owned
    ParticleType *m_smallSmokeParticle; // Owned
    ParticleType *m_dustParticle; // Owned

    // Audio samples
    GE::AudioBuffer *m_sampleShoot; // Owned
    GE::AudioBuffer *m_sampleExplosion; // Owned
    GE::AudioBuffer *m_sampleWhistle; // Owned
    GE::AudioBuffer *m_sampleFire; // Owned
    GE::AudioBuffer *m_sampleHurt; // Owned
    GE::AudioBuffer *m_sampleBackground; // Owned

protected: // Data
    GE::GameWindow *m_gameWindow; // Not owned
    GameMenu *m_currentMenu; // Owned
    GE::AudioBufferPlayInstance *m_fireBurn; // Owned
    GE::AudioMixer *m_mixer; // Not owned
    GameObject *m_tip;
    GameObject *m_indicatorArrow;
    GameObject *m_indicatorCircle;
    GameObject *m_activePlayerIndicatorCircle;
    TextureManager *m_textureManager; // Owned
    GameLevel *m_level; // Owned
    ParticleEngine *m_particleEngine; // Owned
    GameObjectManager *m_objManager; // Owned
    GamePlayer *m_players[GAME_NOF_PLAYERS];
    float m_helpAngle;
    float m_helpPullState;
    int m_helpMode;
    float m_showHelpTime;
    float m_showHelpTimer;
    bool m_toMainMenu;
    bool m_restarted;
    float m_fireTargetVolume;
    float m_fireVolume;
    float m_arrowAngle;
    float m_cameraMatrix[16];
    float m_projectionMatrix[16];
};


#endif // GAMEINSTANCE_H
