############################################################################
##
## Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
## All rights reserved.
## Contact: Nokia Corporation (testabilitydriver@nokia.com)
##
## This file is part of Testability Driver Qt Agent
##
## If you have questions regarding the use of this file, please contact
## Nokia at testabilitydriver@nokia.com .
##
## This library is free software; you can redistribute it and/or
## modify it under the terms of the GNU Lesser General Public
## License version 2.1 as published by the Free Software Foundation
## and appearing in the file LICENSE.LGPL included in the packaging
## of this file.
##
############################################################################



TEMPLATE = lib
TARGET = scenegraphtraverse
CONFIG += plugin

include(../../tasbase.pri)

target.path = $$TAS_TARGET_PLUGIN/traversers

DEPENDPATH += .
INCLUDEPATH += . ../../tascore/corelib


isEmpty(QT_PRIVATE_HEADERS) {
    QT_PRIVATE_HEADERS = $$[QT_INSTALL_HEADERS]
}


QT_PRIVATE_HEADERS = $$[QT_INSTALL_HEADERS]
defined(use_qml_private) {
QT += qml-private
DEFINES += USE_QTQML_PRIVATE_HEADERS
}

# Input
HEADERS += scenegraphtraverse.h
SOURCES += scenegraphtraverse.cpp
OTHER_FILES += scenegraphtraverse.json

QT += xml widgets qml quick

DESTDIR = lib

LIBS += -L../../tascore/lib/ -lqttestability

INSTALLS += target
