/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "GameInstance.h"

#include <QImage>
#include <math.h>

#include "audiobuffer.h"
#include "audiobufferplayinstance.h"
#include "audiomixer.h"

#include "GameLevel.h"
#include "GameMenu.h"
#include "GameObject.h"
#include "GamePlayer.h"
#include "ParticleEngine.h"
#include "TextureManager.h"

// Constants
const int TreeCount(30);


/*!
  \class GameInstance
  \brief The main class of the game engine.
*/


/*!
  Constructor.
*/
GameInstance::GameInstance(int width, int height, GE::AudioMixer *mixer)
    : m_currentMenu(0),
      m_fireBurn(0),
      m_mixer(0),
      m_tip(0),
      m_indicatorArrow(0),
      m_indicatorCircle(0),
      m_activePlayerIndicatorCircle(0),
      m_textureManager(0),
      m_level(0),
      m_particleEngine(0),
      m_objManager(0),
      m_helpAngle(0.0f),
      m_helpPullState(0.0f),
      m_helpMode(0),
      m_showHelpTime(1.5f),
      m_showHelpTimer(0.0f),
      m_toMainMenu(false),
      m_restarted(false),
      m_fireTargetVolume(0.0f),
      m_fireVolume(0.0f),
      m_arrowAngle(0.0f)
{
    // Initialize player array pointers.
    for (int i = 0; i < GAME_NOF_PLAYERS; ++i) {
        m_players[i] = 0;
    }

        // NOTE, FIX THIS.. DOES NOT WORK
    m_mixer = mixer;
    setSize(width, height);

    m_textureManager = new TextureManager();
    m_objManager = new GameObjectManager(this);

    m_particleEngine = new ParticleEngine(this, GAME_MAX_PARTICLES);
    initParticles();
    initSamples();

    // Start the background ambient noise.
    GE::AudioBufferPlayInstance *bgSample =
        m_sampleBackground->playWithMixer(*m_mixer);

    if (bgSample) {
        // Loop the sample forever.
        bgSample->setLoopCount(-1);
    }

    // Launch the main menu.
    m_currentMenu = new GameMenu(this, 0, 1,2);
}


/*!
  Destructor.
*/
GameInstance::~GameInstance()
{
    delete m_currentMenu;
    delete m_textureManager;
    delete m_level;
    delete m_particleEngine;
    delete m_objManager;

    delete m_basicFireParticle;
    delete m_explosionFlareParticle;
    delete m_smokeParticle;
    delete m_smallSmokeParticle;
    delete m_dustParticle;

    delete m_sampleShoot;
    delete m_sampleExplosion;
    delete m_sampleWhistle;
    delete m_sampleFire;
    delete m_sampleHurt;
    delete m_sampleBackground;
}


/*!
  Restarts the game.
*/
void GameInstance::restartGame()
{
    m_restarted = true;
    delete m_level;
    m_level = new GameLevel(this);
    m_objManager->destroyAll();
    m_level->recreate();

    // Add some trees.
    for (int i = 0; i < TreeCount; ++i)
        placeTree();

    // Create players.
    for (int i = 0; i < GAME_NOF_PLAYERS; ++i) {
        m_players[i] =
                (GamePlayer*)m_objManager->addObject(new GamePlayer(this));
        m_players[i]->createAssets();
        m_players[i]->pos().setX(
                GAME_LEVEL_START_X + 8.0f
                + (((float)(rand() & 255) / 255.0f) - 0.5f) * 2.0f
                + (float)i *((GAME_LEVEL_END_X - GAME_LEVEL_START_X) - 16.0f));
        m_players[i]->setOnGround(true);
    }

    // Couple of more trees above the players, just to fake the eyes.
    for (int i = 0; i < 1; ++i) {
        GameObject *tree = placeTree();
        tree->pos().setZ(tree->pos().z() + 1.0f + (rand() & 255) / 512.0f);
    }

    // Create the indicator arrow.
    m_indicatorArrow = m_objManager->addObject(
        new GameStaticObject(this,
                             getTextureManager()->getTexture(":/arrow_down.png")));
    m_indicatorArrow->setRunEnabled(false);
    m_indicatorArrow->setDepthEnabled(false);
    m_indicatorArrow->setr(0.7f);

    // Create the indicator circle indicating the active player.
    m_activePlayerIndicatorCircle = m_objManager->addObject(
        new GameStaticObject(this,
                             getTextureManager()->getTexture(":/indicator.png")));
    m_activePlayerIndicatorCircle->setRunEnabled(false);
    m_activePlayerIndicatorCircle->setDepthEnabled(false);
    m_activePlayerIndicatorCircle->setr(2.0f);
}


