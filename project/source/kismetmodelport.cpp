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

#include "kismetmodelport.h"
#include <QtGui>
#include <QLineEdit>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QMenu>
#include <math.h>
#include "kismetdefine.h"
#include "kismetscene.h"
#include "kismetlinkarray.h"
#include "configmanager.h"
#include "menumanager.h"

const int PORT_SIZE = 18;
const int PORT_ARROW_WIDTH = 4;
const int PORT_WIDTH = PORT_SIZE + PORT_ARROW_WIDTH;
const int PORT_ARROW_HEIGHT = 4;
const int PORT_SPACE = 4;

const int DEFAULT_CTRL_WIDTH = 80;
const int DEFAULT_CTRL_MAXWIDTH = 160;

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

BlueprintControl::BlueprintControl(const QString& datatype, const QStringList& enums, QGraphicsItem * parent) :QGraphicsProxyWidget(parent, Qt::Widget){

	prepareWidget(datatype, enums);

	// ���ñ�־
	setFlag(QGraphicsItem::ItemIsMovable, false);
	setFlag(QGraphicsItem::ItemIsSelectable, true);

	// ������꾭���¼�
	setAcceptHoverEvents(true);

	// �������Ϸ�
	setAcceptDrops(false);
	setVisible(true);

}

BlueprintControl::~BlueprintControl(){
}

QRectF BlueprintControl::boundingRect() const{
	return QGraphicsProxyWidget::boundingRect();
}

bool BlueprintControl::isComboType(){
	return datatype_ == UDQ_T("enum") || datatype_ == UDQ_T("cmp") ||datatype_ == UDQ_T("blend");
}

int BlueprintControl::valueIndex(const QString& v){
	return enums_.indexOf(v);
}


void BlueprintControl::prepareWidget(const QString& datatype, const QStringList& enums ){
	datatype_ = datatype;
	enums_ = enums;

	if (datatype_ == UDQ_T("cmp")){
		enums_.clear();
		enums_.append(UDQ_TR("����"));
		enums_.append(UDQ_TR("������"));
		enums_.append(UDQ_TR("С��"));
		enums_.append(UDQ_TR("С�ڵ���"));
		enums_.append(UDQ_TR("����"));
		enums_.append(UDQ_TR("���ڵ���"));
	}
	else if (datatype_ == UDQ_T("blend")){  // "Normal","Additive","Xor","Copy","DestinationOver","SourceIn","DestinationIn","SourceOut","DestinationOut","SourceAtop","DestinationAtop"],
		enums_.clear();
		enums_.append(UDQ_TR("Normal"));
		enums_.append(UDQ_TR("Additive"));
		enums_.append(UDQ_TR("Xor"));
		enums_.append(UDQ_TR("Copy"));
		enums_.append(UDQ_TR("DestinationOver"));
		enums_.append(UDQ_TR("SourceIn"));
		enums_.append(UDQ_TR("DestinationIn"));
		enums_.append(UDQ_TR("SourceOut"));
		enums_.append(UDQ_TR("DestinationOut"));
		enums_.append(UDQ_TR("SourceAtop"));
		enums_.append(UDQ_TR("DestinationAtop"));
	}

	if (datatype_ == UDQ_T("int")){
		widget_ = new QLineEdit();
		((QLineEdit*)widget_)->setValidator(new QIntValidator());
	}
	else if(datatype_ == UDQ_T("float")){
		widget_ = new QLineEdit();
		QDoubleValidator* dv = new QDoubleValidator();
		dv->setNotation(QDoubleValidator::StandardNotation);
		((QLineEdit*)widget_)->setValidator(dv);
	}
	else if (isComboType()){
		widget_ = new QComboBox();
		((QComboBox*)widget_)->addItems(enums_);
	}
	else if (datatype_ == UDQ_T("bool")){
		widget_ = new QCheckBox();
	}
	else if (datatype_ == UDQ_T("key")){  // ���̰���
		widget_ = new QComboBox();
		((QComboBox*)widget_)->addItems(gConfigManager->keyboardMap());
	}
	else if (datatype_ == UDQ_T("color")){  // ��ɫֵ
		widget_ = new QLineEdit();
		((QLineEdit*)widget_)->setInputMask(UDQ_T("HHHHHH"));
	}
	else{  // ����һ�ɰ��ַ����Դ�
		widget_ = new QLineEdit();
	}

	if (datatype_ != UDQ_T("bool")){
		// �������ѡ��
		QString maxstr;
		int width = DEFAULT_CTRL_WIDTH;

		// ����ؼ�����
		if (isComboType()){
			foreach(QString str, enums_){
				if (str.size() > maxstr.size()){
					maxstr = str;
				}
			}
			if (maxstr.size() > 0){
				QSize s = computeText(maxstr, widget_->font());
				width = qMin(qMax(s.width(), width), DEFAULT_CTRL_MAXWIDTH);
			}
		}
		else if(datatype_ == UDQ_T("key")  ){  // ��������
			width = 120;
		}
		widget_->setFixedSize(QSize(width, PORT_SIZE));
	}

	setWidget(widget_);
}

