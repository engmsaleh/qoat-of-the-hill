/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "GamePlayer.h"

#include <math.h>

#include "audiobuffer.h"
#include "audiobufferplayinstance.h"

#include "GameInstance.h"
#include "GameLevel.h"
#include "ParticleEngine.h"
#include "TextureManager.h"

#define WHISTLE_SPEEDM 3.0f
#define PLAYER_SCALE 2.0f


/*!
  \class GameAmmunition
  \brief Represents an ammunition.
*/


/*!
  Constructor.
*/
GameAmmunition::GameAmmunition(GameInstance *gameInstance)
    : GameObject(gameInstance),
      m_whistleInstance(0)
{
    setTextureID(gameInstance->getTextureManager()->getTexture(":/ammo1.png"));
    m_r = 0.1f;
    m_powerResponse = 0.05f;

    if (gameInstance->audioEnabled()) {
        m_whistleInstance =
                gameInstance->m_sampleWhistle->playWithMixer(
                    *gameInstance->getMixer());
    }

    if (m_whistleInstance)
        m_whistleInstance->setLoopCount(-1);
}


/*!
  Destructor.
*/
GameAmmunition::~GameAmmunition()
{
    if (m_whistleInstance) {
        m_whistleInstance->stop();
        m_whistleInstance = 0;
    }
}


/*!
*/
void GameAmmunition::run(float frameTime)
{
    QVector3D expos = m_pos;
    GameObject::run(frameTime);
    ParticleEngine *particleEngine = m_gameInstance->getParticleEngine();

    // Emit particles among the line our position is moved according dir.
    QVector3D ofs = m_pos - expos;
    float len = ofs.length();
    int steps = (int)(len / 0.05f) + 1;
    QVector3D d;
    ofs /= (float)steps;

    while (steps > 0) {
        QVector3D p = expos + ofs*(float)steps;
        particleEngine->emitParticles(
            1, m_gameInstance->m_smallSmokeParticle, p, 0.025f, d, 0.5f);
        steps--;
    }

    // Play'n'control whistling sound as we go.
    if (m_whistleInstance) {
        float nspeed = (WHISTLE_SPEEDM - m_dir.y() * 0.025f);


        nspeed = WHISTLE_SPEEDM / nspeed;

        if (nspeed<0.1f) nspeed = 0.1f;
        if (nspeed>10.0f) nspeed = 10.0f;

        m_whistleInstance->setSpeed(nspeed);
        nspeed = fabsf(m_dir.length() * 0.008f);

        if (nspeed < 0.1f)
            nspeed = 0.1f;

        if (nspeed > 1.0f)
            nspeed = 1.0f;

        m_whistleInstance->setRightVolume(nspeed);
        m_whistleInstance->setLeftVolume(nspeed);
    }
}


/*!
*/
void GameAmmunition::hit(GameObject *hitObj,
                         QVector3D &collisionPos,
                         QVector3D &collisionNormal )
{
    Q_UNUSED(hitObj);
    Q_UNUSED(collisionPos);

    ParticleEngine *particleEngine = m_gameInstance->getParticleEngine();

    // Stop whistling
    if (m_whistleInstance) {
        m_whistleInstance->stop();
        m_whistleInstance = 0;
    }

    // Play explosion, use minor speed variation
    if (m_gameInstance->audioEnabled()) {
        m_gameInstance->m_sampleExplosion->playWithMixer(
                    *m_gameInstance->getMixer())->setSpeed(
                        0.8f + (rand() & 255) / 255.0f * 0.2f);
    }

    // Emit different types of particles
    QVector3D d = collisionNormal * 20.0f;
    particleEngine->emitParticles(
                20, m_gameInstance->m_basicFireParticle, m_pos,
                m_r, d, 4.0f);
    particleEngine->emitParticles(
                20, m_gameInstance->m_dustParticle, m_pos,
                m_r, d, 80.0f);
    d = QVector3D(0.0f, 0.0f, 0.0f);
    particleEngine->emitParticles(
                20, m_gameInstance->m_basicFireParticle, m_pos,
                m_r / 2.0f, d, 10.0f);
    particleEngine->emitParticles(
                2, m_gameInstance->m_explosionFlareParticle, m_pos,
                m_r / 2.0f, d, 1.0f);
    particleEngine->emitParticles(
                20, m_gameInstance->m_smokeParticle, m_pos,
                m_r / 2.0f, d, 10.0f);

    // Modify the level accordint the explosion.
    m_gameInstance->getLevel()->explosion(m_pos.x(), m_pos.y(), 2.8f);

    // Simulate "blas wave" by adding push-force to game objects.
    m_gameInstance->getObjectManager()->pushObjects(m_pos, 5.0f, 100.0f);

    // Add few burning pieces flying away from the blast site.
    int bp = 4 + (rand() & 3);

    while (bp > 0) {
        GameObject *o = m_gameInstance->getObjectManager()->addObject(
                    new GameBurningPiece(m_gameInstance));
        o->pos() = m_pos;
        o->dir().setX(((float)(rand() & 255) - 128.0f) / 3.0f);
        o->dir().setY(((float)(rand() & 255) - 128.0f) / 3.0f);
        bp--;
    }

    // And btw, we are dead! :)
    die();
}



