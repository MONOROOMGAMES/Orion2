#include "kismeteventwindow.h"
#include "ui_kismeteventwindow.h"
#include "tooldefine.h"
#include "eventtreewidget.h"

const QString NAME_CONDITION = UDQ_TR("��������");
const QString NAME_ACTION = UDQ_TR("����");
const QString NAME_EXPRESS = UDQ_TR("���ʽ");
const QString NAME_SYSTEM = UDQ_TR("System");
const QString NAME_COMMON = UDQ_TR("Common");

KismetEventWindow::KismetEventWindow(QWidget *parent) :
QWidget(parent),
ui(new Ui::KismetEventWindow)
{
	ui->setupUi(this);

	aces_.insert(Value2String(EF_POSITION), EF_POSITION);
	aces_.insert(Value2String(EF_SIZE), EF_SIZE);
	aces_.insert(Value2String(EF_ANGLE), EF_ANGLE);
	aces_.insert(Value2String(EF_ZORDER), EF_ZORDER);
	aces_.insert(Value2String(EF_APPEARANCE), EF_APPEARANCE);
	aces_.insert(Value2String(EF_EFFECTS), EF_EFFECTS);
	aces_.insert(Value2String(EF_SINGLEGLOBAL), EF_SINGLEGLOBAL);

	folderIcon_ = QIcon(UDQ_T(":/images/folder.png"));
	collectCommonEvent();

	// ��ʾϵͳ����ӿ�
	currentPlugin_ = PluginResourceObject::acquirePlugin(UDQ_T("System"));
	prepareEventList(currentPlugin_);
	// 
}

KismetEventWindow::~KismetEventWindow()
{
	delete ui;
}

void	KismetEventWindow::resizeEvent(QResizeEvent * event){

	QSize newSize = event->size() - QSize(WIDGET_MARGIN * 2, WIDGET_MARGIN * 2);
	ui->layoutWidget->setGeometry(QRect(QPoint(WIDGET_MARGIN, WIDGET_MARGIN), newSize));

	QSize tmp = ui->evttabWidget->geometry().size();
	// ����Tab�ؼ���С
	QSize tabsize = ui->stackedWidget->geometry().size();
	tabsize.setWidth(tabsize.width() - 8);
	tabsize.setHeight(tabsize.height() - 8);

	ui->layoutWidget1->setGeometry(QRect(QPoint(WIDGET_MARGIN * 2, WIDGET_MARGIN * 2), tabsize));
	ui->plugintreeWidget->setGeometry(QRect(QPoint(WIDGET_MARGIN*2, WIDGET_MARGIN * 2), tabsize));

	// �������ؼ���С
	tabsize = ui->evttabWidget->geometry().size();
	tabsize.setWidth(tabsize.width() - 8);
	tabsize.setHeight(tabsize.height() - 24);

	ui->cndtreeWidget->setGeometry(QRect(QPoint(WIDGET_MARGIN, WIDGET_MARGIN * 2), tabsize));
	ui->acttreeWidget->setGeometry(QRect(QPoint(WIDGET_MARGIN, WIDGET_MARGIN * 2), tabsize));
	ui->exptreeWidget->setGeometry(QRect(QPoint(WIDGET_MARGIN, WIDGET_MARGIN * 2), tabsize));

}


void KismetEventWindow::clear(){
	ui->cndtreeWidget->clear();
	ui->acttreeWidget->clear();
	ui->exptreeWidget->clear();
	interfaceItems_.clear();
}

