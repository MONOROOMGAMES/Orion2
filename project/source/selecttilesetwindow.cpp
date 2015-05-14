#include "selecttilesetwindow.h"
#include "ui_selecttilesetwindow.h"
#include <QFileDialog>
#include "commondefine.h"
#include "mergeimagewindow.h"
#include "splitimagewindow.h"
#include "menumanager.h"
#include "tooldefine.h"

SelectTilesetWindow::SelectTilesetWindow(ResourceHash id, QWidget *parent) :
QDialog(parent), replaceId_(id),
ui(new Ui::SelectTilesetWindow)
{
	ui->setupUi(this);
	setFixedSize(size());

	Qt::WindowFlags oldflag = windowFlags();
	oldflag &= !Qt::WindowContextHelpButtonHint;
	setWindowFlags(oldflag);

	initFromDb();

	BEGIN_ACTION();
	CONNECT_ACTION(ORION_GALLERY_MERGE, this, mergeImage);
	CONNECT_ACTION(ORION_GALLERY_SPLIT, this, splitImage);

	END_ACTION();

	ui->treeWidget->setCurrentItem(unknowCategoryItem_);
}

SelectTilesetWindow::~SelectTilesetWindow()
{
	BEGIN_ACTION();
	DISCONNECT_ACTION(ORION_GALLERY_MERGE);
	END_ACTION();

	delete ui;
}


void SelectTilesetWindow::initFromDb(void){

	unknowCategoryItem_ = new QTreeWidgetItem(ITEM_UNCLASS);
	unknowCategoryItem_->setText(0, UDQ_TR("δ����"));
	unknowCategoryItem_->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
	ui->treeWidget->addTopLevelItem(unknowCategoryItem_);

	QStringList tagList;

	// �����б�
	for (TResourceObjectIterator<ImageResourceObject> it; it; ++it){
		ImageResourceObject* gallery = (ImageResourceObject*)(*it);

		if (gallery->flags() & URF_TagGarbage)
			continue;

		// ����������Ŀ
		foreach(QString cate, gallery->categories_){
			if (cate.isEmpty() || categoryPaths_.contains(cate))
				continue;

			categoryPaths_.append(cate);

			QTreeWidgetItem* parent = findTreePath(cate, ui->treeWidget);
			Q_ASSERT(parent != NULL);

		}

		tagList << gallery->tags_;
	}

	// �Ƴ��ظ���
	tagList.removeDuplicates();
	ui->treeWidget->expandAll();

}

QTreeWidgetItem* SelectTilesetWindow::findTreePath(const QString& path, QTreeWidget* tree){
	QTreeWidgetItem* parent = NULL;
	QTreeWidgetItem* child = NULL;

	// CateGory·����ʽΪ��A.B.C��
	// ���ν���·���е�Item��
	if (path.isEmpty()){
		return tree->topLevelItem(0);  // Ĭ��δ����
	}

	QStringList entries = path.split(UDQ_T("."));
	foreach(QString entry, entries){
		Q_ASSERT(entry.size() > 0);

		if (parent == NULL){
			for (int i = 0; i < tree->topLevelItemCount(); i++){
				QTreeWidgetItem* item = tree->topLevelItem(i);
				if (item->type() != ITEM_CATEGORY || item->text(0) != entry){
					continue;
				}
				parent = item;
				break;
			}

			if (parent == NULL){
				// ��������
				parent = new QTreeWidgetItem(ITEM_CATEGORY);
				parent->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
				parent->setText(0, entry);
				tree->addTopLevelItem(parent);
			}
			continue;
		}

		for (int i = 0; i < parent->childCount(); i++){
			QTreeWidgetItem* item = parent->child(i);
			if (item->type() != ITEM_CATEGORY || item->text(0) != entry){
				continue;
			}
			child = item;
			break;
		}

		if (child == NULL){
			// ��������
			child = new QTreeWidgetItem(ITEM_CATEGORY);
			child->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
			child->setText(0, entry);
			parent->addChild(child);
		}
		parent = child;
		child = NULL;
	}

	return parent;

}

void SelectTilesetWindow::on_treeWidget_itemSelectionChanged(){

	QTreeWidgetItem* item = ui->treeWidget->currentItem();
	if (item == NULL){
		ui->listWidget->clear();
		return;
	}

	updateCurrentCategory();

}


