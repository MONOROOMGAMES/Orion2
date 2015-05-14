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
#ifndef  ORION_EDITORTILEMAP_H
#define ORION_EDITORTILEMAP_H

#include <QGraphicsItem>
#include <QFont>
#include <QVector3D>
#include <QMap>
#include <QDataStream>
#include <QByteArray>
#include <QPainterPath>
#include <QGraphicsPixmapItem>
#include "leveldefine.h"
#include "editorregion.h"
#include "resourceobject.h"
using namespace ORION;

class EditorTileBrush : public QGraphicsItem{
public:
	ORION_TYPE_DECLARE(LEVEL_TILEBRUSH)
	EditorTileBrush(QGraphicsItem * parent = 0);
	virtual ~EditorTileBrush();

	void setImage(const QImage& img);
	void setRect(const QRect& rect);
	// ����״̬
	void updateState(int state);
	//  ���غ���
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

private:
	QRectF boundingBox_;
	QImage image_; // ͼ��
	QBitArray flip_;  //  ״̬
};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class EditorTileMap : public EditorRegion{
public:
	// ʵ������
	ORION_TYPE_DECLARE(LEVEL_TILEMAP)
	EditorTileMap(ObjectTypeResourceObject* prototype, QGraphicsItem * parent = 0);
	virtual ~EditorTileMap();

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	// ˢ��
	void postinit(bool resize = true);

	// ����ˢ��
	void updateBrush(const QRect& region, int mode);

	// ����
	virtual void encodeData(QDataStream& stream);
	virtual void decodeData(QDataStream& stream);
	// ȱʡ����
	virtual void encodeDefault(QDataStream& stream);
	virtual void decodeDefault(QDataStream& stream);

	// ����ˢ������
	void makeBrushData(const QRect& region);
	void addTileBlock(const QPoint& pos);

	QMap<QPair<int, int>, QImage> bufferImage_;  // ͼ�񻺴�
	QImage tileImage_;  // tileͼ��
	QSize grid_;  // ���Ӵ�С
	QMap<QPair<int,int>, QSize>  mapData_;  // width ������heightΪ��ת״̬��0��7��
	QList<QRect>  brushData_;  // ˢ������
	QSize brushSize_;  // ˢ�Ӵ�С
	QBitArray state_;  // ��ת״̬��0��ת�ߵ���1��ת�ߵ���  4ˮƽ��ת  5��ֱ��ת�� 6 �ԽǷ�ת ��
	int mode_;  // ˢ��ģʽ

	bool isEditing_;  // �Ƿ����ڱ༭
protected:
	virtual void dragEnterEvent(QGraphicsSceneDragDropEvent * event);
	virtual void dragLeaveEvent(QGraphicsSceneDragDropEvent * event);
	virtual void dragMoveEvent(QGraphicsSceneDragDropEvent * event);
	virtual void dropEvent(QGraphicsSceneDragDropEvent * event);
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
	virtual void	mousePressEvent(QGraphicsSceneMouseEvent * event);
	virtual void	mouseMoveEvent(QGraphicsSceneMouseEvent * event);
	virtual void	hoverMoveEvent(QGraphicsSceneHoverEvent * event);
	virtual void	hoverEnterEvent(QGraphicsSceneHoverEvent * event);
	virtual void	hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

private:
	EditorTileBrush* brushItem_;  // ˢ��

};


#endif // ORION_EDITORIMAGE_H
