#include "pluginmanagewindow.h"
#include "ui_pluginmanagewindow.h"
#include "commondefine.h"
#include "tooldefine.h"
#include "configmanager.h"
#include <QFileDialog>
#include "addpluginpropertywindow.h"
#include "pluginfilelinkwindow.h"


PluginManageWindow::PluginManageWindow(QWidget *parent) :
QDialog(parent),
    ui(new Ui::PluginManageWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	QMap<int, QString> entityAttrs;
	entityAttrs.insert(EF_WORLD ,UDQ_TR("�Ƿ�ɼ�"));
	entityAttrs.insert(EF_ROTATABLE, UDQ_TR("�Ƿ�֧����ת"));
	entityAttrs.insert(EF_NOSIZE, UDQ_TR("�Ƿ�ߴ�̶�"));
	entityAttrs.insert(EF_TILING, UDQ_TR("֧��Tile"));
	entityAttrs.insert(EF_SINGLEGLOBAL, UDQ_TR("��ʵ��"));
	entityAttrs.insert(EF_ANIMATIONS, UDQ_TR("֧�ֶ���"));
	entityAttrs.insert(EF_EFFECTS, UDQ_TR("֧����Ч"));
	entityAttrs.insert(EF_BEHAVIORS, UDQ_TR("֧����Ϊ"));
	entityAttrs.insert(EF_PREDRAW, UDQ_TR("Ԥ����"));
	entityAttrs.insert(EF_TEXTURE, UDQ_TR("��������"));
	entityAttrs.insert(EF_POSITION, UDQ_TR("λ�����Խӿ�"));
	entityAttrs.insert(EF_SIZE, UDQ_TR("�ߴ����Խӿ�"));
	entityAttrs.insert(EF_ANGLE, UDQ_TR("��ת���Խӿ�"));
	entityAttrs.insert(EF_APPEARANCE, UDQ_TR("������Խӿ�"));
	entityAttrs.insert(EF_ZORDER, UDQ_TR("������Խӿ�"));

	prepareAttrItems(0, entityAttrs);

	QMap<int, QString> behaviorAttrs;
	behaviorAttrs.insert(EF_ONCE, UDQ_TR("ֻ�����һ��"));
	prepareAttrItems(1, behaviorAttrs);

	QMap<int, QString> effectAttrs;
	effectAttrs.insert(EF_BLENDBG, UDQ_TR("��ϱ���"));
	effectAttrs.insert(EF_CROSSSAMPLE, UDQ_TR("�������"));
	effectAttrs.insert(EF_ANIMATIONDRAW, UDQ_TR("����"));
	prepareAttrItems(2, effectAttrs);

	ui->typecomboBox->setCurrentIndex(-1);
	ui->systemcheckBox->setVisible(gConfigManager->devMode());

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	variableRootItem_ = new QTreeWidgetItem(ITEM_GROUP);
	variableRootItem_->setText(0, UDQ_TR("����"));
	variableRootItem_->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui->proptreeWidget->addTopLevelItem(variableRootItem_);

	conditionRootItem_ = new QTreeWidgetItem(ITEM_GROUP);
	conditionRootItem_->setText(0, UDQ_TR("����"));
	conditionRootItem_->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui->proptreeWidget->addTopLevelItem(conditionRootItem_);

	actionRootItem_ = new QTreeWidgetItem(ITEM_GROUP);
	actionRootItem_->setText(0, UDQ_TR("����"));
	actionRootItem_->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui->proptreeWidget->addTopLevelItem(actionRootItem_);

	expressRootItem_ = new QTreeWidgetItem(ITEM_GROUP);
	expressRootItem_->setText(0, UDQ_TR("���ʽ"));
	expressRootItem_->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui->proptreeWidget->addTopLevelItem(expressRootItem_);

	initFromDb();

}

PluginManageWindow::~PluginManageWindow()
{
    delete ui;

}

void PluginManageWindow::prepareAttrItems(int type, const QMap<int, QString>& items){
	//  ����Item
	QMapIterator<int, QString> iter(items);
	while (iter.hasNext()){
		iter.next();

		QListWidgetItem* item = new QListWidgetItem(iter.value());
		item->setData(ITEM_VALUE, iter.key());
		item->setCheckState(Qt::Unchecked);
		attrItems_.insert(iter.key(), item);
		attrStringItems_.insert(Value2String(iter.key()), item);
		attrItemsGroup_.insert(type, item);
		ui->attrlistWidget->addItem(item);
		item->setHidden(true);
	}
}

