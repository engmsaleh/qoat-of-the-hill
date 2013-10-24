/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "volumekeys.h"
#include "trace.h"

using namespace GE;

VolumeKeys::VolumeKeys(QObject *parent, IVolumeKeyObserver *observer) :
    QObject(parent),
    m_interfaceSelector(NULL),
    m_coreTarget(NULL),
    m_observer(observer)
{
    DEBUG_INFO(this);

    QT_TRAP_THROWING(
        m_interfaceSelector = CRemConInterfaceSelector::NewL();
        m_coreTarget = CRemConCoreApiTarget::NewL(*m_interfaceSelector, *this);
        m_interfaceSelector->OpenTargetL();
    );
}

VolumeKeys::~VolumeKeys()
{
    DEBUG_POINT;

    delete m_interfaceSelector;
    m_interfaceSelector = NULL;
    m_coreTarget = NULL; // owned by interfaceselector
}

void VolumeKeys::MrccatoCommand(TRemConCoreApiOperationId operationId,
                                TRemConCoreApiButtonAction buttonAct)
{
    DEBUG_INFO("operation:" << operationId << " action:" << buttonAct);

    if (buttonAct == ERemConCoreApiButtonClick) {
        if (operationId == ERemConCoreApiVolumeUp)
            m_observer->onVolumeUp();
        else if (operationId == ERemConCoreApiVolumeDown)
            m_observer->onVolumeDown();
    }
}
