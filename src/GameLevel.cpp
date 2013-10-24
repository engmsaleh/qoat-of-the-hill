/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "GameLevel.h"

#include <math.h>

#include "GameInstance.h"
#include "TextureManager.h"
#include "trace.h"

#define LEVEL_Y_MIN -12.0f


// Rock fragment shader
const char* strRockFragmentShader =
"uniform sampler2D sampler2d;\n"
"varying mediump vec2 texCoord;\n"
"varying mediump vec4 color;\n"
"varying mediump vec4 colormul;\n"
"void main (void)\n"
"{\n"
"    lowp vec4 rock = texture2D(sampler2d, texCoord*0.5);\n"
"    gl_FragColor = vec4(rock.xyz*colormul.xyz, clamp(((color[3] + (rock[3]-0.5)*4.0)), 0.0, 1.0)*colormul[3]);\n"
"}";


// WITHOUT rock-texture
const char* strGroundFragmentShader =
"uniform sampler2D sampler2d;\n"
"varying mediump vec2 texCoord;\n"
"varying mediump vec4 color;\n"
"varying mediump vec4 colormul;\n"
"void main (void)\n"
"{\n"
#ifdef Q_WS_MAEMO_5
"    mediump float vv = (texCoord[1]-floor(texCoord[1]))*0.33+0.0016;\n"
"    lowp vec4 snow = texture2D(sampler2d, vec2(texCoord[0], vv+0.6666));\n"
"    gl_FragColor = mix(snow.xxxx, texture2D(sampler2d, vec2(texCoord[0], vv+0.3333)), clamp(color[0]+(snow[3]-0.5)*2.0, 0.0, 1.0)) * colormul;\n"
#else
"    mediump float vv = (texCoord[1]-floor(texCoord[1]))*0.33+0.0016;\n"
"    lowp vec4 snow = texture2D(sampler2d, vec2(texCoord[0], vv+0.6666));\n"
    // Add snowy grass
"    lowp vec4 col1 = mix(texture2D(sampler2d, vec2(texCoord[0], vv)), snow.xxxx, clamp(color[1]+(snow.w-0.5)*2.0, 0.0, 1.0));\n"
    // Add destroyed ground
"    col1 = mix(col1, texture2D(sampler2d, vec2(texCoord[0], vv+0.3333)), clamp(color[0]+(snow[3]-0.5)*2.0, 0.0, 1.0));\n"
"    gl_FragColor = col1 * colormul;\n"
#endif
"}";


// TEST SHADER
/*
const char* strGroundFragmentShader =
"uniform sampler2D sampler2d;\n"
"varying mediump vec2 texCoord;\n"
"varying mediump vec4 color;\n"
"varying mediump vec4 colormul;\n"
"void main (void)\n"
"{\n"
"    mediump float vv = (texCoord[1]-floor(texCoord[1]))*0.248+0.001;\n"
"    mediump vec4 rock = texture2D(sampler2d, vec2(texCoord[0], vv+0.75));\n"

"    lowp vec4 col1 = texture2D(sampler2d, vec2(texCoord[0], vv+0.5)).wwww;\n"                          // get snow
"    lowp float m = clamp(color[1]+(rock[3]-0.5)*2.0, 0.0, 1.0);\n"
"    col1 = col1*(1.0-m) +  texture2D(sampler2d, vec2(texCoord[0], vv)) * (m);\n"                     // add snowy grass

"    m = clamp(color[0]+(rock[3]-0.5)*2.0, 0.0, 1.0);\n"
"    col1 = col1*(1.0-m) + texture2D(sampler2d, vec2(texCoord[0], vv+0.25)) * (m);\n"

// blend everything with rock texture and multiply the whole thing with light
"    m = clamp(color[3]+(rock[3]-0.5)*4.0, 0.0, 1.0);\n"
"    col1 = (col1*(1.0-m) + rock*m);\n"
 "   gl_FragColor = col1*colormul;\n"
"}";
*/


