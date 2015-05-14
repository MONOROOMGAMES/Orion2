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
**  Orion2 is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Orion.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#include <QtGui>
#include <QGraphicsRectItem>
#include <QGraphicsSceneDragDropEvent>
#include <QMenu>
#include <QMessageBox>
#include "levelscene.h"
#include <math.h>
#include "commondefine.h"
#include "componentdefine.h"
#include "menumanager.h"
#include "leveldefine.h"
#include "levelview.h"
#include "editorlayer.h"
#include "mainmanagerframe.h"
#include "configmanager.h"
#include "logmanager.h"
#include "devicemanager.h"
using namespace ORION;

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
const int SCENE_SIZE = 25600;

LevelScene::LevelScene(QObject *parent) : QGraphicsScene(parent){

	setSceneRect(-SCENE_SIZE / 2.0, -SCENE_SIZE / 2.0, SCENE_SIZE, SCENE_SIZE);

	if( gConfigManager->showLogo() ){
		initLogo();
	}

	// �߿�
	QGraphicsRectItem* borderItem = new QGraphicsRectItem(sceneRect());
	borderItem->setPos(0,0);
	borderItem->setVisible( true );
	addItem( borderItem );

	// �Ϸű�־
	dropItem_ = new QGraphicsRectItem(QRectF(0,0,64,64));
	dropItem_->setPos(0, 0);
	dropItem_->setVisible(false);
	addItem(dropItem_);

	// �ӿڶ���
	viewPortItem_ = new EditorViewport();
	viewPortItem_->setMinimized(true);

	QRectF rect = viewPortItem_->boundingRect();
	viewPortItem_->setPos(rect.width()/2, rect.height()/2.0);
	addItem(viewPortItem_);

	layerText_ = new QGraphicsSimpleTextItem();
	layerText_->setVisible(false);
	layerText_->setZValue(EDITOR_DEPTH_LOGO);
	layerText_->setFlags(QGraphicsItem::ItemIgnoresTransformations);
	layerText_ ->setBrush(Qt::blue);
	QFont font(UDQ_TR("΢���ź�"), 24);
	font.setStyleStrategy(QFont::PreferAntialias);
	font.setPixelSize(24);
	font.setBold(true);
	layerText_->setFont(font);
	addItem(layerText_);

	initMacPanel();

	menuHostItem_ = NULL;
	mapMode_ = DOM_POINTER;
	bgColor_ = Qt::lightGray;
	activeLayer_ = 0;
}

//////////////////////////////////////////////////////////////////////////

LevelScene::~LevelScene(){
	foreach(EditorRegion* region, cutObjects_){
		delete region;
	}
}

// ����ģʽ
void LevelScene::setMode(LEVEL_MODE mode){
	LEVEL_MODE oldMode = mapMode_;
	mapMode_  = mode;
	if( mapMode_ == DOM_POINTER ){
		if (oldMode != DOM_TILEEDIT)
			clearSelection();
	}
	else if (mapMode_ == DOM_TILEEDIT){

	}
}

// ��ʼ��LOGO
bool LevelScene::initLogo(){
	logoItem_ = EditorLogoPtr(new EditorLogo(UDQ_T(":/images/unique128.png")));
	logoItem_->setPos(0, 0);
	addItem(logoItem_.get());
	return true;
}

// ��ʼ��
bool LevelScene::initMacPanel(void){

	macPanel_ = new MacDockPanelItem(QSize(800, MAC_PANEL_HEIGHT), QColor(125, 125, 125, 150));
	macPanel_->setDropType(MIME_ORION_IMAGE);
	macPanel_->panel().showShadow(false);
	macPanel_->setVisible(false);
	addItem(macPanel_);
	return true;
}


void LevelScene::drawBackground(QPainter * painter, const QRectF & rect){
	// ���Ƶ�ɫ
	painter->fillRect( rect, bgColor_ );

	// ȷ���ߴ��λ��
	QSize viewRect = views().at(0)->maximumViewportSize();

	// �޸�LOGOλ��
	if (logoItem_.get() != 0){
		logoItem_->setPos(views().at(0)->mapToScene(viewRect.width() - logoItem_->boundingRect().width(), viewRect.height() - logoItem_->boundingRect().height()));
	}

	// �޸��ӿ�λ��
	if (viewPortItem_ && viewPortItem_->isMinimized()){
		viewPortItem_->setPos(views().at(0)->mapToScene(0, 0));
	}

	// �޸�����
	if (layerText_ && layerText_->isVisible()){
		QFontMetricsF fm(layerText_->font());
		qreal pixelsWide = fm.width(layerText_->text());
		qreal pixelsHigh = fm.height();
		layerText_->setPos(views().at(0)->mapToScene((viewRect.width() - pixelsWide) / 2.0, pixelsHigh +4));
	}

	// �޸�MACPANELλ��
	macPanel_->setSize(viewRect.width());
	QPointF viewLBPos = views().at(0)->mapToScene(0, viewRect.height() - MAC_PANEL_HEIGHT);
	QPointF dis = viewLBPos - macPanel_->pos();

	if ((dis.x() * dis.x() + dis.y() * dis.y()) > 0.1)
		macPanel_->setPos(viewLBPos);
	QGraphicsScene::drawBackground(painter, rect);
}

void LevelScene::drawForeground(QPainter * painter, const QRectF & rect){

	QGraphicsScene::drawForeground(painter, rect);
}

// ��ȡ�ɼ����򣨳�������ϵ��
QRectF LevelScene::viewRegionRect(void){

	// ��ȡ��ǰ���ű���
	QSize viewRect = views().at(0)->maximumViewportSize();

	QPointF viewLTPos = views().at(0)->mapToScene( 0, 0 );
	if( viewLTPos.rx() < 0 )
		viewLTPos.rx() = 0;
	if( viewLTPos.ry() < 0 )
		viewLTPos.ry() = 0;

	QRectF sRect = sceneRect();

	QPointF viewRBPos = views().at(0)->mapToScene( viewRect.width(), viewRect.height() );
	if( viewRBPos.rx() > sRect.width() )
		viewRBPos.rx() = sRect.width();
	if( viewRBPos.ry() > sRect.height() )
		viewRBPos.ry() = sRect.height();

	return QRectF(viewLTPos, viewRBPos );
}

