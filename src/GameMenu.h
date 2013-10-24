/**
 * Copyright (c) 2011 Nokia Corporation.
 */


#ifndef GAMEMENU_H
#define GAMEMENU_H

#include <GLES2/gl2.h>

// Forward declarations
class GameInstance;


class GameMenu
{
public:
    GameMenu(GameInstance *gameInstance,
             int logoIndex,
             int button1Index,
             int button2Index);
    ~GameMenu();

public:
    bool run(float frameTime);
    void render();
    void select(int button); // "press" button

protected:
    void drawText(GLuint program,
                  float x,
                  float y,
                  float scale,
                  float angle,
                  int index);

protected: // Data
    GameInstance *m_gameInstance;
    GLuint m_textTexture;
    int m_logoIndex;
    int m_button1Index;
    int m_button2Index;
    int m_selected;
    float m_selectedCounter;
    float m_counter;
    bool m_finished;
};


#endif // GAMEMENU_H
