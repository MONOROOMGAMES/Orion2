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
#ifndef ORION_EDITORITEM_H
#define ORION_EDITORITEM_H

#include <QGraphicsItem>
#include <QGraphicsBlurEffect>
#include <QFont>
#include <QVector3D>
#include <QDataStream>
#include <QByteArray>
#include <QPainterPath>
#include "editordefine.h"
#include "resourceobject.h"
using namespace ORION;

class EditorItem;

class EditorAnchor : public QGraphicsItem{
public:
	// ʵ������
	ORION_TYPE_DECLARE(EDITOR_ANCHOR);

	EditorAnchor(EditorItem *host, int index, QGraphicsItem* parent = NULL);
	virtual ~EditorAnchor();

	//  ���غ���
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	void setAnchorIndex(int index){
		anchorIndex_ = index;
	}

	int anchorIndex(){
		return anchorIndex_;
	}
private:

protected:
	virtual void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
	virtual void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

protected:
	EditorItem   *host_;   // ����
	QRectF  boundingBox_;     // ��������		
	bool underMouse_;
	int anchorIndex_;  // ���Ƶ�������
	bool hostIsLock_;  

	QByteArray oldValues_;

};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
// ��������Ԫ�������п�����ʾһ�����������ز�ͼƬ����Ƶ����������Ƶ��������������
class EditorItem : public QGraphicsItem{
	friend class EditorAnchor;

public:
	// ʵ������
	ORION_TYPE_DECLARE(EDITOR_ITEM)

	EditorItem(QGraphicsItem * parent = 0);
	EditorItem(const QRectF& rect, const QSizeF& origin = QSizeF(0.5,0.5), QGraphicsItem * parent = 0);
	virtual ~EditorItem();

	ResourceHash iid(){
		return iid_;
	}

	//  ���غ���
	QRectF boundingRect() const;
	QPainterPath shape() const;
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	// �������
	QByteArray encode( bool def = false );
	void decode( QByteArray& data, bool def =false );

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	// ����
	virtual void encodeData(QDataStream& stream);
	virtual void decodeData(QDataStream& stream);
	// ����
	virtual void clear(){};
	// ���º���
	virtual void updateChildren(){}
	// ���ʼ��
	virtual void postinit(bool resize = true);

	// ȱʡ����
	virtual void encodeDefault(QDataStream& stream);
	virtual void decodeDefault(QDataStream& stream);

	// ����
	bool isLocking();
	void toggleLock();

	// ���ñ�־
	void setSizeFlag(int flag){
		sizeFlags_ = flag;
	}
	
	virtual void resize(const QSizeF& newSize, bool update = true);

	// ����֧�ֵ�ê��ID
	QList<int> anchors();
	// ����ê��λ��
	QPointF anchorPos(int id, const QRectF& bound );

	// ����ê��
	virtual  QPointF updateAnchor(int id, const QPointF& pos);
	
	QRectF selectBound();
	// ������Ƶ�
	virtual void clearAnchor();

	bool initAnchor();
	void anchorChanged(int id, const QPointF& pos);
	void showAnchor(bool visible);
	void hideAnchor();
	void updateAnchor(const QList<int>& except = QList<int>());

	void setAlpha(qreal a);

	qreal getAlpha(){
		return opacity_;
	}

	static ResourceHash makeUniqueId();
	void adjustSize(QSize& newSize, bool keepAspect = true);
	QPointF rotateAnchor();

	void cacheAspect(){
		oldAspect_ = boundingBox_.size();
	}

protected:
	virtual void keyPressEvent(QKeyEvent * event);
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
protected:
	QRectF  boundingBox_;  // ��������
	bool isLocking_;  // �Ƿ�����
	int sizeFlags_;  // �ı��С��־
	ResourceHash iid_;  // ΨһID
	int anchorId_;  // ��ǰanchorid
	GraphicsItemFlags oldFlags_;
	QPointF hotspot_;  // ê��λ�ã�0,0���Ͻǣ�0.5,0.5���ģ�
	bool canflip_;  // ͼ���Ƿ�֧�ַ�ת�����Ϊ����

	qreal opacity_;  // ͸����
	unsigned int version_;  // ���ݰ汾

	bool isAdjusting_;  // ���ڵ���λ��
	QSizeF oldAspect_;  // ���ű���
	// ��ʼ��
	void init();
	QMap<int, EditorAnchor*> anchors_;  // anchor����

	QByteArray oldValues_;
};


#endif // ORION_EDITORREGION_H