/*!
  Sets the current menu as \a newMenu.
*/
void GameInstance::setCurrentMenu(GameMenu *newMenu)
{
    delete m_currentMenu;
    m_currentMenu = newMenu;
}


/*!
*/
int GameInstance::run(float frameTime, int playerTurn)
{
    if (m_currentMenu) {
        if (!m_currentMenu->run(frameTime)) {
            delete m_currentMenu;
            m_currentMenu = 0;
        }
    } else {
        if (m_toMainMenu) {
            m_currentMenu = new GameMenu(this, 0, 1,2);
            m_toMainMenu = false;
        }
    }

    if (m_objManager && m_level && !m_currentMenu) {
        m_objManager->run(frameTime);

        // Player 2 has won
        if (m_players[0]->isDying() && !m_currentMenu)
            m_currentMenu = new GameMenu(this, 7,1,2);

        // Player 1 has won
        if (m_players[1]->isDying() && !m_currentMenu)
            m_currentMenu = new GameMenu(this, 6,1,2);

        m_fireTargetVolume = 0.0f;
        m_players[0]->setEnemyPos(m_players[1]->pos());
        m_players[1]->setEnemyPos(m_players[0]->pos());
        m_arrowAngle += frameTime * 40.0f;

        if (m_players[playerTurn]->isAiming())
            m_showHelpTimer = 0.0f;

        // If help is not enabled, enable it if enough time has passed.
        if (!m_tip) {
            m_showHelpTimer += frameTime;

            if (m_showHelpTimer > m_showHelpTime) {
                recreateHelp();
            }
        }

        m_helpAngle += frameTime * 10.0f;

        int enemy(0);

        if (playerTurn == 0)
            enemy = 1;

        QVector3D tpos;

        if (m_tip) {
            tpos = m_players[playerTurn]->pos() - QVector3D(0.0, -2.5f, 0.0f);
            m_tip->pos() += (tpos-m_tip->pos()) * frameTime * 15.0f;
        }

        if (m_indicatorCircle) {
            m_indicatorCircle->setUpVector(cosf(m_helpAngle),
                                           sinf(m_helpAngle));

            if (m_players[playerTurn]->isAiming()) {
                if (m_tip) {
                    m_tip->die();
                    m_tip->setRunEnabled(true);
                    m_tip = 0;
                }

                m_indicatorCircle->setAlpha(1.0f);
                QVector3D delta = m_players[playerTurn]->aimPos()
                                  - m_players[playerTurn]->pos();
                float power = delta.length() / 14.0f;

                if (power>1.0f)
                    power = 1.0f;

                m_indicatorCircle->setLightness(power);
                m_indicatorCircle->setr(1.0f);
                m_indicatorCircle->pos() = m_players[playerTurn]->aimPos();
            }
            else {
                if (m_players[playerTurn]->pos().x()
                        < m_players[enemy]->pos().x())
                {
                    tpos = QVector3D(-8.0f, -8.0f, 0.0f);
                }
                else {
                    tpos = QVector3D(8.0f, -8.0f, 0.0f);
                }

                float iphase = m_helpAngle / 4.0f;
                iphase = ((iphase-floor(iphase)) * 2.0f) - 1.0f;

                if (iphase >- 0.1f) {
                    m_helpPullState += (1.0f - m_helpPullState)
                                       * frameTime * 30.0f;
                }
                else {
                    m_helpPullState += (0.0f - m_helpPullState)
                                       * frameTime * 30.0f;
                }

                if (iphase < 0.0f)
                    iphase = 0.0f;

                m_indicatorCircle->setLightness(iphase);
                tpos = m_players[ playerTurn ]->pos() + tpos * (iphase);

                m_indicatorCircle->setAlpha(0.5f + m_helpPullState * 0.5f);
                m_indicatorCircle->setr(2.0f - m_helpPullState);
                m_indicatorCircle->pos() +=
                        (tpos - m_indicatorCircle->pos()) * frameTime * 30.0f;
            }
        }

        if (!m_tip && !m_indicatorCircle) {
            m_activePlayerIndicatorCircle->setAlpha(
                        m_activePlayerIndicatorCircle->getAlpha()
                        + (0.4f - m_activePlayerIndicatorCircle->getAlpha())
                        * frameTime * 10.0f);
        }
        else {
            m_activePlayerIndicatorCircle->setAlpha(
                        m_activePlayerIndicatorCircle->getAlpha()
                        + (0.0f - m_activePlayerIndicatorCircle->getAlpha())
                        * frameTime * 10.0f);
        }

        m_activePlayerIndicatorCircle->setLightness(0.0f);
        m_activePlayerIndicatorCircle->setUpVector(cosf(m_arrowAngle*0.2f),
                                                   sinf(m_arrowAngle*0.2f));
        m_activePlayerIndicatorCircle->pos() += QVector3D(
            (m_players[playerTurn]->pos().x()
                - m_activePlayerIndicatorCircle->pos().x())
                    * frameTime * 30.0f,
            (m_players[playerTurn]->pos().y()
                - m_activePlayerIndicatorCircle->pos().y())
                    * frameTime * 30.0f,
            (m_players[playerTurn]->pos().z()
                - m_activePlayerIndicatorCircle->pos().z())
                    * frameTime * 30.0f);

        if (!m_players[playerTurn]->isAiming()) {
            m_indicatorArrow->setAlpha(
                m_indicatorArrow->getAlpha()
                - m_indicatorArrow->getAlpha() * frameTime * 20.0f);
            m_indicatorArrow->pos() += QVector3D(
                    ((m_players[playerTurn]->pos().x()
                      - m_indicatorArrow->pos().x()) * frameTime * 30.0f),
                    (((m_players[playerTurn]->pos().y() + 3.0f
                       + sinf(m_arrowAngle) *0.7f))
                      - m_indicatorArrow->pos().y()) * frameTime * 30.0f,
                     m_players[playerTurn]->pos().z());
        }
        else {
            m_indicatorArrow->setAlpha(
                m_indicatorArrow->getAlpha()
                + (1.0f - m_indicatorArrow->getAlpha()) * frameTime * 20.0f);
        }
    }

    // Run the level and particles.
    if (m_level)
        m_level->run(frameTime);

    if (m_particleEngine)
        m_particleEngine->run(frameTime);

    // Change fire sample's volume towards its target volume.
    m_fireVolume += (m_fireTargetVolume-m_fireVolume) * frameTime * 5.0f;

    // If fire sample should be active but is not, create'n'start it.
    if (m_fireVolume > 0.05f && !m_fireBurn) {
        m_fireBurn = m_sampleFire->playWithMixer(*m_mixer);
        m_fireBurn->setLoopCount(-1);
    }

    // If fire sample should be disabled but is not, destroy it.
    if (m_fireVolume <= 0.05f && m_fireBurn) {
        m_fireBurn->stop();
        m_fireBurn = 0;
    }

    // Set volume for fire burning sample.
    if (m_fireBurn) {
        m_fireBurn->setLeftVolume(m_fireVolume);
        m_fireBurn->setRightVolume(m_fireVolume);
    }

    return 0;
}


