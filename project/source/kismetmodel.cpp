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

#include "kismetmodel.h"
#include <QtGui>
#include <QJsonDocument>
#include <math.h>
#include "kismetdefine.h"
#include "kismetscene.h"
#include "kismetview.h"
#include "tooldefine.h"

const int DEFAULT_BUTTON_HEIGHT = 20;
const int DEFAULT_BUTTON_WIDTH = 40;
const int DEFAULT_BUTTON_MAXWIDTH = 120;

//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////

BlueprintButtonControl::BlueprintButtonControl(int type, const QString& text, const QString& datatype, const QIcon& ico, const QString& prefix,  QGraphicsItem * parent)
	:QGraphicsProxyWidget(parent, Qt::Widget), type_(type), datatype_(datatype), prefix_(prefix){

	prepareWidget(text, ico);

	// ���ñ�־
	setFlag(QGraphicsItem::ItemIsMovable, false);
	setFlag(QGraphicsItem::ItemIsSelectable, true);

	// ������꾭���¼�
	setAcceptHoverEvents(true);

	// �������Ϸ�
	setAcceptDrops(false);
	setVisible(true);

}

BlueprintButtonControl::~BlueprintButtonControl(){
}


void BlueprintButtonControl::prepareWidget(const QString& text, const QIcon& ico){

	button_ = new QPushButton();
	button_->setIconSize(QSize(16, 16));

	// ����ؼ�����
	QSize s = computeText(text, button_->font());
	int width = qMin(qMax(s.width() + 16, DEFAULT_BUTTON_WIDTH), DEFAULT_BUTTON_MAXWIDTH);

	button_->setFixedSize(QSize(width, DEFAULT_BUTTON_HEIGHT));
	button_->setText(text);
	button_->setIcon(ico);
	button_->setEnabled(true);
	button_->setFlat(true);

	if (type_ == BTN_ADD_IN){
		if (datatype_ == UDQ_T("ctrl")){
			connect(button_, SIGNAL(pressed()), this, SLOT(addInCtrlPort()));
		}
		else{
			connect(button_, SIGNAL(pressed()), this, SLOT(addInDataPort()));
		}
	}
	else if (type_ == BTN_ADD_OUT){
		if (datatype_ == UDQ_T("ctrl")){
			connect(button_, SIGNAL(pressed()), this, SLOT(addOutCtrlPort()));
		}
		else{
			connect(button_, SIGNAL(pressed()), this, SLOT(addOutDataPort()));
		}
	}

	setWidget(button_);
}

void BlueprintButtonControl::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
	setCursor(Qt::ArrowCursor);
	QGraphicsProxyWidget::hoverEnterEvent(event);
	scene()->setActiveWindow(this);
}

void BlueprintButtonControl::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
	QGraphicsProxyWidget::hoverLeaveEvent(event);
}

void BlueprintButtonControl::mousePressEvent(QGraphicsSceneMouseEvent * event){
	if (parentItem()){
		BlueprintModel* model = (BlueprintModel*)parentItem();
		model->backupState();
	}
	QGraphicsProxyWidget::mousePressEvent(event);
}

void BlueprintButtonControl::addInCtrlPort(){
	if (parentItem()){
		BlueprintModel* model = (BlueprintModel*)parentItem();
		BlueprintModelPort* port = model->addCtrlInPort(model->newPortName(BlueprintModelPort::EVT_CTRL_IN, prefix_), BlueprintObject::OF_CAN_DELETE);
		model->adjust();
		model->update();
		((KismetScene*)scene())->notifyObjectInsert(port);
	}
}

void BlueprintButtonControl::addOutCtrlPort(){
	if (parentItem()){
		BlueprintModel* model = (BlueprintModel*)parentItem();
		BlueprintModelPort* port = model->addCtrlOutPort(model->newPortName(BlueprintModelPort::EVT_CTRL_OUT, prefix_), BlueprintObject::OF_CAN_DELETE);
		model->adjust();
		model->update();
		((KismetScene*)scene())->notifyObjectInsert(port);
	}
}

void BlueprintButtonControl::addInDataPort(){
	if (parentItem()){
		BlueprintModel* model = (BlueprintModel*)parentItem();
		BlueprintModelPort* port = model->addDataInPort(model->newPortName(BlueprintModelPort::EVT_DATA_IN, prefix_), datatype_, BlueprintObject::OF_CAN_DELETE);
		model->adjust();
		model->update();
		((KismetScene*)scene())->notifyObjectInsert(port);
	}
}

void BlueprintButtonControl::addOutDataPort(){
	if (parentItem()){
		BlueprintModel* model = (BlueprintModel*)parentItem();
		BlueprintModelPort* port = model->addDataOutPort(model->newPortName(BlueprintModelPort::EVT_DATA_OUT, prefix_), datatype_, BlueprintObject::OF_CAN_DELETE);
		model->adjust();
		model->update();
		((KismetScene*)scene())->notifyObjectInsert(port);
	}
}
//////////////////////////////////////////////////////////////////////////
//////////////////////////////////////////////////////////////////////////
const int PORT_SPACE = 24;
const int TOOL_MINW = 100;
const int TOOL_MINH = 50;
const int TOOL_TITLEHEIGHT = 24;
const int TOOL_IMAGESIZE = 64;
const QString NAME_COMMON = UDQ_TR("Common");

BlueprintModel::BlueprintModel(QGraphicsItem * parent) : BlueprintObject(KISMET_DEPTH_MODEL, parent), isLocking_(false), plugin_(NULL), type_(0), interfaceID_(0){
	color_ = Qt::black;
	//  �޸ı߽�
	boundingBox_.setRect(0, 0, TOOL_MINW, TOOL_MINH);

	// ���ñ�־
	setFlag(QGraphicsItem::ItemIsMovable, !isLocking_);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

	// ������꾭���¼�
	setAcceptHoverEvents(true);

	// �������Ϸ�
	setAcceptDrops(false);
	setVisible(true);

	bomPos_ = 0;
	objPort_ = NULL;
	behPort_ = NULL;
	interface_ = NULL;
}

BlueprintModel::BlueprintModel(ResourceHash pluginHash, int type, ResourceHash inter, QGraphicsItem * parent) :
BlueprintObject(KISMET_DEPTH_MODEL, parent), isLocking_(false), interfaceID_(inter), type_(type){

	color_ = Qt::black;
	//  �޸ı߽�
	boundingBox_.setRect( 0, 0, TOOL_MINW, TOOL_MINH );

	// ���ñ�־
	setFlag(QGraphicsItem::ItemIsMovable, !isLocking_);
	setFlag(QGraphicsItem::ItemIsSelectable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);

	// ������꾭���¼�
	setAcceptHoverEvents( true );

	// �������Ϸ�
	setAcceptDrops( false );
	setVisible( true );

	bomPos_ = 0;
	objPort_ = NULL;
	behPort_ = NULL;

	plugin_ = (PluginResourceObject*)ResourceObject::findResource(pluginHash);
	init();
	adjust();
}

