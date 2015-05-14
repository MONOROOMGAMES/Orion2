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

#include "workspacewindow.h"
#include "ui_workspacewindow.h"
#include <QResizeEvent>
#include <QMessageBox>
#include <QMenu>
#include "commondefine.h"
#include "tooldefine.h"
#include "configmanager.h"
#include "resourceobject.h"
#include "mainmanagerframe.h"
#include "framemanager.h"
#include "menumanager.h"
#include "tooldefine.h"
#include "leveldefinewindow.h"
#include "layerdefinewindow.h"
#include "sounddefinewindow.h"
#include "moviedefinewindow.h"
#include "selectsoundwindow.h"
#include "plugintypeeditwindow.h"
#include "levelscene.h"
#include "editorundocommand.h"
#include "logmanager.h"
#include "recyclelevelwindow.h"
using namespace ORION;

WorkspaceWindow::WorkspaceWindow(QWidget *parent) :
QWidget(parent), isClosing_(false),
    ui(new Ui::WorkspaceWindow)
{
    ui->setupUi(this);

	BEGIN_ACTION();
	CONNECT_ACTION(ORION_EDIT_DELETE, this, deleteProjectItem);
	CONNECT_ACTION(ORION_OBJECTTYPE_DELETE, this, deleteObjectItem)

	CONNECT_ACTION(ORION_OBJECTTYPE_ADDCATEGORY, this, addObjectGroup)

	CONNECT_ACTION(ORION_EDIT_SOUND, this, editSound)
	CONNECT_ACTION(ORION_EDIT_MOVIE, this, editMovie)
	CONNECT_ACTION(ORION_EDIT_OBJECTTYPE, this, editObjectType)

	END_ACTION();

	connect(ui->leveltreeWidget, SIGNAL(layerMoved(unsigned int, int)), this, SLOT(layerMoved(unsigned int, int)));

}

WorkspaceWindow::~WorkspaceWindow()
{
    delete ui;
}

void	WorkspaceWindow::resizeEvent(QResizeEvent * event){
	QSize newSize = event->size() - QSize(WIDGET_MARGIN * 2, WIDGET_MARGIN*2);
	ui->layoutWidget->setGeometry(QRect(QPoint(WIDGET_MARGIN, WIDGET_MARGIN), newSize));

	// ����Tab�ؼ���С
	QSize tabsize = ui->tabWidget->geometry().size();
	tabsize.setWidth(tabsize.width() - 8);
	tabsize.setHeight(tabsize.height() - 36);

	ui->verticalLayoutWidget->setGeometry(QRect(QPoint(WIDGET_MARGIN, WIDGET_MARGIN*2), tabsize));
	ui->layoutWidget1->setGeometry(QRect(QPoint(WIDGET_MARGIN, WIDGET_MARGIN*2), tabsize));
}


void WorkspaceWindow::updateLevel(ProjectResourceObject* proj){

	ui->leveltreeWidget->clear();

	QTreeWidgetItem* rootItem = new QTreeWidgetItem(ui->leveltreeWidget, ITEM_PROJECT);
	rootItem->setText(0, proj->objectName());
	rootItem->setData(0, ITEM_HASHKEY, proj->hashKey());
	proj->leveltreeItem_ = rootItem;


	QTreeWidgetItem* levelItem = NULL;
	QTreeWidgetItem* layerItem = NULL;

	foreach(LevelResourceObject* level, proj->levels_){
		if (level->flags() & URF_TagGarbage)
			continue;

		levelItem = ui->leveltreeWidget->addLevelItem(level);

		// ���ͼ��( Ĭ������ )
		QTreeWidgetItem* layerItem = NULL;
		foreach(LayerResourceObject* layer, level->layers_){
			layerItem = ui->leveltreeWidget->addLayerItem(layer, levelItem);
			layerItem->setHidden(true);
		}
	}

	ui->leveltreeWidget->expandItem(rootItem);
}


