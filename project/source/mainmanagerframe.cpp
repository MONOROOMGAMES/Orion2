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
**  Orion2 is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Orion2.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#include <QtGui>
#include <QtGlobal>
#include <QSettings>
#include <QWhatsThis>
#include <QStatusBar>
#include <QDockWidget>
#include <QDesktopWidget>
#include <QMessageBox>
#include <QApplication>
#include <QMenuBar>
#include <QToolBar>
#include <QListWidget>
#include <QFileDialog>
#include <QProcess>
#include <QStandardPaths>
#include "commondefine.h"
#include "tooldefine.h"
#include "mainmanagerframe.h"
#include "menumanager.h"
#include "framemanager.h"
#include "configmanager.h"

#include "levelscene.h"

#include "gallerydefinewindow.h"
#include "spritedefinewindow.h"
#include "pluginmanagewindow.h"
#include "publishprojectwindow.h"
#include "debugsetupwindow.h"
#include "editorundocommand.h"
#include "exporter.h"

#include "aboutwindow.h"
using namespace ORION;

// ʹ��Qt4��reader
#define ORION_USE_READER_QT4

MainManagerFrame::MainManagerFrame(QWidget *parent) : QMainWindow(parent), levelView_(NULL), spriteEditor_(NULL), kismetEditor_(NULL){
	init();
}

MainManagerFrame::~MainManagerFrame(){
	if (spriteEditor_){
		delete spriteEditor_;
	}

	if (kismetEditor_){
		delete kismetEditor_;
	}
}

void MainManagerFrame::init(){

	// ���ô���ͼ��ͱ���
    setWindowIcon( QIcon( UDQ_T(":/images/unique64.png") ) );
    setWindowTitle( UDQ_TR("Orion2") );
	// ��������
	undoStack_ = new QUndoStack(this);
	undoStack_->setUndoLimit(100);
	undoAction_ = undoStack_->createUndoAction(this, UDQ_TR("����"));
	undoAction_->setShortcuts(QKeySequence::Undo);
	undoAction_->setIcon(QIcon(UDQ_T(":/images/undo.png")));
	redoAction_ = undoStack_->createRedoAction(this, UDQ_TR("����"));
	redoAction_->setShortcuts(QKeySequence::Redo);
	redoAction_->setIcon(QIcon(UDQ_T(":/images/redo.png")));

	initView();
	createActions();

	createDockWindows();
	createMenus();
	createToolBars();
	createStatusBar();

	// ��ӭҳ��
	welcomeWindow_ = new WelcomeWindow(this);
	connect(welcomeWindow_, SIGNAL(openProject()), this, SLOT(prepareProject()));
	welcomeWindow_->show();


	// �ָ�����
	restoreGeometry(gConfigManager->windowGeometry());
	restoreState(gConfigManager->windowState());

	// ��ʱ����
	saveTimer_ = new QTimer(this);
	connect(saveTimer_, SIGNAL(timeout()), this, SLOT(saveCurrentModel()));
	saveTimer_->start(10 * 60 * 1000);  // 10���ӱ���

}

// ��ʼ������
void MainManagerFrame::initView(){
	
	if (levelView_ == NULL){
		levelView_ = new LevelView();
		setCentralWidget(levelView_);
	}

	// �����ź�
	QGraphicsScene* oldScence = levelView_->scene();
	LevelScene* scene = new LevelScene();
	levelView_->setScene(scene);

	// �����ź�
	connect(scene, SIGNAL(pointerMode()), this, SLOT(pointerMode()));
	connect(scene, SIGNAL(addObjectType(unsigned int)), this, SLOT(addObjectType(unsigned int)));
	connect(scene, SIGNAL(refreshTileMap(QGraphicsItem*)), this, SLOT(refreshTileMap(QGraphicsItem*)));
	connect(scene, SIGNAL(selectionChanged()), this, SLOT(selectionChanged()));
	connect(scene, SIGNAL(itemChanged(QGraphicsItem*)), this, SLOT(itemChanged(QGraphicsItem*)));
	connect(scene, SIGNAL(refreshObjectType(unsigned int)), this, SLOT(refreshObjectType(unsigned int)));

	connect(scene, SIGNAL(entityModified(QGraphicsItem*, const QByteArray&)), this, SLOT(entityModified(QGraphicsItem*, const QByteArray&)));
	connect(scene, SIGNAL(entityInserted(QGraphicsItem*, unsigned int )), this, SLOT(entityInserted(QGraphicsItem*, unsigned int )));
	connect(scene, SIGNAL(entityDeleted(QGraphicsItem*, unsigned int )), this, SLOT(entityDeleted(QGraphicsItem*, unsigned int )));

	// �ͷ��ϵ�ͼ
	if (oldScence != 0){
		delete oldScence;
	}

}