//////////////////////////////////////////////////////////////////////////
// �ϷŴ���
//////////////////////////////////////////////////////////////////////////

void LevelScene::dragEnterEvent(QGraphicsSceneDragDropEvent *event){
	clearSelection();

	if (event->mimeData()->hasFormat(MIME_ORION_ENTITY) || event->mimeData()->hasFormat(MIME_ORION_OBJECTTYPE)){
		event->accept();
	}
	else if (event->mimeData()->hasFormat(MIME_ORION_GALLERY)){
		QGraphicsScene::dragEnterEvent(event);
	}
	else{
		event->ignore();
	}

}

void LevelScene::dragLeaveEvent(QGraphicsSceneDragDropEvent *event){
	QGraphicsScene::dragLeaveEvent(event);
}

void LevelScene::dragMoveEvent(QGraphicsSceneDragDropEvent *event){
	if (event->mimeData()->hasFormat(MIME_ORION_ENTITY) || event->mimeData()->hasFormat(MIME_ORION_OBJECTTYPE)) {
		event->accept();
	}
	else if (event->mimeData()->hasFormat(MIME_ORION_GALLERY)){
		QGraphicsScene::dragMoveEvent(event);
	}
	else {
		event->ignore();
	}

}

void LevelScene::dropEvent(QGraphicsSceneDragDropEvent *event){
	// 
	WorkspaceResourceObject*  ws = ResourceObject::currentWorkspace();
	Q_ASSERT(ws != NULL);
	ProjectResourceObject* proj = ws->currentProject();

	if (proj == NULL){
		QMessageBox::critical(views().at(0), UDQ_TR("ҳ��༭"), UDQ_TR("���ȴ�һ������"));
		event->ignore();
		return;
	}

	// �ж��Ƿ��ڵ�ͼ��
	if (event->mimeData()->hasFormat(MIME_ORION_ENTITY)) {
		// �����ʵ�壩
		QByteArray itemData = event->mimeData()->data(MIME_ORION_ENTITY);
		QDataStream stream(&itemData, QIODevice::ReadOnly);

		// ��ȡ������
		ResourceHash key;
		stream >> key;

		DEFINE_RESOURCE_HASHKEY(PluginResourceObject, plugin, key);

		if (!plugin){
			event->ignore();
			return;
		}

		if (plugin->hasAttribute(EF_SINGLEGLOBAL)){
			// ����Ӷ�������
			addSingleGlobalByPlugin(key);
		}
		else{

			EditorLevel* levelItem = getLevelItem();
			// �ж��Ƿ��е�ǰҳ( һ��ҳ�� )
			if (levelItem == NULL || levelItem->level_ == NULL ){
				QMessageBox::critical(views().at(0), UDQ_TR("ҳ��༭"), UDQ_TR("���ȴ�һ������!"));
				event->ignore();
				return;
			}

			// ���뼤�ǰͼ��
			if (plugin->hasAttribute(EF_WORLD) && activeLayer_ == 0){
				QMessageBox::critical(views().at(0), UDQ_TR("ҳ��༭"), UDQ_TR("����ѡ��һ��ͼ��!"));
				event->ignore();
				return;
			}

			ResourceHash typeH = hashStringLower(plugin->objectName());
			EditorRegion* region = NULL;
			if (typeH == COMP_SPRITE){
				// �½�Itemʵ��
				region = addSpriteByPlugin(key, event->scenePos());
			}
			else if (typeH == COMP_TEXT){
				region = addTextByPlugin(key, event->scenePos());
			}
			else if (typeH == COMP_TILEDBG){
				region = addTileBGByPlugin(key, event->scenePos());
			}
			else if (typeH == COMP_SHADOWLIGHT){
				region = addShadowLightByPlugin(key, event->scenePos());
			}
			else if (typeH == COMP_NINEPATCH){
				region = add9PatchByPlugin(key, event->scenePos());
			}
			else if (typeH == COMP_PARTICLES){
				region = addParticleByPlugin(key, event->scenePos());
			}
			else if (typeH == COMP_TILEMAP){
				region = addTileMapByPlugin(key, event->scenePos());
			}
			else if (typeH == COMP_AMAP){
				region = addMapByPlugin(key, event->scenePos());
			}
			else if (typeH == COMP_BUTTON || typeH == COMP_TEXTBOX || typeH == COMP_LIST || typeH == COMP_PROGRESSBAR || typeH == COMP_SLIDERBAR || typeH == COMP_FILECHOOSER){
				region = addInputByPlugin(key, event->scenePos());
			}
			else{ // ����ȫ������EditorObject����
				region = addEntityObjectByPlugin(key, event->scenePos());
			}

			if (region == NULL){
				event->ignore();
			}
			else{
				Q_ASSERT(!activeLayer_ || layers_.contains(activeLayer_));
				emit entityInserted(region, activeLayer_);
				event->accept();
			}
		}
	}
	else if (event->mimeData()->hasFormat(MIME_ORION_OBJECTTYPE)){

		// ����Ƿ���ͼ��
		EditorLevel* levelItem = getLevelItem();
		// �ж��Ƿ��е�ǰҳ( һ��ҳ�� )
		if (levelItem == NULL || levelItem->level_ == NULL ){
			QMessageBox::critical(views().at(0), UDQ_TR("ҳ��༭"), UDQ_TR("���ȴ�һ������!"));
			event->ignore();
			return;
		}

		// ���ݶ������ʹ���һ����ʵ��
		QByteArray itemData = event->mimeData()->data(MIME_ORION_OBJECTTYPE);
		QDataStream stream(&itemData, QIODevice::ReadOnly);

		// ��ȡ������
		ResourceHash key;
		stream >> key;
		DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, objType, key);
		if (objType->prototype_->hasAttribute(EF_SINGLEGLOBAL)){
			QMessageBox::critical(views().at(0), UDQ_TR("ҳ��༭"), UDQ_TR("��ʵ���������Ͳ�����ӵ�������!"));
			event->ignore();
			return;
		}

		// ���뼤�ǰͼ��
		if ( objType->prototype_->hasAttribute(EF_WORLD) && activeLayer_ == 0){
			QMessageBox::critical(views().at(0), UDQ_TR("ҳ��༭"), UDQ_TR("����ѡ��һ��ͼ��!"));
			event->ignore();
			return;
		}

		QString pluginName = objType->prototype_->objectName();
		EditorRegion* region = NULL;
		ResourceHash pluginH = hashStringLower(pluginName);

		if (pluginH == COMP_SPRITE){
			region = new EditorSprite(objType);
		}
		else if (pluginH == COMP_TEXT){
			region = new EditorText(objType);
		}
		else if (pluginH == COMP_TILEDBG){
			region = new EditorTileBG(objType);
		}
		else if (pluginH == COMP_NINEPATCH){
			region = new EditorNinePatch(objType);
		}
		else if (pluginH == COMP_PARTICLES){
			region = new EditorParticle(objType);
		}
		else if (pluginH == COMP_SHADOWLIGHT){
			region = new EditorShadowLight(objType);
		}
		else if (pluginH == COMP_TILEMAP){
			region = new EditorTileMap(objType);
		}
		else if (pluginH == COMP_AMAP){
			region = new EditorMap(objType);
		}
		else if (pluginH == COMP_BUTTON ){
			region = new EditorInputButton(objType);
		}
		else if (pluginH == COMP_TEXTBOX){
			region = new EditorInputTextBox(objType);
		}
		else if (pluginH == COMP_LIST){
			region = new EditorInputList(objType);
		}
		else if (pluginH == COMP_FILECHOOSER){
			region = new EditorInputFileChooser(objType);
		}
		else if (pluginH == COMP_PROGRESSBAR){
			region = new EditorInputProgressBar(objType);
		}
		else if (pluginH == COMP_SLIDERBAR){
			region = new EditorInputSliderBar(objType);
		}
		else{
			region = new EditorObject(objType);
		}

		region->setPos(event->scenePos());  // 
		if (objType->defaultData_.size() > 0){
			region->decode(objType->defaultData_, true);
		}
		else{
			region->postinit();
		}

		// ���뵱ǰͼ��
		Q_ASSERT(!activeLayer_ || layers_.contains(activeLayer_));
		if (region->typePrototype_->prototype_->hasAttribute(EF_WORLD)){
			layers_[activeLayer_]->addInstance(region);
		}
		else{
			addRootInstance(region);
		}

		emit entityInserted(region,activeLayer_);
		event->accept();
	}
	else if (event->mimeData()->hasFormat(MIME_ORION_GALLERY)){
		QGraphicsScene::dropEvent(event);
	}
	else {
		event->ignore();
	}

}

