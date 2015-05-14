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

#include "mainspriteframe.h"
#include "mainmanagerframe.h"
#include "spritescene.h"
#include "menumanager.h"
#include "renamewindow.h"
#include "configmanager.h"
#include "framemanager.h"

#include <queue>
#include <boost/shared_ptr.hpp>

MainSpriteFrame::MainSpriteFrame(QWidget *parent ) : QMainWindow(parent), editorView_(NULL) {
	init();
}

MainSpriteFrame::~MainSpriteFrame(){

}


void MainSpriteFrame::init(){

	// ���ô���ͼ��ͱ���
	setWindowIcon( QIcon( UDQ_T(":/images/unique64.png") ) );
	setWindowTitle( UDQ_T("Orion2 Sprite Editor") );

	initSceneView();

	createActions();
	createMenus();
	createToolBars();
	createStatusBar();
	createDockWindows();


}

// ��ʼ������
void MainSpriteFrame::initSceneView(){

	if( editorView_ == NULL ){
		editorView_ = new SpriteView();
		setCentralWidget( editorView_ );
	}

	// �����ź�
	QGraphicsScene* oldScence = editorView_->scene();
	SpriteScene* scene = new SpriteScene();
	editorView_->setScene( scene );

	// �����ź�
	connect(scene, SIGNAL(error(const QString&)), this, SLOT(errorMessage(const QString&)));

	// �ͷ��ϵ�ͼ
	if( oldScence != 0 ){
		delete oldScence;
	}
}

void MainSpriteFrame::createActions(){

	gMenuManager->createActions( EDITOR_SPRITE, this );

	BEGIN_ACTION();
	//////////////////////////////////////////////////////////////////////////
	// Scenario
	//////////////////////////////////////////////////////////////////////////
	CONNECT_ACTION(ORION_SPRTIE_SAVE, this, submitCurrentSprite);
	CONNECT_ACTION(ORION_SPRTIE_SAVEALL, this, submitAllSprite);

	CONNECT_ACTION(ORION_SPRTIE_REFRESH, this, refreshCurrentSprite);
	CONNECT_ACTION(ORION_SPRTIE_DELETE, this, deleteSelectedItem);

	CONNECT_ACTION(ORION_SPRTIE_ADDSEQ, this, addSequenceItem);
	CONNECT_ACTION(ORION_SPRTIE_DELSEQ, this, removeSequenceItem);
	CONNECT_ACTION(ORION_SPRTIE_RENAMESEQ, this, renameSequenceItem);

	CONNECT_ACTION(ORION_SPRITE_DEFAULTSEQ, this, defaultSequenceItem);

	/*modeGroup_ = new QActionGroup(this);
	modeGroup_->addAction( gMenuManager->getAction(FORCE_EDITOR_POINTER) );
	modeGroup_->addAction( gMenuManager->getAction(FORCE_EDITOR_MODELGROUP) );*/
	//gMenuManager->getAction(FORCE_EDITOR_POINTER)->setChecked(true);

	//////////////////////////////////////////////////////////////////////////
	// View
	//////////////////////////////////////////////////////////////////////////
	END_ACTION();

	QGraphicsScene* scene = editorView_->scene();
	Q_ASSERT( scene != NULL );
	connectSceneEvent( scene );

}

void MainSpriteFrame::connectSceneEvent(QGraphicsScene* scene){
	BEGIN_ACTION();

	CONNECT_ACTION(ORION_SPRTIE_COPYANCHOR, scene, applyAnchorToSequence);

	END_ACTION();
}


void MainSpriteFrame::createMenus(){

}


void MainSpriteFrame::createToolBars(){

}

void MainSpriteFrame::createStatusBar(){
	statusBar()->showMessage(UDQ_TR("����"));
}

