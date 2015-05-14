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

#ifndef ORION_GALLERYCATEGORYWINDOW_H
#define ORION_GALLERYCATEGORYWINDOW_H

#include <QWidget>
#include <QTreeWidgetItem>
#include "commondefine.h"
#include "gallerytreewidget.h"
#include "gallerylistwidget.h"

namespace Ui {
class GalleryCategoryWindow;
}

class GalleryCategoryWindow : public QWidget
{
    Q_OBJECT

public:
    explicit GalleryCategoryWindow(QWidget *parent = 0);
    ~GalleryCategoryWindow();

	// ˢ��Ŀ¼��
	void updateCategoryTree();
	// ���µ�ǰ�����б�
	void updateCategoryPreview(const QString& cate, const QString& tag = UDQ_T(""));
	void updateGalleryItem(QListWidgetItem* item, GalleryResourceObject* gallery);
	void insertGalleryItemToList(GalleryResourceObject* gallery);
	void addGalleryItemToList(GalleryResourceObject* gallery);

	// ���µ�ǰ�����б�
	void updateCategory(const QString& cate, const QString& tag = UDQ_T(""));

protected:
	virtual void	resizeEvent(QResizeEvent * event);

private:
    Ui::GalleryCategoryWindow *ui;
	QStringList categoryPaths_;   // ������Ŀ·��
	QTreeWidgetItem* unknowCategoryItem_;  //  δ����

	GalleryTreeWidget* galleryTree_;
	GalleryListWidget* previewList_;

	QTreeWidgetItem* findTreePath(const QString& path, QTreeWidget* tree);
	QIcon defaultIcon_;      // Icon
private slots:
    void treeItemSelectionChanged();
	void on_filterineEdit_textEdited(const QString & text);

	void on_importpushButton_clicked();
	void on_addpushButton_clicked();

	void addGalleryCategory();
	void deleteGalleryCategory();
	void updateCurrentCategory();
	void removeGalleryCategory(QListWidgetItem* item);

	void transparentImageBG();
	void copyGallery();


};

#endif // GALLERYCATEGORYWINDOW_H
