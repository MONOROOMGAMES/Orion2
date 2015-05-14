#include "mergeimagewindow.h"
#include "ui_mergeimagewindow.h"
#include "tooldefine.h"

MergeImageWindow::MergeImageWindow(QList<ResourceHash>& keys, QWidget *parent) :
QDialog(parent),
    ui(new Ui::MergeImageWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	// ���ذ��б�
	for (TResourceObjectIterator<PackageResourceObject> it; it; ++it){
		PackageResourceObject* pkg = (PackageResourceObject*)(*it);
		ui->pkgcomboBox->addItem(pkg->objectName(), pkg->hashKey());
	}

	// ��ʼ���б�
	foreach(ResourceHash key, keys){
		DEFINE_RESOURCE_HASHKEY_VALID(ImageResourceObject, img, key);

		if (categories_.size() == 0 ){
			categories_.append(img->categories_);
		}

		QListWidgetItem* item = new QListWidgetItem();
		item->setIcon(img->thumbnail_);
		item->setText(img->objectName());
		item->setData(ITEM_HASHKEY, img->hashKey());
		item->setData(ITEM_SIZE, img->image_.size());

		ui->listWidget->addItem(item);
	}

	categories_.removeDuplicates();

}

MergeImageWindow::~MergeImageWindow()
{
    delete ui;
}


