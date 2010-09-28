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
 
#include <QApplication>
#include <QWidget>
#include <QGraphicsItem>
#include <QPoint>
#include <QTouchEvent>
#include <QHash>

#include "uicommandservice.h"

#ifndef TASTOUCHEVENTGENERATOR_H
#define TASTOUCHEVENTGENERATOR_H

struct TasTouchPoints
{
    QPoint screenPoint;
    QPoint lastScreenPoint;
    QPoint startScreenPoint;
    bool isPrimary;
};


class TasTouchEventGenerator : public QObject
{
    Q_OBJECT
public:
    TasTouchEventGenerator(QObject* parent=0);
    ~TasTouchEventGenerator();

	void doTouchBegin(QWidget* target, QGraphicsItem* targetItem, QPoint point, bool primary, QString extraIdentifier);
	void doTouchUpdate(QWidget* target, QGraphicsItem* targetItem, QPoint point, bool primary);
	void doTouchEnd(QWidget* target, QGraphicsItem* targetItem, QPoint point, bool primary, QString extraIdentifier);

	QList<QTouchEvent::TouchPoint> convertToTouchPoints(TargetData targetData, Qt::TouchPointState state);
 
    QList<QTouchEvent::TouchPoint> convertToTouchPoints(QWidget* target, QGraphicsItem* targetItem, Qt::TouchPointState state,
                                                        QList<TasTouchPoints> points, QString extraIdentifier=QString());
    QTouchEvent::TouchPoint makeTouchPoint(QWidget* target, TasTouchPoints points, Qt::TouchPointState state, int id);

	QList<TasTouchPoints> toTouchPoints(QPoint point, bool primary);
    void sendTouchEvent(QWidget* target, QTouchEvent* event);

public slots:
    void doTouchBegin(QWidget* target, QGraphicsItem* targetItem, QList<TasTouchPoints> points, QString extraIdentifier=QString());
    void doTouchUpdate(QWidget* target, QGraphicsItem* targetItem, QList<TasTouchPoints> points);
    void doTouchEnd(QWidget* target, QGraphicsItem* targetItem, QList<TasTouchPoints> points, QString extraIdentifier=QString());

private:
    static int mTouchPointCounter;
};

#endif
