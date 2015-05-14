#include "publishprojectwindow.h"
#include "ui_publishprojectwindow.h"
#include "commondefine.h"
#include "tooldefine.h"
#include "configmanager.h"
#include <QFileDialog>
#include "exporterfactory.h"
#include "exporth5.h"
#include <QProgressDialog>
#include "nwjssettingwindow.h"
#include "cordovasettingwindow.h"

PublishProjectWindow::PublishProjectWindow(ProjectResourceObject* project, QWidget *parent) :
QDialog(parent), project_(project),
    ui(new Ui::PublishProjectWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	init();

	if (project_->exportType_ > 0){
		ui->exportlineEdit->setText(gConfigManager->serverRootPath());
		ui->findpushButton->setEnabled(false);
	}

	if (project_->exportType_ == 1){
		ui->savepushButton->setText(UDQ_TR("Ԥ��"));
	}
	else	if (project_->exportType_ == 2){
		ui->savepushButton->setText(UDQ_TR("����"));
		// ��֧��HTML5
	}
}

PublishProjectWindow::~PublishProjectWindow()
{
    delete ui;
}

void PublishProjectWindow::init(){
	initPlatform();

	// ���ڴ�С
	ui->widthspinBox->setValue(project_->resolution_.width());
	ui->heightspinBox->setValue(project_->resolution_.height());

	if (project_->aspect_){  // ���ֳ����
		aspect_ = project_->resolution_.height() *1.0 / project_->resolution_.width();
		ui->heightspinBox->setReadOnly(true);
	}

	// ͼ���б�
	QStringList levels;
	foreach(LevelResourceObject* level, project_->levels_){
		if (level->flags() & URF_TagGarbage)
			continue;

		levels.append(level->description());
	}
	ui->levelcomboBox->addItems(levels);
	//
	ui->exportlineEdit->setText(project_->exportDir_);
	QString tmpstr;

	ui->imglineEdit->setText(project_->getDefMediaDir(MEDIA_IMAGE));
	ui->audiolineEdit->setText(project_->getDefMediaDir(MEDIA_AUDIO));
	ui->videolineEdit->setText(project_->getDefMediaDir(MEDIA_VIDEO));
	ui->misclineEdit->setText(project_->getDefMediaDir(MEDIA_MISC));
	ui->namelineEdit->setText(project_->getExportName());

}

void PublishProjectWindow::initPlatform(){
	QTreeWidgetItem* webRoot = new QTreeWidgetItem(ui->platformtreeWidget);
	webRoot->setText(0, UDQ_TR("��ҳ"));

	QTreeWidgetItem* platformH5 = new QTreeWidgetItem(webRoot, ITEM_PLATFORM);
	platformH5->setText(0, UDQ_TR("HTML5"));
	platformH5->setIcon(0, QIcon(UDQ_T(":/images/Platform_HTML5.png")));
	platformH5->setData(0, ITEM_VALUE, (int)Exporter::TYPE_HTML5);

	if (project_->exportType_ != 2){
		QTreeWidgetItem* mobileRoot = new QTreeWidgetItem(ui->platformtreeWidget);
		mobileRoot->setText(0, UDQ_TR("�ƶ��豸"));

		QTreeWidgetItem* platformCordova = new QTreeWidgetItem(mobileRoot, ITEM_PLATFORM);
		platformCordova->setText(0, UDQ_TR("Cordova"));
		platformCordova->setIcon(0, QIcon(UDQ_T(":/images/Platform_Cordova.png")));
		platformCordova->setData(0, ITEM_VALUE, (int)Exporter::TYPE_CORDOVA);

		QTreeWidgetItem* desktopRoot = new QTreeWidgetItem(ui->platformtreeWidget);
		desktopRoot->setText(0, UDQ_TR("�����豸"));

		QTreeWidgetItem* platformNode = new QTreeWidgetItem(desktopRoot, ITEM_PLATFORM);
		platformNode->setText(0, UDQ_TR("NW.js"));
		platformNode->setIcon(0, QIcon(UDQ_T(":/images/Platform_NodeWebkit.png")));
		platformNode->setData(0, ITEM_VALUE, (int)Exporter::TYPE_NWJS);
	}

	ui->platformtreeWidget->setCurrentItem(platformH5);
	ui->platformtreeWidget->expandAll();
}

bool PublishProjectWindow::canSave(){

	if (ui->levelcomboBox->currentIndex() < 0){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("����������������!"));
		return false;
	}

	QTreeWidgetItem* item = ui->platformtreeWidget->currentItem();
	if (item->type() != ITEM_PLATFORM){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("��ѡ��һ������ƽ̨!"));
		return false;
	}

	LINEEDIT_CHECK(ui->namelineEdit, UDQ_TR("����"), UDQ_TR("Ӧ������"));
	LINEEDIT_CHECK(ui->exportlineEdit, UDQ_TR("����"), UDQ_TR("����Ŀ¼"));
	COMBOX_CHECK(ui->fullscrcomboBox, UDQ_TR("����"), UDQ_TR("ȫ��ģʽ"));

	// ���ű��Ƿ�Ϊ�գ���������
	if (!project_->validScript()){
		if (QMessageBox::question(this, windowTitle(), UDQ_TR("��ǰ��Ŀ�нű�ģ�Ϳ��ܰ��������Ƿ����?")) == QMessageBox::No){
			return false;
		}
	}

	return true;
}

