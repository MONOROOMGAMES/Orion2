/************************************************************************
**
**  Copyright(C) 2015  guobin  <guobin.gb@alibaba - inc.com>
**
**  This file is part of Orion2.
**
**  Orion2 is free software : you can redistribute it and / or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation.
**
**  Orion2 is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Orion.If not, see < http://www.gnu.org/licenses/>.
**
*************************************************************************/

#ifndef ORION_GALLERYTREEWIDGET_H
#define ORION_GALLERYTREEWIDGET_H

#include <QTreeWidget>
#include "resourceobject.h"
#include "resourcelinker.h"
#include "oriontreewidget.h"
using namespace ORION;

class GalleryTreeWidget : public OrionTreeWidget{
	Q_OBJECT

public:
	GalleryTreeWidget(QWidget *parent = 0);
	~GalleryTreeWidget();

	// ��ȡ��ǰѡ�����·��
	QString getCurrentItemPath();
	QString getItemPath(QTreeWidgetItem* item);

	// ����Ψһ�ӽڵ���
	QString makeUniqueChildName(QTreeWidgetItem* item, const QString& prefix = UDQ_T("Category"));
	QTreeWidgetItem* findChild(QTreeWidgetItem* item, const QString& name);

	// �����ֵܽڵ��Ƿ�������1��ʾ�Լ���>=2��ʾ��������
	int findDuplications(QTreeWidgetItem* item);
signals:
	void galleryCategoryChanged();

protected:
	virtual void dropEvent(QDropEvent * event);
	virtual QStringList	mimeTypes() const;
	virtual void contextMenuEvent(QContextMenuEvent * event);

};

#endif // ORION_GALLERYTREEWIDGET_H
