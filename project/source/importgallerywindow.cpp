#include "importgallerywindow.h"
#include "ui_importgallerywindow.h"

#include <QUrl>
#include <QMessageBox>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QListWidget>
#include <QListWidgetItem>
#include <QFileDialog>

#include "tooldefine.h"
#include "imagehelper.h"
#include "resourceobject.h"
#include "resourcelinker.h"
using namespace ORION;

ImportGalleryWindow::ImportGalleryWindow(const QString& category, int filter, QWidget *parent) :
QDialog(parent), cate_(category),
    ui(new Ui::ImportGalleryWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	if (filter & FILTER_IMAGE){
		ui->imagecheckBox->setChecked(false);
		ui->imagecheckBox->setEnabled(false);
	}

	if (filter & FILTER_AUDIO){
		ui->audiocheckBox->setChecked(false);
		ui->audiocheckBox->setEnabled(false);
	}

	if (filter & FILTER_VIDEO){
		ui->videocheckBox->setChecked(false);
		ui->videocheckBox->setEnabled(false);
	}

	// ���ذ��б�
	for (TResourceObjectIterator<PackageResourceObject> it; it; ++it){
		PackageResourceObject* pkg = (PackageResourceObject*)(*it);
		ui->pkgcomboBox->addItem(pkg->objectName(), pkg->hashKey());
	}
}

ImportGalleryWindow::~ImportGalleryWindow()
{
    delete ui;
}

void ImportGalleryWindow::on_pathpushButton_clicked(){
	QString filePath = QFileDialog::getExistingDirectory(this, windowTitle(), QString());
	if (!filePath.isEmpty()){
		// ��ȡ����Ŀ¼��
		ui->urllineEdit->setText(filePath);

		// ����Ŀ¼�ļ�
		ui->filelistWidget->clear();

		// ����Ŀ¼������ͼƬ
		QVector<QString> dirNames;

		QFileInfoList filst;
		QFileInfoList::iterator curFi;
		QDir dir;

		//��ʼ��
		dirNames << filePath;

		//���������ļ��У�������Щ�ļ����е��ļ�ɾ��  
		for (int i = 0; i < dirNames.size(); ++i){
			dir.setPath(dirNames[i]);
			filst = dir.entryInfoList(QDir::Dirs | QDir::Files
				| QDir::Readable | QDir::Writable
				| QDir::Hidden | QDir::NoDotAndDotDot
				, QDir::Name);
			if (filst.size()>0){
				curFi = filst.begin();
				while (curFi != filst.end()){
					//�����ļ���,��������ļ����б�dirsβ��  
					if (curFi->isDir()){
						dirNames.push_back(curFi->filePath());
					}
					else if (curFi->isFile()){
						int  type = 0;
						//�����ļ�, �����չ��
						if (ImageResourceObject::isValidFile(curFi->suffix())){
							type  = FILTER_IMAGE;
						}
						else if (AudioResourceObject::isValidFile(curFi->suffix())){
							type = FILTER_AUDIO;
						}
						else if (VideoResourceObject::isValidFile(curFi->suffix())){
							type = FILTER_VIDEO;
						}

						if( type > 0){
							loadFiles_.append(curFi->absoluteFilePath());
							loadFile(curFi->absoluteFilePath(), type);
						}
					}
					curFi++;
				}
			}
		}
	}
}

