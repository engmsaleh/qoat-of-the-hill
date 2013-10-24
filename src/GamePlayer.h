/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef GAMEPLAYER_H
#define GAMEPLAYER_H

#include "GameObject.h"

namespace GE {
    class AudioBufferPlayInstance;
}



class GameAmmunition : public GameObject
{
public:
    GameAmmunition(GameInstance *gameInstance);
    virtual ~GameAmmunition();

public:
    void run(float frameTime);
    void hit(GameObject *hitObj,
             QVector3D &collisionPos,
             QVector3D &collisionNormal);

protected:
    GE::AudioBufferPlayInstance *m_whistleInstance;
};



// Small burning piece
class GameBurningPiece : public GameObject
{
public:
    GameBurningPiece(GameInstance *gameInstance);
    virtual ~GameBurningPiece();

public:
    void run(float frameTime);
    void hit(GameObject *hitObj,
             QVector3D &collisionPos,
             QVector3D &collisionNormal);

protected: // Data
    float m_lifeTime;
    float m_burnParticleCounter;
};



class GamePlayer : public GameObject
{
public:
    GamePlayer(GameInstance *gameInstance);
    virtual ~GamePlayer();

public:
    void run(float frameTime);
    void setShootVector(QVector3D shootVector);
    void hit(GameObject *hitObj,
             QVector3D &collisionPos,
             QVector3D &collisionNormal);

    void setEnemyPos(QVector3D &enemyPos)
    {
        m_enemyPos = enemyPos;

        if (m_enemyPos.x() < m_pos.x())
            m_flipX = false;
        else
            m_flipX = true;
    }

    void aimTo(QVector3D &aimPos)
    {
        m_aimPos = aimPos;
        m_aiming = true;
    }

    QVector3D &aimPos() { return m_aimPos; }
    void stopAiming() { m_aiming = false; }

    void pushForce(QVector3D &pos, float r, float power);

    inline bool isAiming() { return m_aiming; }
    QVector3D &gunPos() { return m_gun->pos(); }
    GameObject *gun() { return m_gun; }

    void createAssets();

protected: // Data
    float m_health;
    QVector3D m_enemyPos;
    QVector3D m_aimPos;
    bool m_aiming;
    float m_hit;
    float m_breath;
    float m_upvectorTarget[2];
    GameObject *m_head;
    GameObject *m_gun;
    QVector3D m_previousShootVector;
    GameObject *m_previousShootArrow;
};



// Tree or a plant etc.
class GameTree : public GameObject
{
public:
    GameTree(GameInstance *gameInstance, unsigned int texture);
    virtual ~GameTree();

public:
    void run(float frameTime);
    void pushForce(QVector3D &pos, float r, float power);

protected: // Data
    float m_angle;
    float m_angleInc;
};



class GameStaticObject : public GameObject
{
public:
    GameStaticObject(GameInstance *gameInstance, unsigned int texture);
    virtual ~GameStaticObject();

public:
    void run(float frameTime);
    void hit(GameObject *hitObj,
             QVector3D &collisionPos,
             QVector3D &collisionNormal);
    void burn();

public: // Data
    float m_angle;
    float m_angleInc;

protected: // Data
    float m_burnCounter;
    float m_burnParticleCounter;
};



class GameUIObject : public GameObject
{
public:
    GameUIObject(GameInstance *gameInstance, unsigned int texture);
    virtual ~GameUIObject();

public:
    void run(float frameTime);
    void hit(GameObject *hitObj,
             QVector3D &collisionPos,
             QVector3D &collisionNormal);
};


#endif // GAMEPLAYER_H
