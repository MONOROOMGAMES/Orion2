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
#include "editorinput.h"
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

EditorInputButton::EditorInputButton(ObjectTypeResourceObject* prototype, QGraphicsItem * parent)
	:EditorRegion(prototype, parent){

	boundingBox_.setRect(0, 0, 128, 32);
	hotspot_ = QPointF(0, 0);
	canflip_ = false;
	setAcceptDrops(false);
	setSizeFlag(ANCHORFLAG_RESIZE);

	if (prototype){
		Q_ASSERT(hashStringLower(typePrototype_->prototype_->objectName()) == COMP_BUTTON);
		postinit();
	}
}

EditorInputButton::~EditorInputButton(){
}

void EditorInputButton::drawShape(QPainter *painter){
	painter->setPen(Qt::black);
	painter->fillRect(boundingBox_, Qt::gray);
	int flags = Qt::AlignVCenter;
	QRectF textRect = boundingBox_;
	// ��������������
	if (inputtype_ == 0){  // ��ť��������ʾ
		flags |= Qt::AlignHCenter;
	}
	else if (inputtype_ == 1){  // ��ѡ��
		// ���ƿ�ѡ��
		QRectF header = QRectF(8, 8, 16, 16);
		painter->drawRect(header);

		if (checked_){
			painter->fillRect(header.adjusted(2.5, 2.5, -2.5, -2.5),Qt::black);
		}
		// ��������
		textRect.adjust(32, 0, 0, 0);
		textRect.setHeight(32);
	}
	painter->drawText(textRect, flags, text_);

	// �����ѡ��״̬
	if (isSelected()){
		QPen pen(Qt::red);
		painter->setPen(pen);
		painter->drawRect(boundingBox_);
	}

}

void EditorInputButton::postinit(bool resize){
	// ���ô�С
	text_ = getParameterString(UDQ_T("Text"), typePrototype_->prototype_->objectName());

	// ��ť����
	inputtype_ = getParameterEnum(UDQ_T("Type"), typePrototype_->prototype_->objectName());

	if (text_.isEmpty()){
		text_ = UDQ_TR("Button");
	}

	// ��ѡ״̬
	checked_ = getParameterBool(UDQ_T("Checked"), typePrototype_->prototype_->objectName());

}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

EditorInputTextBox::EditorInputTextBox(ObjectTypeResourceObject* prototype, QGraphicsItem * parent)
	:EditorRegion(prototype, parent){

	boundingBox_.setRect(0, 0, 128, 32);
	hotspot_ = QPointF(0, 0);
	canflip_ = false;
	setAcceptDrops(false);
	setSizeFlag(ANCHORFLAG_RESIZE);

	if (prototype){
		Q_ASSERT(hashStringLower(typePrototype_->prototype_->objectName()) == COMP_TEXTBOX);
		postinit();
	}
}

EditorInputTextBox::~EditorInputTextBox(){
}

void EditorInputTextBox::drawShape(QPainter *painter){

	painter->setPen(Qt::black);
	painter->fillRect(boundingBox_, Qt::gray);
	int flags = Qt::AlignLeft | Qt::AlignTop | Qt::TextSingleLine; 
	QRectF textRect = boundingBox_.adjusted(2,2,-2,-2);
	QString text = text_.isEmpty() ? placeholder_ : text_;

	// ��������������
	if (inputtype_ == 6){  // ����
		flags &= ~Qt::TextSingleLine;
		flags |= Qt::TextWrapAnywhere;
	}
	else if (inputtype_ == 1){  // ����
		for(int i = 0; i < text.size(); i++){
			text[i] = UDQ_C('*');
		}
	}
	painter->drawText(textRect, flags, text);

	// �����ѡ��״̬
	if (isSelected()){
		QPen pen(Qt::red);
		painter->setPen(pen);
		painter->drawRect(boundingBox_);
	}

}

void EditorInputTextBox::postinit(bool resize){
	// ���ô�С
	text_ = getParameterString(UDQ_T("Text"), typePrototype_->prototype_->objectName());

	placeholder_ = getParameterString(UDQ_T("Placeholder"), typePrototype_->prototype_->objectName());
	// ����
	inputtype_ = getParameterEnum(UDQ_T("Type"), typePrototype_->prototype_->objectName());

}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

