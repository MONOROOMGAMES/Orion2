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
**  along with Orion2.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#ifndef  ORION_KISTMET_SCENE_H
#define ORION_KISTMET_SCENE_H

#include <QGraphicsScene>
#include <QMap>
#include <QPixmap>

#include "kismetgroup.h"
#include "kismetcomment.h"
#include "kismetobject.h"
#include "kismetmodel.h"
#include "kismetvariable.h"
#include "kismetinstance.h"
#include "kismetflowcontrol.h"
#include "kismetarithmetic.h"
#include "kismetcollection.h"
#include "kismetinstancevar.h"
#include "selectobjecttypewindow.h"

#include "resourceobject.h"
#include "resourcelinker.h"
using namespace ORION;

class QDragEnterEvent;
class QDragLeaveEvent;
class QDragMoveEvent;
class QDropEvent;

class KismetScene : public QGraphicsScene{
	Q_OBJECT

public:

	enum KISMET_MODE{
		DOM_POINTER = 0,        // ��ʼ״̬��ѡ��, �ƶ���
		DOM_ADDGROUP,         // ���Group
		DOM_ADDCOMMENT,   // ���ע��
		DOM_ADDLINK,   // �������
	};

	KismetScene(QObject *parent = 0);
	virtual ~KismetScene();

	KISMET_MODE getMode(void){
		return mapMode_;
	}
	
	// ��ȡ�ɼ����򣨳�������ϵ��
	QRectF viewRegionRect( void );
	// ˢ�½ű�
	void refreshScript();
	// ����
	void clearScene();
	// ����
	bool saveCurrentScript();
	// ����
	void loadScript(BlueprintResourceObject* blueprint);

	void enterInsertGroup();
	void enterInsertComment();
	void exitInsertState();

	// ���½ű���
	void updateScriptText();
	void updateVariableItem(ScriptVariable* var);
	void removeVariableItem(ScriptVariable* var);

	QList<BlueprintVariable*> variables(const QString& name);
	QList<BlueprintVariable*> variableItems(ScriptVariable* var);

	// �½�����
	BlueprintGroup* addGroupItem( const QString& text, const QPointF& pos);
	BlueprintComment* addCommentItem(const QString& text, const QPointF& pos);
	BlueprintLink* addLinkItem(BlueprintObject* source, BlueprintObject* dest, QList<QPointF>& ctrlPts );

	BlueprintModel* addEventItem(const QPointF& pos, const QByteArray& data);
	BlueprintVariable* addVariableItem(const QPointF& pos, ResourceHash key, const QString& name);
	BlueprintInstance* addInstanceItem(const QPointF& pos, ResourceHash key);
	BlueprintCollection* addCollectionItem(const QPointF& pos, ResourceHash key);
	// ��ӷ�֧
	BlueprintBranch* addBranchItem();
	BlueprintArithmetic* addMathItem();
	BlueprintSequence* addSequenceItem();
	BlueprintInstanceVariable* addInstanceVarItem();

	QStringList groupNames();

	//  �����������
	void clearLink();

	BlueprintObject* menuHost(){
		return menuHostItem_;
	}

	void setMenuHost(BlueprintObject* host){
		menuHostItem_ = host;
	}

	BlueprintResourceObject* parentModelInstance(){
		return parentInstance_;
	}

	// ����¼��˿�( ��ģ���н������)
	BlueprintGroup* addGroupItem( QByteArray& data, ResourceHash newid = 0);
	BlueprintComment* addCommentItem(QByteArray& data, ResourceHash newid = 0);
	BlueprintObject* addModelItem(QByteArray& data, ResourceHash newid = 0);

	BlueprintLink* addLinkItem(QByteArray& data);
	BlueprintObject* decodePortFromData(QByteArray& data);

	// ����IID��ȡ����
	BlueprintObject* getObjectItem(ResourceHash iid);
	void addObjectItem(BlueprintObject*);
	void removeObjectItem(BlueprintObject*);
	void deleteObjectItem(BlueprintObject*);