void PluginManageWindow::on_savepushButton_clicked(){
	// ������
	if (!checkValid())
		return;

	// �������в��
	PluginResourceObject* newPlugin = TFindResource<PluginResourceObject>(
		ui->pkgcomboBox->currentText(),
		ui->idlineEdit->text(), true);
	Q_ASSERT(newPlugin != NULL);

	// �������Ѿ����ڣ��������ڲ����
	if (newPlugin->devmode_ && !gConfigManager->devMode()){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("û��Ȩ���޸�ϵͳ�ڲ����!"));
		return;
	}

	newPlugin->type_ = ui->typecomboBox->currentIndex();
	newPlugin->title_ = ui->namelineEdit->text();
	newPlugin->modelVer_ = ui->versionlineEdit->text();

	QString oldCategory;  // ����Ŀ
	if (newPlugin->categories_.size() > 0){
		oldCategory = newPlugin->categories_[0];
	}

	newPlugin->categories_.clear();
	newPlugin->categories_.append(ui->categorycomboBox->currentText());

	newPlugin->setDescription(ui->plainTextEdit->toPlainText()); 

	if (gConfigManager->devMode()){
		newPlugin->devmode_ = ui->systemcheckBox->isChecked();
	}
	// ͼ��
	QImage img = ui->iconwidget->getPicture();
	if (img.width() > 64)
		img = img.scaledToWidth(64, Qt::SmoothTransformation);
	if (img.height() > 64)
		img = img.scaledToHeight(64, Qt::SmoothTransformation);
	newPlugin->icon_ = img;

	newPlugin->filedata_ = assosiateFiles_;
	Q_ASSERT(!assosiateFiles_.contains(FIT_FX) && !assosiateFiles_.contains(FIT_RUNTIME));

	if (newPlugin->type_ == 2){
		newPlugin->extendBound_.setWidth(ui->exthspinBox->value());
		newPlugin->extendBound_.setHeight(ui->extvspinBox->value());

		if (tmpFiledat_.size() > 0){
			newPlugin->filedata_.insert(FIT_FX, tmpFiledat_);
		}
	}
	else{
		if (tmpFiledat_.size() > 0){
			newPlugin->filedata_.insert(FIT_RUNTIME, tmpFiledat_);
		}
	}

	//
	newPlugin->extendBound_.setWidth(ui->exthspinBox->value());
	newPlugin->extendBound_.setHeight(ui->extvspinBox->value());

	// ����
	newPlugin->attributes_.clear();
	foreach(QListWidgetItem* item, attrItemsGroup_.values(newPlugin->type_)){
		newPlugin->attributes_[item->data(ITEM_VALUE).toInt()] = ( item->checkState() == Qt::Checked )? 1 : 0;
	}

	// ��Ա����
	PluginVariable* var = NULL;
	PluginInterface* inter = NULL;
	QTreeWidgetItem* item = NULL;


	newPlugin->varIndex_.clear();
	newPlugin->variables_.clear();
	QByteArray tmpdat;
	for (int i = 0; i < variableRootItem_->childCount(); i++){
		item = variableRootItem_->child(i);
		var = newPlugin->variables_.value(item->text(0), true);
		Q_ASSERT(var != NULL);
		tmpdat = item->data(0, ITEM_VALUE).toByteArray();
		var->fromByteArray(tmpdat);
		newPlugin->varIndex_.append(hashString(var->name));  // ��¼����˳��
	}

	// �������нӿڣ�
	QList<PluginInterface*> oldInterfaces = newPlugin->conditions_.values();
	for (int i = 0; i < conditionRootItem_->childCount(); i++){
		item = conditionRootItem_->child(i);
		inter = newPlugin->conditions_.value(item->data(0, ITEM_CONTEXT).toString(), true);
		Q_ASSERT(inter != NULL);
		tmpdat = item->data(0, ITEM_VALUE).toByteArray();
		inter->fromByteArray(tmpdat);
		oldInterfaces.removeOne(inter);
	}

	// �޳�����ͬ�Ľӿ�
	foreach(PluginInterface* inter, oldInterfaces){
		newPlugin->conditions_.remove(hashString(inter->name));
	}

	oldInterfaces = newPlugin->actions_.values();
	for (int i = 0; i < actionRootItem_->childCount(); i++){
		item = actionRootItem_->child(i);
		inter = newPlugin->actions_.value(item->data(0, ITEM_CONTEXT).toString(), true);
		Q_ASSERT(inter != NULL);
		tmpdat = item->data(0, ITEM_VALUE).toByteArray();
		inter->fromByteArray(tmpdat);
		oldInterfaces.removeOne(inter);
	}

	// �޳�����ͬ�Ľӿ�
	foreach(PluginInterface* inter, oldInterfaces){
		newPlugin->actions_.remove(hashString(inter->name));
	}


	oldInterfaces = newPlugin->actions_.values();
	for (int i = 0; i < expressRootItem_->childCount(); i++){
		item = expressRootItem_->child(i);
		inter = newPlugin->expresses_.value(item->data(0, ITEM_CONTEXT).toString(), true);
		Q_ASSERT(inter != NULL);
		tmpdat = item->data(0, ITEM_VALUE).toByteArray();
		inter->fromByteArray(tmpdat);
		oldInterfaces.removeOne(inter);
	}

	// �޳�����ͬ�Ľӿ�
	foreach(PluginInterface* inter, oldInterfaces){
		newPlugin->actions_.remove(hashString(inter->name));
	}

	// ����
	newPlugin->postInit();

	newPlugin->clearFlags(URF_TagGarbage);
	newPlugin->setFlags(URF_TagSave);

	// ���浽����
	SAVE_PACKAGE_RESOURCE(PluginResourceObject, newPlugin);

	QTreeWidgetItem* cateItem = NULL;
	QTreeWidgetItem* pluginItem = NULL;
	if (!oldCategory.isEmpty()){  // ���Բ�����ǰ�����ڵ�
		// ��ӽڵ�
		cateItem = findCateory(newPlugin->type_, oldCategory, false);
		if (cateItem){
			for (int i = 0; i < cateItem->childCount(); i++){
				if (cateItem->child(i)->data(0, ITEM_HASHKEY) == newPlugin->hashKey()){

					if (oldCategory != ui->categorycomboBox->currentText()){
						pluginItem = cateItem->takeChild(i);
					}
					else{
						pluginItem = cateItem->child(i);
					}

					break;
				}
			}
		}
	}

	// ��ӽڵ�
	QString newCategory = newPlugin->categories_[0];
	cateItem = findCateory(newPlugin->type_, newCategory);
	cateItem->setExpanded(true);

	if (pluginItem == NULL){
		pluginItem = new QTreeWidgetItem(newPlugin->type_ + ITEM_ENTITY);
		pluginItem->setText(0, newPlugin->title_);
		pluginItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
		pluginItem->setData(0, ITEM_HASHKEY, newPlugin->hashKey());
		pluginItem->setToolTip(0, newPlugin->description());
		cateItem->addChild(pluginItem);

	}
	else{
		pluginItem->setText(0, newPlugin->title_);
		pluginItem->setToolTip(0, newPlugin->description());
		cateItem->addChild(pluginItem);
	}

	ui->plugintreeWidget->setCurrentItem(pluginItem);
	QMessageBox::information(this, windowTitle(), UDQ_TR("�������ɹ�"));
}