void ImportGalleryWindow::loadFile(const QString& localFile, int type){

	QFileInfo info(localFile);

	QListWidgetItem* item = new QListWidgetItem();
	item->setText(info.baseName());
	item->setData(ITEM_DESC, (int)0);
	item->setData(ITEM_VALUE, localFile);
	item->setData(ITEM_CONTEXT, type);
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
	item->setCheckState(Qt::Checked);

	if (type == FILTER_IMAGE){
		// �����ļ�����ͼ
		QImage img(localFile);
		QImage thumb = thnumImage(128, 128, img);
		item->setIcon(QIcon(QPixmap::fromImage(thumb)));
		item->setHidden(!ui->imagecheckBox->isChecked());
	}
	else if (type == FILTER_AUDIO){
		item->setIcon(QIcon(UDQ_T(":/images/gallery_audio.png")));
		item->setHidden(!ui->audiocheckBox->isChecked());
	}
	else if (type == FILTER_VIDEO){
		item->setIcon(QIcon(UDQ_T(":/images/gallery_video.png")));
		item->setHidden(!ui->videocheckBox->isChecked());
	}

	ui->filelistWidget->addItem(item);
}


void ImportGalleryWindow::on_imagecheckBox_stateChanged(int state){
	bool visible = state == Qt::Checked;
	for (int i = 0; i < ui->filelistWidget->count(); i++){
		if (ui->filelistWidget->item(i)->data(ITEM_CONTEXT).toInt() == FILTER_IMAGE){
			ui->filelistWidget->item(i)->setHidden(!visible);
		}
	}
}

void ImportGalleryWindow::on_audiocheckBox_stateChanged(int state){
	bool visible = state == Qt::Checked;
	for (int i = 0; i < ui->filelistWidget->count(); i++){
		if (ui->filelistWidget->item(i)->data(ITEM_CONTEXT).toInt() == FILTER_AUDIO){
			ui->filelistWidget->item(i)->setHidden(!visible);
		}
	}
}

void ImportGalleryWindow::on_videocheckBox_stateChanged(int state){
	bool visible = state == Qt::Checked;
	for (int i = 0; i < ui->filelistWidget->count(); i++){
		if (ui->filelistWidget->item(i)->data(ITEM_CONTEXT).toInt() == FILTER_VIDEO){
			ui->filelistWidget->item(i)->setHidden(!visible);
		}
	}
}

void ImportGalleryWindow::on_cancelpushButton_clicked(){
	reject();
}

void ImportGalleryWindow::on_savepushButton_clicked(){

	QString pkg = ui->pkgcomboBox->currentText();
	if (pkg.isEmpty()){
		QMessageBox::critical(this, windowTitle(), UDQ_TR("��ָ��һ���زİ�!"));
		return;
	}

	bool overwrite = ui->overwritecheckBox->isChecked();

	int invalidItem = 0;
	// �����ݿ��е����ز�
	for (int i = 0; i < ui->filelistWidget->count(); i++){
		QListWidgetItem* item = ui->filelistWidget->item(i);
		if (!item->isHidden() && item->checkState() == Qt::Unchecked)
			continue;

		if (!vaildCheck(item, pkg, overwrite)){
			invalidItem++;
			item->setBackgroundColor(QColor(200, 75, 75));
			continue;
		}
		else{
			item->setBackgroundColor(QColor(75, 200, 75));
			item->setCheckState(Qt::Unchecked);
		}

		// ����ز�
		int fromMem = item->data(ITEM_DESC).toInt();
		if (fromMem == 0){
			QString localFile = item->data(ITEM_VALUE).toString();
			saveToDb(item, pkg, localFile);
		}
		else{
			QByteArray fileData = item->data(ITEM_VALUE).toByteArray();
			saveToDb(item, pkg, fileData, UDQ_T("png"));  //  Ŀǰ֧��pngͼƬ��ʽ
		}
	}

	if (invalidItem > 0){
		QMessageBox::information(this, windowTitle(), UDQ_TR("�����ز�ID���Ϸ�����Ǻ�ɫ�����޷�����!"));
	}
	else{
		QMessageBox::information(this, windowTitle(), UDQ_TR("��ѡ�ز�ȫ������ɹ�!"));
	}

}

void ImportGalleryWindow::on_selectallcheckBox_stateChanged(int state){
	for (int i = 0; i < ui->filelistWidget->count(); i++){
			ui->filelistWidget->item(i)->setCheckState((Qt::CheckState)state);
	}
}

