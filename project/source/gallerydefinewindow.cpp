/************************************************************************
**
**  Copyright (C) 2014  guobin  <guobin.gb@alibaba-inc.com>
**
**  This file is part of Orion.
**
**  Orion is free software: you can redistribute it and/or modify
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

#include "gallerydefinewindow.h"
#include "ui_gallerydefinewindow.h"
#include "addpicturewindow.h"
#include "addaudiowindow.h"
#include "addvideowindow.h"
#include "importgallerywindow.h"
#include "mergeimagewindow.h"
#include "resourcereferencecheckwindow.h"
#include "splitimagewindow.h"
#include <QFileDialog>
#include <QColorDialog>
#include "commondefine.h"
#include "tooldefine.h"
#include "menumanager.h"
#include "imagehelper.h"
#include "resourcertti.h"
using namespace ORION;



GalleryDefineWindow::GalleryDefineWindow(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::GalleryDefineWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	Qt::WindowFlags oldflag = windowFlags();
	oldflag &= !Qt::WindowContextHelpButtonHint;
	setWindowFlags(oldflag);

	initFromDb();

	filter_ = FILTER_ALL;
	
	connect(ui->treeWidget, SIGNAL(galleryCategoryChanged()), this, SLOT(updateCurrentCategory()));
	connect(ui->listWidget, SIGNAL(removeFromCategory(QListWidgetItem*)), this, SLOT(removeGalleryCategory(QListWidgetItem*)));

	BEGIN_ACTION();
	CONNECT_ACTION(ORION_GALLERY_ADDCATEGORY, this, addGalleryCategory);
	CONNECT_ACTION(ORION_GALLERY_DELETECATEGORY, this, deleteGalleryCategory);
	CONNECT_ACTION(ORION_GALLERY_MERGE, this, mergeImage);
	CONNECT_ACTION(ORION_GALLERY_TRANSPARENT, this, transparentImageBG);
	CONNECT_ACTION(ORION_GALLERY_COPY, this, copyGallery);
	CONNECT_ACTION(ORION_GALLERY_SPLIT, this, splitImage);
	END_ACTION();

	ui->treeWidget->setCurrentItem(unknowCategoryItem_);
	modified_ = false;
}

GalleryDefineWindow::~GalleryDefineWindow()
{

	BEGIN_ACTION();
	DISCONNECT_ACTION(ORION_GALLERY_ADDCATEGORY);
	DISCONNECT_ACTION(ORION_GALLERY_DELETECATEGORY);
	DISCONNECT_ACTION(ORION_GALLERY_MERGE);
	END_ACTION();

    delete ui;
}

void GalleryDefineWindow::initFromDb(void){

	unknowCategoryItem_ = new QTreeWidgetItem(ITEM_UNCLASS);
	unknowCategoryItem_->setText(0, UDQ_TR("δ����"));
	unknowCategoryItem_->setFlags(Qt::ItemIsSelectable  | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
	ui->treeWidget->addTopLevelItem(unknowCategoryItem_);

	QStringList tagList;

	// �����б�
	for (TResourceObjectIterator<GalleryResourceObject> it; it; ++it){
		GalleryResourceObject* gallery = (GalleryResourceObject*)(*it);

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

}

QTreeWidgetItem* GalleryDefineWindow::findTreePath(const QString& path, QTreeWidget* tree){
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
				if (item->type() != ITEM_CATEGORY || item->text(0) != entry ){
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


void GalleryDefineWindow::on_cancelpushButton_clicked(){

	if (modified_){
		if (QMessageBox::question(this, windowTitle(), UDQ_TR("�Ƿ񱣴���زĿ���޸�?")) == QMessageBox::Yes){
			on_savepushButton_clicked();
		}
	}

	reject();
}

void GalleryDefineWindow::on_savepushButton_clicked(){

	// ɾ���ز�
	foreach(GalleryResourceObject* gallery, deletedGalleries_){
		gallery->setFlags(URF_TagGarbage);
	}

	// �������п�
	ApplyResourceLinkers<ImageResourceObject>();
	ApplyResourceLinkers<AudioResourceObject>();
	ApplyResourceLinkers<VideoResourceObject>();
	modified_ = false;
	// 
	QMessageBox::information(this, windowTitle(), UDQ_TR("�ز����ݱ������!"));
}

void GalleryDefineWindow::on_deletepushButton_clicked(){

	// ɾ����ǰѡ�����Դ
	QListWidgetItem* item = ui->listWidget->currentItem();

	if (item == NULL){
		QMessageBox::warning(this, UDQ_TR("ɾ����Դ"), UDQ_TR("��ѡ��һ��Ҫɾ������Դ!"));
		return;
	}

	// �����Դ�����������������ã�����ɾ����
	// �����ǹ�������( Ŀǰֻ���Ƕ�������������ӰӦ��)
	// ������������ɾ����Դ�����ṩ�ݴ���
	// ���ù����Լ��
	ResourceHash key = item->data(ITEM_HASHKEY).toUInt();
	ResourceReferenceCheckWindow w(key);

	if (w.isUsed()){
		w.exec();  // ��ʾ������Ϣ,  �޷�ɾ��
	}
	else{
		if (QMessageBox::question(this, windowTitle(), UDQ_TR("ȷ��ɾ��ѡ�е��ز�?")) == QMessageBox::No){
			return;
		}

		DEFINE_RESOURCE_HASHKEY_VALID(GalleryResourceObject, gallery, key);
		ui->listWidget->takeItem(ui->listWidget->row(item));
		delete item;

		modified_ = true;
		deletedGalleries_.append(gallery);
	}

}

void GalleryDefineWindow::on_addpushButton_clicked(){
	// ���ļ�
	QString filepath = QFileDialog::getOpenFileName(this, UDQ_TR("���ļ������ز�"), QString(),
		GalleryResourceObject::fileFilters() );

	if (filepath.isEmpty())
		return;

	// �����Դ����
	QFileInfo fileInfo(filepath);
	QString extName = fileInfo.suffix();

	GalleryResourceObject* gallery = NULL;
	if (ImageResourceObject::isValidFile(extName)){
		// ���ͼƬ�ز�
		AddPictureWindow w(filepath, categoryPaths_, this);
		if (w.exec() == QDialog::Accepted){
			gallery = w.newImage_;
		}
	}
	else if (AudioResourceObject::isValidFile(extName)){
		// �����Ƶ�ز�
		AddAudioWindow w(filepath, categoryPaths_, this);
		if (w.exec() == QDialog::Accepted){
			gallery = w.newAudio_;
		}
	}
	else if (VideoResourceObject::isValidFile(extName)){
		// �����Ƶ�ز�
		AddVideoWindow w(filepath, categoryPaths_, this);
		if (w.exec() == QDialog::Accepted){
			gallery = w.newVideo_;
		}
	}

	// �����б���ʾ
	if (gallery != NULL){
		// 
		QString currentCategory = ui->treeWidget->getCurrentItemPath();
		if (gallery->isBelong(currentCategory)){
			// �������
			insertGalleryItemToList(gallery);
		}
		modified_ = true;
	}

	return;
}

void GalleryDefineWindow::on_exportpushButton_clicked(){
	static QString title = UDQ_TR("�����ز�");
	// ����Դ����������
	QList<QListWidgetItem*> selitems = ui->listWidget->selectedItems();
	if (selitems.size() == 0){
		QMessageBox::warning(this, title, UDQ_TR("��ѡ��һ��Ҫ��������Դ!"));
		return;
	}

	QString dirpath = QFileDialog::getExistingDirectory(this, title);
	if (dirpath.isEmpty()){
		return;
	}

	foreach(QListWidgetItem* item, selitems){

		ResourceHash key = item->data(ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY_VALID(GalleryResourceObject, gallery, key);

		QString filepath = QString(UDQ_T("%1/%2.%3")).arg(dirpath).arg(gallery->objectName()).arg(gallery->getFormat());

		// �ж�ѡ�����Դ����
		if (UIsExactKindOf(ImageResourceObject, gallery)){

			ImageResourceObject* image = UDynamicCast(ImageResourceObject, gallery);
			if (image->format_ == UDQ_T("jpg")){
				image->image_.save(filepath, "JPG");
			}
			else if (image->format_ == UDQ_T("bmp")){
				image->image_.save(filepath, "BMP");
			}
			else if (image->format_ == UDQ_T("png")){
				image->image_.save(filepath, "PNG");
			}
		}
		else if (UIsExactKindOf(AudioResourceObject, gallery)){

			AudioResourceObject* audio = UDynamicCast(AudioResourceObject, gallery);
			QFile file(filepath);
			if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
				QMessageBox::warning(this, title, UDQ_TR("��Ƶ�ļ�[%1]����ʧ��!").arg(filepath));
			}
			file.write(audio->content_);
			file.close();

		}
		else if (UIsExactKindOf(VideoResourceObject, gallery)){
			VideoResourceObject* video = UDynamicCast(VideoResourceObject, gallery);
			QFile file(filepath);
			if (!file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
				QMessageBox::warning(this, title, UDQ_TR("��Ƶ�ļ�[%1]����ʧ��!").arg(filepath));
			}
			file.write(video->content_);
			file.close();
		}
	}
	
	QMessageBox::information(this, title, UDQ_TR("�زĵ������!"));
}


void GalleryDefineWindow::insertGalleryItemToList(GalleryResourceObject* gallery){
	QListWidgetItem* item = NULL;
	// ����Ƿ��Ѿ�����
	for (int i = 0; i < ui->listWidget->count(); i++){
		if (ui->listWidget->item(i)->text() == gallery->objectName()){
			item = ui->listWidget->item(i);
			break;
		}
	}

	if (item == NULL){
		addGalleryItemToList(gallery);
	}
	else{
		// ����Item
		updateGalleryItem(item, gallery);
	}

}

// ���б������
void GalleryDefineWindow::addGalleryItemToList(GalleryResourceObject* gallery){
	QListWidgetItem* item = new QListWidgetItem(ui->listWidget, ITEM_GALLERY);
	updateGalleryItem(item, gallery);
}

void GalleryDefineWindow::updateGalleryItem(QListWidgetItem* item, GalleryResourceObject* gallery){
	if (UIsExactKindOf(ImageResourceObject, gallery)){
		item->setIcon(gallery->thumbnail_);
	}
	else if (UIsExactKindOf(AudioResourceObject, gallery)){
		item->setIcon(QIcon(UDQ_T(":/images/gallery_audio.png")));
	}
	else if (UIsExactKindOf(VideoResourceObject, gallery)){
		item->setIcon(QIcon(UDQ_T(":/images/gallery_video.png")));
	}
	item->setText(gallery->objectName());
	item->setData(ITEM_HASHKEY, gallery->hashKey());
}


void GalleryDefineWindow::on_treeWidget_itemSelectionChanged(){

	QTreeWidgetItem* item = ui->treeWidget->currentItem();
	if (item == NULL){
		ui->listWidget->clear();
		return;
	}

	updateCurrentCategory();

}

void GalleryDefineWindow::updateCategory(const QString& cate, const QString& tag ){
	ui->listWidget->clear();

	// �����б�
	for (TResourceObjectIterator<GalleryResourceObject> it; it; ++it){
		GalleryResourceObject* gallery = (GalleryResourceObject*)(*it);

		if (gallery->flags() & URF_TagGarbage)
			continue;

		if (gallery->isBelong(cate)){

			if (filter_ != FILTER_ALL){
				//ƥ������
				if (!(filter_ & FILTER_IMAGE) && UIsExactKindOf(ImageResourceObject, gallery)){
					continue;
				}

				if (!(filter_ & FILTER_AUDIO) && UIsExactKindOf(AudioResourceObject, gallery)){
					continue;
				}

				if (!(filter_ & FILTER_VIDEO) && UIsExactKindOf(VideoResourceObject, gallery)){
					continue;
				}
			}

			// �ϸ�ƥ���ǩ ������
			if (!tag.isEmpty() && !gallery->tags_.contains(tag) && !gallery->objectName().contains(tag, Qt::CaseInsensitive) ){
				continue;
			}

			addGalleryItemToList(gallery);
		}

	}
}


void GalleryDefineWindow::on_imagecheckBox_stateChanged(int state){
	if (state == Qt::Checked){
		// ���¸���
		filter_ |= FILTER_IMAGE;
	}
	else{
		filter_ &= ~FILTER_IMAGE;
	}

	updateCurrentCategory();
}

void GalleryDefineWindow::on_audiocheckBox_stateChanged(int state){
	if (state == Qt::Checked){
		// ���¸���
		filter_ |= FILTER_AUDIO;
	}
	else{
		filter_ &= ~FILTER_AUDIO;
	}

	updateCurrentCategory();
}

void GalleryDefineWindow::on_videocheckBox_stateChanged(int state){
	if (state == Qt::Checked){
		// ���¸���
		filter_ |= FILTER_VIDEO;
	}
	else{
		filter_ &= ~FILTER_VIDEO;
	}

	updateCurrentCategory();
}


void GalleryDefineWindow::on_taglineEdit_textEdited(const QString & text){
	updateCurrentCategory();
}


void GalleryDefineWindow::addGalleryCategory(){
	//
	QTreeWidgetItem* item = ui->treeWidget->currentItem();

	QTreeWidgetItem* newItem = new QTreeWidgetItem(ITEM_CATEGORY);
	newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable  | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);

	if (item == NULL || item->type() == ITEM_UNCLASS){
		newItem->setText(0, ui->treeWidget->makeUniqueChildName(NULL, UDQ_TR("����Ŀ")));
		ui->treeWidget->addTopLevelItem(newItem);
	}
	else if (item->type() == ITEM_CATEGORY){
		newItem->setText(0, ui->treeWidget->makeUniqueChildName(item, UDQ_TR("����Ŀ")));
		item->addChild(newItem);
		ui->treeWidget->expandItem(item);
	}
	else{
		delete newItem;
		Q_UNREACHABLE();
	}

	// ��������

}

void GalleryDefineWindow::deleteGalleryCategory(){
	QTreeWidgetItem* item = ui->treeWidget->currentItem();

	if (item == NULL || item->type() == ITEM_UNCLASS )
		return;

	QString path = ui->treeWidget->getItemPath(item);
	// ���������ز�
	for (TResourceObjectIterator<GalleryResourceObject> it; it; ++it){
		GalleryResourceObject* gallery = (GalleryResourceObject*)(*it);

		if (gallery->flags() & URF_TagGarbage)
			continue;

		// ����������Ŀ
		gallery->categories_.removeOne(item->text(0));
		gallery->setFlags(URF_TagSave);

		QStringList newCategories;
		foreach(QString cate, gallery->categories_){
			if (!(cate.isEmpty() || cate.startsWith(path))){
				newCategories.append(cate);
			}
		}
		gallery->categories_ = newCategories;
	}

	// ɾ������
	delete item;

}

void GalleryDefineWindow::updateCurrentCategory(){
	updateCategory(ui->treeWidget->getCurrentItemPath(), ui->taglineEdit->text());
}

void GalleryDefineWindow::on_treeWidget_itemChanged(QTreeWidgetItem * item, int column){
	if (item == NULL)
		return;

	Q_ASSERT(column == 0);

	// ����������Ƿ�Ϸ���������Ϸ�����ָ�ԭʼ����
	if (item->text(0) == oldItemName_)
		return;

	if (ui->treeWidget->findDuplications(item) > 1){
		item->setText(0, oldItemName_);
		return;
	}

	// �޸�����
	if (item->type() == ITEM_CATEGORY ){
		QString newPath = ui->treeWidget->getItemPath(item);
		// ���������ز�
		for (TResourceObjectIterator<GalleryResourceObject> it; it; ++it){
			GalleryResourceObject* gallery = (GalleryResourceObject*)(*it);

			if (gallery->flags() & URF_TagGarbage)
				continue;

			// ���µ�����Ŀ
			gallery->setFlags(URF_TagSave);

			QStringList newCategories;
			foreach(QString cate, gallery->categories_){
				if ( cate.startsWith(currentCategoryPath_)){
					cate.replace(0, currentCategoryPath_.size(),  newPath);
					newCategories.append(cate);
				}
				else{
					newCategories.append(cate);
				}
			}
			gallery->categories_ = newCategories;
		}

		updateCurrentCategory();
	}
}

void GalleryDefineWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem * item, int column){
	currentCategoryPath_ = ui->treeWidget->getCurrentItemPath();
	oldItemName_ = item->text(0);
}

void GalleryDefineWindow::removeGalleryCategory(QListWidgetItem* item){
	Q_ASSERT(item != NULL);

	if (ui->treeWidget->currentItem()->type() == ITEM_UNCLASS){
		return;
	}

	ResourceHash key = item->data(ITEM_HASHKEY).toUInt();
	DEFINE_RESOURCE_HASHKEY_VALID(GalleryResourceObject, gallery, key);

	gallery->categories_.removeOne(ui->treeWidget->getCurrentItemPath());
	gallery->setFlags(URF_TagSave);

	delete item;

}

void GalleryDefineWindow::on_importpushButton_clicked(){
	QTreeWidgetItem* item = ui->treeWidget->currentItem();
	QString cate;
	if (item){
		cate = ui->treeWidget->getItemPath(item);
	}
	ImportGalleryWindow w(cate);
	w.exec();

	//   ˢ�µ�ǰ��ʾ
	updateCurrentCategory();

}

void GalleryDefineWindow::mergeImage(){
	QList<QListWidgetItem*> items = ui->listWidget->selectedItems();

	if (items.size() < 2){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("����ѡ��2������ͼƬ���кϲ�!"));
		return;
	}

	QList<ResourceHash> keys;
	foreach(QListWidgetItem* item, items){
		ResourceHash key = item->data(ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY(ImageResourceObject, img, key);
		if (img){
			keys.append(key);
		}
	}

	MergeImageWindow w(keys);
	if (w.exec() == QDialog::Accepted){
		//  ˢ���б�
		updateCurrentCategory();
	}

}

void GalleryDefineWindow::transparentImageBG(){
	QList<QListWidgetItem*> items = ui->listWidget->selectedItems();
	Q_ASSERT(items.size() > 0);

	// ����ѡ����ɫ�Ի���
	QColor color = QColorDialog::getColor(Qt::white, this, windowTitle());
	if (color.isValid()){
		// �޸�ͼƬ����
		foreach(QListWidgetItem* item, items){
			ResourceHash key = item->data(ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY(ImageResourceObject, img, key);
			if (img){
				// ��ָ����ɫ�滻Ϊ͸��ɫ
				QImage newImage = transparentImage(img->image_, color);
				img->setImage(newImage);
				// �����б���ʾ
				item->setIcon(img->thumbnail_);
				img->setFlags(URF_TagSave);
				SAVE_PACKAGE_RESOURCE(ImageResourceObject, img);
			}
		}
	}
}

void GalleryDefineWindow::copyGallery(){
	QList<QListWidgetItem*> items = ui->listWidget->selectedItems();
	Q_ASSERT(items.size() > 0);

	ResourceObject* newGallery = NULL;
	foreach(QListWidgetItem* item, items){
		ResourceHash key = item->data(ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY_VALID(GalleryResourceObject, gallery, key);

		QString newName = ResourceObject::makeUniqueObjectName(gallery->outer(), gallery);

		// �ж�ѡ�����Դ����
		if (UIsExactKindOf(ImageResourceObject, gallery)){
			ImageResourceObject* image = UDynamicCast(ImageResourceObject, gallery);
			// ����
			ImageResourceObject* newGallery = TCloneObject<ImageResourceObject>(image, newName);
			// ���浽����
			newGallery->clearFlags(URF_TagGarbage);
			newGallery->setFlags(URF_TagSave);
			SAVE_PACKAGE_RESOURCE(ImageResourceObject, newGallery);
		}
		else if (UIsExactKindOf(AudioResourceObject, gallery)){

			AudioResourceObject* audio = UDynamicCast(AudioResourceObject, gallery);
			// ����
			AudioResourceObject* newGallery = TCloneObject<AudioResourceObject>(audio, newName);
			newGallery->clearFlags(URF_TagGarbage);
			newGallery->setFlags(URF_TagSave);
			SAVE_PACKAGE_RESOURCE(AudioResourceObject, newGallery);
		}
		else if (UIsExactKindOf(VideoResourceObject, gallery)){
			VideoResourceObject* video = UDynamicCast(VideoResourceObject, gallery);
			// ����
			VideoResourceObject* newGallery = TCloneObject<VideoResourceObject>(video, newName);
			newGallery->clearFlags(URF_TagGarbage);
			newGallery->setFlags(URF_TagSave);
			SAVE_PACKAGE_RESOURCE(VideoResourceObject, newGallery);
		}
	}

	//   ˢ�µ�ǰ��ʾ
	updateCurrentCategory();

}

void GalleryDefineWindow::closeEvent(QCloseEvent * event){
	// ��ʾ����
	if (modified_){
		if (QMessageBox::question(this, windowTitle(), UDQ_TR("�Ƿ񱣴���زĿ���޸�?")) == QMessageBox::Yes){
			on_savepushButton_clicked();
		}
	}
}

void GalleryDefineWindow::splitImage(){
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