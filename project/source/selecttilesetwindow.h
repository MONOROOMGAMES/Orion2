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
#ifndef SELECTTILESETWINDOW_H
#define SELECTTILESETWINDOW_H

#include <QDialog>
#include <QTreeWidgetItem>
#include "resourceobject.h"
using namespace ORION;

namespace Ui {
class SelectTilesetWindow;
}

class SelectTilesetWindow : public QDialog
{
    Q_OBJECT

public:
    explicit SelectTilesetWindow(ResourceHash id = 0, QWidget *parent = 0);
    ~SelectTilesetWindow();

	void initFromDb(void);

	// ����·������A.B.C�������Ҷ�Ӧ����û���򴴽�
	QTreeWidgetItem* findTreePath(const QString& path, QTreeWidget* tree);
	// ���б������
	void addGalleryItemToList(ImageResourceObject* gallery);

	QTreeWidgetItem* unknowCategoryItem_;  //  δ����

	// ���µ�ǰ�����б�
	void updateCategory(const QString& cate, const QString& tag = UDQ_T(""));
	void updateCurrentCategory();

	QStringList categoryPaths_;   // ������Ŀ·��
	QString currentCategoryPath_; // ��ǰ��ʾ����Ŀ·��
	ResourceHash replaceId_;  // �滻ID

	ResourceHash selectedGallery_;
	QSize gridSize_; //

signals:
	void addGallery(unsigned int, unsigned int);

private:
    Ui::SelectTilesetWindow *ui;

private slots:
void on_savepushButton_clicked();
void on_cancelpushButton_clicked();
void on_treeWidget_itemSelectionChanged();
void on_listWidget_itemSelectionChanged();

void mergeImage();
void splitImage();
};

#endif // SELECTTILESETWINDOW_H
