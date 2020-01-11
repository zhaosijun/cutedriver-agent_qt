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

#include "resourceloggingservice.h"
#include "resourcedatagatherer.h"
#include "cpuloadgenerator.h"
#include <taslogger.h>
#include <tassocket.h>
#include <QTimer>
#include <QCoreApplication>
#include <memspysession.h>

const int DEFAULT_LOGGING_INTERVAL_MS   = 1000;
const int INTERVAL_S_TO_MS              = 1000;
const QString RESOURCE_LOGGER           = "resourcelogger";
const QString LOG_FILE_NAME_PREFIX      = "mem_usage_";
const QString LOG_FILE_EXT              = ".log";
const QString LOG_FILE_PATH             = "";

ResourceLoggingService::ResourceLoggingService() :
    mResourceLoggerProcessRunning(false),
    mLoadGenerator(0)
{}

ResourceLoggingService::~ResourceLoggingService()
{
    foreach(ResourceLoggingTimer* timer, mLoggingTimers) {
        delete timer;
        timer = 0;
    }
    mLoggingTimers.clear();

    if (mLoadGenerator) {
        mLoadGenerator->stop();
        delete mLoadGenerator;
        mLoadGenerator = 0;
    }
}

bool ResourceLoggingService::executeService(
        TasCommandModel& model,
        TasResponse& response)
{
    TasLogger::logger()->debug("> ResourceLoggingService::executeService: " + model.service());
    bool status = false;

    if(model.service() == serviceName()) {
        status = true;
        int error = TAS_ERROR_NONE;
        TasCommand* command = 0;
        QString responseData;

        /*
         * Start process memory logging
         */
        if(commandIs(model, "ProcessMemLoggingStart", command)) {
            error = startLogging(ResourceLoggingService::TypeProcessMem, *command);
            if (TAS_ERROR_NONE != error) {
                TasLogger::logger()->error("  Error in logging start: " + QString::number(error));
                response.setErrorMessage("Error in logging start: " + QString::number(error));
            }
        }
        /*
         * Stop process memory logging
         */
        else if (commandIs(model, "ProcessMemLoggingStop", command)) {
            error = stopLogging(ResourceLoggingService::TypeProcessMem, *command, responseData);
            if (TAS_ERROR_NONE != error) {
                TasLogger::logger()->error("  Error in logging stop: " + QString::number(error));
                response.setErrorMessage("Error in logging stop: " + QString::number(error));
            }
            else {
                response.setData(responseData);
            }
        }
        /*
         * Start process memory logging
         */
        else if(commandIs(model, "CpuLoadStart", command)) {
            error = startLoad(*command);
            if (TAS_ERROR_NONE != error) {
                TasLogger::logger()->error("  Error in starting CPU load: " + QString::number(error));
                response.setErrorMessage("Error in starting CPU load: " + QString::number(error));
            }
        }
        /*
         * Stop process memory logging
         */
        else if (commandIs(model, "CpuLoadStop", command)) {
            error = stopLoad(*command, responseData);
            if (TAS_ERROR_NONE != error) {
                TasLogger::logger()->error("  Error in stopping CPU load: " + QString::number(error));
                response.setErrorMessage("Error in stopping CPU load: " + QString::number(error));
            }
            else {
                response.setData(responseData);
            }
        }
        /*
         * Capture thread heap dump
         */
        else if (commandIs(model, "ThreadHeapDump", command)) {
            error = dumpThreadHeap(*command, responseData);
            if (TAS_ERROR_NONE != error) {
                TasLogger::logger()->error("  Error in dumping thread heap: "
                                                + responseData
                                                + QString::number(error));
                response.setErrorMessage("Error in dumping thread heap: "
                                                + responseData
                                                + QString::number(error));
            }
        }
        /*
         * Start device memory logging
         */
        else if (0) {
        }
        /*
         * Stop device memory logging
         */
        else if (0) {
        }
        /*
         * Invalid action
         */
        else {
            TasLogger::logger()->error("  Unknown command: " + model.name());
            response.setErrorMessage("Unknown command: " + model.name());
        }
    }
    //check to see if the server can be stopped
    if( (!mLoadGenerator || !mLoadGenerator->isRunning()) && mLoggingTimers.isEmpty()){
        connect(response.requester(), SIGNAL(messageSent()), this, SLOT(requestQuit()));
    }

    TasLogger::logger()->debug("< ResourceLoggingService::executeService");
    return status;
}

void ResourceLoggingService::requestQuit()
{
    //small delay to allow the server to read the response
    QTimer::singleShot(100, QCoreApplication::instance(), SLOT(quit()));
}


