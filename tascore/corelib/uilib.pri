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

PUBLIC_HEADERS += $$PWD/tastraverserloader.h
PUBLIC_HEADERS += $$PWD/tastraverseutils.h
PUBLIC_HEADERS += $$PWD/tasuitraverser.h
PUBLIC_HEADERS += $$PWD/testabilityutils.h

HEADERS += $$PWD/tastraverseinterface.h
HEADERS += $$PWD/tasservicebase.h
HEADERS += $$PWD/tasservicemanager.h
HEADERS += $$PWD/testabilityservice.h

SOURCES += $$PWD/testabilityservice.cpp
SOURCES += $$PWD/testabilityutils.cpp
SOURCES += $$PWD/tastraverserloader.cpp
SOURCES += $$PWD/tasuitraverser.cpp
SOURCES += $$PWD/tastraverseutils.cpp
