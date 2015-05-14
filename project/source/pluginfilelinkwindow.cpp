#include "pluginfilelinkwindow.h"
#include "ui_pluginfilelinkwindow.h"
#include <QComboBox>
#include <QMessageBox>
#include <QFileDialog>
#include "tooldefine.h"
#include "resourceobject.h"
using namespace ORION;

PluginFileLinkWindow::PluginFileLinkWindow(QMultiHash<int, QByteArray>& files, QWidget *parent) :
QDialog(parent), files_(files),
    ui(new Ui::PluginFileLinkWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	// ��ʼ���ļ��б�
	QString fileName;
	QByteArray data;
	QString datetime;
	bool isText;

	types_.append(UDQ_TR("JS�ļ�"));
	types_.append(UDQ_TR("CSS�ļ�"));
	types_.append(UDQ_TR("HTML�ļ�"));
	types_.append(UDQ_TR("�ı��ļ�"));
	types_.append(UDQ_TR("�����ļ�"));
	types_.append(UDQ_TR("ͼ���ļ�"));
	types_.append(UDQ_TR("��Ƶ�ļ�"));
	types_.append(UDQ_TR("��Ƶ�ļ�"));
	types_.append(UDQ_TR("�����ļ�"));

	fileexts_.insert(FIT_JAVASCRIPT, QStringList()<<UDQ_T("js"));
	fileexts_.insert(FIT_CSS, QStringList() << UDQ_T("css"));
	fileexts_.insert(FIT_HTML, QStringList() << UDQ_T("htm") << UDQ_T("html") << UDQ_T("xhtml"));
	fileexts_.insert(FIT_TEXT, QStringList() << UDQ_T("txt") << UDQ_T("json") << UDQ_T("plist") << UDQ_T("h") << UDQ_T("cpp"));
	fileexts_.insert(FIT_DATA, QStringList() << UDQ_T("dat") << UDQ_T("db"));
	fileexts_.insert(FIT_IMAGE, QStringList() << UDQ_T("bmp") << UDQ_T("png") << UDQ_T("jpg") << UDQ_T("jpeg") << UDQ_T("gif") );
	fileexts_.insert(FIT_AUDIO, QStringList() << UDQ_T("wav") << UDQ_T("mp3") << UDQ_T("ogg") << UDQ_T("m4a"));
	fileexts_.insert(FIT_VIDEO, QStringList() << UDQ_T("mp4") << UDQ_T("ogv") << UDQ_T("webm"));
	
	QHashIterator<int, QByteArray> iter(files_);
	while (iter.hasNext()){
		iter.next();

		{
			QDataStream stream(iter.value());
			stream >> fileName >> datetime >> isText >> data;
		}
		addFileItem(fileName, iter.key(), datetime, data);
	}

	ui->tableWidget->resizeColumnsToContents();

}

PluginFileLinkWindow::~PluginFileLinkWindow()
{
    delete ui;
}


void PluginFileLinkWindow::addFileItem(const QString& name, int type, const QString& date, QByteArray& data){
	int row = ui->tableWidget->rowCount();
	ui->tableWidget->insertRow(row);

	QTableWidgetItem* nameitem = new QTableWidgetItem();
	nameitem->setText(name);
	nameitem->setData(ITEM_CONTEXT, data);
	ui->tableWidget->setItem(row, 0, nameitem);

	QComboBox* combo = new QComboBox();
	combo->addItems(types_);
	combo->setCurrentIndex( type - 2);
	ui->tableWidget->setCellWidget(row, 1, combo);

	QTableWidgetItem* sizeitem = new QTableWidgetItem();
	sizeitem->setText(QString::number(data.size()));
	ui->tableWidget->setItem(row, 2, sizeitem);

	QTableWidgetItem* dateitem = new QTableWidgetItem();
	dateitem->setText(date);
	ui->tableWidget->setItem(row, 3, dateitem);

}


