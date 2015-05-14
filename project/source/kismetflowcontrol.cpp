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

#include "kismetflowcontrol.h"
#include <QtGui>
#include <math.h>
#include "kismetdefine.h"
#include "kismetscene.h"
#include "tooldefine.h"


BlueprintBranch::BlueprintBranch(QGraphicsItem * parent) : BlueprintModel(parent){
	prepare();
	adjust();
}

BlueprintBranch::~BlueprintBranch(){

}

void BlueprintBranch::init(){
	BlueprintModelPort* modelport = NULL;
	foreach(QString pt, addPorts_){
		modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_IN, UDQ_T("ctrl"), pt);
		modelport->setObjectFlags(BlueprintObject::OF_CAN_DELETE);
		inCtrlPorts_.append(modelport);
		modelport->setParentItem(this);
		iidPorts_.insert(modelport->iid(), modelport);
	}
	addPorts_.clear();
}

void BlueprintBranch::prepare(){
	removeAllPort();

	title_ = UDQ_TR("��֧����");
	color_ = CONTROL_COLOR;

	// ׼����������
	BlueprintModelPort* modelport = NULL;
	modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_IN, UDQ_T("ctrl"), UDQ_T("in1"));
	inCtrlPorts_.append(modelport);
	modelport->setParentItem(this);
	iidPorts_.insert(modelport->iid(), modelport);

	modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_IN, UDQ_T("ctrl"), UDQ_T("in2"));
	inCtrlPorts_.append(modelport);
	modelport->setParentItem(this);
	iidPorts_.insert(modelport->iid(), modelport);

	// ׼���������
	modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_OUT, UDQ_T("ctrl"), UDQ_T("true"));
	outCtrlPorts_.append(modelport);
	modelport->setParentItem(this);
	iidPorts_.insert(modelport->iid(), modelport);

	modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_OUT, UDQ_T("ctrl"), UDQ_T("false"));
	outCtrlPorts_.append(modelport);
	modelport->setParentItem(this);
	iidPorts_.insert(modelport->iid(), modelport);

	// �������ݶ˿�
	QStringList items;
	items << UDQ_TR("AND") << UDQ_TR("OR");
	logicPort_ = new BlueprintModelPort(BlueprintModelPort::EVT_DATA_IN, UDQ_T("enum"), UDQ_T("logic"), items);
	inPorts_.append(logicPort_);
	logicPort_->setParentItem(this);
	iidPorts_.insert(logicPort_->iid(), logicPort_);

	// ׼�����밴ť
	BlueprintButtonControl* btn = NULL;
	btn = new BlueprintButtonControl(BlueprintButtonControl::BTN_ADD_IN, UDQ_TR("in"), UDQ_T("ctrl"), QIcon(UDQ_T(":/images/add.png")));
	inButtons_.append(btn);
	btn->setParentItem(this);

}

void BlueprintBranch::encodeData(QDataStream& stream){
	BlueprintModel::encodeData(stream);

	// ����ӵĶ˿�
	addPorts_.clear();
	foreach(BlueprintModelPort* port, inCtrlPorts_)
	{
		if (port->canDelete()){
			addPorts_.append(port->name());
		}
	} 
	stream << addPorts_;
}

void BlueprintBranch::decodeData(QDataStream& stream){
	BlueprintModel::decodeData(stream);
	stream >> addPorts_;
	// ����
	init();
	adjust();
}

bool BlueprintBranch::isORLogic(){
	QString logic = logicPort_->getValue();
	return logic == UDQ_T("OR");
}

