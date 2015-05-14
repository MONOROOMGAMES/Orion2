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

#include "editorundocommand.h"
#include <QtGui>
#include <QMenu>
#include "commondefine.h"
#include "tooldefine.h"
#include "kismetdefine.h"
#include "componentdefine.h"
#include "menumanager.h"
#include "logmanager.h"
#include "editorregion.h"
#include "framemanager.h"
#include "mainmanagerframe.h"
using namespace ORION;


AddInstanceCommand::AddInstanceCommand(EditorItem* item, ResourceHash layer, LevelScene* scene, QUndoCommand *parent)
	: QUndoCommand(parent), scene_(scene), layer_(layer)
{
	data_ = item->encode();
	iid_ = item->iid();
}

AddInstanceCommand::~AddInstanceCommand()
{

}

void AddInstanceCommand::undo()
{
	// ɾ��
	scene_->deleteInstanceByID(iid_);
	scene_->update();
}

void AddInstanceCommand::redo()
{

	// ����ָ��iD�����Ƿ����
	EditorRegion* region = (EditorRegion*)scene_->findInstanceByID(iid_);
	if (region == NULL){
		region = scene_->addInstance(data_);
		Q_ASSERT(region != NULL);

		// ��ӵ�ָ��ͼ��
		EditorLayer* layerItem = scene_->getLayer(layer_);
		if (layerItem){
			layerItem->addInstance(region);
		}
		else{
			scene_->addRootInstance(region);
		}

		// �����������
		if (region->typePrototype_->flags() & URF_TagGarbage){
			MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
			Q_ASSERT(frame != NULL);
			frame->workspaceWindow()->recycleObjectType(region->typePrototype_);
		}

	}

	scene_->clearSelection();
	scene_->update();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

ModifyInstanceCommand::ModifyInstanceCommand(EditorItem* item, const QByteArray& oldData, bool enable, LevelScene* scene, QUndoCommand *parent)
	: QUndoCommand(parent), scene_(scene), oldData_(oldData), enabled_(enable)
{
	newData_ = item->encode();
	iid_ = item->iid();
}

ModifyInstanceCommand::~ModifyInstanceCommand()
{

}

void ModifyInstanceCommand::undo()
{
	// ���ó���ģʽ
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	EditorItem* item = scene_->findInstanceByID(iid_);
	item->decode(oldData_);
	item->postinit(false);

	// ˢ�����Դ���
	frame->refreshProperty(scene_->findInstanceByID(iid_));
}

void ModifyInstanceCommand::redo()
{
	if (enabled_){
		MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
		Q_ASSERT(frame != NULL);

		EditorItem* item = scene_->findInstanceByID(iid_);
		item->decode(newData_);
		item->postinit(false);
		
		// ˢ�����Դ���
		frame->refreshProperty(item);
	}

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

MoveInstanceCommand::MoveInstanceCommand(EditorItem* item, const QString& layerName, LevelScene* scene, QUndoCommand *parent)
	: QUndoCommand(parent), scene_(scene), newlayer_(layerName)
{
	iid_ = item->iid();
	oldlayer_ = ((EditorLayer*)item->parentItem())->layerName();
}

MoveInstanceCommand::~MoveInstanceCommand()
{

}

void MoveInstanceCommand::undo()
{
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	EditorRegion* item = (EditorRegion*)scene_->findInstanceByID(iid_);
	item->changeToLayer(oldlayer_);

	// ˢ�����Դ���
	frame->itemChanged(item);
}

void MoveInstanceCommand::redo()
{
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	EditorRegion* item = (EditorRegion*)scene_->findInstanceByID(iid_);
	item->changeToLayer(newlayer_);

	// ˢ�����Դ���
	frame->itemChanged(item);
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

DeleteInstanceCommand::DeleteInstanceCommand(EditorItem* item, ResourceHash layer, LevelScene* scene, QUndoCommand *parent)
	: QUndoCommand(parent), scene_(scene), layer_(layer)
{
	data_ = item->encode();
	iid_ = item->iid();
}

DeleteInstanceCommand::~DeleteInstanceCommand()
{

}

void DeleteInstanceCommand::undo()
{

	EditorRegion* region = (EditorRegion*)scene_->addInstance(data_);
	Q_ASSERT(region != NULL);

	EditorLayer* layerItem = scene_->getLayer(layer_);
	if (layerItem){
		layerItem->addInstance(region);
	}
	else{
		scene_->addRootInstance(region);
	}

	// �����������
	if (region->typePrototype_->flags() & URF_TagGarbage){
		MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
		Q_ASSERT(frame != NULL);
		frame->workspaceWindow()->recycleObjectType(region->typePrototype_);
	}

	scene_->clearSelection();
	scene_->update();
}

void DeleteInstanceCommand::redo()
{
	scene_->deleteInstanceByID(iid_);
	scene_->update();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

AddLayerCommand::AddLayerCommand(LayerResourceObject* layer, QUndoCommand *parent)
	: QUndoCommand(parent), layer_(layer)
	{
	}

AddLayerCommand::~AddLayerCommand()
{

}

void AddLayerCommand::undo()
{
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	frame->workspaceWindow()->deleteLayer(layer_->treeItem_);
	// ȡ��Item
	layer_->level_->layers_.removeOne(layer_);
	layer_->setFlags(URF_TagGarbage);

}

void AddLayerCommand::redo(){

	// ���ó���ģʽ
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	frame->workspaceWindow()->addLayer(layer_);
	layer_->clearFlags(URF_TagGarbage);
	layer_->setFlags(URF_TagSave);

	// ����
	layer_->level_->layers_.append(layer_);
	layer_->level_->treeItem_->addChild(layer_->treeItem_);
	layer_->treeItem_->setHidden(false);

	// ���볡��
	frame->addLayerToScene(layer_->hashKey());
}

DeleteLayerCommand::DeleteLayerCommand(LayerResourceObject* layer, QUndoCommand *parent)
	: QUndoCommand(parent), layer_(layer)
{
	// ��¼��ǰ��������
	order_ = layer_->level_->layers_.indexOf(layer_);
}

DeleteLayerCommand::~DeleteLayerCommand()
{

}

void DeleteLayerCommand::undo()
{
	// ���ó���ģʽ
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	frame->workspaceWindow()->addLayer(layer_);
	layer_->clearFlags(URF_TagGarbage);
	layer_->setFlags(URF_TagSave);

	// ����
	layer_->level_->layers_.insert(order_, layer_);
	layer_->level_->treeItem_->addChild(layer_->treeItem_);
	layer_->treeItem_->setHidden(false);

	// ���볡��
	frame->addLayerToScene(layer_->hashKey());
}

void DeleteLayerCommand::redo()
{
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);
	// ȡ��Item
	layer_->level_->layers_.removeOne(layer_);
	layer_->setFlags(URF_TagGarbage);

	frame->workspaceWindow()->deleteLayer(layer_->treeItem_);

}

MoveLayerCommand::MoveLayerCommand(LayerResourceObject* layer, int newOrder, QUndoCommand *parent)
	: QUndoCommand(parent), layer_(layer), newOrder_(newOrder)
{
	// ��¼��ǰ��������
	oldOrder_ = layer_->level_->layers_.indexOf(layer_);

}

MoveLayerCommand::~MoveLayerCommand()
{

}

void MoveLayerCommand::undo()
{
	// ���ó���ģʽ
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	layer_->level_->moveLayer(newOrder_, oldOrder_);
	frame->reorderLayers();
}

void MoveLayerCommand::redo()
{
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	layer_->level_->moveLayer(oldOrder_, newOrder_);
	frame->reorderLayers();

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////


ModifyLayerCommand::ModifyLayerCommand(LayerResourceObject* layer, const QByteArray& oldValue, bool enable, QUndoCommand *parent)
	: QUndoCommand(parent), layer_(layer), oldValue_(oldValue), enabled_(enable)
{
	newValue_ = layer->saveProperty();
}

ModifyLayerCommand::~ModifyLayerCommand()
{

}

void ModifyLayerCommand::undo()
{
	// ���ó���ģʽ
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	layer_->restoreProperty(oldValue_);
	// ˢ�����Դ���
	frame->refreshProperty(layer_->treeItem_);
}

void ModifyLayerCommand::redo()
{

	if (enabled_){
		MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
		Q_ASSERT(frame != NULL);
		layer_->restoreProperty(newValue_);
		// ˢ�����Դ���
		frame->refreshProperty(layer_->treeItem_);
	}

}
