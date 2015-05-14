/************************************************************************
**
**  Copyright (C) 2014  guobin  <guobin.gb@alibaba-inc.com>
**
**  This file is part of Orion.
**
**  Orion is free software: you can redistribute it and/or modify
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
#include "addpicturewindow.h"
#include "ui_addpicturewindow.h"
#include <QMessageBox>
#include "tooldefine.h"


AddPictureWindow::AddPictureWindow(const QString& filePath, const QStringList& categories, QWidget *parent) :
QDialog(parent),
    ui(new Ui::AddPictureWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	// ���ذ��б�
	for (TResourceObjectIterator<PackageResourceObject> it; it; ++it){
		PackageResourceObject* pkg = (PackageResourceObject*)(*it);
		ui->pkgcomboBox->addItem(pkg->objectName(), pkg->hashKey());
	}

	//  ��Ŀ
	QStringList cates;
	cates.append(UDQ_TR("δ����"));
	cates.append(categories);
	ui->catecomboBox->addItems(cates);

	// �����ļ�
	ui->prevwidget->showPicture(filePath);

	QFileInfo fileInfo(filePath);
	ui->namelineEdit->setText(fileInfo.baseName());
	format_ = fileInfo.suffix();

	// ͼ����Ϣ
	ui->infolabel->setText(ui->prevwidget->acquireInfo());
}

AddPictureWindow::~AddPictureWindow()
{
    delete ui;
}

void  AddPictureWindow::on_cancelpushButton_clicked(){
	reject();
}

bool AddPictureWindow::canSave(void){
	if (ui->pkgcomboBox->currentIndex() < 0){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("��ָ��һ����Դ��!"));
		return false;
	}

	// ����Ƿ�Ϸ�
	LINEEDIT_VALID(ui->namelineEdit, windowTitle(), UDQ_TR("��ԴID"), REG_GALLERYID);

	// ����Ƿ����
	ImageResourceObject* image = TFindResource<ImageResourceObject>(
		ui->pkgcomboBox->currentText(),
		ui->namelineEdit->text(), false);

	if (image != NULL){
		// ͼ���Ѿ����ڣ���ʾ�Ƿ񸲸�
		if (QMessageBox::question(this, windowTitle(), UDQ_TR("ͼ���Ѿ����ڣ��Ƿ񸲸�?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No){
			return false;
		}
	}

	return true;
}

void  AddPictureWindow::on_savepushButton_clicked(){
	if (!canSave())
		return;

	// ���浽���ݿ���
	newImage_ = TFindResource<ImageResourceObject>(
		ui->pkgcomboBox->currentText(),
		ui->namelineEdit->text(), true);
	Q_ASSERT(newImage_ != 0);

	if (ui->catecomboBox->currentIndex() > 0){
		newImage_->categories_.append(ui->catecomboBox->currentText());
	}

	newImage_->tags_ = ui->taglineEdit->text().split(UDQ_T(";"));
	newImage_->setContent( ui->prevwidget->getPictureData());
	newImage_->format_ = format_;
	newImage_->isGroup_ = false;
	newImage_->frames_.clear();
	newImage_->frames_.append(QRect(0, 0, newImage_->image_.width(), newImage_->image_.height()));

	// ��������ͼ
	newImage_->image_ = ui->prevwidget->getPicture();
	newImage_->makeThumbnail();

	newImage_->clearFlags(URF_TagGarbage);
	newImage_->setFlags(URF_TagSave);

	// ���浽����
	SAVE_PACKAGE_RESOURCE(ImageResourceObject, newImage_);

	accept();

}