void MainManagerFrame::createActions(){

	gMenuManager->createActions( EDITOR_LEVEL, this );

	BEGIN_ACTION();
	//////////////////////////////////////////////////////////////////////////
	// Projects
	//////////////////////////////////////////////////////////////////////////

	CONNECT_ACTION(ORION_PROJECT_EXIT, this, exitApplication);
	CONNECT_ACTION(ORION_PROJECT_SAVE, this, saveCurrentModel);
	CONNECT_ACTION(ORION_PROJECT_CLOSE, this, closeCurrentWorkspace);
	CONNECT_ACTION(ORION_PROJECT_PUBLISH, this, publishProject);

	//////////////////////////////////////////////////////////////////////////
	// Edit
	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	// Gallery
	//////////////////////////////////////////////////////////////////////////
	
	CONNECT_ACTION(ORION_GALLERY_EDIT, this, galleryManage);
	CONNECT_ACTION(ORION_GALLERY_SPRITE, this, spriteManage);
	CONNECT_ACTION(ORION_GALLERY_PLUGIN, this, pluginManage);
	CONNECT_ACTION(ORION_GALLERY_BLUEPRINT, this, kismetManage);

	//////////////////////////////////////////////////////////////////////////
	// Tools
	//////////////////////////////////////////////////////////////////////////

	CONNECT_ACTION(ORION_TOOL_SETUP, this, debugSetup);
	CONNECT_ACTION(ORION_TOOL_PREVIEW, this, previewProject);
	CONNECT_ACTION(ORION_TOOL_DEBUG, this, debugProject);


    //////////////////////////////////////////////////////////////////////////
	// Help
	//////////////////////////////////////////////////////////////////////////
	
	CONNECT_ACTION(ORION_HELP_ABOUT, this, about);
	CONNECT_ACTION(ORION_HELP_WELCOME, this, welcome);


	END_ACTION();

	//////////////////////////////////////////////////////////////////////////
	// �༭��
	//////////////////////////////////////////////////////////////////////////

	QGraphicsScene* scene = levelView_->scene();
	Q_ASSERT(scene != NULL);
	connectSceneEvent(scene);
}

void MainManagerFrame::connectSceneEvent(QGraphicsScene* scene){

	BEGIN_ACTION();
	CONNECT_ACTION(ORION_EDIT_LOCK, scene, toggleLock);
	CONNECT_ACTION(ORION_EDIT_PANELOPTION, scene, toggleOption);
	act->setChecked(false);

	CONNECT_ACTION(ORION_EDIT_TODEFAULTOBJ, scene, saveToObjectDefault);
	CONNECT_ACTION(ORION_EDIT_FROMDEFAULTOBJ, scene, loadFromObjectDefault);

	CONNECT_ACTION(ORION_EDIT_CLONE, scene, cloneObjectType);
	CONNECT_ACTION(ORION_EDIT_COPY, scene,copyInstance);
	CONNECT_ACTION(ORION_EDIT_CUT, scene, cutInstance);
	CONNECT_ACTION(ORION_EDIT_PASTE, scene, pasteInstance);

	CONNECT_ACTION(ORION_EDIT_MOVEFIRST, scene, moveInstanceToFirst);
	CONNECT_ACTION(ORION_EDIT_MOVELAST, scene, moveInstanceToLast);

	END_ACTION();
}

void MainManagerFrame::connectWorkspaceEvent(QWidget* widget){
	BEGIN_ACTION();

	END_ACTION();
}