QString BlueprintControl::value(){
	QString v;
	if (isComboType() || datatype_ == UDQ_T("key") ){
		v = ((QComboBox*)widget_)->currentText(); 
	}
	else if (datatype_ == UDQ_T("bool")){
		v = ((QCheckBox*)widget_)->isChecked() ? UDQ_T("true") : UDQ_T("false");
	}
	else{
		v = ((QLineEdit*)widget_)->text();
	}

	if (datatype_ == UDQ_T("color") && v.size() < 6 ){  // ����6���ַ�
		int num = 6 - v.size();
		for (int i = 0; i < num; i++){
			v.append(UDQ_C('0'));
		}
	}
	return v;
}

void BlueprintControl::setValue(const QString& v){
	if ( isComboType() || datatype_ == UDQ_T("key")){
		QComboBox* box = (QComboBox*)widget_;
		int index = box->findText(v);
		box->setCurrentIndex(index >= 0 ? index : 0);
	}
	else if (datatype_ == UDQ_T("bool")){
		((QCheckBox*)widget_)->setChecked(v == UDQ_T("true"));
	}
	else{
		((QLineEdit*)widget_)->setText(v);
	}
}

void BlueprintControl::paintWindowFrame(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	QGraphicsProxyWidget::paintWindowFrame(painter, option, widget);
}


void BlueprintControl::hoverEnterEvent(QGraphicsSceneHoverEvent *event){
	setCursor(Qt::ArrowCursor);
	QGraphicsProxyWidget::hoverEnterEvent(event);
	scene()->setActiveWindow(this);
}

