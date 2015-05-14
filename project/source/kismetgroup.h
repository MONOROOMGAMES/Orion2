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
**  Orion2 is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Orion2.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#ifndef  ORION_KISMET_GROUP_H
#define ORION_KISMET_GROUP_H

#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsSceneMouseEvent>
#include <QFont>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "kismetdefine.h"
#include "kismetobject.h"
using namespace ORION;

class BlueprintGroupText : public QGraphicsSimpleTextItem{

public:
	ORION_TYPE_DECLARE( KISMET_GROUP_TEXT );
	BlueprintGroupText(const QString& text, QPointF size, QGraphicsItem *parent = 0);
};


//////////////////////////////////////////////////////////////////////////

class BlueprintGroup : public BlueprintObject{
public:
	// ʵ������
	ORION_TYPE_DECLARE( KISMET_GROUP );

	BlueprintGroup(const QString& title = QString(), QGraphicsItem * parent = 0);
	virtual ~BlueprintGroup();

	void resize( const QSizeF& newSize );
	QPainterPath shape() const;
	//  ���غ���
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	//  ��ȡ������
	virtual QString title(){
		return text_->text();
	}

	// ����
	virtual void encodeData(QDataStream& stream);
	virtual void decodeData(QDataStream& stream);

	virtual void postInit();

	void setFillColor( const QColor& color ){
		fillColor_ = color;
	}

	QColor fillColor(){
		return fillColor_;
	}

	void setTitle( const QString& title );

	// 
	void addSubObject(BlueprintObject* obj);
	void removeSubObject(BlueprintObject* obj);
	void removeAllSubObject();

	bool isActiveOnStart(){
		return activeOnStart_;
	}

	void setActiveOnStart(bool flag);

	// ���ݵ�ǰλ�úʹ�С�����Ӷ���
	void updateSubObjects();


	virtual QJsonValue expNode(int dt = -1);

	static QFont titleFont();

	virtual void backupState();
	virtual bool isStateChanged(int type);
	virtual void copyState(QDataStream& stream, int type);

protected:
	virtual void hoverMoveEvent ( QGraphicsSceneHoverEvent * event );
	virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
	virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
	virtual void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

private:
	BlueprintGroupText* text_;  // �ı�
	bool sizingFlag_; // �ı�ߴ�
	QColor fillColor_; //  ���ɫ
	QPointF oldPos_; 
	bool activeOnStart_;  // ����ʱ����

	QSet<BlueprintObject*>  subObjects_;  // ���а�����ģ��

	// ��ʱ����
	QList<ResourceHash> subIds_;


	QString oldText_;
	QColor oldFillColor_;
	bool oldActive_;  //  ����״̬

};


#endif
