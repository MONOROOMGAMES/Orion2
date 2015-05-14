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

#include "welcomewindow.h"
#include "ui_welcomewindow.h"
#include "tooldefine.h"
#include "configmanager.h"
#include "projectmanager.h"
#include "resourceobject.h"
#include "resourcelinker.h"
#include "framemanager.h"
#include "mainmanagerframe.h"
#include <QFileDialog>
using namespace ORION;

WelcomeWindow::WelcomeWindow(QWidget *parent) :
QDialog(parent),
    ui(new Ui::WelcomeWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	defaultIcon_ = QIcon(UDQ_T(":/images/default_project.png"));
	prepareNewProjectList();

	int num = prepareExistProjectList();
	if ( num == 0 ){
		ui->newpushButton->setChecked(true);
		ui->stackedWidget->setCurrentIndex(0);
	}
}

WelcomeWindow::~WelcomeWindow()
{
    delete ui;
}

void  WelcomeWindow::on_browserpushButton_clicked(){
	// ��Ŀ¼����Ŀ
	QString pathName = QFileDialog::getExistingDirectory(this, UDQ_TR("ѡ����ĿĿ¼"));
	if (pathName.isEmpty()){
		return;
	}

	// ����Ŀ
	if (openProjectFromDir(pathName)){
		emit openProject();
		accept();
	}

}

void WelcomeWindow::on_namelineEdit_textChanged(const QString & text){
	// ͬ��������Ŀ·��
	QString path = QString(UDQ_T("%1/%2")).arg(gConfigManager->baseWorkspacePath()).arg(hashString(text));
	ui->pathlineEdit->setText(path);
}

void  WelcomeWindow::on_refreshpushButton_clicked(){
	// ˢ�µ�ǰĿ¼��������Ŀ�б�
	prepareExistProjectList();

}

void  WelcomeWindow::on_projectpushButton_clicked(){
	ui->stackedWidget->setCurrentIndex(1);
}

void  WelcomeWindow::on_newpushButton_clicked(){
	ui->stackedWidget->setCurrentIndex(0);
}

bool WelcomeWindow::openProjectFromDir(const QString& path, const QString& name){
	// �رյ�ǰ�����ռ�
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);
	// �رյ�ǰ�����ռ�
	frame->closeCurrentWorkspace();

	QString wsName = name;
	if (wsName.isEmpty()){
		QDir dir(path);  // ��ʱ��Ŀ¼��Ϊ�ռ�������loadConfig���޸�Ϊ��ȷ�ռ���
		wsName = dir.dirName();
	}
	// ���������ռ�
	WorkspaceResourceObject* ws = ResourceObject::createWorkspace(wsName);
	Q_ASSERT(ws != 0);
	ws->dirPath_ = path;
	// �������ļ��л�ȡ����ͼ
	bool r = ws->loadConfig();

	if (!r){
		QMessageBox::warning(this, UDQ_TR("����Ŀ"), UDQ_TR("����Ŀ�ļ�ʧ�ܣ������ļ��Ƿ�����!"));
		ResourceObject::resetWorkspace(ws);
		return false;
	}

	//  ���ؿռ䣨������ݿ��ʼ����
	ResourceObject::loadWorkspace(ws, RLF_LoadIndex);

	// ��������飬�����빤���ռ���ͬ
	ProjectResourceObject*  proj = ws->findProject(ws->objectName(), true);
	Q_ASSERT(proj != NULL);
	Q_ASSERT(proj->getLinker() != NULL);

	// ���¼��أ��������ݣ�
	ResourceLinker* linker = proj->getLinker();
	Q_ASSERT(linker != 0);
	linker->load<ProjectResourceObject>(proj->hashKey(), RLF_None);

	ws->activiteProject(proj->hashKey());

	return true;
}

void WelcomeWindow::on_projectlistWidget_itemDoubleClicked(QListWidgetItem * item){
	if (item == NULL)
		return;

	QString path = item->data(ITEM_CONTEXT).toString();
	if (openProjectFromDir(path, item->text())){
		emit openProject();
		accept();
	}

}

// ׼������Ŀ�б�
void WelcomeWindow::prepareNewProjectList(){
	ui->typelistWidget->clear();

	foreach (unsigned int key, gProjectManager->projectTypes())
	{
		QStringList info = gProjectManager->projectInfo(key);
		if (info.size() >= 6){
			QListWidgetItem* project = new QListWidgetItem();
			project->setIcon(QIcon(UDQ_T(":/images/") + info[1]));
			project->setText(info[0]);
			project->setData(ITEM_HASHKEY, key);
			project->setToolTip(info[2]);

			QSize s;
			s.setWidth(info[3].toUInt());
			s.setHeight(info[4].toUInt());
			project->setData(ITEM_SCREEN, s);
			project->setData(ITEM_ASPECT, info[5].toUInt());
			ui->typelistWidget->addItem(project);
		}
	}
}

