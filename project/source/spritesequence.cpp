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

#include "spritesequence.h"
#include <QtGui>
#include <QGraphicsSceneHoverEvent>
#include <QApplication>
#include <math.h>
#include "spritescene.h"
#include "commondefine.h"
#include "imagehelper.h"
#include <QMenu>
#include "menumanager.h"
#include "spriteimageeditwindow.h"

const qreal FRAME_TIME = 1000.0 / 60;

SpriteSequenceFrame::SpriteSequenceFrame(SequenceFrame* frame, QGraphicsItem * parent)
	: SpriteObject(0, parent),  frame_(frame)
{
	errorimage_.load(UDQ_T(":/images/sprite_alert.png"));
	refreshImage();
	setPos(frame->pos);
	setFlag(QGraphicsItem::ItemIsMovable, false); // �����ƶ�
	setZValue(100);
	setAcceptedMouseButtons(Qt::LeftButton);
}

SpriteSequenceFrame::~SpriteSequenceFrame(){

}


void SpriteSequenceFrame::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
	Q_UNUSED(widget);

	QPointF p = pos();

	QPen borderPen;
	if (isSelected()){
		// ����ѡ�����ɫ
		borderPen.setColor(Qt::yellow);
	}
	else{
		borderPen.setColor(Qt::black);
	}
	painter->setPen(borderPen);
	painter->setFont(nameFont());

	if (isSelected()){ // ѡ�л���Tile�е�����

	}

	// ���Ʊ���
	painter->fillRect(boundingBox_, Qt::darkGray);
	// ����ͼƬ( )
	painter->drawImage(0, 0, frame_->buffer.isNull() ? errorimage_ : frame_->buffer );
	// ���Ʊ߿�
	painter->drawRect(boundingBox_);
	// ����ê��
	drawAnchor(painter);

}

void SpriteSequenceFrame::drawAnchor(QPainter* painter){
	if (parentItem() == NULL)
		return;

	ResourceHash anchor = ((SpriteSequence*)parentItem())->currentAnchor();

	if (anchor >= 0 && frame_->anchors.contains(anchor)){
		QPointF pos = frame_->anchors[anchor];
		pos.setX(boundingBox_.left() + pos.x() * boundingBox_.width());
		pos.setY(boundingBox_.top() + pos.y() * boundingBox_.height());

		painter->setPen(Qt::white);
		painter->setCompositionMode(QPainter::RasterOp_SourceXorDestination);
		// ����ʮ�ֱ�
		int length = 6;
		painter->drawLine(pos.x() - length, pos.y(), pos.x() + length, pos.y());
		painter->drawLine(pos.x(), pos.y() - length, pos.x(), pos.y() + length);
		painter->setCompositionMode(QPainter::CompositionMode_SourceOver);

		// ��������λ��
		painter->setPen(Qt::black);
		painter->setFont(nameFont());

		QPoint pt = pos.toPoint();
		QString  str = UDQ_TR("(%1,%2)").arg(pt.x()).arg(pt.y());
		QSize size = computeText(str, painter->font());
		// ����ID
		painter->drawText(boundingBox_.left() + (boundingBox_.width() - size.width()) / 2,
			boundingBox_.bottom() + SEQUENCE_MARGIN  / 2 + size.height(),
			str);
	}
}


QSize SpriteSequenceFrame::imageSize(){
	return image().size();
}

