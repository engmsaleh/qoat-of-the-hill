# Copyright (c) 2011-2014 Microsoft Mobile.

INCLUDEPATH += $$PWD

HEADERS  += \
    $$PWD/trace.h \
    $$PWD/geglobal.h \
    $$PWD/audiobuffer.h \
    $$PWD/audiobufferplayinstance.h \
    $$PWD/audiomixer.h \
    $$PWD/audioout.h \
    $$PWD/pushaudioout.h \
    $$PWD/pullaudioout.h \
    $$PWD/audiosourceif.h \
    $$PWD/audioeffect.h \
    $$PWD/echoeffect.h \
    $$PWD/cutoffeffect.h

SOURCES += \
    $$PWD/audiobuffer.cpp \
    $$PWD/audiobufferplayinstance.cpp \
    $$PWD/audiomixer.cpp \
    $$PWD/pushaudioout.cpp \
    $$PWD/pullaudioout.cpp \
    $$PWD/audiosourceif.cpp \
    $$PWD/audioeffect.cpp \
    $$PWD/echoeffect.cpp \
    $$PWD/cutoffeffect.cpp


symbian {
    message(Symbian build)

    CONFIG += mobility
    MOBILITY += multimedia systeminfo

    # For volume keys
    LIBS += -lremconcoreapi -lremconinterfacebase

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
    #DEFINES += QTGAMEENABLER_USE_VOLUME_HACK

    contains(DEFINES, QTGAMEENABLER_USE_VOLUME_HACK) {
        # Include paths and libraries required for the volume hack.
        message(Symbian volume hack enabled)
        INCLUDEPATH += /epoc32/include/mmf/common
        INCLUDEPATH += /epoc32/include/mmf/server
        LIBS += -lmmfdevsound
    }

    DEFINES += GE_PULLMODEAUDIO

    INCLUDEPATH += /epoc32/include/mw
    HEADERS += $$PWD/volumekeys.h
    SOURCES += $$PWD/volumekeys.cpp
}

unix:!symbian {
    maemo5 {
        message(Maemo 5 build)

        QT += multimedia
        CONFIG += mobility
        MOBILITY += systeminfo
    } else {
        contains(MEEGO_EDITION, harmattan) {
            message(Harmattan build)

            DEFINES += MEEGO_EDITION_HARMATTAN
            CONFIG += mobility
            MOBILITY += multimedia
        }
        else {
            message(Unix based desktop build)

            CONFIG += mobility
            MOBILITY += multimedia systeminfo
            #DEFINES += GE_PULLMODEAUDIO
        }
    }
}

windows {
    message(Windows desktop build)

    QT += multimedia
    DEFINES += GE_NOMOBILITY
}

# End of file.
