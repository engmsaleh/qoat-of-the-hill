/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef PARTICLEENGINE_H
#define PARTICLEENGINE_H

#include <GLES2/gl2.h>
#include <QVector3D>

class GameInstance;
class ParticleEngine;
class ParticleType;


class Particle
{
public:
    Particle();
    ~Particle();

public:
    void run(ParticleEngine *engine, int fixedFrameTime);

public: // Data
    ParticleType *m_type;
    int m_pos[3];
    int m_dir[3];
    int m_lifeTime;
    int m_aliveCounter;
    int m_angle;
    int m_angleInc;
    int m_size;
    int m_sizeInc;
    unsigned int m_color;
};



class ParticleEngine
{
public:
    ParticleEngine(GameInstance *gameInstance, int maxParticles);
    virtual ~ParticleEngine();

public:
    void run(float frameTime);
    void render(ParticleType *renderType, GLuint program);
    void emitParticles(int count,
                       ParticleType *type,
                       QVector3D &pos, float posRandom,
                       QVector3D &dir, float dirRandom);
    void emitParticleLine(int count,
                          ParticleType *type,
                          QVector3D &pos1,
                          QVector3D &pos2, float posRandom,
                          QVector3D &dir, float dirRandom);
    GLuint normalProgram() { return m_program; }
    GLuint smokeProgram() { return m_smokeProgram; }

public: // Data
    short m_turbulenceMap[128][128][2];
    int m_turbulencePhase;

protected:
    GameInstance *m_gameInstance;
    Particle *m_particles;
    GLint m_program;
    GLint m_smokeProgram;
    float m_cosTable[512];
    int m_maxParticles;
    int m_currentParticle;
    GLint m_smokeFragmentShader;
    GLint m_fragmentShader;
    GLint m_vertexShader;
    GLuint m_vbo;
};


class ParticleType
{
public:
    ParticleType(unsigned int program, unsigned int textureId);
    virtual ~ParticleType();

public:
    // Attributes for this function as real seconds
    void setVisibility(float fadeInTime,
                       float fadeOutTime,
                       float generalVisibility);
    void setColors(float r, float g, float b,
                   float rr, float gr, float br);

public:
    // How much turbulence affects this type
    int m_turbulenceMul;

    // Angle and anglevariance
    int m_angle;
    int m_angleRandom;

    // Angle increment/time and radiance for it
    int m_angleInc;
    int m_angleIncRandom;

    // Size and sizevariance
    int m_size;
    int m_sizeRandom;

        // Size increment/time and vadiance for it
    int m_sizeInc;
    int m_sizeIncRandom;

    // Lifetime in fixed seconds and variance for it
    int m_lifeTime;
    int m_lifeTimeRandom;

    // Fraction for this type
    int m_fraction;

    // Gravity for this type
    int m_gravity;

    // Angle increments increment/time
    int m_angleIncInc;

    // Size increments increment/time
    int m_sizeIncInc;

    // GLPrgram used for rendering this type
    unsigned int m_program;

    // GLTexture used for rendering this type
    unsigned int m_textureId;

    float m_fadeOutTimeSecs;
    float m_fadeInTimeSecs;
    float m_generalVisibility;
    bool m_additiveParticle;

    float m_col[3];
    float m_colRandom[3];
};


#endif // PARTICLEENGINE_H
