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

#ifndef  ORION_LEVELSCENE_H
#define ORION_LEVELSCENE_H

#include <QGraphicsScene>
#include <QGraphicsSimpleTextItem>
#include <QGraphicsRectItem>
#include <QGraphicsPixmapItem>
#include <QMap>
#include <QPixmap>
#include "editorlogo.h"
#include "macdockpanelitem.h"
#include "editorviewport.h"
#include "editoritem.h"
#include "editorlevel.h"
#include "editorsprite.h"
#include "editortext.h"
#include "editortilebg.h"
#include "editorlayer.h"
#include "editorshadowlight.h"
#include "editorninepatch.h"
#include "editorparticle.h"
#include "editortilemap.h"
#include "editorobject.h"
#include "editormap.h"
#include "editorinput.h"
#include "resourceobject.h"
using namespace ORION;

class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

class LevelScene : public QGraphicsScene{
	Q_OBJECT

public:

	enum LEVEL_MODE{
		DOM_POINTER = 0,        // ��ʼ״̬��ѡ��, �ƶ���
		DOM_TILEEDIT,               // �༭TILE��ͼ
	};

	LevelScene(QObject *parent = 0);
	virtual ~LevelScene();

	LEVEL_MODE getMode(void){
		return mapMode_;
	}
	
	// ���س���
	void loadLevel(LevelResourceObject* level);

	void closeCurrentLevel( bool save = true );
	void saveLevel(LevelResourceObject* level);
	void clearLevel();

	// ��ȡ��ǰ�ĳ�������
	EditorLevel* getLevelItem();

	// ��ȡ�ɼ����򣨳�������ϵ��
	QRectF viewRegionRect( void );

	EditorLayer* addLayerByHash(ResourceHash key);
	void removeLayerByHash(ResourceHash key);
	void removeLayer(EditorLayer* layer);
	void addLayer(EditorLayer* layer);

	EditorLayer* getLayer(const QString& name);
	EditorLayer* getLayer(ResourceHash key);

	EditorRegion* addInstance(QByteArray& data);
	EditorSprite* addSpriteItem(QByteArray& data );
	EditorText* addTextItem(QByteArray& data);
	EditorTileBG* addTileBGItem(QByteArray& data);
	EditorShadowLight* addShadowLightItem(QByteArray& data);
	EditorNinePatch* add9PatchItem(QByteArray& data);
	EditorParticle* addParticleItem(QByteArray& data);
	EditorTileMap* addTileMapItem(QByteArray& data);
	EditorObject* addEntityObjectItem(QByteArray& data);
	EditorMap* addMapItem(QByteArray& data);
	EditorInputButton* addInputButtonItem(QByteArray& data);
	EditorInputTextBox* addInputTextBoxItem(QByteArray& data);
	EditorInputList* addInputListItem(QByteArray& data);
	EditorInputFileChooser* addInputFileChooserItem(QByteArray& data);
	EditorInputProgressBar* addInputProgressBarItem(QByteArray& data);
	EditorInputSliderBar* addInputSliderBarItem(QByteArray& data);

	void addRootInstance(EditorRegion* region);

	void deleteInstance(EditorRegion* region);
	void removeInstance(EditorRegion* region);
	// ����ͼ�����
	void reorderLayers();

	// �������������¶���
	void updateRegions(ObjectTypeResourceObject* obj = NULL );
	// ����Tilemap��ʾ
	void updateTileMap(EditorTileMap* map);

	ObjectTypeResourceObject* addObjectTypeByHash(ResourceHash key);
	EditorSprite* addSpriteByPlugin(ResourceHash key, const QPointF& pos);
	EditorText* addTextByPlugin(ResourceHash key, const QPointF& pos);
	EditorTileBG* addTileBGByPlugin(ResourceHash key, const QPointF& pos);
	EditorShadowLight* addShadowLightByPlugin(ResourceHash key, const QPointF& pos);
	EditorNinePatch* add9PatchByPlugin(ResourceHash key, const QPointF& pos);
	EditorParticle* addParticleByPlugin(ResourceHash key, const QPointF& pos);
	EditorTileMap* addTileMapByPlugin(ResourceHash key, const QPointF& pos);
	EditorObject* addEntityObjectByPlugin(ResourceHash key, const QPointF& pos);
	EditorMap* addMapByPlugin(ResourceHash key, const QPointF& pos);
	EditorRegion* addInputByPlugin(ResourceHash key, const QPointF& pos);