const char* strGroundVertexShader =
"attribute highp vec3 vertex;\n"
"attribute mediump vec2 uv;\n"
"attribute mediump vec4 vertexcolor;\n"
"attribute mediump vec3 vertexnormal;\n"
"uniform mediump mat4 transMatrix;\n"
"uniform mediump mat4 projMatrix;\n"
"varying mediump vec2 texCoord;\n"
"varying mediump vec4 color;\n"
"varying mediump vec4 colormul;\n"
"void main(void)\n"
"{\n"
"mediump vec4 temppos = vec4(vertex,1.0)*transMatrix;\n"
"texCoord = uv;\n"
"gl_Position = temppos * projMatrix;\n"
"color = vertexcolor;\n"
"mediump float l = clamp((vertexnormal.x + vertexnormal.y*0.4)*3.0, 0.25, 1.0);\n"
//"colormul = vec4(l, l, l, clamp(1.2+(vertex.y*0.2), 0.0, 1.0));\n"
"lowp float rockamount = clamp(vertexcolor[3]*0.5, 0.0, 1.0);\n"
"colormul = vec4(l, l, l, clamp(1.0+((vertex.y+10.0)*10.0*rockamount), 0.0, 1.0));\n"
"}";


/*!
  \class GameLevel
  \brief -
*/


