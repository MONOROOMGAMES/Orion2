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

#ifndef ORION_EDITORVIEWPORT_H
#define ORION_EDITORVIEWPORT_H

#include <QGraphicsItem>
#include <QFont>
#include <QList>

#include "leveldefine.h"

using namespace ORION;

class EditorViewportText : public QGraphicsSimpleTextItem{

public:
	ORION_TYPE_DECLARE(LEVEL_VIEWPORT_TEXT);
	EditorViewportText(const QString& text, QPointF pos, QGraphicsItem *parent = 0);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);
};

class EditorViewport : public QGraphicsItem{
public:
	// ʵ������
	ORION_TYPE_DECLARE(LEVEL_VIEWPORT)

	EditorViewport( const QString& device = QString(), qreal zoom = 1.0f, qreal rot = 0.0f, QGraphicsItem * parent = 0);
	virtual ~EditorViewport();

	//  ���غ���
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	bool isMinimized(){
		return minimized_;
	}

	// ������С��״̬
	void setMinimized(bool enable);

protected:
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
	QVariant itemChange(GraphicsItemChange change, const QVariant &value);

private:
	QRectF  boundingBox_;  // ��������
	QImage image_;
	EditorViewportText* text_;  // �ı�

	QPointF oldPos_;

	QString device_;  // �豸��
	QSize baseSize_;  // ������С
	qreal zoom_; // ���ű���
	qreal rotation_;  // ��ת�Ƕ�

	bool minimized_;  // ��С��״̬

	// ��ȡ��ǰ�ַ���
	QString currentTitleString();

	// �����ı�λ��
	void adjustText();

};

#endif // ORION_EDITORCANVAS_H
