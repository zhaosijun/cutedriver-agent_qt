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

#include <QGraphicsWidget>
#include <QGraphicsObject>
#include <QApplication>
#include <QQuickItem>
#include <QQuickWindow>

#include "testabilityutils.h"
#include "taslogger.h"
#include "testabilitysettings.h"
#include "tasqtcommandmodel.h"
#include "taspointercache.h"
#include "tasdeviceutils.h"

/*!
  \class TestabilityUtils
  \brief TasServiveBase is an abstract base class for tasplugin services

*/

/*!
  Casting directly from the id does not work so we need to look
  for the object with a matching id.
*/
QWidget* TestabilityUtils::findWidget(const QString& id)
{
    QWidget* widget = Q_NULLPTR;
    QObject* o = TasPointerCache::instance()->getObject(id);
    if(o != 0){
        widget = qobject_cast<QWidget*>(o);
        if(widget){
            return widget;
        }
    }
    QWidgetList widgetList = QApplication::allWidgets();
    if (!widgetList.empty()){
        QWidgetList::iterator i;
        for (i = widgetList.begin(); i != widgetList.end(); ++i){
            QWidget* current = *i;
            QString currentId = TasCoreUtils::objectId(current);
            if(currentId == id){
                widget = current;
                break;
            }
        }
    }
    return widget;
}

QWindow* TestabilityUtils::findWindow(const QString& id)
{
    QWindow* window = Q_NULLPTR;
    QObject* o = TasPointerCache::instance()->getObject(id);
    if(o != 0){
        window = qobject_cast<QWindow*>(o);
        if(window){
            return window;
        }
    }

    return window;
}

/*!
  Finding QObject from TasPointerCache. Traversing trough all QObjects of
  Application is not feasible.
*/
QObject* TestabilityUtils::findObject(const QString& id)
{
  QObject* o = TasPointerCache::instance()->getObject(id);
  if(o != 0){
      return o;
      }
  return Q_NULLPTR;
}

/*!
  Casting direclty from the id does not work so we need to look
  for the object with a matching id.

  Get the correct QGraphicsItem for the given object id.
  Looks for the item from graphicsviews and scenes
*/
QGraphicsItem* TestabilityUtils::findGraphicsItem(const QString& id)
{
    QGraphicsItem* item = Q_NULLPTR;
    QObject* o = TasPointerCache::instance()->getObject(id);
    if(o != 0){
        TasLogger::logger()->debug("TestabilityUtils::findGraphicsItem Object found from cache try casting");
        QGraphicsObject* go = qobject_cast<QGraphicsObject*>(o);
        if(go){
            TasLogger::logger()->debug("TestabilityUtils::findGraphicsItem object ok returning it.");
            return go;
        }
    }

    QWidgetList widgetList = qApp->topLevelWidgets();
    if (!widgetList.empty()){
        QList<QWidget*>::iterator iter;
        for (iter = widgetList.begin(); iter != widgetList.end(); iter++){
            QWidget *current = *iter;
            item = findFromObject(id, current);
            if(item){
                break;
            }
        }
    }
    else {
        qDebug("TasCommander::getGraphicsItem the list of top level widgets is empty!");
    }
    return item;
}

/*!
  Looks for a graphicsitem with thte given id from the children of the
  given object.
*/
QGraphicsItem* TestabilityUtils::findFromObject(const QString& id, QObject* object)
{
    QGraphicsItem* item = Q_NULLPTR;
    //check view first
    if (!object) return Q_NULLPTR;

    QGraphicsView* view = qobject_cast<QGraphicsView*>(object);
    if (view){
        item = lookForMatch(view->items(), id);
        if(!item){
            QGraphicsScene* scene = view->scene();
            if (scene) {
                item = lookForMatch(scene->items(), id);
            }
        }
    }
    //look from possible scenes
    QGraphicsScene* scene = qobject_cast<QGraphicsScene*>(object);
    if (!item && scene) {
        item = lookForMatch(scene->items(), id);
    }
    //look from child objects if item not on top level
    if(!item){
        QList<QObject*> objList = object->children();
        QList<QObject*>::iterator iter;
        for (iter = objList.begin(); iter != objList.end(); iter++){
            QObject* obj = *iter;
            if (obj->isWidgetType()){
                item = findFromObject(id, obj);
                if(item){
                    break;
                }
            }
        }
    }
    return item;
}