EditorInputList::EditorInputList(ObjectTypeResourceObject* prototype, QGraphicsItem * parent)
	:EditorRegion(prototype, parent){

	boundingBox_.setRect(0, 0, 128, 128);
	hotspot_ = QPointF(0, 0);
	canflip_ = false;
	setAcceptDrops(false);
	setSizeFlag(ANCHORFLAG_RESIZE);

	icon_.load(UDQ_T(":/images/dropdown.png"));

	if (prototype){
		Q_ASSERT(hashStringLower(typePrototype_->prototype_->objectName()) == COMP_LIST);
		postinit();
	}
}

EditorInputList::~EditorInputList(){
}

void EditorInputList::drawShape(QPainter *painter){
	painter->setPen(Qt::black);
	painter->fillRect(boundingBox_, Qt::gray);

	int height = 24;

	if (inputtype_ == 0){  // �б�
		int offy = 0;
		int flags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine;
		QRectF textRect;
		// ��������������
		foreach(QString item, items_){
			textRect = QRectF(4, offy, boundingBox_.width(), height);
			painter->drawText(textRect, flags, item);
			offy += height;
		}
	}
	else if (inputtype_ == 1){  // ������

		if (items_.size() > 0){
			int flags = Qt::AlignLeft | Qt::AlignVCenter | Qt::TextSingleLine;
			QRectF textRect = QRectF(4, 0, boundingBox_.width() - 32, boundingBox_.height());
			painter->drawText(textRect, flags, items_.at(0));
		}

		// ������
		QRectF dropRect = QRectF(boundingBox_.width() - 32, 0, 32, boundingBox_.height());
		painter->fillRect(dropRect, Qt::darkGray);

		if (boundingBox_.height() >= 8 && boundingBox_.width() >= 32 ){
			// ������־
			painter->drawImage(QRectF(boundingBox_.width() - 20, (boundingBox_.height() - 6) / 2, 8, 6), icon_);
		}
	}

	// �����ѡ��״̬
	if (isSelected()){
		QPen pen(Qt::red);
		painter->setPen(pen);
		painter->drawRect(boundingBox_);
	}

}

