/**
 * Copyright (c) 2011-2014 Microsoft Mobile.
 *
 * Part of the Qt GameEnabler.
 */

#include "audiosourceif.h"

using namespace GE;


/*!
  \class AudioSource
  \brief An abstract interface for an audio source.
*/


/*!
  Constructor.
*/
AudioSource::AudioSource(QObject *parent /* = 0 */)
    : QObject(parent)
{
}


/*!
  Destructor.
*/
AudioSource::~AudioSource()
{
}


/*!
  This method is used to check whether this instance can be (auto-)destroyed
  or not.

  To be implemented in the derived class. This default implementation always
  returns false.
*/
bool AudioSource::canBeDestroyed()
{
    return false;
}
