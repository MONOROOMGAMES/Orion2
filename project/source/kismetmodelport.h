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

#ifndef  ORION_KISMET_MODELPORT_H
#define ORION_KISMET_MODELPORT_H

#include <QFont>
#include <QList>
#include <QGraphicsProxyWidget>

#include "kismetdefine.h"
#include "kismetobject.h"
#include "kismetlink.h"

class BlueprintModelPort;

class BlueprintControl : public QGraphicsProxyWidget{
public:
	// ʵ������
	ORION_TYPE_DECLARE(KISMET_MODEL_PORT_WIDGET);

	BlueprintControl(const QString& datatype, const QStringList& enums = QStringList(), QGraphicsItem * parent = 0);
	virtual ~BlueprintControl();
	//  ���غ���
	QRectF boundingRect() const;
	void paintWindowFrame(QPainter *painter, const QStyleOptionGraphicsItem *option,
		QWidget *widget);

	// ��ȡ����ֵ
	QString value();
	// ��������ֵ
	void setValue(const QString& v);

	// �Ƿ�Ϊcombo����
	bool isComboType();

	// ��ȡֵ����������ö��������Ч
	int valueIndex(const QString& v);

protected:
	void	hoverEnterEvent(QGraphicsSceneHoverEvent * event);
	void	hoverLeaveEvent(QGraphicsSceneHoverEvent * event);

	void prepareWidget(const QString& datatype, const QStringList& enums = QStringList());
	QWidget* widget_;  // ���ݿؼ�
	QString datatype_; 
	QStringList enums_;
};



//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class  BlueprintModelPort : public BlueprintObject{
public:
	// ʵ������
	ORION_TYPE_DECLARE( KISMET_MODEL_PORT );

	enum PortType{
		EVT_CTRL_IN = 1,
		EVT_CTRL_OUT = 2,
		EVT_DATA_IN = 7,
		EVT_DATA_OUT  = 8,
	};

	BlueprintModelPort(int orient, const QString& datatype, const QString&name = QString(), const QStringList& enums = QStringList(), QGraphicsItem * parent = 0);
	virtual ~BlueprintModelPort();

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	// ���ӿؼ��İ�Χ��
	QRectF ctrlBoundBox();
	// �Ƿ�Ϊ��Ч��
	bool isNullName();

	// ��ȡ���Ƶ�
	virtual QPointF controlPoint( int orient = -1 );
	// ��ȡ��
	virtual QPointF endPoint( void );

	// ���˿����
	virtual bool checkPortPair(BlueprintModelPort* port);
	virtual bool validate(QStringList& msg);
	// ��������
	void updateLink();
	// ������ɫ
	void updateDataType();

	// ���ÿ��
	void setWidth( int width ){
		textWidth_ = width;
	}

	int getWidth( void ){
		return textWidth_;
	}

	// ��������
	void setName( const QString& name ){
		name_ = name;
	};

	QString name( void){
		return name_;
	}

	void setDescription( const QString& text ){
		desp_ = text;
	};

	QString description( void){
		return desp_;
	}

	//  ��ȡ������
	virtual QString title(){
		return name_;
	}

	int orient(){
		return orient_;
	}

	void setUnderMouse( bool flag );

	// ��ʼ׼��
	void prepare();
	// ������Сλ��
	void adjust();

	// ����������ɫ
	QColor portColor(const QString& dt);

	virtual QColor linkColor(){
		return portcolor_;
	}

	QString getValue();
	void setValue(const QString& v);


	QString dataType(){
		return dataType_;
	}

	int dataTypeId();

	QString orientString();

	// ��ȡ���ʽ
	virtual QJsonValue expNode(int dt = -1);

	virtual void copyState(QDataStream& stream, int type);

protected:
	virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
	virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
	virtual void mouseMoveEvent ( QGraphicsSceneMouseEvent * event );
	virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
	virtual void mouseReleaseEvent ( QGraphicsSceneMouseEvent * event );
	virtual QVariant itemChange ( GraphicsItemChange change, const QVariant & value );
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
protected:
	QString name_;
	int orient_; // ����
	QString dataType_;  // ��������
	qreal oldZ_;  // ��Z

	QColor portcolor_; // ��ɫ

	bool underMouse_; // ��ɫ
	bool underLink_; // �Ƿ��������
	QString desp_;  // ע��

	QPainterPath path_;  // ����ͼ��
	BlueprintControl* ctrl_;   // �ؼ��Ӷ���

	BlueprintLink* tempLink_; // ��ʱ��
	QList<QPointF> tempLineCtrlPts_; // ��ʱ���Ƶ�

	int textWidth_; //�ı����
};

#endif
