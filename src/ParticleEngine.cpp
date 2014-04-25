/**
 * Copyright (c) 2011-2014 Microsoft Mobile.
 */

#include "ParticleEngine.h"

#include <QMatrix4x4>
#include <math.h>
#include <stdlib.h>

#include "GameInstance.h"
#include "GameLevel.h" // For GAME_LEVEL_ZBASE
#include "trace.h"


    // Basic particle fragment program
const char* strParticleFragmentShader =
    "uniform sampler2D sampler2d;\n"
    "uniform mediump vec4 pcol;\n"
    "varying mediump vec2 texCoord;\n"
    "void main (void)\n"
    "{\n"
    "    gl_FragColor = texture2D(sampler2d, texCoord)*pcol;\n"
    "}";


    // Smoke (type of) particle fragment program
const char* strSmokeParticleFragmentShader =
    "uniform sampler2D sampler2d;\n"
    "uniform mediump vec4 pcol;\n"
    "varying mediump vec2 texCoord;\n"
    "uniform mediump mat4 transMatrix;\n"
    "void main (void)\n"
    "{\n"
    "    mediump vec3 transuv = vec3(-(0.5-texCoord[0]), -(0.5-texCoord[1]), 0.0);\n"
    "    transuv.z = 1.0-length(transuv)*2.0;\n"
    "    transuv = normalize(transuv);\n"

    "    mediump float cmx = -(transuv[0] * transMatrix[0][0]*pcol[0] + transuv[1]* transMatrix[1][0]*pcol[0]);\n"
    "    mediump float cmy = -(transuv[0] * transMatrix[0][1]*pcol[0] + transuv[1]* transMatrix[1][1]*pcol[0]);\n"
    "    mediump float cm = clamp(transuv.z*0.3+cmx*1.4 + cmy*0.7, 0.0, 1.0);\n"
    "    gl_FragColor = texture2D(sampler2d, texCoord)*vec4(pcol.yyz*cm, pcol[3]);\n"
    "}";

const char* strParticleVertexShader =
    "attribute highp vec3 vertex;\n"
    //"attribute mediump vec2 uv;\n"
    "uniform mediump mat4 transMatrix;\n"
    "uniform mediump mat4 projMatrix;\n"
    "varying mediump vec2 texCoord;\n"
    "void main(void)\n"
    "{\n"
         "highp vec4 transVertex = vec4(vertex,1.0)*transMatrix;\n"
         "gl_Position = transVertex * projMatrix;\n"
        "texCoord = vec2(0.5, 0.5)-vertex.xy*0.5;\n"
    "}";



/*!
  \class Particle
  \brief -
*/


/*!
  Constructor.
*/
Particle::Particle()
    : m_type(0),
      m_lifeTime(0),
      m_aliveCounter(0),
      m_angle(0),
      m_angleInc(0),
      m_size(0),
      m_sizeInc(0),
      m_color(0)

{
    memset(m_pos, 0, sizeof(int) * 3);
    memset(m_dir, 0, sizeof(int) * 3);
}


/*!
  Destructor.
*/
Particle::~Particle()
{
}