void PluginManageWindow::on_cancelpushButton_clicked(){
	ApplyResourceLinkers<PluginResourceObject>();
	reject();
}


QTreeWidgetItem* PluginManageWindow::findCateory(int type, const QString& cate, bool create){
	QTreeWidgetItem* parentItem = NULL;
	// ������ĿItem
	if (type == PIT_ENTITY){
		parentItem = entityRootItem_;
	}
	else if (type == PIT_BEHAVIOR){
		parentItem = behaviorRootItem_;
	}
	else if (type == PIT_EFFECT){
		parentItem = effectRootItem_;
	}
	else if ( type == PIT_NATIVE)
	{
		parentItem = nativeRootItem_;
	}

	QTreeWidgetItem* categoryItem = NULL;
	for (int i = 0; i < parentItem->childCount(); i++){
		if (parentItem->child(i)->text(0) == cate){
			categoryItem = parentItem->child(i);
			break;
		}
	}

	if (categoryItem == NULL && create){
		categoryItem = new QTreeWidgetItem(parentItem, ITEM_CATEGORY);
		categoryItem->setText(0, cate);
		categoryItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDropEnabled);
	}

	return categoryItem;
}


void PluginManageWindow::initFromDb(void){

	nativeRootItem_ = new QTreeWidgetItem(ITEM_PLUGINGROUP);
	nativeRootItem_->setText(0, UDQ_TR("ϵͳ"));
	nativeRootItem_->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui->plugintreeWidget->addTopLevelItem(nativeRootItem_);

	entityRootItem_ = new QTreeWidgetItem(ITEM_PLUGINGROUP);
	entityRootItem_->setText(0, UDQ_TR("ʵ��"));
	entityRootItem_->setFlags(Qt::ItemIsSelectable |  Qt::ItemIsEnabled);
	ui->plugintreeWidget->addTopLevelItem(entityRootItem_);

	behaviorRootItem_ = new QTreeWidgetItem(ITEM_PLUGINGROUP);
	behaviorRootItem_->setText(0, UDQ_TR("��Ϊ"));
	behaviorRootItem_->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui->plugintreeWidget->addTopLevelItem(behaviorRootItem_);

	effectRootItem_ = new QTreeWidgetItem(ITEM_PLUGINGROUP);
	effectRootItem_->setText(0, UDQ_TR("��Ч"));
	effectRootItem_->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui->plugintreeWidget->addTopLevelItem(effectRootItem_);

	QSet<QString>  categoryList;
	QSet<QString>  pkgList;
	pkgList.insert(DEFAULT_PACKAGE);

	PluginResourceObject* plugin = NULL;

	// �����б�
	for (TResourceObjectIterator<PluginResourceObject> it; it; ++it){
		plugin = (PluginResourceObject*)(*it);

		if (plugin->flags() & URF_TagGarbage)
			continue;

		QString cate;
		Q_ASSERT(plugin->categories_.size() > 0);
		cate = plugin->categories_[0];
		Q_ASSERT(cate != NULL);
	
		pkgList.insert(plugin->getOutermost()->objectName());
		categoryList.insert(cate);

		QTreeWidgetItem* categoryItem = findCateory( plugin->type_, cate);

		// ��Ӳ��Item
		QTreeWidgetItem* pluginItem = new QTreeWidgetItem(ITEM_ENTITY + plugin->type_);
		pluginItem->setText(0, plugin->title_);
		pluginItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsDragEnabled);
		pluginItem->setData(0, ITEM_HASHKEY, plugin->hashKey());
		pluginItem->setToolTip(0, plugin->description());
		categoryItem->addChild(pluginItem);

	}

	//
	ui->categorycomboBox->addItems(categoryList.toList());
	ui->pkgcomboBox->addItems(pkgList.toList());

	ui->plugintreeWidget->expandAll();
}

