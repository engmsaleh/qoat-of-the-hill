# Copyright (c) 2011-2014 Microsoft Mobile.

QT += core

TARGET = qoatofthehill
TEMPLATE = app
VERSION = 1.3

#INCLUDEPATH += ge_src

include(./ge_src/qtgameenabler.pri)

INCLUDEPATH += src_gameenabler
INCLUDEPATH += src

SOURCES += \
    src/GameLevel.cpp \
    src/GameMenu.cpp \
    src/GameObject.cpp \
    src/GamePlayer.cpp \
    src/ParticleEngine.cpp \
    src/TextureManager.cpp \
    src_gameenabler/GameInstance.cpp \
    src_gameenabler/main.cpp \
    src_gameenabler/mygamewindow.cpp \
    src_gameenabler/mygamewindoweventfilter.cpp

HEADERS  += \
    src/GameLevel.h \
    src/GameMenu.h \
    src/GameObject.h \
    src/GamePlayer.h \
    src/ParticleEngine.h \
    src/TextureManager.h \
    src_gameenabler/GameInstance.h \
    src_gameenabler/mygamewindow.h \
    src_gameenabler/mygamewindoweventfilter.h

OTHER_FILES +=

RESOURCES += \
    images/images.qrc \
    sounds/sounds.qrc
    
CONFIG += mobility

# Uncomment the following for Qt GameEnabler's debug prints.
#DEFINES += GE_DEBUG


symbian: {
    message(Symbian build)

    TARGET = QoatOfTheHill
    TARGET.UID3 = 0xee69dadb
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000

    MOBILITY = systeminfo

    ICON = icons/qoatofthehill.svg

    # For enabling hardware floats.
    MMP_RULES += "OPTION gcce -march=armv6"
    MMP_RULES += "OPTION gcce -mfpu=vfp"
    MMP_RULES += "OPTION gcce -mfloat-abi=softfp"
    MMP_RULES += "OPTION gcce -marm"

    backup.sources = backup_registration.xml
    backup.path = !:/private/EE69DADB

    # For checking the current profile.
    LIBS += -lcentralrepository

    LIBS += -llibEGL -llibGLESv2 -lcone -leikcore -lavkon

    # Use the Qt multimedia for now.
    QT += multimedia

    message($$INCLUDEPATH)
    message($$LIBS)
    message($$DEFINES)
    message($$QT)
}


# Unix based platforms
unix:!symbian {
    BINDIR = /opt/usr/bin
    DATADIR = /usr/share

    icon64.files += icons/qoatofthehill.png
    icon64.path = $$DATADIR/icons/hicolor/64x64/apps

    LIBS += -lX11 -lEGL -lGLESv2

    maemo5 {
        # Maemo 5 specific
        message(Maemo 5 build)
        QT += multimedia

        target.path = $$BINDIR

        desktopfile.path = $$DATADIR/applications/hildon
        desktopfile.files += qtc_packaging/debian_fremantle/$${TARGET}.desktop

        INSTALLS += \
            desktopfile \
            icon64
    } else {
        contains(DEFINES, DESKTOP) {
            # Unix based desktop specific
            message(Unix based desktop build)
            QT += multimedia

            target.path = /usr/local/bin

            INCLUDEPATH += ../SDKPackage_OGLES2/Builds/OGLES2/Include
            LIBS += -L../SDKPackage_OGLES2/Builds/OGLES2/LinuxPC/Lib

            INCLUDEPATH += $(HOME)/Downloads/qt-mobility-opensource-src-1.1.0/install/include
            INCLUDEPATH += $(HOME)/Downloads/qt-mobility-opensource-src-1.1.0/install/include/QtMultimedia
            LIBS += -L$(HOME)/Downloads/qt-mobility-opensource-src-1.1.0/install/lib
        }
        else {
            # Harmattan specific
            message(Harmattan build)
            CONFIG += mobility
            MOBILITY += multimedia systeminfo

            target.path = $$BINDIR

            desktopfile.files = qtc_packaging/debian_harmattan/$${TARGET}.desktop
            desktopfile.path = /usr/share/applications

            # Classify the application as a game to support volume keys on Harmattan.
            gameclassify.files += qtc_packaging/debian_harmattan/$${TARGET}.conf
            gameclassify.path = /usr/share/policy/etc/syspart.conf.d

            INSTALLS += \
                desktopfile \
                gameclassify \
                icon64
        }
    }

    INSTALLS += target

    message($$INCLUDEPATH)
    message($$LIBS)
}


windows: {
    message(Windows desktop build)
    QT += multimedia

    TARGET = QoatOfTheHill

    INCLUDEPATH += /PowerVRSDK/Builds/OGLES2/Include
    LIBS += -L/PowerVRSDK/Builds/OGLES2/WindowsPC/Lib

    message($$INCLUDEPATH)
    message($$LIBS)

    LIBS += -llibEGL -llibGLESv2
}

# End of file.
