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

#ifndef ORION_IMAGEANCHOR_H
#define ORION_IMAGEANCHOR_H

#include <QGraphicsItem>
#include <QFont>
#include "spritedefine.h"
using namespace ORION;

class ImageAnchor : public QGraphicsItem{

public:
	// ʵ������
	ORION_TYPE_DECLARE(SPRITE_ANCHOR)

	ImageAnchor();
	virtual ~ImageAnchor();

	//  ���غ���
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);
protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);

private:
	QRectF  boundingBox_;     // ��������		

};


#endif // ORION_IMAGEANCHOR_H