	void addSingleGlobalByPlugin(ResourceHash key);

	MacDockPanelItem* macpanelItem(){
		return macPanel_;
	}

	EditorItem* menuHost(){
		return menuHostItem_;
	}

	void setMenuHost(EditorItem* host){
		menuHostItem_ = host;
	}

	// ����Tileˢ
	void setTileBrush(const QRect& region, int mode ); // 0��ʾ��ӣ�1��ʾɾ��

	// ����ͼ��
	void activeLayer(ResourceHash hash);
	void updateLayer(ResourceHash hash);
	void updateLayerText();
	void updateLayers();

	void resizeAllLayer(QSizeF newSize );
	// ����ͼ���б�
	QList<ResourceHash>  layerIds();
	QStringList layerNames();

	//  ����ͼ
	QImage screenshot();
	//
	void deleteInstanceByType(ObjectTypeResourceObject* obj);
	// ����IDɾ��ʵ��
	void deleteInstanceByID(ResourceHash iid);
	EditorItem* findInstanceByID(ResourceHash iid);

	// ֪ͨ��������
	void notifyUpdateProperty(EditorItem* item);
	void notifyInstanceModified(EditorItem* item, const QByteArray& olddata );

	void notifyUpdateObjectType(EditorRegion* region);

protected:
	virtual void mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent );
	virtual void mouseMoveEvent(QGraphicsSceneMouseEvent * mouseEvent);
	virtual void dragEnterEvent ( QGraphicsSceneDragDropEvent * event );
	virtual void dragLeaveEvent ( QGraphicsSceneDragDropEvent * event );
	virtual void dragMoveEvent ( QGraphicsSceneDragDropEvent * event );
	virtual void dropEvent ( QGraphicsSceneDragDropEvent * event );
	virtual void keyPressEvent ( QKeyEvent * keyEvent );
	virtual void keyReleaseEvent ( QKeyEvent * keyEvent );
	virtual void drawBackground ( QPainter * painter, const QRectF & rect );
	virtual void drawForeground(QPainter * painter, const QRectF & rect);
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent * contextMenuEvent);


public slots:
    void setMode(LEVEL_MODE mode);
	void toggleLock();
	void toggleOption();
	void saveToObjectDefault();
	void loadFromObjectDefault();

	void cloneObjectType();
	void copyInstance();
	void cutInstance();
	void pasteInstance();

	void moveInstanceToFirst();
	void moveInstanceToLast();

signals:
	void entitySelected( QGraphicsItem* entity );
	void pointerMode();
	void addObjectType( unsigned int hashkey );
	void refreshObjectType(unsigned int haskey);
	void refreshTileMap(QGraphicsItem* entity);
	void itemChanged(QGraphicsItem* item);

	// undo
	void entityModified(QGraphicsItem* item, const QByteArray& data);
	void entityInserted(QGraphicsItem* item, unsigned int hashkey);
	void entityDeleted(QGraphicsItem* item, unsigned int hashkey);

private:
	LEVEL_MODE   mapMode_;     // ��ͼģʽ
	QColor bgColor_;  // ������ɫ

	MacDockPanelItem* macPanel_;
	EditorLogoPtr logoItem_;

	EditorItem* menuHostItem_;
	EditorViewport* viewPortItem_;

	QGraphicsSimpleTextItem*  layerText_;  // ͼ����Ϣ
	QGraphicsRectItem* dropItem_; // �ϷŽ����־

	// ��ʼ��
	bool initMacPanel(void);
	// ��ʼ��LOGO
	bool initLogo();

	QList<EditorItem*> regionObjects_; // �����б�����ͼ���еĶ����б�
	QMap<ResourceHash, EditorLayer*>  layers_;  // ͼ����

	ResourceHash activeLayer_;

	QList<QByteArray> copyData_;
	QList<EditorRegion*> cutObjects_;

	QList<QRect>  tileBrush_;  // ����ˢ left=x, top = y, right = index, bottom = angle
	QImage tileThumb_;  // ˢ��ͼƬ
};


#endif
