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

#ifndef ORION_IMAGESTRUCTURE_H
#define ORION_IMAGESTRUCTURE_H

#include <QGraphicsItem>
#include <QFont>

#include "spritedefine.h"
#include "resourceobject.h"
using namespace ORION;

class ImageStructure : public QGraphicsItem{

public:
	// ʵ������
	ORION_TYPE_DECLARE(SPRITE_IMAGE)

	ImageStructure(double width, double height, double depth = DEPTH_SPRTIE_IMAGE);
	virtual ~ImageStructure();

	void setBoundingRect( const QRectF& newRect );
	//  ���غ���
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	// ����ԭʼͼ��
	void setImage(const QImage& img);

	// ����ͼ��ת״̬
	void updateState(const QBitArray& state);

	// ����
	void adjust();
	// 
	void bufferImage();

	//  ͼ��״̬
	QBitArray imageState(){
		return state_;
	}

	// ��ת��ͼ���С
	QSize imageSize();
	// ��ת��ͼ��
	QImage getImage(bool trans = true);

	QPointF offset_;  //  ��ʼλ��

private:
	QRectF  boundingBox_;     // ��������		
	QImage  tile_;  //
	QImage  buffer_;  //  ����
	QBitArray state_;  // ״̬

};


#endif // ORION_IMAGESTRUCTURE_H