/*!
  Iterates a list of graphicsitems looking for a match for the given id.
  NULL is returned if a match is not found.
*/
QGraphicsItem* TestabilityUtils::lookForMatch(QList<QGraphicsItem*> itemList, const QString& targetId)
{
    QGraphicsItem* item = Q_NULLPTR;
    if (!itemList.isEmpty()){
        foreach(QGraphicsItem* child, itemList){
            if(verifyGraphicsItemMatch(targetId, child)){
                item = child;
                break;
            }
        }
    }
    return item;
}

/*!
  Verifys that a given GraphicsItem refences matches to the given id.
  If the item is a decendant of QObject (e.g. GraphicsWIdget) then
  the verification is done by casting the item to object and then
  making the comparison.
  Returns true if the given item matches.
*/
bool TestabilityUtils::verifyGraphicsItemMatch(const QString& targetId, QGraphicsItem* source)
{
    bool doesMatch = false;
    //first check simple match
    QString sourceId = TestabilityUtils::graphicsItemId(source);

    if (sourceId == targetId){
        doesMatch = true;
    }
    else if(source){
        //is object decendant
        QGraphicsObject* object = source->toGraphicsObject();
        if (object) {
            sourceId = TasCoreUtils::objectId(object);
            if (sourceId == targetId){
                doesMatch = true;
            }
        }
    }
    return doesMatch;
}

/*!
    Gets the viewport widget for the item through the scene and
    graphics view. Can be null if the viewport could not be determined.
    Sets the point to be the screen coordinate at the center point of the
    QGraphicsitem.
 */
QWidget* TestabilityUtils::viewPortAndPosition(QGraphicsItem* graphicsItem, QPoint& point)
{
    QGraphicsView* view = getViewForItem(graphicsItem);
    QWidget* widget = Q_NULLPTR;
    if(view){
        widget = view->viewport();
    }

    if (widget){
        QRectF sceneRect = graphicsItem->sceneBoundingRect();
        if(!view->viewportTransform().isIdentity()){
            QTransform transform = view->viewportTransform();
            sceneRect = transform.mapRect(sceneRect);
        }
        QRectF viewPortRect(widget->rect());
        QRectF interSection = viewPortRect.intersected(sceneRect);
        point = widget->mapToGlobal(interSection.center().toPoint());
    }
    return widget;
}

QGraphicsView* TestabilityUtils::getViewForItem(QGraphicsItem* graphicsItem)
{
    if(!graphicsItem){
        return Q_NULLPTR;
    }
    QGraphicsView* match = Q_NULLPTR;

    QGraphicsScene* scene = graphicsItem->scene();
    if(scene){
        QList<QGraphicsView*> list = scene->views();
        foreach(QGraphicsView* view, scene->views()){
            if(view->items().indexOf(graphicsItem) != -1){
                match = view;
                break;
            }
        }
    }
    return match;
}


/*! Return true if widgets belongs to custom traversed object, i.e.
 * Will be traversed even if not visible */
bool TestabilityUtils::isCustomTraverse() {

    QString applicationName = getApplicationName();

    return( applicationName == "webwidgetrunner" );

}

/*!
   See if the GraphicsItems region intersects the viewports visible region.
 */
