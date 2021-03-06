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



#ifndef TASPLUGINLOADER_H
#define TASPLUGINLOADER_H

#include <QObject>
#include <QHash>
#include <QStringList>
#include "tasconstants.h"
#include "tasqtfixtureplugininterface.h"

class TAS_EXPORT TasPluginLoader
{

public:
   TasPluginLoader();
   ~TasPluginLoader();

   TasFixturePluginInterface* loadFixtureInterface(const QString& pluginPath);
   QObject* loadPlugin(QString pluginLoadPath);
   void initializeFixturePlugins();

   //protected:
   TasFixturePluginInterface* tryToLoadFixture(QString filePath, QString id);
   QStringList listPlugins(QString pluginDir);


private:
   QHash<QString, TasFixturePluginInterface*> mFixturePlugins;

};

#endif