void LevelScene::addRootInstance(EditorRegion* region){
	addItem(region);
	regionObjects_.append(region);
}

void LevelScene::deleteInstanceByType(ObjectTypeResourceObject* obj){

	EditorRegion* region = NULL;
	foreach(EditorItem* item, regionObjects_){
		region = (EditorRegion*)item;
		if (region && region->typePrototype_ == obj ){
			deleteInstance((EditorRegion*)item);
		}
	}

	// ͼ���е�ʵ��
	foreach(EditorLayer* layer, layers_.values()){
		foreach(EditorRegion* region, layer->instances_){
			if (region->typePrototype_ == obj){
				deleteInstance(region);
			}
		}
	}
}

void LevelScene::deleteInstanceByID(ResourceHash iid){

	foreach(EditorItem* item, regionObjects_){
		if (item->iid() == iid){
			deleteInstance((EditorRegion*)item);
		}
	}

	// ͼ���е�ʵ��
	foreach(EditorLayer* layer, layers_.values()){
		foreach(EditorRegion* region, layer->instances_){
			if (region->iid() == iid ){
				deleteInstance(region);
			}
		}
	}
}


EditorItem* LevelScene::findInstanceByID(ResourceHash iid){

	foreach(EditorItem* item, regionObjects_){
		if (item->iid() == iid){
			return item;
		}
	}

	foreach(EditorLayer* layer, layers_.values()){
		foreach(EditorRegion* region, layer->instances_){
			if (region->iid() == iid){
				return region;
			}
		}
	}

	return NULL;
}


//////////////////////////////////////////////////////////////////////////

EditorRegion* LevelScene::addInstance(QByteArray& data){
	EditorRegion* region = NULL;
	int type;
	{
		QDataStream stream(data);
		stream >> type;
	}

	if (type == LEVEL_SPRITE){
		region = addSpriteItem(data);
	}
	else if (type == LEVEL_TEXT){
		region = addTextItem(data);
	}
	else if (type == LEVEL_TILEBG){
		region = addTileBGItem(data);
	}
	else if (type == LEVEL_SHADOWLIGHT){
		region = addShadowLightItem(data);
	}
	else if (type == LEVEL_9PATCH){
		region = add9PatchItem(data);
	}
	else if (type == LEVEL_PARTICLE){
		region = addParticleItem(data);
	}
	else if (type == LEVEL_TILEMAP){
		region = addTileMapItem(data);
	}
	else if (type == LEVEL_MAP){
		region = addMapItem(data);
	}
	else if (type == LEVEL_INPUT_BUTTON){
		region = addInputButtonItem(data);
	}
	else if (type == LEVEL_INPUT_TEXTBOX){
		region = addInputTextBoxItem(data);
	}
	else if (type == LEVEL_INPUT_LIST){
		region = addInputListItem(data);
	}
	else if (type == LEVEL_INPUT_FILECHOOSER){
		region = addInputFileChooserItem(data);
	}
	else if (type == LEVEL_INPUT_PROGRESSBAR){
		region = addInputProgressBarItem(data);
	}
	else if (type == LEVEL_INPUT_SLIDERBAR){
		region = addInputSliderBarItem(data);
	}
	else if (type == LEVEL_OBJECT){
		region = addEntityObjectItem(data);
	}

	return region;
}

EditorSprite* LevelScene::addSpriteItem(QByteArray& data){

	// �½�Itemʵ��
	EditorSprite* sprite = new EditorSprite(NULL);
	sprite->decode(data);
	sprite->postinit(false);
	return sprite;

}

EditorText* LevelScene::addTextItem(QByteArray& data){

	// �½�Itemʵ��
	EditorText* text = new EditorText(NULL);
	text->decode(data);
	text->postinit(false);
	return text;

}

