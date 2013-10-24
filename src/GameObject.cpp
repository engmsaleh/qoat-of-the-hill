/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "GameObject.h"

#include <math.h>

#include "GameWindow.h"
#include "GameInstance.h"
#include "GameLevel.h"
#include "trace.h"


// Shader for game objects.
const char* strGOFragmentShader =
"uniform sampler2D sampler2d;\n"
"uniform mediump vec4 pcol;\n"
"varying mediump vec2 texCoord;\n"
"void main (void)\n"
"{\n"
"      gl_FragColor = texture2D(sampler2d, texCoord)*vec4(pcol[2], pcol[2], pcol[2], pcol[3]);\n"
"}";

const char* strGOVertexShader =
"attribute highp vec3 vertex;\n"
"attribute highp vec2 uv;\n"
"uniform mediump mat4 transMatrix;\n"
"uniform mediump mat4 projMatrix;\n"
"varying mediump vec2 texCoord;\n"
"uniform mediump vec4 pcol;\n"
"void main(void)\n"
"{\n"
"highp vec4 transVertex = vec4(vertex,1.0)*transMatrix;\n"
"gl_Position = transVertex * projMatrix;\n"
"texCoord = vec2(uv.x*pcol[0], uv.y*pcol[1]);\n"
     //   "texCoord = uv;"
"}";


/*!
  \class GameObject
  \brief A base implementation for all the game objects.
*/


/*!
  Constructor.
*/
GameObject::GameObject(GameInstance *gameInstance)
    : m_gameInstance(gameInstance),
      m_lightness(1.0f),
      m_alpha(1.0f),
      m_depthEnabled(true),
      m_flipX(false),
      m_flipY(false),
      m_run(true),
      m_dead(false),
      m_dieAnimation(0.0f),
      m_onGround(false),
      m_r(0.0f),
      m_aspect(1.0f),
      m_gravity(100.0f),
      m_airFraction(0.5f),
      m_moveOnGround(true),
      m_powerResponse(1.0f),
      m_centerSprite(true)

{
    m_upvector[0] = 0.0f;
    m_upvector[1] = 1.0f;
}


/*!
  Destructor.
*/
GameObject::~GameObject()
{
}


/*!
*/
void GameObject::run(float frameTime)
{
    if (m_dead) {
        m_dieAnimation += frameTime * 4.0f;
        m_alpha = (1.0f - m_dieAnimation);
    }

    if (m_run == false)
        return;

    QVector3D temp = m_dir * frameTime;
    m_pos += temp;

    // Collision
    GameLevel * l = m_gameInstance->getLevel();

    if (!l)
        return;

    float gheight = l->getHeightAndNormalAt(m_pos.x(), &m_groundNormal);

    if (fabsf(m_pos.z()) > 0.8f) {
        float zofs = m_pos.z() - 0.8f;
        gheight -= zofs * zofs * 3.0f;

        if (m_onGround)
            m_dir.setZ(m_dir.z() + zofs);
    }

    // Object is dropping below accepted limit
    if (m_pos.y() < -6.0f)
        die();

    if (m_centerSprite)
        gheight += m_r;

    float speed = m_dir.length();

    if (!m_onGround) {
        m_dir.setY(m_dir.y() - frameTime * m_gravity);
        temp *= -m_airFraction;
        m_dir += temp;

        if (m_pos.y() < gheight) {
            m_pos.setY(gheight);

            if (speed > BOUNCE_SPEED_LIMIT) {
                // Calculate new direction with vector projection
                float fpos =
                        (m_groundNormal.x() * -m_dir.x()
                         + m_groundNormal.y() * -m_dir.y()
                         + m_groundNormal.z() * -m_dir.z()) /
                        (m_groundNormal.x() * m_groundNormal.x()
                         + m_groundNormal.y() * m_groundNormal.y()
                         + m_groundNormal.z() * m_groundNormal.z());

                QVector3D fp = m_groundNormal * fpos;
                m_dir = -((temp - fp) * 2.0f);
                m_dir.normalize();
                m_dir *= speed * 0.5f;
            }
            else {
                m_onGround = true;
            }

            // Notify the object that it has been hit to the ground.
            hit(0, m_pos, m_groundNormal);
        }
    }
    else {
        m_pos.setY(gheight);

        if (m_moveOnGround) {
            m_dir.setX(m_dir.x() + m_groundNormal.x() * frameTime);
            m_dir += (m_dir * -frameTime * 20.0f);
        }

        if (speed < 10.0f) {
            m_dir = QVector3D(0.0f, 0.0f, 0.0f);
        }
    }
}


