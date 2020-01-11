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


#include <QFontMetricsF>
#include <QTextCodec>
#include <QGraphicsItem>
#include <QGraphicsWidget>

#include "testabilityutils.h"
#include "tastraverseutils.h"
#include "taslogger.h"
#include "tasqtdatamodel.h"
#include "tasqtcommandmodel.h"

const char* const QT_PREFIX ="Qt_";
const char* const VKB_APP_ID = "vkb_app_id";

TasTraverseUtils::TasTraverseUtils()
{
    mTraverseFilter = new TasDataFilter();

}

TasTraverseUtils::~TasTraverseUtils()
{
    delete mTraverseFilter;
}

/*!
 * Createst a filter from the given data. A filter must always be cleared
 * after use.
 */
void TasTraverseUtils::createFilter(TasCommand* command)
{
    mTraverseFilter->initialize(command);
}

/*!
  Clears the filter.
 */
void TasTraverseUtils::clearFilter()
 {
    mTraverseFilter->clear();
}


void TasTraverseUtils::addObjectDetails(TasObject* objectInfo, QObject* object)
{
    if (!object) {
        TasLogger::logger()->warning(" TasTraverseUtils::addObjectDetails object is NULL");
        return;
    }
    if (!objectInfo) {
        TasLogger::logger()->warning(" TasTraverseUtils::addObjectDetails objectInfo is NULL");
        return;
    }

    objectInfo->setId(TasCoreUtils::objectId(object));

    //custom traversers may want to add their own types
    if(objectInfo->getType().isEmpty()){
        QString objectType = object->metaObject()->className();
        objectType.replace(QString(":"), QString("_"));
        objectInfo->setType(objectType);
    }
    if(includeAttribute("parent")){
        objectInfo->setParentId(getParentId(object));
    }
    printProperties(objectInfo, object);
    objectInfo->setName(object->objectName());

    //allows to detect vkb
    if(TasCoreUtils::getApplicationName().contains(PENINPUT_SERVER)){
        objectInfo->addAttribute(VKB_APP_ID, QString::number(qApp->applicationPid()));
    }
}


QString TasTraverseUtils::getParentId(QObject* object)
{
    if (!object) {
        TasLogger::logger()->warning(" TasTraverseUtils::getParentId object is NULL");
        return "";
    }
    QString parentId;
    QGraphicsWidget* go = qobject_cast<QGraphicsWidget*>(object);
    if(go){
        QGraphicsItem* gParent = go->parentItem();
        parentId = gParent ? TestabilityUtils::graphicsItemId(gParent):"";
    }
    if(parentId == 0){
        QObject* parent = object->parent();
        parentId = parent ? TasCoreUtils::objectId(parent):"";
    }
    return parentId;
}


void TasTraverseUtils::addVariantValue(TasAttribute& attr, const QVariant& value)
{
    switch (value.type()) {
    case QVariant::Size:
        attr.addValue(value.toSize());
        break;
    case QVariant::SizeF:
        attr.addValue(value.toSizeF());
        break;
    case QVariant::Point:
        attr.addValue(value.toPoint());
        break;
    case QVariant::PointF:
        attr.addValue(value.toPointF());
        break;
    case QVariant::Rect:
        attr.addValue(value.toRect());
        break;
    case QVariant::RectF:
        attr.addValue(value.toRectF());
        break;
    case QVariant::DateTime:
        attr.addValuePlainString(QString::number(value.toDateTime().toTime_t()));
        break;
    case QVariant::Date:
        attr.addValuePlainString(value.toDate().toString("dd.MM.yyyy"));
        break;
    case QVariant::ByteArray:
        attr.addValue(value.toByteArray().toBase64());
        break;
    default:
        attr.addValue(value.toString());
        break;
    }

    if(QVariant::typeToName(value.type())){
        attr.setDataType(QVariant::typeToName(value.type()));
    }

}


