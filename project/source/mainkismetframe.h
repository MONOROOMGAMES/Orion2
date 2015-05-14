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

#ifndef ORION_KISTMET_EDITOR_H
#define ORION_KISTMET_EDITOR_H

#include <QMainWindow>
#include <QUndoStack>
#include "kismetview.h"
#include "kismetstructwindow.h"
#include "kismetinstancewindow.h"
#include "kismeteventwindow.h"
#include "resourceobject.h"
#include "resourcelinker.h"
using namespace ORION;

class QAction;
class QActionGroup;
class QTreeWidget;
class QMenu;
class QTextEdit;
class QGraphicsView;
class QToolButton;
class QToolBox;
class QButtonGroup;
class QTableWidget;
class QGraphicsScene;
class QAbstractButton;

class MainKismetFrame : public QMainWindow{
	Q_OBJECT

public:
	MainKismetFrame(ProjectResourceObject* proj = 0, QWidget *parent = 0);
	~MainKismetFrame();

	 KismetView* sceneView(){
		return editorView_;
	}

	 KismetInstanceWindow* instanceWidget(){
		 return instWidget_;
	}

	KismetStructWindow* structWidget(){
		return structWidget_;
	}

	KismetEventWindow* eventWidget(){
		return eventWidget_;
	}

	QUndoStack* undoStack(){
		return undoStack_;
	}

	// ����
	void loadScript(BlueprintResourceObject* script);
	void clearScript();
	//  ˢ�µ�ǰ�򿪵Ľű�
	void refreshScript();
	// ���±�������
	void updateVariableType(ScriptVariable* var);
	void removeVariable(ScriptVariable* var);

protected:
	void closeEvent(QCloseEvent *event);
	void showEvent(QShowEvent * event);

public slots:
	void pointerMode();
	void addGroupItem();
	void addCommentItem();

	void addBranchItem();
	void addMathItem();
	void addSequenceItem();

	//////////////////////////////////////////////////////////////////////////
	// �˵���Ϣ
	//////////////////////////////////////////////////////////////////////////
	void refreshBlueprint();
	void saveBlueprint();
	void clearBlueprint();
	void checkBlueprint();

	//////////////////////////////////////////////////////////////////////////
	// undo
	//////////////////////////////////////////////////////////////////////////
	void entityModified(QGraphicsItem* item, const QByteArray& data);
	void entityInserted(QGraphicsItem* item);
	void entityDeleted(const QList<QGraphicsItem*>& items);
	void entityMoved(const QList<QGraphicsItem*>& items);
	void entityResized(QGraphicsItem* item);
	void entityCopy(const QList<QGraphicsItem*>& items, const QPointF& pos);
	

private:
	// ��ʼ��
	void init();
	// �����˵�����
	void createActions();
	void connectSceneEvent(QGraphicsScene* scene);
	// �����˵�
	void createMenus();
	// ����������
	void createToolBars();
	// ����״̬��
	void createStatusBar();
	// ������������
	void createDockWindows();

private:
	// ��ʼ������
	void initSceneView();
	// ���±༭���޸ı�־
	void editorModified( bool flag );
	// ���ģ����Ч��
	bool checkValid( QStringList& error, QStringList& warning );

	///////////////////////////////////////////////////////////////////////////
	// ������
	//////////////////////////////////////////////////////////////////////////
	QToolBar *editorToolBar_;       // ������
	QToolBar *undoToolBar_;       // Undo

	QActionGroup* modeGroup_;  // �༭ģʽ��

	//////////////////////////////////////////////////////////////////////////
	// ˽�б���
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Workspace
	//////////////////////////////////////////////////////////////////////////

	KismetView*  editorView_;

	QDockWidget *structDockWidget_;	 // ����������
	QDockWidget *instanceDockWidget_;		 // ����������
	QDockWidget *eventDockWidget_;		 // ����������

	KismetStructWindow* structWidget_;  // ̬�ƽṹͼ
	KismetInstanceWindow* instWidget_;
	KismetEventWindow* eventWidget_;

	QUndoStack *undoStack_;    // �������� 
	QAction* undoAction_;
	QAction* redoAction_;

};

#endif // KISMET_MAINFRAME_H