void MainSpriteFrame::createDockWindows(){

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	structDockWidget_ = new QDockWidget(this);
	structDockWidget_->setAllowedAreas(Qt::AllDockWidgetAreas);
	structDockWidget_->setFeatures(QDockWidget::NoDockWidgetFeatures);
	addDockWidget(Qt::RightDockWidgetArea, structDockWidget_);

	structWindow_ = new SpriteStructWindow(structDockWidget_);
	structDockWidget_->setWidget(structWindow_);
	structDockWidget_->setWindowTitle(structWindow_->windowTitle());

	connect(structWindow_, SIGNAL(deleteSprite(unsigned int)), this, SLOT(deleteSprite(unsigned int)));

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	galleryDockWidget_ = new QDockWidget(this);
	galleryDockWidget_->setAllowedAreas(Qt::RightDockWidgetArea);
	galleryDockWidget_->setFeatures(/*QDockWidget::NoDockWidgetFeatures | */
		QDockWidget::DockWidgetMovable);
	addDockWidget(Qt::RightDockWidgetArea, galleryDockWidget_);

	galleryWindow_ = new SpriteGalleryWindow(galleryDockWidget_);
	galleryDockWidget_->setWidget(galleryWindow_);
	galleryDockWidget_->setWindowTitle(galleryWindow_->windowTitle());

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	tileDockWidget_ = new QDockWidget(this);
	tileDockWidget_->setAllowedAreas(Qt::RightDockWidgetArea );
	tileDockWidget_->setFeatures(/*QDockWidget::NoDockWidgetFeatures | */
		QDockWidget::DockWidgetMovable);
	addDockWidget(Qt::RightDockWidgetArea, tileDockWidget_);

	tilesetWindow_ = new TileSetWindow(tileDockWidget_);
	tileDockWidget_->setWidget(tilesetWindow_);
	tileDockWidget_->setWindowTitle(tilesetWindow_->windowTitle());



}

void MainSpriteFrame::closeEvent(QCloseEvent *event){

}

bool MainSpriteFrame::saveCurrentSprite(){

	// ���浱ǰ����
	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	Q_ASSERT(scene != NULL);

	// ���浽���ݿ���
	SpriteResourceObject* sprite = scene->spriteObject();
	if (sprite == NULL){
		return false;
	}

	// ������һ������
	scene->save();
	return true;

}

void MainSpriteFrame::submitCurrentSprite(){

	// �ȱ���
	if ( !saveCurrentSprite()){
		return;
	}

	// ���浱ǰ����
	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	Q_ASSERT(scene != NULL);

	// ���浽���ݿ���
	SpriteResourceObject* sprite = scene->spriteObject();

	// ���浽����
	SAVE_PACKAGE_RESOURCE(SpriteResourceObject, sprite);
	QMessageBox::information(this, windowTitle(), UDQ_TR("����[%1]����ɹ�!").arg(sprite->description()));

}

void MainSpriteFrame::submitAllSprite(){

	// ���浱ǰ����
	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	Q_ASSERT(scene != NULL);
	scene->save();

	// �������п�
	ApplyResourceLinkers<SpriteResourceObject>();
	
	// 
	QMessageBox::information(this, windowTitle(), UDQ_TR("���о�����󱣴����!"));
}

void MainSpriteFrame::refreshCurrentSprite(){
	// ˢ�µ�ǰ���飬��ΪͼƬ�ı䣬���¶���֡��ͼƬ����
	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	Q_ASSERT(scene != NULL);

	// ��������
	scene->refreshSequences();

}



void MainSpriteFrame::deleteSelectedItem(){

	if (QMessageBox::question(this, windowTitle(), UDQ_TR("ȷ��ɾ��ѡ�еĶ��󣨲��ɻָ���?")) == QMessageBox::No){
		return;
	}

	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	Q_ASSERT(scene != NULL);

	scene->removeSelectedObjects();
}


void MainSpriteFrame::removeSequenceItem(){

	if (QMessageBox::question(this, windowTitle(), UDQ_TR("ȷ��ɾ��ѡ�еĶ������У����ɻָ���?")) == QMessageBox::No){
		return;
	}
	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	Q_ASSERT(scene != NULL);
	scene->removeSelectedSequence();
}

void MainSpriteFrame::applyAnchorToSequence(){

}