/*
  Print metadata details to the model.
*/
void TasTraverseUtils::printProperties(TasObject* objectInfo, QObject* object)
{
    if (!object) {
        TasLogger::logger()->warning(" TasTraverseUtils::printProperties object is NULL");
        return;
    }
    if (!objectInfo) {
        TasLogger::logger()->warning(" TasTraverseUtils::printProperties objectInfo is NULL");
        return;
    }

    const QMetaObject *metaobject = object->metaObject();
    if (!metaobject) {
        TasLogger::logger()->warning(" TasTraverseUtils::printProperties metaobject is NULL");
        return;
    }
    int count = metaobject->propertyCount();
    for (int i=0; i<count; i++){
        QMetaProperty metaproperty = metaobject->property(i);
        const char *name = metaproperty.name();
        if(includeAttribute(name)){
            TasAttribute& attr = objectInfo->addAttribute();
            //coordinates are always relative to something in the props
            if(QString(name) != "y" && QString(name) != "x" && QString(name) != "width" && QString(name) != "height"){
                attr.setName(name);
            }
            else{
                attr.setName(QString(QT_PREFIX)+name);
            }

            QVariant value = object->property(name);
            if(metaproperty.isEnumType() && !metaproperty.isFlagType()){
                QMetaEnum enumeration = metaproperty.enumerator();
                bool ok = false;
                int enumValue = value.toInt(&ok);
                if(ok){
                    attr.addValue(enumeration.valueToKey(enumValue));
                }
                else{
                    attr.addValue(value.toString());
                }
            }
            else{
                //Workaround for qsTrId lengthvariant handling
                if (QString(name) == "text")
                {
                  const int BinaryTextVariantSeparator = 0x9c;
                  attr.addValue(value.toString().left(value.toString().indexOf(QChar(BinaryTextVariantSeparator))));
                }
                else
                {
                  addVariantValue(attr, value);
                }
            }

            if(mTraverseFilter == 0 || !mTraverseFilter->filterProperties()){
                QString propertyTypes = "";
                if(metaproperty.isReadable()){
                    propertyTypes = "r";
                }
                if(metaproperty.isWritable ()){
                    propertyTypes += QString("w");
                }
                attr.setType(propertyTypes);
            }
        }
    }

    QList<QByteArray> dynNames = object->dynamicPropertyNames();
    foreach(QByteArray bytes, dynNames) {
        QString name(bytes);
        TasAttribute& attr = objectInfo->addAttribute();
        attr.setName(name);
        addVariantValue(attr, object->property(bytes.data()));
    }
}

/*!
  Adds font details to the item. The prefix can be used in situations where one item can have multiple fonts.
 */
void TasTraverseUtils::addFont(TasObject* objectInfo, QFont font)
{
    if(includeAttribute("fontKey")){
        objectInfo->addAttribute("fontKey", font.key());
    }
    if(includeAttribute("fontFamily")){
        objectInfo->addAttribute("fontFamily", font.family());
    }
    if(includeAttribute("bold")){
        objectInfo->addBooleanAttribute("bold", font.bold());
    }
    if(includeAttribute("fixedPitch")){
        objectInfo->addBooleanAttribute("fixedPitch", font.fixedPitch());
    }
    if(includeAttribute("italic")){
        objectInfo->addBooleanAttribute("italic", font.italic());
    }
    if(includeAttribute("pixelSize")){
        objectInfo->addAttribute("pixelSize", font.pixelSize());
    }
    if(includeAttribute("pointSize")){
        objectInfo->addAttribute("pointSize", font.pointSizeF());
    }
    if(includeAttribute("letterSpacing")){
        objectInfo->addAttribute("letterSpacing", font.letterSpacing());
    }
    if(includeAttribute("kerning")){
        objectInfo->addBooleanAttribute("kerning", font.kerning());
    }
    if(includeAttribute("overline")){
        objectInfo->addBooleanAttribute("overline", font.overline());
    }
    if(includeAttribute("strikeOut")){
        objectInfo->addBooleanAttribute("strikeOut", font.strikeOut());
    }
    if(includeAttribute("capitalization")){
        objectInfo->addAttribute("capitalization", font.capitalization());
    }
    if(includeAttribute("fontWeight")){
        objectInfo->addAttribute("fontWeight", font.weight());
    }
    if(includeAttribute("wordSpacing")){
        objectInfo->addAttribute("wordSpacing", font.wordSpacing());
    }
}

