/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "hdmioutput.h"
#include "trace.h"

_LIT(KWgName, "HdmiOutput");
const TUint32 KWgId = 0xeeeeeeee; // TODO: define unique id
const TUint32 KHdScreen = 1;

using namespace GE;

HdmiOutput::HdmiOutput(QObject *parent,
    IHdmiConnectionObserver *observer) :
    QObject(parent),
    m_observer(observer),
    m_accMonitor(NULL),
    m_screenDevice(NULL),
    m_windowGroup(NULL),
    m_window(NULL)
{
    DEBUG_INFO(this);
}

HdmiOutput::~HdmiOutput()
{
    DEBUG_POINT;

    delete m_accMonitor;
    destroyWindow();
}

void HdmiOutput::startMonitoringL()
{
    DEBUG_POINT;

    m_accMonitor = CAccMonitor::NewL();

    RConnectedAccessories array;
    CleanupClosePushL(array);
    m_accMonitor->GetConnectedAccessoriesL(array);

    for (int i = 0; i < array.Count(); i++) {
        if (array[i]->Exists(KAccMonHDMI))
            ConnectedL(array[i]);
    }

    CleanupStack::PopAndDestroy(&array);

    RAccMonCapabilityArray capabilityArray;
    CleanupClosePushL(capabilityArray);
    capabilityArray.Append(KAccMonHDMI);
    m_accMonitor->StartObservingL(this, capabilityArray);
    CleanupStack::PopAndDestroy(&capabilityArray);
}

void HdmiOutput::createWindowL()
{
    DEBUG_POINT;

    if (m_window)
        return;

    m_wsSession.Connect();

    m_screenDevice = new (ELeave) CWsScreenDevice(m_wsSession);
    User::LeaveIfError(m_screenDevice->Construct(KHdScreen));

    m_wsSession.ComputeMode(RWsSession::EPriorityControlDisabled);

    m_windowGroup = new (ELeave) RWindowGroup(m_wsSession);
    User::LeaveIfError(m_windowGroup->Construct(KWgId, m_screenDevice));
    User::LeaveIfError(m_windowGroup->SetName(KWgName));

    m_windowGroup->EnableReceiptOfFocus(EFalse);

    TSize size = m_screenDevice->SizeInPixels();

    m_window = new (ELeave) RWindow(m_wsSession);
    User::LeaveIfError(m_window->Construct(*m_windowGroup, (TUint32)m_window));

    m_window->SetExtent(TPoint(0, 0), size);
    m_window->SetOrdinalPosition(0, ECoeWinPriorityAlwaysAtFront + 1);
    m_window->SetNonFading(ETrue);
    m_window->SetVisible(ETrue);
    m_window->Activate();
}

void HdmiOutput::ConnectedL(CAccMonitorInfo *accessoryInfo)
{
    DEBUG_POINT;

    if (accessoryInfo->Exists(KAccMonHDMI)) {
        if (m_observer)
            m_observer->hdmiConnected(true);
    }
}

void HdmiOutput::DisconnectedL(CAccMonitorInfo *accessoryInfo)
{
    DEBUG_POINT;

    if (accessoryInfo->Exists(KAccMonHDMI)) {
        if (m_observer)
            m_observer->hdmiConnected(false);
    }
}

void HdmiOutput::AccMonitorObserverError(TInt error)
{
    DEBUG_INFO("AccMonitorObserverError:" << error);
}

void HdmiOutput::destroyWindow()
{
    DEBUG_POINT;

    if (!m_window)
        return;

    m_window->Close();
    delete m_window;
    m_window = NULL;

    m_windowGroup->Close();
    delete m_windowGroup;
    m_windowGroup = NULL;

    m_wsSession.Close();
}

RWindow *HdmiOutput::hdmiWindow()
{
    DEBUG_POINT;
    return m_window;
}

int HdmiOutput::width()
{
    DEBUG_POINT;
    return m_screenDevice ? m_screenDevice->SizeInPixels().iWidth : 0;
}

int HdmiOutput::height()
{
    DEBUG_POINT;
    return m_screenDevice ? m_screenDevice->SizeInPixels().iHeight : 0;
}
