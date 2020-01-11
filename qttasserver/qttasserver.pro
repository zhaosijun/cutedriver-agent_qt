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



TEMPLATE = app

mac {
   CONFIG -= app_bundle
}


include(../tasbase.pri)

TARGET = qttasserver
DESTDIR = bin
target.path = $$TAS_TARGET_BIN
DEFINES += TDTASSERVER

win32: {
    LIBS += -lUser32
    LIBS += -lKernel32
}

INCLUDEPATH += .
INCLUDEPATH += services corelib

INCLUDEPATH += ../tascore/corelib

DEPENDPATH += . inc src services corelib

# Input
SOURCES += main.cpp

include(corelib/corelib.pri)
include(services/services.pri)

#include(cucumber_wireprotocol/cucumber_wireprotocol.pri)

include(../tascore/corelib/corelib.pri)

HEADERS += $$PUBLIC_HEADERS

QT -= gui
QT += network xml

unix: {
        QT += testlib
}

wayland {
DEFINES += TAS_WAYLAND
}

debian {
  systemd_service.files=contrib/qttas-server.service
  systemd_service.path=/lib/systemd/system
  INSTALLS += systemd_service
}

INSTALLS += target