void EditorInputList::postinit(bool resize){
	// �б���
	QString items = getParameterString(UDQ_T("Items"), typePrototype_->prototype_->objectName());
	items_ = items.split(UDQ_T(";"));

	// ����
	inputtype_ = getParameterEnum(UDQ_T("Type"), typePrototype_->prototype_->objectName());

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

EditorInputFileChooser::EditorInputFileChooser(ObjectTypeResourceObject* prototype, QGraphicsItem * parent)
	:EditorRegion(prototype, parent){

	boundingBox_.setRect(0, 0, 128, 32);
	hotspot_ = QPointF(0, 0);
	canflip_ = false;
	setAcceptDrops(false);
	setSizeFlag(ANCHORFLAG_RESIZE);

	if (prototype){
		Q_ASSERT(hashStringLower(typePrototype_->prototype_->objectName()) == COMP_FILECHOOSER);
		postinit();
	}
}

EditorInputFileChooser::~EditorInputFileChooser(){
}

void EditorInputFileChooser::drawShape(QPainter *painter){

	// ��������������
	painter->setPen(Qt::black);
	painter->fillRect(boundingBox_, Qt::gray);

	int flags = Qt::AlignCenter;
	QRectF textRect = boundingBox_.adjusted(2, 2, -2, -2);
	painter->drawText(textRect, flags, UDQ_TR("ѡ���ļ�"));

	// �����ѡ��״̬
	if (isSelected()){
		QPen pen(Qt::red);
		painter->setPen(pen);
		painter->drawRect(boundingBox_);
	}

}

void EditorInputFileChooser::postinit(bool resize){

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

EditorInputProgressBar::EditorInputProgressBar(ObjectTypeResourceObject* prototype, QGraphicsItem * parent)
	:EditorRegion(prototype, parent){

	boundingBox_.setRect(0, 0, 128, 32);
	hotspot_ = QPointF(0, 0);
	canflip_ = false;
	setAcceptDrops(false);
	setSizeFlag(ANCHORFLAG_RESIZE);

	if (prototype){
		Q_ASSERT(hashStringLower(typePrototype_->prototype_->objectName()) == COMP_PROGRESSBAR);
		postinit();
	}
}

EditorInputProgressBar::~EditorInputProgressBar(){
}

void EditorInputProgressBar::drawShape(QPainter *painter){

	// ��������������
	painter->setPen(Qt::black);
	painter->fillRect(boundingBox_, Qt::gray);

	QRectF perRect = boundingBox_;
	perRect.setWidth(boundingBox_.width() * percent_ / 100.0);
	painter->fillRect(perRect, QColor(50, 50, 255, 255));

	// �ٷֱ�
	int flags = Qt::AlignCenter;
	QRectF textRect = boundingBox_.adjusted(2, 2, -2, -2);
	painter->drawText(textRect, flags, UDQ_TR("%1%").arg(percent_));


	// �����ѡ��״̬
	if (isSelected()){
		QPen pen(Qt::red);
		painter->setPen(pen);
		painter->drawRect(boundingBox_);
	}

}

void EditorInputProgressBar::postinit(bool resize){

	//����ٷֱ�
	qreal value = getParameterDouble(UDQ_T("Value"), typePrototype_->prototype_->objectName());
	qreal maxvalue = getParameterDouble(UDQ_T("Maximum"), typePrototype_->prototype_->objectName());

	if (maxvalue < 0){
		maxvalue = 0;
	}

	value = clampValuef(value, 0, maxvalue);

	if (maxvalue == 0){
		percent_ = 0;
	}
	else{
		percent_ = qRound(value / maxvalue * 100);
	}

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

EditorInputSliderBar::EditorInputSliderBar(ObjectTypeResourceObject* prototype, QGraphicsItem * parent)
	:EditorRegion(prototype, parent){

	boundingBox_.setRect(0, 0, 128, 32);
	hotspot_ = QPointF(0, 0);
	canflip_ = false;
	setAcceptDrops(false);
	setSizeFlag(ANCHORFLAG_RESIZE);
	icon_.load(UDQ_T(":/images/button.png"));

	if (prototype){
		Q_ASSERT(hashStringLower(typePrototype_->prototype_->objectName()) == COMP_SLIDERBAR);
		postinit();
	}
}

EditorInputSliderBar::~EditorInputSliderBar(){
}

void EditorInputSliderBar::drawShape(QPainter *painter){

	// ��������������
	painter->setPen(Qt::black);
	painter->fillRect(boundingBox_, Qt::gray);

	// ���ƻ���
	QRectF rulerRect = QRectF(8, (boundingBox_.height() - 8) / 2, boundingBox_.width() - 16, 8);
	painter->fillRect(rulerRect, Qt::darkGray);

	if (gridnum_ > 0){  // ���ƿ̶�

	}

	QRectF cursorRect = QRectF(0, 0, 8, 16);
	if (value_ == 0 ||gridnum_ == 0){
		cursorRect.moveCenter(QPointF(rulerRect.left(), rulerRect.center().y()));
	}
	else{
		QRectF fillRect = rulerRect;
		fillRect.setWidth(rulerRect.width() * value_ *1.0 / gridnum_);
		painter->fillRect(fillRect, QColor(50, 50, 255, 255));

		cursorRect.moveCenter(QPointF(rulerRect.left() + rulerRect.width() * value_ *1.0 / gridnum_, rulerRect.center().y()));
	}
	painter->drawImage(cursorRect, icon_);

	// �����ѡ��״̬
	if (isSelected()){
		QPen pen(Qt::red);
		painter->setPen(pen);
		painter->drawRect(boundingBox_);
	}

}

void EditorInputSliderBar::postinit(bool resize){
	//����ٷֱ�
	qreal value = getParameterDouble(UDQ_T("Value"), typePrototype_->prototype_->objectName());
	qreal minvalue = getParameterDouble(UDQ_T("Minimum"), typePrototype_->prototype_->objectName());
	qreal maxvalue = getParameterDouble(UDQ_T("Maximum"), typePrototype_->prototype_->objectName());
	qreal step = getParameterDouble(UDQ_T("Step"), typePrototype_->prototype_->objectName());

	if (step == 0){  // ���ܵ���0
		step = 1;
	}

	if (minvalue > maxvalue ){
		minvalue = maxvalue;
		value_ = 0;
		gridnum_ = 0;
	}
	else{
		value = clampValuef(value, minvalue, maxvalue);
		value_ = qRound((value - minvalue )/ step);
		gridnum_ = qRound((maxvalue - minvalue) / step);
	}

}