// ��Ч�Լ��
bool ImportGalleryWindow::vaildCheck(QListWidgetItem* item, const QString& pkg, bool overwrite){

	QString name = item->text();
	// ��������Ƿ�Ϸ�
	QRegExp r(REG_GALLERYID);
	if (!r.exactMatch(name)){
		return false;
	}

	int type = item->data(ITEM_CONTEXT).toInt();

	if (type == FILTER_IMAGE){
		// ����Ƿ����
		ImageResourceObject* image = TFindResource<ImageResourceObject>(pkg, name, false);
		if (image != NULL && !overwrite){
			// ͼ���Ѿ����ڣ���ʾ�Ƿ񸲸�
			if (QMessageBox::question(this, windowTitle(), QString(UDQ_TR("ͼƬ[%1.%2]�Ѿ�����, �Ƿ񸲸ǣ�")).arg(pkg).arg(name)) == QMessageBox::No)
				return false;
		}
	}
	else if (type == FILTER_AUDIO){
		// ����Ƿ����
		AudioResourceObject* audio = TFindResource<AudioResourceObject>(pkg, name, false);
		if (audio != NULL && !overwrite){
			// ��Ƶ�Ѿ����ڣ���ʾ�Ƿ񸲸�
			if (QMessageBox::question(this, windowTitle(), QString(UDQ_TR(" ��Ƶ[%1.%2]�Ѿ�����, �Ƿ񸲸ǣ�")).arg(pkg).arg(name)) == QMessageBox::No)
				return false;
		}
	}
	else if (type == FILTER_VIDEO){
		// ����Ƿ����
		VideoResourceObject* video = TFindResource<VideoResourceObject>(pkg, name, false);
		if (video != NULL && !overwrite){
			// ͼ���Ѿ����ڣ���ʾ�Ƿ񸲸�
			if (QMessageBox::question(this, windowTitle(), QString(UDQ_TR("��Ƶ[%1.%2]�Ѿ�����, �Ƿ񸲸ǣ�")).arg(pkg).arg(name)) == QMessageBox::No)
				return false;
		}
	}

	return true;
}

void ImportGalleryWindow::saveToDb(QListWidgetItem* item, const QString& pkg, const QString& path){

	QByteArray data;
	QFile file(path);
	if (file.open(QIODevice::ReadOnly)){
		data = file.readAll();
		file.close();
	}
	saveToDb(item, pkg, data, QFileInfo(path).suffix());

}


void ImportGalleryWindow::saveToDb(QListWidgetItem* item, const QString& pkg, const QByteArray& data, const QString& originType){
	QString name = item->text();
	int type = item->data(ITEM_CONTEXT).toInt();

	if (type == FILTER_IMAGE){
		// ���浽���ݿ���
		ImageResourceObject* newImage = TFindResource<ImageResourceObject>(pkg, name, true);
		Q_ASSERT(newImage != 0);
			
		newImage->setContent( data );
		newImage->image_ = QImage::fromData(newImage->content_);
		newImage->format_ = originType;
		newImage->makeThumbnail();
		newImage->isGroup_ = false;
		newImage->frames_.clear();
		newImage->frames_.append(QRect(0, 0, newImage->image_.width(), newImage->image_.height()));

		// ���õ�ǰѡ�����Ŀ
		newImage->categories_.append(cate_);

		newImage->clearFlags(URF_TagGarbage);
		newImage->setFlags(URF_TagSave);

		// ���浽����
		SAVE_PACKAGE_RESOURCE(ImageResourceObject, newImage);
	}
	else if (type == FILTER_AUDIO){
		// ���浽���ݿ���
		AudioResourceObject* newAudio = TFindResource<AudioResourceObject>(pkg, name, true);
		Q_ASSERT(newAudio != 0);
		
		newAudio->setContent( data );
		newAudio->format_ = originType;
		newAudio->makeThumbnail();

		// ���õ�ǰѡ�����Ŀ
		newAudio->categories_.append(cate_);


		newAudio->clearFlags(URF_TagGarbage);
		newAudio->setFlags(URF_TagSave);

		// ���浽����
		SAVE_PACKAGE_RESOURCE(AudioResourceObject, newAudio);
	}
	else if (type == FILTER_VIDEO){
		VideoResourceObject* newVideo = TFindResource<VideoResourceObject>(pkg, name, true);
		Q_ASSERT(newVideo != 0);
	
		newVideo->setContent( data );
		newVideo->format_ = originType;
		newVideo->makeThumbnail();

		// ���õ�ǰѡ�����Ŀ
		newVideo->categories_.append(cate_);

		newVideo->clearFlags(URF_TagGarbage);
		newVideo->setFlags(URF_TagSave);

		// ���浽����
		SAVE_PACKAGE_RESOURCE(VideoResourceObject, newVideo);
	}
}