void PluginManageWindow::on_plugintreeWidget_itemSelectionChanged(){
	QTreeWidgetItem* item = ui->plugintreeWidget->currentItem();

	if (item == NULL)
		return;

	if (item->type() == ITEM_PLUGINGROUP || item->type() == ITEM_CATEGORY)
		return;

	ResourceHash key = item->data(0, ITEM_HASHKEY).toUInt();
	DEFINE_RESOURCE_HASHKEY(PluginResourceObject, plugin, key);

	if (plugin == NULL){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("��Ч�Ĳ������!"));
		return;
	}
	ui->idlineEdit->setText(plugin->objectName());
	ui->namelineEdit->setText(plugin->title_);

	ui->categorycomboBox->setCurrentText(item->parent()->text(0));
	ui->versionlineEdit->setText(plugin->modelVer_);
	ui->plainTextEdit->setPlainText(plugin->description());

	ui->iconwidget->showPicture(plugin->icon_);
	if (gConfigManager->devMode()){
		ui->systemcheckBox->setChecked(plugin->devmode_);
	}

	if (plugin->type_ == 2){
		ui->exthspinBox->setValue(plugin->extendBound_.width());
		ui->extvspinBox->setValue(plugin->extendBound_.height());
	}

	ui->typecomboBox->setCurrentIndex(plugin->type_);
	// ����
	QMapIterator<int, int> iter(plugin->attributes_);
	while (iter.hasNext()){
		iter.next();

		QListWidgetItem* item = attrItems_.value(iter.key());
		if (item){
			item->setCheckState(iter.value() ? Qt::Checked : Qt::Unchecked);
		}
	}

	// �����ļ�
	QList<int> fileids = plugin->filedata_.keys();
	Q_ASSERT(fileids.size() <= 1);

	if (fileids.size() > 0){
		QString fileType;
		if (fileids.contains(FIT_RUNTIME)){
			fileType = UDQ_TR("JS�ļ�");			
			tmpFiledat_ = plugin->filedata_.value(FIT_RUNTIME);
		}
		else if (fileids.contains(FIT_FX)){
			fileType = UDQ_TR("Shader�ļ�");
			tmpFiledat_ = plugin->filedata_.value(FIT_FX);
		}
		else{
			tmpFiledat_.clear();
		}

		// ��ȡ�����ļ�
		if (tmpFiledat_.size() > 0){
			QString fileName;
			QString datetime;
			QByteArray filecontent;
			bool isText;  // �Ƿ�Ϊ�ı��ļ�

			{
				QDataStream stream(tmpFiledat_);
				stream >> fileName >> datetime >> isText >> filecontent;
			}

			// �����ı�
			ui->filelineEdit->setText(UDQ_TR("%1 %2 %3 %4�ֽ�")
				.arg(fileType).arg(fileName).arg(datetime).arg(filecontent.size()));
		}

		// ��������ļ�
		assosiateFiles_ = plugin->filedata_;
		assosiateFiles_.remove(FIT_RUNTIME);
		assosiateFiles_.remove(FIT_FX);

	}
	else{
		tmpFiledat_.clear();
		ui->filelineEdit->clear();
		assosiateFiles_.clear();
	}

	// ��Ա����
	clearChildItem(variableRootItem_);
	foreach(ResourceHash key, plugin->varIndex_){
		PluginVariable* var = plugin->variables_.value(key);
		addVariableItem(var);
	}
	clearChildItem(conditionRootItem_);
	foreach(PluginInterface* inter, plugin->conditions_.values()){
		addInterfaceItem(inter);
	}
	clearChildItem(actionRootItem_);
	foreach(PluginInterface* inter, plugin->actions_.values()){
		addInterfaceItem(inter);
	}
	clearChildItem(expressRootItem_);
	foreach(PluginInterface* inter, plugin->expresses_.values()){
		addInterfaceItem(inter);
	}

	ui->proptreeWidget->resizeColumnToContents(0);
	ui->proptreeWidget->expandAll();

}

void PluginManageWindow::on_importpushButton_clicked(){
	QString fileFilter;
	bool isText = true;
	QString fileType;

	if (ui->typecomboBox->currentIndex() == 2){
		fileFilter = UDQ_TR("Shader�ļ� (*.fx )");
		fileType = UDQ_TR("Shader�ļ�");
	}
	else{
		fileFilter = UDQ_TR("JS�ļ� (*.js )");
		fileType = UDQ_TR("JS�ļ�");
	}

	QString fileName = QFileDialog::getOpenFileName(this, UDQ_TR("�����ļ�"), QString(), fileFilter);
	if (fileName.isEmpty())
		return;

	QFileInfo info(fileName);
	QString datetime = info.lastModified().toString(UDQ_T("yyyy-MM-dd hh:mm:ss"));

	QFile f(fileName);
	if (f.open(QIODevice::ReadOnly | QIODevice::Text)){

		QByteArray data = f.readAll();
		f.close();

		// �����ı�
		ui->filelineEdit->setText(UDQ_TR("%1 %2 %3 %4�ֽ�")
			.arg(fileType).arg(info.fileName()).arg(datetime).arg(data.size()));

		tmpFiledat_.clear();
		{
			QDataStream stream(&tmpFiledat_, QIODevice::WriteOnly);
			stream << info.fileName() << datetime << isText << data;
		}

	}

}