void SelectTilesetWindow::updateCurrentCategory(){

	QString itemPath;
	QTreeWidgetItem* item = ui->treeWidget->currentItem();

	if (item != NULL && item->type() != ITEM_UNCLASS){
		QTreeWidgetItem* current = item;
		while (item != NULL && item->type() == ITEM_CATEGORY){
			itemPath.push_front(item->text(0));

			item = item->parent();
			if (item && item->type() == ITEM_CATEGORY){
				itemPath.push_front(UDQ_T("."));
			}
		}
	}

	updateCategory(itemPath);
}

void SelectTilesetWindow::updateCategory(const QString& cate, const QString& tag){
	ui->listWidget->clear();

	// �����б�
	for (TResourceObjectIterator<ImageResourceObject> it; it; ++it){
		ImageResourceObject* gallery = (ImageResourceObject*)(*it);

		if (gallery->flags() & URF_TagGarbage)
			continue;

		if (gallery->isBelong(cate)){
			//ƥ������
			if (!UIsExactKindOf(ImageResourceObject, gallery)){
				continue;
			}

			// �ϸ�ƥ���ǩ
			if (!tag.isEmpty() && !gallery->tags_.contains(tag)){
				continue;
			}

			addGalleryItemToList(gallery);
		}

	}

}


// ���б������
void SelectTilesetWindow::addGalleryItemToList(ImageResourceObject* gallery){

	QListWidgetItem* item = new QListWidgetItem(ui->listWidget, ITEM_GALLERY);
	item->setIcon(gallery->thumbnail_);
	item->setData(ITEM_DESC, gallery->objectName());
	item->setData(ITEM_HASHKEY, gallery->hashKey());
	item->setData(ITEM_CONTEXT, gallery->image_.size());


}

void SelectTilesetWindow::on_savepushButton_clicked(){
	QList<QListWidgetItem*> items = ui->listWidget->selectedItems();
	if (items.size() == 0){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("��ѡ��һ���ز�"));
		return;
	}

	if (replaceId_ > 0 && items.size() != 1 ){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("ֻ��ѡ��һ���زĽ����滻"));
		return;
	}

	foreach(QListWidgetItem* item, items){

		ResourceHash key = item->data(ITEM_HASHKEY).toUInt();
		emit addGallery(key, replaceId_ );
	}

	if (replaceId_ > 0)
		accept();
}

void SelectTilesetWindow::on_cancelpushButton_clicked(){
	if (replaceId_ == 0)
		accept();
	else
		reject();
}

void SelectTilesetWindow::on_listWidget_itemSelectionChanged(){

	QListWidgetItem* item = ui->listWidget->currentItem();

	ResourceHash key = item->data(ITEM_HASHKEY).toUInt();
	DEFINE_RESOURCE_HASHKEY_VALID(ImageResourceObject, img, key);

	ui->prevwidget->showPicture(img->content_);
	if (img->isGroup_){
		ui->prevwidget->showFrames(img->frames_);
	}
	else{
		ui->prevwidget->hideFrames();
	}

	// ��ʾ��Ϣ
	QSize imgsize = ui->prevwidget->getPicture().size();
	QString info = UDQ_TR("%1: ͼ���С %2x%3")
		.arg(img->objectName())
		.arg(imgsize.width())
		.arg(imgsize.height());
	ui->infolabel->setText(info);

}


void SelectTilesetWindow::mergeImage(){
	QList<QListWidgetItem*> items = ui->listWidget->selectedItems();
	Q_ASSERT(items.size() > 1);

	QList<ResourceHash> keys;
	foreach(QListWidgetItem* item, items){
		keys.append(item->data(ITEM_HASHKEY).toUInt());
	}

	MergeImageWindow w(keys);
	if (w.exec() == QDialog::Accepted){
		//  ˢ���б�
		updateCurrentCategory();
	}

}

void SelectTilesetWindow::splitImage(){
	QList<QListWidgetItem*> items = ui->listWidget->selectedItems();
	Q_ASSERT(items.size() == 1);

	// ��ȡ��һ��ͼ��
	ResourceHash imgkey = items[0]->data(ITEM_HASHKEY).toUInt();
	DEFINE_RESOURCE_HASHKEY(ImageResourceObject, img, imgkey);
	if (img == NULL){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("��ѡ��һ�����ʵ�ͼ����Դ!"));
		return;
	}

	QSize imgsize = img->image_.size();
	if (imgsize.width() < 8 || imgsize.height() < 8){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("ѡ���ͼ��ߴ�̫С���޷��ָ�!"));
		return;
	}

	SplitImageWindow w(imgkey);
	w.exec();
}