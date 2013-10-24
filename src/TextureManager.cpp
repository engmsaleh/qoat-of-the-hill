/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "TextureManager.h"
#include "GameInstance.h"


/*!
  \class TextureManager
  \brief -
*/


/*!
  Constructor.
*/
TextureManager::TextureManager()
    : m_list(0)
{
}


/*!
  Destructor.
*/
TextureManager::~TextureManager()
{
    releaseAll();
}


/*!
  Frees all allocated resources.
*/
void TextureManager::releaseAll()
{
    TextureManager::STextureCapsule *l = m_list;

    while (l) {
        TextureManager::STextureCapsule *n = l->next;

        if (l->name)
            delete [] l->name;

        glDeleteTextures(1, &l->textureID);
        delete l;
        l = n;
    }

    m_list = 0;
}



/*!
  Returns a texture with \a name or 0 in case of an error.
*/
GLuint TextureManager::getTexture(const char *name)
{
    if (!name || name[0] == 0)
        return 0;

    TextureManager::STextureCapsule *l = m_list;

    while (l) {
        if (!strcmp(l->name, name))
            return l->textureID;

        l = l->next;
    }

    TextureManager::STextureCapsule *ncap =
            new TextureManager::STextureCapsule;

    ncap->next = m_list;
    m_list = ncap;
    int nlen = strlen(name);
    ncap->name = new char[nlen +1];
    memcpy(ncap->name, name, nlen);
    ncap->name[nlen] = 0;
    ncap->textureID = GameInstance::loadGLTexture(QString(name));

    return ncap->textureID;
}

