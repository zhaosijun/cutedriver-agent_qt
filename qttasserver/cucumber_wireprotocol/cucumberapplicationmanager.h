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

#ifndef APPLICATIONINSTANCE_H
#define APPLICATIONINSTANCE_H

#include <QObject>
#include <QMap>
#include <QVariant>
#include <QStringList>
#include <QVariantList>
#include <QWeakPointer>

#include <cucumberstepdata.h>

class QTimer;

class CucumberApplicationManager;

typedef void (CucumberApplicationManager::*InvokableStepFn)(const QString &, const QVariantList &, QObject *);

class CucumberApplicationManager : public QObject
{
    Q_OBJECT


public:

    Q_INVOKABLE void pluginResultCB(bool success, const QString &text, quintptr callId);


    Q_INVOKABLE void callStepInPlugin(const QString &regExpPattern, const QVariantList &args, QObject *sender);

    //Q_INVOKABLE void callScriptMethod(const QString &regExpPattern, const QVariantList &args, QObject *sender);
    //Q_INVOKABLE void checkScriptProperty(const QString &regExpPattern, const QVariantList &args, QObject *sender);

    Q_INVOKABLE void selectApp(const QString &regExpPattern, const QVariantList &args, QObject *sender);
    Q_INVOKABLE void attachApp(const QString &regExpPattern, const QVariantList &args, QObject *sender);
    Q_INVOKABLE void startApp(const QString &regExpPattern, const QVariantList &args, QObject *sender);
    Q_INVOKABLE void startAppTable(const QString &regExpPattern, const QVariantList &args, QObject *sender);

    explicit CucumberApplicationManager(QObject *parent = 0);

    void registerSteps(QObject *registrarObject, const char *method);
    // method is of type:
    // void method(const QRegExp &regExp, QObject *object, const char *method, const char *sourceFile, int sourceLine);

    void beginScenario();
    void endScenario();

signals:

private slots:
    void doRetryTimer();
    void pendingSenderTimeout();


    //void applicationRegistered();

private:

    void checkPendingSender();

    QString doStartOrWaitApp(const QString &id, const QString &program, const QStringList &arguments);
    void doReplyOrRetry(InvokableStepFn fn, const QString &errorString,
                        const QString &regExpPattern, const QVariantList &args, QObject *sender);
    bool invokePlainSender(QObject *sender, const QString &errorMsg);

    QMap<QString, QString> mPidMap; // maps application id to it's "pid" as received from StartAppService
    QString mCurrentApplicationId;
    QString mWorkingDirectoryPath;
    QStringList mStartEnvironment;

    struct StepRetryData {
        InvokableStepFn stepFn; // NULL here means, nothing pending
        QString regExpPattern;
        QVariantList args;
        QWeakPointer<QObject> sender;
        int retriesLeft;

        StepRetryData() : stepFn(NULL), retriesLeft(0) {}
        void clear() { stepFn=NULL; sender.clear(); retriesLeft=0; }
        bool hasCallback() { return (stepFn && !sender.isNull()); }
        bool equals(InvokableStepFn thisFn, const QString &thisRegExpPattern, const QVariantList &thisArgs, QObject *thisSender) {
            return (thisFn == stepFn && thisSender == sender.data() && thisRegExpPattern == regExpPattern && thisArgs == args);
        }
    } mRetryData;

    int mRetryTimeout;
    int mRetryInterval;

    QTimer *mRetryTimer;

    QObject *mPendingSender;
    QTimer *mPendingTimer;

    CucumberStepDataMap mPluginSteps;

};

#endif // APPLICATIONINSTANCE_H