/*!
*/
void GameInstance::setCamera(QMatrix4x4 &camera)
{
    // The view matrix is an inverted camera matrix. With
    // orthogonal/otrhonormal matrices, the inversion can be done with
    // a simple swith of rows/columns.
    //
    // Since QMatrices have different order of these automatically, we can
    // just do the view matrix setting by copying the values.
    //
    for (int f = 0; f < 4; f++) {
        for (int g = 0; g < 4; g++) {
            m_cameraMatrix[f * 4 + g] = camera.constData()[f * 4 + g];
        }
    }
}


/*!
  (Re)sets the projection according to \a width and \a height.
*/
void GameInstance::setSize(int width, int height)
{
    QMatrix4x4 projection;
    projection.setToIdentity();
    projection.perspective(45.0f, (float)width / (float)height, 0.1f, 100.0f);

    for (int f = 0; f < 4; f++) {
        for (int g = 0; g < 4; g++) {
            m_projectionMatrix[f * 4 + g] = projection.constData()[g * 4 + f];
        }
    }
}


/*!
  Transforms a matrix with the current camera matrix.
*/
void GameInstance::cameraTransform(float *m, bool transformPosition)
{
    float temp[16];
    memset(temp, 0, sizeof(float) * 16);
    temp[15] = 1.0f;

    // Subtract camera's position.
    if (transformPosition) {
        m[3] -= m_cameraMatrix[12];
        m[7] -= m_cameraMatrix[13];
        m[11] -= m_cameraMatrix[14];
    }

    // And do 3x3 matrix multiply.
    for (int f = 0; f < 4; f++) {
        for (int g = 0; g < 3; g++) {
            temp[g * 4 + f] =
                m[f + 0 * 4] * m_cameraMatrix[g + 0 * 4] +
                m[f + 1 * 4] * m_cameraMatrix[g + 1 * 4] +
                m[f + 2 * 4] * m_cameraMatrix[g + 2 * 4];
        }
    }

    if (!transformPosition) {
        // Reset the position.
        temp[3] = m[3];
        temp[7] = m[7];
        temp[11] = m[11];
    }

    // Copy transformed back into the target.
    memcpy(m, temp, sizeof(float) * 16);
}