void MainSpriteFrame::addSequenceItem(){

	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	Q_ASSERT(scene != NULL);

	// ���浽���ݿ���
	SpriteResourceObject* sprite = scene->spriteObject();
	if (sprite == NULL){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("���ȴ�һ���������Ȼ����Ӷ�������"));
		return;
	}

	// ����ǵ�һ������
	QString seqName;
	if (sprite->sequences_.size() == 0){
		seqName = UDQ_T("Default");
		sprite->defaultSeq_ = seqName;
	}

	// ���浽���ݿ���
	SpriteSequenceResourceObject* seq = TAddChildObject<SpriteSequenceResourceObject>(sprite, seqName);
	Q_ASSERT(seq != 0);

	QRectF rect = scene->viewRegionRect();
	seq->pos_ = rect.center();

	sprite->setFlags(URF_TagSave);
	scene->addSequenceItem(seq);

}

void MainSpriteFrame::defaultSequenceItem(){
	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	Q_ASSERT(scene != NULL);

	SpriteSequenceResourceObject* seq = scene->currentSequence();
	if (seq != NULL){
			//  �޸�ȱʡ����
			SpriteResourceObject* sprite = (SpriteResourceObject*)seq->outer();
			sprite->defaultSeq_ = seq->description();
			sprite->setFlags(URF_TagSave);
	}
}

void MainSpriteFrame::renameSequenceItem(){
	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	Q_ASSERT(scene != NULL);

	SpriteSequenceResourceObject* seq = scene->currentSequence();
	if (seq != NULL){
		QString oldName = seq->description();
		RenameWindow w(seq, false);
		if (w.exec() == QDialog::Accepted){
			//  �޸�ȱʡ����
			SpriteResourceObject* sprite = (SpriteResourceObject*)seq->outer();
			if (sprite->defaultSeq_ == oldName){
				sprite->defaultSeq_ = seq->description();
			}
			sprite->setFlags(URF_TagSave);
		}
	}
}



void  MainSpriteFrame::loadSprite(ResourceHash key, const QString& title){
	DEFINE_RESOURCE_HASHKEY(SpriteResourceObject, sprite, key);
	if (sprite == NULL){
		QMessageBox::critical(this, windowTitle(), UDQ_TR("��ǰѡ��ľ���[%1]����û���ҵ�!").arg(title));
		return;
	}

	tilesetWindow_->clear();
	galleryWindow_->init(sprite);

	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	Q_ASSERT(scene != NULL);

	// ������һ������
	scene->save();

	//  �����¾���
	scene->load(sprite);
}

void MainSpriteFrame::errorMessage(const QString& msg){
	QMessageBox::critical(this, windowTitle(), msg);
}

void MainSpriteFrame::updateSpriteName(){
	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	scene->updateAnimText();
}

void MainSpriteFrame::deleteSprite(unsigned int key){
	//  �����ǰ�������ڱ༭����ձ༭��
	SpriteScene* scene = (SpriteScene*)editorView_->scene();
	SpriteResourceObject* cursprite = scene->spriteObject();
	if (cursprite && cursprite->hashKey() == key){
		scene->clearSprite();
		tilesetWindow_->clear();
		galleryWindow_->clear();
	}

	// �������еĶ�������
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	if (ws){
		// ��ȡ��ǰ������
		ProjectResourceObject* proj = ws->currentProject();
		if (proj != NULL){
			foreach(ObjectTypeResourceObject* obj, proj->objTypes_){
				if (obj->flags() & URF_TagGarbage)
					continue;

				if (obj->animation_ &&obj->animation_->hashKey() == key){  // ��������Ϊɾ���Ķ���
					obj->clearImageResource();
				}
			}
		}
	}

	// ֪ͨ��ǰ�༭�����и���
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);
	frame ->refreshLayers();

	// ����ɾ������
	DEFINE_RESOURCE_HASHKEY_VALID(SpriteResourceObject, sprite, key);
	SAVE_PACKAGE_RESOURCE(SpriteResourceObject, sprite);

}