void BlueprintControl::hoverLeaveEvent(QGraphicsSceneHoverEvent *event){
	QGraphicsProxyWidget::hoverLeaveEvent(event);
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

BlueprintModelPort::BlueprintModelPort(int orient, const QString& datatype, const QString&name, const QStringList& enums, QGraphicsItem * parent)
	:BlueprintObject(KISMET_DEPTH_MODELPORT, parent), ctrl_(NULL){
	name_ = name;
	orient_ = orient;
	dataType_ = datatype;

	underMouse_ = false;
	underLink_ = false;

	boundingBox_.setRect(0, 0, PORT_WIDTH, PORT_SIZE);

	// ���ñ�־
	setFlag(QGraphicsItem::ItemIsMovable, false);

	if (orient == EVT_DATA_IN && dataType_ != UDQ_T("object")){ // ֻ���������ݴ��ؼ�
		ctrl_ = new BlueprintControl(dataType_, enums );
		ctrl_->setParentItem(this);
	}

	prepare();
	adjust();

}


BlueprintModelPort::~BlueprintModelPort(){

}

QColor BlueprintModelPort::portColor(const QString& dt){
	QColor clr = TYPE_COLOR;

	// �˿���ɫ
	if (dt == UDQ_T("bool")){
		clr = BOOL_COLOR;
	}
	else if (dt == UDQ_T("int") || dt == UDQ_T("color")){
		clr = INT_COLOR;
	}
	else if (dt == UDQ_T("float")){
		clr = FLOAT_COLOR;
	}
	else if (dt == UDQ_T("any")){
		clr = ANY_COLOR;
	}
	else if (dt == UDQ_T("string") ||  dt == UDQ_T("layer") || dt == UDQ_T("layout") || dt==UDQ_T("event")){
		clr = STRING_COLOR;
	}
	else if (dt == UDQ_T("object")){
		clr = OBJECT_COLOR;
	}
	else if (dt == UDQ_T("family")){
		clr = FAMILY_COLOR;
	}
	return clr;
}

void BlueprintModelPort::prepare(){

	// �˿���ɫ
	portcolor_ = portColor(dataType_);
	
	//  �˿���״
	QPainterPath path;
	if (orient_ < EVT_DATA_IN){ // ���ƶ˿�
		QPointF pos(PORT_WIDTH, PORT_SIZE / 2);
		path.moveTo(0,0);
		path.lineTo(PORT_SIZE / 2, 0);
		path.lineTo(pos);
		path.lineTo(PORT_SIZE / 2, PORT_SIZE);
		path.lineTo(0, PORT_SIZE);
		path.closeSubpath();
	}
	else{  // ���ݶ˿�
		QRectF rect(0, 0, PORT_SIZE, PORT_SIZE);
		path.addEllipse(rect);
		path.closeSubpath();

		QPointF pos(rect.right(), rect.center().y());
		path.moveTo(pos.x(), pos.y() - PORT_ARROW_HEIGHT);
		path.lineTo(pos.x() + PORT_ARROW_WIDTH, pos.y());
		path.lineTo(pos.x(), pos.y() + PORT_ARROW_HEIGHT);
		path.closeSubpath();
	}
	path_ = path;
}

void BlueprintModelPort::adjust(){
	// ����ؼ�����
	QSizeF size(0, 0);
	if (name_.size() > 0){
		QFontMetrics fm(textFont());
		size = fm.size(Qt::TextSingleLine, name_) ;
		size.setWidth( size.width() + +PORT_SPACE);
	}

	prepareGeometryChange();
	boundingBox_.setRect(0, 0, PORT_WIDTH + size.width(), PORT_SIZE);

	// �����ؼ��Ӷ���λ��
	if (ctrl_){
		Q_ASSERT(orient_ == EVT_DATA_IN);
		ctrl_->setPos(boundingBox_.width(), 0);
	}
}

QRectF BlueprintModelPort::ctrlBoundBox(){
	if ( ctrl_ )
		return boundingBox_.united(childrenBoundingRect());

	return boundingBox_;
}

bool BlueprintModelPort::isNullName(){
	return name_.isEmpty()  || name_ == UDQ_T("???");
}

void BlueprintModelPort::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	Q_UNUSED(widget);

	painter->setRenderHint(QPainter::HighQualityAntialiasing);
	//
	QBrush borderBrush;
	borderBrush.setStyle( Qt::SolidPattern );

	if( underMouse_  ){
		// ����ѡ�����ɫ
		borderBrush.setColor( SELECT_COLOR );
	}
	else{
		borderBrush.setColor( portcolor_ );
	}

	qreal textPos = PORT_WIDTH + 2;
	painter->save();
	if (orient_ == EVT_CTRL_OUT || orient_ == EVT_DATA_OUT){
		textPos = 0;
		QTransform trans = painter->transform();
		trans.translate(boundingBox_.right() - PORT_WIDTH, 0);
		painter->setTransform(trans);
	}
	// ����Path
	if (hasLink()){
		painter->fillPath(path_, borderBrush);
	}
	else{
		QPen pen(underMouse_ ? SELECT_COLOR : portcolor_);
		pen.setWidth(2.0);
		painter->setPen(pen);
		painter->drawPath(path_);
	}

	painter->restore();

	bool simpledraw = canSimpleDraw();
	if (!simpledraw){
		// ��������
		painter->setPen(Qt::black);
		painter->setFont(textFont());
		painter->drawText(textPos, (PORT_SIZE + painter->font().pixelSize()) / 2, name_);
	}

}


void BlueprintModelPort::hoverEnterEvent(QGraphicsSceneHoverEvent * event){
	// ����ZValue
	oldZ_ =zValue();
	setZValue(EDITOR_DEPTH_FRONT);

	underMouse_ = true;
	BlueprintObject::hoverEnterEvent(event);
	update();
}

void BlueprintModelPort::hoverLeaveEvent(QGraphicsSceneHoverEvent * event){
	setZValue(oldZ_);
	underMouse_ = false;
	BlueprintObject::hoverLeaveEvent(event);
	update();
}

void BlueprintModelPort::mouseMoveEvent(QGraphicsSceneMouseEvent * event){
	// ������ʱ��
	if( underLink_ ){
		tempLink_->updateTempLink( event->scenePos() );
		return;
	}

	QGraphicsItem::mouseMoveEvent( event );
}


