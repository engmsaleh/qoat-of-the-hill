# Copyright (c) 2011 Nokia Corporation.

QT += core
CONFIG += mobility

TARGET = qoatofthehill_gamesapi
TEMPLATE = app
VERSION = 1.2

include (./ge_src/qtgameenableraudio.pri)

INCLUDEPATH += \
    src \
    src_gamesapi

SOURCES += \
    src/GameLevel.cpp \
    src/GameMenu.cpp \
    src/GameObject.cpp \
    src/GamePlayer.cpp \
    src/ParticleEngine.cpp \
    src/TextureManager.cpp \
    src_gamesapi/GameInstance.cpp \
    src_gamesapi/main_gamesapi.cpp \
    src_gamesapi/mygamewindow_gamesapi.cpp \
    src_gamesapi/mygamewindoweventfilter_gamesapi.cpp \
    src_gamesapi/GameWindow.cpp

HEADERS  += \
    src/GameMenu.h \
    src/GameLevel.h \
    src/GameObject.h \
    src/GamePlayer.h \
    src/ParticleEngine.h \
    src/TextureManager.h \
    src_gamesapi/GameInstance.h \
    src_gamesapi/mygamewindow_gamesapi.h \
    src_gamesapi/mygamewindoweventfilter_gamesapi.h \
    src_gamesapi/GameWindow.h

RESOURCES += \
    images/images.qrc \
    sounds/sounds.qrc

# Uncomment the following for Qt GameEnabler's debug prints.
#DEFINES += GE_DEBUG

symbian: {
    message(Symbian build)

    TARGET = QoatOfTheHill
    TARGET.UID3 = 0xee69dadb
    #TARGET.CAPABILITY +=
    TARGET.EPOCSTACKSIZE = 0x14000
    TARGET.EPOCHEAPSIZE = 0x020000 0x800000

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

    # Uncomment the following define to enable a very ugly hack to set the
    # volume level on Symbian devices higher. By default, on Symbian, the volume
    # level is very low when audio is played using QAudioOutput class. For now,
    # this ugly hack is the only way to set the volume louder.
    #
    # WARNING: The volume hack (see the GEAudioOut.cpp file) is VERY DANGEROUS
    # because the data to access the volume interface is retrieved manually with
    # pointer manipulation. Should the library, in which the interface is
    # implemented, be modified even a tiny bit, the application using this hack
    # might crash.
    #
    DEFINES += QTGAMEENABLER_USE_VOLUME_HACK

    # NOTE
    # The following lines use the multimedia of Qt Mobility instead of the
    # multimedia of the plain Qt. Warning: Enabling the multimedia of Qt
    # mobility will make QAudioOutput volume hack useless (due to the fact that
    # the libraries have been changed)!
    #CONFIG += mobility
    #MOBILITY += multimedia

    # Use the Qt multimedia for now.
    QT += multimedia

    contains(DEFINES, QTGAMEENABLER_USE_VOLUME_HACK) {
        # Include paths and libraries required for the volume hack.
        message(Symbian volume hack enabled)
        INCLUDEPATH += /epoc32/include/mmf/common
        INCLUDEPATH += /epoc32/include/mmf/server
        LIBS += -lmmfdevsound
    }

    message($$INCLUDEPATH)
    message($$LIBS)
    message($$DEFINES)
    message($$QT)
}

contains(MEEGO_EDITION, harmattan) {
    message(Harmattan build)
    MOBILITY += multimedia
    LIBS += -lX11 -lEGL -lGLESv2 -lQGameOpenGLES2

    BINDIR = /opt/usr/bin
    DATADIR = /usr/share

    target.path = $$BINDIR
    desktopfile.files = qtc_packaging/debian_harmattan/$${TARGET}.desktop
    desktopfile.path = /usr/share/applications
    icon64.files += icons/qoatofthehill.png
    icon64.path = $$DATADIR/icons/hicolor/64x64/apps

    # Classify the application as a game to support volume keys on Harmattan.
    gameclassify.files += qtc_packaging/debian_harmattan/$${TARGET}.conf
    gameclassify.path = /usr/share/policy/etc/syspart.conf.d

    INSTALLS += \
        target \
        desktopfile \
        icon64 \
        gameclassify

    message($$LIBS)
}


# End of file.













