/*
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef HDMIOUTPUT_H
#define HDMIOUTPUT_H

#include <QtGui>
#include <gcc_mingw/AccMonitor.h>

namespace GE {

class IHdmiConnectionObserver
{
public:
    virtual void hdmiConnected(bool connected) = 0;
};

class HdmiOutput : public QObject,
                   public MAccMonitorObserver
{
    Q_OBJECT

public:
    explicit HdmiOutput(QObject *parent, IHdmiConnectionObserver *observer);
    ~HdmiOutput();

public:
    void startMonitoringL();
    void stopMonitoring();

    void createWindowL();
    void destroyWindow();
    RWindow *hdmiWindow();

    int width();
    int height();

    // from MAccMonitorObserver
private:
    virtual void ConnectedL(CAccMonitorInfo *accessoryInfo);
    virtual void DisconnectedL(CAccMonitorInfo *accessoryInfo);
    virtual void AccMonitorObserverError(TInt error);

private:
    IHdmiConnectionObserver *m_observer;
    CAccMonitor *m_accMonitor;
    RWsSession m_wsSession;
    CWsScreenDevice *m_screenDevice;
    RWindowGroup *m_windowGroup;
    RWindow *m_window;
};

} // namespace GE

#endif // HDMIOUTPUT_H