bool BlueprintBranch::validate(QStringList& msg){
	BlueprintModel::validate(msg);

	//////////////////////////////////////////////////////////////////////////
	// ����Ƿ�֧
	QList<BlueprintModel*> upmodels = upstreamCtrl(KISMET_FLOW_BRANCH);
	if (upmodels.size() > 1){
		isError_ = true;
		msg.append(ERROR_MSG.arg(UDQ_TR("��֧ģ�͵Ŀ��������в��ܽ�������1�����ϵķ�֧ģ��!")));
	}
	else if (upmodels.size() == 1){
		QList<BlueprintModelPort*> ports = upmodels[0]->modelOutPort(this);
		if (ports.size() > 1){
			isError_ = true;
			msg.append(ERROR_MSG.arg(UDQ_TR("��֧ģ�͵Ŀ�����������ͬһ����֧ģ�͵�2����ͬ�������!")));
		}
		else {
			Q_ASSERT(ports.size() == 1);
			QString portname = ports[0]->name();
			if (portname == UDQ_T("true") && !hasUpStream(KISMET_MODEL, PT_CONDITION)){  // �������true�˿ڣ����������������
				isError_ = true;
				msg.append(ERROR_MSG.arg(UDQ_TR("��֧ģ�͵ı�������������!")));
			}
			else if (portname == UDQ_T("false") && !hasUpStream(KISMET_MODEL, PT_CONDITION)){//�������ģ���Ƿ�֧��False�˿ڣ�����û�������������룬��False�˿ڲ�������
				BlueprintModelPort* falseport = outCtrlPort(UDQ_T("false"));
				if (falseport && falseport->hasLink()){
					isError_ = true;
					msg.append(ERROR_MSG.arg(UDQ_TR("û��������������ķ�֧ģ�Ͳ���ʹ��False����˿�!")));
				}
			}
		}
	}

	return !isError_;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

BlueprintSequence::BlueprintSequence(QGraphicsItem * parent) : BlueprintModel(parent){
	prepare();
	adjust();
}

BlueprintSequence::~BlueprintSequence(){

}

void BlueprintSequence::init(){
	BlueprintModelPort* modelport = NULL;
	foreach(QString pt, addPorts_){
		modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_OUT, UDQ_T("ctrl"), pt);
		modelport->setObjectFlags(BlueprintObject::OF_CAN_DELETE);
		outCtrlPorts_.append(modelport);
		modelport->setParentItem(this);
		iidPorts_.insert(modelport->iid(), modelport);
	}
	addPorts_.clear();
}

void BlueprintSequence::prepare(){
	removeAllPort();

	title_ = UDQ_TR("˳��ִ��");
	color_ = CONTROL_COLOR;

	// ׼����������
	BlueprintModelPort* modelport = NULL;
	modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_IN, UDQ_T("ctrl"), UDQ_T("in"));
	inCtrlPorts_.append(modelport);
	modelport->setParentItem(this);
	iidPorts_.insert(modelport->iid(), modelport);

	// ׼���������
	modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_OUT, UDQ_T("ctrl"), UDQ_T("out1"));
	outCtrlPorts_.append(modelport);
	modelport->setParentItem(this);
	iidPorts_.insert(modelport->iid(), modelport);

	modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_OUT, UDQ_T("ctrl"), UDQ_T("out2"));
	outCtrlPorts_.append(modelport);
	modelport->setParentItem(this);
	iidPorts_.insert(modelport->iid(), modelport);

	// ׼�����밴ť
	BlueprintButtonControl* btn = NULL;
	btn = new BlueprintButtonControl(BlueprintButtonControl::BTN_ADD_OUT, UDQ_TR("out"), UDQ_T("ctrl"), QIcon(UDQ_T(":/images/add.png")));
	outButtons_.append(btn);
	btn->setParentItem(this);

}

void BlueprintSequence::encodeData(QDataStream& stream){
	BlueprintModel::encodeData(stream);

	// ����ӵĶ˿�
	addPorts_.clear();
	foreach(BlueprintModelPort* port, outCtrlPorts_)
	{
		if (port->canDelete()){
			addPorts_.append(port->name());
		}
	}
	stream << addPorts_;
}

void BlueprintSequence::decodeData(QDataStream& stream){
	BlueprintModel::decodeData(stream);
	stream >> addPorts_;
	// ����
	init();
	adjust();
}

bool BlueprintSequence::validate(QStringList& msg){
	BlueprintModel::validate(msg);

	//////////////////////////////////////////////////////////////////////////
	// seqģ�鲻��ֱ������
	if (hasDownStream(KISMET_FLOW_SEQUENCE)){
		isError_ = true;
		msg.append(ERROR_MSG.arg(UDQ_TR("����ģ�Ͳ���������ģ��ֱ������!")));
	}

	return !isError_;
}