/*!
*/
void Particle::run(ParticleEngine *engine, int fixedFrameTime)
{
    // Move
    m_pos[0] += (((m_dir[0] >> 2) * fixedFrameTime) >> 10);
    m_pos[1] += (((m_dir[1] >> 2) * fixedFrameTime) >> 10);
    m_pos[2] += (((m_dir[2] >> 2) * fixedFrameTime) >> 10);

    // Fraction
    int te = ((m_type->m_fraction * fixedFrameTime) >> 12);

    if (te > 4096)
        te = 4096;

    m_dir[0] -= (((m_dir[0]>>2) * te) >> 10);
    m_dir[1] -= (((m_dir[1]>>2) * te) >> 10);
    m_dir[2] -= (((m_dir[2]>>2) * te) >> 10);

    // Gravity
    m_dir[1] -= (((m_type->m_gravity >> 2) * fixedFrameTime) >> 10);

    // Turbulence
    int turmul = ((m_type->m_turbulenceMul * fixedFrameTime) >> 12);

    if (turmul > 0) {
        m_dir[0] += ((engine->m_turbulenceMap[
                      (((engine->m_turbulencePhase+m_pos[1]) >> 10) & 127)]
                     [(((engine->m_turbulencePhase+m_pos[0]) >> 10) & 127)][0])
                     * turmul) >> 12;
        m_dir[1] += ((engine->m_turbulenceMap[
                      (((engine->m_turbulencePhase+m_pos[1]) >> 10) & 127)]
                     [(((engine->m_turbulencePhase+m_pos[0]) >> 10) & 127)][1])
                     * turmul) >> 12;
    }

    // Size increment
    m_size += (((m_sizeInc >> 4) * fixedFrameTime) >> 8);

    // Angle increment
    m_angle += (((m_angleInc >> 4) * fixedFrameTime) >> 8);

    // Second phase increments for size and angle increments.
    m_sizeInc += ((m_type->m_sizeIncInc * fixedFrameTime));
    m_angleInc += ((m_type->m_angleIncInc * fixedFrameTime));

    m_lifeTime -= fixedFrameTime;
    m_aliveCounter += fixedFrameTime;

    // Particle is dead if size has been dropped below one.
    if (m_size < 1 << 12)
        m_lifeTime = 0;
}



/*!
  \class ParticleEngine
  \brief -
*/


/*!
  Constructor.
*/
ParticleEngine::ParticleEngine(GameInstance *gameInstance, int maxParticles)
    : m_gameInstance(gameInstance),
      m_particles(0),
      m_maxParticles(maxParticles),
      m_currentParticle(0)
{
    m_particles = new Particle[maxParticles];
    memset(m_particles, 0, sizeof(Particle) * maxParticles);

    for (int f = 0; f < 512; f++) {
        m_cosTable[f] = cosf((float)f / 256.0f  * 3.14159265f);
    }

    // There are two different programs for particles in QOTH. Both of them share the same vertex shader and
    // only fragment shaders are program specific. We could use GameWindow's glhelpCreateShader  for building
    // them (to simplify the code), but it would take one additional vertex shader.
    GLint retval;
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1,
                   (const char**)&strParticleFragmentShader, NULL);
    glCompileShader(m_fragmentShader);
    glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO COMPILE FRAGMENT SHADER!");
    else
        DEBUG_INFO("Fragment shader compiled successfully!");

    m_smokeFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_smokeFragmentShader, 1,
                   (const char**)&strSmokeParticleFragmentShader, NULL);
    glCompileShader(m_smokeFragmentShader);
    glGetShaderiv(m_smokeFragmentShader, GL_COMPILE_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO COMPILE FRAGMENT SHADER FOR SMOKE PARTICLES!");
    else
        DEBUG_INFO("Smoke particle fragment shader compiled successfully!");

    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1,
                   (const char**)&strParticleVertexShader, NULL);
    glCompileShader(m_vertexShader);
    glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO COMPILE VERTEX SHADER!");
    else
        DEBUG_INFO("Vertex shader compiled successfully!");

    m_program = glCreateProgram();
    glAttachShader(m_program, m_fragmentShader);
    glAttachShader(m_program, m_vertexShader);

    // Bind the custom vertex attributes
    glBindAttribLocation(m_program, 0, "vertex");
    glBindAttribLocation(m_program, 1, "uv");

    glLinkProgram(m_program);

    // Check if the linking succeeded in the same way we checked the
    // compilation success.
    glGetProgramiv(m_program, GL_LINK_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO LINK PROGRAM!");
    else
        DEBUG_INFO("Program linked successfully!");

    glUniform1i(glGetUniformLocation(m_program, "sampler2d"), 0);

    // Smoke program
    m_smokeProgram = glCreateProgram();
    glAttachShader(m_smokeProgram, m_smokeFragmentShader);
    glAttachShader(m_smokeProgram, m_vertexShader);

     // Bind the custom vertex attributes
    glBindAttribLocation(m_smokeProgram, 0, "vertex");
    glBindAttribLocation(m_smokeProgram, 1, "uv");

    glLinkProgram(m_smokeProgram);

    // Check if the linking succeeded.
    glGetProgramiv(m_smokeProgram, GL_LINK_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO LINK SMOKE PROGRAM!");
    else
        DEBUG_INFO("Smoke program linked successfully!");

    glUniform1i(glGetUniformLocation(m_smokeProgram, "sampler2d"), 0);

    // Create turbulence map
    for (int y = 0; y < 128; y++) {
        for (int x = 0; x < 128; x++) {
            m_turbulenceMap[y][x][0] =
                (short)(cosf((float)y / 127.0f * 3.14159f * 6.0f) * 40000.0f);
            m_turbulenceMap[y][x][1] =
                (short)(sinf((float)x / 127.0f * 3.14159f * 4.0f)
                        * sinf((float)y / 127.0f * 3.14159f * 8.0f) * 40000.0f);
        }
    }

    // create VBO for particles.
    glGenBuffers(1, &m_vbo);

    GLfloat vertices[] = { -1.0f, -1.0f, 0.0f, 1.0f, -1.0f, 0.0f,
                           1.0f, 1.0f, 0.0f, -1.0f, 1.0f, 0.0f };

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 4 * sizeof(GLfloat) * 3,
                 vertices, GL_STATIC_DRAW);
}