int ResourceLoggingService::startLogging(
        ResourceLoggingService::ResourceType resourceType,
        TasCommand& command)
{
    Q_UNUSED(resourceType)
    TasLogger::logger()->debug("> ResourceLoggingService::startLogging: " + command.name());

    QString processName = command.parameter("thread_name");
    if (processName.isEmpty()) {
        return TAS_ERROR_PARAMETER;
    }

    QString logFileName = command.parameter("file_name");
    if (logFileName.isEmpty()) {
        logFileName.append(LOG_FILE_PATH);
        logFileName.append(LOG_FILE_NAME_PREFIX);
        logFileName.append(processName);
        logFileName.append(LOG_FILE_EXT);
    }

    bool timestampAbsolute = false;
    if (command.parameter("timestamp") == "absolute") {
        timestampAbsolute = true;
    }

    bool ok;
    int interval = command.parameter("interval_s").toInt(&ok);
    if (ok && interval > 0) {
        interval *= INTERVAL_S_TO_MS;
    }
    else {
        interval = DEFAULT_LOGGING_INTERVAL_MS;
    }

    ResourceLoggingTimer* timer = new ResourceLoggingTimer(resourceType, processName,
            logFileName, timestampAbsolute, interval);
    if (!timer) {
        return TAS_ERROR_CREATION_FAILED;
    }
    connect(timer, SIGNAL(error()), this, SLOT(timerError()));
    mLoggingTimers.append(timer);
    timer->run();
    timer->start();

    TasLogger::logger()->debug("< ResourceLoggingService::startLogging");
    return TAS_ERROR_NONE;
}

int ResourceLoggingService::stopLogging(
        ResourceLoggingService::ResourceType resourceType,
        TasCommand& command,
        QString& responseData)
{
    Q_UNUSED(resourceType)
    TasLogger::logger()->debug("> ResourceLoggingService::stopLogging: " + command.name());

    QString processName = command.parameter("thread_name");
    if (processName.isEmpty()) {
        return TAS_ERROR_PARAMETER;
    }

    bool returnData = false;
    if (command.parameter("return_data") == "true") {
        returnData = true;
    }

    bool foundAny = false;
    QMutableListIterator<ResourceLoggingTimer*> iter(mLoggingTimers);
    while (iter.hasNext()) {
        ResourceLoggingTimer* timer = iter.next();
        if (timer->resource() == processName) {
            foundAny = true;
            timer->stop();
            if (returnData) {
                responseData = *timer->getLogFileData();
            }
            else {
                responseData = timer->getLogFileName();
            }
            delete timer;
            iter.remove();
            break;
        }
    }
    if (!foundAny) {
        return TAS_ERROR_NOT_FOUND;
    }

    TasLogger::logger()->debug("< ResourceLoggingService::stopLogging");
    return TAS_ERROR_NONE;
}

int ResourceLoggingService::startLoad(TasCommand& command)
{
    TasLogger::logger()->debug("> CpuLoadService::startLoad: " + command.name());

    bool ok;
    int load = command.parameter("cpu_load").toInt(&ok);
    int error = TAS_ERROR_NONE;
    if (ok && load > 0) {
        if (!mLoadGenerator) {
            mLoadGenerator = new CpuLoadGenerator();
        }
        error = mLoadGenerator->start(load);
    }
    else {
        error = TAS_ERROR_PARAMETER;
    }

    TasLogger::logger()->debug("< CpuLoadService::startLoad");
    return error;
}

int ResourceLoggingService::stopLoad(TasCommand& command, QString& responseData)
{
    Q_UNUSED(responseData);
    TasLogger::logger()->debug("> CpuLoadService::stopLoad: " + command.name());

    int error = TAS_ERROR_NONE;
    if (mLoadGenerator) {
        error = mLoadGenerator->stop();
//        delete mLoadGenerator;
//        mLoadGenerator = 0;
    }

    TasLogger::logger()->debug("< CpuLoadService::stopLoad");
    return error;
}

int ResourceLoggingService::dumpThreadHeap(TasCommand& command, QString& response)
{
    TasLogger::logger()->debug("> ResourceLoggingService::dumpThreadHeap: " + command.name());
    int error = TAS_ERROR_NONE;

    QString threadName = command.parameter("thread_name");
    if (threadName.isEmpty()) {
        response = "No thread name given: ";
        return TAS_ERROR_PARAMETER;
    }

    RMemSpySession memSpy;
    error = memSpy.Connect();
    if (KErrNone == error) {
        TRAP(error, memSpy.SwitchOutputToFileL(HEAP_DUMP_PATH));
        if (error) {
            QString dumpPath = QString::fromUtf16(HEAP_DUMP_PATH().Ptr(), HEAP_DUMP_PATH().Length());
            response = "memSpy.SwitchOutputToFileL(" + dumpPath + ") failed: ";
        }
        TPtrC descThreadName(threadName.utf16(), threadName.length());
        TRAP(error, memSpy.OutputThreadHeapDataL(descThreadName));
        if (error) {
            response = "memSpy.OutputThreadHeapDataL(" + threadName + ") failed: ";
        }
        memSpy.Close();
    }
    else {
        response = "memSpy.Connect() failed: ";
    }

    TasLogger::logger()->debug("< ResourceLoggingService::dumpThreadHeap");
    return error;
}

void ResourceLoggingService::timerError(int errorCode, QString& resourceIdentifier)
{
    TasLogger::logger()->debug("> ResourceLoggingService::timerError: " + QString::number(errorCode));
    int i = 0;
    foreach (ResourceLoggingTimer* timer, mLoggingTimers) {
        if (timer->resource() == resourceIdentifier) {
            timer->stop();
            delete timer;
            mLoggingTimers.removeAt(i);
            break;
        }
        i++;
    }
}