bool TasTraverseUtils::includeAttribute(const QString& attributeName)
{
    if(mTraverseFilter){
        if(mTraverseFilter->includeAttribute(attributeName) || mTraverseFilter->includeAttribute(QT_PREFIX+attributeName)){
            return true;
        }
        return false;
    }
    return true;
}
bool TasTraverseUtils::useViewCrop()
{
    return mTraverseFilter && mTraverseFilter->useViewCrop();
}

QPair<QPoint,QPoint> TasTraverseUtils::addGraphicsItemCoordinates(TasObject* objectInfo,
                                                                 QGraphicsItem* graphicsItem,
                                                                 TasCommand* command)
{
    objectInfo->addAttribute("scenePos", graphicsItem->scenePos());
    ItemLocationDetails locationDetails = TestabilityUtils::getItemLocationDetails(graphicsItem, command);

    QPair<QPoint,QPoint> coords;
    if(true /*locationDetails.visible*/){
        objectInfo->addAttribute("x", locationDetails.windowPoint.x());
        objectInfo->addAttribute("y", locationDetails.windowPoint.y());

        objectInfo->addAttribute("x_absolute", locationDetails.screenPoint.x());
        objectInfo->addAttribute("y_absolute", locationDetails.screenPoint.y());

        coords.first = locationDetails.windowPoint;
        coords.second = locationDetails.screenPoint;

        objectInfo->addAttribute("width", locationDetails.width);
        objectInfo->addAttribute("height", locationDetails.height);
    }

    // if the onDisplay property is calculated by the fw, believe it if is set to false.
    QGraphicsObject* qobj = graphicsItem->toGraphicsObject();
    if (qobj) {
        QVariant onDisplay = qobj->property("onDisplay");
        if (onDisplay.isValid() && !onDisplay.toBool()) {
            objectInfo->addBooleanAttribute("visibleOnScreen", false);
        } else {
            objectInfo->addBooleanAttribute("visibleOnScreen", locationDetails.visible);
        }
    } else {
        objectInfo->addBooleanAttribute("visibleOnScreen", locationDetails.visible);
    }
    return coords;
}


/*!
  Add details about the text into the attribute. Including elide and coded details.
  Add the elided text, e.g. "This is a long text" -> "This is a long ...", into attributes
*/
void TasTraverseUtils::addTextInfo(TasObject* objectInfo, const QString& text,
                                  const QFont& font, qreal width, Qt::TextElideMode mode)
{
    QFontMetricsF metrics(font);

    QString elided = metrics.elidedText(text, mode, width);
    objectInfo->addAttribute("elidedText", elided);
    bool inFont = true;
    for (int i = 0; i < text.size(); ++i) {
        // endline char will always fail the comparison
        if (text.at(i) != '\n' && !metrics.inFont(text.at(i))) {
            inFont = false;
            break;
        }
    }
    objectInfo->addBooleanAttribute("canDisplayText", inFont);
}


