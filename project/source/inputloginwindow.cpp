#include "inputloginwindow.h"
#include "ui_inputloginwindow.h"
#include "configmanager.h"
#include "commondefine.h"
#include <QMessageBox>

InputLoginWindow::InputLoginWindow(QWidget *parent) :
QDialog(parent),
    ui(new Ui::InputLoginWindow)
{
    ui->setupUi(this);
	setFixedSize(size());
	connect(ui->loginwidget, SIGNAL(inputPwd(const QByteArray&)), this, SLOT(processAuthorization(const QByteArray&)));
	updateLabel();

}

InputLoginWindow::~InputLoginWindow()
{
    delete ui;
}

void InputLoginWindow::processAuthorization(const QByteArray& data){

	if (data.size() == 0){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("����ͼ��������Ҫ������������"));
		firstCode_.clear();
		ui->loginwidget->setLock(true);
	}
	else{

		if (firstCode_.size() == 0){
			firstCode_ = data;
			ui->loginwidget->setLock(true);
		}
		else {
			if (firstCode_ != data){
				firstCode_.clear();
				ui->loginwidget->setLock(true);
			}
			else{
				// ����code
				gConfigManager->setLoginCode(firstCode_);
				accept();
			}
		}

	}

	updateLabel();

}

void InputLoginWindow::updateLabel(){

	if (firstCode_.size() == 0){
		ui->infolabel->setText(UDQ_TR("�״�ʹ��, ���������ͼ������������4���㣩"));
	}
	else{
		ui->infolabel->setText(UDQ_TR("���ٴ��������ͼ������������4���㣩"));
	}

}
