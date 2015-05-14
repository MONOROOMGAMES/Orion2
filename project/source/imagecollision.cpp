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

#include "imagecollision.h"
#include <QtGui>
#include <QGraphicsSceneMouseEvent>
#include <math.h>
#include "imagescene.h"
#include "commondefine.h"
#include "configmanager.h"


ImageCollision::ImageCollision(const QList<QPointF>& pts, QGraphicsItem * parent){
	fillColor_ = QColor(255, 0, 255, 50);
	editing_ = false;
	// ���ñ�־
	setFlag(ItemIsMovable, true);
	setFlag(ItemIsSelectable, false);
	setFlag(ItemIsFocusable, true);
	setFlag(ItemSendsGeometryChanges, true);

	// ������꾭���¼�
	setAcceptHoverEvents(true);

	// �������Ϸ�
	setAcceptDrops(false);

	initPolygon(pts);
	initJoint();
	adjust();
}

ImageCollision::~ImageCollision(){
	clear();
}

//  ���غ���
void ImageCollision::clear(){
	clearJoint();
}

// ���ɳ�ʼ�ı���
void ImageCollision::initPolygon(const QList<QPointF>& pts){
	// ����λ������˳ʱ�룩
	points_ = pts;
	updatePath();
}


void ImageCollision::showJoint(bool visible){
	initJoint();
	foreach(ImageCollisionJoint* joint, joints_){
		joint->setVisible(visible);
		if (visible){
			scene()->addItem(joint);
		}
		else{
			scene()->removeItem(joint);
		}
	}
	adjust();
}


void ImageCollision::clearJoint(){

	foreach(ImageCollisionJoint* joint, joints_){
		delete joint;
	}
	joints_.clear();
}

void  ImageCollision::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget){
	// ���
	painter->fillPath(path(), fillColor_);

	QPen borderPen;
	// ���� �߿���ɫ
	borderPen.setColor(Qt::red);
	borderPen.setStyle(Qt::DashDotLine);
	borderPen.setWidth(0);
	painter->setPen(borderPen);

	// ���ƾ���
	painter->drawPath(path());
}

QVariant ImageCollision::itemChange(GraphicsItemChange change, const QVariant & value){
	static QPointF oldPos;
	// λ�ñ仯ʱ���������Ƶ�λ��
	if (change == QGraphicsItem::ItemPositionChange && scene()){
		oldPos = pos();
	}
	else if (change == QGraphicsItem::ItemPositionHasChanged && scene()){
		if (!editing_){
			// �ƶ���λ��
			QPointF delta = pos() - oldPos;
			for (int i = 0; i < points_.size(); i++){
				points_[i] += delta;
			}
			initJoint();
			adjust();
		}
	}
	return QGraphicsItem::itemChange(change, value);
}

void ImageCollision::initJoint(){

	if (joints_.size() == 0){
		int index = 0;
		foreach(QPointF pt, points_){
			ImageCollisionJoint *joint = new ImageCollisionJoint(this, index++);
			joints_.append(joint);
			joint->ready_ = true;
		}
	}
	else{

		while (joints_.size() > points_.size()){
			ImageCollisionJoint* joint = joints_.last();
			joints_.pop_back();
			delete joint;
		}

		while (joints_.size() < points_.size()){
			ImageCollisionJoint *joint = new ImageCollisionJoint(this, joints_.size());
			joints_.append(joint);
			joint->ready_ = true;
		}

	}

}


// ���¿��Ƶ�
QPointF ImageCollision::updateJoint(ImageCollisionJoint* joint){
	// �޸Ŀ��Ƶ�����
	points_.replace(joint->index_, joint->scenePos());
	updatePath();
	update();
	return joint->scenePos();
}


// �Ƕ�����
bool jointSort(const QVector3D &p1, const QVector3D &p2){
	return p1.z() < p2.z();
}

void ImageCollision::adjust(){
	//  ����anchorλ��
	for (int i = 0; i < joints_.size(); i++) {
		joints_[i]->changePos(points_.at(joints_[i]->index_));  // ���볡����
	}

}


void ImageCollision::removeJoint(ImageCollisionJoint* joint){

	if (points_.size() <= 3)  // ��������3����
		return;

	int index = joint->index_;
	points_.removeAt(joint->index_);

	if (joints_.size() > index){
		joints_.removeAt(index);
		//  ���º����ڵ�����
		for (int i = index; i < joints_.size(); i++){
			joints_[i]->index_ = i;
		}
	}

	delete joint;
	updatePath();
	adjust();
	update();
}

bool pointOnSegment(const QLineF& line, const QPointF& pt){

	qreal  a = QLineF(pt, line.p1()).length();
	qreal  b = QLineF(pt, line.p2()).length();

	if (qAbs((a + b) - line.length()) < 4){
		return true;
	}
	return false;
}