void MainManagerFrame::createMenus(){

	//////////////////////////////////////////////////////////////////////////
	// ��������
	//////////////////////////////////////////////////////////////////////////

	modelMenu_ = menuBar()->addMenu(UDQ_TR("�ļ�[&P]"));
	modelMenu_->addAction(gMenuManager->getAction(ORION_PROJECT_NEW));
	modelMenu_->addAction(gMenuManager->getAction(ORION_PROJECT_OPEN));
	modelMenu_->addSeparator();
	modelMenu_->addAction(gMenuManager->getAction(ORION_PROJECT_CLOSE));
	modelMenu_->addAction(gMenuManager->getAction(ORION_PROJECT_SAVE));
    modelMenu_->addAction( gMenuManager->getAction(ORION_PROJECT_SAVEAS) );
	modelMenu_->addSeparator();
	modelMenu_->addAction(gMenuManager->getAction(ORION_PROJECT_EXPORT));
	modelMenu_->addAction(gMenuManager->getAction(ORION_PROJECT_IMPORT));
 	modelMenu_->addSeparator();
	modelMenu_->addAction(gMenuManager->getAction(ORION_PROJECT_PUBLISH));
	modelMenu_->addSeparator();
	modelMenu_->addAction(gMenuManager->getAction(ORION_PROJECT_EXIT));

	//////////////////////////////////////////////////////////////////////////
	// �༭��
	//////////////////////////////////////////////////////////////////////////

	editMenu_ = menuBar()->addMenu(UDQ_TR("�༭[&G]"));
	editMenu_->addAction(gMenuManager->getAction(ORION_EDIT_POINTER));
	editMenu_->addSeparator();
	editMenu_->addAction(undoAction_);
	editMenu_->addAction(redoAction_);
	editMenu_->addSeparator();
	editMenu_->addAction(gMenuManager->getAction(ORION_EDIT_COPY));
	editMenu_->addAction(gMenuManager->getAction(ORION_EDIT_CUT));
	editMenu_->addAction(gMenuManager->getAction(ORION_EDIT_PASTE));
	editMenu_->addSeparator();
	editMenu_->addAction(gMenuManager->getAction(ORION_EDIT_PANELOPTION));

	//////////////////////////////////////////////////////////////////////////
	// ����
	//////////////////////////////////////////////////////////////////////////
	
	debugMenu_ = menuBar()->addMenu(UDQ_TR("����[&D]"));
	debugMenu_->addAction(gMenuManager->getAction(ORION_TOOL_SETUP));
	debugMenu_->addSeparator();
	debugMenu_->addAction(gMenuManager->getAction(ORION_TOOL_PREVIEW));
	debugMenu_->addAction(gMenuManager->getAction(ORION_TOOL_DEBUG));


	//////////////////////////////////////////////////////////////////////////
	// ����
	//////////////////////////////////////////////////////////////////////////

	galleryMenu_ = menuBar()->addMenu(UDQ_TR("����[&T]"));
	galleryMenu_->addAction(gMenuManager->getAction(ORION_GALLERY_EDIT));
	galleryMenu_->addSeparator();
	galleryMenu_->addAction(gMenuManager->getAction(ORION_GALLERY_SPRITE));
	galleryMenu_->addAction(gMenuManager->getAction(ORION_GALLERY_BLUEPRINT));
	galleryMenu_->addSeparator();
	galleryMenu_->addAction(gMenuManager->getAction(ORION_GALLERY_PLUGIN));

	//////////////////////////////////////////////////////////////////////////
	// ��̬�˵�
	//////////////////////////////////////////////////////////////////////////

	menuBar()->addSeparator();


	//////////////////////////////////////////////////////////////////////////
	// ����
	//////////////////////////////////////////////////////////////////////////

	windowMenu_ = menuBar()->addMenu(UDQ_TR("����[&W]"));
	windowMenu_->addAction(tileDockWidget_->toggleViewAction());
	windowMenu_->addAction(galleryDockWidget_->toggleViewAction());
	windowMenu_->addAction(componentDockWidget_->toggleViewAction());
	windowMenu_->addAction(propertyDockWidget_->toggleViewAction());
	

	//////////////////////////////////////////////////////////////////////////
	// Help
	//////////////////////////////////////////////////////////////////////////

	helpMenu_ = menuBar()->addMenu(UDQ_TR("����[&H]"));
	helpMenu_->addAction( gMenuManager->getAction(ORION_HELP_ABOUT));
	helpMenu_->addAction(gMenuManager->getAction(ORION_HELP_WELCOME));

}



void MainManagerFrame::createToolBars()
{

	//////////////////////////////////////////////////////////////////////////
	// Model
	//////////////////////////////////////////////////////////////////////////
	modelToolBar_ = addToolBar(UDQ_TR("����"));
	modelToolBar_->setObjectName(UDQ_T("modelToolBar"));
	modelToolBar_->addAction(gMenuManager->getAction(ORION_HELP_WELCOME));

	//////////////////////////////////////////////////////////////////////////
	// �༭
	//////////////////////////////////////////////////////////////////////////
	editToolBar_ = addToolBar(UDQ_TR("�༭"));
	editToolBar_->setObjectName(UDQ_T("editToolBar"));
	editToolBar_->addAction(undoAction_);
	editToolBar_->addAction(redoAction_);
	editToolBar_->addSeparator();
	editToolBar_->addAction(gMenuManager->getAction(ORION_GALLERY_EDIT));
	editToolBar_->addAction(gMenuManager->getAction(ORION_GALLERY_SPRITE));
	editToolBar_->addAction(gMenuManager->getAction(ORION_GALLERY_BLUEPRINT));

	//////////////////////////////////////////////////////////////////////////
	// ����
	//////////////////////////////////////////////////////////////////////////
	debugToolBar_ = addToolBar(UDQ_TR("����"));
	debugToolBar_->setObjectName(UDQ_T("debugToolBar"));
	debugToolBar_->addAction(gMenuManager->getAction(ORION_TOOL_PREVIEW));
	debugToolBar_->addAction(gMenuManager->getAction(ORION_TOOL_DEBUG));

}

