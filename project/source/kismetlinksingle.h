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

#ifndef  ORION_KISMET_LINK_SINGLE_H
#define ORION_KISMET_LINK_SINGLE_H

#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "kismetdefine.h"
#include "kismetobject.h"
#include "kismetlink.h"

//////////////////////////////////////////////////////////////////////////

class BlueprintLinkSingleAnchor;

class BlueprintLinkSingle : public BlueprintLink{
public:
	// ʵ������
	ORION_TYPE_DECLARE( KISMET_LINK_SINGLE );

	BlueprintLinkSingle(BlueprintObject *sourceNode, BlueprintObject *destNode, const QPointF& srcPt = QPointF(0, 0), const QPointF& desPt = QPointF(0, 0), bool temp = false);
	virtual ~BlueprintLinkSingle();

	void adjust();

	// ��������
	virtual void drawLine(QPainter *painter, const QStyleOptionGraphicsItem *option);

	// ����
	virtual void encode( QDataStream& stream );

	// ��������
	virtual void upgradeLink(BlueprintObject *node);

	// ����ê��
	virtual  QPointF updateAnchor( BlueprintLinkAnchor* anchor  );
	// ���¶˿�
	virtual  void updatePort(BlueprintObject* node);

	// ������Ƶ�
	virtual void clearAnchor();

	// ��ʾ���Ƶ�
	virtual void showAnchor(bool visible = true);

	// �л���ʾê��
	virtual void toggleAnchor();

	QPainterPath shape() const;

	void setSourceCtrlPoint( const QPointF& pt ){
		sourceCtrlPoint_ = pt;
	}

	void setDestCtrlPoint( const QPointF& pt ){
		destCtrlPoint_ = pt;
	}

	// ���Ƶ�
	virtual void changeControlPoints(const QList<QPointF>& ctrlPts);
	virtual QList<QPointF> controlPoints();

protected:
	QPointF sourceCtrlPoint_;
	QPointF destCtrlPoint_;

	BlueprintLinkSingleAnchor* sourceAnchor_;
	BlueprintLinkSingleAnchor* destAnchor_;

	void initAnchor();

};


//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class BlueprintLinkSingleAnchor : public BlueprintLinkAnchor{
public:
	// ʵ������
	ORION_TYPE_DECLARE( KISMET_LINK_ANCHOR_SINGLE );

	BlueprintLinkSingleAnchor(BlueprintLink* host, int type, QGraphicsItem* parent = NULL);
	virtual ~BlueprintLinkSingleAnchor();
	

	int anchorType(){
		return anchorType_;
	}

protected:
	int anchorType_;  // ���Ƶ�����
};



#endif
