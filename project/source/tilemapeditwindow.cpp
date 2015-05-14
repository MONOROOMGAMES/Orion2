#include "tilemapeditwindow.h"
#include "ui_tilemapeditwindow.h"
#include "tooldefine.h"

TileMapEditWindow::TileMapEditWindow(ObjectTypeResourceObject* obj, const QSize& grid, int index, QWidget *parent) :
QDialog(parent), objType_(obj),
    ui(new Ui::TileMapEditWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	scene_ = new ImageScene();
	ui->editorView->setScene(scene_);

	scene_->setMode(ImageScene::POM_COLLISION);
	scene_->setAnchorVisible(false);
	scene_->setCollisionVisible(true);

	// ����Tile��
	initTileBlocks(grid);

	if (index < 0 || index >= ui->listWidget->count()){
		index = 0;
	}
	ui->listWidget->setCurrentRow(index);
}

TileMapEditWindow::~TileMapEditWindow()
{
    delete ui;
}

void TileMapEditWindow::initTileBlocks(const QSize& size){
	gridsize_ = size;

	// ��ȡ��ײ����
	QDataStream stream(objType_->tilemapData_);
	stream >> collision_;

	QImage& img = objType_->staticImage_->image_;
	int col = img.width() / size.width();
	int row = img.height() / size.height();

	QImage block;
	for (int i = 0; i < row; i++){
		for (int j = 0; j < col; j++){
			QListWidgetItem* item = new QListWidgetItem();
			item->setText(QString::number(i*col+j));
			block = img.copy(j*size.width(), i*size.height(), size.width(), size.height());
			item->setIcon(QPixmap::fromImage(block));
			item->setData(ITEM_VALUE, QPoint(i, j));
			ui->listWidget->addItem(item);
		}
	}

}

void TileMapEditWindow::on_savepushButton_clicked(){

	// ���浱ǰ
	QListWidgetItem* item = ui->listWidget->currentItem();
	if (item){
		int index = item->text().toInt();
		collision_[index] = scene_->collisions();
	}

	// ���浱ǰ��ê�����ײ����
	objType_->tilemapData_.clear();
	QDataStream stream(&(objType_->tilemapData_), QIODevice::WriteOnly);
	stream << collision_;

}

void TileMapEditWindow::on_applypushButton_clicked(){

	QListWidgetItem* item = ui->listWidget->currentItem();
	if (item == NULL){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("����ѡ��һ����ͼ��!"));
		return;
	}

	// 
	if (QMessageBox::question(this, windowTitle(), UDQ_TR("ȷ�Ͻ���ǰ��ײ����Ӧ�õ�������ͼ���ϣ�")) == QMessageBox::No){
		return;
	}

	QList<QPointF> pts = scene_->collisions();
	collision_.clear();
	for (int i = 0; i < ui->listWidget->count(); i++){
		collision_[i] = pts;
	}

}

void TileMapEditWindow::on_listWidget_currentItemChanged(QListWidgetItem * current, QListWidgetItem * previous){

	// �л��򱣴�
	if (previous){
		int index = previous->text().toInt();
		collision_[index] = scene_->collisions();
	}

	scene_->clearImage();

	if (current){
		QPoint coord = current->data(ITEM_VALUE).toPoint();
		QImage block = objType_->staticImage_->image_.copy(coord.y()*gridsize_.width(), coord.x()*gridsize_.height(), gridsize_.width(), gridsize_.height());
		scene_->setImage(block, QBitArray());
		scene_->setAnchor(QPointF(0, 0));

		int index = current->text().toInt();

		QList<QPointF> pts;
		if (collision_.contains(index)){
			pts = collision_[index];
			Q_ASSERT(pts.size() >= 3);
			// ������ײ����
			scene_->setCollision(pts);
		}
		else{
			// Ĭ�Ͼ���
			scene_->setDefaultCollision();
		}
	}
}

void TileMapEditWindow::on_defaultpushButton_clicked(){
	scene_->setDefaultCollision();
}

void TileMapEditWindow::on_clearpushButton_clicked(){

	// 
	if (QMessageBox::question(this, windowTitle(), UDQ_TR("ȷ��ɾ�����е�ͼ�����ײ����")) == QMessageBox::No){
		return;
	}

	collision_.clear();

	QListWidgetItem* item = ui->listWidget->currentItem();
	if (item != NULL){
		scene_->setDefaultCollision();
	}

}