/*!
*/
GLint GameInstance::loadGLTexture(QString filename)
{
    QImage image = QImage(filename);
    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
    GLuint* pTexData = new GLuint[image.width() * image.height()];
    GLuint* sdata = (GLuint*)image.bits();
    GLuint* tdata = pTexData;

    for (int y = 0; y < image.height(); y++) {
        for (int x = 0; x < image.width(); x++) {
            *tdata = ((*sdata&255) << 16) | (((*sdata>>8)&255) << 8)
                    | (((*sdata>>16)&255) << 0) | (((*sdata>>24)&255) << 24);
            sdata++;
            tdata++;
        }
    }

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image.width(), image.height(), 0,
                 GL_RGBA, GL_UNSIGNED_BYTE, pTexData);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    delete [] pTexData;
    return texture;
}


/*!
*/
void GameInstance::renderParticleTypes()
{
    glDepthMask(GL_FALSE);
    glDisable(GL_DEPTH_TEST);

    // Different smoke type particles. All with smoke program and texture.
    glUseProgram(m_smallSmokeParticle->m_program);
    glBindTexture(GL_TEXTURE_2D, m_smallSmokeParticle->m_textureId);

    m_particleEngine->render(m_smallSmokeParticle,
                             m_smallSmokeParticle->m_program);

    // Don't render all of the particle types with Maemo 5.
#ifndef Q_WS_MAEMO_5
    m_particleEngine->render(m_smokeParticle,
                             m_smallSmokeParticle->m_program);
    m_particleEngine->render(m_dustParticle,
                             m_smallSmokeParticle->m_program);

    // Fire
    glUseProgram(m_basicFireParticle->m_program);
    glBindTexture(GL_TEXTURE_2D, m_basicFireParticle->m_textureId);
    m_particleEngine->render(m_basicFireParticle,
                             m_basicFireParticle->m_program);
#endif

    // The explosion flares without depth testing.
    glDisable(GL_DEPTH_TEST);
    glUseProgram(m_explosionFlareParticle->m_program);
    glBindTexture(GL_TEXTURE_2D, m_explosionFlareParticle->m_textureId);
    m_particleEngine->render(m_explosionFlareParticle,
                             m_explosionFlareParticle->m_program);
}


/*!
*/
void GameInstance::killHelp()
{
    m_showHelpTimer = 0.0f;

    if (m_tip) {
        m_tip->setRunEnabled(true);
        m_tip->die();
    }

    m_tip = 0;

    if (m_indicatorCircle) {
        m_indicatorCircle->setRunEnabled(true);
        m_indicatorCircle->die();
    }

    m_indicatorCircle = 0;
}


