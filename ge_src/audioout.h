/**
 * Copyright (c) 2011 Nokia Corporation.
 *
 * Part of the Qt GameEnabler.
 */

#ifndef GEAUDIOOUT_H
#define GEAUDIOOUT_H

#include <QAudioFormat>
#include "audiosourceif.h"

namespace GE {

const QString GEDefaultAudioCodec("audio/pcm");
const QAudioFormat::Endian GEByteOrder(QAudioFormat::LittleEndian);
const QAudioFormat::SampleType GESampleType(QAudioFormat::SignedInt);

class AudioOut
{
public:
    AudioOut() {}
    virtual ~AudioOut() {}

public:
    virtual bool needsManualTick() const { return false; };
    virtual void tick() {};
};

} // namespace GE

#endif // GEAUDIOOUT_H