void MainManagerFrame::createStatusBar()
{
	statusBar()->showMessage(UDQ_TR("����"));
}

void MainManagerFrame::createDockWindows(){


	////////////////////////////////////////////////////////////////////////
	// �����ռ�
	////////////////////////////////////////////////////////////////////////

	workspaceDockWidget_ = new QDockWidget(this);
	workspaceDockWidget_->setObjectName(UDQ_T("workspaceDock"));
	workspaceDockWidget_->setAllowedAreas(Qt::AllDockWidgetAreas);
	workspaceDockWidget_->setFeatures(QDockWidget::NoDockWidgetFeatures);
	addDockWidget(Qt::RightDockWidgetArea, workspaceDockWidget_);

	workspaceWindow_ = new WorkspaceWindow(workspaceDockWidget_);
	workspaceDockWidget_->setWidget(workspaceWindow_);
	workspaceDockWidget_->setWindowTitle(workspaceWindow_->windowTitle());

	connect(workspaceWindow_, SIGNAL(activeLayer(unsigned int)), this, SLOT(activeLayer(unsigned int)));
	connect(workspaceWindow_, SIGNAL(refreshLayers()), this, SLOT(refreshLayers()));
	connect(workspaceWindow_, SIGNAL(refreshCurrentLayer()), this, SLOT(refreshCurrentLayer()));
	connect(workspaceWindow_, SIGNAL(deleteObjectType(unsigned int)), this, SLOT(deleteObjectType(unsigned int)));
	connect(workspaceWindow_, SIGNAL(deleteFamily(unsigned int)), this, SLOT(deleteFamily(unsigned int)));


	//////////////////////////////////////////////////////////////////////////
	// ����
	//////////////////////////////////////////////////////////////////////////

	propertyDockWidget_ = new QDockWidget(this);
	propertyDockWidget_->setObjectName(UDQ_T("propertyDock"));
	propertyDockWidget_->setAllowedAreas(Qt::AllDockWidgetAreas);
	propertyDockWidget_->setFeatures(/*QDockWidget::NoDockWidgetFeatures | */
		QDockWidget::DockWidgetClosable |
		QDockWidget::DockWidgetMovable);
	addDockWidget(Qt::LeftDockWidgetArea, propertyDockWidget_);

	propertyWindow_ = new ObjectPropertyWindow(propertyDockWidget_);
	propertyDockWidget_->setWidget(propertyWindow_);
	propertyDockWidget_->setWindowTitle(propertyWindow_->windowTitle());

	////////////////////////////////////////////////////////////////////////
	// Tile��ͼ
	////////////////////////////////////////////////////////////////////////

	tileDockWidget_ = new QDockWidget(this);
	tileDockWidget_->setObjectName(UDQ_T("tileDock"));
	tileDockWidget_->setAllowedAreas(Qt::AllDockWidgetAreas);
	tileDockWidget_->setFeatures(/*QDockWidget::NoDockWidgetFeatures | */
		QDockWidget::DockWidgetClosable |
		QDockWidget::DockWidgetMovable);
	addDockWidget(Qt::LeftDockWidgetArea, tileDockWidget_);

	layerWindow_ = new LayerWindow(tileDockWidget_);
	tileDockWidget_->setWidget(layerWindow_);
	tileDockWidget_->setWindowTitle(layerWindow_->windowTitle());


	////////////////////////////////////////////////////////////////////////
	// �زĿ�
	////////////////////////////////////////////////////////////////////////

	galleryDockWidget_ = new QDockWidget(this);
	galleryDockWidget_->setObjectName(UDQ_T("galleryDock"));
	galleryDockWidget_->setAllowedAreas(Qt::AllDockWidgetAreas);
	galleryDockWidget_->setFeatures(/*QDockWidget::NoDockWidgetFeatures | */
		QDockWidget::DockWidgetClosable |
		QDockWidget::DockWidgetMovable);
	addDockWidget(Qt::LeftDockWidgetArea, galleryDockWidget_);

	categoryWindow_ = new GalleryCategoryWindow(galleryDockWidget_);
	galleryDockWidget_->setWidget(categoryWindow_);
	galleryDockWidget_->setWindowTitle(categoryWindow_->windowTitle());

	//////////////////////////////////////////////////////////////////////////
	// �����
	//////////////////////////////////////////////////////////////////////////

	componentDockWidget_ = new QDockWidget(this);
	componentDockWidget_->setObjectName(UDQ_T("componentDock"));
	componentDockWidget_->setAllowedAreas(Qt::AllDockWidgetAreas);
	componentDockWidget_->setFeatures(/*QDockWidget::NoDockWidgetFeatures | */
		QDockWidget::DockWidgetClosable |
		QDockWidget::DockWidgetMovable);
	addDockWidget(Qt::RightDockWidgetArea, componentDockWidget_);

	componentWindow_ = new EntityComponentWindow(componentDockWidget_);
	componentDockWidget_->setWidget(componentWindow_);
	componentDockWidget_->setWindowTitle(componentWindow_->windowTitle());

}

