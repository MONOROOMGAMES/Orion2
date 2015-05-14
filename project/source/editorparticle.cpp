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
#include "editorparticle.h"
#include <QtGui>
#include <QDialog>
#include <QVector2D>
#include <math.h>
#include "levelscene.h"
#include "commondefine.h"
#include "componentdefine.h"
#include "resourcelinker.h"
#include "spritedefine.h"
using namespace ORION;

EditorParticle::EditorParticle(ObjectTypeResourceObject* prototype, QGraphicsItem * parent)
	:EditorRegion(prototype, parent){

	boundingBox_.setRect(0, -64, 128, 128);
	hotspot_ = QPointF(0, 0.5);
	canflip_ = false;
	setAcceptDrops(true);
	// ������TileBG���ʵ��
	if (prototype){
		Q_ASSERT(hashStringLower(typePrototype_->prototype_->objectName()) == COMP_PARTICLES);
		postinit();
	}
}

EditorParticle::~EditorParticle(){
}

void EditorParticle::drawShape(QPainter *painter){

	painter->setOpacity(opacity_);

	// ���û��ָ������������Ʊ�����
	if (typePrototype_->staticImage_ != NULL){
		if (boundingBox_.width() > 0 && boundingBox_.height() > 0){
			// ���л�������ͼ��
			painter->drawImage(boundingBox_.left() + (boundingBox_.width() - tileImage_.width()) / 2,
				boundingBox_.top() + (boundingBox_.height() - tileImage_.height()) / 2,
				tileImage_);
		}
	}

	painter->fillRect(boundingBox_, SPRITE_FILLCOLOR);
	// ����׶��
	painter->setPen(Qt::red);
	painter->save();
	painter->rotate(cone_ / 2);
	painter->drawLine(boundingBox_.left(), boundingBox_.center().y(), boundingBox_.left() + 64, boundingBox_.center().y());
	painter->rotate(-cone_ );
	painter->drawLine(boundingBox_.left(), boundingBox_.center().y(), boundingBox_.left() + 64, boundingBox_.center().y());
	painter->restore();

	// �����ѡ��״̬
	if (isSelected()){
		QPen pen(Qt::red);
		painter->setPen(pen);
		painter->drawRect(boundingBox_);
	}

}

void	EditorParticle::dropEvent(QGraphicsSceneDragDropEvent * event){
	if (event->mimeData()->hasFormat(MIME_ORION_GALLERY)){
		QByteArray itemData = event->mimeData()->data(MIME_ORION_GALLERY);
		QDataStream stream(&itemData, QIODevice::ReadOnly);

		LevelScene* level = (LevelScene*)scene();
		// ��ȡ������
		int index;
		stream >> index;
		Q_ASSERT(index == 1);
		ResourceHash key;
		stream>>key;

		if (typePrototype_->setImageResource(key)){
			postinit();
			update();
		}
	
		event->accept();
	}

	EditorRegion::dropEvent(event);
}

void EditorParticle::postinit(bool resize){

	if (typePrototype_->staticImage_ != NULL){  // ��̬ͼƬ������ê��ʼ��Ϊ����
		tileImage_ = typePrototype_->staticImage_->image_;
	}


	cone_ = getParameterDouble(UDQ_T("SprayCone"), typePrototype_->prototype_->objectName());

	isAdjusting_ = true;
	updateAnchor();
	isAdjusting_ = false;
}

void EditorParticle::dragEnterEvent(QGraphicsSceneDragDropEvent * event){

	if (event->mimeData()->hasFormat(MIME_ORION_GALLERY)){
		setSelected(true);
		update();
		event->accept();
		return;
	}

	EditorRegion::dragEnterEvent(event);
}

void EditorParticle::dragLeaveEvent(QGraphicsSceneDragDropEvent * event){
	if (event->mimeData()->hasFormat(MIME_ORION_GALLERY)){
		setSelected(false);
		update();
		event->accept();
		return;
	}
	EditorRegion::dragLeaveEvent(event);
}

void EditorParticle::dragMoveEvent(QGraphicsSceneDragDropEvent * event){
	EditorRegion::dragMoveEvent(event);
}
