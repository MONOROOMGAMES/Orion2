#include "debugsetupwindow.h"
#include "ui_debugsetupwindow.h"
#include "configmanager.h"
#include "tooldefine.h"
#include <QFileDialog>
#include <QSettings>

DebugSetupWindow::DebugSetupWindow(QWidget *parent) :
QDialog(parent),
    ui(new Ui::DebugSetupWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	ui->iecomboBox->addItem(UDQ_T("Chrome"));
#ifdef Q_OS_MAC
	ui->iecomboBox->addItem(UDQ_T("Safari"));
#else
	ui->iecomboBox->addItem(UDQ_T("IE"));
#endif
	ui->iecomboBox->setCurrentIndex(-1);

	ui->rootlineEdit->setText(gConfigManager->serverRootPath());
	ui->portspinBox->setValue(gConfigManager->serverPort());
	ui->cmdlineEdit->setText(gConfigManager->explorerCmd());


}

DebugSetupWindow::~DebugSetupWindow()
{
    delete ui;
}

void DebugSetupWindow::on_savepushButton_clicked(){
	if (!canSave()){
		return;
	}

	gConfigManager->setServerRootPath(ui->rootlineEdit->text());
	gConfigManager->setServerPort(ui->portspinBox->value());
	gConfigManager->setExplorerCmd(ui->cmdlineEdit->text());

	accept();
}

void DebugSetupWindow::on_cancelpushButton_clicked(){
	reject();
}

bool DebugSetupWindow::canSave(){

	LINEEDIT_CHECK(ui->rootlineEdit, UDQ_TR("Ԥ����������"), UDQ_TR("��������Ŀ¼"));
	LINEEDIT_CHECK(ui->cmdlineEdit, UDQ_TR("Ԥ����������"), UDQ_TR("���������"));

	return true;
}

void DebugSetupWindow::on_pathpushButton_clicked(){
	QString dir = QFileDialog::getExistingDirectory(this, UDQ_TR("���ط�������Ŀ¼"),
		"",
		QFileDialog::ShowDirsOnly
		| QFileDialog::DontResolveSymlinks);

	ui->rootlineEdit->setText(dir);
}

void DebugSetupWindow::on_findpushButton_clicked(){

#ifdef Q_OS_MAC
	QString fileName = QFileDialog::getOpenFileName(this, UDQ_TR("���������"),
		"",
		UDQ_TR(" Ӧ�ó��� (*.app)"));
#else
	QString fileName = QFileDialog::getOpenFileName(this, UDQ_TR("���������"),
		"",
		UDQ_TR("Ӧ�ó��� (*.exe)"));
#endif

	ui->cmdlineEdit->setText(UDQ_TR("\"%1\"").arg(fileName) + UDQ_T(" \"%1\""));
}


static const QString Config_Chrome = UDQ_T("HKEY_CLASSES_ROOT\\ChromeHTML\\shell\\open\\command");
static const QString Config_IE = UDQ_T("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\App Paths\\IEXPLORE.EXE");

void DebugSetupWindow::on_iecomboBox_currentIndexChanged(int index){
	if (index < 0){
		ui->cmdlineEdit->clear();
	}
	else if (index == 0){
		QSettings settings(Config_Chrome, QSettings::NativeFormat);
		ui->cmdlineEdit->setText(settings.value(UDQ_T("Default")).toString());
	}
	else{
#ifdef Q_OS_MAC
		// ƻ��ϵͳ����β���Safari����

#else
		QSettings settings(Config_IE, QSettings::NativeFormat);
		ui->cmdlineEdit->setText(UDQ_TR("\"%1\"").arg(settings.value(UDQ_T("Default")).toString()) + UDQ_T(" \"%1\""));
#endif
	}

}