// ����
void MainManagerFrame::about(){
	AboutWindow w(this);
	w.exec();
}

void MainManagerFrame::welcome(){
	welcomeWindow_->show();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

void MainManagerFrame::closeEvent(QCloseEvent * event){
	// �ر������༭������
	if (spriteEditor_){
		spriteEditor_->close();
	}

	if (kismetEditor_){
		kismetEditor_->close();
	}

	// ��ʾ�Ƿ񱣴�
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws != NULL){
		// ��ȡ��ǰ������
		ProjectResourceObject* proj = ws->currentProject();
		if (proj != NULL ){
			if(QMessageBox::question(this, windowTitle(), UDQ_TR("�Ƿ񱣴浱ǰ��Ŀ<%1>?").arg(proj->objectName())) == QMessageBox::Yes ){
				saveCurrentModel();
			}
		}
	}

	gConfigManager->saveWindowGeometry(saveGeometry());
	gConfigManager->saveWindowState(saveState());
}

void	MainManagerFrame::resizeEvent(QResizeEvent * event){

}

//////////////////////////////////////////////////////////////////////////
// �źŲۺ���
//////////////////////////////////////////////////////////////////////////

void MainManagerFrame::exitApplication(){
	qApp->closeAllWindows();
}

void MainManagerFrame::galleryManage(){
	GalleryDefineWindow w;
	w.exec();

	// ˢ���ز��б�
	categoryWindow_->updateCategoryTree();

	//  ˢ�µ�ǰ����
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws ){
		ProjectResourceObject* proj = ws->currentProject();
		if (proj){	
			LevelScene* scene = (LevelScene*)levelView_->scene();
			scene->updateRegions();
		}
	}
	
}

void MainManagerFrame::spriteManage(){
	if (spriteEditor_ == NULL){
		spriteEditor_ = new MainSpriteFrame();
		gFrameManager->addWindow(FrameManager::WID_SPRITE, spriteEditor_);
		spriteEditor_->showMaximized();
	}
	else{
		spriteEditor_->showMaximized();
	}
}

void MainManagerFrame::kismetManage(){

	// �����һ������
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL){
		QMessageBox::warning(this, UDQ_TR("�ű��༭"), UDQ_TR("���ȴ�һ������!"));
		return;
	}

	// ��ȡ��ǰ������
	ProjectResourceObject* proj = ws->currentProject();
	if (proj == NULL){
		QMessageBox::warning(this, UDQ_TR("�ű��༭"), UDQ_TR("���ȴ�һ������!"));
		return;
	}

	// ���浱ǰ�༭����
	if (proj){
		proj->setFlags(URF_TagSave);
		// ���浱ǰ�༭������
		LevelScene* scene = (LevelScene*)levelView_->scene();
		EditorLevel* levelItem = scene->getLevelItem();
		if (levelItem){
			scene->saveLevel(levelItem->level_);
		}
	}

	if (kismetEditor_ == NULL){
		kismetEditor_ = new MainKismetFrame();
		gFrameManager->addWindow(FrameManager::WID_BLUEPRINT, kismetEditor_);
		kismetEditor_->showMaximized();
	}
	else{
		kismetEditor_->showMaximized();
	}
}

void MainManagerFrame::pluginManage(){
	PluginManageWindow w;
	w.exec();

	// ���½ű��༭��
	MainKismetFrame* frame = (MainKismetFrame*)gFrameManager->getWindow(FrameManager::WID_BLUEPRINT);
	if (frame){
		// ˢ�µ�ǰ��ʾ�Ľű�ģ�ͣ��޳��ı�Ľӿ�ģ��

	}

}


void MainManagerFrame::prepareProject(){

	//  ��ʼ�������ռ�
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	Q_ASSERT(ws != NULL);

	workspaceDockWidget_->setWindowTitle(UDQ_TR("�����ռ�[ %1 ]").arg(ws->objectName()));

	// ����Ŀ¼��
	workspaceWindow_->updateWorkspace();

	// ���½ű�
	if (kismetEditor_){
		kismetEditor_->clearScript();
		kismetEditor_->structWidget()->updateWorkspace();
	}

	//  ���Undo
	undoStack_->clear();
}

// �л�����
void MainManagerFrame::switchLevel(ResourceHash key){

	DEFINE_RESOURCE_HASHKEY(LevelResourceObject, level, key);
	LevelScene* scene = (LevelScene*)levelView_->scene();

	EditorLevel* levelItem = scene->getLevelItem();
	if ( level == NULL || (levelItem && levelItem->level_ == level ) ){
		return;
	}
	// ���浱ǰ����
	scene->closeCurrentLevel();
	// ���undo
	undoStack_->clear();

	// �����³���
	if (level){
		scene->loadLevel(level);
		// �Զ������һ��ͼ��
		scene->activeLayer(0);
	}

}

