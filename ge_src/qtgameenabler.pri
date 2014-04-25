# Copyright (c) 2011-2014 Microsoft Mobile.

# Comment the following line out for better performance. Using the definition
# enables debug logging which is convenient for locating problems in the code
# but is also very costly in terms of performance.
#DEFINES += GE_DEBUG

INCLUDEPATH += $$PWD

include(./qtgameenableraudio.pri)

HEADERS += $$PWD/GameWindow.h
SOURCES += $$PWD/GameWindow.cpp

symbian {
    LIBS += -llibEGL -llibGLESv2 -lcone -leikcore -lavkon

    # For HD output
    LIBS += -lws32 -laccmonitor

    HEADERS += $$PWD/hdmioutput.h
    SOURCES += $$PWD/hdmioutput.cpp
}

unix:!symbian {
    LIBS += -lX11 -lEGL -lGLESv2

    maemo5 {
        # Maemo 5 specific
    }
    else {
        contains(MEEGO_EDITION, harmattan) {
            # Harmattan specific
            QT += meegographicssystemhelper
            CONFIG += mobility
            MOBILITY += systeminfo
        }
        else {
            # Unix based desktop specific
            INCLUDEPATH += ../SDKPackage_OGLES2/Builds/OGLES2/Include
            LIBS += -L../SDKPackage_OGLES2/Builds/OGLES2/LinuxPC/Lib
        }
    }
}

windows {
    INCLUDEPATH += /PowerVRSDK/Builds/OGLES2/Include
    LIBS += -L/PowerVRSDK/Builds/OGLES2/WindowsPC/Lib

    LIBS += -llibEGL -llibGLESv2
}

message($$INCLUDEPATH)
message($$LIBS)
message($$MOBILITY)

# End of file.
