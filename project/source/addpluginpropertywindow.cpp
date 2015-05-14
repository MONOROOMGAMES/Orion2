#include "addpluginpropertywindow.h"
#include "ui_addpluginpropertywindow.h"
#include <QMessageBox>
#include "tooldefine.h"

AddPluginPropertyWindow::AddPluginPropertyWindow(int pluginType, const QMultiHash<int, QString>& existIds, QWidget *parent) :
QDialog(parent), ui(new Ui::AddPluginPropertyWindow), editmode_(false)
{
    ui->setupUi(this);
	setFixedSize(size());

	init(pluginType);
	existIds_ = existIds;

}

AddPluginPropertyWindow::AddPluginPropertyWindow(int pluginType, const QMultiHash<int, QString>& existIds,  const QByteArray& editprop, QWidget *parent) :
QDialog(parent), ui(new Ui::AddPluginPropertyWindow), editmode_(true)
{
	ui->setupUi(this);
	setFixedSize(size());

	init(pluginType);
	existIds_ = existIds;

	ui->addpushButton->setText(UDQ_TR("����"));

	int index;
	QString id;
	{
		QDataStream stream(editprop);
		stream >> index >> id;
		newIds_.insert(index, id);
	}

	existIds_.remove(index, id);  // �������ʱ�������Լ�

	loadProperty(editprop);
	ui->propcomboBox->setCurrentIndex(index);
	
}

AddPluginPropertyWindow::~AddPluginPropertyWindow()
{
    delete ui;
}

void AddPluginPropertyWindow::init(int type){

	ui->layoutWidget1->setGeometry(ui->property->geometry().adjusted(4, 4, -48, -96));
	ui->layoutWidget2->setGeometry(ui->function->geometry().adjusted(4, 4, -48, -96));

	QStringList dataType;
	if (type == PIT_EFFECT){
		dataType << UDQ_T("float") << UDQ_T("percent");
	}
	else{
		dataType << UDQ_T("int") << UDQ_T("float") << UDQ_T("string") << UDQ_T("enum");
		if (type == PIT_ENTITY){
			dataType << UDQ_T("color") << UDQ_T("font");
		}
	}
	ui->datatypecomboBox->addItems(dataType);


	QMap<int, QString> cndFlags;
	cndFlags.insert(FT_NONE, UDQ_TR("��"));
	cndFlags.insert(FT_DEPRECATED, UDQ_TR("��ʱ"));
	cndFlags.insert(FT_TRIGGER, UDQ_TR("������"));
	cndFlags.insert(FT_FAKE_TRIGGER, UDQ_TR("�ٴ�����"));
	cndFlags.insert(FT_STATIC, UDQ_TR("��̬"));
	cndFlags.insert(FT_INVERTIBLE, UDQ_TR("������"));
	cndFlags.insert(FT_INCOMPATIBLE, UDQ_TR("�봥����������"));
	cndFlags.insert(FT_LOOPING, UDQ_TR("ѭ��"));
	cndFlags.insert(FT_FAST_TRIGGER, UDQ_TR("���ٴ���"));
	prepareFlagItems(PT_CONDITION, cndFlags);

	QMap<int, QString> actFlags;
	actFlags.insert(FT_NONE, UDQ_TR("��"));
	actFlags.insert(FT_DEPRECATED, UDQ_TR("��ʱ"));
	prepareFlagItems(PT_ACTION, actFlags);

	QMap<int, QString> expFlags;
	expFlags.insert(FT_NONE, UDQ_TR("��"));
	expFlags.insert(FT_DEPRECATED, UDQ_TR("��ʱ"));
	expFlags.insert(FT_RETURN_INT, UDQ_TR("��������"));
	expFlags.insert(FT_RETURN_FLOAT, UDQ_TR("���ظ�����"));
	expFlags.insert(FT_RETURN_STRING, UDQ_TR("�����ַ���"));
	expFlags.insert(FT_RETURN_ANY, UDQ_TR("������������"));
	expFlags.insert(FT_VARIADIC, UDQ_TR("�ɱ����"));
	prepareFlagItems(PT_EXPRESS, expFlags);

}

