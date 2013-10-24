/**
 * Copyright (c) 2011 Nokia Corporation.
 */


#ifndef GAMELEVEL_H
#define GAMELEVEL_H

#include <QVector3D>
#include <GLES2/gl2.h>

// Size of the level grid
#define GAME_LEVEL_GRID_WIDTH 64
#define GAME_LEVEL_GRID_HEIGHT 5

// Attributes for level vertex creation
const float noise_mul[GAME_LEVEL_GRID_HEIGHT] =
    {0.1f, 0.1f, 0.5f, 0.7f, 0.0f};
const float modify_mul[GAME_LEVEL_GRID_HEIGHT] =
    {1.0f, 1.0f, 0.7f, 0.5f,  0.0f};
const float height_mul[GAME_LEVEL_GRID_HEIGHT] =
    {1.0f, 1.0f, 0.92f, 0.75f, 0.0f};
const float zarray[GAME_LEVEL_GRID_HEIGHT] =
    { -1.5f, 1.5f, 2.0f, 3.0f, 3.2f };
const float varray[GAME_LEVEL_GRID_HEIGHT] =
    { -1.5f/3.0f, 1.5f/3.0f, 3.0f/3.0f, 4.5/3.0, 4.0f };

// Level base size
#define GAME_LEVEL_START_X -29.0f
#define GAME_LEVEL_END_X 29.0f
#define GAME_LEVEL_ZBASE 0.0f

// Forward declarations
class GameInstance;


class GameLevel
{
public:
    GameLevel(GameInstance *gameInstance);
    ~GameLevel();

public:
    void recreate();
    void destroy();
    void run(float frameTime);
    void render();
    float getHeightAndNormalAt(float x, QVector3D *normalTarget);
    void explosion(float x, float y, float r);

protected:
    void recreateVertices();
    void recreateNormals();
    void updateMesh();

private:
    float getNoiseValue(int *rtable, int fx);

protected: // Data
    GameInstance *m_gameInstance;
    QVector3D m_normalArray[GAME_LEVEL_GRID_WIDTH];
    char m_randomArray[GAME_LEVEL_GRID_WIDTH][GAME_LEVEL_GRID_HEIGHT];
    float m_peakArray[GAME_LEVEL_GRID_WIDTH];
    float m_originalPeakArray[GAME_LEVEL_GRID_WIDTH];
    float m_destroyedArray[GAME_LEVEL_GRID_WIDTH];
    float m_xposArray[GAME_LEVEL_GRID_WIDTH + 1];
    GLfloat *m_vertices;
    GLushort *m_indices;
    int m_vertexCount;
    int m_indexCount;
    bool m_forceUpdate;
    GLint m_rockProgram;
    GLint m_rockFragmentShader;
    GLint m_program;
    GLint m_fragmentShader;
    GLint m_vertexShader;
    GLuint m_vbo;
    GLuint m_indexBuffer;
};


#endif // GAMELEVEL_H