EditorTileBG* LevelScene::addTileBGItem(QByteArray& data){

	// �½�Itemʵ��
	EditorTileBG* tilebg = new EditorTileBG(NULL);
	tilebg->decode(data);
	tilebg->postinit(false);
	return tilebg;

}

EditorShadowLight* LevelScene::addShadowLightItem(QByteArray& data){
	// �½�Itemʵ��
	EditorShadowLight* light = new EditorShadowLight(NULL);
	light->decode(data);
	light->postinit(false);
	return light;
}

EditorObject* LevelScene::addEntityObjectItem(QByteArray& data){
	EditorObject* obj = new EditorObject(NULL);
	obj->decode(data);
	obj->postinit(false);
	return obj;
}

EditorNinePatch* LevelScene::add9PatchItem(QByteArray& data){
	// �½�Itemʵ��
	EditorNinePatch* light = new EditorNinePatch(NULL);
	light->decode(data);
	light->postinit(false);
	return light;
}

EditorParticle* LevelScene::addParticleItem(QByteArray& data){
	// �½�Itemʵ��
	EditorParticle* part = new EditorParticle(NULL);
	part->decode(data);
	part->postinit(false);
	return part;
}

EditorTileMap* LevelScene::addTileMapItem(QByteArray& data){
	// �½�Itemʵ��
	EditorTileMap* map = new EditorTileMap(NULL);
	map->decode(data);
	map->postinit(false);
	return map;
}


EditorMap* LevelScene::addMapItem(QByteArray& data){
	// �½�Itemʵ��
	EditorMap* map = new EditorMap(NULL);
	map->decode(data);
	map->postinit(false);
	return map;
}

EditorInputButton* LevelScene::addInputButtonItem(QByteArray& data){
	// �½�Itemʵ��
	EditorInputButton* input = new EditorInputButton(NULL);
	input->decode(data);
	input->postinit(false);
	return input;
}

EditorInputTextBox* LevelScene::addInputTextBoxItem(QByteArray& data){
	EditorInputTextBox* input = new EditorInputTextBox(NULL);
	input->decode(data);
	input->postinit(false);
	return input;
}

EditorInputList* LevelScene::addInputListItem(QByteArray& data){
	EditorInputList* input = new EditorInputList(NULL);
	input->decode(data);
	input->postinit(false);
	return input;
}

EditorInputFileChooser* LevelScene::addInputFileChooserItem(QByteArray& data){
	EditorInputFileChooser* input = new EditorInputFileChooser(NULL);
	input->decode(data);
	input->postinit(false);
	return input;
}

EditorInputProgressBar* LevelScene::addInputProgressBarItem(QByteArray& data){
	EditorInputProgressBar* input = new EditorInputProgressBar(NULL);
	input->decode(data);
	input->postinit(false);
	return input;
}

EditorInputSliderBar* LevelScene::addInputSliderBarItem(QByteArray& data){
	EditorInputSliderBar* input = new EditorInputSliderBar(NULL);
	input->decode(data);
	input->postinit(false);
	return input;
}


ObjectTypeResourceObject* LevelScene::addObjectTypeByHash(ResourceHash key){
	WorkspaceResourceObject*  ws = ResourceObject::currentWorkspace();
	Q_ASSERT(ws != NULL);
	ProjectResourceObject* proj = ws->currentProject();

	DEFINE_RESOURCE_HASHKEY(PluginResourceObject, plugin, key);
	if (plugin == NULL){
		QString errStr = UDQ_TR("�½����ʵ�����޷��ҵ����(%1)��������").arg(key);
		gLogManager->logError(errStr);
		QMessageBox::critical(views().at(0), UDQ_TR("ҳ��༭"), errStr);
		return NULL;
	}

	// �½�ObjectType
	ObjectTypeResourceObject* type = proj->addNewType(plugin);
	Q_ASSERT(type != NULL);

	if (plugin->hasAttribute(EF_SINGLEGLOBAL)){
		type->setDescription(plugin->objectName());
	}
	else{
		type->setDescription(type->objectName());
	}

	// ֪ͨ���������
	emit addObjectType(type->hashKey());
	return type;
}

void LevelScene::addSingleGlobalByPlugin(ResourceHash key){

	// ����Ƿ��Ѿ����
	WorkspaceResourceObject*  ws = ResourceObject::currentWorkspace();
	Q_ASSERT(ws != NULL);
	ProjectResourceObject* proj = ws->currentProject();

	foreach(ObjectTypeResourceObject* obj, proj->objTypes_){
		if (obj->flags() & URF_TagGarbage)
			continue;

		if (obj->prototype_->hashKey() == key){
			QMessageBox::critical(views().at(0), UDQ_TR("ҳ��༭"), UDQ_TR("�ö�������ֻ�����һ��!"));
			return;
		}
	}

	// �½�ObjectType
	ObjectTypeResourceObject* obj = addObjectTypeByHash(key);
	Q_ASSERT(obj != NULL);

}

EditorSprite* LevelScene::addSpriteByPlugin(ResourceHash key, const QPointF& pos){

	// �½�ObjectType
	ObjectTypeResourceObject* type = addObjectTypeByHash(key);
	Q_ASSERT(type != NULL);

	// �½�Itemʵ��
	EditorSprite* sprite = new EditorSprite(type);
	sprite->setPos(pos);  // 

	// ���뵱ǰͼ��
	Q_ASSERT(layers_.contains(activeLayer_));
	layers_[activeLayer_]->addInstance(sprite);

	return sprite;
}


EditorText* LevelScene::addTextByPlugin(ResourceHash key, const QPointF& pos){

	// �½�ObjectType
	ObjectTypeResourceObject* type = addObjectTypeByHash(key);
	Q_ASSERT(type != NULL);

	// �½�Itemʵ��
	EditorText* text = new EditorText(type);
	text->setPos(pos);  // 

	// ���뵱ǰͼ��
	Q_ASSERT(layers_.contains(activeLayer_));
	layers_[activeLayer_]->addInstance(text);

	return text;
}

EditorTileBG* LevelScene::addTileBGByPlugin(ResourceHash key, const QPointF& pos){

	// �½�ObjectType
	ObjectTypeResourceObject* type = addObjectTypeByHash(key);
	Q_ASSERT(type != NULL);

	// �½�Itemʵ��
	EditorTileBG* tilebg = new EditorTileBG(type);
	tilebg->setPos(pos);  // 

	// ���뵱ǰͼ��
	Q_ASSERT(layers_.contains(activeLayer_));
	layers_[activeLayer_]->addInstance(tilebg);

	return tilebg;
}


