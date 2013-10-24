/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef VOLUMEKEYS_H
#define VOLUMEKEYS_H

#include <QtGui>
#include <remconcoreapitargetobserver.h>
#include <remconcoreapitarget.h>
#include <remconinterfaceselector.h>

namespace GE {

class IVolumeKeyObserver
{
public:
    virtual void onVolumeUp() = 0;
    virtual void onVolumeDown() = 0;
};

class VolumeKeys : public QObject,
                   public MRemConCoreApiTargetObserver
{
    Q_OBJECT

public:
    explicit VolumeKeys(QObject *parent, IVolumeKeyObserver *observer);
    ~VolumeKeys();

private: // from MRemConCoreApiTargetObserver
    void MrccatoCommand(TRemConCoreApiOperationId operationId,
        TRemConCoreApiButtonAction buttonAct);

private:
    CRemConInterfaceSelector *m_interfaceSelector;
    CRemConCoreApiTarget *m_coreTarget;
    IVolumeKeyObserver *m_observer;
};

} // namespace GE

#endif // VOLUMEKEYS_H
