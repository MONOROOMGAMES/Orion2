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

#ifndef  ORION_KISMET_LINK_H
#define ORION_KISMET_LINK_H

#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <QGraphicsSceneMouseEvent>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "kismetdefine.h"
#include "kismetobject.h"

//////////////////////////////////////////////////////////////////////////

class BlueprintLinkAnchor;

// ʵ�ּ򵥵�ֱ�߹���
class BlueprintLink : public QGraphicsPathItem{  

public:
	// ʵ������
	ORION_TYPE_DECLARE( KISMET_LINK );

	enum{
		END_SOURCE,  // ����˿�
		END_DEST,  // ����˿�
	};

	BlueprintLink(BlueprintObject *sourceNode, BlueprintObject *destNode, bool temp = false);
	virtual ~BlueprintLink();

	BlueprintObject *sourceNode() const;
	void setSourceNode(BlueprintObject *node);

	BlueprintObject *destNode() const;
	void setDestNode(BlueprintObject *node);

	void setUnderMouse( bool flag = true ){
		underMouse_ = flag;
	}

	// �������ߣ��������Ƶ㣩
	virtual void adjust();

	// �Ͽ���������
	virtual void unlink();

	// ��������
	virtual void drawLine(QPainter *painter, const QStyleOptionGraphicsItem *option);

	// ����
	virtual void encode( QDataStream& stream );

	// ������ʱ��
	virtual void updateTempLink( const QPointF& pos );

	// ��������
	virtual void upgradeLink(BlueprintObject *node);

	// ����ê��
	virtual  QPointF updateAnchor( BlueprintLinkAnchor* anchor  );
	virtual  void removeAnchor(BlueprintLinkAnchor* anchor);
	// ���¶˿�
	virtual  void updatePort(BlueprintObject* node);

	// ����˿�ê��
	virtual QPointF portAnchorPoint(BlueprintObject* node);

	// ������Ƶ�
	virtual void clearAnchor();

	// ��ʾ���Ƶ�
	virtual void showAnchor(bool visible = true);

	// �л���ʾê��
	virtual void toggleAnchor();

	// �˿����ͼ��
	virtual bool validate(QStringList& msg);

	// ���Ƶ�
	virtual void changeControlPoints(const QList<QPointF>& ctrlPts){
	}

	virtual QList<QPointF> controlPoints(){
		return QList<QPointF>();
	}
	virtual void copyState(QDataStream& stream);

	QPainterPath shape() const;
protected:
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);
	virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
	virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
	virtual void keyPressEvent ( QKeyEvent * event );
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

protected:
	QByteArray encodeEnd(BlueprintObject* obj, bool tmpl = false);
	BlueprintObject *source_;
	BlueprintObject *dest_;

	// ��ʱʹ��
	QPointF sourcePoint_;
	QPointF destPoint_;

	bool tempLink_; // �Ƿ�Ϊ��ʱ��
	bool editCtrl_;  //  �Ƿ����༭���Ƶ�״̬
	bool underMouse_;
	//  �ǳ־�
	bool isError_;  // ����
	QList<QPointF> oldCtrlPoints_;  // �ɵĿ��Ƶ�
	unsigned int version_;  //  �汾
};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class BlueprintLinkAnchor : public QGraphicsItem{
public:
	// ʵ������
	ORION_TYPE_DECLARE( KISMET_LINK_ANCHOR );

	BlueprintLinkAnchor(BlueprintLink *host, QGraphicsItem* parent = NULL);
	virtual ~BlueprintLinkAnchor();

	//  ���غ���
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint( QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget );

protected:
	virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
	virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
	virtual void keyPressEvent ( QKeyEvent * event );
	virtual QVariant itemChange ( GraphicsItemChange change, const QVariant & value );

protected:
	BlueprintLink   *host_;   // ����
	QRectF  boundingBox_;     // ��������		
	bool underMouse_; 

};



#endif