/*!
  \class GameBurningPiece
  \brief A small burning piece. Explosions create these.
*/


/*!
  Constructor.
*/
GameBurningPiece::GameBurningPiece(GameInstance *gameInstance)
    : GameObject(gameInstance),
      m_lifeTime(0.25f + (float)(rand() & 255) / 128.0f),
      m_burnParticleCounter((float)(rand() & 255) / 255.0f)
{
    setTextureID(gameInstance->getTextureManager()->getTexture(":/ammo1.png"));
    m_r = 0.05f + (float)(rand() & 255) / 10000.0f;
    m_gravity = 200.0f;
    m_airFraction = 2.0f;
}


/*!
  Destructor.
*/
GameBurningPiece::~GameBurningPiece()
{
}


/*!
*/
void GameBurningPiece::run(float frameTime)
{
    GameObject::run(frameTime);
    ParticleEngine *particleEngine = m_gameInstance->getParticleEngine();
    m_gameInstance->markFireBurning();
    QVector3D d = QVector3D(0, 0, 0);
    m_burnParticleCounter += frameTime * 30.0f;
    int particleCount((int)m_burnParticleCounter);

    // Emit basic fire and smoke particles.
    if (particleCount > 0) {
        particleEngine->emitParticles(
            particleCount * 2, m_gameInstance->m_basicFireParticle,
            m_pos, m_r / 4.0f, d, 4.0f);
        particleEngine->emitParticles(
            particleCount, m_gameInstance->m_smokeParticle,
            m_pos, m_r / 2.0f, d, 3.0f);
        m_burnParticleCounter -= (float)particleCount;
    }

    m_lifeTime -= frameTime;

    // We are dead => Emit fire and smoke particles
    if (m_dead == false && m_lifeTime <= 0.0f) {
        particleEngine->emitParticles(
            20, m_gameInstance->m_basicFireParticle,
            m_pos, m_r / 2.0f, d, 10.0f);
        particleEngine->emitParticles(
            10, m_gameInstance->m_smokeParticle,
            m_pos, m_r / 2.0f, d, 1.0f);
        die();
    }
}


/*!
*/
void GameBurningPiece::hit(GameObject *hitObj,
                           QVector3D &collisionPos,
                           QVector3D &collisionNormal)
{
    Q_UNUSED(hitObj);
    Q_UNUSED(collisionPos);
    Q_UNUSED(collisionNormal);

    // About 50% change to die when hitted (bouncing)
    if ((rand() & 255) < 128)
        die();
}



/*!
  \class GamePlayer
  \brief Represents the qoat of the player.
*/


/*!
  Constructor.
*/
GamePlayer::GamePlayer(GameInstance *gameInstance)
    : GameObject(gameInstance),
      m_health(1.0f),
      m_aiming(false),
      m_hit(0.0f),
      m_breath((float)(rand() & 255) / 64.0f),
      m_head(0),
      m_gun(0),
      m_previousShootArrow(0)
{
    setTextureID(gameInstance->getTextureManager()->getTexture(":/player.png"));

    m_r = 0.4f * PLAYER_SCALE;
    m_upvectorTarget[0] = 0.0f;
    m_upvectorTarget[1] = 1.0f;
    m_previousShootVector = QVector3D(0.0f, 0.0f, 0.0f);

    setAirFraction(3.0f);
    setGravity(150.0f);

    m_gun = m_gameInstance->getObjectManager()->addObject(
                new GameStaticObject(m_gameInstance,
                                      m_gameInstance
                                      ->getTextureManager()
                                      ->getTexture(":/gun.png")));
    m_gun->setRunEnabled(false);
    m_gun->setr(0.4f * PLAYER_SCALE);
    m_gun->setAspect(0.8f);
    m_gun->setCenterSprite(true);
}


