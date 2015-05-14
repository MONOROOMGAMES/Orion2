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
#ifndef ORION_EDITORLEVEL_H
#define ORION_EDITORLEVEL_H

#include "editoritem.h"
#include <QGraphicsBlurEffect>
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <QVector3D>
#include <QDataStream>
#include <QByteArray>
#include <QPainterPath>
#include "leveldefine.h"
#include "resourceobject.h"
using namespace ORION;

class EditorLevelText : public QGraphicsSimpleTextItem{

public:
	ORION_TYPE_DECLARE(LEVEL_VIEWPORT_TEXT);
	EditorLevelText(const QString& text, QPointF pos, QGraphicsItem *parent = 0);
};


// ��������Ԫ�������п�����ʾһ�����������ز�ͼƬ����Ƶ����������Ƶ��������������
class EditorLevel : public EditorItem{
public:
	// ʵ������
	ORION_TYPE_DECLARE(LEVEL_PROFILE)
	EditorLevel(LevelResourceObject* level, QGraphicsItem * parent = 0);
	virtual ~EditorLevel();

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	// ���Ƴ�����߽�
	void drawMargin(QPainter *painter);
	virtual void resize(const QSizeF& newSize, bool update = true);

	// ���º���
	virtual void updateChildren();
	void updateText();

	void enableEdit(bool flag){
		setSizeFlag(flag ? ANCHORFLAG_RESIZE_R : ANCHORFLAG_NONE);
	}

	// �л�ѡ��ģʽ
	bool toggleSelectable();

	QColor fillColor_; // ���ɫ
	QColor marginfillColor_; // �߽����ɫ
	int flags_;  // ���

	LevelResourceObject* level_;  
	EditorLevelText* text_;  // �ı�

	// ��ȡ�ı�λ��
	QPointF textPos();

	// ˢ������
	void refresh();

protected:
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant &value);

protected:
	// ��ȡ��ǰ�ַ���
	QString currentTitleString();
	void adjustText();
};


#endif // ORION_EDITORREGION_H
