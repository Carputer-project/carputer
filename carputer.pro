QT += core gui gui-private qml quick network serialport concurrent multimedia
CONFIG += c++17

# GStreamer
PKGCONFIG += gstreamer-1.0 gstreamer-app-1.0 gstreamer-pbutils-1.0
CONFIG += link_pkgconfig

TARGET = carputer
TEMPLATE = app

CONFIG(debug, debug|release) {
    DEFINES += QT_QML_DEBUG
    CONFIG += warn_on
}



SOURCES += \
    main.cpp \
    systemmanager.cpp \
    configmanager.cpp \
    thememanager.cpp \
    diagnosticmanager.cpp \
    remotemanager.cpp \
    updatemanager.cpp \
    mediamanager.cpp \
    artworkprovider.cpp \
    videoframeprovider.cpp \
    dvrmanager.cpp \
    carplaymanager.cpp \
    carcontrolmanager.cpp \
    cameramanager.cpp \
    sensormanager.cpp \
    audiomanager.cpp \
    internalwifimanager.cpp \
    debugmanager.cpp \
    installmanager.cpp \
    dtcmanager.cpp \
    tripcomputer.cpp \
    datalogger.cpp \
    engineprofilemanager.cpp

HEADERS += \
    systemmanager.h \
    configmanager.h \
    thememanager.h \
    diagnosticmanager.h \
    remotemanager.h \
    updatemanager.h \
    mediamanager.h \
    artworkprovider.h \
    videoframeprovider.h \
    dvrmanager.h \
    carplaymanager.h \
    carcontrolmanager.h \
    cameramanager.h \
    sensormanager.h \
    audiomanager.h \
    internalwifimanager.h \
    debugmanager.h \
    installmanager.h \
    dtcmanager.h \
    tripcomputer.h \
    datalogger.h \
    engineprofilemanager.h

RESOURCES += qml.qrc

target.path = /usr/bin
INSTALLS += target
INSTALLS += systemd