void AddPluginPropertyWindow::prepareFlagItems(int type, const QMap<int, QString>& items){
	//  ����Item
	QMapIterator<int, QString> iter(items);
	while (iter.hasNext()){
		iter.next();

		QListWidgetItem* item = new QListWidgetItem(iter.value());
		item->setData(ITEM_VALUE, iter.key());
		item->setCheckState(Qt::Unchecked);
		flagItems_.insert(iter.key(), item);
		flagItemsGroup_.insert(type, item);
		ui->flaglistWidget->addItem(item);
		item->setHidden(true);
	}

}

void AddPluginPropertyWindow::loadProperty(const QByteArray& editprop){

		QDataStream stream(editprop);
		int index;
		QString id;
		QString datatype;
		QString def;
		QString desc;
		stream >> index >> id >> datatype >> def >> desc;

		if (index == 0){

			ui->vidlineEdit->setText(id);
			ui->datatypecomboBox->setCurrentText(datatype);
			ui->deflineEdit->setText(def);
			ui->plainTextEdit->setPlainText(desc);

			if (datatype == UDQ_T("enum")){
				// ö��ֵ
				QStringList values;
				stream >> values;

				foreach(QString v, values){
					addEnum(v);
				}
			}
		}
		else {
			Q_ASSERT(index >= 1);


		}
}

void AddPluginPropertyWindow::on_addpushButton_clicked(){
	// ���Ϸ���
	if (!checkValid()){
		return;
	}
	
	// ��ӵ�����
	int index = ui->propcomboBox->currentIndex();

	QByteArray dat;
	{
		QDataStream stream(&dat, QIODevice::WriteOnly);
		stream << index;
		if (index == 0){
			stream << ui->vidlineEdit->text();
			stream << ui->datatypecomboBox->currentText();
			stream << ui->deflineEdit->text();
			stream << ui->vplainTextEdit->toPlainText();
			if (ui->datatypecomboBox->currentText() == UDQ_T("enum")){
				QStringList values;
				for (int i = 0; i < ui->enumlistWidget->count(); i++){
					values.append(ui->enumlistWidget->item(i)->text());
				}
				stream << values;
			}
		}
		else{
			stream << ui->idlineEdit->text();
			QList<int> flags;
			foreach(QListWidgetItem* item, flagItemsGroup_.values(index)){
				if (item->checkState() == Qt::Checked){
					flags.append(item->data(ITEM_VALUE).toInt());
				}	
			}
			stream << flags;
			stream << ui->namelineEdit->text();
			stream << ui->funclineEdit->text();
			stream << ui->plainTextEdit->toPlainText();

			if (index < PT_EXPRESS){
				stream << ui->textlineEdit->text();
			}

			// ����


		}
	}

	if (index == PT_VARIABLE){
		newVar_.insert(ui->vidlineEdit->text(), dat);
	}
	else if (index == PT_CONDITION){
		newCnd_.insert(ui->idlineEdit->text(), dat);
	}
	else if (index == PT_ACTION){
		newAct_.insert(ui->idlineEdit->text(), dat);
	}
	else if (index == PT_EXPRESS){
		newExp_.insert(ui->idlineEdit->text(), dat);
	}

	if (editmode_)  // �༭ģʽ�����һ��
		accept();
	else{
		QMessageBox::information(this, windowTitle(), UDQ_TR("��ӳɹ�"));
	}

}