// ׼���¼��嵥
void KismetEventWindow::prepareEventList(PluginResourceObject* plugin){
	clear();
	if (plugin->flags() & URF_TagGarbage || plugin->objectName() == UDQ_T("Common") || plugin->type_ == PIT_EFFECT)
		return;

	// ��ʾ�����
	ui->pluginpushButton->setText(plugin->title_);
	ui->pluginpushButton->setIcon(QPixmap::fromImage(plugin->icon_));

	//  ͳ�Ʋ�����Ա�־
	QList<int> attrs;
	attrs.append(0);
	QMapIterator<int, int> iter(plugin->attributes_);
	while (iter.hasNext()){
		iter.next();
		if (iter.key() == EF_POSITION && iter.value() == 1){
			attrs.append(EF_POSITION);
		}
		if (iter.key() == EF_SIZE && iter.value() == 1){
			attrs.append(EF_SIZE);
		}
		if (iter.key() == EF_ANGLE && iter.value() == 1){
			attrs.append(EF_ANGLE);
		}
		if (iter.key() == EF_ZORDER && iter.value() == 1){
			attrs.append(EF_ZORDER);
		}
		if (iter.key() == EF_APPEARANCE && iter.value() == 1){
			attrs.append(EF_APPEARANCE);
		}
		if (iter.key() == EF_EFFECTS && iter.value() == 1){
			attrs.append(EF_EFFECTS);
		}
		if (iter.key() == EF_SINGLEGLOBAL && iter.value() == 0){  //ע�⣺ �ǵ�ʵ�����
			attrs.append(EF_SINGLEGLOBAL);
		}
	}

	QIcon icon = QPixmap::fromImage(plugin->icon_);


	// �������
	ui->cndtreeWidget->invisibleRootItem()->setData(0, ITEM_HASHKEY, currentPlugin_->hashKey());
	addCommonConditionEvent(plugin, attrs, ui->cndtreeWidget->invisibleRootItem());
	addConditionEvent(plugin, ui->cndtreeWidget->invisibleRootItem());
	ui->cndtreeWidget->expandAll();

	// ��Ӷ���
	ui->acttreeWidget->invisibleRootItem()->setData(0, ITEM_HASHKEY, currentPlugin_->hashKey());
	addCommonActionEvent(plugin, attrs, ui->acttreeWidget->invisibleRootItem());
	addActionEvent(plugin, ui->acttreeWidget->invisibleRootItem());
	ui->acttreeWidget->expandAll();

	// ��ӱ��ʽ
	ui->exptreeWidget->invisibleRootItem()->setData(0, ITEM_HASHKEY, currentPlugin_->hashKey());
	addCommonExpressEvent(plugin, attrs, ui->exptreeWidget->invisibleRootItem());
	addExpressEvent(plugin, ui->exptreeWidget->invisibleRootItem());
	ui->exptreeWidget->expandAll();

}

QTreeWidgetItem* KismetEventWindow::addConditionItem(PluginInterface* inter, QTreeWidgetItem* rootItem){

	Q_ASSERT(inter != NULL);

	QTreeWidgetItem* parent = NULL;
	if (inter->category.isEmpty()){
		parent = rootItem;
	}
	else{
		parent = findPropertyItem(ITEM_CATEGORY, inter->category, rootItem);
		Q_ASSERT(parent != NULL);
		parent->setIcon(0, folderIcon_);
		parent->setData(0, ITEM_HASHKEY, currentPlugin_->hashKey());
	}

	// ����Item
	QTreeWidgetItem* item = findPropertyItem(ITEM_CONDITION, inter->name, parent);
	Q_ASSERT(item != NULL);
	item->setData(0, ITEM_VALUE, hashString(inter->func));

	interfaceItems_.append(item);

	return item;

}

QTreeWidgetItem* KismetEventWindow::addActionItem(PluginInterface* inter, QTreeWidgetItem* rootItem){

	Q_ASSERT(inter != NULL);


	QTreeWidgetItem* parent = NULL;
	if (inter->category.isEmpty()){
		parent = rootItem;
	}
	else{
		parent = findPropertyItem(ITEM_CATEGORY, inter->category, rootItem);
		Q_ASSERT(parent != NULL);
		parent->setIcon(0, folderIcon_);
		parent->setData(0, ITEM_HASHKEY, currentPlugin_->hashKey());
	}


	// ����Item
	QTreeWidgetItem* item = findPropertyItem(ITEM_ACTION, inter->name, parent);
	Q_ASSERT(item != NULL);
	item->setData(0, ITEM_VALUE, hashString(inter->func));
	interfaceItems_.append(item);

	return item;

}


QTreeWidgetItem* KismetEventWindow::addExpressItem(PluginInterface* inter, QTreeWidgetItem* rootItem){

	Q_ASSERT(inter != NULL);

	QTreeWidgetItem* parent = NULL;
	if (inter->category.isEmpty()){
		parent = rootItem;
	}
	else{
		parent = findPropertyItem(ITEM_CATEGORY, inter->category, rootItem);
		Q_ASSERT(parent != NULL);
		parent->setIcon(0, folderIcon_);
		parent->setData(0, ITEM_HASHKEY, currentPlugin_->hashKey());
	}


	// ����Item
	QTreeWidgetItem* item = findPropertyItem(ITEM_EXPRESS, inter->name, parent);
	Q_ASSERT(item != NULL);
	item->setData(0, ITEM_VALUE, hashString(inter->func));
	interfaceItems_.append(item);

	return item;

}


