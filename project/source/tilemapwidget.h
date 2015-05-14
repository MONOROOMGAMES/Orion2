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
#ifndef TILEMAPWIDGET_H
#define TILEMAPWIDGET_H

#include <QWidget>
#include "resourceobject.h"
using namespace ORION;

class TileMapWidget : public QWidget{
	Q_OBJECT

public:
	TileMapWidget(QWidget *parent = 0);
	~TileMapWidget();

	void init(const QImage& img, const QSize& grid, const QByteArray& collison );
	void updateBlock();
	void updateCollision(const QByteArray& collison);

	// ���
	void clear();

	// �ı�ѡ��ģʽ
	void setSelectMode(int mode);

	// ����
	void zoomOut();
	void zoomIn();

	QRect activeRegion(){
		return currentRect_.toRect();
	}

	
	int currentIndex(){
		return currentIndex_;
	}

protected:
	virtual void paintEvent ( QPaintEvent * event );
	virtual void mouseDoubleClickEvent ( QMouseEvent * event );
	virtual void mousePressEvent(QMouseEvent * event);
	virtual void mouseReleaseEvent(QMouseEvent * event);
	virtual void mouseMoveEvent(QMouseEvent * event);

signals:
	void changeBrush(const QRect& region );// 0��ʾ��ӣ�1��ʾɾ��

private:
	QImage image_;
	QSize gridsize_;  // �����С
	QMap<int, QList<QPointF> > collision_; // ��ײ����
	int selectMode_;  // ѡ��ģʽ0��ѡ��1��ѡ

	QSize validBlock_;   // ��Ч��Χ, ÿ�α任gridsize��ͼƬʱ��������
	QRectF currentRect_;  // ��ǰ�Tile��
	int currentIndex_;   // ��ǰѡ��Ŀ����������ѡ�����飬�򷵻����Ͻǿ�

	QRectF activeBlock(const QPoint& pos);
	QPoint startPos_;  // ��ѡ���

	int zoom_;  // ���ű�����ֻ����С



};

#endif // TILEMAPWIDGET_H