/*!
*/
void GameObject::render(GameObjectManager *gameObjectMgr, float *m)
{
    GLfloat col[4];

    if (m_flipX)
        col[0] = -1.0f;
    else
        col[0] = 1.0f;

    if (m_flipY)
        col[1] = -1.0f;
    else
        col[1] = 1.0f;

    col[2] = m_lightness;
    col[3] = m_alpha; // General alpha

    int colorLocation = glGetUniformLocation(gameObjectMgr->m_program, "pcol");
    glUniform4fv(colorLocation, 1, col);

    int location = glGetUniformLocation(gameObjectMgr->m_program, "transMatrix");
    glUniformMatrix4fv(location, 1, GL_FALSE, m);

    // Draws the object as quad
    glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
}


/*!
*/
void GameObject::pushForce(QVector3D &pos, float r, float power)
{
    if (m_powerResponse <= 0.0001f)
        return;

    QVector3D temp = m_pos - pos;
    float d = temp.length();
    temp /= d;
    d = (r - d) / r * power * m_powerResponse;

    if (d < 0.0f)
        return;

    m_dir += temp * d;

    if ((rand() & 255) < 128)
        m_dir.setZ(0.0f);

    m_onGround = false;
}


/*!
*/
bool GameObject::isDead()
{
    if (m_dead == true && m_dieAnimation >= 1.0f)
        return true;

    return false;
}



/*!
  \class GameObjectManager
  \brief -
*/