void ImportGalleryWindow::on_filepushButton_clicked(){
	QString file = QFileDialog::getOpenFileName(this, windowTitle(), QString(), UDQ_TR("EaselJS(*.json);;cocos2d(*.plist)"));
	if (!file.isEmpty()){
		// ��ȡ����Ŀ¼��
		ui->urllineEdit->setText(file);

		// ����Ŀ¼�ļ�
		ui->filelistWidget->clear();

		QFileInfo info(file);
		if (info.suffix().toLower() == UDQ_T("json")){
			// ����json�ļ�������json�ļ��������б���
			QJsonParseError error;
			QByteArray jsonData;
			QFile inFile(file);
			if (inFile.open(QIODevice::ReadOnly | QIODevice::Text)){
				jsonData = inFile.readAll();
				inFile.close();
			}

			if (jsonData.size() == 0){
				QMessageBox::warning(this, windowTitle(), UDQ_TR("�ļ�Ϊ��"));
				return;
			}

			// ��ȡ�ļ�����Ŀ¼
			QString fileDir = info.absolutePath();

			// TODO:
			// Ŀǰ�����ǵ��ļ���Ĭ����jsonͬ����������չΪ֧�ֶ��ļ�
			QString textureFile = fileDir + UDQ_T("/") + info.baseName() + UDQ_T(".png");

			// ����png�ļ�
			QImage img;
			img.load(textureFile);

			// ����json
			QJsonDocument document = QJsonDocument::fromJson(jsonData, &error);
			Q_ASSERT(error.error == QJsonParseError::NoError && document.isObject());

			QJsonObject obj = document.object();
			QJsonArray arr = obj.value(UDQ_T("frames")).toArray();

			for (int i = 0; i < arr.size(); i++){
				QJsonArray frame = arr[i].toArray();
				QRect bound;
				bound.setLeft(frame[0].toInt());
				bound.setTop(frame[1].toInt());
				bound.setWidth(frame[2].toInt());
				bound.setHeight(frame[3].toInt());

				// ��ԭʼͼ����ȡ����ͼƬ
				QImage itemImg = img.copy(bound);
				QByteArray data;
				{
					QBuffer buffer(&data);
					buffer.open(QIODevice::WriteOnly);
					itemImg.save(&buffer, "PNG");
				}

				QListWidgetItem* item = new QListWidgetItem();
				item->setText(info.baseName() + QString(UDQ_T("frame%1")).arg(i));
				item->setData(ITEM_DESC, (int)1);  // �ڴ�����
				item->setData(ITEM_VALUE, data);
				item->setData(ITEM_CONTEXT, (int)FILTER_IMAGE);
				item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);

				// �����ļ�����ͼ
				QImage thumb = thnumImage(128, 128, itemImg);
				item->setIcon(QIcon(QPixmap::fromImage(thumb)));
				item->setHidden(!ui->imagecheckBox->isChecked());
				ui->filelistWidget->addItem(item);

			}

		}

	}
}