// ����ظ�ID
bool AddPluginPropertyWindow::duplicatedId(int type, const QString& id){

	if (!editmode_){
		// ����Ƿ��ظ�
		QStringList newIds = newIds_.values(type);
		if (newIds.contains(id)){
			if (QMessageBox::question(this, windowTitle(), UDQ_TR("ͬ�������Ѿ�����ӣ��Ƿ񸲸�?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No){
				return false;
			}
			return true;
		}
	}

	QStringList existIds = existIds_.values(type);
	if (existIds.contains(id)){
		if (editmode_){
			QMessageBox::warning(this, windowTitle(), UDQ_TR("ͬ�������Ѿ�����, ���޸ĺ�����"));
			return false;
		}
		else{
			if (QMessageBox::question(this, windowTitle(), UDQ_TR("ͬ�������Ѿ����ڣ��Ƿ񸲸�?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No){
				return false;
			}
		}

	}

	return true;

}


bool AddPluginPropertyWindow::checkValid(){
	int index = ui->propcomboBox->currentIndex();

	if (index == 0){
		LINEEDIT_CHECK(ui->vidlineEdit, UDQ_TR("��ӱ���"), UDQ_TR("ID"));
		COMBOX_CHECK(ui->datatypecomboBox, UDQ_TR("��ӱ���"), UDQ_TR("��������"));

	
		if (!duplicatedId(index, ui->vidlineEdit->text()))
			return false;

	}
	else if(index >= 1){
		LINEEDIT_CHECK(ui->idlineEdit, UDQ_TR("�������"), UDQ_TR("ID"));
		LINEEDIT_CHECK(ui->namelineEdit, UDQ_TR("�������"), UDQ_TR("����"));
		LINEEDIT_CHECK(ui->funclineEdit, UDQ_TR("�������"), UDQ_TR("�ű�����"));

		// ����Ƿ��ظ�
		if (!duplicatedId(index, ui->idlineEdit->text()))
			return false;
	}

	return true;

}

void AddPluginPropertyWindow::on_cancelpushButton_clicked(){
	reject();
}

void AddPluginPropertyWindow::on_propcomboBox_currentIndexChanged(int index){
	ui->stackedWidget->setCurrentIndex( index > 0  ? 1 : 0);

	// ��������Item
	for (int i = 0; i < ui->flaglistWidget->count(); ++i){
		ui->flaglistWidget->item(i)->setHidden(true);
	}

	if (index < 0)
		return;

	foreach(QListWidgetItem* item, flagItemsGroup_.values(index)){
		item->setHidden(false);
	}

}

void AddPluginPropertyWindow::on_datatypecomboBox_currentIndexChanged(const QString & text){
	if (text == UDQ_T("enum")){
		ui->addenumpushButton->setEnabled(true);
		ui->delenumpushButton->setEnabled(true);
		ui->enumlistWidget->setEnabled(true);
	}
	else{
		ui->addenumpushButton->setEnabled(false);
		ui->delenumpushButton->setEnabled(false);
		ui->enumlistWidget->setEnabled(false);
	}
}

void AddPluginPropertyWindow::on_addenumpushButton_clicked(){
		// ����Ψһ��ö��ֵ
	QString enumName = UDQ_T("Value");
	int index = 1;
	QString str = enumName;
	while (ui->enumlistWidget->findItems(str, Qt::MatchFixedString).size() > 0){
		str = QString(UDQ_T("%1%2")).arg(enumName).arg(index++);
	}
	addEnum(str);
}


QListWidgetItem* AddPluginPropertyWindow::addEnum(const QString& value){
	// �����ö��ֵ
	QListWidgetItem* item = new QListWidgetItem();
	item->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled);
	item->setText(value);
	ui->enumlistWidget->addItem(item);
	
	return item;
}

void AddPluginPropertyWindow::on_delenumpushButton_clicked(){
	if (ui->enumlistWidget->currentRow() < 0)
		return;
	QListWidgetItem* item = ui->enumlistWidget->takeItem(ui->enumlistWidget->currentRow());
	delete item;
}

void AddPluginPropertyWindow::on_enumlistWidget_itemDoubleClicked(QListWidgetItem * item){
	if (item == NULL)
		return;
	ui->deflineEdit->setText(item->text());
}

void AddPluginPropertyWindow::on_addparampushButton_clicked(){

}

void AddPluginPropertyWindow::on_delparampushButton_clicked(){

}