void PluginManageWindow::on_exportpushButton_clicked(){

	if (tmpFiledat_.isEmpty() || ui->filelineEdit->text().isEmpty()){
		return;
	}

	QString fileName;
	QByteArray data;
	QString datetime;
	bool isText;
	{
		QDataStream stream(tmpFiledat_);
		stream >> fileName >> datetime >> isText >> data;
	}

	// ѡ�񵼳��ļ�·��
	QString filepath = QFileDialog::getSaveFileName(this, windowTitle(), QDir::tempPath() + UDQ_T("/") + fileName);
	if (filepath.size() > 0){

		QIODevice::OpenMode flag = QIODevice::WriteOnly | QIODevice::Truncate;
		if (isText){
			flag |= QIODevice::Text;
		}

		QFile file(filepath);
		if (file.open(flag)){
			file.write(data);
			file.close();
		}
	}

}

void PluginManageWindow::on_clearpushButton_clicked(){
	tmpFiledat_.clear();
	ui->filelineEdit->clear();
}

void PluginManageWindow::on_filepushButton_clicked(){
	PluginFileLinkWindow w(assosiateFiles_);
	w.exec();
}



void PluginManageWindow::on_plugintreeWidget_itemDoubleClicked(QTreeWidgetItem * item, int column){
	
}

void PluginManageWindow::on_typecomboBox_currentIndexChanged(int index){
	// ���������б�
	
	// ��������Item
	for (int i = 0; i < ui->attrlistWidget->count(); ++i){
		ui->attrlistWidget->item(i)->setHidden(true);
	}

	ui->exthspinBox->setEnabled(index == 2);
	ui->extvspinBox->setEnabled(index == 2);

	if (index < 0 || index > 2)
		return;

	foreach(QListWidgetItem* item, attrItemsGroup_.values(index)){
		item->setHidden(false);
	}

}

bool PluginManageWindow::checkValid(){

	LINEEDIT_CHECK(ui->idlineEdit, UDQ_TR(" ������"), UDQ_TR("���ID"));
	LINEEDIT_VALID(ui->idlineEdit, UDQ_TR(" ������"), UDQ_TR("���ID"), REG_NAMEID);

	LINEEDIT_CHECK(ui->namelineEdit, UDQ_TR(" ������"), UDQ_TR("�������"));
	COMBOX_CHECK(ui->pkgcomboBox, UDQ_TR(" ������"), UDQ_TR("��Դ��"));
	COMBOX_CHECK(ui->categorycomboBox, UDQ_TR(" ������"), UDQ_TR("������"));
	COMBOX_CHECK(ui->typecomboBox, UDQ_TR(" ������"), UDQ_TR("�������"));

	LINEEDIT_CHECK(ui->versionlineEdit, UDQ_TR(" ������"), UDQ_TR("����汾"));

	return true;
}

void PluginManageWindow::on_iconpushButton_clicked(){
	// ���ļ�
	QString filepath = QFileDialog::getOpenFileName(this, UDQ_TR("ѡ��ͼƬ"), QString(), ImageResourceObject::fileFilters());
	if (filepath.isEmpty())
		return;
	ui->iconwidget->showPicture(filepath);

}


QTreeWidgetItem* PluginManageWindow::findPropertyItem(int type, const QString& id, bool create){
	QTreeWidgetItem* parentItem = NULL;
	int itemtype;
	// ������ĿItem
	if (type == PT_VARIABLE ){
		parentItem = variableRootItem_;
		itemtype = ITEM_VARIABLE;
	}
	else{
		Q_UNREACHABLE();
	}
	
	QTreeWidgetItem* propItem = NULL;
	for (int i = 0; i < parentItem->childCount(); i++){
		if (parentItem->child(i)->text(0) == id){
			propItem = parentItem->child(i);
			break;
		}
	}

	if (propItem == NULL && create){
		propItem = new QTreeWidgetItem(parentItem, itemtype);
		propItem->setText(0, id);
		propItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled );
	}

	return propItem;
}

QTreeWidgetItem* PluginManageWindow::findInterfaceItem(int type, const QString& id, bool create){
	QTreeWidgetItem* parentItem = NULL;
	int itemtype;
	// ������ĿItem
	 if (type == PT_CONDITION){
		parentItem = conditionRootItem_;
		itemtype = ITEM_CONDITION;
	}
	else if (type == PT_ACTION){
		parentItem = actionRootItem_;
		itemtype = ITEM_ACTION;
	}
	else if (type == PT_EXPRESS){
		parentItem = expressRootItem_;
		itemtype = ITEM_EXPRESS;
	}

	QTreeWidgetItem* propItem = NULL;
	for (int i = 0; i < parentItem->childCount(); i++){
		if (parentItem->child(i)->data(0,ITEM_CONTEXT).toString() == id){
			propItem = parentItem->child(i);
			break;
		}
	}

	if (propItem == NULL && create){
		propItem = new QTreeWidgetItem(parentItem, itemtype);
		propItem->setText(0, id);
		propItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	}

	return propItem;
}

