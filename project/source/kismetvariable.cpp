#include "kismetvariable.h"
#include <QtGui>
#include <math.h>
#include "kismetscene.h"
#include "kismetmodel.h"
#include "kismetlinkarray.h"

const int VARPORT_SIZE =  64;
const int VARPORTT_NAMEW = 100;
const int VARPORT_NAMEH = 20;

BlueprintVariable::BlueprintVariable() : BlueprintModelPort(EVT_DATA_OUT, UDQ_T("event")), quickCloning_(false), var_(NULL), script_(NULL){
	setZValue(KISMET_DEPTH_VARIABLE);
	// ���ݴ�С���µ��������д�С
	boundingBox_.setRect(0, 0, VARPORT_SIZE, VARPORT_SIZE);
	// ���ñ�־
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);

}

BlueprintVariable::BlueprintVariable(ResourceHash key, ScriptVariable* sv)
	: BlueprintModelPort(EVT_DATA_OUT, sv->datatype, sv->name), quickCloning_(false), var_(sv){

	setZValue(KISMET_DEPTH_VARIABLE);
	// ���ݴ�С���µ��������д�С
	boundingBox_.setRect(0, 0, VARPORT_SIZE, VARPORT_SIZE);

	// ���ñ�־
	setFlag(QGraphicsItem::ItemIsMovable, true);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges, true);
	setFlag(QGraphicsItem::ItemIsFocusable, true);

	script_ = (BlueprintResourceObject*)ResourceObject::findResource(key);
	postinit();

}

BlueprintVariable::~BlueprintVariable(){

}

void BlueprintVariable::postinit(){
	if (var_){
		setDescription(var_->desc);
	}
	else{
		setDescription(QString());
	}

}

// ��ȡ���Ƶ�
QPointF BlueprintVariable::controlPoint(int orient){

	QPointF point = sceneCenter();
	point.setX(point.x() + VARPORTT_NAMEW);
	return point;
}

// ���˿����
bool BlueprintVariable::checkPortPair(BlueprintModelPort* port){

	if( port == this )
		return false;

	return orient_ - port->orient() == 0;  // ��������ͬ����

	// �������һ��

}


void	BlueprintVariable::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event){
	// �޸�����
	KismetScene* parent = (KismetScene*)scene();
}

void BlueprintVariable::paint(QPainter *painter, const QStyleOptionGraphicsItem *item, QWidget *widget){
	Q_UNUSED(widget);

	painter->setRenderHint( QPainter::Antialiasing, true );
	QPen borderPen;
	if( isSelected() || isUnderMouse()){
		// ����ѡ�����ɫ
		borderPen.setColor( Qt::yellow );
	}
	else{
		borderPen.setColor( Qt::black  );
	}
	borderPen.setWidth(1);
	painter->setPen( borderPen );

	QRectF circleR = boundingBox_.adjusted(4,4,-4,-4);
	QPainterPath path;
	path.moveTo( circleR.center().rx(), circleR.top() );
	path.lineTo( circleR.right(), circleR.center().ry() );
	path.lineTo( circleR.center().rx(), circleR.bottom() );
	path.lineTo( circleR.left(), circleR.center().ry() );
	path.lineTo( circleR.center().rx(), circleR.top() );
	portcolor_ = portColor(var_->datatype);
	painter->fillPath(path, portcolor_);  // ��������ȷ����ɫ

	QPainterPath pathbound;
	pathbound.moveTo(boundingBox_.center().rx(), boundingBox_.top());
	pathbound.lineTo(boundingBox_.right(), boundingBox_.center().ry());
	pathbound.lineTo(boundingBox_.center().rx(), boundingBox_.bottom());
	pathbound.lineTo(boundingBox_.left(), boundingBox_.center().ry());
	pathbound.lineTo(boundingBox_.center().rx(), boundingBox_.top());
	painter->drawPath(pathbound);
	
	bool simpleDraw = canSimpleDraw();
	if (!simpleDraw){
		painter->setFont(nameFont());
		// ����ȱʡֵ
		painter->setPen(Qt::white);
		painter->drawText(computeTextPos(circleR, var_->datatype, nameFont()), var_->datatype);

		painter->setPen(borderPen);
		// ��������
		QSize commentSize = computeText(var_->name, nameFont());
		painter->drawText(boundingBox_.left() + (boundingBox_.width() - commentSize.width()) / 2.0, boundingBox_.bottom() + VARPORT_NAMEH, var_->name);
	}
}

void BlueprintVariable::mousePressEvent(QGraphicsSceneMouseEvent * event){
		
	KismetScene* parent = (KismetScene*)scene();
	// ����״̬
	if( event->button() == Qt::LeftButton && quickCloning_ ){
		// ��ӿ�¡���߶˿�
		BlueprintVariable* var = parent->addVariableItem(event->scenePos(), script_->hashKey(), name_);
		var->update();
		return;
	}
	else if( isUnderMouse() && isSelected()  ){
		// ���ͬʱ����ALt, ɾ����������
		if( event->modifiers() & Qt::AltModifier ){
			clearLink();
		}
		return;
	}

	QGraphicsItem::mousePressEvent( event );

}

QVariant BlueprintVariable::itemChange(GraphicsItemChange change, const QVariant & value){

	if( change == ItemPositionHasChanged && scene() ){
		foreach (BlueprintLink *line, lineList_){
			line->updatePort(this);
		}
		return BlueprintObject::itemChange(change, value);
	}

	return BlueprintModelPort::itemChange(change, value);
}


void BlueprintVariable::keyPressEvent(QKeyEvent * event){

	if (isSelected()){
		if (event->key() == Qt::Key_X){
			quickCloning_ = true;
			grabMouse();
			return;
		}
	}

	QGraphicsItem::keyPressEvent(event);

}

void BlueprintVariable::keyReleaseEvent(QKeyEvent * event){

	if (event->key() == Qt::Key_X && quickCloning_){
		quickCloning_ = false;
		ungrabMouse();
		return;
	}

	QGraphicsItem::keyReleaseEvent(event);
}

// ����
void BlueprintVariable::encodeData(QDataStream& stream){
	BlueprintObject::encodeData(stream);
	stream << script_->hashKey();
	stream << hashString(var_->name);
}

void BlueprintVariable::decodeData(QDataStream& stream){
	BlueprintObject::decodeData(stream);

	ResourceHash key;
	ResourceHash keyvar;
	stream >> key >> keyvar;
	script_ = (BlueprintResourceObject*)ResourceObject::findResource(key);

	if (script_){
		var_ = script_->variables_.value(keyvar);
	}
	else{
		var_ = NULL;
	}
	postinit();
}

QJsonValue BlueprintVariable::expNode(int dt){

	if (dt == 11){
		return varName();
	}

	QJsonArray arr;
	arr.append((int)23);
	arr.append(varName());
	return arr;
}

bool BlueprintVariable::validate(QStringList& msg){
	if (!hasLink()){
		// ���棺 û������
		msg.append(WARNING_MSG.arg(UDQ_TR("����<%1>û������!").arg(varName())));
	}
	return !isError_;
}