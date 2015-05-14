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
#ifndef ORION_EDITORREGION_H
#define ORION_EDITORREGION_H

#include "editoritem.h"
#include <QGraphicsBlurEffect>
#include <QFont>
#include <QVector3D>
#include <QDataStream>
#include <QByteArray>
#include <QPainterPath>
#include "editordefine.h"
#include "resourceobject.h"
using namespace ORION;

// ��������Ԫ�������п�����ʾһ�����������ز�ͼƬ����Ƶ����������Ƶ��������������
class EditorRegion : public EditorItem{
public:
	// ʵ������
	ORION_TYPE_DECLARE(EDITOR_REGION)
	EditorRegion(ObjectTypeResourceObject* prototype, QGraphicsItem * parent = 0);
	EditorRegion(const QRectF& rect,  const QSizeF& origin = QSizeF(0.5, 0.5), QGraphicsItem * parent = 0);
	virtual ~EditorRegion();

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	// ����
	virtual void encodeData(QDataStream& stream);
	virtual void decodeData(QDataStream& stream);
	// ȱʡ����
	virtual void encodeDefault(QDataStream& stream);
	virtual void decodeDefault(QDataStream& stream);

	void saveDefault();
	void loadDefault();

	// �ƶ���ͼ��
	void changeToLayer(const QString& layer);

	// ��ȡ��͸����
	qreal getParentOpacity();

	void setParameter(const QString& prop, const QString& value, const QString& plugin = QString());

	QString getParameterString(const QString& name, const QString& plugin = QString());
	int getParameterInt(const QString& name, const QString& plugin = QString());
	double getParameterDouble(const QString& name, const QString& plugin = QString());
	bool getParameterBool(const QString& name, const QString& plugin = QString());
	int getParameterEnum( const QString& name, const QString& plugin );

	int blend_; // ���ģʽ
	int fallback_;  // �ص�����
	QColor borderColor_; // �߿���ɫ
	//
	ObjectTypeResourceObject* typePrototype_;  // ����ԭ��
	QMap<ResourceHash, QString> parameters_; // ʵ��������keyΪ pluginName.paramName��

protected:
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);

	bool boundScene_;  // �Ƿ������ڳ�����
};


#endif // ORION_EDITORREGION_H