QTreeWidgetItem* KismetEventWindow::findPropertyItem(int type, const QString& id, QTreeWidgetItem* parentItem, bool create){

	QTreeWidgetItem* propItem = NULL;
	for (int i = 0; i < parentItem->childCount(); i++){
		if (parentItem->child(i)->text(0) == id){
			propItem = parentItem->child(i);
			break;
		}
	}

	if (propItem == NULL && create){
		propItem = new QTreeWidgetItem(parentItem, type);
		propItem->setText(0, id);
		propItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled | (type == ITEM_CATEGORY ? Qt::NoItemFlags : Qt::ItemIsDragEnabled));
	}

	return propItem;
}

void KismetEventWindow::collectCommonEvent(){

	commonCondition_.clear();
	commonAction_.clear();
	commonExpress_.clear();

	// �ҵ�Common�ӿڲ��
	PluginResourceObject* commonPlugin = PluginResourceObject::acquireCommonPlugin();
	if (commonPlugin){
		foreach(PluginInterface* inter, commonPlugin->conditions_.values()){
			addCommonEventItem(PT_CONDITION, inter);
		}

		foreach(PluginInterface* inter, commonPlugin->actions_.values()){
			addCommonEventItem(PT_ACTION, inter);
		}

		foreach(PluginInterface* inter, commonPlugin->expresses_.values()){
			addCommonEventItem(PT_EXPRESS, inter);
		}
	}
}


void KismetEventWindow::addCommonEventItem(int type, PluginInterface* inter){

	ResourceHash key = 0;  // 0��ʾ���������߱��Ľӿ�
	foreach(QString f, inter->flags){
		if (aces_.contains(f)){
			key = aces_[f];
			break;
		}
	}

	if (type == PT_CONDITION){
		commonCondition_.insert(key, inter);
	}
	else if (type == PT_ACTION){
		commonAction_.insert(key, inter);
	}
	else if (type == PT_EXPRESS){
		commonExpress_.insert(key, inter);
	}
}


// ��ӹ����ӿ�
void KismetEventWindow::addCommonConditionEvent(PluginResourceObject* plugin, const QList<int>& attrs, QTreeWidgetItem* rootItem){

	// ����
	QTreeWidgetItem* item = NULL;
	QHashIterator<int, PluginInterface*> cnditer(commonCondition_);
	while (cnditer.hasNext()){
		cnditer.next();
		if (attrs.contains(cnditer.key())){
			item = addConditionItem(cnditer.value(), rootItem);
			item->setData(0, ITEM_HASHKEY, (int)PT_CONDITION);
		}
	}

}

void KismetEventWindow::addConditionEvent(PluginResourceObject* plugin, QTreeWidgetItem* rootItem){

	// ����
	QTreeWidgetItem* item = NULL;
	foreach(PluginInterface* inter, plugin->conditions_.values()){
		item = addConditionItem(inter, rootItem);
		item->setData(0, ITEM_HASHKEY, (int)PT_CONDITION);
	}
}


// ��ӹ����ӿ�
void KismetEventWindow::addCommonActionEvent(PluginResourceObject* plugin, const QList<int>& attrs, QTreeWidgetItem* rootItem){

	QIcon folderIcon = QIcon(UDQ_T(":/images/folder.png"));
	// ������Ŀ��Ŀ

	QTreeWidgetItem* item = NULL;
	// ����
	QHashIterator<int, PluginInterface*> actiter(commonAction_);
	while (actiter.hasNext()){
		actiter.next();
		if (attrs.contains(actiter.key())){
			item = addActionItem(actiter.value(), rootItem);
			item->setData(0, ITEM_HASHKEY, (int)PT_ACTION);
		}
	}
}

void KismetEventWindow::addActionEvent(PluginResourceObject* plugin, QTreeWidgetItem* rootItem){

	// ����
	QTreeWidgetItem* item = NULL;
	foreach(PluginInterface* inter, plugin->actions_.values()){
		item = addActionItem(inter, rootItem);
		item->setData(0, ITEM_HASHKEY, (int)PT_ACTION);
	}

}

// ��ӹ����ӿ�
void KismetEventWindow::addCommonExpressEvent(PluginResourceObject* plugin, const QList<int>& attrs, QTreeWidgetItem* rootItem){

	QIcon folderIcon = QIcon(UDQ_T(":/images/folder.png"));
	// ������Ŀ��Ŀ


	QTreeWidgetItem* item = NULL;
	// ���ʽ
	QHashIterator<int, PluginInterface*> expiter(commonExpress_);
	while (expiter.hasNext()){
		expiter.next();
		if (attrs.contains(expiter.key())){
			item = addExpressItem(expiter.value(), rootItem);
			item->setData(0, ITEM_HASHKEY, (int)PT_EXPRESS);
		}
	}

}