bool TestabilityUtils::isItemInView(QGraphicsView* view, QGraphicsItem* graphicsItem)
{
    if(view){
        QRectF sceneRect = graphicsItem->sceneBoundingRect();
        if(!view->viewportTransform().isIdentity()){
            QTransform transform = view->viewportTransform();
            sceneRect = transform.mapRect(sceneRect);
        }
        QRect viewPortRect = view->viewport()->rect();

        // if item intersect with view
        if(viewPortRect.intersects(sceneRect.toRect())) {
            QRegion clippedVisibleRegion = viewPortRect.intersected(sceneRect.toRect());
            if (!clippedVisibleRegion.isEmpty()) {
                QPoint resultPoint = clippedVisibleRegion.rects().at(0).center();
                QList<QGraphicsItem*> topItems = view->items(resultPoint);

                QGraphicsItem* topItem = Q_NULLPTR;

                // top most item check disabled by default
                if(isVisibilityCheckOn()) {

                    // check top most item in item coordinates
                    for (int i = 0; i < topItems.size(); ++i) {
                        topItem = topItems.at(i);
                        QGraphicsObject* topObject = topItem->toGraphicsObject();
                        QRectF sceneRect = topItem->sceneBoundingRect();

                        // I am top
                        if(topItem == graphicsItem){
                            break;
                        }
                        // ignore special overlay items
                        else if (topObject && isItemBlackListed(topObject->objectName(), topObject->metaObject()->className())) {
                            continue;
                        }
                        // ignore items with no width or height - should not get these when using point??
                        else if(sceneRect.width() == 0 || sceneRect.height() == 0) {
                            continue;
                        }
                        // found the top most item
                        else {
                            break;
                        }
                    }

                    //QGraphicsObject* topObject = topItem->toGraphicsObject();
                    //QGraphicsObject* itemObject = graphicsItem->toGraphicsObject();
                    //TasLogger::logger()->debug("TestabilityUtils::isItemInView top item with id " + QString::number((quintptr)topObject) + " name " + topObject->metaObject()->className() + " item " + QString::number((quintptr)itemObject) + " itemname " + itemObject->metaObject()->className());

                    // if the item itself is the top most item or item is father of topmost item
                    if (topItem && (topItem == graphicsItem || graphicsItem->isAncestorOf(topItem))) {
                        return true;
                    }
                    else {
                        return false;
                    }
                }
                else {
                    // no top most check so return true as within viewport
                    return true;
                }
            }
            else {
                return false;
            }
        }
        else {
            return false;
        }
    }
    else{
        return false;
    }
}

QWidget* TestabilityUtils::getApplicationWidget()
{
    // attempt to find a window type widget
    QWidget* target = qApp->activePopupWidget();

    if (!target) {

        target = qApp->activeModalWidget();

        if (!target) {
            target = qApp->activeWindow();

            if(!target || !target->isWindow() || target->graphicsProxyWidget()){

                TasLogger::logger()->debug("TestabilityUtils::getApplicationWidget no active window - look for suitable");

                //no active, take first from list and use it
                QListIterator<QWidget*> iter(qApp->topLevelWidgets());

                while(iter.hasNext()){
                    QWidget* w = iter.next();
                    if((w->isVisible() ||

                        (isCustomTraverse() && w->inherits("QGraphicsView")) )
                            && w->isWindow() && w->graphicsProxyWidget() == 0){
                        TasLogger::logger()->debug("TestabilityUtils::getApplicationWindow window found");
                        target = w;
                        break;
                    }
                }
            }
        }
    }

    return target;
}


QWindow* TestabilityUtils::getApplicationWindow()
{
    //attemp to find a window
    QWindow* target = QApplication::focusWindow();

    if (!target) {
        target = QApplication::modalWindow();

        if (!target) {
            TasLogger::logger()->debug("TestabilityUtils::getApplicationWindow no active window - look for suitable");

            target = QApplication::topLevelWindows().first();

            if (!target) {
                target = QApplication::allWindows().first();
            }
        }
    }

    return target;
}


bool TestabilityUtils::isBlackListed()
{
    QString filePath = qApp->applicationFilePath();
    QVariant value = TestabilitySettings::settings()->getValue(BLACK_LISTED);
    if(value.isValid() && value.canConvert<QString>()){
        QStringList blackList = value.toString().split(",");
        for (int i = 0; i < blackList.size(); i++){
            QString blackListed = blackList.at(i);
            if(filePath.contains(blackListed)){
                return true;
            }
        }
    }
    return false;
}

bool TestabilityUtils::isVisibilityCheckOn()
{
    QVariant value = TestabilitySettings::settings()->getValue(VISIBILITY_CHECK);
    if(value.isValid() && value.canConvert<QString>()){
        QString onOff = value.toString();
        if(onOff.toLower() == "on"){
            return true;
        }
    }
    return false;
}

