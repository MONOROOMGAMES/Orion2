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

#ifndef ORION_KISMET_COMMENT_H
#define ORION_KISMET_COMMENT_H

#include <QFont>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "kismetdefine.h"
#include "kismetobject.h"
using namespace ORION;


class BlueprintComment  : public BlueprintObject{
public:
	// ʵ������
	ORION_TYPE_DECLARE( KISMET_COMMENT );

	BlueprintComment(const QString& text = QString(), QGraphicsItem * parent = 0);
	virtual ~BlueprintComment();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	// ����
	virtual void encodeData(QDataStream& stream);
	virtual void decodeData(QDataStream& stream);

	//  ��ȡ������
	virtual QString title(){
		return comment_;
	}

	// �����ߴ�
	virtual void adjust();

	// �����ı�
	void setText( const QString& text ){
		comment_ = text;
	}

	void setTextColor( const QColor& color ){
		textColor_ = color;
	}

	QColor textColor(){
		return textColor_;
	}

	// �����ı�
	void changeText( const QString& text );
	// ׷���ı�
	void appendText( const QString& text );
		
	QPainterPath shape() const;

	virtual void backupState();
	virtual bool isStateChanged(int type);
	virtual void copyState(QDataStream& stream, int type);

protected:
	virtual void mouseDoubleClickEvent ( QGraphicsSceneMouseEvent * event );

private:
	QString comment_;  // ע���ı�
	QColor textColor_; // �ı���ɫ


	QColor oldColor_;
	QString oldComment_;
};


#endif
