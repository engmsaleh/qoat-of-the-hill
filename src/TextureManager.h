/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef TEXTUREMANAGER_H
#define TEXTUREMANAGER_H

#include <GLES2/gl2.h>


class TextureManager
{
public: // Data types

    struct STextureCapsule {
        char *name;
        GLuint textureID;
        STextureCapsule *next;
    };

public:
    TextureManager();
    ~TextureManager();

public:
    void releaseAll();
    GLuint getTexture(const char *name);

public: // Data
    STextureCapsule *m_list;
};


#endif // TEXTUREMANAGER_H