/*!
  Destructor.
*/
ParticleEngine::~ParticleEngine()
{
    glDeleteBuffers(1, &m_vbo);
    delete [] m_particles;
}


/*!
*/
void ParticleEngine::render(ParticleType *renderType, GLuint program)
{
    if (!renderType)
        return;

    glEnable(GL_BLEND);

    if (renderType->m_additiveParticle) {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }
    else {
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    glUniformMatrix4fv(glGetUniformLocation(program, "projMatrix"), 1,
                       GL_FALSE, m_gameInstance->getProjectionMatrix());
    glUniform1i(glGetUniformLocation(program, "sampler2d"), 0);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, 3 * sizeof(GL_FLOAT), 0);

    float iden[16];
    float m[16];
    memset(iden, 0, sizeof(float) * 16);
    iden[0] = 1.0f;
    iden[5] = 1.0f;
    iden[10] = 1.0f;
    iden[15] = 1.0f;
    GLfloat col[4];
    float ftemp;
    int colorLocation = glGetUniformLocation(program, "pcol");
    int location = glGetUniformLocation(program, "transMatrix");

    float *cam = m_gameInstance->getCameraMatrix();
    Q_UNUSED(cam);

    float sizeMul;
    Particle *p = m_particles;
    Particle *p_target = m_particles + m_maxParticles;

    while (p != p_target) {
        if (p->m_type == renderType && p->m_lifeTime > 0) {
            memcpy(m, iden, sizeof(float) * 16);
            sizeMul = (float)p->m_size / 409600.0f;
            m[0] = m_cosTable[(p->m_angle >> 8) & 511] * sizeMul;
            m[1] = m_cosTable[((p->m_angle >> 8) + 128) & 511] * sizeMul;
            m[4] = m[1];
            m[5] = -m[0];
            m[3] = (float)p->m_pos[0] / 4096.0f;
            m[7] = (float)p->m_pos[1] / 4096.0f;
            m[11] = (float)p->m_pos[2] / 4096.0f + GAME_LEVEL_ZBASE;

            // NOTE: Particle might work without "full" camera transform.
            // Just by taking care of the position.
            m_gameInstance->cameraTransform(m);

            // Carry the scaling information for smoke program if active
            if (program == m_smokeProgram)
                col[0] = 1.0f/sizeMul;
            else
                col[0] = (float)((p->m_color) & 255) / 255.0f;

            col[1] = (float)((p->m_color >> 8) & 255) / 255.0f;
            col[2] = (float)((p->m_color >> 16) & 255) / 255.0f;
            col[3] = p->m_type->m_generalVisibility * (float)p->m_lifeTime
                    / (65536.0f / 4.0f) * p->m_type->m_fadeOutTimeSecs;

            if (col[3] > p->m_type->m_generalVisibility)
                col[3] = p->m_type->m_generalVisibility;

            ftemp = ((float)p->m_aliveCounter / (65536.0f / 4.0f))
                    * p->m_type->m_fadeInTimeSecs;

            if (ftemp < col[3])
                col[3] = ftemp;

            glUniform4fv(colorLocation, 1, col);
            glUniformMatrix4fv(location, 1, GL_FALSE, m);

            // Draw particle
            glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
        }

        p++;
    }

    glDisableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


/*!
*/
void ParticleEngine::run(float frameTime)
{
    // -> 12bit fixedpoint
    int fixedFrameTime = (int)(frameTime * 4096.0f);

    m_turbulencePhase += fixedFrameTime * 6;
    Particle *p = m_particles;
    Particle *p_target = p + m_maxParticles;

    while (p != p_target) {
        if (p->m_lifeTime > 0)
            p->run(this, fixedFrameTime);

        p++;
    }
}


/*!
  Emits \a count number of particles of type \a type. The initial position is
  set as \a pos. \a posRandom defines a variance for the emit position.
  Direction of the emit and the variance are determined by \a dir and
  \a dirRandom.
*/
void ParticleEngine::emitParticles(int count,
                                   ParticleType *type,
                                   QVector3D &pos,
                                   float posRandom,
                                   QVector3D &dir,
                                   float dirRandom)
{
    int fixedPosition[3];
    int fixedDirection[3];

    int fixedPosRandom = (int)(posRandom * 256.0f);
    int fixedDirRandom = (int)(dirRandom * 256.0f);

    fixedPosition[0] = (int)(pos.x() * 4096.0f);
    fixedPosition[1] = (int)(pos.y() * 4096.0f);
    fixedPosition[2] = (int)(pos.z() * 4096.0f);

    fixedDirection[0] = (int)(dir.x() * 4096.0f);
    fixedDirection[1] = (int)(dir.y() * 4096.0f);
    fixedDirection[2] = (int)(dir.z() * 4096.0f);

    int fixedRandom[3];
    int temp;
    float c[3];

    Particle *p;

    while (count > 0) {
        p = m_particles + m_currentParticle;
        p->m_type = type;

        p->m_aliveCounter = 0;

        // Create a random vector
        fixedRandom[0] = (rand() & 255) - 128;
        fixedRandom[1] = (rand() & 255) - 128;
        fixedRandom[2] = (rand() & 255) - 128;

        temp = (int)sqrtf(fixedRandom[0] * fixedRandom[0]
                          + fixedRandom[1] * fixedRandom[1]
                          + fixedRandom[2] * fixedRandom[2]);

        if (temp > 0) {
            fixedRandom[0] = (fixedRandom[0] << 16) / temp;
            fixedRandom[1] = (fixedRandom[1] << 16) / temp;
            fixedRandom[2] = (fixedRandom[2] << 16) / temp;
        }

        // Position
        p->m_pos[0] = ((fixedRandom[0] * fixedPosRandom) >> 12) + fixedPosition[0];
        p->m_pos[1] = ((fixedRandom[1] * fixedPosRandom) >> 12) + fixedPosition[1];
        p->m_pos[2] = ((fixedRandom[2] * fixedPosRandom) >> 12) + fixedPosition[2];

        // Direction
        p->m_dir[0] = ((fixedRandom[0] * fixedDirRandom) >> 12) + fixedDirection[0];
        p->m_dir[1] = ((fixedRandom[1] * fixedDirRandom) >> 12) + fixedDirection[1];
        p->m_dir[2] = ((fixedRandom[2] * fixedDirRandom) >> 12) + fixedDirection[2];

        p->m_angle = type->m_angle
                + (((rand() & 255) * type->m_angleRandom) >> 8);
        p->m_angleInc = type->m_angleInc
                + (((rand() & 255) * type->m_angleIncRandom) >> 8);

        p->m_size = type->m_size
                + (((rand() & 255) * type->m_sizeRandom) >> 8);
        p->m_sizeInc = type->m_sizeInc
                + (((rand() & 255) * type->m_sizeIncRandom) >> 8);

        p->m_lifeTime = type->m_lifeTime
                + (((rand() & 255) * type->m_lifeTimeRandom) >> 8);

        c[0] = type->m_col[0] + ((float)(rand() & 255) / 255.0f) * type->m_colRandom[0];
        c[1] = type->m_col[1] + ((float)(rand() & 255) / 255.0f) * type->m_colRandom[1];
        c[2] = type->m_col[2] + ((float)(rand() & 255) / 255.0f) * type->m_colRandom[2];

        if (c[0]>1.0f) c[0] = 1.0f; if (c[0]<0.0f) c[0] = 0.0f;
        if (c[1]>1.0f) c[1] = 1.0f; if (c[1]<0.0f) c[1] = 0.0f;
        if (c[2]>1.0f) c[2] = 1.0f; if (c[2]<0.0f) c[2] = 0.0f;

        p->m_color = (unsigned int)(c[0] * 255.0f)
                | ((unsigned int)(c[1] * 255.0f) << 8)
                | ((unsigned int)(c[2] * 255.0f) << 16);

        count--;
        m_currentParticle++;

        if (m_currentParticle >= m_maxParticles)
            m_currentParticle = 0;
    }
}


/*!
*/
void ParticleEngine::emitParticleLine(int count,
                                      ParticleType *type,
                                      QVector3D &pos1,
                                      QVector3D &pos2,
                                      float posRandom,
                                      QVector3D &dir,
                                      float dirRandom)
{
    Q_UNUSED(count);
    Q_UNUSED(type);
    Q_UNUSED(pos1);
    Q_UNUSED(pos2);
    Q_UNUSED(posRandom);
    Q_UNUSED(dir);
    Q_UNUSED(dirRandom);

    /*
    if (count<1)
        return;

    QVector3D ofs = pos2 - pos1;
    ofs /= (float)count;
    int c = 0;

    while (c < count) {
        QVector3D p = expos + ofs * (float)c;
        peng->emitParticles(
                    1, m_gameInstance->m_smallSmokeParticle, p,
                    posRandom, dir, dirrandom);
        c++;
    }
    */
}



/*!
  \class ParticleType
  \brief -
*/


/*!
  Constructor.
*/
ParticleType::ParticleType(unsigned int program, unsigned int textureId)
    : m_turbulenceMul(4096 * 10),
      m_angle(0),
      m_angleRandom(0),
      m_angleInc(0),
      m_angleIncRandom(0),
      m_size(4096),
      m_sizeRandom(0),
      m_sizeInc(0),
      m_sizeIncRandom(0),
      m_lifeTime(4096),
      m_lifeTimeRandom(0),
      m_fraction(0),
      m_gravity(0),
      m_angleIncInc(0),
      m_sizeIncInc(0),
      m_program(program),
      m_textureId(textureId),
      m_fadeOutTimeSecs(1.0f / 2000.0f),
      m_fadeInTimeSecs(0),
      m_generalVisibility(0),
      m_additiveParticle(true)
{
    setVisibility(0.0f, 0.5f, 1.0f);
    m_col[0] = 1.0f;
    m_col[1] = 1.0f;
    m_col[2] = 1.0f;
    m_colRandom[0] = 0.0f;
    m_colRandom[1] = 0.0f;
    m_colRandom[2] = 0.0f;
}


/*!
  Destructor.
*/
ParticleType::~ParticleType()
{
}


/*!
  Sets the visibility.
*/
void ParticleType::setVisibility(float fadeInTime,
                                 float fadeOutTime,
                                 float generalVisibility)
{
    Q_UNUSED(generalVisibility);

    m_fadeOutTimeSecs = 1.0f / (fadeOutTime);
    m_fadeInTimeSecs = 1.0f / (fadeInTime);
    m_generalVisibility = 1.0f;
}


/*!
  Sets the colors.
*/
void ParticleType::setColors(float r, float g, float b,
                             float rr, float gr, float br)
{
    m_col[0] = r;
    m_col[1] = g;
    m_col[2] = b;
    m_colRandom[0] = rr;
    m_colRandom[1] = gr;
    m_colRandom[2] = br;
}