/*!
  Constructs the particles used in the game.
*/
void GameInstance::initParticles()
{
    m_basicFireParticle =  new ParticleType(
                m_particleEngine->normalProgram(),
                getTextureManager()->getTexture(":/fire_particle.png"));

    m_basicFireParticle->m_lifeTime = 500;
    m_basicFireParticle->m_lifeTimeRandom = 1000;
    m_basicFireParticle->m_fraction = 40000;
    m_basicFireParticle->m_gravity = -200  <<  12;
    m_basicFireParticle->m_size = (70  <<  12);
    m_basicFireParticle->m_sizeRandom = (40 << 12);
    m_basicFireParticle->m_angleRandom = 512 << 12;
    m_basicFireParticle->m_sizeInc = -300 << 12;
    m_basicFireParticle->m_sizeIncRandom = 200 << 12;
    m_basicFireParticle->m_angle = -200 << 12;
    m_basicFireParticle->m_angleInc = 400 << 12;
    m_basicFireParticle->m_angleInc = -50 << 12;
    m_basicFireParticle->m_angleIncRandom = 100 << 12;
    m_basicFireParticle->setVisibility(0.08f, 0.4f, 1.0f);
    m_basicFireParticle->m_additiveParticle = false;

    m_smokeParticle = new ParticleType(
                m_particleEngine->smokeProgram(),
                getTextureManager()->getTexture(":/smoke1.png"));

    m_smokeParticle->m_lifeTime = 2000;
    m_smokeParticle->m_lifeTimeRandom = 800;
    m_smokeParticle->m_fraction = 5 << 12;
    m_smokeParticle->m_gravity = -120 << 12;
    m_smokeParticle->m_size = (20 << 12);
    m_smokeParticle->m_sizeRandom = (10 << 12);
    m_smokeParticle->m_angleRandom = 512 << 12;
    m_smokeParticle->m_sizeInc = 330 << 12;//-120 << 12;
    m_smokeParticle->m_sizeIncRandom = (130 << 12);
    m_smokeParticle->m_angleInc = -90 << 12;
    m_smokeParticle->m_angleIncRandom = 180 << 12;
    m_smokeParticle->m_additiveParticle = false;
    m_smokeParticle->setVisibility(0.1f, 0.1f, 1.0f);
    m_smokeParticle->setColors(0.6f, 0.6f, 0.6f, 0.3f, 0.3f, 0.3f);

    m_smallSmokeParticle = new ParticleType(
                m_particleEngine->smokeProgram(),
                getTextureManager()->getTexture(":/smoke1.png"));

    m_smallSmokeParticle->m_lifeTime = 500;
    m_smallSmokeParticle->m_lifeTimeRandom = 1000;
    m_smallSmokeParticle->m_fraction = 0;
    m_smallSmokeParticle->m_gravity = -20 << 12;
    m_smallSmokeParticle->m_size = (22 << 12);
    m_smallSmokeParticle->m_sizeRandom = (12 << 12);
    m_smallSmokeParticle->m_angleRandom = 512 << 12;
    m_smallSmokeParticle->m_sizeInc = 0 << 12;
    m_smallSmokeParticle->m_sizeIncRandom = (20 << 12);
    m_smallSmokeParticle->m_angleInc = -40 << 12;
    m_smallSmokeParticle->m_angleIncRandom = 80 << 12;
    m_smallSmokeParticle->m_additiveParticle = false;
    m_smallSmokeParticle->setVisibility(0.05f, 0.1f, 1.0f);
    m_smallSmokeParticle->setColors(0.6f, 0.6f, 0.6f, 0.3f, 0.3f, 0.3f);
    m_smallSmokeParticle->m_turbulenceMul = 8000;

    m_dustParticle = new ParticleType(
                m_particleEngine->smokeProgram(),
                getTextureManager()->getTexture(":/smoke1.png"));

    m_dustParticle->m_lifeTime = 4000;
    m_dustParticle->m_lifeTimeRandom = 2000;
    m_dustParticle->m_fraction = 90 << 12;
    m_dustParticle->m_gravity = 20 << 12;
    m_dustParticle->m_size = (50 << 12);
    m_dustParticle->m_sizeRandom = (40 << 12);
    m_dustParticle->m_angleRandom = 512 << 12;
    m_dustParticle->m_sizeInc = 40 << 12;
    m_dustParticle->m_sizeIncRandom = (80 << 12);
    m_dustParticle->m_angleInc = -40 << 12;
    m_dustParticle->m_angleIncRandom = 80 << 12;
    m_dustParticle->m_turbulenceMul = 18000;
    m_dustParticle->m_additiveParticle = false;
    m_dustParticle->setVisibility(0.05f, 0.5f, 0.5f);
    m_dustParticle->setColors(0.2f, 0.3f, 0.15f, 0.1f, 0.1f, 0.1f);

    m_explosionFlareParticle = new ParticleType(
                m_particleEngine->normalProgram(),
                getTextureManager()->getTexture(":/explo_flare1.png"));

    m_explosionFlareParticle->m_additiveParticle = true;
    m_explosionFlareParticle->m_lifeTime = 500;
    m_explosionFlareParticle->m_lifeTimeRandom = 200;
    m_explosionFlareParticle->m_fraction = 40000;
    m_explosionFlareParticle->m_gravity = 0;
    m_explosionFlareParticle->m_size = (4 << 12);
    m_explosionFlareParticle->m_sizeRandom = 0;
    m_explosionFlareParticle->m_angleRandom = 512 << 12;
    m_explosionFlareParticle->m_sizeInc = 12000 << 12;
    m_explosionFlareParticle->m_sizeIncInc = -16000;
    m_explosionFlareParticle->m_sizeIncRandom = 0;
    m_explosionFlareParticle->m_angleInc = -40 << 12;
    m_explosionFlareParticle->m_angleIncRandom = 80 << 12;
    m_explosionFlareParticle->m_fadeOutTimeSecs = 1.0f/500.0f;
    m_explosionFlareParticle->setVisibility(0.0f, 0.05f, 1.0f);
}