void KismetEventWindow::addExpressEvent(PluginResourceObject* plugin, QTreeWidgetItem* rootItem){

	QTreeWidgetItem* item = NULL;
	// ���ʽ
	foreach(PluginInterface* inter, plugin->expresses_.values()){
		item = addExpressItem(inter, rootItem);
		item->setData(0, ITEM_HASHKEY, (int)PT_EXPRESS);
	}

}



void KismetEventWindow::on_refreshpushButton_clicked(){
	// ˢ�¹����ӿ�
	collectCommonEvent();
	//  ˢ�µ�ǰ����ӿ�
	prepareEventList( currentPlugin_);
}


void KismetEventWindow::on_pluginpushButton_clicked(){

	// ��ʾ������б�
	if (ui->pluginpushButton->isChecked()){
		// ˢ�²�����б�
		preparePluginList();
		ui->stackedWidget->setCurrentIndex(1);
		ui->plugintreeWidget->expandAll();
	}
	else{
		ui->stackedWidget->setCurrentIndex(0);
	}

}

void KismetEventWindow::on_plugintreeWidget_itemClicked(QTreeWidgetItem * item, int column){
	if (item == NULL || item->type() == ITEM_CATEGORY)
		return;

	ResourceHash key = item->data(0, ITEM_HASHKEY).toUInt();
	DEFINE_RESOURCE_HASHKEY_VALID(PluginResourceObject, plugin, key);
	currentPlugin_ = plugin;

	//  ˢ�²����
	prepareEventList(currentPlugin_);
	ui->stackedWidget->setCurrentIndex(0);
	ui->pluginpushButton->setChecked(false);
}

void KismetEventWindow::preparePluginList(){
	ui->plugintreeWidget->clear();

	// ����ʵ���
	QTreeWidgetItem* entityRootItem = new QTreeWidgetItem(ITEM_CATEGORY);
	entityRootItem->setText(0, UDQ_TR("ʵ��"));
	entityRootItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui->plugintreeWidget->addTopLevelItem(entityRootItem);

	// ������Ϊ��
	QTreeWidgetItem* behaviorRootItem = new QTreeWidgetItem(ITEM_CATEGORY);
	behaviorRootItem->setText(0, UDQ_TR("��Ϊ"));
	behaviorRootItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
	ui->plugintreeWidget->addTopLevelItem(behaviorRootItem);

	QString cate;
	for (TResourceObjectIterator<PluginResourceObject> it; it; ++it){
		PluginResourceObject* plugin = (PluginResourceObject*)(*it);

		if (plugin->flags() & URF_TagGarbage || plugin->objectName() == UDQ_T("Common") || plugin->type_ == PIT_EFFECT )
			continue;

		Q_ASSERT(plugin->categories_.size() > 0);
		cate = plugin->categories_[0];
		Q_ASSERT(cate != NULL);

		QTreeWidgetItem* categoryItem = NULL;
		QTreeWidgetItem* pluginItem = NULL;

		if (plugin->type_ == PIT_ENTITY){
			categoryItem = findCateory(cate, entityRootItem);
			pluginItem = new QTreeWidgetItem(ITEM_ENTITY);
		}
		else{
			categoryItem = findCateory(cate, behaviorRootItem);
			pluginItem = new QTreeWidgetItem(ITEM_BEHAVIOR);
		}

		pluginItem->setText(0, plugin->title_);
		pluginItem->setIcon(0, QPixmap::fromImage(plugin->icon_));
		pluginItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
		pluginItem->setData(0, ITEM_HASHKEY, plugin->hashKey());
		pluginItem->setToolTip(0, plugin->description());
		categoryItem->addChild(pluginItem);
	}

	ui->plugintreeWidget->expandAll();

}

QTreeWidgetItem* KismetEventWindow::findCateory(const QString& cate, QTreeWidgetItem* rootItem, bool create){

	QTreeWidgetItem* categoryItem = NULL;
	for (int i = 0; i < rootItem->childCount(); i++){
		if (rootItem->child(i)->text(0) == cate){
			categoryItem = rootItem->child(i);
			break;
		}
	}

	if (categoryItem == NULL && create){
		categoryItem = new QTreeWidgetItem(ITEM_CATEGORY);
		categoryItem->setText(0, cate);
		categoryItem->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled );
		rootItem->addChild(categoryItem);
	}

	return categoryItem;
}

void KismetEventWindow::on_filterlineEdit_textEdited(const QString & text){
	// �������нӿ�Item
	foreach(QTreeWidgetItem* item, interfaceItems_){
		if (item->text(0).contains(text, Qt::CaseInsensitive)){  // ���ְ���( ��Сд������ )
			item->setHidden(false);
		}
		else{
			item->setHidden(true);
		}
	}
}