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
#ifndef  ORION_EDITORINPUT_H
#define ORION_EDITORINPUT_H

#include <QGraphicsItem>
#include <QFont>
#include <QVector3D>
#include <QMap>
#include <QDataStream>
#include <QByteArray>
#include <QPainterPath>
#include "leveldefine.h"
#include "editorregion.h"
#include "resourceobject.h"
using namespace ORION;

class EditorInputButton : public EditorRegion{
public:
	// ʵ������
	ORION_TYPE_DECLARE(LEVEL_INPUT_BUTTON)
	EditorInputButton(ObjectTypeResourceObject* prototype, QGraphicsItem * parent = 0);
	virtual ~EditorInputButton();

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	void postinit(bool resize = true);

private:
	int inputtype_;  // ����
	QString text_;  //����
	bool checked_;  // ��ѡ״̬

};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class EditorInputTextBox : public EditorRegion{
public:
	// ʵ������
	ORION_TYPE_DECLARE(LEVEL_INPUT_TEXTBOX)
	EditorInputTextBox(ObjectTypeResourceObject* prototype, QGraphicsItem * parent = 0);
	virtual ~EditorInputTextBox();

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	void postinit(bool resize = true);

private:
	QString text_;
	QString placeholder_;
	int inputtype_;  // ����
};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class EditorInputList : public EditorRegion{
public:
	// ʵ������
	ORION_TYPE_DECLARE(LEVEL_INPUT_LIST)
	EditorInputList(ObjectTypeResourceObject* prototype, QGraphicsItem * parent = 0);
	virtual ~EditorInputList();

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	void postinit(bool resize = true);

private:
	QStringList items_;
	int inputtype_;  // ����
	QImage icon_;  // ������ͷ
};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class EditorInputFileChooser : public EditorRegion{
public:
	// ʵ������
	ORION_TYPE_DECLARE(LEVEL_INPUT_FILECHOOSER)
	EditorInputFileChooser(ObjectTypeResourceObject* prototype, QGraphicsItem * parent = 0);
	virtual ~EditorInputFileChooser();

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	void postinit(bool resize = true);
};


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class EditorInputProgressBar : public EditorRegion{
public:
	// ʵ������
	ORION_TYPE_DECLARE(LEVEL_INPUT_PROGRESSBAR)
	EditorInputProgressBar(ObjectTypeResourceObject* prototype, QGraphicsItem * parent = 0);
	virtual ~EditorInputProgressBar();

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	void postinit(bool resize = true);

private:
	int percent_;  // ���Ȱٷֱ�
};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class EditorInputSliderBar : public EditorRegion{
public:
	// ʵ������
	ORION_TYPE_DECLARE(LEVEL_INPUT_SLIDERBAR)
	EditorInputSliderBar(ObjectTypeResourceObject* prototype, QGraphicsItem * parent = 0);
	virtual ~EditorInputSliderBar();

	// ���ƴ���������״
	virtual void drawShape(QPainter *painter);
	void postinit(bool resize = true);

private:
	int value_;  // ���Ȱٷֱ�
	int gridnum_;
	QImage icon_;  // ������ͷ
};


#endif // ORION_EDITORIMAGE_H