void WorkspaceWindow::updateObject(ProjectResourceObject* proj){

	ui->objecttreeWidget->init( proj );

}
// �ı乤���ռ�
void WorkspaceWindow::updateWorkspace(){

	// ����̬����
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL){
		QMessageBox::warning(this, UDQ_TR("�½���Ŀ"), UDQ_TR("���ȴ�һ����Ŀ!"));
		return;
	}

	// ��ȡ��ǰ������
	ProjectResourceObject* proj = ws->currentProject();
	Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

	updateLevel(proj);
	updateObject(proj);

	isClosing_ = false;

}


void WorkspaceWindow::updateWorkspaceIcon(){
	// ����̬����
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL){
		QMessageBox::warning(this, UDQ_TR("�½���Ŀ"), UDQ_TR("���ȴ�һ����Ŀ!"));
		return;
	}

	// ��ȡ��ǰ����
	ProjectResourceObject* proj = ws->currentProject();
	Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

	// �����
	foreach(LevelResourceObject* level, proj->levels_){
		if (level->flags() & URF_TagGarbage)
			continue;
		// ���ҳ��
		ui->leveltreeWidget->updateItemIcon(level->treeItem_);
	}
}

void WorkspaceWindow::clearWorkspace(){

	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	Q_ASSERT(ws != NULL);

	// ���������Ϣ
	ResourceObject::resetWorkspace(ws);

		// �����
	ui->leveltreeWidget-> clear();
	ui->objecttreeWidget->clearAllItems();

	setWindowTitle(UDQ_TR("�����ռ�"));
}


void  WorkspaceWindow::on_levelpushButton_clicked(){
	// ���ó���ģʽ
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL){
		QMessageBox::warning(this, UDQ_TR("�½�����"), UDQ_TR("���ȴ�һ������!"));
		return;
	}

	// ��ȡ��ǰ������
	ProjectResourceObject* proj = ws->currentProject();
	Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

	// Ŀǰ���ܴ�һ����Ŀ���Ժ���չ�������Ŀ
	QTreeWidgetItem* rootItem = ui->leveltreeWidget->getProjectItem();

	LevelResourceObject* level = proj->addNewLevel(UDQ_T(""));
	Q_ASSERT(level != 0);
	QTreeWidgetItem* levelItem = ui->leveltreeWidget->addLevelItem(level);
	levelItem->setExpanded(false);

	// ���ñ�����
	proj->setFlags(URF_TagSave);

	Q_ASSERT(levelItem != NULL);
	ui->leveltreeWidget->expandItem(rootItem);
	ui->leveltreeWidget->scrollToItem(levelItem);

}

void  WorkspaceWindow::on_layerpushButton_clicked(){

	// ���ó���ģʽ
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL){
		QMessageBox::warning(this, UDQ_TR("�½�ͼ��"), UDQ_TR("���ȴ�һ������!"));
		return;
	}

	// ��ȡ��ǰ������
	ProjectResourceObject* proj = ws->currentProject();
	Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

	// ֻ����򿪵ĳ������ͼ��
	LevelScene* scene = (LevelScene*)(frame->sceneView()->scene());
	EditorLevel* levelItem = scene->getLevelItem();
	if (levelItem == NULL || levelItem->level_ == NULL ){
		QMessageBox::warning(this, UDQ_TR("�½�ͼ��"), UDQ_TR("���ȴ�һ������!"));
		return;
	}

	LayerResourceObject* layer = levelItem->level_->addNewLayer(UDQ_T(""));
	Q_ASSERT(layer != 0);
	
	QUndoCommand* command = new AddLayerCommand(layer);
	frame->undoStack()->push(command);

}

void WorkspaceWindow::on_leveltreeWidget_currentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous){
	// ���ó���ģʽ
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	if (isClosing_){
		frame->propertyWindow()->clear();
	}
	else{

		if (current && current->type() == ITEM_LAYER){
			ResourceHash key = current->data(0, ITEM_HASHKEY).toUInt();
			frame->activeLayer(key);
		}
		frame->updateProperty(current);
	}

}

void WorkspaceWindow::deleteLevel(QTreeWidgetItem* item){
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	item->setSelected(false);

	// ɾ������ҳ
	ResourceHash key = item->data(0, ITEM_HASHKEY).toUInt();
	// ɾ���༭���е�ҳ��
	frame->removeLevel(key);

	// ɾ��ҳ����
	item->parent()->removeChild(item);
	delete item;
}


