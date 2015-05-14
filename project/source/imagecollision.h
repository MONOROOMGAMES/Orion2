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

#ifndef ORION_IMAGECOLLISION_H
#define ORION_IMAGECOLLISION_H

#include <QGraphicsItem>
#include <QGraphicsPathItem>
#include <QFont>
#include "spritedefine.h"
using namespace ORION;

class ImageCollisionJoint;

// ѡ��״̬�£���ʾê�㣬��ѡ�к�ɾ�����ƶ���ê����������ĵ�ķ�λ˳ʱ������
// ��alt������������������ê��
class ImageCollision : public 	QGraphicsPathItem{
public:
	ORION_TYPE_DECLARE(SPRITE_COLLISION)

	ImageCollision(const QList<QPointF>& pts,  QGraphicsItem * parent = 0);
	virtual ~ImageCollision();

	//  ���غ���
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	// ���ɳ�ʼ�ı���
	void initPolygon(const QList<QPointF>& pts );

	// ����
	virtual void clear();

	// ����·��
	void updatePath();

	// �޸�·����( λ��Ϊ�������� )
	void changePointAt(int index, const QPointF& newPoint);

	QList<QPointF>  points_;  //  ����
	QList<ImageCollisionJoint*> joints_;  // ê���б�

	void initJoint();

	QPointF updateJoint(ImageCollisionJoint* joint);
	void removeJoint(ImageCollisionJoint* joint);
	void clearJoint();
	void adjust();
	void showJoint(bool visible);
protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);

private:
	QColor fillColor_;
	bool editing_;

};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class ImageCollisionJoint : public QGraphicsItem{
public:
	// ʵ������
	ORION_TYPE_DECLARE(SPRITE_COLLISION_JOINT)

	ImageCollisionJoint(ImageCollision* host, int index, QGraphicsItem* parent = NULL);
	virtual ~ImageCollisionJoint();

	int  index_;   // ����
	ImageCollision   *host_;   // ����
	bool ready_;  // �Ƿ��ھ���״̬

	//  ���غ���
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	// �ı�λ�ã���������֪ͨ
	void changePos(const QPointF& newPos);

protected:
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
	void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
	void keyReleaseEvent(QKeyEvent * event);

private:
	QRectF  boundingBox_;     // ��������		
	bool underMouse_; // �Ƿ�������·�

};



#endif // ORION_IMAGEANCHOR_H