void PublishProjectWindow::on_savepushButton_clicked(){
	if (!canSave()){
		return;
	}

	// ��������
	project_->resolution_.setWidth(ui->widthspinBox->value());
	project_->resolution_.setHeight(ui->heightspinBox->value());
	project_->startLevel_ = ui->levelcomboBox->currentText();
	project_->fullscreenMode_ = ui->fullscrcomboBox->currentIndex();
	project_->fullscreenQuality_ = ui->fsqualitycomboBox->currentIndex();
	project_->downScaling_ = ui->downscalecomboBox->currentIndex();
	project_->orientation_ = ui->orientcomboBox->currentIndex();
	project_->sampleType_ = ui->samplecomboBox->currentIndex();
	project_->physicalEngine_ = ui->phyenginecomboBox->currentIndex();
	project_->loaderStyle_ = ui->loadercomboBox->currentIndex();

	project_->useLoaderLayout_ = ui->loadercheckBox->isChecked();
	project_->useHighDPI_ = ui->dpicheckBox->isChecked();
	project_->useWebGL_ = ui->webglcheckBox->isChecked();
	project_->pixelRounding_ = ui->roundcheckBox->isChecked();
	project_->preloadSound_ = ui->audiocheckBox->isChecked();
	project_->pauseOnUnfocus_ = ui->focuscheckBox->isChecked();
	project_->clearBackground_ = ui->clearbgcheckBox->isChecked();

	project_->exportDir_ = ui->exportlineEdit->text();
	project_->mediaDirs_.insert(MEDIA_IMAGE, ui->imglineEdit->text());
	project_->mediaDirs_.insert(MEDIA_AUDIO, ui->audiolineEdit->text());
	project_->mediaDirs_.insert(MEDIA_VIDEO, ui->videolineEdit->text());
	project_->mediaDirs_.insert(MEDIA_MISC, ui->misclineEdit->text());
	project_->mediaDirs_.insert(MEDIA_PROJECT, ui->namelineEdit->text());

	project_->setFlags(URF_TagSave);

	// ����
	QTreeWidgetItem* item = ui->platformtreeWidget->currentItem();
	Q_ASSERT(item->type() == ITEM_PLATFORM);
	platform_ = item->data(0, ITEM_VALUE).toInt();

	if (platform_ == Exporter::TYPE_CORDOVA && ( project_->domainId_.isEmpty() ||project_->pubVersion_.isEmpty() ) ){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("��ĿID���汾��Ϣ����Ϊ�գ��벹�����������µ���"));
		return;
	}

	Exporter* exporter = gExporterFactory->getExporter(platform_, project_);

	//////////////////////////////////////////////////////////////////////////////
	////// ����ģ���
	//QString dbFile = QString(UDQ_T("%1/Exporter/%2.db")).arg(QApplication::applicationDirPath()).arg(exporter->name());
	//QString srcFile = QString(UDQ_T("%1/Exporter/%2")).arg(QApplication::applicationDirPath()).arg(exporter->name());
	//exporter->makeTemplateDB(dbFile, srcFile);

	if (exporter){

		//////////////////////////////////////////////////////////////////////////
		//  ���db�ļ��Ƿ���ڣ�������������˳�
		QString dbPath = QString(UDQ_T("%1/Exporter/%2.db")).arg(QApplication::applicationDirPath()).arg(exporter->name());
		if (!QFile::exists(dbPath)){
			QMessageBox::warning(this, windowTitle(), UDQ_TR("�������ݿ��ļ�<%1>��ʧ���޷�����!").arg(dbPath));
			delete exporter;
			return;
		}

		if (platform_ == Exporter::TYPE_NWJS){
			// ����NW���öԻ���
			NWJSSettingWindow w((NWJSExporter*)exporter);
			if (w.exec() == QDialog::Rejected){
				delete exporter;
				return;
			}
		}
		else if (platform_ == Exporter::TYPE_CORDOVA){
			CordovaSettingWindow w((CordovaExporter*)exporter);
			if (w.exec() == QDialog::Rejected){
				delete exporter;
				return;
			}
		}

		ui->progressBar->setValue(0);
		connect(exporter, SIGNAL(progress(int)), ui->progressBar, SLOT(setValue(int)));
	
		exporter->write();
		ui->progressBar->setValue(100);

		delete exporter;

		QMessageBox::information(this, windowTitle(), UDQ_TR("�������!"));
	}
	else{
		QMessageBox::warning(this, windowTitle(), UDQ_TR("Ŀǰ��δ֧�֣�����ȴ�!"));
		return;
	}

	accept();
}

void PublishProjectWindow::on_cancelpushButton_clicked(){
	reject();
}

void PublishProjectWindow::on_findpushButton_clicked(){
	QString dir = QFileDialog::getExistingDirectory(this, UDQ_TR("ѡ��Ŀ¼"),
		gConfigManager->baseWorkspacePath(), QFileDialog::ShowDirsOnly| QFileDialog::DontResolveSymlinks);

	if (dir.isEmpty())
		return;

	ui->exportlineEdit->setText(dir);
}

void PublishProjectWindow::on_widthspinBox_valueChanged(int i){
	if (project_->aspect_){
		// ���ֳ����
		ui->heightspinBox->setValue(qRound(i * aspect_));
	}
}