/*!
  Constructor.
*/
GameObjectManager::GameObjectManager(GameInstance *gameInstance)
    : m_gameInstance(gameInstance),
      m_objectList(0)
{

    // Shader
    GLint retval;
    m_fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(m_fragmentShader, 1,
                   (const char**)&strGOFragmentShader, NULL);
    glCompileShader(m_fragmentShader);
    glGetShaderiv(m_fragmentShader, GL_COMPILE_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO COMPILE FRAGMENT SHADER!");
    else
        DEBUG_INFO("Fragment shader compiled successfully!");

    m_vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(m_vertexShader, 1,
                   (const char**)&strGOVertexShader, NULL);
    glCompileShader(m_vertexShader);
    glGetShaderiv(m_vertexShader, GL_COMPILE_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO COMPILE VERTEX SHADER!");
    else
        DEBUG_INFO("Vertex shader compiled successfully!");

    // Program
    m_program = glCreateProgram();
    glAttachShader(m_program, m_fragmentShader);
    glAttachShader(m_program, m_vertexShader);

    // Bind the custom vertex attributes
    glBindAttribLocation(m_program, 0, "vertex");
    glBindAttribLocation(m_program, 1, "uv");

    glLinkProgram(m_program);
    glGetProgramiv(m_program, GL_LINK_STATUS, &retval);

    if (!retval)
        DEBUG_INFO("FAILED TO LINK PROGRAM!");
    else
        DEBUG_INFO("Program linked successfully!");

    glUniform1i(glGetUniformLocation(m_program, "sampler2d"), 0);
    glGenBuffers(1, &m_vbo);

    // Pass the vertex data
    GLfloat vertices[] = { -1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
                           1.0f, -1.0f, 0.0f, 1.0f, 0.0f,
                           1.0f, 1.0f, 0.0f,  1.0f, 1.0f,
                           -1.0f, 1.0f, 0.0f, 0.0f, 1.0f };

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, 4 * sizeof(GLfloat) * 5,
                 vertices, GL_STATIC_DRAW);


}


/*!
  Destructor.
*/
GameObjectManager::~GameObjectManager()
{
    destroyAll();
    glDeleteBuffers(1, &m_vbo);
    glDeleteProgram(m_program);
    glDeleteShader(m_fragmentShader);
    glDeleteShader(m_vertexShader);
}


/*!
*/
void GameObjectManager::run(float frameTime)
{
    GameObject *prev = 0;
    GameObject *l = m_objectList;
    GameObject *next;

    // Run the objects and destoy the dead ones from the list.
    while (l) {
        next = l->m_next;

        if (l->isDead()) {
            if (prev)
                prev->m_next=next;
            else
                m_objectList = next;

            delete l; // Destroy it
        }
        else {
            l->run(frameTime);
            prev = l;
        }

        l = next;
    }

    // Order game objects according their ZPositions
    prev = 0;
    l = m_objectList;

    while (l) {
        next = l->m_next;

        if (!next)
            break;

        if (l->depthEnabled() == false && next->depthEnabled() == false
                && l->pos().z() > next->pos().z()) {
            // Swap objects
            l->m_next = next->m_next;
            next->m_next = l;

            if (prev)
                prev->m_next = next;
            else
                m_objectList = next;

            prev = l;
            l = next;
        }
        else {
            prev = l;
            l = next;
        }
    }
}


/*!
*/
void GameObjectManager::render(bool bgObjects)
{
    bool depthTest(true);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);

    glUseProgram(m_program);
    glUniformMatrix4fv(glGetUniformLocation(m_program, "projMatrix"),
                       1, GL_FALSE, m_gameInstance->getProjectionMatrix());
    glUniform1i(glGetUniformLocation(m_program, "sampler2d"), 0);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA,GL_ONE_MINUS_SRC_ALPHA);

    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(0,3, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5, 0);
    glVertexAttribPointer(1,2, GL_FLOAT, GL_FALSE, sizeof(GLfloat) * 5,
                          (void*)(sizeof(GLfloat) * 3));

    float id[16];
    float m[16];
    memset(id, 0, sizeof(float) * 16);
    id[0] = 1.0f;
    id[5] = 1.0f;
    id[10] = 1.0f;
    id[15] = 1.0f;
    GLuint currentTexture = 90000;
    bool isBgObj;

    GameObject *l = m_objectList;

    while (l) {
        if (l->pos().z() < 0.3f)
            isBgObj = true;
        else
            isBgObj = false;

        if (l->depthEnabled() == false)
            isBgObj = false;

        if ((bgObjects == true && isBgObj == true) ||
            (bgObjects == false && isBgObj == false)) {
            if (l->depthEnabled() != depthTest) {
                if (l->depthEnabled()) {
                    glEnable(GL_DEPTH_TEST);
                }
                else {
                    glDisable(GL_DEPTH_TEST);
                }

                depthTest = l->depthEnabled();
            }

            memcpy(m, id, sizeof(float) * 16);
            m[3] = l->pos().x();
            m[7] = l->pos().y();
            m[11] = l->pos().z() + GAME_LEVEL_ZBASE;

            m[0] = l->getUpVector()[1] * l->r();
            m[1] = l->getUpVector()[0] * l->r();
            m[4] = m[1];
            m[5] = -m[0];

            if (l->isCenterSprite() == false) {
                m[3] -= m[1];
                m[7] += m[0];
            }

            if (l->aspect() != 1.0f) {
                m[4] *= l->aspect();
                m[5] *= l->aspect();
                m[0] *= (1.0f / l->aspect());
                m[1] *= (1.0f / l->aspect());
            }

            if (currentTexture != l->m_textureID) {
                currentTexture = l->m_textureID;
                glBindTexture(GL_TEXTURE_2D, l->m_textureID);
            }

            m_gameInstance->cameraTransform(m);
            l->render(this, m);
        }

        l = l->m_next;
    }

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}


/*!
*/
GameObject *GameObjectManager::addObject(GameObject *object)
{
    object->m_next = 0;

    if (!m_objectList) {
        m_objectList = object;
    }
    else {
        GameObject *l = m_objectList;

        while (l->m_next)
            l = l->m_next;

        l->m_next = object;
    }

    return object;
}


/*!
*/
void GameObjectManager::destroyAll()
{
    GameObject *l = m_objectList;

    while (l) {
        GameObject *n = l->m_next;
        delete l;
        l = n;
    }

    m_objectList = 0;
}


/*!
  Adds push power into the objects.
*/
void GameObjectManager::pushObjects(QVector3D &pos, float r, float power)
{
    GameObject *l = m_objectList;

    while (l) {
        l->pushForce(pos, r, power);
        l = l->m_next;
    }
}

