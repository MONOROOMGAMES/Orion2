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
#ifndef ORION_KISMET_MODEL_H
#define ORION_KISMET_MODEL_H

#include <QGraphicsItem>
#include <QGraphicsSimpleTextItem>
#include <QFont>
#include <QList>
#include <QPushButton>

#include "kismetdefine.h"
#include "kismetobject.h"
#include "kismetlink.h"
#include "kismetmodelport.h"
#include "resourceobject.h"
using namespace ORION;


class BlueprintButtonControl : public QGraphicsProxyWidget{
	Q_OBJECT
public:
	// ʵ������
	ORION_TYPE_DECLARE(KISMET_MODEL_BUTTON);

	enum{
		BTN_ADD_IN = 1,
		BTN_ADD_OUT,
	};

	BlueprintButtonControl(int type, const QString& text, const QString& datatype, const QIcon& ico = QIcon(), const QString& prefix = QString(), QGraphicsItem * parent = 0);
	virtual ~BlueprintButtonControl();

	QPushButton* button(){
		return button_;
	}

public slots:
	void addInCtrlPort();
	void addOutCtrlPort();
	void addInDataPort();
	void addOutDataPort();

protected:
	void hoverEnterEvent(QGraphicsSceneHoverEvent * event);
	void hoverLeaveEvent(QGraphicsSceneHoverEvent * event);
	virtual void mousePressEvent(QGraphicsSceneMouseEvent * event);
	// ��ʼ����ť
	void prepareWidget(const QString& text, const QIcon& ico);

	QPushButton* button_;  // ���ݿؼ�
	int type_;  // ��������
	QString datatype_;  // ��������
	QString prefix_;  // �˿���ǰ׺

};

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////


class BlueprintModel  : public BlueprintObject{
public:
	// ʵ������
	ORION_TYPE_DECLARE( KISMET_MODEL );

	BlueprintModel(QGraphicsItem * parent = 0);
	BlueprintModel(ResourceHash pluginHash, int type, ResourceHash inter, QGraphicsItem * parent = 0);
	virtual ~BlueprintModel();

	// ����
	virtual void encodeData(QDataStream& stream);
	virtual void decodeData(QDataStream& stream);

	virtual bool validate(QStringList& msg);

