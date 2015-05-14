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
#include "gallerycategorywindow.h"
#include "ui_gallerycategorywindow.h"
#include "tooldefine.h"
#include "framemanager.h"
#include "mainmanagerframe.h"
#include <QSplitter>
#include <QFileDialog>
#include <QColorDialog>
#include "addpicturewindow.h"
#include "importgallerywindow.h"
#include "levelscene.h"
#include "imagehelper.h"
#include "menumanager.h"
#include "resourceobject.h"
#include "resourcelinker.h"
using namespace ORION;

GalleryCategoryWindow::GalleryCategoryWindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GalleryCategoryWindow)
{
    ui->setupUi(this);
	defaultIcon_ = QIcon(UDQ_T(":/images/folder.png"));

	QSplitter *splitter = new QSplitter(ui->scrollArea);
	galleryTree_ = new GalleryTreeWidget;
	galleryTree_->setDragDropMode(QAbstractItemView::DropOnly);

	previewList_ = new GalleryListWidget;
	previewList_->setAutoResize(true);
	previewList_->setResizeMode(QListView::Adjust);
	previewList_->setIconSize(QSize(48, 48));

	splitter->addWidget(galleryTree_);
	splitter->addWidget(previewList_);
	ui->scrollArea->setWidget(splitter);

	connect(galleryTree_, SIGNAL(itemSelectionChanged()), this, SLOT(treeItemSelectionChanged()));
	connect(galleryTree_, SIGNAL(galleryCategoryChanged()), this, SLOT(updateCurrentCategory()));
	connect(previewList_, SIGNAL(removeFromCategory(QListWidgetItem*)), this, SLOT(removeGalleryCategory(QListWidgetItem*)));

	BEGIN_ACTION();
	CONNECT_ACTION(ORION_GALLERY_ADDCATEGORY, this, addGalleryCategory);
	CONNECT_ACTION(ORION_GALLERY_DELETECATEGORY, this, deleteGalleryCategory);
	CONNECT_ACTION(ORION_GALLERY_TRANSPARENT, this, transparentImageBG);
	CONNECT_ACTION(ORION_GALLERY_COPY, this, copyGallery);
	END_ACTION();

	updateCategoryTree();
}

GalleryCategoryWindow::~GalleryCategoryWindow()
{
    delete ui;
}

void	GalleryCategoryWindow::resizeEvent(QResizeEvent * event){
	QSize newSize = event->size() - QSize(WIDGET_MARGIN * 2, WIDGET_MARGIN * 2);
	ui->layoutWidget->setGeometry(QRect(QPoint(WIDGET_MARGIN, WIDGET_MARGIN), newSize));
}

// ˢ��Ŀ¼��
void GalleryCategoryWindow::updateCategoryTree(){
	galleryTree_->clear();
	categoryPaths_.clear();

	unknowCategoryItem_ = new QTreeWidgetItem(ITEM_UNCLASS);
	unknowCategoryItem_->setText(0, UDQ_TR("δ����"));
	unknowCategoryItem_->setIcon(0, defaultIcon_);
	unknowCategoryItem_->setFlags(Qt::ItemIsSelectable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
	galleryTree_->addTopLevelItem(unknowCategoryItem_);

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

			QTreeWidgetItem* parent = findTreePath(cate, galleryTree_);
			Q_ASSERT(parent != NULL);

		}
	}

}

QTreeWidgetItem* GalleryCategoryWindow::findTreePath(const QString& path, QTreeWidget* tree){
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
				parent->setIcon(0, defaultIcon_);
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
			child->setIcon(0, defaultIcon_);
			child->setText(0, entry);
			parent->addChild(child);
		}
		parent = child;
		child = NULL;
	}

	return parent;
}

void GalleryCategoryWindow::treeItemSelectionChanged(){

	QTreeWidgetItem* item = galleryTree_->currentItem();
	if (item == NULL){
		previewList_->clear();
		return;
	}
	updateCategoryPreview(galleryTree_->getCurrentItemPath(), ui->filterineEdit->text());
}


void GalleryCategoryWindow::updateCategoryPreview(const QString& cate, const QString& tag){
	previewList_->clear();
	// �����б�
	for (TResourceObjectIterator<GalleryResourceObject> it; it; ++it){
		GalleryResourceObject* gallery = (GalleryResourceObject*)(*it);

		if (gallery->flags() & URF_TagGarbage)
			continue;

		if (gallery->isBelong(cate)){

			// �ϸ�ƥ���ǩ
			if (!tag.isEmpty() && !gallery->tags_.contains(tag) && !gallery->objectName().contains(tag, Qt::CaseInsensitive)){
				continue;
			}

			// Ŀǰ������ͼƬ
			if (UIsExactKindOf(ImageResourceObject, gallery)){
				QListWidgetItem* item = new QListWidgetItem(previewList_,ITEM_GALLERY);
				updateGalleryItem(item, gallery);
			}
		}
	}
}

void GalleryCategoryWindow::updateGalleryItem(QListWidgetItem* item, GalleryResourceObject* gallery){
	if (UIsExactKindOf(ImageResourceObject, gallery)){
		item->setIcon(gallery->thumbnail_.scaledToWidth(64));
	}
	else {
		Q_UNREACHABLE();
	}
	item->setText(gallery->objectName());
	item->setData(ITEM_HASHKEY, gallery->hashKey());
}


