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
#ifndef ORION_EDITORTEXT_H
#define ORION_EDITORTEXT_H

#include <QGraphicsItem>
#include <QFont>
#include <QVector3D>
#include <QDataStream>
#include <QByteArray>
#include <QPainterPath>
#include "leveldefine.h"
#include "editorregion.h"
#include "resourceobject.h"
using namespace ORION;

class EditorText : public EditorRegion{
public:
	// ʵ������
	ORION_TYPE_DECLARE(LEVEL_TEXT)

	EditorText(ObjectTypeResourceObject* prototype, QGraphicsItem * parent = 0);
	virtual ~EditorText();

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	// ����
	virtual void encodeData(QDataStream& stream);
	virtual void decodeData(QDataStream& stream);
	// ȱʡ����
	virtual void encodeDefault(QDataStream& stream);
	virtual void decodeDefault(QDataStream& stream);

	// ˢ��
	void postinit(bool resize = true);

	// ��������
	void parseFont(const QString& fnt);

	QString text_;
	int lineHeight_;
	QFont font_;
	QColor color_;  // ������ɫ
	int flags_;  // ��ʾ��־

	// ȱʡ����
	static QFont defaultFont();

};


#endif // ORION_EDITORIMAGE_H
