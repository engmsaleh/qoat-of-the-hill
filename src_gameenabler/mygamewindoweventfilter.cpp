/**
 * Copyright (c) 2011-2014 Microsoft Mobile.
 * Programmed by Tuomo Hirvonen.
 */

#include "mygamewindoweventfilter.h"
#include <QKeyEvent>
#include "trace.h"


bool KeyPressEater::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *keyEvent = static_cast<QKeyEvent *>(event);
        Q_UNUSED(keyEvent); // To prevent compiler warnings.
        DEBUG_INFO("Ate key press" << keyEvent->key());
        return true;
    }

    // Let the event propagate.
    return QObject::eventFilter(obj, event);
}

