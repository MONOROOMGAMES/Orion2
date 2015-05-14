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
#ifndef  ORION_SPRITESEQUENCE_H
#define ORION_SPRITESEQUENCE_H

#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <QTimeLine>
#include <boost/shared_ptr.hpp>
#include <vector>
#include "editordefine.h"
#include "spritedefine.h"
#include "spriteobject.h"

#include "resourceobject.h"
using namespace ORION;


class SpriteSequenceFrame : public SpriteObject{
	Q_OBJECT
public:
	// ʵ������
	ORION_TYPE_DECLARE(SPRITE_SEQUENCEFRAME);

	SpriteSequenceFrame(SequenceFrame* frame, QGraphicsItem * parent = 0);
	virtual ~SpriteSequenceFrame();

	//  ���غ���
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	//  ˢ��ͼƬ
	void refreshImage();

	// ͼƬ��С
	QSize imageSize();

	// ͼƬ
	QImage& image();

	// ������ê��Ϊԭ��������С
	QRect anchorRegion();

	// �ָ��Ϸ�ǰ״̬
	bool restoreDrag();

	// ͼƬ�Ƿ���Ч
	bool isValidImage(){
		return  !frame_->buffer.isNull();
	}

	// ��������
	void updateInstance();

	SequenceFrame* getFrame() const{
		return frame_;
	}
protected:
	virtual void	mouseMoveEvent(QGraphicsSceneMouseEvent * event);
	virtual void	mousePressEvent(QGraphicsSceneMouseEvent * event);
	virtual void	mouseReleaseEvent(QGraphicsSceneMouseEvent * event);
	virtual  void  contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);

	QRectF imgBound_;  // ͼƬ��������
	SequenceFrame* frame_;  // ����֡
	QImage errorimage_;  // ����ͼƬ
	QGraphicsItem* oldParent_; // �����У����ڳ����Ϸţ�
	int oldIndex_;  

	// ����ê��
	void drawAnchor(QPainter* painter);

};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class SpriteSequence : public SpriteObject{
	Q_OBJECT
public:
	// ʵ������
	ORION_TYPE_DECLARE(SPRITE_SEQUENCE);

	SpriteSequence(SpriteSequenceResourceObject* instance, QGraphicsItem * parent = 0);
	virtual ~SpriteSequence();
	//  ��ȡ������
	virtual QString title();
	//  ���غ���
	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	// ����ʵ��
	void  updateInstance();

	// ˢ�¶���֡����������ê��ʹ�С��
	void refreshFrames();

	// ��ȡ����ͼ���С
	QSize acquireFrameMaxSize();

	// ��������λ��
	void adjust();

	// �Ƴ���ǰ֡�������Ƴ�ǰ������
	int removeFrame(SpriteSequenceFrame* frame);
	// ����ɾ��
	void deleteFrame(SpriteSequenceFrame* frame);
	// ��Ӷ���֡
	void addFrame(SpriteSequenceFrame* frame, int index = -1);

	// ����֡��
	int frameCount(){
		return frames_.size();
	}
	// ���ò����ٶ�
	void setPlaySpeed(qreal speed);
	void setPlayBack(bool flag = true);
	void setPingPong(bool flag = true);
	void setRepeat(bool flag = true);
	void setRepeatCount(int count = 1);
	void setRepeatTo(int index = 0);

	void playAnim( bool enable = true);
	void resetAnim();

	bool isPlaying();
	bool isPausing();
	bool isPlayBack();
	qreal playSpeed();

	bool isPingpong();
	bool isRepeat();
	int repeatCount();
	int repeatTo();


	SpriteSequenceResourceObject* resourceInstance(){
		return instance_;
	}

	int currentAnchor(){
		return currentAnchor_;
	}

	void setCurrentAnchor(int id){
		currentAnchor_ = id;
	}

protected:
	virtual void	dragEnterEvent(QGraphicsSceneDragDropEvent * event);
	virtual void	dragLeaveEvent(QGraphicsSceneDragDropEvent * event);
	virtual void	dragMoveEvent(QGraphicsSceneDragDropEvent * event);
	virtual void	dropEvent(QGraphicsSceneDragDropEvent * event);
	virtual  void  contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	virtual QVariant itemChange(GraphicsItemChange change, const QVariant & value);
	virtual void	mousePressEvent(QGraphicsSceneMouseEvent * event);
	virtual void	mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event);

private:
	SpriteSequenceResourceObject* instance_;
	QList<SpriteSequenceFrame*> frames_;  // ����֡����

	int insertAnchor_;  // ����λ�ñ��

	QRectF imgBound_;  // ͼƬ��������
	QPointF imgOrigin_;  // ��������ԭ��
	QTimeLine *timeLine_; // ����
	int currrentFrame_;  // ��ǰ���Ŷ���֡

	// ê��
	ResourceHash currentAnchor_;  // ��ǰ��ʾ��ê��
	QMap<ResourceHash, QRectF> anchorBounds_;  // ê���������

	void initFrames();
	// ��ʼ������
	void initAnimation();

	// ����ê��
	void drawAnchor(QPainter* painter);

private slots:
	void setValue(int value);
};


#endif