/*!
  Destructor.
*/
GamePlayer::~GamePlayer()
{
}


/*!
*/
void GamePlayer::createAssets()
{
    m_head = m_gameInstance->getObjectManager()->addObject(
                new GameStaticObject(m_gameInstance,
                                      m_gameInstance->getTextureManager()
                                      ->getTexture(":/player_head.png")));
    m_head->setRunEnabled(false);
    m_head->setr(0.3f * PLAYER_SCALE);
}


/*!
*/
void GamePlayer::pushForce(QVector3D &pos, float r, float power)
{
    QVector3D temp = m_pos - pos;
    float dis = temp.length();

    if (dis < r) {
        float power = (r - dis) / r;

        // Play "auts" sample according power affecting us
        if (m_gameInstance->audioEnabled()) {
            GE::AudioBufferPlayInstance *i =
                    m_gameInstance->m_sampleHurt->playWithMixer(
                        *m_gameInstance->getMixer());
            power = power * 2.0f + 0.1f;

            if (power > 1.0f)
                power = 1.0f;

            i->setLeftVolume(power);
            i->setRightVolume(power);
        }
    }

    // Apply rest of the push normally
    GameObject::pushForce(pos, r, power);
}


/*!
*/
void GamePlayer::hit(GameObject *hitObj,
                     QVector3D &collisionPos,
                     QVector3D &collisionNormal)
{
    Q_UNUSED(hitObj);
    Q_UNUSED(collisionPos);

    float power = m_dir.length();
    m_hit += 0.1f + power / 200.0f;
    ParticleEngine *particleEngine = m_gameInstance->getParticleEngine();
    QVector3D d = collisionNormal * 21.0f;
    particleEngine->emitParticles(
                10, m_gameInstance->m_dustParticle, m_pos, m_r, d, 20.0f);
    m_upvectorTarget[0] = -m_groundNormal.x();
    m_upvectorTarget[1] = m_groundNormal.y();
}


/*!
*/
void GamePlayer::run(float frameTime)
{
    GameObject::run(frameTime);

    // Uncomment the following to enable player dying.
    /*
    if (m_health <= 0.0f) {
        if (!m_gun->isRunEnabled()) {
            m_gun->setRunEnabled(true);
            m_head->setRunEnabled(true);
            ((GameStaticObject*)m_gun)->m_angleInc =
                ((rand() & 255) / 255.0f - 0.5f) * 50.0f;
            ((GameStaticObject*)m_head)->m_angleInc =
                ((rand() & 255) / 255.0f - 0.5f) * 50.0f;
        }

        return;
    }
    */

    m_breath += frameTime * 30.0f;
    m_hit -= m_hit * frameTime * 20.0f;
    m_aspect = 1.0f - m_hit * 1.6f + sinf(m_breath) * 0.013f;

    if (m_onGround) {
        m_upvectorTarget[0] = 0.0f - m_groundNormal.x();
        m_upvectorTarget[1] = 1.0f + m_groundNormal.y();
        m_upvector[0] += (m_upvectorTarget[0] - m_upvector[0]) * frameTime * 2.0f;
        m_upvector[1] += (m_upvectorTarget[1] - m_upvector[1]) * frameTime * 2.0f;
    }
    else {
        m_upvectorTarget[0] = m_dir.x() * 20.0f;
        m_upvectorTarget[1] = 1.0f;
        m_upvector[0] += (m_upvectorTarget[0] - m_upvector[0]) * frameTime * 10.0f;
        m_upvector[1] += (m_upvectorTarget[1] - m_upvector[1]) * frameTime * 10.0f;
    }

    float l = sqrtf(m_upvector[0] * m_upvector[0]
                    + m_upvector[1] * m_upvector[1]) + 0.00001f;
    m_upvector[0] /= l;
    m_upvector[1] /= l;
    float flipmul = 1.0f;

    if (m_flipX)
        flipmul = -1.0f;

    QVector3D temp;

    if (m_head) {
        m_head->pos() =
                QVector3D(m_pos.x() - m_upvector[0] * m_r * 0.33f
                          - m_upvector[1] * m_r * 0.9f * flipmul,
                          m_pos.y() + m_upvector[1] * m_r * 0.33f
                          - m_upvector[0] * m_r * 0.9f * flipmul,
                          m_pos.z() + 0.0f);
        m_head->setFlipX(m_flipX);
        temp = m_enemyPos - m_head->pos();
        temp.normalize();
        temp += QVector3D(m_upvector[0], m_upvector[1], 0.0f);
        temp.normalize();
        m_head->setUpVector(-temp.y() * flipmul, -temp.x() * flipmul);
    }

    if (!m_aiming) {
        temp = QVector3D(m_pos.x() + flipmul, m_pos.y() + 0.5f, 0.0f) - m_aimPos;
        temp *= frameTime * 10.0f;
        m_aimPos += temp;
    }

    // Manipulate gun child object according aiming position
    if (m_gun) {
        m_gun->setFlipX(m_flipX);
        temp = m_aimPos-m_pos;
        temp.normalize();
        m_gun->setUpVector(-temp.y() * flipmul, -temp.x() * flipmul);
        m_gun->pos() += QVector3D(
            ((m_pos.x() - m_upvector[0] * m_r * 0.55f * PLAYER_SCALE - temp.x() * 0.5f)
             - m_gun->pos().x()) * frameTime * 40.0f,
            ((m_pos.y() + m_upvector[1] * m_r * 0.55f * PLAYER_SCALE - temp.y() * 0.5f)
             - m_gun->pos().y()) * frameTime * 40.0f,
            0.0f);
    }

    // Position the arrow displaying previous shoot if any.
    if (m_previousShootArrow) {
        m_previousShootArrow->pos() +=
                (m_pos + m_previousShootVector - m_previousShootArrow->pos())
                * frameTime * 10.0f;

        float *uv = m_previousShootArrow->getUpVector();
        temp = m_previousShootVector;
        temp.normalize();
        uv[0] += (temp.x() - uv[0]) * frameTime * 10.0f;
        uv[1] += (-temp.y() - uv[1]) * frameTime * 10.0f;
    }
}