/*!
  Constructor.
*/
GameLevel::GameLevel(GameInstance *gameInstance)
    : m_gameInstance(gameInstance),
      m_vertices(0),
      m_indices(0),
      m_vertexCount(0),
      m_indexCount(0),
      m_forceUpdate(false)
{
    for (int f = 0; f < GAME_LEVEL_GRID_HEIGHT; f++) {
        for (int g = 0; g < GAME_LEVEL_GRID_WIDTH; g++) {
            m_randomArray[g][f] = (char)(-127 + (rand() & 255));
        }
    }


    // There are two different programs for QOTH's ground rendering. Both of them share the same vertex shader and
    // only fragment shaders are program specific. We could use GameWindow's glhelpCreateShader  for building
    // them (to simplify the code), but it would take one additional vertex shader.
    GLint retval;
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1,
                   (const char**)&strGroundFragmentShader, NULL);
    glCompileShader(m_fragmentShader);
    glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO COMPILE GROUND FRAGMENT SHADER!");
    else
        DEBUG_INFO("Ground fragment shader compiled successfully!");

    m_rockFragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_rockFragmentShader, 1,
                   (const char**)&strRockFragmentShader, NULL);
    glCompileShader(m_rockFragmentShader);
    glGetShaderiv(m_rockFragmentShader, GL_COMPILE_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO COMPILE ROCK FRAGMENT SHADER!");
    else
        DEBUG_INFO("Rock fragment shader compiled successfully!");

    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1,
                   (const char**)&strGroundVertexShader, NULL);
    glCompileShader(m_vertexShader);
    glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO COMPILE VERTEX SHADER!");
    else
        DEBUG_INFO("Vertex shader compiled successfully!");

    // Main program for the top.
    m_program = glCreateProgram();
    glAttachShader(m_program, m_fragmentShader);
    glAttachShader(m_program, m_vertexShader);

    // Bind the custom vertex attributes.
    glBindAttribLocation(m_program, 0, "vertex");
    glBindAttribLocation(m_program, 1, "uv");
    glBindAttribLocation(m_program, 2, "vertexcolor");
    glBindAttribLocation(m_program, 3, "vertexnormal");

    glLinkProgram(m_program);

    // Check if the linking succeeded.
    glGetProgramiv(m_program, GL_LINK_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO LINK PROGRAM!");
    else
        DEBUG_INFO("Program linked successfully!");

    glUniform1i(glGetUniformLocation(m_program, "sampler2d"), 0);

    // Rock program.
    m_rockProgram = glCreateProgram();
    glAttachShader(m_rockProgram, m_rockFragmentShader);
    glAttachShader(m_rockProgram, m_vertexShader);

    // Bind the custom vertex attributes
    glBindAttribLocation(m_rockProgram, 0, "vertex");
    glBindAttribLocation(m_rockProgram, 1, "uv");
    glBindAttribLocation(m_rockProgram, 2, "vertexcolor");
    glBindAttribLocation(m_rockProgram, 3, "vertexnormal");

    glLinkProgram(m_rockProgram);

    // Check if the linking succeeded.
    glGetProgramiv(m_rockProgram, GL_LINK_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO LINK ROCK PROGRAM!");
    else
        DEBUG_INFO("Rock program linked successfully!");

    glUniform1i(glGetUniformLocation(m_rockProgram, "sampler2d"), 0);

    glGenBuffers(1, &m_vbo);
    glGenBuffers(1, &m_indexBuffer);
}


/*!
  Destructor.
*/
GameLevel::~GameLevel()
{
    destroy();
    glDeleteBuffers(1, &m_vbo);
    glDeleteBuffers(1, &m_indexBuffer);
    glDeleteProgram(m_program);
    glDeleteProgram(m_rockProgram);
    glDeleteShader(m_fragmentShader);
    glDeleteShader(m_rockFragmentShader);
    glDeleteShader(m_vertexShader);
}


/*!
*/
void GameLevel::recreate()
{
    int randTable[256];

    for (int f = 0; f < 256; f++)
        randTable[f] = (rand() & 1) * 65536;

    float edge;

    // Init'n'create peak-array
    for (int f = 0; f < GAME_LEVEL_GRID_WIDTH + 1; f++) {
        m_xposArray[f] = GAME_LEVEL_START_X
                + (GAME_LEVEL_END_X - GAME_LEVEL_START_X)
                * (float)f / (float)(GAME_LEVEL_GRID_WIDTH - 1);

        if (f < GAME_LEVEL_GRID_WIDTH) {
            edge = ((float)f - (float)((GAME_LEVEL_GRID_WIDTH - 1) / 2.0f))
                    / ((float)GAME_LEVEL_GRID_WIDTH / 2.0f);
            float fedge = 1.0f - powf(fabsf(edge), 0.75f);
            float medge = powf(edge, 14.0f);

            // Use a cutted cosine as our levels base form
            m_peakArray[f] =
                    cosf(((float)f / (float)(GAME_LEVEL_GRID_WIDTH - 1)
                          * 1.4f + 0.2f) * 3.14159f) * 4.0f;

            // Lower the very furthest edges exponentially
            m_peakArray[f] = m_peakArray[f] * (1.0f - medge) + (medge * -14.0f);

            // Add noise to the centre of the level
            m_peakArray[f] += (getNoiseValue(randTable, f * 48) * 14.0f * fedge);

            m_originalPeakArray[f] = m_peakArray[f];
            m_destroyedArray[f] = -1.0f;
        }
    }

    recreateVertices();
    m_forceUpdate = true;
}


/*!
*/
void GameLevel::destroy()
{
    if (m_vertices)
        delete [] m_vertices;

    if (m_indices)
        delete [] m_indices;

    m_vertices = 0;
    m_indices = 0;
    m_vertexCount = 0;
    m_indexCount = 0;
}


/*!
*/
void GameLevel::run(float frameTime)
{
    Q_UNUSED(frameTime);

    if (!m_forceUpdate)
        return;

    m_forceUpdate = false;

    // calculate 2D normals
    recreateNormals();
    updateMesh();
}


/*!
*/
void GameLevel::render()
{
    glEnable(GL_CULL_FACE);
    glFrontFace(GL_CW);
    glDepthMask(GL_TRUE);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    glEnableVertexAttribArray(3);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12, 0);
    glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12,
                          (void*)(sizeof(GLfloat) * 3));
    glVertexAttribPointer(2,4, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12,
                          (void*)(sizeof(GLfloat) * 5));
    glVertexAttribPointer(3,3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 12,
                          (void*)(sizeof(GLfloat) * 9));

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);

    float m[16];
    memset(m, 0, sizeof(GLfloat) * 16);
    m[0] = 1.0f;
    m[5] = 1.0f;
    m[10] = 1.0f;
    m[15] = 1.0f;
    m_gameInstance->cameraTransform(m);

    glEnable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);

    // Draw the top
    glBindTexture(GL_TEXTURE_2D,
        m_gameInstance->getTextureManager()->getTexture(":/ground.png"));


    glUseProgram(m_program);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "transMatrix"),
                       1, GL_FALSE, m);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "projMatrix"),
                       1, GL_FALSE, m_gameInstance->getProjectionMatrix());
    glUniform1i(glGetUniformLocation(m_program, "sampler2d"), 0);

    glDrawElements(GL_TRIANGLES,
                   (m_indexCount - (GAME_LEVEL_GRID_WIDTH - 1) * 6),
                   GL_UNSIGNED_SHORT, 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    glDepthFunc(GL_LEQUAL);

    glUseProgram(m_rockProgram);
    glUniformMatrix4fv(glGetUniformLocation(m_rockProgram, "transMatrix"),
                       1, GL_FALSE, m);
    glUniformMatrix4fv(glGetUniformLocation(m_rockProgram, "projMatrix"),
                       1, GL_FALSE, m_gameInstance->getProjectionMatrix());
    glUniform1i(glGetUniformLocation(m_rockProgram, "sampler2d"), 0);

    glBindTexture(GL_TEXTURE_2D,
        m_gameInstance->getTextureManager()->getTexture(":/rock_wall.png"));

    int start = (6 * (GAME_LEVEL_GRID_WIDTH - 1) * 1);
    start = 12 * (GAME_LEVEL_GRID_WIDTH - 1) * 2;
    glDrawElements(GL_TRIANGLES,
                   6 * (GAME_LEVEL_GRID_WIDTH - 1) * 2,
                   GL_UNSIGNED_SHORT, (void*)start);

    glDepthFunc(GL_LESS);
    glDisable(GL_CULL_FACE);

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}