QTreeWidgetItem* WorkspaceWindow::addLayer(LayerResourceObject* layer){
	Q_ASSERT(layer->level_ && layer->level_->treeItem_);
	QTreeWidgetItem* item = ui->leveltreeWidget->addLayerItem(layer, layer->level_->treeItem_);

	return item;
}


// ɾ������ҳ��
void WorkspaceWindow::deleteLayer(QTreeWidgetItem* item){
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	item->setSelected(false);

	// ɾ������ҳ
	ResourceHash key = item->data(0, ITEM_HASHKEY).toUInt();
	// ɾ���༭���е�ҳ��
	frame->removeLayerFromScene(key);

	// ɾ��ҳ����
	item->parent()->removeChild(item);
	delete item;
}

void WorkspaceWindow::deleteProjectItem(){
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	QTreeWidgetItem* selectedItem = ui->leveltreeWidget->currentItem();
	if (selectedItem != NULL){

		if (selectedItem->type() == ITEM_LEVEL){
			if (QMessageBox::question(this, windowTitle(), UDQ_TR("ȷ��ɾ����������ͨ������վ�ָ�����")) == QMessageBox::No){
				return;
			}
			// ɾ��ѡ�еĳ���
			deleteLevel(selectedItem);
		}
		else if (selectedItem->type() == ITEM_LAYER){
			// ɾ��ѡ�е�ͼ��
			ResourceHash key = selectedItem->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, key);

			QUndoCommand* command = new DeleteLayerCommand(layer);
			frame->undoStack()->push(command);
		}
		else{
			Q_UNREACHABLE();
		}
	}
}

void WorkspaceWindow::addObjectGroup(){
	
	QTreeWidgetItem* item = ui->objecttreeWidget->currentItem();
	if (item == NULL)
		return;

	ui->objecttreeWidget->addGroup(item);

}