void BlueprintModelPort::mousePressEvent(QGraphicsSceneMouseEvent * event){
	// ����״̬

	if (isUnderMouse() && event->button() == Qt::LeftButton){  // ���

		KismetScene* parent = (KismetScene*)scene();

		// ���ͬʱ����ALt
		if (event->modifiers() & Qt::AltModifier){
			clearLink();
		}
		else{
			if (!underLink_){
				underLink_ = true;
				parent->setMode(KismetScene::DOM_ADDLINK);

				if (orient_ == EVT_CTRL_IN || orient_ == EVT_DATA_IN)
					tempLink_ = new BlueprintLinkArray(0, this, QList<QPointF>(), true);
				else
					tempLink_ = new BlueprintLinkArray(this, 0, QList<QPointF>(), true);

				tempLink_->updateTempLink(event->scenePos());
				parent->addItem(tempLink_);
				grabMouse();
			}
		}
	}

	QGraphicsItem::mousePressEvent( event );

}

// ���˿����
bool BlueprintModelPort::checkPortPair(BlueprintModelPort* port){
	if( port == this )
		return false;

	// �������
	int diff = orient_ - port->orient_;
	return   (qAbs(diff) == 1 );  //  �������෴����
}


void BlueprintModelPort::mouseReleaseEvent(QGraphicsSceneMouseEvent * event){

	// �˳�����
	if( underLink_  ){
		KismetScene* parent = (KismetScene*)scene();

		if( event->button() == Qt::RightButton ){
			underLink_ = false;
			// ������ߺϷ��ԣ�������Ϸ���ɾ��
			scene()->removeItem( tempLink_ );
			delete tempLink_;
			tempLink_ = 0;
			tempLineCtrlPts_.clear();
			ungrabMouse();
			parent->setMode(KismetScene::DOM_POINTER);
		}
		else{

			KismetScene* parent = (KismetScene*)scene();
			//  ����Ƿ�ѡ��Ŀ��˿�
			QList<QGraphicsItem *> items = scene()->items ( event->scenePos() );
			items.removeAll( tempLink_ );

			// Ѱ�ҵ�һ����Ӧport
			for (int i = 0; i < items.size(); ++i) {
				BlueprintModelPort* port = dynamic_cast<BlueprintModelPort*>(items.at(i));
				if( port ){
					// ���˿�����
					if( checkPortPair(port) ){
						// ����ظ�
						bool found = false;
						BlueprintObject* curSrc = tempLink_->sourceNode();
						BlueprintObject* curDest = tempLink_->destNode();

						if( curSrc == NULL ){
							curSrc = port;
						}
						else{
							curDest = port;
						}

						foreach( BlueprintLink* link, links() ){
							if( link->sourceNode() == curSrc
								&& link->destNode() ==curDest ){
									found = true;
									break;
							}
						}

						if( !found ){

							// �����Լ�����
							if( curSrc->parentItem() != curDest->parentItem() ){

								// ���Ϊ��ʽ����
								tempLink_->upgradeLink( port );
								parent->notifyObjectInsert(tempLink_);

								tempLink_ = 0;
								underLink_ = false;						
								ungrabMouse();
								parent->setMode(KismetScene::DOM_POINTER);
								return;
							}
						}
					}
				}
			}

			if( items.size() == 1 ){  // ʲôҲû�е�ѡ��
				((BlueprintLinkArray*)tempLink_)->addControlPoint( event->scenePos() );
			}

		}
	
		return;
	}

	QGraphicsItem::mouseReleaseEvent( event );

}

// ��ȡ���Ƶ�
QPointF BlueprintModelPort::controlPoint(int orient){

	QPointF point = sceneCenter();

	if( orient_ == EVT_CTRL_IN || orient_ == EVT_DATA_IN )
			point.setX( point.x() - 50 );
	else
			point.setX( point.x() + 50 );

	return point;
}

// ��ȡ��
QPointF BlueprintModelPort::endPoint(void){
	QPointF point = sceneCenter();

	if (orient_ == EVT_CTRL_IN || orient_ == EVT_DATA_IN)
		point.setX(point.x() - boundingBox_.width() / 2);
	else
		point.setX(point.x() + boundingBox_.width() / 2);

	return point;
}

// ��������
void BlueprintModelPort::updateLink(){
	foreach (BlueprintLink *line, lineList_){
		line->updatePort( this );
	}
}


void BlueprintModelPort::setUnderMouse(bool flag){
	foreach(BlueprintLink *line, lineList_){
		line->setUnderMouse( flag );
		line->update();
	}
}

