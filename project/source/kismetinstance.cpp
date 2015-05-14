/************************************************************************
**
**  Copyright (C) 2015  guobin  <guobin.gb@alibaba-inc.com>
**
**  This file is part of Orion2.
**
**  Orion2 is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation.
**
**  Orion is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Orion.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/
#include "kismetinstance.h"
#include <QtGui>
#include <math.h>
#include <QMenu>
#include "kismetscene.h"
#include "kismetmodel.h"
#include "kismetlinkarray.h"
#include "menumanager.h"

const int  INSTPORT_SIZE =  64;
const int  INSTPORT_NAMEW = 120;
const int  INSTPORT_NAMEH = 20;

BlueprintInstance::BlueprintInstance(ResourceHash key) : BlueprintModelPort(EVT_DATA_OUT, UDQ_T("object")){
	setZValue(KISMET_DEPTH_INSTANCE);
	boundingBox_.setRect(0, 0, INSTPORT_SIZE, INSTPORT_SIZE);
	// ���ñ�־
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

	setObjectType(key);
	portcolor_ = OBJECT_COLOR;
}

BlueprintInstance::~BlueprintInstance(){

}

void BlueprintInstance::setObjectType(ResourceHash key){
	prototype_ = (ObjectTypeResourceObject*)ResourceObject::findResource(key);
	if (prototype_){
		name_ = prototype_->description();
	}
	else{
		name_.clear();
	}

}

QPointF BlueprintInstance::controlPoint(int orient){

	QPointF point = sceneCenter();
	point.setX(point.x() + boundingBox_.width() /2 + 50 );
	return point;
}

void BlueprintInstance::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget){
	Q_UNUSED(widget);

	painter->setRenderHint( QPainter::Antialiasing, true );
	QPen borderPen;
	if( isSelected() || isUnderMouse() ){
		// ����ѡ�����ɫ
		borderPen.setColor( Qt::yellow );
	}
	else{
		borderPen.setColor( Qt::black  );
	}
	borderPen.setWidth(1);
	painter->setPen( borderPen );

	QRectF circleR = boundingBox_.adjusted(4,4,-4,-4).normalized();
	QPainterPath path;
	path.addEllipse( circleR );

	bool simpleDraw = canSimpleDraw();

	if (simpleDraw){
		painter->fillPath(path, portcolor_);
	}
	else{
		painter->fillPath(path, Qt::black);
	}

	painter->drawEllipse(boundingBox_);

	if (!simpleDraw){
		// ��������ͼ
		painter->setClipPath(path);
		painter->setClipping(true);
		QPixmap img;
		if (prototype_){
			if (prototype_->prototype_->hasAttribute(EF_ANIMATIONS) || prototype_->prototype_->hasAttribute(EF_TEXTURE)){
				img = prototype_->thumbnailImage();
			}
			if (img.isNull()){
				img = QPixmap::fromImage(prototype_->prototype_->icon_);
			}
			painter->drawPixmap((boundingBox_.width() - img.width()) / 2, (boundingBox_.height() - img.height()) / 2, img);
		}
		else{
			painter->setFont(nameFont());
			painter->setPen(Qt::white);
			// �������λ��
			QString typeStr;
			typeStr = UDQ_T("object");
			painter->drawText(computeTextPos(circleR, typeStr, nameFont()), typeStr);
		}
		painter->setClipping(false);

		// ��������
		painter->setPen(borderPen);
		QSize nameSize = computeText(name_, nameFont());
		painter->drawText(boundingBox_.left() + (boundingBox_.width() - nameSize.width()) / 2.0, boundingBox_.bottom() + INSTPORT_NAMEH, name_);

	}


}

void BlueprintInstance::mousePressEvent(QGraphicsSceneMouseEvent * event){
	// ����״̬

	if( isUnderMouse()  && isSelected() ){
		// ���ͬʱ����ALt
		if( event->modifiers() & Qt::AltModifier ){
			clearLink();
		}
		return;
	}

	QGraphicsItem::mousePressEvent( event );

}


QVariant BlueprintInstance::itemChange(GraphicsItemChange change, const QVariant & value){

	if (change == ItemPositionHasChanged && scene()){
		foreach(BlueprintLink *line, lineList_){
			line->updatePort(this);
		}
		return BlueprintObject::itemChange(change, value);
	}

	return BlueprintModelPort::itemChange(change, value);
}

// ����
void BlueprintInstance::encodeData(QDataStream& stream){
	BlueprintObject::encodeData(stream);
	if (prototype_){
		stream << prototype_->hashKey();
	}
	else{
		stream << (ResourceHash)0;
	}
}

void BlueprintInstance::decodeData(QDataStream& stream){
	BlueprintObject::decodeData(stream);

	ResourceHash key;
	stream >> key;
	setObjectType(key);

}

QJsonValue BlueprintInstance::expNode(int dt){

	if (dt == 4){
		return QString::number(prototype_->hashKey());
	}

	QJsonArray arr;
	arr.append((int)20);
	arr.append(QString::number(prototype_->hashKey()));  // ����ʱ��Ҫ�滻Ϊ����
	return arr;
}

void BlueprintInstance::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
	((KismetScene*)scene())->setMenuHost(this);

	QMenu menu;
	QAction* act = gMenuManager->getAction(ORION_KISMET_ADDINSTANCEVAR);
	menu.addAction(act);

	act = gMenuManager->getAction(ORION_KISMET_ADDINSTANCEINDEX);
	menu.addAction(act);

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	menu.exec(event->screenPos());
}


bool BlueprintInstance::validate(QStringList& msg){
	if (!hasLink()){
		// ���棺 û������
		msg.append(WARNING_MSG.arg(UDQ_TR("����ʵ��<%1>û������!").arg(instName())));
	}

	if (prototype_ == NULL){
		// ���� ������������û���ҵ�
		msg.append(ERROR_MSG.arg(UDQ_TR("����ʵ��<%1>�Ķ���ԭ��û���ҵ�!").arg(instName())));
		isError_ = true;
	}
	return !isError_;
}