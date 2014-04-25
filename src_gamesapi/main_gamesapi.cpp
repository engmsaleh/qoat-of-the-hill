/**
 * Copyright (c) 2011-2014 Microsoft Mobile.
 */

#include <QtGui/QApplication>
#include <QtGui/QMainWindow>
#include "mygamewindow_gamesapi.h"

#if defined(Q_WS_X11)
#include <X11/Xlib.h>
#endif

int main(int argc, char *argv[])
{
#if defined(Q_WS_X11)
    XInitThreads();
#endif

    QCoreApplication::setAttribute(Qt::AA_NativeWindows, true);
    QCoreApplication::setAttribute(Qt::AA_ImmediateWidgetCreation, true);

    MyGameApplication app(argc, argv);
    return app.exec();
}