bool TestabilityUtils::isItemBlackListed(QString objectName, QString className)
{
    //black list will most likely contain names without dynamic qml extension
    if(className.contains("_QML")){
        QStringList stringList = className.split("_QML");
        className = stringList.takeFirst();
    }

    QVariant value = TestabilitySettings::settings()->getValue(VISIBILITY_BLACKLIST);
    if(value.isValid() && value.canConvert<QString>()){
        QStringList blackList = value.toString().split(",");
        for (int i = 0; i < blackList.size(); i++){
            QString blackListed = blackList.at(i);
            if(!objectName.isEmpty() && blackListed.contains(objectName)){
                return true;
            }
            if(blackListed.contains(className)){
                return true;
            }
        }
    }
    return false;
}
/*!
    Returns the Proxy Widget if any parent widget has a proxy
 */
QGraphicsProxyWidget* TestabilityUtils::parentProxy(QWidget* widget) {
    if (!widget) {
        return Q_NULLPTR;
    }

    QGraphicsProxyWidget* proxy = widget->graphicsProxyWidget();
    if (proxy) {
        return proxy;
    } else {
        return parentProxy(widget->parentWidget());
    }
}


QPoint TestabilityUtils::proxyCoordinates(QGraphicsItem* item, bool absolute)
{
    QRectF sceneRect = item->sceneBoundingRect();
    QGraphicsView* view = TestabilityUtils::getViewForItem(item);
    if(!view->viewportTransform().isIdentity()){
        QTransform transform = view->viewportTransform();
        sceneRect = transform.mapRect(sceneRect);
    }
    if (absolute) {
        return view->viewport()->mapToGlobal(sceneRect.topLeft().toPoint());
    } else {
        return sceneRect.topLeft().toPoint();
    }
}

QGraphicsItem* TestabilityUtils::findTopMostRotated(QGraphicsItem *item)
{
    QGraphicsItem* target = Q_NULLPTR;
    if(item->parentItem()){
        target = findTopMostRotated(item->parentItem());
    }
    if(!target){
        if(item->rotation() != 0){
            target = item;
        }
    }
    return target;
}

