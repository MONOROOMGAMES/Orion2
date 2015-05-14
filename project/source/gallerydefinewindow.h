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
#ifndef ORION_GALLERYDEFINEWINDOW_H
#define ORION_GALLERYDEFINEWINDOW_H

#include <QDialog>
#include <QListWidget>
#include "resourceobject.h"
#include "resourcelinker.h"
using namespace ORION;

namespace Ui {
class GalleryDefineWindow;
}

class GalleryDefineWindow : public QDialog
{
    Q_OBJECT

public:
    explicit GalleryDefineWindow(QWidget *parent = 0);
    ~GalleryDefineWindow();

	void initFromDb(void);
protected:
	virtual void	closeEvent(QCloseEvent * event);
private:
    Ui::GalleryDefineWindow *ui;

	// ����·������A.B.C�������Ҷ�Ӧ����û���򴴽�
	QTreeWidgetItem* findTreePath(const QString& path, QTreeWidget* tree);
	// ���б������
	void insertGalleryItemToList(GalleryResourceObject* gallery);
	void addGalleryItemToList(GalleryResourceObject* gallery);
	void updateGalleryItem(QListWidgetItem* item, GalleryResourceObject* gallery);

	QTreeWidgetItem* unknowCategoryItem_;  //  δ����

	// ���µ�ǰ�����б�
	void updateCategory(const QString& cate, const QString& tag = UDQ_T("") );

	QStringList categoryPaths_;   // ������Ŀ·��
	unsigned int filter_; // ��������
	QString currentCategoryPath_; // ��ǰ��ʾ����Ŀ·��
	QString oldItemName_;// 

	bool modified_;  // �Ƿ��޸Ĺ�

	QList<GalleryResourceObject*> deletedGalleries_; // ��ɾ������Դ

private slots:
	void on_savepushButton_clicked();
	void on_deletepushButton_clicked();
	void on_cancelpushButton_clicked();
	void on_importpushButton_clicked();
	void on_exportpushButton_clicked();
	void on_addpushButton_clicked();

	void on_treeWidget_itemSelectionChanged();
	void on_treeWidget_itemChanged(QTreeWidgetItem * item, int column);
	void on_treeWidget_itemDoubleClicked(QTreeWidgetItem * item, int column);

	void on_imagecheckBox_stateChanged(int state);
	void on_audiocheckBox_stateChanged(int state);
	void on_videocheckBox_stateChanged(int state);

	void on_taglineEdit_textEdited(const QString & text);

	void addGalleryCategory();
	void deleteGalleryCategory();
	void updateCurrentCategory();
	void removeGalleryCategory(QListWidgetItem* item);

	void mergeImage();
	void transparentImageBG();
	void copyGallery();
	void splitImage();
};

#endif // GALLERYDEFINEWINDOW_H