/*!
*/
float GameLevel::getHeightAndNormalAt(float x, QVector3D *normalTarget)
{
    if (x < GAME_LEVEL_START_X || x > GAME_LEVEL_END_X) {
        normalTarget->setX(0.0f);
        normalTarget->setY(1.0f);
        normalTarget->setZ(0.0f);
        return -100.0f;
    }

    int ind1 = 0;
    int ind2 = 1;

    while (m_xposArray[ind2] < x && ind2 < GAME_LEVEL_GRID_WIDTH)
	ind2++;

    if (ind2 > 0)
        ind1 = ind2-1;

    float m2 = ((x - m_xposArray[ind1]) / (m_xposArray[1] - m_xposArray[0]));
    float height = m_peakArray[ind1] * (1.0f - m2) + m_peakArray[ind2] * m2;

    if (normalTarget) {
        *normalTarget = m_normalArray[ind1] * (1.0f - m2)
                + m_normalArray[ind1] * m2;
    }

    return height;
}


/*!
*/
void GameLevel::explosion(float x, float y, float r)
{
    float distance;
    float dx;
    float dy;

    for (int f = 0; f < GAME_LEVEL_GRID_WIDTH; f++) {
        dx = (x - m_xposArray[f]);
        dy = (y - m_peakArray[f]);
        distance = sqrtf(dx * dx + dy * dy);

        if (distance < r) {
            m_peakArray[f] -= (r - distance);
            m_destroyedArray[f] +=(r - distance) / 2.0f;

            if (m_destroyedArray[f] > 3.0f)
                m_destroyedArray[f] = 3.0f;
        }
    }

    m_forceUpdate = true;
}