EditorShadowLight* LevelScene::addShadowLightByPlugin(ResourceHash key, const QPointF& pos){
	// �½�ObjectType
	ObjectTypeResourceObject* type = addObjectTypeByHash(key);
	Q_ASSERT(type != NULL);

	// �½�Itemʵ��
	EditorShadowLight* light = new EditorShadowLight(type);
	light->setPos(pos);  // 

	// ���뵱ǰͼ��
	Q_ASSERT(layers_.contains(activeLayer_));
	layers_[activeLayer_]->addInstance(light);

	return light;
}

EditorObject* LevelScene::addEntityObjectByPlugin(ResourceHash key, const QPointF& pos){
	// �½�ʵ�����
	ObjectTypeResourceObject* type = addObjectTypeByHash(key);
	Q_ASSERT(type != NULL);

	// �½�Itemʵ��
	EditorObject* obj = new EditorObject(type);
	obj->setPos(pos);  // 

	regionObjects_.append(obj);
	return obj;
}


EditorNinePatch* LevelScene::add9PatchByPlugin(ResourceHash key, const QPointF& pos){
	// �½�ObjectType
	ObjectTypeResourceObject* type = addObjectTypeByHash(key);
	Q_ASSERT(type != NULL);

	// �½�Itemʵ��
	EditorNinePatch* patch = new EditorNinePatch(type);
	patch->setPos(pos);  // 

	// ���뵱ǰͼ��
	Q_ASSERT(layers_.contains(activeLayer_));
	layers_[activeLayer_]->addInstance(patch);

	return patch;
}



EditorParticle* LevelScene::addParticleByPlugin(ResourceHash key, const QPointF& pos){
	// �½�ObjectType
	ObjectTypeResourceObject* type = addObjectTypeByHash(key);
	Q_ASSERT(type != NULL);

	// �½�Itemʵ��
	EditorParticle* part = new EditorParticle(type);
	part->setPos(pos);  // 

	// ���뵱ǰͼ��
	Q_ASSERT(layers_.contains(activeLayer_));
	layers_[activeLayer_]->addInstance(part);

	return part;
}

EditorTileMap* LevelScene::addTileMapByPlugin(ResourceHash key, const QPointF& pos){
	// �½�ObjectType
	ObjectTypeResourceObject* type = addObjectTypeByHash(key);
	Q_ASSERT(type != NULL);

	EditorLevel* level = getLevelItem();

	// �½�Itemʵ��
	EditorTileMap* map = new EditorTileMap(type);
	map->resize(level->boundingRect().size());   // Ĭ�ϵ�ǰ������С
	map->setPos(0,0);  // 

	// ���뵱ǰͼ��
	Q_ASSERT(layers_.contains(activeLayer_));
	layers_[activeLayer_]->addInstance(map);

	return map;
}

EditorMap* LevelScene::addMapByPlugin(ResourceHash key, const QPointF& pos){
	// �½�ObjectType
	ObjectTypeResourceObject* type = addObjectTypeByHash(key);
	Q_ASSERT(type != NULL);

	EditorLevel* level = getLevelItem();

	// �½�Itemʵ��
	EditorMap* map = new EditorMap(type);
	map->resize(level->boundingRect().size());   // Ĭ�ϵ�ǰ������С
	map->setPos(0, 0);  // 

	// ���뵱ǰͼ��
	Q_ASSERT(layers_.contains(activeLayer_));
	layers_[activeLayer_]->addInstance(map);

	return map;
}


EditorRegion* LevelScene::addInputByPlugin(ResourceHash key, const QPointF& pos){
	// �½�ObjectType
	ObjectTypeResourceObject* type = addObjectTypeByHash(key);
	Q_ASSERT(type != NULL);

	EditorLevel* level = getLevelItem();

	// �½�Itemʵ��
	EditorRegion* input = NULL;
	ResourceHash typeH = hashStringLower(type->prototype_->objectName());
	if (typeH == COMP_BUTTON){
		input = new EditorInputButton(type);
	}
	else if (typeH == COMP_TEXTBOX){
		input = new EditorInputTextBox(type);
	}
	else if (typeH == COMP_LIST){
		input = new EditorInputList(type);
	}
	else if (typeH == COMP_FILECHOOSER){
		input = new EditorInputFileChooser(type);
	}
	else if (typeH == COMP_PROGRESSBAR){
		input = new EditorInputProgressBar(type);
	}
	else if (typeH == COMP_SLIDERBAR){
		input = new EditorInputSliderBar(type);
	}

	input->setPos(pos);  // 

	// ���뵱ǰͼ��
	Q_ASSERT(layers_.contains(activeLayer_));
	layers_[activeLayer_]->addInstance(input);

	return input;
}

void LevelScene::deleteInstance(EditorRegion* region){
	region->setSelected(false);
	if (region->parentItem()){
		EditorLayer* parent = (EditorLayer*)region->parentItem();
		parent->removeInstance(region);
	}
	else{
		regionObjects_.removeOne(region);
	}
	delete region;
}

void LevelScene::removeInstance(EditorRegion* region){

	region->setSelected(false);
	if (region->parentItem()){
		EditorLayer* parent = (EditorLayer*)region->parentItem();
		parent->removeInstance(region);
		removeItem(region);
	}

}

//////////////////////////////////////////////////////////////////////////

void LevelScene::mousePressEvent(QGraphicsSceneMouseEvent * mouseEvent){

	QGraphicsScene::mousePressEvent( mouseEvent );
}

void LevelScene::keyPressEvent(QKeyEvent * keyEvent){


	if (keyEvent->key() == Qt::Key_Delete ){
		//
		QList<QGraphicsItem*> items = selectedItems();
		foreach(QGraphicsItem* item, items){
			// ɾ��ѡ�е�ʵ��
			if (item->type() > LEVEL_LAYER ){
				EditorLayer* layer = (EditorLayer*)item->parentItem();
				emit entityDeleted(item,  layer ? layer->layer_->hashKey() : 0);
			}
		}
	}
	QGraphicsScene::keyPressEvent( keyEvent );
}