/*!
  Initializes the samples.
*/
void GameInstance::initSamples()
{
    m_sampleShoot = GE::AudioBuffer::loadWav(":/cannon.wav");
    m_sampleExplosion = GE::AudioBuffer::loadWav(":/explosion.wav");
    m_sampleWhistle = GE::AudioBuffer::loadWav(":/whistle.wav");
    m_sampleFire = GE::AudioBuffer::loadWav(":/fire.wav");
    m_sampleHurt = GE::AudioBuffer::loadWav(":/auts.wav");
    m_sampleBackground = GE::AudioBuffer::loadWav(":/bg_ambient.wav");
}


/*!
*/
void GameInstance::recreateHelp()
{
    killHelp();
    m_showHelpTimer = 0.0f;
    m_showHelpTime = 4.0f; // Increase the time we show the help next time

    // Create the tip box.
    m_tip = m_objManager->addObject(
        new GameUIObject(this, getTextureManager()->getTexture(":/tip.png")));
    m_tip->setRunEnabled(false);
    m_tip->setDepthEnabled(false);
    m_tip->setr(2.7f);
    m_tip->setAspect(0.5f);

    m_indicatorCircle = m_objManager->addObject(
        new GameUIObject(this, getTextureManager()->getTexture(":/indicator.png")));
    m_indicatorCircle->setRunEnabled(false);
    m_indicatorCircle->setDepthEnabled(false);
    m_indicatorCircle->setr(2.0f);
}


/*!
  Finds a good place for a tree and creates it there. Returns the created
  GameObject instance.
*/
GameObject *GameInstance::placeTree()
{
    GameObject *tree =
        m_objManager->addObject(
            new GameTree(this, getTextureManager()->getTexture(":/tree1.png")));

    float x;
    QVector3D vec;
    int safeCount(0);

    while (1) {
        x = GAME_LEVEL_START_X
            + (GAME_LEVEL_END_X - GAME_LEVEL_START_X)
            * (float)(rand() & 1023) / 1023.0f;
        float height = m_level->getHeightAndNormalAt(x, &vec);

        if (vec.y() > 0.75f && height > -0.3f)
            break;

        safeCount++;

        if (safeCount > 100)
            break;
    }

    tree->pos().setX(x);
    tree->pos().setY(5.0f);
    tree->pos().setZ(-0.5f + (float)(rand() & 255) / 255.0f * 0.3f);
    return tree;
}