void PluginManageWindow::on_addproppushButton_clicked(){

	//int type = ui->typecomboBox->currentIndex();
	//if (type < 0){
	//	QMessageBox::warning(this, windowTitle(), UDQ_TR("��ѡ��������"));
	//	return;
	//}

	//QMultiHash<int, QString>  existIds;

	//// ����
	//for (int i = 0; i < variableRootItem_->childCount(); ++i){
	//	existIds.insert(PT_VARIABLE, variableRootItem_->child(i)->text(0));
	//}

	//// ����
	//for (int i = 0; i < conditionRootItem_->childCount(); ++i){
	//	existIds.insert(PT_CONDITION, conditionRootItem_->child(i)->text(0));
	//}

	//// ����
	//for (int i = 0; i < actionRootItem_->childCount(); ++i){
	//	existIds.insert(PT_ACTION, actionRootItem_->child(i)->text(0));
	//}

	//// ���ʽ
	//for (int i = 0; i < expressRootItem_->childCount(); ++i){
	//	existIds.insert(PT_EXPRESS, expressRootItem_->child(i)->text(0));
	//}

	//AddPluginPropertyWindow w(type, existIds);
	//w.exec();

	//// ��������
	//foreach(QByteArray dat, w.newVar_.values()){
	//		addVariableItem(dat);
	//}
}

void PluginManageWindow::on_delproppushButton_clicked(){

}

void PluginManageWindow::on_proptreeWidget_itemDoubleClicked(QTreeWidgetItem * item, int column){
	// �༭����
	//if (item == NULL || item->type() == ITEM_GROUP)
	//	return;

	//int type = ui->typecomboBox->currentIndex();
	//QByteArray data = item->data(0, ITEM_VALUE).toByteArray();

	//int proptype;
	//if (item->type() == ITEM_VARIABLE){
	//	proptype = PT_VARIABLE;
	//}
	//else if(item->type() == ITEM_VARIABLE){
	//	proptype = PT_VARIABLE;
	//}
	//else if (item->type() == ITEM_VARIABLE){
	//	proptype = PT_VARIABLE;
	//}
	//else if (item->type() == ITEM_VARIABLE){
	//	proptype = PT_VARIABLE;
	//}

	//QMultiHash<int, QString>  existIds;
	//if (proptype == PT_VARIABLE){
	//	// ����
	//	for (int i = 0; i < variableRootItem_->childCount(); ++i){
	//		existIds.insert(PT_VARIABLE, variableRootItem_->child(i)->text(0));
	//	}
	//}
	//else if (proptype == PT_CONDITION){
	//	// ����
	//	for (int i = 0; i < conditionRootItem_->childCount(); ++i){
	//		existIds.insert(PT_CONDITION, conditionRootItem_->child(i)->text(0));
	//	}
	//}
	//else if (proptype == PT_ACTION){
	//	// ����
	//	for (int i = 0; i < actionRootItem_->childCount(); ++i){
	//		existIds.insert(PT_ACTION, actionRootItem_->child(i)->text(0));
	//	}
	//}
	//else if (proptype == PT_EXPRESS){
	//	// ���ʽ
	//	for (int i = 0; i < expressRootItem_->childCount(); ++i){
	//		existIds.insert(PT_EXPRESS, expressRootItem_->child(i)->text(0));
	//	}
	//}

	//AddPluginPropertyWindow w(type, existIds, data);
	//if (w.exec() == QDialog::Accepted){

	//	if (proptype == PT_VARIABLE){
	//		// ��������
	//		foreach(QByteArray dat, w.newVar_.values()){
	//			addVariableItem(dat);
	//		}
	//	}
	//	else if (proptype == PT_CONDITION){
	//		// ��������
	//		foreach(QByteArray dat, w.newCnd_.values()){
	//			//addConditionItem(PT_CONDITION, dat);
	//		}
	//	}
	//}
}

void PluginManageWindow::clearChildItem(QTreeWidgetItem* parent){
	if (parent == NULL)
		return;

	while (parent->childCount() > 0 ){
		QTreeWidgetItem* item = parent->takeChild(0);
		delete item;
	}
}