void WelcomeWindow::on_createpushButton_clicked(){

	if (!checkValid())
		return;

	QListWidgetItem* item = ui->typelistWidget->currentItem();
	Q_ASSERT(item != NULL);
	// ��Ŀ����

	// ���������ռ�
	WorkspaceResourceObject* ws = ResourceObject::createWorkspace(ui->namelineEdit->text());
	Q_ASSERT(ws != 0);
	ws->dirPath_ = ui->pathlineEdit->text();
	ws->projectType_ = item->data(ITEM_HASHKEY).toUInt();

	// ���������ļ���project.cfg��
	ws->saveConfig();

	//  ���ؿռ䣨������ݿ��ʼ����
	ResourceObject::loadWorkspace(ws, RLF_None);

	// �����µ����飬�����빤���ռ���ͬ
	ProjectResourceObject*  proj = ws->findProject(ws->objectName(), true);
	Q_ASSERT(proj != NULL);
	proj->setFlags(URF_TagSave);

	// ��������������Ŀȱʡ����
	// ��Ŀ�ߴ�
	proj->resolution_ = item->data(ITEM_SCREEN).toSize();
	// �Ƿ�̶������
	proj->aspect_ = item->data(ITEM_ASPECT).toUInt() == 1;

	proj->createDate_ = QDateTime::currentDateTime();
	ws->activiteProject(proj->hashKey());
	SAVE_WORKSPACE_RESOURCE(ProjectResourceObject, proj);

	// ������Ŀ�б�
	prepareExistProjectList();

	// ������Ŀ
	emit openProject();
	accept();
}

bool WelcomeWindow::checkValid(){
	QListWidgetItem* item = ui->typelistWidget->currentItem();
	if (item == NULL){
		QMessageBox::warning(this, UDQ_TR("�½���Ŀ"), UDQ_TR("��ѡ��һ����Ŀ����!"));
		return false;
	}

	LINEEDIT_CHECK(ui->pathlineEdit, UDQ_TR("�½���Ŀ"), UDQ_TR("��Ŀ·��"));
	LINEEDIT_CHECK(ui->namelineEdit, UDQ_TR("�½���Ŀ"), UDQ_TR("��Ŀ��"));

	// ��鹤���ռ��Ƿ����
	QString wsName = ui->namelineEdit->text();
	if (wsName.indexOf(UDQ_T("@")) >= 0){
		QMessageBox::warning(this, UDQ_TR("�½���Ŀ"), UDQ_TR("��Ŀ���в��ܰ���@�ַ�!").arg(wsName));
		return false;
	}

	// ���Ŀ¼���Ƿ�Ϊ��
	QString path = ui->pathlineEdit->text();
	QDir dir(ui->pathlineEdit->text());

	if (dir.exists()){
		QStringList children = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs | QDir::Files);
		if (children.size() > 0){
			if (QMessageBox::question(this, UDQ_TR("�½���Ŀ"), UDQ_TR("��ĿĿ¼[ %1 ]��Ϊ��,�Ƿ����(Ŀ¼�����)?").arg(dir.absolutePath())) == QMessageBox::No)
				return false;

			// ���Ŀ¼
			deleteDir(dir.absolutePath(), false);
		}
	}
	else{
		if (!dir.mkpath(path)){
			QMessageBox::warning(this, UDQ_TR("�½���Ŀ"), UDQ_TR("��Ŀ Ŀ¼[%1]����ʧ��!").arg(dir.absolutePath()));
			return false;
		}
	}

	return true;

}


int  WelcomeWindow::prepareExistProjectList(){
	ui->projectlistWidget->clear();

	QString wspath = gConfigManager->baseWorkspacePath();
	Q_ASSERT(!wspath.isEmpty());

	int projnum = 0;
	// �ݹ�Ŀ¼
	QDir dir(wspath);
	if (dir.exists()){
		// ���ļ���������ͼ
		WorkspaceResourceObject* ws = ResourceObject::tempWorkspace();

		QStringList children = dir.entryList(QDir::NoDotAndDotDot | QDir::Dirs );
		if (children.size() > 0){

			QString projPath;
			foreach(QString name, children){

				projPath = wspath + UDQ_T("/") + name;
				ws->dirPath_ = projPath;
				if (ws->loadConfig()){
					QListWidgetItem* item = new QListWidgetItem();
					item->setText(ws->tmpName_);
					item->setIcon(defaultIcon_);
					item->setData(ITEM_DESC, ws->description());
					item->setData(ITEM_CONTEXT, projPath);
					ui->projectlistWidget->addItem(item);
					projnum++;
				}
			}
		
		}
	}

	return projnum;
}