ItemLocationDetails TestabilityUtils::getItemLocationDetails(QGraphicsItem* graphicsItem, TasCommand* command)
{
    bool isVisible = false;
    QGraphicsView* view = getViewForItem(graphicsItem);
    ItemLocationDetails locationDetails = {QPoint(), QPoint(), QPoint(), 0, 0, false};
    if(view){
        // for webkit
        // If the window is embedded into a another app, wee need to figure out if the widget is visible
        QPoint windowSize(0,0);
        if (command && command->parameter("parent_size") != "") {
            QStringList sizes = command->parameter("parent_size").split(",");
            windowSize.setX(sizes[0].toInt());
            windowSize.setY(sizes[1].toInt());
        }
        isVisible = isItemInView(view, graphicsItem);

        if(true /*isVisible*/){
            //add coordinates also
            QRectF sceneRect = graphicsItem->sceneBoundingRect();
#if QT_VERSION >= 0x040704 && QT_VERSION < 0x050000
            if( view->testAttribute(Qt::WA_SymbianNoSystemRotation)){
                //this a bit problematic, lets look for the top most item in the hierachy
                //with rotation transformation
                QGraphicsItem* rotator = findTopMostRotated(graphicsItem);
                if(rotator){
                    QTransform transform = rotator->sceneTransform().inverted();
                    sceneRect = transform.mapRect(sceneRect);
                }
            }
            else if(!view->viewportTransform().isIdentity()){
#else
            if(!view->viewportTransform().isIdentity()){
#endif
                QTransform transform = view->viewportTransform();
                sceneRect = transform.mapRect(sceneRect);
            }
            QPoint point = sceneRect.topLeft().toPoint();
            QPoint screenPoint = view->viewport()->mapToGlobal(point);
            QPoint windowPoint = view->viewport()->window()->mapFromGlobal(screenPoint);

            // for webkit
            // If the window is embedded into a another app, wee need to figure out if the widget is visible
            if (command && command->parameter("x_parent_absolute") != "" &&
                    command->parameter("y_parent_absolute") != "") {
                QPoint p(command->parameter("x_parent_absolute").toInt(),
                         command->parameter("y_parent_absolute").toInt());
                screenPoint += p;
                point += p;
                windowPoint += p;
            }

            int height = sceneRect.height();
            int width = sceneRect.width();
            int xAbs = screenPoint.x();
            int yAbs = screenPoint.y();
            // Verify that the window is in application view
            if (windowSize != QPoint(0,0)) {
                // Windows with top left corner off screen are always off screen
                if (screenPoint.x() > windowSize.x() ||
                        screenPoint.y() > windowSize.y()) {
                    isVisible = false;
                } else {
                    // Window starts off-screen
                    if (xAbs < 0) {
                        width = xAbs+width;
                        xAbs = 0;
                    }
                    if (yAbs < 0) {
                        height = yAbs+height;
                        yAbs = 0;
                    }


                    // Window ends off-screen
                    if (xAbs+width > windowSize.x()) {
                        width = width - (xAbs+width - windowSize.x());
                    }
                    if (yAbs+height > windowSize.y()) {
                        height = height - (yAbs+height - windowSize.y());
                    }

                    if (width < 0 || height < 0) {
                        isVisible = false;
                    }

                }

            }
            screenPoint.setX(xAbs);
            screenPoint.setY(yAbs);

            locationDetails.scenePoint = point;
            locationDetails.screenPoint = screenPoint;
            locationDetails.windowPoint = windowPoint;
            locationDetails.width = width;
            locationDetails.height = height;
        }
    }

    locationDetails.visible = isVisible;
    return locationDetails;
}

QString TestabilityUtils::graphicsItemId(QGraphicsItem* graphicsItem)
{
    return QString::number((quintptr)graphicsItem);
}

bool TestabilityUtils::compareObjectName(const QString& objectName,const QString& targetObjectName) {
    if (targetObjectName == objectName) {
        return true;
    } else if (targetObjectName.contains('/')) {
        return QRegularExpression(targetObjectName).match(objectName).hasMatch();
    }
    return false;
}

QQuickItem* TestabilityUtils::findQuickItemByObjectName(QQuickItem* current, const QString& objectName, const QString& id)
{
    QString currentId = TasCoreUtils::objectId(current);

    if (id == currentId) {
        return current;
    }

    if (compareObjectName(current->objectName(), objectName)) {
        return current;
    }

    foreach (QQuickItem* child, current->childItems()) {
        QQuickItem* hit = findQuickItemByObjectName(child, objectName, id);
        if (hit) {
            // lets cache item
            TasCoreUtils::objectId(hit);
            return hit;
        }
    }

    return Q_NULLPTR;
}

QQuickItem* TestabilityUtils::findQuickItem(const QString& id, QQuickItem* current)
{
    QString currentId = TasCoreUtils::objectId(current);

    if (id == currentId) {
        return current;
    }

    foreach (QQuickItem* child, current->childItems()) {
        QQuickItem* hit = findQuickItem(id, child);
        if (hit) {
            // lets cache item
            TasCoreUtils::objectId(hit);
            return hit;
        }
    }

    return Q_NULLPTR;
}

QQuickItem* TestabilityUtils::findQuickItem(const QString& id)
{
    QObject* o = TasPointerCache::instance()->getObject(id);
    QQuickItem* item = qobject_cast<QQuickItem*>(o);

    if (item) {
        return item;
    }

    foreach (QWindow *w, QApplication::topLevelWindows()) {
        QQuickWindow* view = qobject_cast<QQuickWindow*>(w);
        if (view) {
            item = findQuickItem(id, view->contentItem());

            if (item) {
                break;
            }
        }
    }

    if (item) {
        // lets cache item
        TasCoreUtils::objectId(item);
    }

    return item;
}

QGraphicsWidget* TestabilityUtils::castToGraphicsWidget(QGraphicsItem* graphicsItem)
{
    if(graphicsItem && graphicsItem->isWidget()){
        return static_cast<QGraphicsWidget*>(graphicsItem);
    }
    return Q_NULLPTR;
}