void LevelScene::keyReleaseEvent(QKeyEvent * keyEvent){

	if (keyEvent->key() == Qt::Key_Space){  // ����ǿո��
		EditorLevel* level = getLevelItem();
		if (selectedItems().size() == 0){ // ʲô����ѡ�������	
			bool flag = level->toggleSelectable();
			level->setSelected(flag);
			update();
		}
		return;
	}

	QGraphicsScene::keyReleaseEvent( keyEvent );
}

void LevelScene::toggleLock(){
	Q_ASSERT(menuHostItem_->type() >= EDITOR_REGION );
	menuHostItem_->toggleLock();
	menuHostItem_ = NULL;
}

void  LevelScene::contextMenuEvent(QGraphicsSceneContextMenuEvent * event){

	if (selectedItems().size() == 0){  // ���û��ѡ�����
		menuHostItem_ = NULL;
		QMenu menu;
		QAction* act = gMenuManager->getAction(ORION_EDIT_PASTE);
		act->setEnabled(copyData_.size() > 0);
		menu.addAction(act);

		menu.exec(event->screenPos());
		return;
	}

	QGraphicsScene::contextMenuEvent(event);
}

// ���س���
void LevelScene::loadLevel(LevelResourceObject* level){
	Q_ASSERT(level != NULL);

	// ���EditorLevel
	EditorLevel* levelItem = new EditorLevel(level);
	levelItem->setPos(0, 0);
	addItem(levelItem);
	regionObjects_.append(levelItem);

	LevelView* view = (LevelView*)views().at(0);
	if (level->version() >= PACKAGE_RESOURCE_ADD_VIEWPORT){
		view->setTransform(level->matrix_);
		view->centerOn(level->viewport_);
		view->setZoomStep(level->zoomStep_);
	}

	QList<QByteArray> validinst;
	EditorRegion* inst = NULL;

	// ��ӹ�������
	foreach(QByteArray dat, level->instances_){
		inst = addInstance(dat);
		if (inst){  // ȥ����Ч�Ķ���ʵ������
			addItem(inst);
			regionObjects_.append(inst);
			validinst.append(dat);
		}
	}
	level->instances_ = validinst;

	// ���ͼ��
	LayerResourceObject* layer = NULL;
	EditorLayer* layerItem = NULL;

	for (int i = 0; i < level->layers_.size(); ++i){

		layer = level->layers_[i];
		if (layer->flags() & URF_TagGarbage)
			continue;

		layerItem = new EditorLayer(layer);
		layerItem->setZValue(EDITOR_DEPTH_BACK + i * 10);  // 10��ֵ��ʾһ��ͼ���ȣ����֧��100��ͼ��
		addItem(layerItem);
		layers_.insert(layer->hashKey(), layerItem);

		// ����ͼ��Ԫ��
		validinst.clear();
		foreach(QByteArray dat, layer->instances_){
			inst = addInstance(dat);
			if (inst){  // ȥ����Ч�Ķ���ʵ������
				layerItem->addInstance(inst);
				validinst.append(dat);
			}
		}
		layer->instances_ = validinst;
	}
}

EditorLevel* LevelScene::getLevelItem(){
	foreach(EditorItem* region, regionObjects_){
		if (region->type() == LEVEL_PROFILE){
			return (EditorLevel*)region;
		}
	}
	return NULL;
}

void LevelScene::closeCurrentLevel(bool save){

	if (save){
		EditorLevel* levelItem = getLevelItem();
		if (levelItem)
			saveLevel(levelItem->level_);
	}

	clearLevel();
}

void LevelScene::saveLevel(LevelResourceObject* level){
	if (level == NULL )
		return;

	LevelView* view = (LevelView*)views().at(0);
	// �����ӵ�
	level->matrix_ =  view->transform();
	level->viewport_ =   viewRegionRect().center();
	level->zoomStep_ =   view->zoomStep();
	level->updateVersion(PACKAGE_RESOURCE_VERSON);

	EditorLevel* levelItem = getLevelItem();
	levelItem->level_->size_ = levelItem->boundingRect().size().toSize();

	// �������
	QImage thumb = screenshot();
	level->setThumb(thumb);
	level->updateThumb(); // ��������ͼ

	// ���湫������
	level->instances_.clear();
	foreach(EditorItem* item, regionObjects_){
		if (item->type() == LEVEL_PROFILE)
			continue;
		level->instances_.append(item->encode());
	}

	// ��ͼ�㱣��
	for (int i = 0; i < levelItem->level_->layers_.size(); ++i){
		LayerResourceObject* layer = levelItem->level_->layers_[i];
		Q_ASSERT(layer != NULL);

		layer->instances_.clear();

		EditorLayer* layerItem = layers_.value(layer->hashKey());
		if (layerItem){
			// ����ͼ������
			foreach(EditorRegion* inst, layerItem->instances_ )
			{
				layer->instances_.append(inst->encode());
			}
		}
	}

	if (levelItem->level_ != level){
		// ����Level����
		levelItem->level_->copyObject(level);
	}

}


void LevelScene::clearLevel(){
	// ִ��������
	clearSelection();

	activeLayer_ = 0;  // ��յ�ǰͼ��
	layerText_->setText(UDQ_T(""));
	layerText_->setVisible(false);

	foreach(EditorItem* item, regionObjects_){
		item->clearAnchor();
		removeItem(item);
		delete item;
	}
	regionObjects_.clear();

	// ���ͼ��
	foreach(EditorLayer* l, layers_.values()){
		l->clearAnchor();
		removeItem(l);
		delete l;
	}
	layers_.clear();

}

void LevelScene::activeLayer(ResourceHash hash){
	activeLayer_ = hash;
	updateLayerText();
}

void LevelScene::updateLayerText(){
	EditorLevel* levelItem = getLevelItem();
	if (levelItem == NULL || levelItem->level_ == NULL){
		layerText_->setVisible(false);
		return;
	}

	QString title = levelItem->level_->description();

	if (activeLayer_){
		DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, activeLayer_);
		title += UDQ_T(" : ") + layer->description();
	}

	layerText_->setText(title);
	layerText_->setVisible(true);
	layerText_->update();
}