void MainManagerFrame::removeLevel(ResourceHash key){
	DEFINE_RESOURCE_HASHKEY(LevelResourceObject, level, key);
	Q_ASSERT(level != NULL);

	LevelScene* scene = (LevelScene*)levelView_->scene();
	// ����ǵ�ǰ����
	EditorLevel* levelItem = scene->getLevelItem();
	if (levelItem &&  levelItem->level_ == level ){
		switchLevel(0);   // �رյ�ǰ���������浱ǰ״̬��, ���Undo����
	}

	// ɾ������ͼ��
	level->setFlags(URF_TagGarbage);
	level->clear();
}

// ����ͼ��
void MainManagerFrame::updateLayer(unsigned int key){
	//
	LevelScene* scene = (LevelScene*)levelView_->scene();

	EditorLevel* levelItem = scene->getLevelItem();
	if (levelItem == NULL || levelItem->level_ == NULL ){
		return;
	}

	DEFINE_RESOURCE_HASHKEY(LayerResourceObject, layer, key);
	Q_ASSERT(layer != NULL);
	if (levelItem->level_->layers_.contains(layer)){
			scene->updateLayer(key);
	}
}

void MainManagerFrame::updateProperty(QTreeWidgetItem* item){
	propertyWindow_->loadProperty(item);
}

void MainManagerFrame::refreshProperty(QTreeWidgetItem* item){
	if (propertyWindow_->currentItem() == item){
		propertyWindow_->loadProperty(item);
	}
}

void MainManagerFrame::refreshProperty(EditorItem* item){
	if (propertyWindow_->currentSceneItem() == item){
		propertyWindow_->updateProperty(item, true);
	}
}

void MainManagerFrame::activeLayer(unsigned int key){
	LevelScene* scene = (LevelScene*)levelView_->scene();
	EditorLevel* levelItem = scene->getLevelItem();
	if (levelItem == NULL || levelItem->level_ == NULL){
		return;
	}

	DEFINE_RESOURCE_HASHKEY(LayerResourceObject, layer, key);
	if (levelItem->level_->layers_.contains(layer)){
		scene->activeLayer(key);
	}

}

void MainManagerFrame::addLayerToScene(ResourceHash key){
	LevelScene* scene = (LevelScene*)levelView_->scene();
	scene->addLayerByHash(key);
}

void MainManagerFrame::removeLayerFromScene(ResourceHash key){
	LevelScene* scene = (LevelScene*)levelView_->scene();
	scene->removeLayerByHash(key);
}

void MainManagerFrame::refreshCurrentLayer(){
	LevelScene* scene = (LevelScene*)levelView_->scene();
	scene->updateLayerText();
}

void MainManagerFrame::refreshLayers(){
	LevelScene* scene = (LevelScene*)levelView_->scene();
	scene->updateLayers();
}

void MainManagerFrame::reorderLayers(){
	LevelScene* scene = (LevelScene*)levelView_->scene();
	EditorLevel* levelItem = scene->getLevelItem();
	if (levelItem == NULL || levelItem->level_ == NULL){
		return;
	}

	workspaceWindow_->reorderLayerItems(levelItem->level_);
	scene->reorderLayers();
	scene->update();
}


void MainManagerFrame::refreshRegions(ObjectTypeResourceObject* obj){
	if (obj == NULL)
		return;

	LevelScene* scene = (LevelScene*)levelView_->scene();
	scene->updateRegions(obj);
}

void MainManagerFrame::addObjectType(unsigned int hashkey){
	workspaceWindow_->addObjectType(hashkey);
}

void MainManagerFrame::refreshObjectType(unsigned int hashkey){
	workspaceWindow_->refreshObjectType(hashkey);
}


void MainManagerFrame::refreshObjectTypes(){
	workspaceWindow_->refreshObjectTypes();
}