BlueprintModel::~BlueprintModel(){

}

PluginInterface* BlueprintModel::getInterface(){
	PluginInterface* inter;
	if (type_ == PT_ACTION){
		inter = plugin_->getAction(interfaceID_);
	}
	else if (type_ == PT_CONDITION){
		inter = plugin_->getCondition(interfaceID_);
	}
	else if (type_ == PT_EXPRESS){
		inter = plugin_->getExpress(interfaceID_);
	}
	return inter;
}

void BlueprintModel::init(){
	removeAllPort();

	interface_ = getInterface();
	//Q_ASSERT(inter != NULL);

	if (interface_){
		// ������hash
		title_ = interface_->name;
	}

	// ������ɫ
	if (type_ == PT_CONDITION){
		color_ = CONDITION_COLOR;
		title_.insert(0, UDQ_TR("��������: "));
	}
	else if (type_ == PT_ACTION){
		color_ = ACTION_COLOR;
		title_.insert(0, UDQ_TR("����: "));
	}
	else if (type_ == PT_EXPRESS){
		color_ = EXPRESS_COLOR;
		title_.insert(0, UDQ_TR("���ʽ: "));
	}
	else{
		color_ = Qt::black;
	}

	if (type_ != PT_EXPRESS && interface_){
		// ׼���������˿�
		prepareCtrlPort(interface_);
	}

	if (interface_){
		// ׼��ȱʡ�˿�
		prepareDefPort(interface_);

		// ��������˿�
		for (int i = 0; i < interface_->params.size(); i++){
			prepareInPort(interface_->params.at(i));
		}

		// ��ť
		prepareButtonPort(interface_);
	}

}

void BlueprintModel::prepareCtrlPort(PluginInterface* inter){
	BlueprintModelPort* modelport = NULL;

	if (type_ == PT_ACTION ){  // ֻ�ж����п�������
		modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_IN, UDQ_T("ctrl"), UDQ_T("in"));
		inCtrlPorts_.append(modelport);
		modelport->setParentItem(this);
		iidPorts_.insert(modelport->iid(), modelport);
	}

	if (type_ == PT_CONDITION){
		modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_OUT, UDQ_T("ctrl"), UDQ_T("true"));
		outCtrlPorts_.append(modelport);
		modelport->setParentItem(this);
		iidPorts_.insert(modelport->iid(), modelport);

		if ( !(inter->hasFlag(FT_INVERTIBLE) || inter->hasFlag(FT_FAST_TRIGGER) || inter->hasFlag(FT_TRIGGER) || inter->hasFlag(FT_FAKE_TRIGGER))){
			modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_OUT, UDQ_T("ctrl"), UDQ_T("false"));
			outCtrlPorts_.append(modelport);
			modelport->setParentItem(this);
			iidPorts_.insert(modelport->iid(), modelport);
		}

	}
	else{
		modelport = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_OUT, UDQ_T("ctrl"), UDQ_T("out"));
		outCtrlPorts_.append(modelport);
		modelport->setParentItem(this);
		iidPorts_.insert(modelport->iid(), modelport);
	}

}

void BlueprintModel::prepareDefPort(PluginInterface* inter){
	BlueprintModelPort* modelport = NULL;
	// ����
	objPort_ = NULL;
	if (plugin_->type_ != PIT_NATIVE && !plugin_->hasAttribute(EF_SINGLEGLOBAL)){  // SYSTEM����û�ж�������,SingleGlobal����Ҳû��
		// ������������
		QString portName = plugin_->type_ == PIT_ENTITY ? plugin_->objectName() : UDQ_T("Object");
		objPort_ = new BlueprintModelPort(BlueprintModelPort::EVT_DATA_IN, UDQ_T("object"), portName);
		inPorts_.append(objPort_);
		objPort_->setParentItem(this);
		iidPorts_.insert(objPort_->iid(), objPort_);
	}

	behPort_ = NULL;
	if (plugin_->type_ == PIT_BEHAVIOR ){  // �������Ϊ��Ϊ��Ϊ��
		// ��Ϊ��
		behPort_ = new BlueprintModelPort(BlueprintModelPort::EVT_DATA_IN, UDQ_T("string"), UDQ_T("Behavior"));
		inPorts_.append(behPort_);
		behPort_->setParentItem(this);
		iidPorts_.insert(behPort_->iid(), behPort_);
	}

	if (type_ == PT_EXPRESS){ // ֻ�б��ʽ���������
		// ���
		QString datatype;
		if (inter->hasFlag(FT_RETURN_INT)){
			datatype = UDQ_T("int");
		}
		else	if (inter->hasFlag(FT_RETURN_FLOAT)){
			datatype = UDQ_T("float");
		}
		else	if (inter->hasFlag(FT_RETURN_ANY)){
			datatype = UDQ_T("any");
		}
		else{  // Ĭ��Ϊ�ַ���
			datatype = UDQ_T("string");
		}

		modelport = new BlueprintModelPort(BlueprintModelPort::EVT_DATA_OUT, datatype, inter->func);
		outPorts_.append(modelport);
		modelport->setParentItem(this);
		iidPorts_.insert(modelport->iid(), modelport);
	}

}

void BlueprintModel::prepareInPort(PluginVariable* var){

	BlueprintModelPort* modelport = new BlueprintModelPort(BlueprintModelPort::EVT_DATA_IN, var->datatype, var->name, var->enums );
	inPorts_.append(modelport);
	modelport->setParentItem(this);
	iidPorts_.insert(modelport->iid(), modelport);
}

