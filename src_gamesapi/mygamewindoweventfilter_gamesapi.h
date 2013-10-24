/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#ifndef MYGAMEWINDOWEVENTFILTER_H
#define MYGAMEWINDOWEVENTFILTER_H

#include <QObject>

// Forward declarations
class MyGameApplication;


class OurEventFilter : public QObject
{
     Q_OBJECT
public:
    explicit OurEventFilter(MyGameApplication *receiver, QObject *parent = 0);

protected:
    bool eventFilter(QObject *obj, QEvent *event);

protected: // Data
    MyGameApplication *m_eventReceiver;
};

#endif // MYGAMEWINDOWEVENTFILTER_H