/*!
    Print graphicsitem details that could be usable to the model.
*/
void TasTraverseUtils::printGraphicsItemProperties(TasObject* objectInfo, QGraphicsItem* graphicsItem)
{
    QGraphicsObject* gObject = graphicsItem->toGraphicsObject();

    if(mTraverseFilter->includeAttribute("visible")){
        if(gObject && !gObject->property("visible").isValid())
            objectInfo->addBooleanAttribute("visible", graphicsItem->isVisible());
    }
    if(mTraverseFilter->includeAttribute("enabled")){
        if(gObject && !gObject->property("enabled").isValid())
            objectInfo->addBooleanAttribute("enabled", graphicsItem->isEnabled());
    }
    if(mTraverseFilter->includeAttribute("selected")){
        if(gObject && !gObject->property("selected").isValid())
            objectInfo->addBooleanAttribute("selected", graphicsItem->isSelected());
    }
    if(mTraverseFilter->includeAttribute("obscured")){
        if(gObject && !gObject->property("obscured").isValid())
            objectInfo->addBooleanAttribute("obscured", graphicsItem->isObscured());
    }
    if(mTraverseFilter->includeAttribute("focus")){
        if(gObject && !gObject->property("focus").isValid())
            objectInfo->addBooleanAttribute("focus", graphicsItem->hasFocus());
    }
    if(mTraverseFilter->includeAttribute("under-mouse")){
        if(gObject && !gObject->property("under-mouse").isValid())
            objectInfo->addBooleanAttribute("under-mouse", graphicsItem->isUnderMouse());
    }
    if(mTraverseFilter->includeAttribute("droppable")){
        if(gObject && !gObject->property("droppable").isValid())
            objectInfo->addBooleanAttribute("droppable", graphicsItem->acceptDrops());
    }
    if(mTraverseFilter->includeAttribute("hoverable")){
        if(gObject && !gObject->property("hoverable").isValid())
            objectInfo->addBooleanAttribute("hoverable", graphicsItem->acceptHoverEvents());
    }
    if(mTraverseFilter->includeAttribute("tooltip")){
        if(gObject && !gObject->property("tooltip").isValid())
            objectInfo->addAttribute("tooltip", graphicsItem->toolTip());
    }
    if(mTraverseFilter->includeAttribute("z-value")){
        if(gObject && !gObject->property("z-value").isValid())
            objectInfo->addAttribute("z-value", QString::number(graphicsItem->zValue()));
    }

    //allows to detect vkb
    if(TasCoreUtils::getApplicationName().contains(PENINPUT_SERVER)){
        objectInfo->addAttribute(VKB_APP_ID, QString::number(qApp->applicationPid()));
    }

}


TasDataFilter::TasDataFilter()
{
    mExcludeProperties = true;
}

TasDataFilter::~TasDataFilter()
{
    clear();
}

void TasDataFilter::initialize(TasCommand* command)
{
    clear();
    if(!command){
        return;
    }
    //if the command contains filtering instructions
    //set the them for all traverser plugins
    QStringList attributeBlackList;
    if(!command->apiParameter("attributeBlackList").isEmpty()){
         mAttributeBlackList = command->apiParameter("attributeBlackList").split(",");
    }
    QStringList attributeWhiteList;
    if(!command->apiParameter("attributeWhiteList").isEmpty()){
        mAttributeWhiteList = command->apiParameter("attributeWhiteList").split(",");
    }
    mExcludeProperties = false;
    if(command->apiParameter("filterProperties") =="true"){
        mExcludeProperties = true;
    }
    mUseViewCrop = command->apiParameter("useViewCrop") == "true" || command->parameter("useViewCrop") == "true";
}

void TasDataFilter::clear()
{
    mAttributeWhiteList.clear();
    mAttributeBlackList.clear();
    mExcludeProperties = true;
}

bool TasDataFilter::filterProperties()
{
    return mExcludeProperties;
}

bool TasDataFilter::includeAttribute(const QString& attributeName)
{
    bool include = false;
    if(mAttributeWhiteList.isEmpty() && mAttributeBlackList.isEmpty()){
        include = true;
    }
    //black list is valued higher than white list
    else if(mAttributeWhiteList.contains(attributeName) && !mAttributeBlackList.contains(attributeName)){
        include = true;
    }
    else if(mAttributeWhiteList.isEmpty() && !mAttributeBlackList.contains(attributeName)){
        include = true;
    }
    return include;
}

bool TasDataFilter::useViewCrop()
{
    return mUseViewCrop;
}



