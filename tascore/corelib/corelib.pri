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

PUBLIC_HEADERS += $$PWD/tasconstants.h
PUBLIC_HEADERS += $$PWD/tascoreutils.h
PUBLIC_HEADERS += $$PWD/taslogger.h
PUBLIC_HEADERS += $$PWD/tasqtcommandmodel.h
PUBLIC_HEADERS += $$PWD/tasqtdatamodel.h
PUBLIC_HEADERS += $$PWD/tasqtfixtureplugininterface.h
PUBLIC_HEADERS += $$PWD/tastraverseinterface.h
PUBLIC_HEADERS += $$PWD/taspluginloader.h
PUBLIC_HEADERS += $$PWD/tasextensioninterface.h
PUBLIC_HEADERS += $$PWD/infologger.h
PUBLIC_HEADERS += $$PWD/testabilityinterface.h
!symbian: {
#PUBLIC_HEADERS += $$PWD/cucumberutils.h
#PUBLIC_HEADERS += $$PWD/cucumberstepdata.h
}

HEADERS += $$PWD/tasmessages.h
HEADERS += $$PWD/testabilitysettings.h
HEADERS += $$PWD/tasdatashare.h
HEADERS += $$PWD/tascommand.h
HEADERS += $$PWD/tasfixtureplugininterface.h
HEADERS += $$PWD/tasqtfixtureplugininterface.h
HEADERS += $$PWD/tasservicemanager.h
HEADERS += $$PWD/tasxmlwriter.h
HEADERS += $$PWD/tassocket.h
HEADERS += $$PWD/tasdeviceutils.h
HEADERS += $$PWD/taspointercache.h
#HEADERS += $$PWD/tascommandparser.h

SOURCES += $$PWD/tasmessages.cpp
!symbian: {
#SOURCES += $$PWD/cucumberutils.cpp
}
#SOURCES += $$PWD/tascommandparser.cpp
SOURCES += $$PWD/taslogger.cpp
SOURCES += $$PWD/taspluginloader.cpp
SOURCES += $$PWD/tasqtcommandmodel.cpp
SOURCES += $$PWD/tasqtdatamodel.cpp
SOURCES += $$PWD/tasservicemanager.cpp
SOURCES += $$PWD/tassocket.cpp
SOURCES += $$PWD/tasxmlwriter.cpp
SOURCES += $$PWD/tasdatashare.cpp
SOURCES += $$PWD/testabilitysettings.cpp
SOURCES += $$PWD/tascoreutils.cpp
SOURCES += $$PWD/infologger.cpp
SOURCES += $$PWD/taspointercache.cpp

unix: {
    macx|CONFIG(no_x):{
      SOURCES += $$PWD/tasdeviceutils.cpp
    }
    else{
      SOURCES += $$PWD/tasdeviceutils_unix.cpp
    }
}

win32: {
    SOURCES += $$PWD/tasdeviceutils_win.cpp
    LIBS += -lPsapi
    LIBS +=	-lUser32
}

unix:!macx:!CONFIG(no_x):!wayland {
    LIBS += -lX11 -lXtst
}

unix:!macx {
    LIBS += -lrt
}