void GalleryCategoryWindow::on_filterineEdit_textEdited(const QString & text){

	QTreeWidgetItem* item = galleryTree_->currentItem();
	if (item == NULL){
		galleryTree_->clear();
		return;
	}

	updateCategoryPreview(galleryTree_->getCurrentItemPath(), text);

}


void GalleryCategoryWindow::on_addpushButton_clicked(){
	// ���ļ�
	QString filepath = QFileDialog::getOpenFileName(this, UDQ_TR("���ļ������ز�"), QString(),
		ImageResourceObject::fileFilter());

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
	
	// �����б���ʾ
	if (gallery != NULL){
		// 
		QString currentCategory = galleryTree_->getCurrentItemPath();
		if (gallery->isBelong(currentCategory)){
			// �������
			insertGalleryItemToList(gallery);
		}

		// ����
		gallery->setFlags(URF_TagSave);
		SAVE_PACKAGE_RESOURCE(ImageResourceObject, gallery);
	}

	return;
}

// ���б������
void GalleryCategoryWindow::addGalleryItemToList(GalleryResourceObject* gallery){
	QListWidgetItem* item = new QListWidgetItem(previewList_, ITEM_GALLERY);
	updateGalleryItem(item, gallery);
}


void GalleryCategoryWindow::insertGalleryItemToList(GalleryResourceObject* gallery){
	QListWidgetItem* item = NULL;
	// ����Ƿ��Ѿ�����
	for (int i = 0; i < previewList_->count(); i++){
		if (previewList_->item(i)->text() == gallery->objectName()){
			item = previewList_->item(i);
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

void GalleryCategoryWindow::on_importpushButton_clicked(){
	QTreeWidgetItem* item = galleryTree_->currentItem();
	QString cate;
	if (item){
		cate = galleryTree_->getItemPath(item);
	}
	ImportGalleryWindow w(cate);
	w.exec();

	//   ˢ�µ�ǰ��ʾ
	updateCurrentCategory();

}

void GalleryCategoryWindow::updateCategory(const QString& cate, const QString& tag){
	previewList_->clear();

	// �����б�
	for (TResourceObjectIterator<GalleryResourceObject> it; it; ++it){
		GalleryResourceObject* gallery = (GalleryResourceObject*)(*it);

		if (gallery->flags() & URF_TagGarbage)
			continue;

		if (gallery->isBelong(cate)){

			// ֻ����ͼƬ
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



void GalleryCategoryWindow::addGalleryCategory(){
	//
	QTreeWidgetItem* item = galleryTree_->currentItem();

	QTreeWidgetItem* newItem = new QTreeWidgetItem(ITEM_CATEGORY);
	newItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsDropEnabled | Qt::ItemIsEnabled);
	newItem->setIcon(0, defaultIcon_);
	if (item == NULL || item->type() == ITEM_UNCLASS){
		newItem->setText(0, galleryTree_->makeUniqueChildName(NULL, UDQ_TR("����Ŀ")));
		galleryTree_->addTopLevelItem(newItem);
	}
	else if (item->type() == ITEM_CATEGORY){
		newItem->setText(0, galleryTree_->makeUniqueChildName(item, UDQ_TR("����Ŀ")));
		item->addChild(newItem);
		galleryTree_->expandItem(item);
	}
	else{
		delete newItem;
		Q_UNREACHABLE();
	}

	// ��������

}

void GalleryCategoryWindow::deleteGalleryCategory(){
	QTreeWidgetItem* item = galleryTree_->currentItem();

	if (item == NULL || item->type() == ITEM_UNCLASS)
		return;

	QString path = galleryTree_->getItemPath(item);
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
		if (UIsExactKindOf(ImageResourceObject, gallery)){
			SAVE_PACKAGE_RESOURCE(ImageResourceObject, gallery);
		}
	}

	// ɾ������
	delete item;

}


void GalleryCategoryWindow::transparentImageBG(){
	QList<QListWidgetItem*> items = previewList_->selectedItems();
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

void GalleryCategoryWindow::copyGallery(){
	QList<QListWidgetItem*> items = previewList_->selectedItems();
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
		else{
			Q_UNREACHABLE();
		}

	}

	//   ˢ�µ�ǰ��ʾ
	updateCurrentCategory();

}

void GalleryCategoryWindow::updateCurrentCategory(){
	updateCategory(galleryTree_->getCurrentItemPath(), ui->filterineEdit->text());
}


void GalleryCategoryWindow::removeGalleryCategory(QListWidgetItem* item){
	Q_ASSERT(item != NULL);

	if (galleryTree_->currentItem()->type() == ITEM_UNCLASS){
		return;
	}

	ResourceHash key = item->data(ITEM_HASHKEY).toUInt();
	DEFINE_RESOURCE_HASHKEY_VALID(GalleryResourceObject, gallery, key);

	gallery->categories_.removeOne(galleryTree_->getCurrentItemPath());
	gallery->setFlags(URF_TagSave);

	delete item;

}
