/**
 * Copyright (c) 2011 Nokia Corporation.
 */

#include "mygamewindoweventfilter_gamesapi.h"
#include <QMouseEvent>
#include "mygamewindow_gamesapi.h"


/*!
  \class OurEventFilter
  \brief Game event filter.
*/


/*!
  Constructor.
*/
OurEventFilter::OurEventFilter(MyGameApplication *receiver, QObject *parent)
    : QObject(parent),
      m_eventReceiver(receiver)
{

}


/*!
  Handles \a event.
*/
bool OurEventFilter::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type())
    {
        case (QEvent::MouseButtonPress):
        {
            QMouseEvent *tt = static_cast<QMouseEvent*>(event);
            m_eventReceiver->mousePressEvent(tt);
            return true;
        }
        case (QEvent::MouseMove):
        {
            QMouseEvent *tt = static_cast<QMouseEvent*>(event);
            m_eventReceiver->mouseMoveEvent(tt);
            return true;
        }
        case(QEvent::MouseButtonRelease):
        {

            QMouseEvent *tt = static_cast<QMouseEvent*>(event);
            m_eventReceiver->mouseReleaseEvent(tt);
            return true;
        }
        default:
            // standard event processing
            return obj->eventFilter(obj, event);
     }
}