/*!
*/
void GamePlayer::setShootVector(QVector3D shootVector)
{
    if (!m_previousShootArrow) {
        m_previousShootArrow = m_gameInstance->getObjectManager()->addObject(
                    new GameStaticObject(m_gameInstance,
                                          m_gameInstance
                                          ->getTextureManager()
                                          ->getTexture(":/arrow_down.png")));

        m_previousShootArrow->setRunEnabled(false);
        m_previousShootArrow->setDepthEnabled(false);
        m_previousShootArrow->setr(0.5f);
        m_previousShootArrow->pos() = pos();
    }

    m_previousShootVector = shootVector;
}



/*!
  \class GameTree
  \brief Represents a tree or a plant.
*/


/*!
  Constructor.
*/
GameTree::GameTree(GameInstance *gameInstance, unsigned int texture)
    : GameObject(gameInstance),
      m_angle(((float)(rand() & 255) / 255.0f - 0.5f) * 0.2f),
      m_angleInc(0.0f)

{
    setTextureID(texture);
    m_r = 0.9f + (float)(rand() & 255) / 512.0f;
    setAspect(1.2f);
    m_centerSprite = false;
    m_moveOnGround = false;

    if ((rand() & 255) < 128)
        m_flipX = true;

    m_onGround = true;
}


/*!
  Destructor.
*/
GameTree::~GameTree()
{
}


/*!
*/
void GameTree::pushForce(QVector3D &pos, float r, float power)
{
    if (m_powerResponse <= 0.0001f)
        return;

    QVector3D temp = m_pos - pos;
    float d = temp.length();
    temp /= d;
    d = (r - d) / r * power * m_powerResponse;

    if (d < 0.0f)
        return;

    temp *= d;

    if (d > 75.0f) {
        die();

        for (int f = 0; f < 4; f++) {
            GameStaticObject *dobj = (GameStaticObject*)
                m_gameInstance->getObjectManager()->addObject(
                    new GameStaticObject(m_gameInstance, m_gameInstance
                                         ->getTextureManager()
                                         ->getTexture(":/treepart1.png")));

            dobj->pos() = QVector3D(
                m_pos.x() + (((float)(rand() & 255) - 128.0f) / 128.0f) * m_r/2.0f,
                m_pos.y() + m_r * 0.5 + (float)f / 3.0f * m_r,
                m_pos.z());

            dobj->setAirFraction(10.0f);
            dobj->setGravity(300.0f);

            if ((rand() & 255) < 16)
                dobj->burn();

            dobj->m_angle = 0.0f;
            dobj->m_angleInc = 50.0f * ((float)(rand() & 255) / 255.0f - 0.5f);
            dobj->setr(m_r / 2.0f);
        }
    }
    else {
        m_angleInc -= temp.x() / 80.0f;
    }
}