QVariant BlueprintModelPort::itemChange(GraphicsItemChange change, const QVariant & value){
	if( change == ItemPositionHasChanged && scene() ){
		foreach(BlueprintLink *line, lineList_){
				line->adjust();
		}
	}
	return QGraphicsItem::itemChange( change, value );
}

// ������ɫ
void BlueprintModelPort::updateDataType(){
	foreach(BlueprintLink *line, lineList_){
		line->update();
	}
	update();
}

QString BlueprintModelPort::getValue(){
	if (ctrl_){
		return ctrl_->value();
	}
	return QString();
}

void BlueprintModelPort::setValue(const QString& v){
	if (ctrl_){
		ctrl_->setValue(v);
	}
}

int BlueprintModelPort::dataTypeId(){
	//0 = number(��ֵ)��1 = string���ַ�������2 = audio����Ƶ�ļ�����3 = combo��ö��ֵ����
	//4 = object���������ͣ���5 = layer��ͼ�㣩��6 = layout�����棩��7 = any���������ͣ���
	//	8 = cmp���Ƚϣ���9 = keyboard�����̣���10 = instance������ʵ������
	//	11 = event���¼�����12 = file���ļ���Ϣ����13 = variadic���������飩
	int deftype = -1;

	if ( dataType_ == UDQ_T("int") || dataType_ == UDQ_T("float") || dataType_ == UDQ_T("color")){
		deftype = 0;
	}
	else if (dataType_ == UDQ_T("string")){
		deftype = 1;
	}
	else if (dataType_ == UDQ_T("audio")){
		deftype = 2;
	}
	else if (dataType_ == UDQ_T("enum") || dataType_ == UDQ_T("bool") ){
		deftype = 3;
	}
	else if (dataType_ == UDQ_T("object") || dataType_ == UDQ_T("family")){
		deftype = 4;
	}
	else if (dataType_ == UDQ_T("layer")){
		deftype = 5;
	}
	else if (dataType_ == UDQ_T("layout")){
		deftype = 6;
	}
	else if (dataType_ == UDQ_T("any")){
		deftype = 7;
	}
	else if (dataType_ == UDQ_T("cmp")){
		deftype = 8;
	}
	else if (dataType_ == UDQ_T("key")){
		deftype = 9;
	}
	else if (dataType_ == UDQ_T("instance")){
		deftype = 10;
	}
	else if (dataType_ == UDQ_T("event")){
		deftype = 11;
	}
	else{
		Q_UNREACHABLE();
	}
	return deftype;
}


