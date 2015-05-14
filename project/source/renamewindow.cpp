#include "renamewindow.h"
#include "ui_renamewindow.h"
#include <QMessageBox>

RenameWindow::RenameWindow(ResourceObject* host, bool createNew , QWidget *parent) :
QDialog(parent), host_(host), createNew_(createNew), newObj_(NULL),
    ui(new Ui::RenameWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	if (createNew){
			setWindowTitle(UDQ_TR("����"));
	}

	QRegExp rx(REG_NAMEASCII);
	QValidator *validator = new QRegExpValidator(rx, this);
	ui->renamelineEdit->setValidator(validator);
	ui->renamelineEdit->setText(host->objectName());
}

RenameWindow::~RenameWindow()
{
    delete ui;
}

void RenameWindow::on_okpushButton_clicked()
{
	QString newName = ui->renamelineEdit->text();

	// ���Ϸ���
	if (newName.isEmpty()){
		QMessageBox::warning(this, windowTitle(), tr("���Ʋ���Ϊ��.\n"));
		return;
	}


	// ����Ƿ���ͬ
	if (newName == host_->objectName()){
		accept();
		return;
	}

	// ���Ψһ��
	QString path = host_->outer() ? host_->outer()->pathName() : UDQ_T("");
	QString tmpPath = QString(UDQ_T("%1@%2.%3")).arg(host_->getRTTI()->getName()).arg(path).arg(newName);

	if (ResourceObject::findResource(hashString(tmpPath))){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("����������������������.\n"));
		return;
	}

	if (createNew_){
		// �����¶���
		newObj_ = host_->clone(newName);
	}
	else{
		// ִ���޸�
		host_->rename(newName);
		host_->setFlags(URF_TagSave);
	}

	accept();
}