void BlueprintModel::prepareButtonPort(PluginInterface* inter){
	BlueprintButtonControl* btn = NULL;

	if (inter->flags.contains(UDQ_T("inctrl_variadic"))){  // ������ƿɱ�
		btn = new BlueprintButtonControl(BlueprintButtonControl::BTN_ADD_IN, UDQ_TR("in"), UDQ_T("ctrl"), QIcon(UDQ_T(":/images/add.png")));
		inButtons_.append(btn);
		btn->setParentItem(this);
	}

	if (inter->flags.contains(UDQ_T("outctrl_variadic"))){  // ����ɱ�
		btn = new BlueprintButtonControl(BlueprintButtonControl::BTN_ADD_OUT, UDQ_TR("out"), UDQ_T("ctrl"), QIcon(UDQ_T(":/images/add.png")));
		outButtons_.append(btn);
		btn->setParentItem(this);
	}

	if (inter->flags.contains(UDQ_T("int_variadic"))){  //  ����ɱ�
		btn = new BlueprintButtonControl(BlueprintButtonControl::BTN_ADD_IN, UDQ_TR("data"), UDQ_T("int"), QIcon(UDQ_T(":/images/add.png")));
		inButtons_.append(btn);
		btn->setParentItem(this);
	}
	else if (inter->flags.contains(UDQ_T("float_variadic"))){  //  ����ɱ�
		btn = new BlueprintButtonControl(BlueprintButtonControl::BTN_ADD_IN, UDQ_TR("data"), UDQ_T("float"), QIcon(UDQ_T(":/images/add.png")));
		inButtons_.append(btn);
		btn->setParentItem(this);
	}
	else if (inter->flags.contains(UDQ_T("string_variadic"))){  //  ����ɱ�
		btn = new BlueprintButtonControl(BlueprintButtonControl::BTN_ADD_IN, UDQ_TR("data"), UDQ_T("int"), QIcon(UDQ_T(":/images/add.png")));
		inButtons_.append(btn);
		btn->setParentItem(this);
	}
	else if (inter->flags.contains(UDQ_T("any_variadic"))){  //  ����ɱ�
		btn = new BlueprintButtonControl(BlueprintButtonControl::BTN_ADD_IN, UDQ_TR("data"), UDQ_T("any"), QIcon(UDQ_T(":/images/add.png")));
		inButtons_.append(btn);
		btn->setParentItem(this);
	}
	else if (inter->flags.contains(UDQ_T("object_variadic"))){  //  ����ɱ�
		btn = new BlueprintButtonControl(BlueprintButtonControl::BTN_ADD_IN, UDQ_TR("data"), UDQ_T("object"), QIcon(UDQ_T(":/images/add.png")));
		inButtons_.append(btn);
		btn->setParentItem(this);
	}
}

// ������ж˿�
void BlueprintModel::removeAllPort(){
	clearPortLink();

	foreach( BlueprintModelPort* port, iidPorts_ ){
		delete port;
	}

	foreach(BlueprintButtonControl* btn, inButtons_){
		delete btn;
	}

	foreach(BlueprintButtonControl* btn, outButtons_){
		delete btn;
	}

	inCtrlPorts_.clear();
	outCtrlPorts_.clear();
	inPorts_.clear();
	outPorts_.clear();
	iidPorts_.clear();
}

void BlueprintModel::updatePort(){
	adjust();
}

//  ��ȡ������
QString BlueprintModel::title(){
	return QString();
}

// ���ͱ�ʾ( ���ģ��û��������)
QString BlueprintModel::typeName(){
	return title_;
}

QColor BlueprintModel::tilteColor(){
	return color_;
}

//  ��ȡ����ͼ��64,64��
QImage BlueprintModel::thumbnail(){
	QImage image(64,64,QImage::Format_ARGB32);
	image.fill(0);

	return image;
}

BlueprintModelPort* BlueprintModel::inport(const QString& text){
	BlueprintModelPort* found = NULL;

	foreach(BlueprintModelPort* port, inPorts_){
		if( port->name() == text ){
			found = port;
			break;
		}
	}

	return found;
}

BlueprintModelPort* BlueprintModel::outport(const QString& text){
	BlueprintModelPort* found = NULL;

	foreach(BlueprintModelPort* port, outPorts_){
		if( port->name() == text ){
			found = port;
			break;
		}
	}

	return found;
}


BlueprintModelPort* BlueprintModel::inCtrlPort(const QString& text){
	BlueprintModelPort* found = NULL;
	foreach(BlueprintModelPort* port, inCtrlPorts_){
		if (port->name() == text){
			found = port;
			break;
		}
	}
	return found;
}

BlueprintModelPort* BlueprintModel::outCtrlPort(const QString& text){
	BlueprintModelPort* found = NULL;
	foreach(BlueprintModelPort* port, outCtrlPorts_){
		if (port->name() == text){
			found = port;
			break;
		}
	}
	return found;
}

QVariant BlueprintModel::itemChange(GraphicsItemChange change, const QVariant & value){

	if( change == ItemPositionHasChanged && scene() ){
		// ֪ͨ���ж˿ڸ�������
		foreach(BlueprintModelPort *port, iidPorts_.values()){
			port->updateLink();
		}
	}
	else if( change == ItemSelectedHasChanged && scene() ){

		// ֪ͨ���ж˿ڸ�������
		foreach(BlueprintModelPort *port, iidPorts_.values()){
			port->setUnderMouse( value.toBool() );
		}
	}
	return BlueprintObject::itemChange( change, value );
}


void BlueprintModel::drawLock(QPainter* painter){

	if( lockImage_.isNull() ){
		lockImage_ = QImage( UDQ_T(":/images/locky.png") ).scaledToWidth( 16 );
	}
	//
	QRectF bodyRect( 4, 2, 16, 16 );
	painter->setRenderHint( QPainter::SmoothPixmapTransform, true );
	painter->drawImage( bodyRect,lockImage_);
}


void BlueprintModel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	Q_UNUSED(widget);
	//
	QPen borderPen;
	if( isSelected()){
		// ����ѡ�����ɫ
		borderPen.setColor( SELECT_COLOR );
	}
	else{
		borderPen.setColor( Qt::black  );
	}
	painter->setPen( borderPen );

	// ���ƶ�ʵ�������
	QRectF headRect;
	headRect = QRectF ( 0, 0, boundingBox_.width(), TOOL_TITLEHEIGHT - 2 );

	// ���Ʊ����
	painter->setOpacity(0.75);
	painter->fillRect( headRect, tilteColor()  );
	painter->setOpacity(1.0);
	painter->drawRect(  headRect );

	// ��������
	QRectF bodyRect( 0, TOOL_TITLEHEIGHT, boundingBox_.width(), boundingBox_.height() - TOOL_TITLEHEIGHT );
	painter->fillRect( bodyRect, Qt::gray  );
	painter->drawRect(  bodyRect );

	// ������ű�����С���򲻻�������
	// ����ע��	( ӳ��ʵ���� )
	bool simpleDraw = canSimpleDraw();
	if (!simpleDraw){
		painter->drawText(0, -4, title());

		if (isLocking_){
			drawLock(painter);
		}

		//  ��������ͼ
		drawPreview(painter);

		// �����ı�
		painter->setPen(Qt::yellow);
		painter->setFont(nameFont());
		QSize typeSize = computeText(typeName(), painter->font());
		painter->drawText(headRect.left() + (headRect.width() - typeSize.width()) / 2,
			headRect.bottom() - (headRect.height() - painter->font().pixelSize()) / 2, typeName());
	}

	// ���ƴ�����ʾ
	if (isError_){
		// ��������
		QRectF errorRect(0, TOOL_TITLEHEIGHT, boundingBox_.width(), TOOL_TITLEHEIGHT);
		painter->fillRect(errorRect.adjusted(2, 2, -2, -2), Qt::red);

		if (!simpleDraw){
			// �����ı�
			painter->setPen(Qt::yellow);
			painter->setFont(nameFont());
			painter->drawText(errorRect, Qt::AlignCenter, UDQ_TR("Error"));
		}
	}

	// ����װ��
	if (interface_){
		if (interface_->hasFlag(FT_FAKE_TRIGGER)){

		}
	}
}

