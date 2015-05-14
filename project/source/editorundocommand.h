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


#ifndef ORION_EDITOR_UNDOCOMMAND_H
#define ORION_EDITOR_UNDOCOMMAND_H

#include <QUndoCommand>
#include "levelscene.h"
#include "resourceobject.h"
using namespace ORION;

class AddInstanceCommand : public QUndoCommand{
public:
	AddInstanceCommand(EditorItem* item, ResourceHash layer, LevelScene* scene, QUndoCommand *parent = 0);
	~AddInstanceCommand();

	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

private:
	LevelScene* scene_;
	ResourceHash layer_;  // ͼ����
	QByteArray data_;  // ��������
	ResourceHash iid_;  // ����ID
};

//////////////////////////////////////////////////////////////////////////

class ModifyInstanceCommand : public QUndoCommand{
public:
	ModifyInstanceCommand(EditorItem* item, const QByteArray& oldData, bool enable, LevelScene* scene, QUndoCommand *parent = 0);
	~ModifyInstanceCommand();

	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

	void setEnabled(bool flag){
		enabled_ = flag;
	}

private:
	LevelScene* scene_;
	QByteArray newData_;
	QByteArray oldData_;
	ResourceHash iid_;  // ����ID
	bool enabled_;
};

//////////////////////////////////////////////////////////////////////////

class MoveInstanceCommand : public QUndoCommand{
public:
	MoveInstanceCommand(EditorItem* item, const QString& layerName, LevelScene* scene, QUndoCommand *parent = 0);
	~MoveInstanceCommand();

	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

private:
	LevelScene* scene_;
	QString oldlayer_;
	QString newlayer_;
	ResourceHash iid_;  // ����ID

};



//////////////////////////////////////////////////////////////////////////

class DeleteInstanceCommand : public QUndoCommand{
public:
	DeleteInstanceCommand(EditorItem* item, ResourceHash layer, LevelScene* scene, QUndoCommand *parent = 0);
	~DeleteInstanceCommand();

	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

private:
	LevelScene* scene_;
	ResourceHash layer_;  // ͼ����
	QByteArray data_;  // ��������
	ResourceHash iid_;  // ����ID

};


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class AddLayerCommand : public QUndoCommand{
public:
	AddLayerCommand(LayerResourceObject* layer, QUndoCommand *parent = 0);
	~AddLayerCommand();

	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

private:
	LayerResourceObject* layer_;   // ͼ��
};



class DeleteLayerCommand : public QUndoCommand{
public:
	DeleteLayerCommand(LayerResourceObject* layer, QUndoCommand *parent = 0);
	~DeleteLayerCommand();

	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

private:
	LayerResourceObject* layer_;   // ͼ��
	int order_;  // ����
};


class MoveLayerCommand : public QUndoCommand{
public:
	MoveLayerCommand(LayerResourceObject* layer, int newOrder, QUndoCommand *parent = 0);
	~MoveLayerCommand();

	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

private:
	LayerResourceObject* layer_;   // ͼ��
	int newOrder_;
	int oldOrder_;
};


class ModifyLayerCommand : public QUndoCommand{
public:
	ModifyLayerCommand(LayerResourceObject* layer,  const QByteArray& oldValue, bool enable = true, QUndoCommand *parent = 0);
	~ModifyLayerCommand();

	void undo() Q_DECL_OVERRIDE;
	void redo() Q_DECL_OVERRIDE;

	void setEnabled(bool flag){
		enabled_ = flag;
	}

private:
	LayerResourceObject* layer_;   // ͼ��
	QByteArray newValue_;
	QByteArray oldValue_;
	bool enabled_; 
};



#endif // ORION_TILELISTWIDGET_H
