/***************************************************************************
**
** Copyright (C) 2010 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: Nokia Corporation (testabilitydriver@nokia.com)
**
** This file is part of Testability Driver Qt Agent
**
** If you have questions regarding the use of this file, please contact
** Nokia at testabilitydriver@nokia.com .
**
** This library is free software; you can redistribute it and/or
** modify it under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation
** and appearing in the file LICENSE.LGPL included in the packaging
** of this file.
**
****************************************************************************/



#ifndef TASCOMMANDPARSER_H
#define TASCOMMANDPARSER_H

#include <QObject>
#include <QDomElement>
#include <QDomDocument>

#include "tasqtcommandmodel.h"

class TasCommandParser
{
public:
    static TasCommandModel* parseCommandXml(const QString& commandXml);

private:
    static TasTargetObject* parseTargetDetails(QDomElement root);
};


#endif