// ��ȡ���ʽ
QJsonValue BlueprintModelPort::expNode(int dtt){
	QJsonValue exp;
	Q_ASSERT(!hasLink() && orient() == EVT_DATA_IN);  // û���ⲿ����

	QString value = getValue();
	QString dt = dataType();

	QJsonArray arr;
	if (dt == UDQ_T("bool")){
		exp = value == UDQ_T("true") ? 1 : 0;
	}
	else if (dt == UDQ_T("int")){
		arr.append((int)0);
		arr.append(value.toInt());
	}
	else if (dt == UDQ_T("float")){
		arr.append((int)1);
		arr.append(value.toDouble());
	}
	else if (dt == UDQ_T("any")){
		// �ж�ֵ�Ƿ�Ϊ���������������ַ���
		bool r = false;
		int intv = value.toInt(&r);
		if (r){  // ����Ƿ�Ϊ����
			arr.append((int)0);
			arr.append(intv);
		}
		else{
			double dv = value.toDouble(&r);
			if (r){  // ����Ƿ�Ϊ������
				arr.append((int)1);
				arr.append(dv);
			}
			else{  // ���Ĭ��Ϊ�ַ���
				arr.append((int)2);
				arr.append(value);
			}
		}
	}
	else if (dt == UDQ_T("string")){
		arr.append((int)2);
		arr.append(value);
	}
	else if (dt == UDQ_T("object")){
		// ���������ҵ�����������ObjectType�е�����
		Q_UNREACHABLE();
	}
	else if (dt == UDQ_T("family")){
		//  ���������ҵ���ObjectType�е�����
		Q_UNREACHABLE();
	}
	else if (dt == UDQ_T("layer")){
		//
		arr.append((int)2);
		arr.append(value);
	}
	else if (dt == UDQ_T("layout")){
		exp = value;  // ������
	}
	else if (dt == UDQ_T("enum")){
		// �ҵ�ֵ������
		exp = ctrl_->valueIndex(value);
	}
	else if (dt == UDQ_T("cmp")){
		// cmp����ֵ
		exp = ctrl_->valueIndex(value);
	}
	else if (dt == UDQ_T("key")){
		// �ҵ���ֵ
		exp = gConfigManager->keyboardValue(value);
	}
	else if (dt == UDQ_T("event")){
		// ������
		exp = value;
	}
	else if (dt == UDQ_T("color")){
		// ������
		arr.append((int)0);
		// �ߵ���ɫ����
		Q_ASSERT(value.size() == 6);
		bool ok;
		int red = value.left(2).toInt(&ok,16);
		int green = value.mid(2, 2).toInt(&ok,16);
		int blue = value.right(2).toInt(&ok,16);
		int t = red | (green << 8 ) | (blue << 16);
		arr.append(t);
	}
	else if (dt == UDQ_T("instance")){
		// ʵ������������
		//  �ҵ�Object�˿ڣ���ȡ��������
		// û���ҵ���Ϊ-1��
		exp = -1;
		BlueprintModel* parent = (BlueprintModel*)parentItem();
		if (parent){
			BlueprintModelPort* objport = parent->inport(UDQ_T("Sprite"));
			if (objport == NULL){
				objport = parent->inport(UDQ_T("Object"));
			}

			if (objport){
				QList<BlueprintLink *> links = objport->links();
				if (links.size() > 0){
					BlueprintObject* upport = NULL;
					for (int i = 0; i < links.size(); i++){
						upport = links[i]->sourceNode() == objport ? links[i]->destNode() : links[i]->sourceNode();
						if (upport->type() == KISMET_INSTANCE){
							BlueprintInstance* instance = (BlueprintInstance*)upport;
							Q_ASSERT(instance != NULL);
							exp = QString::number(hashString(instance->prototype_->description() + UDQ_T(".") + value));  // ����ʱ�滻
							break;
						}
					}
				}
			}
		}
	}
	else{
		Q_UNREACHABLE();
	}

	if (!arr.isEmpty()){
		exp = arr;
	}

	return exp;

}

void BlueprintModelPort::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){

	if (type() != KISMET_MODEL_PORT || underLink_ )
		return;

	((KismetScene*)scene())->setMenuHost(this);

	QMenu menu;
	QAction* act = gMenuManager->getAction(ORION_KISMET_CLEARPORTLINK);
	menu.addAction(act);


	if (canDelete()){
		menu.addSeparator();
		act = gMenuManager->getAction(ORION_KISMET_DELETEPORT);
		menu.addAction(act);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	menu.exec(event->screenPos());
	event->setAccepted(true);
}

void BlueprintModelPort::copyState(QDataStream& stream, int type){
	if (type == BST_DISPLAY){
		stream << name_ << orient_ << dataType_;
	}
}


QString BlueprintModelPort::orientString(){
	if (orient_ == EVT_DATA_IN){
		return UDQ_TR("��������˿�");
	}
	else if (orient_ == EVT_DATA_OUT){
		return UDQ_TR("��������˿�");
	}
	else if (orient_ == EVT_CTRL_IN){
		return UDQ_TR("��������˿�");
	}
	else if (orient_ == EVT_CTRL_OUT){
		return UDQ_TR("��������˿�");
	}

	return UDQ_TR("δ֪���Ͷ˿�");
}

bool BlueprintModelPort::validate(QStringList& msg){
	QStringList errors;

	isError_ = false;
	bool res;
	foreach(BlueprintLink* l, links()){
		res = l->validate(errors);
		if (!res){
			isError_ = true;
		}
	}

	int msgtype;
	foreach (QString e, errors){
		QStringList items = e.split(UDQ_T("@"));
		Q_ASSERT(items.size() >= 2);

		msgtype = items[0].toUInt() ;

		if (msgtype == 1){
			msg.append(ERROR_MSG.arg(UDQ_TR("%1<%2>���ߴ��ڴ���%3").arg(orientString()).arg(title()).arg(items[1])));
		}
		else{
			msg.append(WARNING_MSG.arg(UDQ_TR("%1<%2>���ߣ�%3").arg(orientString()).arg(title()).arg(items[1])));
		}
	}

	return !isError_;
}