/*!
*/
void GameTree::run(float frameTime)
{
    // Override die animation
    if (m_dead)
        m_dieAnimation = 2.0f;

    if (m_onGround) {
        m_angleInc -= m_angle * frameTime * 15.0f;
        m_angleInc -= m_angleInc * frameTime * 3.0f;

        if ((rand() & 255) < 2) {
            m_angleInc += (float)((rand() & 255) - 128.0f) / 2048.0f;
        }
    }

    m_angle += m_angleInc * frameTime * 10.0f;
    m_upvector[0] = sinf(m_angle);
    m_upvector[1] = cosf(m_angle);
    GameObject::run(frameTime);
}



/*!
  \class GameStaticObject
  \brief -
*/


/*!
  Constructor.
*/
GameStaticObject::GameStaticObject(GameInstance *gameInstance,
                                   unsigned int texture)
    : GameObject(gameInstance),
      m_angle(0.0f),
      m_angleInc(0.0f),
      m_burnCounter(-1.0f),
      m_burnParticleCounter(0.0f)
{
    setTextureID(texture);
    m_r = 1.0f;
}


/*!
  Destructor.
*/
GameStaticObject::~GameStaticObject()
{
}


/*!
*/
void GameStaticObject::burn()
{
    m_burnCounter = ((rand() & 255) / 255.0f) * 2.0f;
    m_burnParticleCounter = (float)(rand() & 255) / 255.0f;
}


/*!
*/
void GameStaticObject::run(float frameTime)
{
    if (!m_run)
        return;

    m_upvector[0] = sinf(m_angle);
    m_upvector[1] = cosf(m_angle);

    if (m_onGround) {
        m_angle -= m_dir.x() * 4.0f;
    }
    else {
        m_angle += m_angleInc * frameTime;
    }

    if (m_burnCounter >= 0.0f) {
        m_gameInstance->markFireBurning();
        m_burnCounter -= frameTime;
        m_burnParticleCounter += frameTime * 125.0f;
        int particleCount = (int)m_burnParticleCounter;
        ParticleEngine *particleEngine = m_gameInstance->getParticleEngine();
        QVector3D d = QVector3D(0,0,0);

        // Emit fire particles
        if (particleCount > 0) {
            particleEngine->emitParticles(
                particleCount, m_gameInstance->m_basicFireParticle,
                m_pos, m_r / 6.0f, d, 2.0f);
            m_burnParticleCounter -= (float)particleCount;
        }

        // Emit smoke particles
        if (m_burnCounter <= 0.0f) {
            particleEngine->emitParticles(
                10, m_gameInstance->m_smokeParticle,
                m_pos, m_r / 2.0f, d, 10.0f);
            m_burnCounter = -1.0f; // Stop burning
        }
    }

    GameObject::run(frameTime);
}


/*!
*/
void GameStaticObject::hit(GameObject *hitObj,
                           QVector3D &collisionPos,
                           QVector3D &collisionNormal)
{
    Q_UNUSED(hitObj);
    Q_UNUSED(collisionPos);

    m_angleInc *= -0.5f;

    if ((rand() & 255) < 100) {
        QVector3D d = collisionNormal * 10.0f;
        ParticleEngine *particleEngine = m_gameInstance->getParticleEngine();

        // Emit some dust particles
        particleEngine->emitParticles(
            8, m_gameInstance->m_dustParticle, m_pos, m_r, d, 20.0f);
        die();
    }
}



/*!
  \class GameUIObject
  \brief -
*/


/*!
  Constructor.
*/
GameUIObject::GameUIObject(GameInstance *gameInstance,
                           unsigned int texture)
    : GameObject(gameInstance)
{
    setTextureID(texture);
    m_r = 1.0f;
}


/*!
  Destructor.
*/
GameUIObject::~GameUIObject()
{
}


/*!
*/
void GameUIObject::run(float frameTime)
{
    m_run = false;
    GameObject::run(frameTime);
}


/*!
*/
void GameUIObject::hit(GameObject *hitObj,
                       QVector3D &collisionPos,
                       QVector3D &collisionNormal)
{
    Q_UNUSED(hitObj);
    Q_UNUSED(collisionPos);
    Q_UNUSED(collisionNormal);
}

