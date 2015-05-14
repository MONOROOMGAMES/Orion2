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

#ifndef OSS_PATHWIDGET_H
#define OSS_PATHWIDGET_H

#include <QWidget>


class PathWidget : public QWidget{
	Q_OBJECT

public:
	PathWidget(QWidget *parent = 0);
	~PathWidget();

	struct PathButton
	{
		QString text;
		QString title;
		QRect rect;
		int state; // (0: ������1��ʾmousehover��2��ʾmousepresss)
	};

	void setEnable(bool flag = true);

	// ��ȡ��ǰ·��
	QString currentPath();
	QString currentRoot();
	QString currentLocation();
	void setRoot(const QString& root, const QString& loc = QString());  // OSS  ��  Local
	void enterPath(const QString& subPath);
	void setButton(const QString& text, PathButton& btn);

protected:
	virtual void	resizeEvent(QResizeEvent * event);
	virtual void   paintEvent ( QPaintEvent * event );
	virtual void	mouseMoveEvent(QMouseEvent * event);
	virtual void	mousePressEvent(QMouseEvent * event);
	virtual void	mouseReleaseEvent(QMouseEvent * event);
	virtual void    leaveEvent(QEvent * event);
private:
	QColor bgColor_;  //  ������ɫ
	QColor hoverColor_;  // ��ť������ɫ
	QColor color_;  // ��ť������ɫ

	QString text_;  // ��ʾ����
	QColor textcolor_;  // ������ɫ
	
	bool disableMode_;  // ����ģʽ

	QPoint currentMousePos_; //
	PathButton rootButton_;
	QList<PathButton> buttonBounds_;  // ��ť����λ�ã�1��Ϊ���ڵ㣩

	void adjust();

	void drawButtons(QPainter& painter);
	void drawButton(QPainter& painter, PathButton& button);
	void drawPathButton(QPainter& painter, PathButton& button, bool root = false);

	static QFont textFont(void);

signals:
	void changePath(const QString& path);

};

#endif // PICTUREWIDGET_H