bool ResourceLoggingService::commandIs(
        TasCommandModel& commandModel,
        const QString& commandName,
        TasCommand*& command)
{
    TasTarget* target = commandModel.findTarget(APPLICATION_TARGET);
    if(target) {
        command = target->findCommand(commandName);
        if (command) {
            return true;
        }
        else {
            return false;
        }
    }
    return false;
}

ResourceLoggingTimer::ResourceLoggingTimer(
        ResourceLoggingService::ResourceType resourceType,
        QString& resourceIdentifier,
        QString& logFileName,
        bool timestampAbsolute,
        int milliSecInterval) :
            mTimer(0),
            mDataGatherer(0),
            mLogFile(0),
            mLogFileData(0),
            mResourceType(resourceType),
            mMilliSecInterval(milliSecInterval),
            mFirstLogFileWrite(true)
{
    mResourceIdentifier = resourceIdentifier;
    mLogFileName = logFileName;

    int errorCode = TAS_ERROR_NONE;
    mTimer = new QTimer(this);
    if (mTimer) {
        connect(mTimer, SIGNAL(timeout()), this, SLOT(run()));
        mTimer->setInterval(milliSecInterval);
        mLogFile = new QFile(mLogFileName);
        if (mLogFile) {
            if (mLogFile->exists()) {
                if (!mLogFile->remove()) {
                    errorCode = TAS_ERROR_FILE_ERROR;
                }
            }
            if (mLogFile->open(QIODevice::ReadWrite | QIODevice::Text | QIODevice::Truncate)) {
                mLogFileStream.setDevice(mLogFile);
                mDataGatherer = new ResourceDataGatherer(resourceType,
                        resourceIdentifier, timestampAbsolute);
                if (!mDataGatherer) {
                    errorCode = TAS_ERROR_CREATION_FAILED;
                }
            }
            else {
                errorCode = TAS_ERROR_FILE_ERROR;
            }
        }
        else {
            errorCode = TAS_ERROR_CREATION_FAILED;
        }
    }
    else {
        errorCode = TAS_ERROR_CREATION_FAILED;
    }

    if (TAS_ERROR_NONE != errorCode) {
        emit error(errorCode, mResourceIdentifier);
    }
}

ResourceLoggingTimer::~ResourceLoggingTimer()
{
    if (mTimer->isActive()) {
        mTimer->stop();
    }
    delete mTimer;
    if (mLogFile) {
        if (mLogFile->isOpen()) {
            mLogFile->close();
        }
        delete mLogFile;
    }
    delete mDataGatherer;
    delete mLogFileData;
}

int ResourceLoggingTimer::interval()
{
    if (mTimer) {
        return mTimer->interval();
    }
    else {
        emit error(TAS_ERROR_INVALID_STATE, mResourceIdentifier);
        return TAS_ERROR_INVALID_STATE;
    }
}

void ResourceLoggingTimer::start()
{
    TasLogger::logger()->debug("> ResourceLoggingTimer::start");
    if (mTimer) {
        int interval = mTimer->interval();
        if (!interval) {
            mTimer->start(mMilliSecInterval);
        }
        else if (interval < 0) {
            emit error(interval, mResourceIdentifier);
        }
        else {
            mTimer->start();
        }
    }
    else {
        emit error(TAS_ERROR_INVALID_STATE, mResourceIdentifier);
    }
}

void ResourceLoggingTimer::start(int milliSecInterval)
{
    TasLogger::logger()->debug("> ResourceLoggingTimer::start: " + QString::number(milliSecInterval));
    if (mTimer) {
        if (milliSecInterval != interval()) {
            mTimer->start(milliSecInterval);
        }
        else {
            mTimer->start();
        }
    }
    else {
        emit error(TAS_ERROR_INVALID_STATE, mResourceIdentifier);
    }
}

void ResourceLoggingTimer::stop()
{
    TasLogger::logger()->debug("> ResourceLoggingTimer::stop");
    if (mTimer) {
        if (mTimer->isActive()) {
            mTimer->stop();
        }
    }
    else {
        emit error(TAS_ERROR_INVALID_STATE, mResourceIdentifier);
    }
}

QString* ResourceLoggingTimer::getLogFileData()
{
    if (mLogFile) {
        if (mLogFile->isOpen()) {
            mLogFileStream.seek(0);
            delete mLogFileData;
            mLogFileData = 0;
            mLogFileData = new QString(mLogFileStream.readAll());
            return mLogFileData;
        }
        else {
            emit error(TAS_ERROR_FILE_ERROR, mResourceIdentifier);
            return 0;
        }
    }
    else {
        emit error(TAS_ERROR_INVALID_STATE, mResourceIdentifier);
        return 0;
    }
}

void ResourceLoggingTimer::run()
{
    QString logEntry;
    int errorCode = mDataGatherer->getLogEntryData(logEntry);
    if (!errorCode) {
        mLogFileStream << logEntry;
    }
    else {
        emit error(errorCode, mResourceIdentifier);
    }
}
