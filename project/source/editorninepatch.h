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
#ifndef  ORION_EDITOR9PATCH_H
#define ORION_EDITOR9PATCH_H

#include <QGraphicsItem>
#include <QFont>
#include <QVector3D>
#include <QMap>
#include <QDataStream>
#include <QByteArray>
#include <QPainterPath>
#include "leveldefine.h"
#include "editorregion.h"
#include "resourceobject.h"
using namespace ORION;

class EditorNinePatch : public EditorRegion{
public:
	// ʵ������
	ORION_TYPE_DECLARE(LEVEL_9PATCH)
	EditorNinePatch(ObjectTypeResourceObject* prototype, QGraphicsItem * parent = 0);
	virtual ~EditorNinePatch();

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	// ˢ��
	void postinit(bool resize = true);

	// �и�ͼƬ
	void splitImage(QImage& img);
	QImage copyImage(QImage& img, QRect& rect);
	// ���ͼƬ
	void fillImage(QPainter* painter, int pos, const QRectF& rect, int style = 1);

	QRectF margin_;  // �߿��С
	int borderFill_;  // �߿����ģʽ��0Tile1���죩
	int centerFill_; // �������ģʽ��0Tile,1���죩
	int seam_;  // �߽��϶����
	enum{
		PATCH_LT = 0,
		PATCH_TOP,
		PATCH_RT,
		PATCH_LEFT,
		PATCH_CENTER,
		PATCH_RIGHT,
		PATCH_LB,
		PATCH_BOTTOM,
		PATCH_RB,
	};
	QMap<int, QImage> patchImages_;  // patchͼƬ

protected:
	virtual void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
	virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent * event);
	virtual void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
	virtual void dropEvent(QGraphicsSceneDragDropEvent * event);

};


#endif // ORION_EDITORIMAGE_H