void LevelScene::updateLayers(){
	EditorLevel* levelItem = getLevelItem();
	if (levelItem == NULL || levelItem->level_ == NULL){
		return;
	}

	//////////////////////////////////////////////////////////////////////////
	// ����Level��С
	levelItem->refresh();
	levelItem->updateChildren();

	EditorLayer* layerItem = NULL;
	// ����ͼ������
	// ���ͼ�����¼ӵ�ͼ�㣬�����ͼ������
	for (int i = 0; i < levelItem->level_->layers_.size(); i++){
		LayerResourceObject* layer = levelItem->level_->layers_[i];

		// ���ҵ�ǰͼ��
		if (layers_.contains(layer->hashKey())){
			layerItem = layers_[layer->hashKey()];
		}
		else{
			//  ������ͼ��
			layerItem = new EditorLayer(layer);
			addItem(layerItem);
			layers_.insert(layer->hashKey(), layerItem);

			QList<QByteArray> validinst;
			// ����ͼ��Ԫ��
			foreach(QByteArray dat, layer->instances_){
				EditorRegion* inst = addInstance(dat);
				if (inst){  // ȥ����Ч�Ķ���ʵ������
					layerItem->addInstance(inst);
					validinst.append(dat);
				}
			}
			layer->instances_ = validinst;
		}
	
		Q_ASSERT(layerItem != NULL);
		layerItem->setZValue(EDITOR_DEPTH_BACK + i * 10);  // 10��ֵ��ʾһ��ͼ���ȣ����֧��100��ͼ��
		layerItem->update();
	}

	// ����ͼ���ı�
	updateLayerText();

}

void LevelScene::updateLayer(ResourceHash hash){

	if (hash == 0)
		return;

	DEFINE_RESOURCE_HASHKEY(LayerResourceObject, layer, hash);
	if (layer == NULL)
		return;

	if (hash == activeLayer_){
		updateLayerText();
	}

	EditorLayer* layerItem = layers_.value(hash);
	Q_ASSERT(layerItem != NULL);

	layerItem->setVisible(layer->isVisible_);
	// ͼ������������
	foreach(QGraphicsItem* chid, layerItem->childItems()){
		if (chid->type() > LEVEL_LAYER){
			chid->setFlag(QGraphicsItem::ItemIsMovable, !layer->isLocking_);
		}
	}

	layerItem->update();
}

void LevelScene::resizeAllLayer(QSizeF newSize){
	foreach(EditorLayer* layer, layers_.values()){
			layer->resize(newSize);
	}
}

QList<ResourceHash>  LevelScene::layerIds(){
	return layers_.keys();
}

QStringList LevelScene::layerNames(){
	QStringList names;
	foreach(EditorLayer* layer, layers_){
		names.append(layer->layerName());
	}
	return names;
}

EditorLayer* LevelScene::getLayer(const QString& name){
	foreach(EditorLayer* layer, layers_.values()){
		if (layer->layer_->description() == name){
			return layer;
		}
	}
	return NULL;
}

EditorLayer* LevelScene::getLayer(ResourceHash key){
	return layers_[key];
}

EditorLayer* LevelScene::addLayerByHash(ResourceHash key){
	DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, key);

	EditorLevel* levelItem = getLevelItem();
	if (levelItem == NULL || levelItem->level_ == NULL )
		return NULL;

	EditorLayer* layerItem = new EditorLayer(layer);
	addItem(layerItem);

	// ���Ԫ��
	QList<QByteArray> validinst;
	// ����ͼ��Ԫ��
	foreach(QByteArray dat, layer->instances_){
		EditorRegion* inst = addInstance(dat);
		if (inst){  // ȥ����Ч�Ķ���ʵ������
			layerItem->addInstance(inst);
			validinst.append(dat);
		}
	}
	// ������Ч����
	layer->instances_ = validinst;
	layers_.insert(key, layerItem);
	reorderLayers();  // ��������
	return layerItem;
}


void LevelScene::removeLayerByHash(ResourceHash key){

	EditorLevel* levelItem = getLevelItem();
	if (levelItem == NULL || levelItem->level_ == NULL)
		return;

	DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, key);  //������һ��ͼ��
	EditorLayer* layerItem = layers_.value(key);
	if(layerItem != NULL){
		if (activeLayer_ == key){  // ����Ǽ����
			activeLayer_ = 0;
			updateLayerText();
		}

		// ���浱ǰͼ������
		foreach(EditorRegion* inst, layerItem->instances_)
		{
			layer->instances_.append(inst->encode());
		}

		removeItem(layerItem);
		delete layerItem;
		layers_.remove(key);
		reorderLayers();  // ��������
	}

}

void LevelScene::reorderLayers(){

	EditorLevel* levelItem = getLevelItem();
	if (levelItem == NULL || levelItem->level_ == NULL)
		return;

	LayerResourceObject* layer = NULL;
	for (int i = 0; i < levelItem->level_->layers_.size(); i++){
		layer = levelItem->level_->layers_[i];

		if (layer->flags() & URF_TagGarbage)
			continue;

		EditorLayer* layerItem = layers_.value(layer->hashKey());
		Q_ASSERT(layerItem != NULL);
		layerItem->setZValue(EDITOR_DEPTH_BACK + i* 10);
	}
}

QImage LevelScene::screenshot(){

	// ��ȡ��ǰ���ű���
	EditorLevel* level = getLevelItem();
	if (level == NULL){
		return QImage();
	}

	clearSelection();

	QRectF r = level->sceneBoundingRect();

	// ����ͼƬ
	QImage image(r.size().toSize(), QImage::Format_ARGB32);
	image.fill(Qt::transparent);

	bool showViewport = viewPortItem_->isVisible();
	viewPortItem_->setVisible(false);

	bool showPanel = macPanel_->isVisible();
	macPanel_->setVisible(false);

	// ����ͼƬ
	QPainter painter(&image);
	render(&painter, QRectF(), r);

	viewPortItem_->setVisible(showViewport);
	macPanel_->setVisible(showPanel);

	return image;
}

void LevelScene::toggleOption(){
	// ���ö���
	if (!macPanel_->isVisible()){
		macPanel_->setVisible(true);
	}
	else{
		macPanel_->setVisible(false);
	}
}

void LevelScene::saveToObjectDefault(){
	Q_ASSERT(menuHostItem_ != NULL);
	((EditorRegion*)menuHostItem_)->saveDefault();

}

