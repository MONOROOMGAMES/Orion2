#include "sounddefinewindow.h"
#include "ui_sounddefinewindow.h"
#include "selectaudiowindow.h"
#include "tooldefine.h"

SoundDefineWindow::SoundDefineWindow(SoundResourceObject* sound, QWidget *parent) :
QDialog(parent), sound_(sound),
    ui(new Ui::SoundDefineWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	// ���ذ��б�
	for (TResourceObjectIterator<PackageResourceObject> it; it; ++it){
		PackageResourceObject* pkg = (PackageResourceObject*)(*it);
		ui->pkgcomboBox->addItem(pkg->objectName(), pkg->hashKey());
	}


	if (sound_){
		ui->namelineEdit->setText(sound_->description());
		// �����ļ��б�

		QList<ResourceHash> valids;
		foreach(ResourceHash key, sound_->audios_){
			DEFINE_RESOURCE_HASHKEY(AudioResourceObject, audio, key);
			if (audio){
				valids.append(key);
				addAudioItem(audio);
			}
		}
		sound_->audios_ = valids;
	}
}

SoundDefineWindow::~SoundDefineWindow()
{
    delete ui;
}


void SoundDefineWindow::on_savepushButton_clicked(){
	QString name = ui->namelineEdit->text();
	if (name.isEmpty()){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("���Ʋ���Ϊ��"));
		return;
	}

	QString pkgName = ui->pkgcomboBox->currentText();
	if (pkgName.isEmpty()){
		pkgName = DEFAULT_PACKAGE;
	}


	if (sound_ == NULL){
		// �½�һ������
		DEFINE_RESOURCE_PATH(PackageResourceObject, pkgObj, pkgName);
		Q_ASSERT(pkgObj != NULL);

		// ���浽���ݿ���
		sound_ = TCreateResource<SoundResourceObject>(pkgObj, UDQ_T(""));
		Q_ASSERT(sound_ != NULL);
	}
	

	// �������
	for (TResourceObjectIterator<SoundResourceObject> it; it; ++it){
		SoundResourceObject* sound = (SoundResourceObject*)(*it);
		if (sound->description() == name){
			QMessageBox::warning(this, windowTitle(), UDQ_TR("���������������޸ĺ󱣴�!"));
			return;
		}
	}

	sound_->setDescription(ui->namelineEdit->text());
	sound_->tags_ = ui->taglineEdit->text().split(UDQ_T(";"));

	for (int i = 0; i < ui->listWidget->count(); i++){
		ResourceHash key = ui->listWidget->item(i)->data(ITEM_HASHKEY).toUInt();
		sound_->audios_.append(key);
	}

	sound_->setFlags(URF_TagSave);
	SAVE_PACKAGE_RESOURCE(SoundResourceObject, sound_);

	accept();
}

void SoundDefineWindow::on_cancelpushButton_clicked(){
	reject();
}

void SoundDefineWindow::on_filepushButton_clicked(){
	// ���ļ�������Ƶ�ļ�

	// �������Ƶ

}

void SoundDefineWindow::on_gallerypushButton_clicked(){
	// ����Ƶ�ز�
	SelectAudioWindow w;
	if (w.exec() == QDialog::Accepted){
		DEFINE_RESOURCE_HASHKEY_VALID(AudioResourceObject, audio, w.audioHash_);		
		addAudioItem(audio);
	}
}

void SoundDefineWindow::addAudioItem(AudioResourceObject* audio){
	// ����Ƿ��Ѿ�����
	QListWidgetItem* item = NULL;
	for (int i = 0; i < ui->listWidget->count(); i++){
		ResourceHash key = ui->listWidget->item(i)->data(ITEM_HASHKEY).toUInt();
		if (key == audio->hashKey()){
			return;  // �Ѿ�����
		}
	}

	item = new QListWidgetItem();
	item->setText(UDQ_TR("%1(%2)").arg(audio->objectName()).arg(audio->getFormat()));
	item->setData(ITEM_HASHKEY, audio->hashKey());
	item->setData(ITEM_VALUE, audio->objectName());
	ui->listWidget->addItem(item);
}
