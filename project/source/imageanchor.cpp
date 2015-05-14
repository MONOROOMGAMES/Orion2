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

#include "imageanchor.h"
#include <QtGui>
#include <math.h>
#include "imagescene.h"
#include "commondefine.h"
#include "configmanager.h"

const int ANCHOR_SIZE = 16;

ImageAnchor::ImageAnchor(){

	boundingBox_.setRect(-ANCHOR_SIZE / 2, -ANCHOR_SIZE / 2, ANCHOR_SIZE, ANCHOR_SIZE);
	// �������ֵ
	setZValue(DEPTH_SPRTIE_ANCHOR);
	// ����������¼�
	setAcceptedMouseButtons(Qt::AllButtons);
	// ���κα�־
	setFlags(QGraphicsItem::ItemIsMovable | QGraphicsItem::ItemIgnoresTransformations );
	// ������꾭���¼�
	setAcceptHoverEvents( true );
	// �����Ϸ�
	setAcceptDrops( false );

}

ImageAnchor::~ImageAnchor(){

}

QRectF ImageAnchor::boundingRect() const{
	return boundingBox_;
}

QPainterPath ImageAnchor::shape() const{
	QPainterPath path;
	path.addRect(boundingBox_);
	return path;
}

void ImageAnchor::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	Q_UNUSED(widget);

	// �������
	QRectF bound = boundingRect();

	painter->setPen(Qt::red);
	painter->drawEllipse(boundingBox_);

	QRectF lines = bound.adjusted(-4, -4, 4, 4);
	QPointF center = lines.center();
	painter->drawLine(lines.left(), center.y(), lines.right(), center.y());
	painter->drawLine(center.x(), lines.top(), center.x(), lines.bottom());
}


void ImageAnchor::hoverEnterEvent(QGraphicsSceneHoverEvent * event){
	setCursor(Qt::ArrowCursor);
	QGraphicsItem::hoverEnterEvent(event);
}

void ImageAnchor::hoverLeaveEvent(QGraphicsSceneHoverEvent * event){

	QGraphicsItem::hoverLeaveEvent(event);
}