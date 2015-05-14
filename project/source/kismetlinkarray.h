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

#ifndef  ORION_KISMET_LINK_ARRAY_H
#define ORION_KISMET_LINK_ARRAY_H

#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "kismetdefine.h"
#include "kismetobject.h"
#include "kismetlink.h"

//////////////////////////////////////////////////////////////////////////

class BlueprintLinkArrayAnchor;

class BlueprintLinkArray : public BlueprintLink{

public:
	// ʵ������
	ORION_TYPE_DECLARE( KISMET_LINK_ARRAY );

	BlueprintLinkArray(BlueprintObject *sourceNode, BlueprintObject *destNode, const QList<QPointF>& ctrlPts, bool temp = false);
	virtual ~BlueprintLinkArray();

	virtual void adjust();

	void setControlPoints( const QList<QPointF>& ctrlPts ){
		ctrlPoints_ = ctrlPts;
	}

	virtual void changeControlPoints(const QList<QPointF>& ctrlPts);

	// ��ӿ��Ƶ�
	void addControlPoint( const QPointF& pt );

	// ������ʱ��
	virtual void updateTempLink( const QPointF& pos );

	// ��������
	virtual void drawLine(QPainter *painter, const QStyleOptionGraphicsItem *option);

	// ����
	virtual void encode( QDataStream& stream );
	
	// ��������
	virtual void upgradeLink(BlueprintObject *node);

	// ����ê��
	virtual  QPointF updateAnchor( BlueprintLinkAnchor* anchor  );
	virtual  void removeAnchor(BlueprintLinkAnchor* anchor);
	// ���¶˿�
	virtual  void updatePort(BlueprintObject* node);

	// ������Ƶ�
	virtual void clearAnchor();

	// ��ʾ���Ƶ�
	virtual void showAnchor(bool visible = true);

	// �л���ʾê��
	virtual void toggleAnchor();

	QPainterPath shape() const;

	// ���Ƶ�
	virtual QList<QPointF> controlPoints(){
		return ctrlPoints_;
	}

protected:
	QList<QPointF> ctrlPoints_;  // ���Ƶ��б�
	QList<BlueprintLinkArrayAnchor*> ctrlAnchors_;  // ê���б�
	void initAnchor();
	bool directline_;  // �Ƿ����ֱ��

};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class BlueprintLinkArrayAnchor : public BlueprintLinkAnchor{
public:
	// ʵ������
	ORION_TYPE_DECLARE( KISMET_LINK_ANCHOR_ARRAY );

	BlueprintLinkArrayAnchor(BlueprintLink* host, int type, QGraphicsItem* parent = NULL);
	virtual ~BlueprintLinkArrayAnchor();

	//  ���غ���
	void paint( QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget );


	void setAnchorIndex( int index ){
		anchorIndex_ = index;
	}

	int anchorIndex(){
		return anchorIndex_;
	}
private:
	int anchorIndex_;  // ���Ƶ�������

};



#endif