void PluginFileLinkWindow::on_addpushButton_clicked(){
	// ����ļ�
	QString fileFilter;
	QString fileType;

	QStringList fileNames = QFileDialog::getOpenFileNames(this, UDQ_TR("�����ļ�"), QString(), fileFilter);
	if (fileNames.size() == 0 )
		return;

	bool skiped = false;
	int row = -1;
	int type = 0;
	QString datetime;
	foreach(QString fileName, fileNames){
		QFileInfo info(fileName);
		datetime = info.lastModified().toString(UDQ_T("yyyy-MM-dd hh:mm:ss"));

		skiped = false;
		row = -1;

		// ����Ƿ�����
		for (int i = 0; i < ui->tableWidget->rowCount(); i++){
			if (ui->tableWidget->item(i, 0)->text() == info.fileName()){
				if (QMessageBox::question(this, windowTitle(), UDQ_TR("�ļ��Ѿ����ڣ��Ƿ񸲸�?")) == QMessageBox::Yes){
					row = i;
				}
				else{
					skiped = true;
				}
				break;
			}
		}

		if (skiped){
			continue;
		}
	
		// �Զ��ж��ļ�����
		type = acquireFileType(info.suffix());

		QFile f(fileName);
		if (f.open(QIODevice::ReadOnly)){
			QByteArray data = f.readAll();
			f.close();

			if (row >= 0){
				// �滻
				ui->tableWidget->item(row, 0)->setData(ITEM_CONTEXT, data);
				ui->tableWidget->item(row, 2)->setText(QString::number(data.size()));
				ui->tableWidget->item(row, 3)->setText(datetime);
			}
			else{
				// ��ӵ��б�
				addFileItem(info.fileName(), type, datetime, data);
			}
		}

	}

	ui->tableWidget->resizeColumnsToContents();


}


int PluginFileLinkWindow::acquireFileType(const QString& ext){
	int type= FIT_OTHER;

	QHashIterator<int, QStringList> iter(fileexts_);
	while (iter.hasNext()){
		iter.next();

		if (iter.value().contains(ext)){
			type = iter.key();
			break;
		}
	}

	return type;
}

void PluginFileLinkWindow::on_cancelpushButton_clicked(){

	// �����ļ����б�
	files_.clear();

	QByteArray tmpdata;
	for (int i = 0; i < ui->tableWidget->rowCount(); i++){

		int type = ((QComboBox*)ui->tableWidget->cellWidget(i, 1))->currentIndex() + 2;
		tmpdata.clear();
		{
			QDataStream stream(&tmpdata, QIODevice::WriteOnly);
			stream << ui->tableWidget->item(i, 0)->text();
			stream << ui->tableWidget->item(i, 3)->text();

			// �Ƿ�Ϊ�ı��ļ�
			bool isText = type< FIT_DATA;
			stream << isText;

			stream << ui->tableWidget->item(i, 0)->data(ITEM_CONTEXT).toByteArray();
		}
		files_.insert(type, tmpdata);
	}

	reject();
}

void PluginFileLinkWindow::on_deletepushButton_clicked(){
	QList<QTableWidgetItem*> selectedItems = ui->tableWidget->selectedItems();
	if (selectedItems.size() == 0)
		return;

	if (QMessageBox::question(this, windowTitle(), UDQ_TR("ȷ��ɾ��ѡ�е��ļ�?")) == QMessageBox::No){
		return;
	}

	// ɾ��ѡ�е��ļ�
	QSet<int> rows;
	foreach(QTableWidgetItem* item, selectedItems){
		rows.insert(item->row());
	}

	QList<int> list = rows.toList();
	qSort(list.begin(), list.end());

	for (int i = list.size() - 1; i >= 0; i--){
		ui->tableWidget->removeRow(list.at(i));
	}

}

void PluginFileLinkWindow::on_emptypushButton_clicked(){
	if (QMessageBox::question(this, windowTitle(), UDQ_TR("ȷ������ļ��б�?")) == QMessageBox::No){
		return;
	}

	ui->tableWidget->clear();
}