/************************************************************************
**
**  Copyright (C) 2015  guobin  <guobin.gb@alibaba-inc.com>
**
**  This file is part of Orion2.
**
**  Orion2 is free software: you can redistribute it and/or modify
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

#include "plugintypeeditwindow.h"
#include "ui_plugintypeeditwindow.h"
#include "componentdefine.h"
#include "tooldefine.h"
#include "addinstancevariablewindow.h"
#include "addinstancebehaviorwindow.h"
#include "addinstanceeffectwindow.h"
#include "selectspritewindow.h"

PluginTypeEditWindow::PluginTypeEditWindow(ObjectTypeResourceObject* objType, QWidget *parent) :
QDialog(parent), objtype_(objType), varRoot_(NULL), behaviorRoot_(NULL), effectRoot_(NULL),
    ui(new Ui::PluginTypeEditWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	init();

}

PluginTypeEditWindow::~PluginTypeEditWindow()
{
    delete ui;
}


void PluginTypeEditWindow::init(){
	PluginResourceObject* plugin = objtype_->prototype_;  // ���ԭ��
	Q_ASSERT(plugin != NULL);

	//////////////////////////////////////////////////////////////////////////
	//
	ui->pluginlineEdit->setText(plugin->title_);
	ui->namelineEdit->setText(objtype_->description());
	ui->globalcheckBox->setChecked(objtype_->global_);

	if (plugin->hasAttribute(EF_ANIMATIONS)){
		ui->animpushButton->setEnabled(true);
	}

	if (plugin->hasAttribute(EF_EFFECTS)){
		ui->effectpushButton->setEnabled(true);
	}

	if (plugin->hasAttribute(EF_BEHAVIORS)){
		ui->behaviorpushButton->setEnabled(true);
	}

	// ��ʵ������û��ʵ������
	if (!plugin->hasAttribute(EF_SINGLEGLOBAL)){
		ui->varpushButton->setEnabled(true);
	}

	sprite_ = objtype_->animation_;
	image_ = objtype_->staticImage_;
	updateImage();

	// ʵ��
	if (varRoot_ == NULL){
		varRoot_ = new QTreeWidgetItem(ITEM_CATEGORY);
		varRoot_->setText(0, UDQ_TR("����"));
		ui->treeWidget->addTopLevelItem(varRoot_);
	}

	foreach(PluginVariable* def, objtype_->variables_.values())
	{
		addVariableItem(def, varRoot_);
	}

	// ��Ϊ
	if (behaviorRoot_ == NULL){
		behaviorRoot_ = new QTreeWidgetItem(ITEM_CATEGORY);
		behaviorRoot_->setText(0, UDQ_TR("��Ϊ"));
		ui->treeWidget->addTopLevelItem(behaviorRoot_);
	}

	foreach(QString name, objtype_->behaviors_)
	{
		addBehaviorItem(name, behaviorRoot_);
	}

	// ��Ч
	if (effectRoot_ == NULL){
		effectRoot_ = new QTreeWidgetItem(ITEM_CATEGORY);
		effectRoot_->setText(0, UDQ_TR("��Ч"));
		ui->treeWidget->addTopLevelItem(effectRoot_);
	}

	foreach(QString name, objtype_->effects_)
	{
		addEffectItem(name, effectRoot_);
	}

	ui->treeWidget->expandAll();
}


void PluginTypeEditWindow::updateImage(){
	PluginResourceObject* plugin = objtype_->prototype_;  // ���ԭ��
	Q_ASSERT(plugin != NULL);

	QImage image;
	// ͼ��
	if (hashStringLower(plugin->objectName()) == COMP_SPRITE){
		if (sprite_ != NULL){
			SpriteSequenceResourceObject* seq = NULL;
			QString seqName = objtype_->getParameter(UDQ_T("InitSequence"), objtype_->prototype_->objectName());
			if (seqName.isEmpty()){
				// ��ȡ��һ������
				seq =  sprite_->sequences_.size() > 0 ? sprite_->sequences_[0] : NULL;
			}
			else{
				seq = sprite_->sequence(seqName);
			}

			if (seq){
				int index = objtype_->getParameter(UDQ_T("InitFrame")).toInt();
				image = seq->frameImage(index);
			}
			ui->imglabel->setText(sprite_->description());
		}
		else if (image_ != NULL){
			image = image_->image_;
			ui->imglabel->setText(image_->objectName());
		}
	}
	else if (hashStringLower(plugin->objectName()) == COMP_TILEDBG){
		if (image_ != NULL){
			image = image_->image_;
			ui->imglabel->setText(image_->objectName());
		}
	}

	ui->previewwidget->showPicture(image);
}

QTreeWidgetItem* PluginTypeEditWindow::addVariableItem(PluginVariable* def, QTreeWidgetItem* parentItem){
	return addVariableItem(def, def->value, parentItem);
}

QTreeWidgetItem* PluginTypeEditWindow::addVariableItem(PluginVariable* def, const QString& value, QTreeWidgetItem* parentItem){

	QTreeWidgetItem* item = new QTreeWidgetItem(parentItem, ITEM_ATTRIBUTE);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item->setText(0, def->name);
	item->setToolTip(0, def->desc);
	item->setText(1, def->datatype);
	item->setText(2, value);
	item->setData(0, ITEM_HASHKEY, hashString(def->name));  // ������ϣֵ

	return item;
}


QTreeWidgetItem* PluginTypeEditWindow::findPropertyItem(const QString& name, QTreeWidgetItem* parentItem){
	if (parentItem == NULL)
		return NULL;

	int num = parentItem->childCount();
	for (int i = 0; i < num; i++){
		if (parentItem->child(i)->text(0) == name){
			return parentItem->child(i);
		}
	}

	return NULL;
}


QTreeWidgetItem* PluginTypeEditWindow::addBehaviorItem(const QString& name, QTreeWidgetItem* parentItem){
	PluginResourceObject* plugin = objtype_->attachments_.value(name);
	Q_ASSERT(plugin != NULL);

	QTreeWidgetItem* item = new QTreeWidgetItem(parentItem, ITEM_BEHAVIOR);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item->setText(0, name);
	item->setText(1, plugin->title_);
	item->setData(0, ITEM_HASHKEY, plugin->hashKey());  // ������ϣֵ
	return item;
}

QTreeWidgetItem* PluginTypeEditWindow::addEffectItem(const QString& name, QTreeWidgetItem* parentItem){
	PluginResourceObject* plugin = objtype_->attachments_.value(name);
	Q_ASSERT(plugin != NULL);

	QTreeWidgetItem* item = new QTreeWidgetItem(parentItem, ITEM_EFFECT);
	item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
	item->setText(0, name);
	item->setText(1, plugin->title_);
	item->setData(0, ITEM_HASHKEY, plugin->hashKey());  // ������ϣֵ
	return item;
}

//////////////////////////////////////////////////////////////////////////

bool PluginTypeEditWindow::canSave(){

	LINEEDIT_CHECK(ui->namelineEdit, UDQ_TR("�������ͱ༭"), UDQ_TR("����"));

	QString value = ui->namelineEdit->text();
	// �������
	ObjectTypeResourceObject* find = ((ProjectResourceObject*)objtype_->outer())->getObjectType(value);
	if (find != NULL && find != objtype_){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("����������<%1>��������").arg(value));
		return false;
	}

	// �������Ƿ�����
	QStringList vars;
	for (int i = 0; i < varRoot_->childCount(); i++){
		vars.append(varRoot_->child(i)->text(0));
	}
	if (vars.removeDuplicates() != 0){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("ʵ������������������!"));
		return false;
	}

	// ������Ƿ�����
	QStringList plugins;
	plugins.append(objtype_->prototype_->objectName());

	for (int i = 0; i < behaviorRoot_->childCount(); i++){
		plugins.append(behaviorRoot_->child(i)->text(0));
	}
	for (int i = 0; i < effectRoot_->childCount(); i++){
		plugins.append(effectRoot_->child(i)->text(0));
	}

	if (plugins.removeDuplicates() != 0){
		QMessageBox::warning(this, windowTitle(), UDQ_TR("��Ϊ/��Ч������������!"));
		return false;
	}

	return true;
}

void  PluginTypeEditWindow::on_cancelpushButton_clicked(){

	reject();
}

void  PluginTypeEditWindow::on_savepushButton_clicked(){

	if (!canSave()){
		return;
	}

	objtype_->animation_ = sprite_;
	objtype_->staticImage_ = image_;
	objtype_->setDescription(ui->namelineEdit->text());
	if (sprite_){
		objtype_->setParameter(UDQ_T("Animation"), QString::number(sprite_->hashKey()), objtype_->prototype_->objectName());
		objtype_->setParameter(UDQ_T("StaticImage"), UDQ_T("false"), objtype_->prototype_->objectName());
	}
	
    if( image_ ){
		objtype_->setParameter(UDQ_T("Animation"), QString::number(image_->hashKey()), objtype_->prototype_->objectName());
		objtype_->setParameter(UDQ_T("StaticImage"), UDQ_T("true"), objtype_->prototype_->objectName());
	}

	QTreeWidgetItem* item = NULL;
	ResourceHash key = 0;

	// ���±���
	PluginVariable* var = NULL;
	objtype_->variables_.clear();
	for (int i = 0; i < varRoot_->childCount(); i++){
		item = varRoot_->child(i);
		var = objtype_->variables_.value(item->text(0), true);
		Q_ASSERT(var != NULL);

		var->name = item->text(0);
		var->datatype = item->text(1);
		var->value = item->text(2);
		var->desc = item->toolTip(0);
	}

	objtype_->attachments_.clear();
	// ��Ϊ
	objtype_->behaviors_.clear();
	for (int i = 0; i < behaviorRoot_->childCount(); i++){
		item = behaviorRoot_->child(i);
		objtype_->behaviors_.append(item->text(0));
		key = item->data(0, ITEM_HASHKEY).toUInt();

		DEFINE_RESOURCE_HASHKEY_VALID(PluginResourceObject, plugin, key);
		objtype_->attachments_.insert(item->text(0), plugin);
	}


	// ��Ч
	objtype_->effects_.clear();
	for (int i = 0; i < effectRoot_->childCount(); i++){
		item = effectRoot_->child(i);
		objtype_->effects_.append(item->text(0));
		key = item->data(0, ITEM_HASHKEY).toUInt();

		DEFINE_RESOURCE_HASHKEY_VALID(PluginResourceObject, plugin, key);
		objtype_->attachments_.insert(item->text(0), plugin);
	}

	objtype_->setFlags(URF_TagSave);
	accept();
}

void PluginTypeEditWindow::on_varpushButton_clicked(){
	// ���ʵ������
	QStringList vars;
	for (int i = 0; i < varRoot_->childCount(); i++){
		vars.append(varRoot_->child(i)->text(0));
	}

	AddInstanceVariableWindow w(vars);
	if (w.exec() == QDialog::Accepted){
		// �����б�
		QTreeWidgetItem* item = findPropertyItem(w.newvar_[0], varRoot_);
		if (item == NULL ){
			item = new QTreeWidgetItem(varRoot_, ITEM_ATTRIBUTE);
			item->setText(0, w.newvar_[0]);
		}

		item->setText(1, w.newvar_[1]);
		item->setText(2, w.newvar_[2]);
		item->setToolTip(0, w.newvar_[3]);

		ui->treeWidget->expandItem(varRoot_);
	}
}

QStringList PluginTypeEditWindow::attachmentNames(){

	QStringList names;
	names.append(objtype_->prototype_->objectName());

	for (int i = 0; i < behaviorRoot_->childCount(); i++){
		names.append(behaviorRoot_->child(i)->text(0));
	}

	for (int i = 0; i < effectRoot_->childCount(); i++){
		names.append(effectRoot_->child(i)->text(0));
	}

	return names;
}


void PluginTypeEditWindow::on_behaviorpushButton_clicked(){

	// �����Ϊ
	AddInstanceBehaviorWindow w(attachmentNames());
	if (w.exec() == QDialog::Accepted){
		// ������Ϊ�б�
		DEFINE_RESOURCE_HASHKEY_VALID(PluginResourceObject, plugin, w.hash_);
		Q_ASSERT(plugin != NULL);

		QTreeWidgetItem* item = new QTreeWidgetItem(behaviorRoot_, ITEM_BEHAVIOR);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		item->setText(0, w.newbehavior_);
		item->setText(1, plugin->title_);
		item->setData(0, ITEM_HASHKEY, plugin->hashKey());  // ������ϣֵ
		
		ui->treeWidget->expandItem(behaviorRoot_);
	}
}

void PluginTypeEditWindow::on_effectpushButton_clicked(){

	AddInstanceEffectWindow w(attachmentNames());
	if (w.exec() == QDialog::Accepted){
		// �����б�
		DEFINE_RESOURCE_HASHKEY_VALID(PluginResourceObject, plugin, w.hash_);
		Q_ASSERT(plugin != NULL);

		QTreeWidgetItem* item = new QTreeWidgetItem(effectRoot_, ITEM_EFFECT);
		item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable);
		item->setText(0, w.neweffect_);
		item->setText(1, plugin->title_);
		item->setData(0, ITEM_HASHKEY, plugin->hashKey());  // ������ϣֵ

		ui->treeWidget->expandItem(effectRoot_);
	}
}

void PluginTypeEditWindow::on_deltoolButton_clicked(){
	QTreeWidgetItem* item = ui->treeWidget->currentItem();
	if (item == NULL || item->type() == ITEM_CATEGORY){
		return;
	}

	if (QMessageBox::question(this, windowTitle(), UDQ_TR("ȷ��ɾ�����"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No){
		return;
	}

	Q_ASSERT(item->parent() != NULL);
	item->parent()->removeChild(item);

}

void PluginTypeEditWindow::on_uptoolButton_clicked(){
	QTreeWidgetItem* item = ui->treeWidget->currentItem();
	if (item == NULL || (item->type() != ITEM_BEHAVIOR && item->type() != ITEM_EFFECT)){
		return;
	}
	Q_ASSERT(item->parent() != NULL);
	QTreeWidgetItem* parentItem = item->parent();

	int index = parentItem->indexOfChild(item);
	if (index > 0){
		parentItem->takeChild(index);
		parentItem->insertChild(index - 1, item);
	}
}

void PluginTypeEditWindow::on_downtoolButton_clicked(){
	QTreeWidgetItem* item = ui->treeWidget->currentItem();
	if (item == NULL || (item->type() != ITEM_BEHAVIOR && item->type() != ITEM_EFFECT)){
		return;
	}
	Q_ASSERT(item->parent() != NULL);
	QTreeWidgetItem* parentItem = item->parent();

	int index = parentItem->indexOfChild(item);
	if (index < parentItem->childCount() - 1){
		parentItem->takeChild(index);
		parentItem->insertChild(index + 1, item);
	}
}

void PluginTypeEditWindow::on_animpushButton_clicked(){
	ResourceHash oldKey = 0;
	if (objtype_->animation_){
		oldKey = objtype_->animation_->hashKey();
	}
	SelectSpriteWindow w(oldKey);
	if (w.exec() == QDialog::Accepted){
		DEFINE_RESOURCE_HASHKEY_VALID(SpriteResourceObject, sprite, w.spriteH_);
		sprite_ = sprite;
		image_ = NULL;
		updateImage();
	}
}