void	SpriteSequenceFrame::mouseMoveEvent(QGraphicsSceneMouseEvent * event){

		if (QLineF(event->screenPos(), event->buttonDownScreenPos(Qt::LeftButton)).length() >= QApplication::startDragDistance()) {
			// ֪ͨ���Ƴ���ǰ֡
			oldParent_ = parentItem();
			Q_ASSERT(oldParent_ != NULL);

			oldIndex_ = ((SpriteSequence*)oldParent_)->removeFrame(this);

			QDrag *drag = new QDrag(event->widget());
			QMimeData* mmdata = new QMimeData;
			QByteArray itemData;
			mmdata->setData(MIME_ORION_FRAME, itemData);  // �ӳ����в�������Frame��

			QImage thumb = thnumImage(64, 64, image());
			drag->setPixmap(QPixmap::fromImage(thumb));
			drag->setHotSpot(QPoint(32, 32));
			drag->setMimeData(mmdata);

			if (drag->exec() == Qt::IgnoreAction){
				restoreDrag();
			}
			setCursor(Qt::OpenHandCursor);
			return;
		}
		SpriteObject::mouseMoveEvent(event);
}

	void	SpriteSequenceFrame::mousePressEvent(QGraphicsSceneMouseEvent * event){
		SpriteObject::mousePressEvent(event);
	}

	void	SpriteSequenceFrame::mouseReleaseEvent(QGraphicsSceneMouseEvent * event){
		SpriteObject::mouseReleaseEvent(event);
	}

	// ͼƬ
	QImage& SpriteSequenceFrame::image(){
		if (frame_->buffer.isNull()){
			refreshImage();  // ���³��Ի�ȡͼƬ
		}
		return frame_->buffer.isNull() ? errorimage_ : frame_->buffer;
	}

	void SpriteSequenceFrame::refreshImage(){
		if (parentItem() == NULL){
			boundingBox_.setRect(0, 0, errorimage_.width(), errorimage_.height());
			return;
		}

		((SpriteSequence*)parentItem())->resourceInstance()->refreshImage(frame_);
		prepareGeometryChange();
		if (frame_->buffer.isNull()){
			boundingBox_.setRect(0, 0, errorimage_.width(), errorimage_.height());
		}
		else{
			boundingBox_.setRect(0, 0, frame_->buffer.width(), frame_->buffer.height());
		}
	}

	// �ָ��Ϸ�ǰ״̬
	bool SpriteSequenceFrame::restoreDrag(){
		if (oldParent_ == NULL)
			return false;
		((SpriteSequence*)oldParent_)->addFrame(this, oldIndex_);
		((SpriteSequence*)oldParent_)->adjust();
		oldParent_ = NULL;
		oldIndex_ = -1;

		return true;
	}

	// ����
	void SpriteSequenceFrame::updateInstance(){
		if (frame_){
			frame_->pos = pos();
		}
	}

	void  SpriteSequenceFrame::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){

		setSelected(true);
		((SpriteScene*)scene())->setCurrentMenuHost(this);
		// �����Ҽ��˵�
		QMenu menu;
		// ��Ӳ˵�
		menu.addAction(gMenuManager->getAction(ORION_SPRTIE_DELETE));
		menu.exec(event->screenPos());
		event->accept();
	}

	QVariant SpriteSequenceFrame::itemChange(GraphicsItemChange change, const QVariant & value){
		if (change == ItemSelectedChange && scene()){
			if (value.toBool()){  // �����ѡ�У����������ͬ�����е�֡����
				bool canSelect = true;
				QList<QGraphicsItem*> selitems = scene()->selectedItems();
				foreach(QGraphicsItem* item, selitems){
					if (item->type() == SPRITE_SEQUENCE ){
						canSelect = false;
						break;
					}
				}
				return canSelect;
			}
		}
		else if (change == ItemSelectedHasChanged && scene()){
			if (value.toBool()){  // �����ѡ�У����������ͬ�����е�֡����

				QList<QGraphicsItem*> selitems = scene()->selectedItems();
				foreach(QGraphicsItem* item, selitems){
					if (item->type() == SPRITE_SEQUENCEFRAME && item != this){
						item->setSelected(false);
					}
				}
			}

			return value;
		}
		return QGraphicsItem::itemChange(change, value);
	}

	// ������ê��Ϊԭ��������С
	QRect  SpriteSequenceFrame::anchorRegion(){
		ResourceHash cekey = hashString(ANCHOR_CENTER);
		Q_ASSERT(frame_ != NULL );
		if (!frame_->anchors.contains(cekey)){  // ���ȱʡê��
			frame_->anchors.insert(cekey, QPointF(0.5, 0.5));
		}
		QPointF pos = frame_->anchors[cekey];

		QSize size = imageSize();
		QRect rect(0, 0, size.width(), size.height());
		rect.translate(-pos.x() * size.width(), -pos.y() * size.height());

		return rect;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////


	SpriteSequence::SpriteSequence(SpriteSequenceResourceObject* instance, QGraphicsItem * parent)
		: SpriteObject(0, parent), instance_(instance), insertAnchor_(-1), currrentFrame_(-1), timeLine_(NULL), currentAnchor_(0)
	{
		setFlag(QGraphicsItem::ItemIsMovable, !instance_->isLocking_);
		// �����Ϸ�
		setAcceptDrops(true);
		setPos(instance_->pos_);

		// ��ʼ������֡
		initFrames();
		initAnimation();

		adjust();


	}

	SpriteSequence::~SpriteSequence(){

	}

	QString SpriteSequence::title(){
		if (instance_){
			return instance_->objectName();
		}

		return QString();
	}

	void SpriteSequence::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget){
		Q_UNUSED(widget);

		QPen borderPen;
		if (isSelected()){
			// ����ѡ�����ɫ
			borderPen.setColor(Qt::yellow);
		}
		else{
			borderPen.setColor(Qt::black);
		}
		painter->setPen(borderPen);
		painter->setFont(nameFont());
		// ���Ʊ����
		QString  str = title();
		QSize size = computeText(str, painter->font());
		QRectF	headRect = QRectF(0, 0, size.width() + 16, SEQUENCE_TITLEHEIGHT - 2);
		painter->fillRect(headRect, Qt::gray);
		painter->drawRect(headRect);
		// ����������
		painter->drawText(4, headRect.bottom() - (headRect.height() - painter->font().pixelSize()) / 2, str);

		// ����֡��
		QRectF miRect(headRect.width() + 2, 0, 38, SEQUENCE_TITLEHEIGHT - 2);
		painter->fillRect(miRect, Qt::red);
		painter->drawRect(miRect);
		// ��������
		str = QString::number(frames_.size());
		size = computeText(str, painter->font());
		painter->drawText(miRect.left() + (miRect.width() - size.width()) / 2, miRect.bottom() - (miRect.height() - painter->font().pixelSize()) / 2, str);

		// ���������
		QRectF bodyRect(0, SEQUENCE_TITLEHEIGHT, imgBound_.width() + 2 * SEQUENCE_MARGIN, boundingBox_.height() - SEQUENCE_TITLEHEIGHT);
		painter->fillRect(bodyRect, Qt::darkGray);
		painter->drawRect(bodyRect);

		// ����֡��
		QRectF frameRect(bodyRect.right(), bodyRect.top() + SEQUENCE_MARGIN, boundingBox_.width() - bodyRect.width(), bodyRect.height() - SEQUENCE_MARGIN);
		painter->fillRect(frameRect, Qt::gray);
		painter->drawRect(frameRect);

		// ����ê����
		drawAnchor(painter);

		// ���Ʋ�����
		if (insertAnchor_ >= 0){
			if (frames_.size() == 0){
				Q_ASSERT(insertAnchor_ == 0);
				frameRect.adjust(SEQUENCE_MARGIN, SEQUENCE_MARGIN, -SEQUENCE_MARGIN, -SEQUENCE_MARGIN);
				painter->fillRect(frameRect, Qt::magenta);
			}
			else{
				QRectF insertrect;
				if (insertAnchor_ == frames_.size()){
					insertrect.setLeft(frameRect.right() - 2 * SEQUENCE_MARGIN);
					insertrect.setTop(frameRect.top());
					insertrect.setWidth(SEQUENCE_MARGIN);
					insertrect.setHeight(frameRect.height());
				}
				else{
					insertrect = frames_[insertAnchor_]->mapRectToParent(frames_[insertAnchor_]->boundingRect());
					insertrect.setLeft(insertrect.left() - SEQUENCE_MARGIN);
					insertrect.setTop(frameRect.top());
					insertrect.setWidth(SEQUENCE_MARGIN);
					insertrect.setHeight(frameRect.height());
				}
				insertrect.adjust(SEQUENCE_MARGIN / 4, SEQUENCE_MARGIN / 2, -SEQUENCE_MARGIN / 4, -SEQUENCE_MARGIN / 2);
				painter->fillRect(insertrect, Qt::magenta);
			}
		}

		painter->fillRect(imgBound_, Qt::black);

		// ���ƶ���֡
		if (currrentFrame_ >= 0 && currrentFrame_ < frames_.size()){
			// ���л���( Ĭ��ê��Ϊ���ĵ㣬��������ê���Զ���)

			QRect rect = frames_[currrentFrame_]->anchorRegion();
			QPointF pos = imgOrigin_ + rect.topLeft();
			// ����
			painter->drawImage(pos, frames_[currrentFrame_]->image());
		}

	}


	void  SpriteSequence::updateInstance(){
		if (instance_){
			instance_->pos_ = pos();
			// ����frames
			foreach(SpriteSequenceFrame* frame, frames_)
			{
				frame->updateInstance();
			}
		}
	}

	void	SpriteSequence::mouseDoubleClickEvent(QGraphicsSceneMouseEvent * event){
		if (event->button() == Qt::LeftButton){
			SpriteImageEditWindow w(instance_);
			w.exec();
			
			// ������ʾ
			adjust();
			update();

		}
		SpriteObject::mouseDoubleClickEvent(event);
	}

	void	SpriteSequence::dragEnterEvent(QGraphicsSceneDragDropEvent * event){

		if (event->mimeData()->hasFormat(MIME_ORION_TILE) || event->mimeData()->hasFormat(MIME_ORION_FRAME)){
			setSelected(true);
			update();
			event->accept();
		}
		else{
			event->ignore();
		}
	}

	void	SpriteSequence::dragLeaveEvent(QGraphicsSceneDragDropEvent * event){
		if (event->mimeData()->hasFormat(MIME_ORION_TILE) || event->mimeData()->hasFormat(MIME_ORION_FRAME)){
			setSelected(false);
			insertAnchor_ = -1;
			update();
			event->accept();
		}
		else{
			event->ignore();
		}
	}

	void	SpriteSequence::dragMoveEvent(QGraphicsSceneDragDropEvent * event){
		if (event->mimeData()->hasFormat(MIME_ORION_TILE) || event->mimeData()->hasFormat(MIME_ORION_FRAME) ) {
			insertAnchor_ = frames_.size();

			// ������ʾ����ָʾ
			if (frames_.size() > 0){  // ���û���Ӷ���	
				QPointF pos = event->pos();
				for (int i = 0; i < frames_.size(); i++){
					QRectF rect = frames_[i]->mapRectToParent(frames_[i]->boundingRect());
					if (pos.x() < rect.left()){
						insertAnchor_ = i;
						break;
					}
				}
			}
			update();
			event->accept();
		}
		else {
			event->ignore();
		}
	}

	void	SpriteSequence::dropEvent(QGraphicsSceneDragDropEvent * event){

		if (event->mimeData()->hasFormat(MIME_ORION_TILE) ) {
	
			QByteArray itemData = event->mimeData()->data(MIME_ORION_TILE);
			QDataStream stream(&itemData, QIODevice::ReadOnly);

			// ��ȡHostId
			int hostid;
			stream >> hostid;

			int tileNum;
			stream >> tileNum;

			SequenceFrame* frame = NULL;

			for (int i = 0; i < tileNum; i++){
				frame = new SequenceFrame();
				frame->pos = QPointF(0, 0);
				frame->imghost = hostid;
				frame->state.fill(false, 8);
				stream >> frame->imgindex;

				instance_->frames_.insert(insertAnchor_, frame);
				// ��ǰ������Ӷ���֡
				SpriteSequenceFrame* frameItem = new SpriteSequenceFrame(frame);
				// ����λ��
				addFrame(frameItem, insertAnchor_);
				frameItem->refreshImage();
			}

			// ��������frameλ��
			adjust();

			insertAnchor_ = -1;
			event->accept();
		}
		else if (event->mimeData()->hasFormat(MIME_ORION_FRAME)){
			QList<QGraphicsItem*> childrens = scene()->items();

			foreach(QGraphicsItem* item, childrens){
				if (item->type() == SPRITE_SEQUENCEFRAME && item->parentItem() == NULL ){
					addFrame((SpriteSequenceFrame*)item, insertAnchor_);
				}
			}

			// ��������frameλ��
			adjust();

			insertAnchor_ = -1;
			event->accept();
		}
		else {
			event->ignore();
		}

	}

	void  SpriteSequence::contextMenuEvent(QGraphicsSceneContextMenuEvent *event){

		setSelected(true);

		// �����Ҽ��˵�
		QMenu menu;
		QAction* act;

		// ��Ӳ˵�
		act = gMenuManager->getAction(ORION_SPRTIE_RENAMESEQ);
		menu.addAction(act);
		act = gMenuManager->getAction(ORION_SPRITE_DEFAULTSEQ);
		menu.addAction(act);
		menu.addSeparator();
		menu.addAction(gMenuManager->getAction(ORION_SPRTIE_DELSEQ));
		menu.exec(event->screenPos());
		event->accept();
	}

	void  SpriteSequence::addFrame(SpriteSequenceFrame* frame, int index){
		frame->setParentItem(this);
		frame->setVisible(true);

		if (index == -1){
			frames_.append(frame);
		}
		else{
			Q_ASSERT(index <= frames_.size());
			frames_.insert(index, frame);
		}
	}

	// �Ƴ���ǰ֡
	int SpriteSequence::removeFrame(SpriteSequenceFrame* frame){
		frame->setParentItem(NULL);
		frame->setVisible(false);

		int index = frames_.indexOf(frame);  // ���ں����ָ�
		frames_.removeOne(frame);
		// ���µ���
		adjust();

		return index;
	}

	void SpriteSequence::deleteFrame(SpriteSequenceFrame* frame){
		instance_->frames_.remove(frame->getFrame());

		delete frame;
		frames_.removeOne(frame);
		// ���µ���
		adjust();
	}

	QSize SpriteSequence::acquireFrameMaxSize(){
		QSize size(0, 0);
		foreach(SpriteSequenceFrame* f, frames_){
			QSize fs = f->imageSize();
			if (fs.width() > size.width()){
				size.setWidth(fs.width());
			}

			if (fs.height() > size.height()){
				size.setHeight(fs.height());
			}
		}
		return size;
	}

	void SpriteSequence::adjust(){

		if (frames_.size() == 0){
			boundingBox_.setRect(0, 0, SEQUENCE_MINW, SEQUENCE_MINH);
			imgBound_.setRect(SEQUENCE_MARGIN, SEQUENCE_TITLEHEIGHT + SEQUENCE_MARGIN, 128, 128);
			imgOrigin_ = imgBound_.center();
		}
		else{
			// ����bound��С
			int width = 2 * SEQUENCE_MARGIN;  // β����һ��
			int height = 2 * SEQUENCE_MARGIN + SEQUENCE_TITLEHEIGHT;
			int startX = 3 * SEQUENCE_MARGIN;
			// ����������ʾ����С
			QRect displaySize;
			foreach(SpriteSequenceFrame* f, frames_){
				f->refreshImage();
				width += f->imageSize().width() + SEQUENCE_MARGIN;
				if (displaySize.isEmpty()){
					displaySize = f->anchorRegion();
				}
				else{
					displaySize = displaySize.united(f->anchorRegion());
				}
			}
			width += displaySize.width() + 2 * SEQUENCE_MARGIN;
			height += displaySize.height();
			startX += displaySize.width();

			prepareGeometryChange();
			boundingBox_.setRect(0, 0, width, height);
			imgBound_.setRect(SEQUENCE_MARGIN, SEQUENCE_TITLEHEIGHT + SEQUENCE_MARGIN, displaySize.width(), displaySize.height());

			// �����������ĵ�
			imgOrigin_ = imgBound_.topLeft() - displaySize.topLeft();

			// ��������֡λ��
			for (int i = 0; i < frames_.size(); i++){
				QSizeF fs = frames_[i]->boundingRect().size();
				frames_[i]->setPos(startX, (height + SEQUENCE_TITLEHEIGHT + SEQUENCE_MARGIN - fs.height()) / 2.0);
				startX += (fs.width() + SEQUENCE_MARGIN);
			}
		}

		anchorBounds_.clear();
		qreal offx = imgBound_.width() + 2 * SEQUENCE_MARGIN;
		// ����anchor����λ��
		foreach(QString anchor, instance_->anchors_){
			QSize s = computeText(anchor, nameFont());
			anchorBounds_.insert(hashString(anchor), QRectF(offx, SEQUENCE_TITLEHEIGHT, s.width() + SEQUENCE_MARGIN, SEQUENCE_MARGIN));
			offx += s.width() + SEQUENCE_MARGIN;
		}
	
		if (timeLine_){
			// ���¶���
			if (frames_.size() == 0){
				timeLine_->stop();
			}

			int duration = qRound(FRAME_TIME * frames_.size() / instance_->speed_);
			timeLine_->setDuration(duration > 0 ? duration : 1000 );
			timeLine_->setFrameRange(0, frames_.size());
			timeLine_->setStartFrame(0);
		}

		// �������λ��
		if (isSelected()){
			foreach(QGraphicsItem* item, childItems()){
				if (item->type() == SPRITE_ANIMATINCONTROL ){
					item->setPos(boundingBox_.bottomLeft());
					((SpriteAnimationControl*)item)->bind(this);  // �������״̬
				}
			}
		}

		update();
	}
	
	QVariant SpriteSequence::itemChange(GraphicsItemChange change, const QVariant & value){

		if (change == QGraphicsItem::ItemSelectedHasChanged && scene() ){  // ����Ը�����

			if (value.toBool()){  // �����ѡ�У����������ͬ�����е�֡����		
				QList<QGraphicsItem*> selitems = scene()->selectedItems();
				foreach(QGraphicsItem* item, selitems){
					if (item->type() == SPRITE_SEQUENCEFRAME){
						item->setSelected(false);
					}
				}
			}

			// ֪ͨ��������
			((SpriteScene*)scene())->bindAnimControl(value.toBool() == true ? this : NULL );
			return value;
		}

		return SpriteObject::itemChange(change, value);
	}

	void SpriteSequence::setPlaySpeed(qreal speed){
		instance_->speed_ = speed;

		int duration = qRound(FRAME_TIME * frames_.size() / instance_->speed_);
		timeLine_->setDuration(duration > 0 ? duration : 1000);

	}

	void  SpriteSequence::setPlayBack(bool flag ){
		instance_->playback_ = flag;
		timeLine_->setDirection(instance_->playback_ ? QTimeLine::Backward : QTimeLine::Forward);
	}


	void SpriteSequence::playAnim(bool enable){
		Q_ASSERT(instance_ != NULL);

		// ���ö���
		if (timeLine_ != NULL){
			
			if (enable ){
				if (timeLine_->state() == QTimeLine::NotRunning){  // ����
					currrentFrame_ = frames_.size() > 0 ? 0 : -1;
					timeLine_->start();
				}
				else if (timeLine_->state() == QTimeLine::Paused){ // ����
					timeLine_->setPaused(false);
				}
			}
			else{
				if (timeLine_->state() == QTimeLine::Running){  // ��ͣ
					timeLine_->setPaused(true);
				}
			}
		}
	}

	void SpriteSequence::resetAnim(){
		Q_ASSERT(instance_ != NULL);

		if (timeLine_ != NULL){
			timeLine_->stop();
			int duration = qRound(FRAME_TIME * frames_.size() / instance_->speed_);
			timeLine_->setDuration(duration > 0 ? duration : 1000);
			timeLine_->setDirection(instance_->playback_ ? QTimeLine::Backward : QTimeLine::Forward);
			timeLine_->setFrameRange(0, frames_.size());
			timeLine_->setStartFrame(0);
			currrentFrame_ = -1;
		}
	}

	void SpriteSequence::initAnimation(){

		Q_ASSERT(instance_ != NULL);

		if (timeLine_ == NULL){
			timeLine_ = new QTimeLine( FRAME_TIME, this);

			int duration = qRound(FRAME_TIME * frames_.size() / instance_->speed_);
			timeLine_->setDuration(duration > 0 ? duration : 1000);

			timeLine_->setFrameRange(0, frames_.size());
			timeLine_->setCurveShape(QTimeLine::LinearCurve);
			timeLine_->setLoopCount(0);
			timeLine_->setDirection( instance_->playback_ ? QTimeLine::Backward : QTimeLine::Forward);
			timeLine_->setStartFrame(0);
			connect( timeLine_, SIGNAL(frameChanged(int)), this, SLOT(setValue(int)));
		}
	}

	void SpriteSequence::setValue(int value){
		currrentFrame_ = value;
		update();
	}

	bool SpriteSequence::isPlaying(){
		if (timeLine_ == NULL)
			return false;

		return (timeLine_->state() == QTimeLine::Running);
	}

	void SpriteSequence::setPingPong(bool flag ){
		instance_->pingpong_ = flag;
	}

	void SpriteSequence::setRepeat(bool flag ){
		instance_->repeat_ = flag;
	}

	void SpriteSequence::setRepeatCount(int count ){
		instance_->count_ = count;
	}

	void SpriteSequence::setRepeatTo(int index ){
		instance_->repeatTo_ = index;
	}

	bool SpriteSequence::isPausing(){
		if (timeLine_ == NULL)
			return false;

		return (timeLine_->state() == QTimeLine::Paused);
	}

	bool SpriteSequence::isPlayBack(){
		return instance_->playback_;
	}

	qreal SpriteSequence::playSpeed(){
		return instance_->speed_;
	}


	bool SpriteSequence::isPingpong(){
		return instance_->pingpong_;
	}

	bool SpriteSequence::isRepeat(){
		return instance_->repeat_;
	}

	int SpriteSequence::repeatCount(){
		return instance_->count_;
	}

	int SpriteSequence::repeatTo(){
		return instance_->repeatTo_;
	}

	void SpriteSequence::initFrames(){
		
		foreach(SequenceFrame* frame, instance_->frames_.values()){
			instance_->refreshImage(frame);

			SpriteSequenceFrame* frameItem = new SpriteSequenceFrame(frame);
			addFrame(frameItem);
			frameItem->refreshImage();
		}

	}

	// ����ê��
	void SpriteSequence::drawAnchor(QPainter* painter){

		foreach(QString anchor,  instance_->anchors_){
			ResourceHash key = hashString(anchor);
			QRectF bound = anchorBounds_[key];

			painter->fillRect(bound, currentAnchor_ == key ? Qt::red : Qt::green);
			painter->setPen(isSelected() ? Qt::yellow : Qt::black);
			painter->drawRect(bound);

			// �������ֱ�ʾ��Ce��
			painter->setPen(currentAnchor_ == key ? Qt::yellow : Qt::black);
			painter->setFont(nameFont());
			// ����ID
			painter->drawText(bound, Qt::AlignCenter, anchor);
		}

	}

	void SpriteSequence::mousePressEvent(QGraphicsSceneMouseEvent * event){
		if (event->button() == Qt::LeftButton){
			// �ж��Ƿ�����ê������
			foreach(ResourceHash key, anchorBounds_.keys()){
				QRectF bound = anchorBounds_[key];
				if (bound.contains(event->pos())){
					if (currentAnchor_ == key){
						currentAnchor_ = 0;
					}
					else{
						currentAnchor_ = key;
					}
				}
				update();
			}
		}
		SpriteObject::mousePressEvent(event);
	}

	void SpriteSequence::refreshFrames(){
		QList<SpriteSequenceFrame*> deleteFrames;
		foreach(SpriteSequenceFrame* f, frames_){
			f->refreshImage();
			// ���û��ͼ����ɾ��
			if (!f->isValidImage()){
				deleteFrames.append(f);
			}
		}

		foreach( SpriteSequenceFrame* f, deleteFrames){
			deleteFrame(f);
		}
		adjust();
		update();
	}