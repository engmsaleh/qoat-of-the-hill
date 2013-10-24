/**
 * Copyright (c) 2011 Nokia Corporation.
 * Programmed by Tuomo Hirvonen.
 */

#ifndef MYGAMEWINDOWEVENTFILTER_H
#define MYGAMEWINDOWEVENTFILTER_H

#include <QObject>


class KeyPressEater : public QObject
 {
     Q_OBJECT

 protected:
     bool eventFilter(QObject *obj, QEvent *event);
 };


#endif // MYGAMEWINDOWEVENTFILTER_H
