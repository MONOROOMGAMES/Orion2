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
#ifndef  ORION_KISMTE_OBJECT_H
#define ORION_KISMTE_OBJECT_H

#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include<QJsonValue>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "kismetdefine.h"
#include "resourceobject.h"
using namespace ORION;

//////////////////////////////////////////////////////////////////////////
class BlueprintLink;
class BlueprintGroup;

class BlueprintObject  : public QGraphicsItem{
public:
	// ʵ������
	ORION_TYPE_DECLARE( KISMET_OBJECT );

	enum{
		OF_CAN_DELETE = 1,  //  ��ɾ��
		OF_NO_SAVE = 2,  // ���ɱ���
	};

	BlueprintObject(double depth = 0, QGraphicsItem * parent = 0);
	virtual ~BlueprintObject();

	ResourceHash iid(){
		return iid_;
	}

	void setIID(ResourceHash iid){
		iid_ = iid;
	}

	// ��Գ���������
	virtual QPointF sceneCenter( void );

	//  ��ȡ������
	virtual QString title(){
		return QString();
	}

	// ���ͱ�ʾ
	virtual QString typeName(){
		return QString();
	}

	//  ��ȡ����ͼ��64,64��
	virtual QImage thumbnail();

	// ��ȡ���Ƶ�( -1��ʾ���ܿ���)
	virtual QPointF controlPoint( int orient = -1 ){
		return QPointF();
	};

	// �������
	void addLink(BlueprintLink *line);
	// ɾ������
	void removeLink(BlueprintLink *line);
	// �������
	void clearLink();

	// ����
	QList<BlueprintLink *> links() const{
		return lineList_;
	}

	bool hasLink(){
		return lineList_.size() > 0;
	}

	BlueprintGroup* eventGroup(){
		return evtgroup_;
	}

	void setEventGroup(BlueprintGroup* grp){
		evtgroup_ = grp;
	}

	//  �򵥻���
	bool canSimpleDraw();

	void addGroup(BlueprintGroup* grp);

	//  ���غ���
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	virtual QColor linkColor(){
		return Qt::black;
	}

	// �������
	QByteArray encode();
	void decode(QByteArray& data);

	// ����
	virtual void encodeData(QDataStream& stream);
	virtual void decodeData(QDataStream& stream);

	virtual void postInit(){};
	// ��ȡ���ʽ
	virtual QJsonValue expNode(int dt = -1){
		return QJsonValue();
	}
	// �Լ�
	virtual bool validate(QStringList& msg){
		isError_ = false;
		return !isError_;
	}

	unsigned int objectFlags() const{
		return objectflags_;
	}

	void setObjectFlags(unsigned int newFlags){
		objectflags_ |= newFlags;
	}

	void clearObjectFlags(unsigned int newFlags){
		objectflags_ &= ~newFlags;
	}

	bool canDelete(){
		return objectflags_ & OF_CAN_DELETE;
	}

	static QFont nameFont();
	static QFont textFont();
	static ResourceHash makeUniqueId();

	void setError(bool flag){
		isError_ = flag;
	}

	bool hasError(){
		return isError_;
	}

	// ״̬ö��
	enum{
		BST_MOVE,
		BST_RESIZE,
		BST_DISPLAY,
	};

	virtual void backupState();
	virtual bool isStateChanged( int type );
	virtual void copyState(QDataStream& stream, int type);

protected:
	virtual void hoverEnterEvent ( QGraphicsSceneHoverEvent * event );
	virtual void hoverLeaveEvent ( QGraphicsSceneHoverEvent * event );
	virtual QVariant itemChange ( GraphicsItemChange change, const QVariant & value );
	virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );	

protected:
	QRectF boundingBox_;
	QList<BlueprintLink *>    lineList_;
	BlueprintGroup* evtgroup_;  // ������
	ResourceHash iid_;  // ΨһID
	int objectflags_;  // ��־
	unsigned int version_;  // �汾

	//  �ǳ־�
	bool isError_;  // ����
	// undo
	QPointF oldPos_;  // �ɵ�λ��
	QSizeF oldSize_;  // �ɵĴ�С
	ResourceHash oldGroup_;  // �ɵ��¼���
};


#endif