	//////////////////////////////////////////////////////////////////////////
	// �ű���飬���ش�����ʾ��Ϣ
	bool validateScript();
	// ���������ģ�ͺϷ���
	bool checkModelSequence(BlueprintModel* up, BlueprintModel* down);
	void saveToJson();
	// �����¼������
	QJsonArray generateBlockCode(BlueprintModel* head);
	QJsonArray generateBlockElseCode(BlueprintModel* head);
	QJsonArray generateActionsCode(BlueprintModel* head, bool isElse = false);
	QJsonArray generateSubsCode(BlueprintModel* head, bool isElse = false );

	QStringList& scriptErrors(){
		return scriptErrors_;
	}

	QList<BlueprintGroup*>& groupObjects(){
		return grpObjects_;
	}

	int groupIndex(BlueprintGroup* grp){
		return grpObjects_.indexOf(grp);
	}

	QString newGroupName();

	// ѡ���������
	void selectObjectType(const QPointF& pos);

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	void notifyObjectInsert(QGraphicsItem* item);
	void notifyLinkChanged(BlueprintLink* link);

protected:
	virtual void mousePressEvent ( QGraphicsSceneMouseEvent * mouseEvent );
	virtual void mouseReleaseEvent(QGraphicsSceneMouseEvent * mouseEvent);
	virtual void dragEnterEvent ( QGraphicsSceneDragDropEvent * event );
	virtual void dragLeaveEvent ( QGraphicsSceneDragDropEvent * event );
	virtual void dragMoveEvent ( QGraphicsSceneDragDropEvent * event );
	virtual void dropEvent ( QGraphicsSceneDragDropEvent * event );
	virtual void keyPressEvent ( QKeyEvent * keyEvent );
	virtual void keyReleaseEvent ( QKeyEvent * keyEvent );
	virtual void drawBackground ( QPainter * painter, const QRectF & rect );
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
	virtual void mouseDoubleClickEvent(QGraphicsSceneMouseEvent * mouseEvent);

public slots:
    void setMode(KISMET_MODE mode);
	void addModelPort();
	void addInstanceVariable();
	void addInstanceIndex();
	void deleteModelPort();
	void clearModelPortLink();

	void removeVariable(const QString& name);
	void updateVariable(const QString& name);

signals:
	void entityInserted( QGraphicsItem* entity );
	void entityResized(QGraphicsItem* entity);
	void entityDeleted(const QList<QGraphicsItem*>& items);
	void entityMoved(const QList<QGraphicsItem*>& items);
	void entityCopy(const QList<QGraphicsItem*>& items, const QPointF& pos);
	void entityModified(QGraphicsItem* entity, const QByteArray& oldData);

	void entityExpanded( unsigned int hashkey );
	void entityCollapsed( unsigned int hashkey );

	void pointerMode();

private:
	KISMET_MODE   mapMode_;     // ��ͼģʽ
	bool insertInputEvent_; // �Ƿ���������¼�

	QColor bgColor_;  // ������ɫ
	QGraphicsSimpleTextItem*  scriptText_;  // ͼ����Ϣ

	BlueprintObject* menuHostItem_;
	BlueprintResourceObject* parentInstance_;   // ģ�͸�ʵ��

	QList<BlueprintModel*> modelObjects_;   // ( ģ�� )
	QList<BlueprintVariable*>  varObjects_;  // ����
	QList<BlueprintInstance*>  instObjects_;  // ʵ������
	QList<BlueprintCollection*> aggrObjects_;  // ���϶���
	QList<BlueprintObject*> assetObjects_;   // ( ע�� ���������� )
	QList<BlueprintGroup*> grpObjects_;   // ( ��)

	QHash<ResourceHash, BlueprintObject*>  iidObjects_;  // ����IIDӳ��

	QStringList scriptErrors_; // ������Ϣ


	// ���㵱ǰͼ�����Ч����
	QRect validRegion();
	QRect validRegionGroup(BlueprintGroup* grp);
	QRect validRegionViewPort();
	QList<BlueprintGroup*> selectedGroups();
	void screenShotGroup(BlueprintGroup* grp);

	//////////////////////////////////////////////////////////////////////////
	QMap<ResourceHash, QByteArray>   oldData_;  // ��ǰѡ�ж��������

};


#endif
