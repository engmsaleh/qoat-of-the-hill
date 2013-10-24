/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef GAMEOBJECT_H
#define GAMEOBJECT_H

#include <QVector3D>
#include <GLES2/gl2.h>

class GameInstance;
class GameObjectManager;

// A speed below bouncing/movement stops completely
#define BOUNCE_SPEED_LIMIT 15.0f


class GameObject
{
public:
    GameObject(GameInstance *gameInstance);
    virtual ~GameObject();

public:
    inline QVector3D &pos() { return m_pos; }
    inline QVector3D &dir() { return m_dir; }

    inline void setTextureID(unsigned int textureID)
    {
        m_textureID = textureID;
    }

    void setDepthEnabled(bool set) { m_depthEnabled = set; }
    inline bool depthEnabled() { return m_depthEnabled; }
    inline void setGravity(float g) { m_gravity = g; }
    inline void setAirFraction(float set) { m_airFraction = set; }
    inline float r() { return m_r; }
    inline void setr(float set) { m_r = set; }
    inline bool isOnGround() { return m_onGround; }
    inline void setOnGround(bool set) { m_onGround = set; }
    inline void setRunEnabled(bool set) { m_run = set; }
    inline bool isRunEnabled() { return m_run ;}
    inline void setFlipX(bool set) { m_flipX = set; }
    inline bool getFlipX() { return m_flipX; }
    inline void setFlipY(bool set) { m_flipY = set; }
    inline bool getFlipY() { return m_flipY; }
    inline float getAlpha() { return m_alpha; }
    inline float getLightness() { return m_lightness; }
    inline void setAlpha(float alpha) { m_alpha = alpha; }
    inline void setLightness(float lightness) { m_lightness = lightness; }

    virtual void run(float frameTime);
    virtual void render(GameObjectManager *gameObjectMgr, float *m);
    virtual void pushForce(QVector3D &pos, float r, float power);

    virtual void hit(GameObject *target,
                     QVector3D &collisionPos,
                     QVector3D &collisionNormal)
    {
        Q_UNUSED(target);
        Q_UNUSED(collisionPos);
        Q_UNUSED(collisionNormal);
    }

    inline void die() { m_dead = true; }

    virtual bool isDead();

    bool isDying() { return m_dead; }
    inline float dieAnimation() { return m_dieAnimation; }
    inline float* getUpVector() { return m_upvector; }

    inline void setUpVector(float x, float y)
    {
        m_upvector[0] = x;
        m_upvector[1] = y;
    }

    inline void setAspect(float set) { m_aspect = set;}
    inline float aspect() { return m_aspect; }

    inline bool isCenterSprite() { return m_centerSprite; }
    inline void setCenterSprite(bool set) { m_centerSprite = set; }

public: // Data
    GameObject *m_next; // Next item in game object list
    unsigned int m_textureID;

protected: // Data
    GameInstance *m_gameInstance;
    QVector3D m_groundNormal;
    float m_lightness;
    float m_alpha;
    bool m_depthEnabled;
    bool m_flipX;
    bool m_flipY;
    bool m_run;
    bool m_dead;
    float m_dieAnimation;
    bool m_onGround;
    float m_upvector[2];
    float m_r;
    float m_aspect;
    float m_gravity;
    float m_airFraction;
    bool m_moveOnGround;
    float m_powerResponse;
    bool m_centerSprite;
    QVector3D m_pos;
    QVector3D m_dir;
};



class GameObjectManager
{
public:
    GameObjectManager(GameInstance *gameInstance);
    virtual ~GameObjectManager();

public:
    void run(float frameTime);
    void render(bool bgObjects);
    GameObject *addObject(GameObject *object);
    void destroyAll();
    void pushObjects(QVector3D &pos, float r, float power);

public: // Data
    GameInstance *m_gameInstance;
    GLuint m_program;

protected: // Data
    GameObject *m_objectList;
    GLuint m_fragmentShader;
    GLuint m_vertexShader;
    GLuint m_vbo;
};


#endif // GAMEOBJECT_H