void MainManagerFrame::closeCurrentWorkspace(){
	// ��ձ༭��
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL)
		return;

	workspaceWindow_->clearSelection();

	// ������Դ���
	propertyWindow_->clearUndo();

	if (QMessageBox::question(this, windowTitle(), UDQ_TR("�Ƿ񱣴浱ǰ��Ŀ?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes){
		// ����̬����Ϣ
		saveCurrentModel();
	}

	// ��ձ༭��
	clearScene();

	// ��սű��༭��
	clearKismet();

	// ��տռ���
	workspaceWindow_->clearWorkspace();
	workspaceDockWidget_->setWindowTitle(workspaceWindow_->windowTitle());

}


void MainManagerFrame::saveCurrentModel(){
	// ����
	WorkspaceResourceObject*  ws = ResourceObject::currentWorkspace();
	if (ws == NULL)
		return;

	if (ws->currentProject()){
		ws->currentProject()->setFlags(URF_TagSave);
		// ���浱ǰ�༭������
		LevelScene* scene = (LevelScene*)levelView_->scene();

		EditorLevel* levelItem = scene->getLevelItem();
		if (levelItem){
			scene->saveLevel(levelItem->level_);
		}
	}

	// ����������͵ķ�����Ϣ
	if (workspaceWindow_){
		workspaceWindow_->saveObjectType();
	}

	// ����׼��(��������̬��)
	for (TResourceObjectIterator<ProjectResourceObject> it; it; ++it){
		ProjectResourceObject* proj = (ProjectResourceObject*)(*it);

		// ����������������ռ�ĵ�����
		Q_ASSERT(proj->getOutermost() == ws);

		if (!(proj->flags() & URF_TagGarbage) && proj->getLinker() == 0){
			// ����Linker
			QString projFile = proj->getOutermost()->objectName() + UDQ_T(".") + proj->fileType();
			ResourceLinker* linker = ResourceObject::getWorkspaceLinker(proj->getOutermost(), projFile);
			Q_ASSERT(linker != 0);
			proj->setLinker(linker);
		}
	}

	// ���浽����
	for (TResourceObjectIterator<ProjectResourceObject> it; it; ++it){
		ProjectResourceObject* proj = (ProjectResourceObject*)(*it);
		if (!(proj->flags() & URF_TagGarbage)){
			SAVE_WORKSPACE_RESOURCE(ProjectResourceObject, proj);
		}
	}



}

// ��ձ༭��
void MainManagerFrame::clearScene(){

	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws){
		// ��ȡ��ǰ������
		ProjectResourceObject* proj = ws->currentProject();
		Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

		LevelScene* scene = (LevelScene*)sceneView()->scene();
		scene->clearLevel();
	}

}

void MainManagerFrame::clearKismet(){
	if (kismetEditor_){
		kismetEditor_->clearScript();
	}
}

void MainManagerFrame::deleteObjectType(unsigned int hashkey){

	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws){
		// ��ȡ��ǰ������
		ProjectResourceObject* proj = ws->currentProject();
		Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

		LevelScene* scene = (LevelScene*)sceneView()->scene();

		DEFINE_RESOURCE_HASHKEY(ObjectTypeResourceObject, objType, hashkey);
		if (objType == NULL)
			return;

		// ɾ��ʵ��
		scene->deleteInstanceByType(objType);
		//  ɾ����������
		objType->setFlags(URF_TagGarbage);
		objType->treeItem_ = NULL;
		proj->objTypes_.removeOne(objType);

	}

}

void MainManagerFrame::deleteFamily(unsigned int hashkey){
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws){
		// ��ȡ��ǰ������
		ProjectResourceObject* proj = ws->currentProject();
		Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

		DEFINE_RESOURCE_HASHKEY(FamilyResourceObject, family, hashkey);
		if (family == NULL)
			return;
		//  ɾ����������
		family->setFlags(URF_TagGarbage);
		family->clear();
		proj->families_.removeOne(family);

	}
}

void MainManagerFrame::publishProject(){

	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws){
		// ��ȡ��ǰ������
		ProjectResourceObject* proj = ws->currentProject();
		if (proj == NULL ){
			QMessageBox::warning(this, UDQ_TR("����"), UDQ_TR("���ȴ�һ������"));
			return;
		}

		proj->exportType_ = 0;
		PublishProjectWindow w(proj);
		if (w.exec() == QDialog::Accepted){
			
		}
	}
}

void MainManagerFrame::debugSetup(){
	DebugSetupWindow w;
	w.exec();
}

void MainManagerFrame::previewProject(){
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL){
		QMessageBox::warning(this, UDQ_TR("Ԥ��"), UDQ_TR("���ȴ�һ������"));
		return;
	}

	// ��ȡ��ǰ
	ProjectResourceObject* proj = ws->currentProject();
	if (proj == NULL){
		QMessageBox::warning(this, UDQ_TR("Ԥ��"), UDQ_TR("���ȴ�һ������"));
		return;
	}

	// ������ã����û�������򵯳��Ի���
	QString rootPath = gConfigManager->serverRootPath();
	int port = gConfigManager->serverPort();
	QString serverCmd = gConfigManager->explorerCmd();
	if (rootPath.isEmpty() || serverCmd.isEmpty()){
		DebugSetupWindow w;
		if (w.exec() != QDialog::Accepted){
			return;
		}
	}

	proj->exportType_ = 1;
	// ���������Ի���
	PublishProjectWindow w(proj);
	if (w.exec() == QDialog::Accepted){

		// ����ģʽ
		if (w.exportPlatform() == Exporter::TYPE_HTML5){
			// ���������
			QString local = UDQ_TR("http://localhost:%1/%2/index.html").arg(port).arg(proj->objectName());
			QProcess::startDetached(serverCmd.arg(local));
		}
		else{
			// �򿪵���Ŀ¼
			QString destPath = QDir::toNativeSeparators(proj->exportDir_ + UDQ_T("/") + proj->objectName());
#ifdef Q_OS_WIN
			// windowƽ̨
			QProcess::startDetached(UDQ_TR("explorer /e, %1").arg(destPath));
#elif (defined Q_OS_OSX)
			// osxƽ̨
			QProcess::startDetached(UDQ_TR("open %1").arg(destPath));
#endif
		}

	}

}