QImage MergeImageWindow::mergeImage(const QSize& maxsize, bool fixsize, bool twom){

	frames_.clear();
	int num = ui->listWidget->count();

	// ȡ����ͼ���С
	QSize baseSize = ui->listWidget->item(0)->data(ITEM_SIZE).toSize();
	for (int i = 1; i <num; i++){
		QSize s = ui->listWidget->item(i)->data(ITEM_SIZE).toSize();
		if (s.width() > baseSize.width()){
			baseSize.setWidth(s.width());
		}

		if (s.height() > baseSize.height()){
			baseSize.setHeight(s.height());
		}
	}

	// ����ͼ���С
	QSize newSize(0, 0);
	if (fixsize){
		newSize = maxsize;
	}
	else{
		// Ŀǰ��������ͼƬ��Сһ�£�����С�����
		// ������չΪ֧�������С
		if (isSqrt(num)){
			// �������ȫƽ����
			int root = qRound(qSqrt(num));
			newSize = root * baseSize;
		}
		else{
			int row = 1;
			while (row <= num){
				int col = qCeil(num * 1.0 / row);
				if ( (col * baseSize.width() <= maxsize.width()) && 
					(row * baseSize.height() <= maxsize.height()) ){
					newSize.setWidth(col * baseSize.width());
					newSize.setHeight(row * baseSize.height());
					break;
				}		
				row++;
			}
		}
	}

	if (newSize.width() == 0 || newSize.height() == 0){
		return QImage();
	}

	if (twom){  // ������2���ݴ�
		newSize.setWidth(twoPower(newSize.width()));
		// ���µ����߶�
		int newcol = newSize.width() / baseSize.width();
		int newrow = num / newcol + num % newcol ? 1 : 0;
		newSize.setHeight(twoPower(newrow * baseSize.height()));
	}

	QImage image(newSize, QImage::Format_ARGB32);
	image.fill(Qt::transparent);
	QPainter painter(&image);

	int x = 0, y = 0;
	QRect r;
	for (int i = 0; i < ui->listWidget->count(); i++){
		QSize s = ui->listWidget->item(i)->data(ITEM_SIZE).toSize();
		ResourceHash key = ui->listWidget->item(i)->data(ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY_VALID(ImageResourceObject, img, key);

		r.setLeft(x + (baseSize.width() - s.width()) / 2);
		r.setTop(y + (baseSize.height() - s.height()) / 2);
		r.setSize(img->image_.size());
		frames_.append(r);

		// ����ƫ��
		painter.drawImage(r.topLeft(), img->image_);
		x += baseSize.width();

		if (x > newSize.width() - baseSize.width()){
			x = 0;
			y += baseSize.height();
		}
	}

	return image;
}

bool MergeImageWindow::canSave(void){

	if (ui->pkgcomboBox->currentIndex() < 0){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("��ָ��һ����Դ��!"));
		return false;
	}

	// ����Ƿ�Ϸ�
	LINEEDIT_VALID(ui->namelineEdit, windowTitle(), UDQ_TR("�ز�ID"), REG_GALLERYID);

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

void MergeImageWindow::on_savepushButton_clicked(){

	if (!canSave())
		return;

	// ���浽���ݿ���
	ImageResourceObject* newImage = TFindResource<ImageResourceObject>(
		ui->pkgcomboBox->currentText(),
		ui->namelineEdit->text(), true);
	Q_ASSERT(newImage != 0);

	newImage->tags_ = ui->taglineEdit->text().split(UDQ_T(";"));
	newImage->setContent( ui->previewwidget->getPictureData() );
	newImage->format_ = ui->typecomboBox->currentText();
	newImage->categories_ = categories_;
	newImage->isGroup_ = true;
	newImage->frames_ = frames_;

	// ��������ͼ
	newImage->image_ = ui->previewwidget->getPicture();
	newImage->makeThumbnail();

	newImage->clearFlags(URF_TagGarbage);
	newImage->setFlags(URF_TagSave);

	// ���浽����
	SAVE_PACKAGE_RESOURCE(ImageResourceObject, newImage);

	accept();
}

void MergeImageWindow::on_cancelpushButton_clicked(){
	reject();
}

void MergeImageWindow::on_mergepushButton_clicked(){
	QSize size;
	size.setWidth(ui->widthspinBox->value());
	size.setHeight(ui->heightspinBox->value());

	// ���ͼƬ��С
	for (int i = 0; i < ui->listWidget->count(); i++){
		QSize s = ui->listWidget->item(i)->data(ITEM_SIZE).toSize();
		if (s.width() > size.width() || s.height() > size.height()){
			QPalette palette;
			palette.setColor(QPalette::Background, Qt::red);
			ui->infolabel->setPalette(palette);
			ui->infolabel->setText(UDQ_TR("ͼƬ[%1]�ߴ糬�����ߴ�").arg(ui->listWidget->item(i)->text()));
			return;
		}
	}

	QImage newImage = mergeImage(size, ui->fixedcheckBox->isChecked(), ui->twocheckBox->isChecked());
	ui->previewwidget->showPicture(newImage);
}

void MergeImageWindow::on_squarecheckBox_toggled(bool checked){
	ui->heightspinBox->setReadOnly(checked);
	ui->heightspinBox->setValue(ui->widthspinBox->value());
}

void MergeImageWindow::on_widthspinBox_valueChanged(int i){
	if (ui->squarecheckBox->isChecked()){
		ui->heightspinBox->setValue(i);
	}
}

bool spriteNameLessThan(const QString &s1, const QString &s2)
{
	QString sn1 = s1, sn2 = s2;
	int si1 = -1, si2 = -1;

	int index = s1.size() - 1;
	while (index >= 0 && s1[index].isDigit()){
		index--;
	}
	index++;
	// �����ַ�Ϊ�ַ���β������
	if (index > 0){
		sn1 = s1.left(index);
	}
	if (index < s1.size()){
		si1 = s1.right(s1.size() - index).toInt();
	}

	index = s2.size() - 1;
	while (index >= 0 && s2[index].isDigit()){
		index--;
	}
	index++;
	// �����ַ�Ϊ�ַ���β������
	if (index > 0){
		sn2 = s2.left(index);
	}
	if (index < s2.size()){
		si2 = s2.right(s2.size() - index).toInt();
	}

	return (sn1 < sn2) || (sn1 == sn2  && si1 < si2);
}

void MergeImageWindow::on_sortpushButton_clicked(){
	QMap<QString, QListWidgetItem*> unsortItems;
	QList<QString> names;

	while (ui->listWidget->count() > 0 ){
		QListWidgetItem* item = ui->listWidget->takeItem(0);
		unsortItems.insert(item->text(), item);
		names.append(item->text());
	}

	qStableSort(names.begin(), names.end(), spriteNameLessThan);

	// ���¼���
	foreach(QString name, names){
		Q_ASSERT(unsortItems.contains(name));
		ui->listWidget->addItem(unsortItems[name]);
	}

}