	void paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget);

	int modelType(){
		return type_;
	}

	//////////////////////////////////////////////////////////////////////////

	void setPortValue(const QString& name, const QString& v);
	void setPortValue(const QString& name, int v);
	void setPortValue(const QString& name, double v);
	void setPortValue(const QString& name, bool v);

	//////////////////////////////////////////////////////////////////////////
	BlueprintModelPort* addCtrlInPort(const QString& port, int flags = 0);
	BlueprintModelPort* addCtrlOutPort(const QString& port, int flags = 0);

	BlueprintModelPort* addDataInPort(const QString& port, const QString& datatype, int flags = 0, const QStringList& items = QStringList());
	BlueprintModelPort* addDataOutPort(const QString& port, const QString& datatype, int flags = 0, const QStringList& items = QStringList());

	// �滻�˿�ID
	void changePortIID(BlueprintModelPort* port, ResourceHash iid);

	void removePort(BlueprintModelPort* port);
	void deletePort(BlueprintModelPort* port, QList<BlueprintModelPort*>& container);
	//////////////////////////////////////////////////////////////////////////
	// �¶˿���
	virtual QString newPortName( int type, const QString& prefix = QString() );

	// ������ж˿�
	void removeAllPort();

	//////////////////////////////////////////////////////////////////////////
	// ��ʼ��
	virtual void init();
	// ������С
	virtual void adjust();

	virtual QPointF logicInPos( int index = -1);
	virtual QPointF logicOutPos(  int index = -1);

	//  ��ȡ������
	virtual QString title();
	// ���ͱ�ʾ
	virtual QString typeName();
	//  ��ȡ����ͼ��64, 64��
	virtual QImage thumbnail();

	void toggleLock();
	bool isLocking( void ){
		return isLocking_;
	}

	// ����IID���Ҷ˿�( ���ж˿ڱ��������֣����ұ���Ψһ)
	BlueprintModelPort* findPort(ResourceHash iid);
	BlueprintModelPort* findPort(const QString& name, int orient );

	BlueprintModelPort* inport(const QString& name);
	BlueprintModelPort* outport(const QString& name);
	BlueprintModelPort* inCtrlPort(const QString& name);
	BlueprintModelPort* outCtrlPort(const QString& name);

	// ������ж˿�����
	void clearPortLink();

	// �˿������б�
	QStringList  portNames();
	QStringList  inportNames();
	QStringList  outportNames();

	// ���¶˿�
	void updatePort();

	// ׼���˿�
	void prepareCtrlPort(PluginInterface* inter);
	// ׼��ȱʡ�˿�
	void prepareDefPort(PluginInterface* inter);
	// ׼�������˿�
	void prepareInPort(PluginVariable* var);
	// ���ư�ť�˿�
	void prepareButtonPort(PluginInterface* inter);

	// ���ƶ˿�
	QList<BlueprintModelPort*>  inCtrlPorts_;  // Ĭ��Ϊ1����
	QList<BlueprintModelPort*>  outCtrlPorts_;  // Ĭ��Ϊ1���������ж��
	// ���ݶ˿�
	QList<BlueprintModelPort*>  inPorts_;
	QList<BlueprintModelPort*>  outPorts_;

	QList<BlueprintButtonControl*> inButtons_;  // ����˰�ť
	QList<BlueprintButtonControl*> outButtons_;  // ����˰�ť

	// �������ж˿�
	QList<BlueprintModelPort*> ports(){
		return iidPorts_.values();
	}

	// ��ȡPort�ı��ʽ
	QJsonArray portExpNode(BlueprintModelPort* port = NULL);
	QJsonValue objTypeNode();

	//////////////////////////////////////////////////////////////////////////
	// ����ģ���Ƿ����ָ������
	bool hasLinkModel(QList<BlueprintModelPort*> ports, int type, int subtype);
	bool hasDownStream(int type, int subtype = 0);
	// ����ģ���Ƿ����ָ������
	bool hasUpStream(int type, int subtype = 0);

	// ��ȡ����ģ��
	BlueprintModel* downStreamCtrl( const QString& name );
	// ����ģ���б�
	QList<BlueprintModel*> upstreamCtrl();
	QList<BlueprintModel*> upstreamCtrl(int type, int subtype = 0);

	// ���Ҷ�Ӧ����˿�
	QList<BlueprintModelPort*> modelInPort(BlueprintModel* model);
	QList<BlueprintModelPort*> modelOutPort(BlueprintModel* model);

	PluginResourceObject* pluginObject(){
		return plugin_;
	}

	ResourceHash interfaceId(){
		return interfaceID_;
	}

	PluginInterface* getInterface();

	//  �Ƿ�Ϊ�ڲ��˿�
	bool isInternalPort(BlueprintModelPort* port);

protected:
	virtual QVariant itemChange ( GraphicsItemChange change, const QVariant & value );
	virtual void mousePressEvent ( QGraphicsSceneMouseEvent * event );
	virtual void contextMenuEvent(QGraphicsSceneContextMenuEvent *event);
protected:
	QColor tilteColor();
	void drawLock( QPainter* painter );
	void drawPreview(QPainter* painter);

	QColor color_;//
	bool isLocking_; // �Ƿ�����

	QMap<ResourceHash, BlueprintModelPort*> iidPorts_; 

	int bomPos_; // �ײ�˿�λ��
	QImage lockImage_;

	PluginResourceObject* plugin_; // �������
	BlueprintModelPort* objPort_;  // �ڲ�����˿�
	BlueprintModelPort* behPort_;  // �ڲ���Ϊ�˿�
	int type_;  // ����( �����������ͱ��ʽ )
	QString title_;  // ����
	ResourceHash interfaceID_;  // ID
	PluginInterface* interface_;   // �ӿڶ���

};


#endif