void BlueprintModel::drawPreview(QPainter* painter){

	if (plugin_ == NULL || plugin_->icon_.isNull())
		return;

	QRectF bodyRect(0, 0, TOOL_TITLEHEIGHT, TOOL_TITLEHEIGHT);
	painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
	painter->drawImage(bodyRect.adjusted(2,2,-2,-2), plugin_->icon_);

}


// ������С
void BlueprintModel::adjust(){
	// 

	QSize nameSize = computeText(title_, nameFont());

	// ������
	qreal intextHeight = 0;
	qreal intextWidth = 0;

	foreach(BlueprintModelPort *port, inCtrlPorts_){
		QSizeF size = port->ctrlBoundBox().size();
		if (size.width() > intextWidth)
			intextWidth = size.width();
	}

	foreach (BlueprintModelPort *port, inPorts_){
		QSizeF size = port->ctrlBoundBox().size();
		if( size.width() > intextWidth )
			intextWidth = size.width();
	}

	foreach(BlueprintButtonControl *btn, inButtons_){
		QSizeF size = btn->boundingRect().size();
		if (size.width() > intextWidth)
			intextWidth = size.width();
	}

	intextHeight = ( inPorts_.size() + inCtrlPorts_.size() + inButtons_.size() ) * PORT_SPACE;

	qreal outextHeight = 0;
	qreal outtextWidth = 0;

	foreach(BlueprintModelPort *port, outCtrlPorts_){
		QSizeF size = port->ctrlBoundBox().size();
		if (size.width() > outtextWidth)
			outtextWidth = size.width();
	}

	foreach(BlueprintModelPort *port, outPorts_){
		QSizeF size = port->ctrlBoundBox().size();
		if( size.width() > outtextWidth )
			outtextWidth = size.width();
	}

	foreach(BlueprintButtonControl *btn, outButtons_){
		QSizeF size = btn->boundingRect().size();
		if (size.width() > outtextWidth)
			outtextWidth = size.width();
	}
	outextHeight = ( outPorts_.size() + outCtrlPorts_.size() + outButtons_.size() ) * PORT_SPACE;

	int portHeight = TOOL_TITLEHEIGHT * 3+ qMax(intextHeight, outextHeight);
	int maxWidth = qMax((qreal)nameSize.width() + TOOL_TITLEHEIGHT * 2, intextWidth + outtextWidth + TOOL_IMAGESIZE) + PORT_SPACE * 2;
	
	prepareGeometryChange();
	boundingBox_.setRect(0, 0, maxWidth, portHeight);
	
	// �޸Ķ˿�λ��
	int index = 0;
	foreach(BlueprintModelPort *port, inCtrlPorts_){
		port->setPos(logicInPos(index++));
		// ��������
		port->updateLink();
	}

	foreach(BlueprintModelPort *port, inPorts_){
		port->setPos( logicInPos(index++) );
		// ��������
		port->updateLink();
	}

	foreach(BlueprintButtonControl *btn, inButtons_){
		btn->setPos(logicInPos(index++));
	}

	index = 0;
	foreach(BlueprintModelPort *port, outCtrlPorts_){
		QPointF pos = logicOutPos(index++);
		pos.setX(pos.x() - port->boundingRect().width());
		port->setPos(pos);
		// ��������
		port->updateLink();
	}

	foreach(BlueprintModelPort *port, outPorts_){
		QPointF pos = logicOutPos(index++);
		pos.setX(pos.x() - port->boundingRect().width());
		port->setPos(pos);
		// ��������
		port->updateLink();
	}

	foreach(BlueprintButtonControl *btn, outButtons_){
		QPointF pos = logicOutPos(index++);
		pos.setX(pos.x() - btn->boundingRect().width());
		btn->setPos(pos);
	}

}

const int PORT_MARGIN = 4;
QPointF BlueprintModel::logicInPos(int index){
	if( index < 0 )
		index = inPorts_.size();
	return QPointF(PORT_MARGIN, TOOL_TITLEHEIGHT * 2 + index*PORT_SPACE );
}

QPointF BlueprintModel::logicOutPos(int index){
	if( index < 0 )
		index = outPorts_.size();
	return QPointF(boundingBox_.width() - PORT_MARGIN, TOOL_TITLEHEIGHT * 2 + index*PORT_SPACE);
}

void BlueprintModel::changePortIID(BlueprintModelPort* port, ResourceHash iid){
	iidPorts_.remove(port->iid());
	port->setIID(iid);
	iidPorts_.insert(port->iid(), port);
}

BlueprintModelPort* BlueprintModel::addCtrlInPort(const QString& port, int flag){
	// ����λ��
	BlueprintModelPort* pt = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_IN,  UDQ_T("ctrl"), port);
	pt->setObjectFlags(flag);
	pt->setParentItem(this);
	inCtrlPorts_.append(pt);
	iidPorts_.insert(pt->iid(), pt);
	return pt;
}

BlueprintModelPort* BlueprintModel::addCtrlOutPort(const QString& port, int flag){
	// ����λ��
	BlueprintModelPort* pt = new BlueprintModelPort(BlueprintModelPort::EVT_CTRL_OUT, UDQ_T("ctrl"), port);
	pt->setObjectFlags(flag);
	pt->setParentItem(this);
	outCtrlPorts_.append( pt );
	iidPorts_.insert(pt->iid(), pt);
	return pt;
}


BlueprintModelPort* BlueprintModel::addDataInPort(const QString& port, const QString& datatype, int flag, const QStringList& items){
	// ����λ��
	BlueprintModelPort* pt = new BlueprintModelPort(BlueprintModelPort::EVT_DATA_IN, datatype, port, items);
	pt->setObjectFlags(flag);
	pt->setParentItem(this);
	inPorts_.append(pt);
	iidPorts_.insert(pt->iid(), pt);
	return pt;
}