void LevelScene::loadFromObjectDefault(){
	Q_ASSERT(menuHostItem_ != NULL);
	((EditorRegion*)menuHostItem_)->loadDefault();
	menuHostItem_->update();
}


void LevelScene::cloneObjectType(){
	// ���Ƴ��µĶ�������
	Q_ASSERT(menuHostItem_ != NULL);
	EditorRegion* region = (EditorRegion*)menuHostItem_;

	ObjectTypeResourceObject* oldType = region->typePrototype_;
	Q_ASSERT(oldType != NULL);

	ObjectTypeResourceObject* newType = addObjectTypeByHash(oldType->prototype_->hashKey());
	Q_ASSERT(newType != NULL);

	oldType->copyObject(newType);
	region->typePrototype_ = newType;
	region->saveDefault();

}

void LevelScene::copyInstance(){
	copyData_.clear();
	cutObjects_.clear();

	if (menuHostItem_){
		// ������ǰ����
		copyData_.append(menuHostItem_->encode());
		menuHostItem_ = NULL;
	}
	else{
		// ��������ѡ�еĶ���
		QList<QGraphicsItem*> items = selectedItems();
		foreach(QGraphicsItem* item, items){
			// ɾ��ѡ�е�ʵ��
			if (item->type() > LEVEL_LAYER){
				EditorRegion* region = (EditorRegion*)item;
				copyData_.append(region->encode());
			}
		}
	}

}

void LevelScene::cutInstance(){
	copyData_.clear();
	cutObjects_.clear();

	if (menuHostItem_){
		Q_ASSERT(menuHostItem_->type() > LEVEL_LAYER);
		// ������ǰ����ɾ������
		EditorRegion* region = (EditorRegion*)menuHostItem_;
		cutObjects_.append(region);
		removeInstance(region);
		menuHostItem_ = NULL;
	}
	else{
		// ��������ѡ�еĶ���
		QList<QGraphicsItem*> items = selectedItems();
		foreach(QGraphicsItem* item, items){
			// ɾ��ѡ�е�ʵ��
			if (item->type() > LEVEL_LAYER){
				cutObjects_.append((EditorRegion*)item);
				removeInstance((EditorRegion*)item);
			}
		}
	}
}

void LevelScene::pasteInstance(){

	// ����Ƿ�����Ч������ͼ��
	EditorLevel* levelItem = getLevelItem();
	// �ж��Ƿ��е�ǰҳ( һ��ҳ�� )
	if (levelItem == NULL || levelItem->level_ == NULL ){
		QMessageBox::critical(views().at(0), UDQ_TR("ҳ��༭"), UDQ_TR("���ȴ�һ������!"));
		return;
	}

	// ���뼤�ǰͼ��
	if (activeLayer_ == 0){
		QMessageBox::critical(views().at(0), UDQ_TR("ҳ��༭"), UDQ_TR("����ѡ��һ��ͼ��!"));
		return;
	}

	QList<QByteArray> newCopyData;
	foreach(QByteArray data, copyData_){
		EditorRegion* region = addInstance(data);
		// λ��ƫ��
		region->setPos(region->pos() + QPointF(50, 50));
		layers_[activeLayer_]->addInstance(region);
		newCopyData.append(region->encode());
	}
	copyData_ = newCopyData;

	foreach(EditorRegion* region, cutObjects_){
		addItem(region);
		layers_[activeLayer_]->addInstance(region);
	}

	cutObjects_.clear();
}

void LevelScene::updateTileMap(EditorTileMap* map){
	Q_ASSERT(map->typePrototype_!= NULL );
	if (!map->isSelected()){
		map->updateBrush(QRect(),0);
		emit refreshTileMap(NULL);
	}
	else{
		emit refreshTileMap(map);
	}

}


void LevelScene::setTileBrush(const QRect& region, int mode){
	QList<QGraphicsItem*> selItems = selectedItems();
	foreach(QGraphicsItem* item, selectedItems()){
		if (item->type() == LEVEL_TILEMAP){
			EditorTileMap* map = (EditorTileMap*)item;
			map->updateBrush(region, mode);
			break;
		}
	}
}

void LevelScene::mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent){
	QGraphicsScene::mouseMoveEvent(mouseEvent);
}

void LevelScene::updateRegions(ObjectTypeResourceObject* obj){
	EditorRegion* region = NULL;
	foreach(EditorItem* item, regionObjects_){	
		if (item->type() > LEVEL_LAYER){
			region = (EditorRegion*)item;
			if (region->typePrototype_ == obj ||obj == NULL ){
				region->postinit(false);
				region->update();
			}
		}
	}

	foreach(EditorLayer* layer, layers_){
		layer->updateRegions(obj);
	}
}

void LevelScene::notifyUpdateProperty(EditorItem* item){
	emit itemChanged(item);
}

void LevelScene::notifyInstanceModified(EditorItem* item, const QByteArray& olddata){
	emit entityModified(item, olddata);
}


void LevelScene::notifyUpdateObjectType(EditorRegion* region){
	if (region->typePrototype_){
		emit refreshObjectType(region->typePrototype_->hashKey());
	}
}

void LevelScene::moveInstanceToFirst(){
	if (menuHostItem_ == NULL || menuHostItem_->parentItem() == NULL )
		return;

	// �ƶ���ǰ��������ͼ���������
	EditorLayer* layer = (EditorLayer*)menuHostItem_->parentItem();
	Q_ASSERT(layer != NULL);
	layer->moveFirst((EditorRegion*)menuHostItem_);

}

void LevelScene::moveInstanceToLast(){
	if (menuHostItem_ == NULL || menuHostItem_->parentItem() == NULL)
		return;

	// �ƶ���ǰ��������ͼ���������
	EditorLayer* layer = (EditorLayer*)menuHostItem_->parentItem();
	Q_ASSERT(layer != NULL);
	layer->moveLast((EditorRegion*)menuHostItem_);

}

void LevelScene::removeLayer(EditorLayer* layer){
	removeItem(layer);
	layers_.remove(layer->layer_->hashKey());
}

void LevelScene::addLayer(EditorLayer* layer){
	addItem(layer);
	layers_.insert(layer->layer_->hashKey(), layer);
}