/*!
*/
void GameLevel::recreateVertices()
{
    destroy();
    int x;
    int y;
    m_vertexCount = GAME_LEVEL_GRID_WIDTH * GAME_LEVEL_GRID_HEIGHT;
    m_vertices = new GLfloat[m_vertexCount * 12];
    GLfloat *v = m_vertices;

    for (y = 0; y < GAME_LEVEL_GRID_HEIGHT; y++) {
        float u = 0.0f;

        for (x = 0; x < GAME_LEVEL_GRID_WIDTH; x++) {
            v[0] = m_xposArray[x];
            v[2] = zarray[y] + GAME_LEVEL_ZBASE
                   + (float)m_randomArray[GAME_LEVEL_GRID_WIDTH - 1 - x][GAME_LEVEL_GRID_HEIGHT - 1 - y]
                   / 1024.0f * noise_mul[y];
            v[1] = m_peakArray[x] * height_mul[y]
                   + LEVEL_Y_MIN * (1.0f - height_mul[y]);

            if (x > 0) {
                float dx = m_xposArray[x-1] - m_xposArray[x];
                float dy = m_peakArray[x-1] - m_peakArray[x];
                u += 0.1f + sqrtf(dx * dx + dy * dy) / 4.5f;
            }

            v[3] = u + (float)((rand() & 255) - 128) / 2000.0f;
            v[4] = varray[y];
            v[5] = -3.0f;
            v[6] = 3.0f - (float)(rand() & 255) * 6.0f / 255.0f;

            if (y >= GAME_LEVEL_GRID_HEIGHT-2)
                v[8] = 2.0f;
            else
                v[8] = -2.0f;

            v += 12;
        }
    }

    // Indices
    m_indexCount = (GAME_LEVEL_GRID_WIDTH - 1)
            * (GAME_LEVEL_GRID_HEIGHT - 1) * 6;
    m_indices = new GLushort[m_indexCount];
    GLushort *i = m_indices;

    for (y = 0; y < GAME_LEVEL_GRID_HEIGHT - 1; y++) {
        for (x = 0; x < GAME_LEVEL_GRID_WIDTH - 1; x++) {
            // Triangle 1
            i[0] = y * GAME_LEVEL_GRID_WIDTH + x;
            i[1] = y * GAME_LEVEL_GRID_WIDTH + x + 1;
            i[2] = (y + 1) * GAME_LEVEL_GRID_WIDTH + x + 1;

            // Triangle 2
            i[3] = y * GAME_LEVEL_GRID_WIDTH + x;
            i[4] = (y + 1) * GAME_LEVEL_GRID_WIDTH + x + 1;
            i[5] = (y + 1) * GAME_LEVEL_GRID_WIDTH + x;

            i += 6;
        }
    }

    // Update index buffer
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                 m_indexCount * sizeof(GLushort),
                 m_indices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    recreateNormals();
}


/*!
*/
void GameLevel::recreateNormals()
{
    float iw = m_xposArray[1] - m_xposArray[0];
    float nextPeak;

    for (int f = 0; f < GAME_LEVEL_GRID_WIDTH; f++) {
        if (f < GAME_LEVEL_GRID_WIDTH - 1)
            nextPeak = m_peakArray[f + 1];
        else
            nextPeak = m_peakArray[f];

        m_normalArray[f] = QVector3D(-(nextPeak - m_peakArray[f]), iw, 0.0f);
        m_normalArray[f].normalize();
    }
}


