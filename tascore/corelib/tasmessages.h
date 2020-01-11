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



#ifndef TASMESSAGES_H
#define TASMESSAGES_H

#include "tasconstants.h"

#include <QPointer>
#include <QByteArray>

class TasSocket;

class TasMessage
{
 public:
    TasMessage();
    TasMessage(quint8 flag, bool compressed, const QByteArray& data, qint32 messageId);
    ~TasMessage();

    void setData(const QString& message);
    void setFlag(quint8 flag);
    void setIsCompressed(bool compressed);
    void setTimedOut(bool timedOut);
    void setMessageId(qint32 messageId);
    void setIsError(bool isError);
    void setErrorMessage(const QString& message);
    void setData(const QByteArray& data, bool isCompressed = false);
    bool isError() const;
    bool isRequest() const;
    bool isResponse() const;

    QByteArray& data();
    QByteArray& dataCompressed();
    QString dataAsString();
    quint8 flag() const;
    bool isCompressed() const;
    qint32 messageId();
    void uncompressData();

private:
    QByteArray mData;
    quint8 mFlag;
    bool mCompressed;
    qint32 mMessageId;
};

class TasResponse : public TasMessage
{
public:

    TasResponse(qint32 messageId, const QByteArray& data=QByteArray(), bool isCompressed = false, bool isError = false);
    ~TasResponse();

    void setRequester(TasSocket* socket);
    TasSocket* requester();


private:
    QPointer<TasSocket> mSocket;
};

class ResponseFilter
{
public:
    virtual ~ResponseFilter() {}
    virtual void filterResponse(TasMessage& response) = 0;
};


#endif
