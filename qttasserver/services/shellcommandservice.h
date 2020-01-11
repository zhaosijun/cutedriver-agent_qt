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



#ifndef SHELLCOMMANDSERVICE_H
#define SHELLCOMMANDSERVICE_H

#include <tasconstants.h>
#include <tascoreutils.h>

#include "tasservercommand.h"
class ShellTask;

class ShellCommandService : public QObject, public TasServerCommand
{
    Q_OBJECT
public:
    ShellCommandService();
    ~ShellCommandService();

    /*!
      From ServiceInterface
    */
    bool executeService(TasCommandModel& model, TasResponse& response);
    QString serviceName() const { return SHELL_COMMAND; }
private slots:
    void finished();
private:
    void killTask(qint64 pid, TasResponse& response);
    void shellStatus(qint64 pid, TasResponse& response);
    void shellCommand(QString message, TasResponse& response);
    void detachedShellCommand(QString message, TasResponse& response);
    void shellTask(const QString&  command, TasResponse &response);

    QMap<qint64, ShellTask*> mTasks;
};

#endif