void WorkspaceWindow::deleteObjectItem(){
	// ɾ����������
	QTreeWidgetItem* item = ui->objecttreeWidget->currentItem();
	if (item == NULL)
		return;

	if (item->type() == ITEM_OBJECTTYPE){
		// 
		if (QMessageBox::question(this, UDQ_TR("ɾ��"), UDQ_TR("ȷ��ɾ����������[%1]��������ʵ��Ҳ�ᱻɾ��,�Ҳ��ɻָ���?").arg(item->text(0)), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No){
			return;
		}
		ResourceHash key = item->data(0, ITEM_HASHKEY).toUInt();
		emit deleteObjectType(key);

		// ɾ��
		ui->objecttreeWidget->clearSelection();
		ui->objecttreeWidget->deleteObjectType(key);
	}
	else if (item->type() == ITEM_FAMILY){
		// ɾ������
		if (QMessageBox::question(this, UDQ_TR("ɾ��"), UDQ_TR("ȷ��ɾ������[%1]?").arg(item->text(0)), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No){
			return;
		}

		ResourceHash key = item->data(0, ITEM_HASHKEY).toUInt();
		emit deleteFamily(key);

		// ɾ��
		ui->objecttreeWidget->rootFamilyItem_->removeChild(item);
	}
	else if (item->type() == ITEM_FAMILYMEMBER){

		ResourceHash key = item->data(0, ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, type, key);

		// ���������ʹӼ������Ƴ�
		FamilyResourceObject* family = ui->objecttreeWidget->findFamily(item);
		family->members_.removeOne(type);

		// ɾ��
		QTreeWidgetItem* parentItem = item->parent();
		parentItem->removeChild(item);
	}
	else if (item->type() == ITEM_CATEGORY){
		ui->objecttreeWidget->deleteGroupItem(item);
	}

}

void WorkspaceWindow::on_leveltreeWidget_itemDoubleClicked(QTreeWidgetItem * item, int column){

	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// �л�����
	if ( item && item->type() == ITEM_LEVEL && column == 0){

		LevelScene* scene = (LevelScene*)frame->sceneView()->scene();
		// ���ؾɳ���
		EditorLevel* levelItem = scene->getLevelItem();
		if (levelItem && levelItem->level_ ){
			QTreeWidgetItem* oldLevelItem = levelItem->level_->treeItem_;
			oldLevelItem->setExpanded(false);
			for (int i = 0; i < oldLevelItem->childCount(); i++){
				oldLevelItem->child(i)->setHidden(true);
			}
		}

		ResourceHash key = item->data(0, ITEM_HASHKEY).toUInt();
		if (key == 0){
			gLogManager->logError(UDQ_TR("�л�����<%1>��keyֵ��Ч���л�ʧ��"));
			return;
		}

		// ����ͼ������
		item->setExpanded(true);
		for (int i = 0; i < item->childCount(); i++){
			item->child(i)->setHidden(false);
		}

		// �л�����
		frame->switchLevel(key);
	}
}

void WorkspaceWindow::clearSelection(){

	ui->objecttreeWidget->clearSelection();
	ui->leveltreeWidget->clearSelection();
	isClosing_ = true;

}

void WorkspaceWindow::addObjectType(ResourceHash key){
	DEFINE_RESOURCE_HASHKEY(ObjectTypeResourceObject, type, key);
	if (key){
		ui->objecttreeWidget->addObjectTypeItem(type);
	}
}

void WorkspaceWindow::on_objecttreeWidget_currentItemChanged(QTreeWidgetItem * current, QTreeWidgetItem * previous){
	// ���ó���ģʽ
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	frame->updateProperty(current);
}

void WorkspaceWindow::refreshObjectType(ResourceHash key){
	
	DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, objType, key);

	// ����ͼ
	QPixmap img;
	if (objType->prototype_->hasAttribute(EF_ANIMATIONS) || objType->prototype_->hasAttribute(EF_TEXTURE)){
		img = objType->thumbnailImage();
		if (img.isNull()){
			img = QPixmap::fromImage(objType->prototype_->icon_);
		}
	}

	objType->treeItem_->setText(0, objType->description());
	// ����ͼ
	if (!img.isNull()){
		objType->treeItem_->setIcon(0, img);
	}

	// ����Family

}

void WorkspaceWindow::refreshObjectTypes(){

	int num = ui->objecttreeWidget->rootEntityItem_->childCount();
	QTreeWidgetItem* item = NULL;
	for (int i = 0; i < num; i++){
		item = ui->objecttreeWidget->rootEntityItem_->child(i);
		ResourceHash key = item->data(0, ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, objType, key);
		item->setText(0, objType->description());

		// ����ͼ
		if (objType->prototype_->hasAttribute(EF_ANIMATIONS) || objType->prototype_->hasAttribute(EF_TEXTURE)){
			QPixmap img = objType->thumbnailImage();
			if (img.isNull()){
				img = QPixmap::fromImage(objType->prototype_->icon_);
			}
			item->setIcon(0, img);
		}
	}

	// ����Family

	// ����Sound

	// ����Movie
}

void  WorkspaceWindow::reorderLayerItems(LevelResourceObject* level){
	if (level->treeItem_ == NULL)
		return;
	// ���µ���
	ui->leveltreeWidget->updateLevelItem(level->treeItem_);
}

void WorkspaceWindow::layerMoved(unsigned int key, int order ){
	//  ˢ�µ�ǰ����
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, key);
	QUndoCommand* command = new MoveLayerCommand(layer, order);
	frame->undoStack()->push(command);
}

void  WorkspaceWindow::on_familypushButton_clicked(){
	// �½��ű�
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL){
		QMessageBox::warning(this, UDQ_TR("�½�����"), UDQ_TR("���ȴ�һ������!"));
		return;
	}

	// ��ȡ��ǰ������
	ProjectResourceObject* proj = ws->currentProject();
	Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

	FamilyResourceObject* family = proj->addNewFamily();
	Q_ASSERT(family != 0);
	QTreeWidgetItem* familyItem = ui->objecttreeWidget->addFamilyItem(family);
	
	// ���ñ�����
	proj->setFlags(URF_TagSave);
}