BlueprintModelPort* BlueprintModel::addDataOutPort(const QString& port, const QString& datatype, int flag, const QStringList& items){
	// ����λ��
	BlueprintModelPort* pt = new BlueprintModelPort(BlueprintModelPort::EVT_DATA_OUT, datatype, port, items);
	pt->setObjectFlags(flag);
	pt->setParentItem(this);
	outPorts_.append(pt);
	iidPorts_.insert(pt->iid(), pt);
	return pt;
}




void BlueprintModel::toggleLock(){
	isLocking_ = !isLocking_;
	setFlag(QGraphicsItem::ItemIsMovable, !isLocking_);
}


void BlueprintModel::deletePort(BlueprintModelPort* port, QList<BlueprintModelPort*>& container){
	port->clearLink();
	iidPorts_.remove(port->iid());
	container.removeAll(port);
	delete port;
}

void BlueprintModel::removePort(BlueprintModelPort* port){
	if (port == NULL || !(port->canDelete())){
		return;
	}

	if (port->orient() == BlueprintModelPort::EVT_CTRL_IN){
		deletePort(port, inCtrlPorts_);
	}
	else if (port->orient() == BlueprintModelPort::EVT_CTRL_OUT){
		deletePort(port, outCtrlPorts_);
	}
	else if (port->orient() == BlueprintModelPort::EVT_DATA_IN){
		deletePort(port, inPorts_);
	}
	else if (port->orient() == BlueprintModelPort::EVT_DATA_OUT){
		deletePort(port, outPorts_);
	}
	else{
		Q_UNREACHABLE();
	}
	// ���µ�����С
	adjust();

}


// ������ж˿�����
void BlueprintModel::clearPortLink(){

	foreach(BlueprintModelPort* port, iidPorts_){
		port->clearLink();
	}

}

// �˿������б�
QStringList BlueprintModel::portNames(){
	QStringList names;
	names<<inportNames()<<outportNames();
	return names;
}

QStringList BlueprintModel::inportNames(){
	QStringList names;

	foreach(BlueprintModelPort* port, inPorts_){
		names.append( port->name() );
	}

	return names;
}

QStringList BlueprintModel::outportNames(){
	QStringList names;

	foreach(BlueprintModelPort* port, outPorts_){
		names.append( port->name() );
	}

	return names;
}

void BlueprintModel::mousePressEvent(QGraphicsSceneMouseEvent * event){
	// ����״̬
	if( isUnderMouse() ){
		KismetScene* parent = (KismetScene*)scene();
		// ���ͬʱ����ALt
		if( event->modifiers() & Qt::AltModifier ){
			// ������ж˿�����
			clearPortLink();
			return;
		}
	}

	BlueprintObject::mousePressEvent( event );

}

// ����
void BlueprintModel::encodeData(QDataStream& stream){
	BlueprintObject::encodeData(stream);

	if (plugin_){
		stream << plugin_->hashKey();
	}
	else{
		stream << (ResourceHash)0;
	}

	stream << type_;
	stream << interfaceID_;
	stream << isLocking_;

	// ����˿�ֵ
	QMap<QString, QString> values;
	QString v;
	foreach(BlueprintModelPort* port, inPorts_){
		v = port->getValue();
		if (!v.isEmpty()){
			values.insert(port->name(), v);
		}
	}
	stream << values;

}

void BlueprintModel::decodeData(QDataStream& stream){
	BlueprintObject::decodeData(stream);

	ResourceHash key;
	stream >> key;
	plugin_ = (PluginResourceObject*)ResourceObject::findResource(key);

	stream >> type_;
	stream >> interfaceID_;
	stream >> isLocking_;


	init();

	// ��ȡ�˿�ֵ
	QMap<QString, QString> values;
	stream >> values;
	foreach(BlueprintModelPort* port, inPorts_)
	{
		if (values.contains(port->name())){
			port->setValue(values[port->name()]);
		}
	}


	adjust();

}

BlueprintModelPort* BlueprintModel::findPort(ResourceHash iid){
	if (iidPorts_.contains(iid)){
		return iidPorts_[iid];
	}
	return NULL;
}

BlueprintModelPort* BlueprintModel::findPort(const QString& name, int orient){

	if (orient == BlueprintModelPort::EVT_DATA_IN){
		return inport(name);
	}
	else if (orient == BlueprintModelPort::EVT_DATA_OUT){
		return outport(name);
	}
	else if (orient == BlueprintModelPort::EVT_CTRL_IN){
		return inCtrlPort(name);
	}
	else if (orient == BlueprintModelPort::EVT_CTRL_OUT){
		return outCtrlPort(name);
	}

	return NULL;
}