/*!
*/
void GameLevel::updateMesh()
{
    int x;
    int y;
    int f;

    // Update the vertices with new information
    GLfloat *v = m_vertices;

    for (y = 0; y < GAME_LEVEL_GRID_HEIGHT; y++) {
        for (x = 0; x < GAME_LEVEL_GRID_WIDTH; x++) {
            // Update the y coordinate of the vertex
            v[1] = m_peakArray[x] * modify_mul[y]
                   + m_originalPeakArray[x] * (1.0f - modify_mul[y]);
            v[1] = v[1] * height_mul[y] + LEVEL_Y_MIN * (1.0f - height_mul[y]);
            v[1] -= (float)m_randomArray[x][y] / 256.0f * noise_mul[y];

            // Update the destroyed - color attribute of the vertex
            v[5] = m_destroyedArray[x] - (1.0f - modify_mul[y]) * 4.0f;
            v += 12;
        }
    }

    v = m_vertices;

    // Clear the vertex normals
    for (f = 0; f < GAME_LEVEL_GRID_WIDTH * GAME_LEVEL_GRID_HEIGHT; f++) {
        v[9] = 0.0f;
        v[10] = 0.0f;
        v[11] = 0.0f;
        v += 12;
    }

    // Loop through all the faces and calculate their normals.
    // Add the face normal to each contributed vertices normal.
    QVector3D tv1;
    QVector3D tv2;
    QVector3D facenormal;
    v = m_vertices;
    GLushort *i = m_indices;
    int faceCount = (GAME_LEVEL_GRID_WIDTH - 1)
            * (GAME_LEVEL_GRID_HEIGHT - 1) * 2;

    for (f = 0; f < faceCount; f++) {
        tv1 = QVector3D(v[i[1] * 12] - v[i[0] * 12], v[i[1] * 12 + 1]
                        - v[i[0] * 12 + 1], v[i[1] * 12 + 2] - v[i[0] * 12 + 2]);
        tv2 = QVector3D(v[i[2] * 12] - v[i[0] * 12], v[i[2] * 12 + 1]
                        - v[i[0] * 12 + 1], v[i[2] * 12 + 2] - v[i[0] * 12 + 2]);
        facenormal = QVector3D::crossProduct(tv2, tv1);
        facenormal.normalize();

        // Distribute normal to each contributed vertex
        m_vertices[i[0] * 12 + 9] += facenormal.x();
        m_vertices[i[0] * 12 + 10] += facenormal.y();
        m_vertices[i[0] * 12 + 11] += facenormal.z();

        m_vertices[i[1] * 12 + 9] += facenormal.x();
        m_vertices[i[1] * 12 + 10] += facenormal.y();
        m_vertices[i[1] * 12 + 11] += facenormal.z();

        m_vertices[i[2] * 12 + 9] += facenormal.x();
        m_vertices[i[2] * 12 + 10] += facenormal.y();
        m_vertices[i[2] * 12 + 11] += facenormal.z();
        i += 3;
    }

    float ftemp;

    // Normalize freshly updated vertex normals
    v = m_vertices;

    for (f = 0; f < GAME_LEVEL_GRID_WIDTH * GAME_LEVEL_GRID_HEIGHT; f++) {
        ftemp = 1.0f / sqrtf(v[9] * v[9] + v[10] * v[10] + v[11] * v[11]);
        v[9] *= ftemp;
        v[10] *= ftemp;
        v[11] *= ftemp;
        v += 12;
    }

    // Update the data in our vbo
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER,
                 m_vertexCount * sizeof(GLfloat) * 12,
                 m_vertices, GL_STATIC_DRAW);
}


/*!
  Returns a noise value using a very simple 3-level perlin noise.
*/
float GameLevel::getNoiseValue(int *rtable, int fx)
{
    int v1,v2;
    int currentBits = 10;
    float rval = 0.0f;
    float amp = 1.0f;

    for (int f = 0; f < 5; f++) {
        int currentAnd = (1 << currentBits) - 1;
        v1 = rtable[(f * 73 + ((f + 5) * 7) * (fx >> currentBits)) & 255];
        v2 = rtable[(f * 73 + ((f + 5) * 7) * ((fx >> currentBits) + 1)) & 255];

        v1 = ((v1 * (currentAnd ^ (fx & currentAnd)))
              + (v2 * (fx & currentAnd))) >> currentBits;
        rval += ((float)v1 / 65536.0f) * amp;
        amp *= 0.5f;   // reduce amplitude
        currentBits--; // increase wavelength
    }

    return rval;
}