void PluginManageWindow::on_jsonimppushButton_clicked(){
	// ��JSON�ļ�����
	QString filepath = QFileDialog::getOpenFileName(this, UDQ_TR("ѡ���������ļ�"), QString(), UDQ_TR("JSON�ļ�(*.json)"));
	if (filepath.isEmpty())
		return;

	QFile f(filepath);
	// ��ȡ������
	if (!f.open(QIODevice::ReadOnly)) {
		return;
	}

	QByteArray jsonData = f.readAll();
	f.close();

	QJsonParseError error;
	QJsonDocument loadDoc = QJsonDocument::fromJson(jsonData, &error);

	// ��������
	if (error.error != QJsonParseError::NoError){
		QMessageBox::critical(this, windowTitle(), UDQ_TR("JSON�ļ�����ʧ��:%1").arg(error.errorString()));
		return;
	}

	if(!loadDoc.isObject() ){
		QMessageBox::critical(this, windowTitle(), UDQ_TR("�����JSON��ʽ����ȷ"));
		return;
	}

	tmpJsonDat_ = loadDoc.toBinaryData();
	QJsonObject rootObj = loadDoc.object();

	QString type = rootObj.value(JSON_TYPE).toString();
	if (type == UDQ_T("Entity")){
		ui->typecomboBox->setCurrentIndex(0);
	}
	else if (type == UDQ_T("Behavior")){
		ui->typecomboBox->setCurrentIndex(1);
	}
	else if (type == UDQ_T("Effect")){
		ui->typecomboBox->setCurrentIndex(2);
	}
	else if (type == UDQ_T("Native")){
		ui->typecomboBox->setCurrentIndex(3);
	}

	// ��������
	ui->idlineEdit->setText(rootObj.value(JSON_ID).toString());
	ui->namelineEdit->setText(rootObj.value(JSON_NAME).toString());
	ui->categorycomboBox->setCurrentText(rootObj.value(JSON_CATEGORY).toString());
	ui->versionlineEdit->setText(rootObj.value(JSON_VER).toString());
	ui->plainTextEdit->setPlainText(rootObj.value(JSON_DESC).toString());
	
	int exth = 0;
	int extv = 0;
	if (rootObj.contains(UDQ_T("extendbox"))){
		QJsonArray arr = rootObj.value(UDQ_T("extendbox")).toArray();
		if (arr.size() > 0){
			exth = arr[0].toString().toInt();
		}
		if (arr.size() > 1){
			extv = arr[1].toString().toInt();
		}
	}
	ui->exthspinBox->setValue(exth);
	ui->extvspinBox->setValue(extv);

	// ����
	for (int i = 0; i < ui->attrlistWidget->count(); i++){
		ui->attrlistWidget->item(i)->setCheckState(Qt::Unchecked);
	}
	QString attr;
	QJsonArray arr = rootObj.value(UDQ_T("attributes")).toArray();
	for (int i = 0; i < arr.size(); i++){
		attr = arr[i].toString();
		if (attrStringItems_.contains(attr)){
			attrStringItems_[attr]->setCheckState(Qt::Checked);
		}
	}

	// ����
	clearChildItem(variableRootItem_);
	arr = rootObj.value(UDQ_T("variables")).toArray();
	for (int i = 0; i < arr.size(); i++){
		Q_ASSERT(arr[i].isObject());
		QJsonObject child = arr[i].toObject();
		addVariableItem(child);
	}

	// ����
	clearChildItem(conditionRootItem_);
	arr = rootObj.value(UDQ_T("conditions")).toArray();
	for (int i = 0; i < arr.size(); i++){
		Q_ASSERT(arr[i].isObject());
		QJsonObject child = arr[i].toObject();
		addInterfaceItem(PT_CONDITION, child);
	}

	// ����
	clearChildItem(actionRootItem_);
	arr = rootObj.value(UDQ_T("actions")).toArray();
	for (int i = 0; i < arr.size(); i++){
		Q_ASSERT(arr[i].isObject());
		QJsonObject child = arr[i].toObject();
		addInterfaceItem(PT_ACTION, child);
	}

	// ���ʽ
	clearChildItem(expressRootItem_);
	arr = rootObj.value(UDQ_T("expresses")).toArray();
	for (int i = 0; i < arr.size(); i++){
		Q_ASSERT(arr[i].isObject());
		QJsonObject child = arr[i].toObject();
		addInterfaceItem(PT_EXPRESS, child);
	}

	ui->proptreeWidget->expandAll();
}

void PluginManageWindow::on_jsonexppushButton_clicked(){
	// TODO: ��JSON�ļ�����
	QMessageBox::warning(this, windowTitle(), UDQ_TR("�ù�����δʵ��"));
}

QStringList acquireStringList(QJsonArray& arr){
	QStringList params;
	for (int i = 0; i < arr.size(); i++){
		params << arr[i].toString();
	}
	return params;
}

