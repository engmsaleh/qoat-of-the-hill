/**
 * Copyright (c) 2011-2014 Microsoft Mobile.
 * Programmed by Tuomo Hirvonen.
 */

#include <QtGui/QApplication>
#include <QPaintEngine>

#ifdef Q_OS_SYMBIAN
    #include <eikenv.h>
    #include <eikappui.h>
    #include <aknenv.h>
    #include <aknappui.h>
#endif

#include "gamewindow.h"
#include "mygamewindow.h"
#include "mygamewindoweventfilter.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

#ifdef Q_OS_SYMBIAN
    // Lock orientation to landscape.
    CAknAppUi* appUi = dynamic_cast<CAknAppUi*>( CEikonEnv::Static()->AppUi() );
    TRAPD( error,
        if (appUi) {
            appUi->SetOrientationL( CAknAppUi::EAppUiOrientationLandscape );
        }
    );

    Q_UNUSED(error);
#endif

    GE::GameWindow *gameWindow = new MyGameWindow();
    gameWindow->create();
    gameWindow->setWindowState(Qt::WindowNoState);

    qApp->installEventFilter(gameWindow);

#ifdef Q_OS_WIN32
    gameWindow->showMaximized();
#else
    gameWindow->showFullScreen();
#endif

    int result = app.exec();
    gameWindow->destroy();
    delete gameWindow;
    return result;
}