void ImageCollision::mousePressEvent(QGraphicsSceneMouseEvent * event){

	// �����ctrl+���������ڶ���α��ϣ��������ߵ��е����һ����ê��
	if ((event->modifiers() & Qt::ControlModifier) && event->button() == Qt::LeftButton){
		bool valid = true;

		// ��ײ���
		for (int i = 0; i < points_.size(); i++){
			// ����ǵ㵽���㣬�򲻴���
			QVector2D vec(event->scenePos() - points_.at(i));
			if (vec.length() < 4){
				valid = false;
				break;
			}
		}

		if (valid){

			// ��ײ���
			for (int i = 0; i < points_.size(); i++){
				int end = i < points_.size() - 1 ? i + 1 : 0;
				QLineF line(points_.at(i), points_.at(end));
				if (pointOnSegment(line, event->scenePos())){  // ���ؾ���
					// ����Ϊ�����ֱ����, ֱ���е��������ê��
					QPointF newPoint = line.pointAt(0.5);
					if (end != 0){
						points_.insert(end, newPoint);
					}
					else{
						points_.append(newPoint);
					}

					showJoint(true);
					break;
				}
			}
		}
	}

	QGraphicsItem::mousePressEvent(event);
}

// ����·��
void ImageCollision::updatePath(){
	editing_ = true;

	QPainterPath path;
	bool start = true;
	QPointF begin = points_[0];
	foreach(QPointF pt, points_){

		if (start){
			path.moveTo(QPointF(0,0));
			start = false;
		}
		path.lineTo(pt - begin);
	}
	path.closeSubpath();
	setPath(path);
	setPos(begin);
	editing_ = false;
}


void ImageCollision::hoverEnterEvent(QGraphicsSceneHoverEvent * event){
	setCursor(Qt::ArrowCursor);
	QGraphicsItem::hoverEnterEvent(event);
}

void ImageCollision::hoverLeaveEvent(QGraphicsSceneHoverEvent * event){

	QGraphicsItem::hoverLeaveEvent(event);
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

ImageCollisionJoint::ImageCollisionJoint(ImageCollision* host, int index, QGraphicsItem* parent)
	: QGraphicsItem(parent), host_(host), index_(index)
{

	setZValue(DEPTH_SPRTIE_COLLISIONJOINT);
	setFlags(ItemIsSelectable | ItemIsMovable | ItemIsFocusable);
	setFlag(ItemSendsGeometryChanges, true);
	setFlag(ItemIgnoresTransformations, true);
	setAcceptHoverEvents(true);

	boundingBox_ = QRect(-4, -4, 8, 8);
	underMouse_ = false;
	ready_ = false;
}

QRectF ImageCollisionJoint::boundingRect() const{
	return boundingBox_;
}

QPainterPath ImageCollisionJoint::shape() const{
	QPainterPath path;
	path.addRect(boundingBox_);
	return path;
}

ImageCollisionJoint::~ImageCollisionJoint(){

}

void ImageCollisionJoint::hoverEnterEvent(QGraphicsSceneHoverEvent * event){
	underMouse_ = true;
	setCursor(Qt::ArrowCursor);
	QGraphicsItem::hoverEnterEvent(event);
	update();
}

void ImageCollisionJoint::hoverLeaveEvent(QGraphicsSceneHoverEvent * event){
	underMouse_ = false;
	QGraphicsItem::hoverLeaveEvent(event);
	update();
}

void ImageCollisionJoint::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget){

	QPen borderPen;
	if (isSelected() || underMouse_){
		// ����ѡ�����ɫ
		painter->fillRect(boundingBox_, Qt::yellow);
	}
	else{
		painter->fillRect(boundingBox_, Qt::blue);
	}

	painter->setPen(Qt::white);
	// ���ɫ
	painter->drawRect(boundingBox_);
}

QVariant ImageCollisionJoint::itemChange(GraphicsItemChange change, const QVariant & value){

	if (change == ItemPositionHasChanged && scene()){
		// ���λ�÷����ı䣬�޸Ĺ켣
		Q_ASSERT(host_ != NULL);
		if (ready_){
			QPointF newPos = host_->updateJoint(this);
			return newPos;
		}
	}

	return QGraphicsItem::itemChange(change, value);
}

void ImageCollisionJoint::changePos(const QPointF& newPos){
	ready_ = false;
	setPos(newPos);
	ready_ = true;
}

void ImageCollisionJoint::keyReleaseEvent(QKeyEvent * event){

	if (isSelected() && isPressDelete(event->key())){
		// ɾ�����Ƶ�	
		host_->removeJoint(this);
		return;
	}

	QGraphicsItem::keyPressEvent(event);
}