void parseVariable(const QJsonObject& data, PluginVariable* var){
	var->name = data.value(JSON_NAME).toString().trimmed();
	var->title = data.value(JSON_TITLE).toString().trimmed();
	QJsonArray tmparr;
	if (data.contains(JSON_ENUM)){
		tmparr = data.value(JSON_ENUM).toArray();
		var->enums = acquireStringList(tmparr);
	}
	var->desc = data.value(JSON_DESC).toString();
	var->datatype = data.value(JSON_TYPE).toString().trimmed();
	var->value = data.value(JSON_DEF).toString();

	// ��������
	if (var->datatype == UDQ_T("font")){
		bool bold = false;
		bool italic = false;
		int pt = 0;
		QString fnt = var->value;
		QString family;
		QFont font;

		int index = 0;
		bool isFamily = false;
		QRegExp exp(UDQ_T("\\d+pt"));
		QString substr;
		int i = fnt.indexOf(UDQ_T(" "), index);
		while (i >= 0){
			substr = fnt.mid(index, i - index);

			if (substr.toLower() == UDQ_T("bold")){
				bold = true;
			}
			else if (substr.toLower() == UDQ_T("italic")){
				italic = true;
			}
			else if (substr.contains(exp)){
				pt = substr.toUInt();
				break;
			}
			index = i;
			i = fnt.indexOf(UDQ_T(" "), index);
		}

		if (i > 0){
			family = fnt.mid(i + 1).trimmed();
		}

		if (family.size() > 0 && pt > 0){
			font = QFont(family, pt, bold ? QFont::Bold : QFont::Normal, italic);
			font.setStyleStrategy(QFont::PreferAntialias);
		}
		else{
			font.setStyleStrategy(QFont::PreferAntialias);
			font.setPixelSize(24);
			font.setFamily(UDQ_L8("����"));
		}
		var->value = font.toString();
	}

	bool extend = data.value(JSON_EXTEND).toBool();
	if (extend){
		var->flags = VAR_EXTEND;
	}
}


QTreeWidgetItem* PluginManageWindow::addVariableItem( PluginVariable* var ){
	Q_ASSERT(var != NULL);
	// ����Item
	QTreeWidgetItem* item = findPropertyItem(PT_VARIABLE, var->name);
	Q_ASSERT(item != NULL);
	item->setData(0, ITEM_VALUE, var->toByteArray() ); 
	item->setData(0, ITEM_CONTEXT, var->flags);

	item->setToolTip(0, var->desc );
	item->setText(1, UDQ_TR("%1 : %2")
		.arg(var->datatype)
		.arg(var->desc));
	item->setText(2, var->value);

	return item;
}


QTreeWidgetItem* PluginManageWindow::addVariableItem(const QJsonObject& data){

	if (!data.contains(JSON_NAME))
		return NULL;

	PluginVariable var;
	parseVariable(data, &var);

	return addVariableItem(&var);
}


QTreeWidgetItem* PluginManageWindow::addInterfaceItem(PluginInterface* inter){
	Q_ASSERT(inter != NULL);

	// ����Item
	QTreeWidgetItem* item = findInterfaceItem(inter->type, inter->name);
	Q_ASSERT(item != NULL);
	item->setData(0, ITEM_CONTEXT, inter->func);
	item->setData(0, ITEM_VALUE, inter->toByteArray());

	// ��DESC�е�%1�滻Ϊ������
	QString desc = inter->desc;
	for (int i = 0; i < inter->params.size(); i++){
		desc = desc.replace(UDQ_TR("%") + QString::number(i + 1), inter->params.at(i)->name);
	}
	item->setToolTip(0, desc);

	item->setText(1, UDQ_TR("%1 : %2")
		.arg(inter->func)
		.arg(desc));

	return item;
}


QTreeWidgetItem* PluginManageWindow::addInterfaceItem(int type, const QJsonObject& data){

	if (!data.contains(JSON_NAME))
		return NULL;
	PluginInterface inter;
	inter.type = type;
	inter.name = data.value(JSON_NAME).toString();

	QJsonArray tmparr;
	if (data.contains(JSON_FLAGS)){
		tmparr = data.value(JSON_FLAGS).toArray();
		inter.flags = acquireStringList(tmparr);
	}
	inter.desc = data.value(JSON_DESC).toString();
	inter.func = data.value(JSON_FUNC).toString().trimmed();
	inter.category = data.value(JSON_CATEGORY).toString();

	// ����
	QJsonValue v = data.value(JSON_PARAM);
	if (v.isArray() ){ // �����������
		QJsonArray arr = v.toArray();
		for (int i = 0; i < arr.size(); i++){
			QJsonObject pd = arr[i].toObject();
			PluginVariable* var = inter.params.add();
			parseVariable(pd, var);
		}
	}

	return addInterfaceItem(&inter);
}

void PluginManageWindow::on_deletepushButton_clicked(){
	
	QTreeWidgetItem* item = ui->plugintreeWidget->currentItem();
	if (item == NULL || item->type() == ITEM_PLUGINGROUP || item->type() == ITEM_CATEGORY){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("��ѡ��һ�����!"));
		return;
	}

	ResourceHash key = item->data(0, ITEM_HASHKEY).toUInt();
	DEFINE_RESOURCE_HASHKEY_VALID(PluginResourceObject, plugin, key);

	// �������Ѿ����ڣ��������ڲ����
	if (plugin->devmode_ && !gConfigManager->devMode()){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("����ɾ��ϵͳ�ڲ����!"));
		return;
	}

	if (QMessageBox::question(this, windowTitle(), UDQ_TR("ȷ��ɾ�����<%1>?").arg(plugin->objectName())) == QMessageBox::No){
		return;
	}

	ui->plugintreeWidget->clearSelection();
	plugin->setFlags(URF_TagGarbage);

	item->parent()->takeChild(item->parent()->indexOfChild(item));
	delete item;

}