void  WorkspaceWindow::on_soundpushButton_clicked(){
	// �½��ű�
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL){
		QMessageBox::warning(this, UDQ_TR("�½�����"), UDQ_TR("���ȴ�һ������!"));
		return;
	}
	// ��ȡ��ǰ������
	ProjectResourceObject* proj = ws->currentProject();
	Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

	SelectSoundWindow w(0);
	if (w.exec() == QDialog::Accepted){
		// ����µ�����
		DEFINE_RESOURCE_HASHKEY_VALID(SoundResourceObject, sound, w.soundKey_);
		ui->objecttreeWidget->addSoundItem( sound );
		// ���ñ�����
		proj->setFlags(URF_TagSave);
	}

}

void  WorkspaceWindow::on_moviepushButton_clicked(){
	// �½��ű�
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL){
		QMessageBox::warning(this, UDQ_TR("�½�����"), UDQ_TR("���ȴ�һ������!"));
		return;
	}

	// ��ȡ��ǰ������
	ProjectResourceObject* proj = ws->currentProject();
	Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

}

void WorkspaceWindow::editSound(){
	QTreeWidgetItem* currentItem = ui->objecttreeWidget->currentItem();
	if (currentItem){
		if (currentItem->type() == ITEM_SOUND){
			ResourceHash key = currentItem->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(SoundResourceObject, sound, key);

			SoundDefineWindow w(sound);
			w.exec();

			// ��������
			currentItem->setText(0, sound->description());
		}
	}
}

void WorkspaceWindow::editMovie(){

}

void WorkspaceWindow::editObjectType(){
	QTreeWidgetItem* currentItem = ui->objecttreeWidget->currentItem();
	if (currentItem){
		if (currentItem->type() == ITEM_OBJECTTYPE){
			ResourceHash key = currentItem->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, objType, key);

			PluginTypeEditWindow w(objType);
			if (w.exec() == QDialog::Accepted){
				ui->objecttreeWidget->refreshItem(currentItem);
				// ˢ�������б�
				MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
				Q_ASSERT(frame != NULL);
				frame->updateProperty(currentItem);
			}
		}
	}
}

void WorkspaceWindow::on_objecttreeWidget_itemChanged(QTreeWidgetItem * item, int column){
	if (item && item->type() == ITEM_OBJECTTYPE && column == 0 ){
		ResourceHash key = item->data(0, ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, objType, key);
		ui->objecttreeWidget->refreshFamilyMember(objType->hashKey());
	}
	else if (item && item->type() == ITEM_CATEGORY && column == 0){
		// �������
		bool dup = false;
		int num = item->parent()->childCount();
		QTreeWidgetItem* child = NULL;
		for (int i = 0; i < num; i++){
			child = item->parent()->child(i);
			if (child != item && child->text(0) == item->text(0)){
				// ������������������
				QMessageBox::warning(this, windowTitle(), UDQ_TR("��������"));
				item->setText(0, item->data(0, ITEM_VALUE).toString());
				dup = true;
				break;
			}
		}

		if (!dup){
			item->setData(0,  ITEM_VALUE, item->text(0));
		}
	}
}

void WorkspaceWindow::saveObjectType(){
	ui->objecttreeWidget->updateObjectTypeCategory();
}


void  WorkspaceWindow::on_trashpushButton_clicked(){
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("���ȴ�һ������!"));
		return;
	}

	// ��ȡ��ǰ������
	ProjectResourceObject* proj = ws->currentProject();
	Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

	RecycleLevelWindow w(proj);
	if (w.exec() == QDialog::Accepted){
		// ����³�������
		QTreeWidgetItem* levelItem = ui->leveltreeWidget->addLevelItem(w.level_);
		// ���ͼ��( Ĭ������ )
		QTreeWidgetItem* layerItem = NULL;
		foreach(LayerResourceObject* layer, w.level_->layers_){
			layerItem = ui->leveltreeWidget->addLayerItem(layer, levelItem);
			layerItem->setHidden(true);
		}
	}
}

void WorkspaceWindow::recycleObjectType(ObjectTypeResourceObject* obj){

	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	Q_ASSERT(ws != NULL);

	// ��ȡ��ǰ������
	ProjectResourceObject* proj = ws->currentProject();
	Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

	obj->clearFlags(URF_TagGarbage);
	proj->objTypes_.append(obj);

	ui->objecttreeWidget->addObjectTypeItem(obj);
}