void MainManagerFrame::debugProject(){
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws == NULL){
		QMessageBox::warning(this, UDQ_TR("Ԥ��"), UDQ_TR("���ȴ�һ������"));
		return;
	}

	// ��ȡ��ǰ������
	ProjectResourceObject* proj = ws->currentProject();
	if (proj == NULL){
		QMessageBox::warning(this, UDQ_TR("Ԥ��"), UDQ_TR("���ȴ�һ������"));
		return;
	}

	// ������ã����û�������򵯳��Ի���
	QString rootPath = gConfigManager->serverRootPath();
	int port = gConfigManager->serverPort();
	QString serverCmd = gConfigManager->explorerCmd();
	if (rootPath.isEmpty() || serverCmd.isEmpty()){
		DebugSetupWindow w;
		if (w.exec() != QDialog::Accepted){
			return;
		}
	}

	proj->exportType_ = 2;
	// ���������Ի���
	PublishProjectWindow w(proj);
	if (w.exec() == QDialog::Accepted){
		// ���������
		QString local = UDQ_TR("http://localhost:%1/%2/debug").arg(port).arg(proj->objectName());
		QProcess::startDetached(serverCmd.arg(local));
	}

}

void MainManagerFrame::refreshTileMap(QGraphicsItem* enttiy){

	if (enttiy){
		Q_ASSERT(enttiy->type() == LEVEL_TILEMAP);
		EditorTileMap* tilemap = (EditorTileMap*)enttiy;

		int width = tilemap->getParameterInt(UDQ_T("TileWidth"), tilemap->typePrototype_->prototype_->objectName());
		int height = tilemap->getParameterInt(UDQ_T("TileHeight"), tilemap->typePrototype_->prototype_->objectName());

		layerWindow_->loadTileMap(tilemap->typePrototype_, QSize(width, height));
	}
	else{
		layerWindow_->clear();
	}

	layerWindow_->update();

}

void MainManagerFrame::selectionChanged(){
	LevelScene* scene = (LevelScene*)sceneView()->scene();
	QList<QGraphicsItem*> items = scene->selectedItems();

	bool found = false;
	// ֻȡ��һ����Ч����
	foreach(QGraphicsItem* it, items){
		if (it->type() <= LEVEL_LAYER)
			continue;
		found = true;
		propertyWindow_->loadProperty((EditorItem*)it);
		break;
	}

	if (!found){
		propertyWindow_->clear();
	}

}

void MainManagerFrame::itemChanged(QGraphicsItem* item){
	if (item->type() <= LEVEL_LAYER)
		return;
	propertyWindow_->updateProperty((EditorItem*)item);
}


void MainManagerFrame::entityModified(QGraphicsItem* item, const QByteArray& oldData ){
	if (item->type() <= LEVEL_LAYER)
		return;
	EditorItem* edititem = (EditorItem*)item;
	QByteArray newData = edititem->encode();

	QByteArray newH = QCryptographicHash::hash(newData, QCryptographicHash::Md5);
	QByteArray oldH = QCryptographicHash::hash(oldData, QCryptographicHash::Md5);

	if (oldH != newH){
		LevelScene* scene = (LevelScene*)sceneView()->scene();
		ModifyInstanceCommand* command = new ModifyInstanceCommand(edititem, oldData, false, scene);
		undoStack_->push(command);
		command->setEnabled(true);
	}
}

void MainManagerFrame::entityInserted(QGraphicsItem* item, unsigned int hashkey){

	LevelScene* scene = (LevelScene*)sceneView()->scene();
	QUndoCommand* command = new AddInstanceCommand((EditorItem*)item, hashkey, scene);
	undoStack_->push(command);

}

void MainManagerFrame::entityDeleted(QGraphicsItem* item, unsigned int hashkey){
	//����������
	propertyWindow_->loadProperty((EditorItem*)NULL);

	LevelScene* scene = (LevelScene*)sceneView()->scene();
	QUndoCommand* command = new DeleteInstanceCommand((EditorItem*)item, hashkey, scene);
	undoStack_->push(command);
}