QString BlueprintModel::newPortName(int type, const QString& prefix ){
	QString baseName = prefix;
	if (baseName.isEmpty()){
		if (type == BlueprintModelPort::EVT_DATA_IN){
			baseName = UDQ_T("data");
		}
		else if (type == BlueprintModelPort::EVT_DATA_OUT){
			baseName = UDQ_T("res");
		}
		else if (type == BlueprintModelPort::EVT_CTRL_IN){
			baseName = UDQ_T("in");
		}
		else if (type == BlueprintModelPort::EVT_CTRL_OUT){
			baseName = UDQ_T("out");
		}
	}

	bool found = false;
	QString newName;

	int index = 1;
	while (true){	
		newName = baseName + QString::number(index++);
		found = false;
		// ��ֹ�ظ�
		foreach(BlueprintModelPort* port, iidPorts_){
			if (port->name() == newName){
				found = true;
				break;
			}
		}
		if (!found){
			break;
		}
	}

	return newName;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

bool BlueprintModel::hasDownStream(int type, int subtype){
	return hasLinkModel(outCtrlPorts_, type, subtype);
}

bool BlueprintModel::hasUpStream(int type, int subtype){
	return hasLinkModel(inCtrlPorts_, type, subtype);
}

bool BlueprintModel::hasLinkModel(QList<BlueprintModelPort*> ports, int type, int subtype){
	if (ports.size() == 0)
		return false;

	bool found = false;
	BlueprintObject* upport = NULL;
	BlueprintModel* upmodel = NULL;

	// ������е�����ģ�ͣ����û����Ϊ����
	foreach(BlueprintModelPort* port, ports){
		// �˿ڵ���������
		foreach(BlueprintLink* link, port->links()){
			// ����������ӵ�ģ�Ͷ���
			upport = link->sourceNode() == port ? link->destNode() : link->sourceNode();
			// �������ģ��
			upmodel = dynamic_cast<BlueprintModel*>(upport->parentItem());
			Q_ASSERT(upmodel != NULL);

			if (upmodel->type() == type){
				if (type == KISMET_MODEL){
					found = upmodel->modelType() == subtype;
				}
				else{
					found = true;
				}
			}

			if (found)
				break;
		}

		if (found)
			break;
	}

	return found;
}

QJsonValue BlueprintModel::objTypeNode(){
	QJsonValue v;
	// �����Ͷ���
	if (objPort_ == NULL){

		// �ж��Ƿ�Ϊsingleglobal����
		if (plugin_->hasAttribute(EF_SINGLEGLOBAL)){
			// �ҳ���Ӧ���͵�Ψһ��������
			v = QString::number(plugin_->hashKey());
		}
		else{
			v = -1;
		}
	}
	else{
		// ��������
		// ȷ������˿�������
		Q_ASSERT(objPort_->hasLink());

		QList<BlueprintLink *> links = objPort_->links();
		Q_ASSERT(links.size() == 1);

		BlueprintObject* obj = links[0]->sourceNode() == objPort_ ? links[0]->destNode() : links[0]->sourceNode();
		Q_ASSERT(obj->type() == KISMET_INSTANCE || obj->type() == KISMET_COLLECTION);

		if (obj->type() == KISMET_INSTANCE){
			BlueprintInstance* inst = (BlueprintInstance*)obj;
			v =QString::number(inst->prototype_->hashKey());  // ����ʱ��Ҫ�滻
		}
		else{
			BlueprintCollection* coll = (BlueprintCollection*)obj;
			Q_ASSERT(coll->prototype_->members_.size() > 0);  // Family������Ч
			v = QString::number(coll->prototype_->hashKey());  // ����ʱ��Ҫ�滻
		}
	}

	return v;
}


QJsonArray BlueprintModel::portExpNode(BlueprintModelPort* port){
	static const QString NAME_SYSTEM = UDQ_TR("System");

	QJsonArray exp;
	if ( port == NULL){

		// ���ģ�͵ı��ʽ�������Ͷ���ģ��û��������ʽ
		// Ŀǰֻ����ѧ����ͱ��ʽģ���ܹ���������ݶ˿�( ����ֻ����һ�����)
		if (type() == KISMET_ARITHMETIC || type() == KISMET_INSTANCE_VARIABLE|| modelType() == PT_EXPRESS ){
			Q_ASSERT(outPorts_.size() > 0);
			// ģ�ͱ��ʽ���
			if (type() == KISMET_ARITHMETIC || type() == KISMET_INSTANCE_VARIABLE ){
				exp = expNode().toArray();
			}
			else{
				// ���ʽ
				Q_ASSERT(modelType() == PT_EXPRESS);

				if (objPort_ == NULL){
					// ���Ϊϵͳ���
					if (plugin_->hasAttribute(EF_SINGLEGLOBAL)){
						Q_ASSERT(plugin_->type_ == PIT_ENTITY);
						exp.append((int)20);

						// д����ID������ʱ�滻
						exp.append(QString::number(plugin_->hashKey()));

						// ��������
						PluginInterface* inter = plugin_->getExpress(interfaceID_);
						exp.append(QString::number(hashString(plugin_->runtimeString(inter))));

						exp.append(inter->hasFlag(FT_RETURN_STRING));  // �Ƿ񷵻��ַ���
						exp.append(QJsonValue());  // ��ʵ��������

					}
					else{
						// ϵͳ���ʽ
						exp.append((int)19);
						// ����
						PluginInterface* inter = plugin_->getExpress(interfaceID_);
						exp.append(QString::number(hashString(plugin_->runtimeString(inter))));
					}
				}
				else{  // ʵ����ʽ
					// ���������
					if (plugin_->type_ == PIT_ENTITY || plugin_->type_ == PIT_BEHAVIOR){

						if (plugin_->type_ == PIT_ENTITY){
							exp.append((int)20);
						}
						else{
							exp.append((int)22);
						}

						// ��������ģ��
						Q_ASSERT(objPort_->hasLink());
						QList<BlueprintLink *> links = objPort_->links();
						Q_ASSERT(links.size() == 1);
						// ֻȡ��һ������
						BlueprintObject* upport = links[0]->sourceNode() == objPort_ ? links[0]->destNode() : links[0]->sourceNode();

						if (upport->type() == KISMET_INSTANCE){
							// ������������
							BlueprintInstance* inst = (BlueprintInstance*)upport;
							exp.append(QString::number(inst->prototype_->hashKey()));

							if (plugin_->type_ == PIT_BEHAVIOR){ // ��Ϊ��
								if (behPort_){
									exp.append(behPort_->getValue());
								}
							}

							// ��������
							PluginInterface* inter = plugin_->getExpress(interfaceID_);
							exp.append(QString::number(hashString(plugin_->runtimeString(inter))));

							exp.append(inter->hasFlag(FT_RETURN_STRING));  // �Ƿ񷵻��ַ���
							exp.append(QJsonValue());  // ��ʵ��������
						}
						else{ // ������ʵ��������todo
							BlueprintModel* upmodel = (BlueprintModel*)upport->parentItem();
							if (upmodel){
								// ʵ��������
							}
						}
					}
					else{
						Q_UNREACHABLE();
					}

				}

				// ����
				QJsonArray params;
				foreach(BlueprintModelPort* port, inPorts_){
					if ( isInternalPort(port) ){  // ����Ƕ���˿�, ����
						continue;
					}
					params.append(portExpNode(port));
				}
				if (params.size() > 0){
					exp.append(params);
				}

			}
			// ���Ƕ���ʵ��������
		}
		else if (type() == KISMET_MODEL &&  modelType() == PT_ACTION ){

			// ��������
			exp.append(objTypeNode());
	
			// API, ����ʱ��Ҫ�滻
			PluginInterface* inter = plugin_->getAction(interfaceID_);
			exp.append(QString::number(hashString(plugin_->runtimeString(inter))));

			if (plugin_->type_ == PIT_BEHAVIOR && behPort_){ // ��Ϊ��
				exp.append(behPort_->getValue());
			}
			else{
				exp.append(QJsonValue());
			}

			exp.append((qint64)iid());	// sid
			exp.append(false);  // ����

			// ����
			QJsonArray params;
			foreach(BlueprintModelPort* port, inPorts_){
				if ( isInternalPort(port) ){  // ����Ƕ���˿�, ����
					continue;
				}
				params.append(portExpNode(port));
			}
			exp.append(params);
		}

	}
	else if (port->orient() == BlueprintModelPort::EVT_CTRL_OUT){

		if (type() == KISMET_MODEL &&  modelType() == PT_CONDITION ){
			// �����Ͷ���
			exp.append(objTypeNode());
	
			// API, ����ʱ��Ҫ�滻
			PluginInterface* inter = plugin_->getCondition(interfaceID_);
			exp.append(QString::number(hashString(plugin_->runtimeString(inter))));


			if (plugin_->type_ == PIT_BEHAVIOR && behPort_){ // ��Ϊ��
				exp.append(behPort_->getValue());
			}
			else{
				exp.append(QJsonValue());
			}

			// ����������
			if (inter->hasFlag(FT_FAST_TRIGGER)){
				exp.append((int)2);
			}
			else if (inter->hasFlag(FT_TRIGGER)){
				exp.append((int)1);
			}
			else{
				exp.append((int)0);
			}

			exp.append(inter->hasFlag(FT_LOOPING));  // ѭ��
			exp.append(port->name() == UDQ_T("false"));	 // ��
			exp.append(inter->hasFlag(FT_STATIC));		// ��̬

			// sid
			exp.append((qint64)iid());
			exp.append(false);  // ����

			// ����
			QJsonArray params;
			foreach(BlueprintModelPort* port, inPorts_){
				if (isInternalPort(port))
					continue;
				params.append(portExpNode(port));
			}
			exp.append(params);
		}
	}
	else if (port->orient() == BlueprintModelPort::EVT_DATA_IN){

		QJsonValue node;
		if (port->hasLink()){  // ����˿�����������
			QList<BlueprintLink *> links = port->links();
			// ֻȡ��һ������
			BlueprintObject* upport = links[0]->sourceNode() == port ? links[0]->destNode() : links[0]->sourceNode();
			// �������ģ��
			BlueprintModel* upmodel = dynamic_cast<BlueprintModel*>(upport->parentItem());
			if (upmodel){
				node = upmodel->portExpNode();  // ģ������ı��ʽ
			}
			else{
				node = upport->expNode(port->dataTypeId());
			}
		}
		else{   // ���ݶ˿���������exp
			node = port->expNode();
		}

		if (type() == KISMET_MODEL && modelType() != PT_EXPRESS){  // �����������������Ϊ�������ʽ
			// �����ڵ�
			exp.append(port->dataTypeId());
			exp.append(node);
		}
		else{
			exp = node.toArray();
		}

	}
	else{
		Q_UNREACHABLE();
	}

	return exp;
}

BlueprintModel* BlueprintModel::downStreamCtrl(const QString& name){
	BlueprintModelPort* port = outCtrlPort(name);
	if (port == NULL || !port->hasLink())
		return NULL;

	QList<BlueprintLink *> links = port->links();
	Q_ASSERT(links.size() == 1);

	BlueprintObject* upport = links[0]->sourceNode() == port ? links[0]->destNode() : links[0]->sourceNode();
	BlueprintModel* upmodel = dynamic_cast<BlueprintModel*>(upport->parentItem());
	Q_ASSERT(upmodel != NULL);

	return upmodel;
}

QList<BlueprintModel*> BlueprintModel::upstreamCtrl(){
	QList<BlueprintModel*> upmodels;

	foreach(BlueprintModelPort* port, inCtrlPorts_){
		QList<BlueprintLink *> links = port->links();
		if (links.size() > 0){
			for (int i = 0; i < links.size(); i++){
				BlueprintObject* upport = links[i]->sourceNode() == port ? links[i]->destNode() : links[i]->sourceNode();
				BlueprintModel* upmodel = dynamic_cast<BlueprintModel*>(upport->parentItem());
				Q_ASSERT(upmodel != NULL);
				upmodels.append(upmodel);
			}
		}
	}

	return upmodels;
}

QList<BlueprintModel*> BlueprintModel::upstreamCtrl(int type, int subtype ){
	QSet<BlueprintModel*> upmodels;
	foreach(BlueprintModel* m, upstreamCtrl()){
		if (m->type() == type){
			if (m->type() == KISMET_MODEL && m->modelType() != subtype){
				continue;
			}
			upmodels.insert(m);
		}
	}
	return upmodels.toList();
}

QList<BlueprintModelPort*> BlueprintModel::modelInPort(BlueprintModel* model){
	QList<BlueprintModelPort*> ports;

	if (model){
		foreach(BlueprintModelPort* port, inCtrlPorts_){
			QList<BlueprintLink *> links = port->links();
			for (int i = 0; i < links.size(); i++){
				BlueprintObject* upport = links[0]->sourceNode() == port ? links[0]->destNode() : links[0]->sourceNode();
				BlueprintModel* upmodel = dynamic_cast<BlueprintModel*>(upport->parentItem());
				Q_ASSERT(upmodel != NULL);

				if (upmodel == model){
					ports.append(port);
				}
			}
		}
	}

	return ports;
}

QList<BlueprintModelPort*> BlueprintModel::modelOutPort(BlueprintModel* model){
	QList<BlueprintModelPort*> ports;

	if (model){
		foreach(BlueprintModelPort* port, outCtrlPorts_){
			QList<BlueprintLink *> links = port->links();
			for (int i = 0; i < links.size(); i++){
				BlueprintObject* upport = links[0]->sourceNode() == port ? links[0]->destNode() : links[0]->sourceNode();
				BlueprintModel* upmodel = dynamic_cast<BlueprintModel*>(upport->parentItem());
				Q_ASSERT(upmodel != NULL);

				if (upmodel == model){
					ports.append(port);
				}
			}
		}
	}

	return ports;
}

//  �Ƿ�Ϊ�ڲ��˿�
bool BlueprintModel::isInternalPort(BlueprintModelPort* port){
	if (port == NULL)
		return false;

	if (objPort_ && port == objPort_){
		return true;
	}

	if (behPort_ && port == behPort_){
		return true;
	}

	return false;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

bool checkModelSequence(BlueprintModel* up, BlueprintModel* down){
	bool res = true;
	if (up->type() == KISMET_FLOW_BRANCH){  // ��֧����
		res = !(down->type() == KISMET_MODEL && down->modelType() == PT_CONDITION);  // ����������
	}
	else if (up->type() == KISMET_FLOW_SEQUENCE){  // ����
	}
	else if (up->type() == KISMET_MODEL && up->modelType() == PT_CONDITION){  // ����ģ��
		res = !(down->type() == KISMET_MODEL && down->modelType() == PT_CONDITION);  // ����������
	}
	else if (up->type() == KISMET_MODEL && up->modelType() == PT_EXPRESS){  // ���ʽģ��
		// ���α�������ѧ�������
		res = (down->type() == KISMET_ARITHMETIC || (down->type() == KISMET_MODEL && down->modelType() == PT_ACTION));
	}
	else if (up->type() == KISMET_MODEL && up->modelType() == PT_ACTION){  // ����ģ��
		// ����ģ�ͱ�����ACTION
		res = (down->type() == KISMET_MODEL && down->modelType() == PT_ACTION);
	}
	return res;
}

bool BlueprintModel::validate(QStringList& msg){

	bool hasInCtrls = false;
	bool hasOutCtrls = false;
	bool hasOutData = false;
	BlueprintModel* upmodel = NULL;
	BlueprintObject* upport = NULL;

	isError_ = false;
	// ���objPort,ȷ��ObjPort����������
	if (objPort_){
		if (!objPort_->hasLink()){
			isError_ = true;
			msg.append(ERROR_MSG.arg(UDQ_TR("��������˿�<%1>����������!").arg(objPort_->title())));
		}
		else{
			// �����������ͱ���һ��
			ResourceHash srcPlugin = plugin_->hashKey();

			foreach(BlueprintLink* l, objPort_->links()){
				upport = l->sourceNode() == objPort_ ? l->destNode() : l->sourceNode();
				if (upport->type() != KISMET_INSTANCE){
					isError_ = true;
					msg.append(ERROR_MSG.arg(UDQ_TR("��������˿�<%1>���Ӵ��󣬱����Ƕ���!").arg(objPort_->title())));
				}
				else{
					ResourceHash destPlugin = ((BlueprintInstance*)upport)->prototype_->prototype_->hashKey();
					if (destPlugin != srcPlugin ){
						if (plugin_->type_ == PIT_ENTITY){ // ֻ��ʵ��������Ҫ����ƥ��
							isError_ = true;
							msg.append(ERROR_MSG.arg(UDQ_TR("��������˿�<%1>���Ӵ��󣬱�����<%2>���Ͷ���!").arg(objPort_->title()).arg(plugin_->objectName())));
						}
						else if (plugin_->type_ == PIT_BEHAVIOR){
							// ��Ҫ�������Ƿ������������Ϊ
							if (!((BlueprintInstance*)upport)->prototype_->hasAttachment(plugin_)){
								isError_ = true;
								msg.append(ERROR_MSG.arg(UDQ_TR("��������˿�<%1>���Ӵ������ζ���û�а���<%2>��Ϊ!").arg(objPort_->title()).arg(plugin_->objectName())));
							}
						}
					}
				}
			}
		}
	}

	// ������е�����ģ�ͣ����û����Ϊ����
	foreach(BlueprintModelPort* port, inCtrlPorts_){

		// �˿ڵ���������
		if (!hasInCtrls){
			hasInCtrls = port->hasLink();
		}

		if (port->links().size() > 1 && !(type() == KISMET_MODEL && modelType() == PT_ACTION)){  // ����ģ����������������
			// ���ƶ˿�ֻ����һ������
			isError_ = true;
			msg.append(ERROR_MSG.arg(UDQ_TR("��������˿�<%2>���ڶ���1��������!").arg(port->title())));
		}

		foreach(BlueprintLink* link, port->links()){
			// ����������ӵ�ģ�Ͷ���
			upport = link->sourceNode() == port ? link->destNode() : link->sourceNode();

			// �������ģ��
			upmodel = dynamic_cast<BlueprintModel*>(upport->parentItem());
			Q_ASSERT(upmodel != NULL);

			if (!checkModelSequence(upmodel, this)){ // �������ӱ����Ƿ�֧���ƣ����к�����
				// ���ƶ˿�ֻ����һ������
				isError_ = true;
				msg.append(ERROR_MSG.arg(UDQ_TR("��������˿�<%1>����ģ�����Ͳ���ȷ!").arg(port->title())));
			}
		}
	}

	foreach(BlueprintModelPort* port, outCtrlPorts_){
		// �˿ڵ���������	
		if (!hasOutCtrls){
			hasOutCtrls = port->hasLink();
		}

		if (port->links().size() > 1){
			if ((type() == KISMET_MODEL && modelType() == PT_CONDITION) && !hasDownStream(KISMET_MODEL, PT_ACTION)){
				// ����ģ����β���blockͷģ�ͣ��������ж���������
			}
			else{
				// ���ƶ˿�ֻ����һ������
				isError_ = true;
				msg.append(ERROR_MSG.arg(UDQ_TR("��������˿�<%1>���ڶ���1��������!").arg(port->title())));
			}
		}
	}

	// ��������
	foreach(BlueprintModelPort* port, inPorts_){
		// ��������Ƿ�Ϊ��
		if (!port->hasLink() && port->getValue().isEmpty()){
			if (port->dataType() == UDQ_T("object")){
				isError_ = true;
				msg.append(ERROR_MSG.arg(UDQ_TR("��������˿�<%1>����Ϊ��ֵ!").arg(port->title())));
			}
			else{
				msg.append(WARNING_MSG.arg(UDQ_TR("��������˿�<%1>Ϊ��ֵ!").arg(port->title())));
			}
		}

		// �˿ڵ���������	
		if (port->links().size() > 1){
			// ����˿�ֻ����һ������
			isError_ = true;
			msg.append(ERROR_MSG.arg(UDQ_TR("��������˿�<%1>���ڶ���1������������!").arg(port->title())));
		}

		// ���˿���������
		QStringList portMsg;
		bool r = port->validate(msg);
		if (!r){
			isError_ = true;
		}

	}

	// �������
	foreach(BlueprintModelPort* port, outPorts_){
		// ��������Ƿ�Ϊ��
		if (!hasOutData){
			hasOutData = port->hasLink();
		}

		// ���˿���������
		QStringList portMsg;
		bool r = port->validate(msg);
		if (!r){
			isError_ = true;
		}
	}

	if (!hasInCtrls && inCtrlPorts_.size() > 0){
		// û����������
		isError_ = true;
		msg.append(ERROR_MSG.arg(UDQ_TR("��������˿�û������!")));
	}

	if (!hasOutCtrls && outCtrlPorts_.size() > 0 && modelType() != PT_ACTION){
		// û����������
		msg.append(WARNING_MSG.arg(UDQ_TR("��������˿�û������!")));
	}

	if (!hasOutData && outPorts_.size() > 0){
		// û����������
		msg.append(WARNING_MSG.arg(UDQ_TR("��������˿�û������!")));
	}

	return !isError_;
}

void BlueprintModel::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){
	BlueprintObject::contextMenuEvent(event);

	event->setAccepted(true);
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////


void BlueprintModel::setPortValue(const QString& name, const QString& v){

}
void BlueprintModel::setPortValue(const QString& name, int v){

}
void BlueprintModel::setPortValue(const QString& name, double v){

}
void BlueprintModel::setPortValue(const QString& name, bool v){

}