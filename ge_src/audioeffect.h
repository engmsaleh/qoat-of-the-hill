/**
 * Copyright (c) 2011 Nokia Corporation.
 *
 * Part of the Qt GameEnabler.
 */

#ifndef AUDIOEFFECT_H
#define AUDIOEFFECT_H

#include <QObject>
#include <QPointer>
#include "geglobal.h"
#include "audiosourceif.h"
#include "audiobuffer.h"

namespace GE {

class Q_GE_EXPORT AudioEffect : public QObject
{
    Q_OBJECT

public:
    explicit AudioEffect(QObject *parent = 0);
    virtual ~AudioEffect();

    AudioEffect *next();
    void linkTo(AudioEffect *next);

public:
    virtual void flush();
    virtual int process(AUDIO_SAMPLE_TYPE *target, int bufferLength);

protected:
    QPointer<GE::AudioEffect> m_next; // Not owned
};

} // namespace GE

#endif // AUDIOEFFECT_H
