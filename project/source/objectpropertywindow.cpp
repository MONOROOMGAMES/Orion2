#include "objectpropertywindow.h"
#include "ui_objectpropertywindow.h"
#include "tooldefine.h"

#include <QColorDialog>

#include <QtBoolPropertyManager>
#include <QtIntPropertyManager>
#include <QtStringPropertyManager>
#include <QtSizePropertyManager>
#include <QtRectPropertyManager>
#include <QtEnumPropertyManager>
#include <QtGroupPropertyManager>

#include <QtLineEditFactory>
#include <QtDoubleSpinBoxFactory>
#include "commondefine.h"
#include "framemanager.h"
#include "mainmanagerframe.h"
#include "leveldefine.h"
#include "levelscene.h"
#include "editorundocommand.h"

ObjectPropertyWindow::ObjectPropertyWindow(QWidget *parent) :
QWidget(parent), currentItem_(NULL), currentSceneItem_(NULL), isUpdating_(false),
    ui(new Ui::ObjectPropertyWindow)
{
    ui->setupUi(this);
	// ��ʼ��
	init();

}

ObjectPropertyWindow::~ObjectPropertyWindow()
{
    delete ui;
}


void	ObjectPropertyWindow::resizeEvent(QResizeEvent * event){
	QSize newSize = event->size() - QSize(WIDGET_MARGIN * 2, WIDGET_MARGIN * 2);
	ui->widget->setGeometry(QRect(QPoint(WIDGET_MARGIN, WIDGET_MARGIN), newSize));

}

void ObjectPropertyWindow::init(){

	QtIntPropertyManager *intManager = new QtIntPropertyManager(this);
	managers_.insert(MT_INT, intManager);

	QtStringPropertyManager *stringManager = new QtStringPropertyManager(this);
	managers_.insert(MT_STRING, stringManager);

	QtSizePropertyManager *sizeManager = new QtSizePropertyManager(this);
	managers_.insert(MT_SIZE, sizeManager);

	QtRectPropertyManager *rectManager = new QtRectPropertyManager(this);
	managers_.insert(MT_RECT, rectManager);

	QtEnumPropertyManager *enumManager = new QtEnumPropertyManager(this);
	managers_.insert(MT_ENUM, enumManager);

	QtColorPropertyManager *colorManager = new QtColorPropertyManager(this);
	managers_.insert(MT_COLOR, colorManager);

	QtGroupPropertyManager *groupManager = new QtGroupPropertyManager(this);
	managers_.insert(MT_GROUP, groupManager);

	QtPointPropertyManager *pointManager = new QtPointPropertyManager(this);
	managers_.insert(MT_POINT, pointManager);

	QtDoublePropertyManager *doubleManager = new QtDoublePropertyManager(this);
	managers_.insert(MT_DOUBLE, doubleManager);

	QtDateTimePropertyManager *datetimeManager = new QtDateTimePropertyManager(this);
	managers_.insert(MT_DATETIME, datetimeManager);

	QtFontPropertyManager* fontManager = new QtFontPropertyManager(this);
	managers_.insert(MT_FONT, fontManager);

	connect(intManager, SIGNAL(valueChanged(QtProperty *, int)),this, SLOT(valueChanged(QtProperty *, int)));
	connect(colorManager, SIGNAL(valueChanged(QtProperty *, const QColor&)), this, SLOT(valueChanged(QtProperty *, const QColor&)));
	connect(stringManager, SIGNAL(valueChanged(QtProperty *, const QString&)), this, SLOT(valueChanged(QtProperty *, const QString &)));
	connect(sizeManager, SIGNAL(valueChanged(QtProperty *, const QSize &)), this, SLOT(valueChanged(QtProperty *, const QSize &)));
	connect(rectManager, SIGNAL(valueChanged(QtProperty *, const QRect &)), this, SLOT(valueChanged(QtProperty *, const QRect &)));
	connect(enumManager, SIGNAL(valueChanged(QtProperty *, int)),this, SLOT(valueChanged(QtProperty *, int)));
	connect(pointManager, SIGNAL(valueChanged(QtProperty *, const QPoint&)), this, SLOT(valueChanged(QtProperty *, const QPoint&)));
	connect(doubleManager, SIGNAL(valueChanged(QtProperty *, double)), this, SLOT(valueChanged(QtProperty *, double)));
	connect(datetimeManager, SIGNAL(valueChanged(QtProperty *, const QDateTime&)), this, SLOT(valueChanged(QtProperty *, const QDateTime&)));
	connect(fontManager, SIGNAL(valueChanged(QtProperty *, const QFont&)), this, SLOT(valueChanged(QtProperty *, const QFont&)));

	QtDoubleSpinBoxFactory *doubleSpinBoxFactory = new QtDoubleSpinBoxFactory(this);
    QtCheckBoxFactory *checkBoxFactory = new QtCheckBoxFactory(this);
	QtSpinBoxFactory *spinBoxFactory = new QtSpinBoxFactory(this);
	QtLineEditFactory *lineEditFactory = new QtLineEditFactory(this);
	QtEnumEditorFactory *comboBoxFactory = new QtEnumEditorFactory(this);
	QtColorEditorFactory *colorFactory = new QtColorEditorFactory(this);
	QtDateTimeEditFactory *datetimeFactory = new QtDateTimeEditFactory(this);
	QtFontEditorFactory *fontFactory = new QtFontEditorFactory(this);


	propertyEditor_ = new QtTreePropertyBrowser(ui->scrollArea);
	propertyEditor_->setFactoryForManager(intManager, spinBoxFactory);
	propertyEditor_->setFactoryForManager(doubleManager, doubleSpinBoxFactory);
	propertyEditor_->setFactoryForManager(stringManager, lineEditFactory);
	propertyEditor_->setFactoryForManager(sizeManager->subIntPropertyManager(), spinBoxFactory);
	propertyEditor_->setFactoryForManager(rectManager->subIntPropertyManager(), spinBoxFactory);
	propertyEditor_->setFactoryForManager(pointManager->subIntPropertyManager(), spinBoxFactory);
	propertyEditor_->setFactoryForManager(enumManager, comboBoxFactory);
	propertyEditor_->setFactoryForManager(colorManager, colorFactory);
	propertyEditor_->setFactoryForManager(datetimeManager, datetimeFactory);
	propertyEditor_->setFactoryForManager(fontManager, fontFactory);

	ui->scrollArea->setWidget(propertyEditor_);

	connect(propertyEditor_, SIGNAL(currentItemChanged(QtBrowserItem *)), this, SLOT(propertySelected(QtBrowserItem *)));


	propertyEditor_->setAlternatingRowColors(false);
}

void ObjectPropertyWindow::valueChanged(QtProperty *property, int value)
{
	QtEnumPropertyManager* enumManager = (QtEnumPropertyManager*)managers_[MT_ENUM];

	if (!subproperties_.contains(property) || isUpdating_)
		return;

	QString propId = property->whatsThis();

	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// ��������ֵ�仯
	if (currentItem_){
		if (currentItem_->type() == ITEM_LAYER){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, id);

			if (propId == UDQ_T("InitVisible")){
				layer->initVisible_ = (value == 0) ;
			}
			else if (propId == UDQ_T("Transparent")){
				layer->transparent_ = (value == 1);
			}
			else if (propId == UDQ_T("Opacity")){
				layer->opacity_ = value;
			}
			else if (propId == UDQ_T("OwnTexture")){
				layer->ownTexture_ = (value == 1);
			}
			else if (propId == UDQ_T("CellRender")){
				layer->useRenderCell_ = (value == 1);
			}
			else if (propId == UDQ_T("ScaleRate")){
				layer->scale_ = value;
			}
			else if (propId == UDQ_T("Visible")){
				layer->isVisible_ = (value == 1);
			}
			else if (propId == UDQ_T("Lock")){
				layer->isLocking_ = (value == 1);
			}
			else if (propId == UDQ_T("Global")){
				layer->isGlobal_ = (value == 1);
			}
			// ����ͼ��
			frame->updateLayer(id);
		}
		else if (currentItem_->type() == ITEM_LEVEL){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(LevelResourceObject, level, id);

			if (propId == UDQ_T("UnBound")){
				level->unboundScroll_ = (value == 1);
			}
			else if (propId == UDQ_T("Script")){
				// ���������ҵ��ű�����
				QStringList items = enumManager->enumNames(property);
				Q_ASSERT(value < items.size());
				if (value == 0){
					level->blueprint_ = 0;
				}
				else{
					level->blueprint_ = scriptToId_.value(items.at(value));
				}
			}

		}
		else if (currentItem_->type() == ITEM_OBJECTTYPE){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, objType, id);

			if (propId == UDQ_T("Global")){
				objType->global_ = (value == 1);
			}
			else if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateObjectTypeProperty(objType, varID, QString::number(value));
				frame->refreshRegions(objType);
			}
		}
		else if (currentItem_->type() == ITEM_FAMILY){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(FamilyResourceObject, family, id);

			if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateFamilyProperty(family, varID, QString::number(value));
			}
		}
	}
	else if (currentSceneItem_){
		// ��������
		EditorRegion* region = (EditorRegion*)currentSceneItem_;
		if (region){
			if (propId == UDQ_T("Global")){
				region->typePrototype_->global_ = (value == 1);
			}
			else if (propId == UDQ_T("Layer")){
				// ����ͼ��
				QStringList items = enumManager->enumNames(property);
				Q_ASSERT(value < items.size());

				QUndoCommand* command = new MoveInstanceCommand(currentSceneItem_, items.at(value),(LevelScene*)frame->sceneView()->scene());
				frame->undoStack()->push(command);
			}
			else if (propId == UDQ_T("Angle")){
				// ����ͼ��
				region->setRotation(value);
				region->updateAnchor();
			}
			else if (propId == UDQ_T("Opacity")){
				// ����ͼ��
				region->setAlpha(value);
			}
			else if (propId == UDQ_T("Blend")){
				// ����ͼ��
				region->blend_ = value;
			}
			else if (propId == UDQ_T("Fallback")){
				// ����ͼ��
				region->fallback_ = value;
			}
			else if (propId.startsWith(UDQ_T("@"))){  // ��������
					QString varID = propId.mid(1);
					updateRegionProperty(region, varID, QString::number(value));
					region->postinit(false);
			}	
			region->update();
		}
	}

}


void ObjectPropertyWindow::valueChanged(QtProperty *property, double value)
{
	QtDoublePropertyManager* floatManager = (QtDoublePropertyManager*)managers_[MT_DOUBLE];

	if (!subproperties_.contains(property) || isUpdating_)
		return;

	QString propId = property->whatsThis();

	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// ��������ֵ�仯
	if (currentItem_){
		if (currentItem_->type() == ITEM_OBJECTTYPE){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, objType, id);

			if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateObjectTypeProperty(objType, varID, QString::number(value));
				frame->refreshRegions(objType);
			}
		}
		else if (currentItem_->type() == ITEM_FAMILY){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(FamilyResourceObject, family, id);
	
			if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateFamilyProperty(family, varID, QString::number(value));
			}
		}
	}
	else if (currentSceneItem_){
		EditorRegion* region = (EditorRegion*)currentSceneItem_;
		if (region){
			if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateRegionProperty(region, varID, QString::number(value));
				region->postinit(false);
			}
			region->update();
		}
	}

}

void ObjectPropertyWindow::valueChanged(QtProperty *property, const QDateTime& value)
{
	QtDateTimePropertyManager* datetimeManager = (QtDateTimePropertyManager*)managers_[MT_DATETIME];

	if (!subproperties_.contains(property) || isUpdating_)
		return;
	QString propId = property->whatsThis();

	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// ��������ֵ�仯


}


void ObjectPropertyWindow::valueChanged(QtProperty *property, const QFont &value){
	QtFontPropertyManager* fontManager = (QtFontPropertyManager*)managers_[MT_FONT];

	if (!subproperties_.contains(property) || isUpdating_)
		return;
	QString propId = property->whatsThis();

	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// ��������ֵ�仯
	if (currentItem_){
		if (currentItem_->type() == ITEM_OBJECTTYPE){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, objType, id);

			if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateObjectTypeProperty(objType, varID, value.toString());
				frame->refreshRegions(objType);
			}
		}
		else if (currentItem_->type() == ITEM_FAMILY){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(FamilyResourceObject, family, id);

			if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateFamilyProperty(family, varID, value.toString());
			}
		}
	}
	else if (currentSceneItem_){

		EditorRegion* region = (EditorRegion*)currentSceneItem_;
		if (region){
			if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateRegionProperty(region, varID, value.toString());
				region->postinit(false);
			}
			region->update();
		}
	}
}

void ObjectPropertyWindow::valueChanged(QtProperty *property, const QString &value)
{
	QtStringPropertyManager* stringManager = (QtStringPropertyManager*)managers_[MT_STRING];

	if (!subproperties_.contains(property) || isUpdating_)
		return;

	QString propId = property->whatsThis();

	// ��������ֵ�仯
	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// ��������ֵ�仯
	if (currentItem_){
		if (currentItem_->type() == ITEM_LAYER){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, id);

			if (layer->description() == value)
				return;

			if (propId == UDQ_T("Name")){
				Q_ASSERT(layer->level_ != NULL);
				// �������
				LayerResourceObject* find = layer->level_->findLayer(value);
				if (find == NULL){
					layer->setDescription(value);
					currentItem_->setText(0, value);
				}
				else if (find != layer){
					QMessageBox::warning(this, windowTitle(), UDQ_TR("%1��������").arg(value));
					stringManager->setValue(property, layer->description());
				}
			}

			// ����ͼ��
			frame->updateLayer(id);
		}
		else if (currentItem_->type() == ITEM_LEVEL){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(LevelResourceObject, level, id);

			if (level->description() == value)
				return;

			if (propId == UDQ_T("Name")){
				// �������
				LevelResourceObject* find = ((ProjectResourceObject*)level->outer())->getLevel(value);
				if (find == NULL){
					level->setDescription(value);
					currentItem_->setText(0, value);
				}
				else if (find != level){
					QMessageBox::warning(this, windowTitle(), UDQ_TR("%1��������").arg(value));
					stringManager->setValue(property, level->description());
					frame->refreshCurrentLayer();
				}
			}

		}
		else if (currentItem_->type() == ITEM_PROJECT){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(ProjectResourceObject, proj, id);

			if (propId == UDQ_T("ID")){
				proj->domainId_ = value;
			}
			else if (propId == UDQ_T("Author")){
				proj->author_ = value;
			}
			else if (propId == UDQ_T("Email")){
				proj->email_ = value;
			}
			else if (propId == UDQ_T("Website")){
				proj->website_ = value;
			}
			else if (propId == UDQ_T("Version")){
				proj->pubVersion_ = value;
			}

		}
		else if (currentItem_->type() == ITEM_OBJECTTYPE){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, objType, id);

			if (propId == UDQ_T("ObjectType")){
				if (objType->description() == value)
					return;

				// �������
				ObjectTypeResourceObject* find = ((ProjectResourceObject*)objType->outer())->getObjectType(value);
				if (find == NULL){
					objType->setDescription(value);
					currentItem_->setText(0, value);
				}
				else if (find != objType){
					QMessageBox::warning(this, windowTitle(), UDQ_TR("%1��������").arg(value));
					stringManager->setValue(property, objType->description());
				}
			}
			else if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1); 
				updateObjectTypeProperty(objType, varID, value);
				frame->refreshRegions(objType);
			}
		}
		else if (currentItem_->type() == ITEM_FAMILY){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(FamilyResourceObject, family, id);

			if (propId == UDQ_T("Name")){
				if (family->description() == value)
					return;

				// �������
				FamilyResourceObject* find = ((ProjectResourceObject*)family->outer())->getFamily(value);
				if (find == NULL){
					family->setDescription(value);
					currentItem_->setText(0, value);
				}
				else if (find != family){
					QMessageBox::warning(this, windowTitle(), UDQ_TR("%1��������").arg(value));
					stringManager->setValue(property, family->description());
				}
			}
			else if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateFamilyProperty(family, varID, value);
			}
		}
	}
	else if (currentSceneItem_){
		// ��������
		EditorRegion* region = (EditorRegion*)currentSceneItem_;
		if (region){
		
			if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateRegionProperty(region, varID, value);
				region->postinit(false);
			}
			
			region->update();
		}

	}
}

void ObjectPropertyWindow::valueChanged(QtProperty *property, const QColor &value)
{
	if (!subproperties_.contains(property) || isUpdating_)
		return;

	QString propId = property->whatsThis();

	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// ��������ֵ�仯
	if (currentItem_){
		if (currentItem_->type() == ITEM_LAYER){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, id);

			if (propId == UDQ_T("BackgroundColor")){
				layer->bgColor_ = value;
			}

			// ����ͼ��
			frame->updateLayer(id);
		}
		if (currentItem_->type() == ITEM_OBJECTTYPE){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, objType, id);

			if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateObjectTypeProperty(objType, varID, value.name());
				frame->refreshRegions(objType);
			}
		}
		else if (currentItem_->type() == ITEM_FAMILY){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(FamilyResourceObject, family, id);

			if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateFamilyProperty(family, varID, value.name());
			}
		}
	}
	else if (currentSceneItem_){
		// ��������
		EditorRegion* region = (EditorRegion*)currentSceneItem_;
		if (region){
			if (propId.startsWith(UDQ_T("@"))){  // ��������
				QString varID = propId.mid(1);
				updateRegionProperty(region, varID, value.name());
				region->postinit(false);
			}
			region->update();
		}
	}

}

void ObjectPropertyWindow::valueChanged(QtProperty *property, const QPoint &value)
{
	if (!subproperties_.contains(property) || isUpdating_)
		return;

	QString propId = property->whatsThis();

	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// ��������ֵ�仯
	if (currentItem_){
		if (currentItem_->type() == ITEM_LAYER){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, id);

			if (propId == UDQ_T("Parallax")){
				layer->parallax_ = value;
			}

			// ����ͼ��
			frame->updateLayer(id);
		}
	}
	else if (currentSceneItem_){
		// ��������
		EditorRegion* region = (EditorRegion*)currentSceneItem_;
		if (region){
			if (propId == UDQ_T("Position")){
				region->setPos(value);
			}

			region->update();

		}
	}
}

void ObjectPropertyWindow::valueChanged(QtProperty *property, const QRect &value)
{
	if (!subproperties_.contains(property) || isUpdating_)
		return;

	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// ��������ֵ�仯

}

void ObjectPropertyWindow::valueChanged(QtProperty *property, const QSize &value)
{
	if (!subproperties_.contains(property) || isUpdating_)
		return;

	QString propId = property->whatsThis();

	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// ��������ֵ�仯
	if (currentItem_){
		if (currentItem_->type() == ITEM_LEVEL){
			ResourceHash id = currentItem_->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(LevelResourceObject, level, id);

			if (propId == UDQ_T("Size")){
				level->size_ = value;
			}
			if (propId == UDQ_T("Margins")){
				level->margin_ = value;
			}

			frame->refreshLayers();
		}
	}
	else if (currentSceneItem_){
		// ��������
		EditorRegion* region = (EditorRegion*)currentSceneItem_;
		if (region){
			if (propId == UDQ_T("Size")){
				// ����ͼ��
				region->resize(value);
				region->updateAnchor();
			}
			region->update();
		}	
	}
}

void ObjectPropertyWindow::processUndo(QTreeWidgetItem* obj){

	QTreeWidgetItem* oldItem = currentItem_;
	EditorItem* oldSceneItem = currentSceneItem_;

	currentItem_ = obj;
	currentSceneItem_ = NULL;

	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// ����undo
	if (oldValues_.size() > 0){

		if (oldItem && oldItem->type() == ITEM_LAYER){  // ֻ����ͼ��	

			ResourceHash id = oldItem->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, id);

			QByteArray newData = layer->saveProperty();
			// hash�Ƚ�
			QByteArray newHash = QCryptographicHash::hash(newData, QCryptographicHash::Md5);
			QByteArray oldHash = QCryptographicHash::hash(oldValues_, QCryptographicHash::Md5);
			if (newHash != oldHash){  // ���ֵ�仯�ˣ������undo
				ModifyLayerCommand* command = new ModifyLayerCommand(layer, oldValues_, false);
				frame->undoStack()->push(command);
				command->setEnabled(true);
			}
		}
		else if (oldSceneItem){
			frame->entityModified(oldSceneItem, oldValues_);
		}
	}
}
void ObjectPropertyWindow::processUndo(EditorItem* obj){
	QTreeWidgetItem* oldItem = currentItem_;
	EditorItem* oldSceneItem = currentSceneItem_;

	currentSceneItem_ = obj;
	currentItem_ = NULL;

	MainManagerFrame* frame = (MainManagerFrame*)gFrameManager->getWindow(FrameManager::WID_LEVEL);
	Q_ASSERT(frame != NULL);

	// ����undo
	if (oldValues_.size() > 0){

		if (oldItem && oldItem->type() == ITEM_LAYER){  // ֻ����ͼ��	

			ResourceHash id = oldItem->data(0, ITEM_HASHKEY).toUInt();
			DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, id);

			QByteArray newData = layer->saveProperty();
			// hash�Ƚ�
			QByteArray newHash = QCryptographicHash::hash(newData, QCryptographicHash::Md5);
			QByteArray oldHash = QCryptographicHash::hash(oldValues_, QCryptographicHash::Md5);
			if (newHash != oldHash){  // ���ֵ�仯�ˣ������undo
				ModifyLayerCommand* command = new ModifyLayerCommand(layer, oldValues_, false);
				frame->undoStack()->push(command);
				command->setEnabled(true);
			}
		}
		else if (oldSceneItem){
			frame->entityModified(oldSceneItem, oldValues_);
		}

		oldValues_.clear();
	}
}

// ��������
void ObjectPropertyWindow::loadProperty(QTreeWidgetItem* obj){
	updateExpandState();
	processUndo(obj);
	clear();

	if (obj == NULL)
		return;

	if (obj->type() == ITEM_LAYER){
		ResourceHash id = obj->data(0, ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY_VALID(LayerResourceObject, layer, id);
		oldValues_ = layer->saveProperty();
		prepareLayerProperty(layer);
	}
	else if (obj->type() == ITEM_LEVEL){
		ResourceHash id = obj->data(0, ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY_VALID(LevelResourceObject, level, id);
		prepareLevelProperty(level);
	}
	else if (obj->type() == ITEM_PROJECT){
		ResourceHash id = obj->data(0, ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY_VALID(ProjectResourceObject, proj, id);
		prepareProjectProperty(proj);
	}
	else if (obj->type() == ITEM_OBJECTTYPE){
		ResourceHash id = obj->data(0, ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY_VALID(ObjectTypeResourceObject, objType, id);
		prepareObjectTypeProperty(objType);
	}
	else if (obj->type() == ITEM_FAMILY){
		ResourceHash id = obj->data(0, ITEM_HASHKEY).toUInt();
		DEFINE_RESOURCE_HASHKEY_VALID(FamilyResourceObject, family, id);
		prepareFamilyProperty(family);
	}
}

void ObjectPropertyWindow::loadProperty(EditorItem* obj){

	updateExpandState();
	processUndo(obj);
	clear();


	if (obj == NULL)
		return;


	EditorRegion* region = (EditorRegion*)obj;
	if ( region ){
		oldValues_ = region->encode();
		prepareRegionProperty(region);
	}

}

void ObjectPropertyWindow::clear(){

	QMap<QtProperty *, ResourceHash>::ConstIterator itProp = propertyToId_.constBegin();
	while (itProp != propertyToId_.constEnd()) {
		delete itProp.key();
		itProp++;
	}
	propertyToId_.clear();
	subproperties_.clear();

}

void ObjectPropertyWindow::clearUndo(){
	currentItem_ = NULL;
	currentSceneItem_ = NULL;
	clear();
}


void ObjectPropertyWindow::updateExpandState()
{
	QList<QtBrowserItem *> list = propertyEditor_->topLevelItems();
	QListIterator<QtBrowserItem *> it(list);
	while (it.hasNext()) {
		QtBrowserItem *item = it.next();
		QtProperty *prop = item->property();
		idToExpanded_[propertyToId_[prop]] = propertyEditor_->isExpanded(item);
	}
}


void ObjectPropertyWindow::addProperty(QtProperty *property, ResourceHash id, QtProperty *parent)
{
	if (parent){
		subproperties_.append(property);
		 parent->addSubProperty(property);
	}
	else{
		propertyToId_[property] = id;
		QtBrowserItem *item = propertyEditor_->addProperty(property);
		if (idToExpanded_.contains(id)){
			propertyEditor_->setExpanded(item, idToExpanded_[id]);
		}
	}

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

void ObjectPropertyWindow::prepareLayerProperty(LayerResourceObject* layer){
	QtProperty *property = NULL;
	QtGroupPropertyManager* groupManager = (QtGroupPropertyManager*)managers_[MT_GROUP];
	QtStringPropertyManager* stringManager = (QtStringPropertyManager*)managers_[MT_STRING];
	QtColorPropertyManager* colorManager = (QtColorPropertyManager*)managers_[MT_COLOR];
	QtIntPropertyManager* intManager = (QtIntPropertyManager*)managers_[MT_INT];
	QtEnumPropertyManager* enumManager = (QtEnumPropertyManager*)managers_[MT_ENUM];
	QtDoublePropertyManager* floatManager = (QtDoublePropertyManager*)managers_[MT_DOUBLE];
	QtPointPropertyManager* pointManager = (QtPointPropertyManager*)managers_[MT_POINT];

	QStringList boolEnums;
	boolEnums << UDQ_TR("��") << UDQ_TR("��");

	// ͼ��
	property = groupManager->addProperty(UDQ_TR("ͼ������"));
	addProperty(property, layer->hashKey());

	// ���������
	QtProperty *name = stringManager->addProperty(UDQ_TR("����"));
	name->setToolTip(UDQ_TR("ͼ������"));
	name->setWhatsThis(UDQ_T("Name"));
	stringManager->setValue(name, layer->description());
	addProperty(name, 0, property);

	QtProperty *initvisible = enumManager->addProperty(UDQ_TR("��ʼ�ɼ�"));
	initvisible->setToolTip(UDQ_TR("�ڳ�����ʼʱͼ���Ƿ�ɼ�"));
	initvisible->setWhatsThis(UDQ_T("InitVisible"));
	enumManager->setEnumNames(initvisible, QStringList() << UDQ_TR("�ɼ�") << UDQ_TR("���ɼ�"));
	enumManager->setValue(initvisible, layer->initVisible_ ? 0 : 1);
	addProperty(initvisible, 0, property);

	QtProperty *bgcolor = colorManager->addProperty(UDQ_TR("������ɫ"));
	bgcolor->setToolTip(UDQ_TR("ͼ��ı��������ɫ"));
	bgcolor->setWhatsThis(UDQ_T("BackgroundColor"));
	colorManager->setValue(bgcolor, layer->bgColor_);
	addProperty(bgcolor, 0, property);

	QtProperty *transparent = enumManager->addProperty(UDQ_TR("����͸��"));
	transparent->setToolTip(UDQ_TR("ͼ��ı����Ƿ���ȫ͸��"));
	transparent->setWhatsThis(UDQ_T("Transparent"));
	enumManager->setEnumNames(transparent, boolEnums);
	enumManager->setValue(transparent, layer->transparent_ ? 1 : 0);
	addProperty(transparent, 0, property);

	QtProperty *opacity = intManager->addProperty(UDQ_TR("͸����"));
	opacity->setToolTip(UDQ_TR("ͼ���͸����(ȡֵ0-100)"));
	opacity->setWhatsThis(UDQ_T("Opacity"));
	intManager->setRange(opacity, 0, 100);
	intManager->setValue(opacity, layer->opacity_);
	addProperty(opacity, 0, property);

	QtProperty *owntexture = enumManager->addProperty(UDQ_TR("������"));
	owntexture->setToolTip(UDQ_TR("ͼ�㱳�������Ƿ��Ȼ��Ƶ�������"));
	owntexture->setWhatsThis(UDQ_T("OwnTexture"));
	enumManager->setEnumNames(owntexture, QStringList() << UDQ_TR("�ر�") << UDQ_TR("����"));
	enumManager->setValue(owntexture, layer->ownTexture_ ? 1 : 0);
	addProperty(owntexture, 0, property);

	QtProperty *cellrender = enumManager->addProperty(UDQ_TR("��Ⱦ����"));
	cellrender->setToolTip(UDQ_TR("ͼ�����Ⱦ�Ƿ������Ⱦ���������ڰ�����������Ĵ��ͳ�����"));
	cellrender->setWhatsThis(UDQ_T("CellRender"));
	enumManager->setEnumNames(cellrender, QStringList() << UDQ_TR("��ʹ��") << UDQ_TR("ʹ��"));
	enumManager->setValue(cellrender, layer->useRenderCell_ ? 1 : 0);
	addProperty(cellrender, 0, property);

	QtProperty *scalerate = intManager->addProperty(UDQ_TR("���ű仯�ٶ�"));
	scalerate->setToolTip(UDQ_TR("ͼ�������ʱ�仯�ٶ�"));
	scalerate->setWhatsThis(UDQ_T("ScaleRate"));
	intManager->setRange(scalerate, 0, 100);
	intManager->setValue(scalerate, layer->scale_);
	addProperty(scalerate, 0, property);

	QtProperty *parallax = pointManager->addProperty(UDQ_TR("��Թ����ٶ�"));
	parallax->setToolTip(UDQ_TR("ͼ�������ڳ����Ĺ����ٶ�"));
	parallax->setWhatsThis(UDQ_T("Parallax"));
	pointManager->setValue(parallax, layer->parallax_);
	addProperty(parallax, 0, property);

	// �༭��
	property = groupManager->addProperty(UDQ_TR("�༭������"));
	addProperty(property, hashString(UDQ_T("EditorProperty")));

	// 
	QtProperty *visible = enumManager->addProperty(UDQ_TR("�Ƿ�ɼ�"));
	visible->setToolTip(UDQ_TR("ͼ���ڱ༭�����Ƿ�ɼ�"));
	visible->setWhatsThis(UDQ_T("Visible"));
	enumManager->setEnumNames(visible, boolEnums);
	enumManager->setValue(visible, layer->isVisible_ ? 1 : 0);
	addProperty(visible, 0, property);
	// 
	QtProperty *lock = enumManager->addProperty(UDQ_TR("�Ƿ�����"));
	lock->setToolTip(UDQ_TR("ͼ���ڱ༭�����Ƿ񲻿ɱ༭"));
	lock->setWhatsThis(UDQ_T("Lock"));
	enumManager->setEnumNames(lock, boolEnums);
	enumManager->setValue(lock, layer->isLocking_ ? 1 : 0);
	addProperty(lock, 0, property);
	// 
	QtProperty *global = enumManager->addProperty(UDQ_TR("�Ƿ�̬"));
	global->setToolTip(UDQ_TR("��ͼ��ɱ�����������ã�����ͬ��ͼ��"));
	global->setWhatsThis(UDQ_T("Global"));
	enumManager->setEnumNames(global, boolEnums);
	enumManager->setValue(global, layer->isGlobal_ ? 1 : 0);
	addProperty(global, 0, property);
}


void ObjectPropertyWindow::prepareLevelProperty(LevelResourceObject* level){
	QtProperty *property = NULL;
	QtGroupPropertyManager* groupManager = (QtGroupPropertyManager*)managers_[MT_GROUP];
	QtStringPropertyManager* stringManager = (QtStringPropertyManager*)managers_[MT_STRING];
	QtEnumPropertyManager* enumManager = (QtEnumPropertyManager*)managers_[MT_ENUM];
	QtSizePropertyManager* sizeManager = (QtSizePropertyManager*)managers_[MT_SIZE];

	// ͼ��
	property = groupManager->addProperty(UDQ_TR("��������"));
	addProperty(property, level->hashKey());

	// ���������
	QtProperty *name = stringManager->addProperty(UDQ_TR("����"));
	name->setToolTip(UDQ_TR("��������"));
	name->setWhatsThis(UDQ_T("Name"));
	stringManager->setValue(name, level->description());
	addProperty(name, 0, property);

	QtProperty *size = sizeManager->addProperty(UDQ_TR("��С"));
	size->setToolTip(UDQ_TR("�����Ĵ�С����λΪ���أ�"));
	size->setWhatsThis(UDQ_T("Size"));
	sizeManager->setMinimum(size, QSize(1, 1));
	sizeManager->setValue(size, level->size_);
	addProperty(size, 0, property);

	QtProperty *margins = sizeManager->addProperty(UDQ_TR("�߿�"));
	margins->setToolTip(UDQ_TR("�����ı߿��С����λΪ���أ�"));
	margins->setWhatsThis(UDQ_T("Margins"));
	sizeManager->setMinimum(margins, QSize(0, 0));
	sizeManager->setValue(margins, level->margin_);
	addProperty(margins, 0, property);

	QtProperty *unbound = enumManager->addProperty(UDQ_TR("�Ƿ����޹���"));
	unbound->setToolTip(UDQ_TR("�����ڳ��������Ƶ��ƶ�"));
	unbound->setWhatsThis(UDQ_T("UnBound"));
	enumManager->setEnumNames(unbound, QStringList() << UDQ_TR("������") << UDQ_TR("����"));
	enumManager->setValue(unbound, level->unboundScroll_ ? 1 : 0);
	addProperty(unbound, 0, property);

	// ��ȡ��ǰ��Ŀ�����нű�
	WorkspaceResourceObject* ws = ResourceObject::currentWorkspace();
	Q_ASSERT(ws != NULL);
	// ��ȡ��ǰ������
	ProjectResourceObject* proj = ws->currentProject();
	Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));
	QStringList items;
	items.append(UDQ_TR("��"));
	QList<ResourceHash> itemHashs;
	itemHashs.append(0);
	foreach(BlueprintResourceObject* blueprint, proj->blueprints_){
		if (blueprint->flags() & URF_TagGarbage)
			continue;

		items.append(blueprint->description());
		itemHashs.append(blueprint->hashKey());
		scriptToId_.insert(blueprint->description(), blueprint->hashKey());
	}
	QtProperty *script = enumManager->addProperty(UDQ_TR("�¼��ű�"));
	script->setToolTip(UDQ_TR("����ʹ�õ��¼��ű���"));
	script->setWhatsThis(UDQ_T("Script"));
	enumManager->setEnumNames(script, items);
	enumManager->setValue(script, itemHashs.indexOf(level->blueprint_));
	addProperty(script, 0, property);

}

void ObjectPropertyWindow::prepareProjectProperty(ProjectResourceObject* proj){
	QtProperty *property = NULL;
	QtGroupPropertyManager* groupManager = (QtGroupPropertyManager*)managers_[MT_GROUP];
	QtStringPropertyManager* stringManager = (QtStringPropertyManager*)managers_[MT_STRING];
	QtColorPropertyManager* colorManager = (QtColorPropertyManager*)managers_[MT_COLOR];
	QtIntPropertyManager* intManager = (QtIntPropertyManager*)managers_[MT_INT];
	QtEnumPropertyManager* enumManager = (QtEnumPropertyManager*)managers_[MT_ENUM];
	QtDoublePropertyManager* floatManager = (QtDoublePropertyManager*)managers_[MT_DOUBLE];
	QtPointPropertyManager* pointManager = (QtPointPropertyManager*)managers_[MT_POINT];
	QtDateTimePropertyManager* datetimeManager = (QtDateTimePropertyManager*)managers_[MT_DATETIME];

	// ͼ��
	property = groupManager->addProperty(UDQ_TR("��Ŀ����"));
	addProperty(property, proj->hashKey());

	// ���������
	QtProperty *name = stringManager->addProperty(UDQ_TR("����"));
	name->setToolTip(UDQ_TR("��Ŀ����"));
	name->setWhatsThis(UDQ_T("Name"));
	name->setEnabled(false);
	stringManager->setValue(name, proj->objectName());
	addProperty(name, 0, property);

	QtProperty *version = stringManager->addProperty(UDQ_TR("�汾"));
	version->setToolTip(UDQ_TR("��Ŀ�İ汾��"));
	version->setWhatsThis(UDQ_T("Version"));
	stringManager->setValue(version, proj->pubVersion_);
	addProperty(version, 0, property);

	QtProperty *createDate = datetimeManager->addProperty(UDQ_TR("��������"));
	createDate->setToolTip(UDQ_TR("��Ŀ�Ĵ�������"));
	createDate->setWhatsThis(UDQ_T("CreateDate"));
	createDate->setEnabled(false);
	datetimeManager->setValue(createDate, proj->createDate_);
	addProperty(createDate, 0, property);

	QtProperty *author = stringManager->addProperty(UDQ_TR("����"));
	author->setToolTip(UDQ_TR("��Ŀ������"));
	author->setWhatsThis(UDQ_T("Author"));
	stringManager->setValue(createDate, proj->author_);
	addProperty(author, 0, property);

	QtProperty *domain = stringManager->addProperty(UDQ_TR("ID"));
	domain->setToolTip(UDQ_TR("��Ŀ��ID"));
	domain->setWhatsThis(UDQ_T("ID"));
	stringManager->setValue(domain, proj->domainId_);
	addProperty(domain, 0, property);

	QtProperty *email = stringManager->addProperty(UDQ_TR("����"));
	email->setToolTip(UDQ_TR("��Ŀ����ϵ����"));
	email->setWhatsThis(UDQ_T("Email"));
	stringManager->setValue(email, proj->email_);
	addProperty(email, 0, property);

	QtProperty *website = stringManager->addProperty(UDQ_TR("������ַ"));
	website->setToolTip(UDQ_TR("��Ŀ�ķ�����ַ"));
	website->setWhatsThis(UDQ_T("Website"));
	stringManager->setValue(website, proj->website_);
	addProperty(website, 0, property);

}


void ObjectPropertyWindow::prepareRegionProperty(EditorRegion* region){
	QtProperty *property = NULL;
	QtGroupPropertyManager* groupManager = (QtGroupPropertyManager*)managers_[MT_GROUP];
	QtStringPropertyManager* stringManager = (QtStringPropertyManager*)managers_[MT_STRING];
	QtColorPropertyManager* colorManager = (QtColorPropertyManager*)managers_[MT_COLOR];
	QtIntPropertyManager* intManager = (QtIntPropertyManager*)managers_[MT_INT];
	QtEnumPropertyManager* enumManager = (QtEnumPropertyManager*)managers_[MT_ENUM];
	QtDoublePropertyManager* floatManager = (QtDoublePropertyManager*)managers_[MT_DOUBLE];
	QtPointPropertyManager* pointManager = (QtPointPropertyManager*)managers_[MT_POINT];
	QtDateTimePropertyManager* datetimeManager = (QtDateTimePropertyManager*)managers_[MT_DATETIME];
	QtSizePropertyManager* sizeManager = (QtSizePropertyManager*)managers_[MT_SIZE];

	QStringList boolEnums;
	boolEnums << UDQ_TR("��") << UDQ_TR("��");

	// ͼ��
	property = groupManager->addProperty(UDQ_TR("������������"));
	addProperty(property, region->iid());

	//////////////////////////////////////////////////////////////////////////
	// ���������
	QtProperty *name = stringManager->addProperty(UDQ_TR("����"));
	name->setToolTip(UDQ_TR("������������"));
	name->setWhatsThis(UDQ_T("ObjectType"));
	name->setEnabled(false);
	stringManager->setValue(name, region->typePrototype_->description());
	addProperty(name, 0, property);

	QtProperty *plugin = stringManager->addProperty(UDQ_TR("�������"));
	plugin->setToolTip(UDQ_TR("�������͵Ĳ������"));
	plugin->setWhatsThis(UDQ_T("Plugin"));
	plugin->setEnabled(false);
	stringManager->setValue(plugin, region->typePrototype_->prototype_->title_);
	addProperty(plugin, 0, property);

	QtProperty *global = enumManager->addProperty(UDQ_TR("�Ƿ�ȫ��"));
	global->setToolTip(UDQ_TR("���������Ƿ�Ϊȫ�ֶ���(ȫ�ֶ����ڳ��������󲻻ᱻɾ��)"));
	global->setWhatsThis(UDQ_T("Global"));
	enumManager->setEnumNames(global, boolEnums);
	enumManager->setValue(global, region->typePrototype_->global_ ? 1:0);
	addProperty(global, 0, property);

	//////////////////////////////////////////////////////////////////////////
	// ��������
	property = groupManager->addProperty(UDQ_TR("��������"));
	addProperty(property, hashString(UDQ_T("Common")));

	//  ͼ��
	// ��õ�ǰ������ͼ���б�
	LevelScene* scene = (LevelScene*)currentSceneItem_->scene();
	Q_ASSERT(scene != NULL);
	QStringList layerNames = scene->layerNames();

	EditorLayer* l = (EditorLayer*)region->parentItem();
	if (l){
		QtProperty *layer = enumManager->addProperty(UDQ_TR("ͼ��"));
		layer->setToolTip(UDQ_TR("����ʵ�����ڵ�ͼ��"));
		layer->setWhatsThis(UDQ_T("Layer"));
		enumManager->setEnumNames(layer, layerNames);
		enumManager->setValue(layer, layerNames.indexOf(l->layerName()));
		addProperty(layer, 0, property);
	}


	// λ��
	QtProperty *pos = pointManager->addProperty(UDQ_TR("λ��"));
	pos->setToolTip(UDQ_TR("����ʵ����͸���ȣ�0Ϊ��ȫ͸����100Ϊ��͸����"));
	pos->setWhatsThis(UDQ_T("Position"));
	pointManager->setValue(pos, region->pos().toPoint());
	addProperty(pos, 0, property);

	if (region->typePrototype_->prototype_->hasAttribute(EF_WORLD)){
		// �Ƕ�
		QtProperty *angle = intManager->addProperty(UDQ_TR("�Ƕ�"));
		angle->setToolTip(UDQ_TR("����ʵ������ת�Ƕȣ��ҷ�Ϊ0�ȣ�˳ʱ����ת��"));
		angle->setWhatsThis(UDQ_T("Angle"));
		intManager->setRange(angle, 0, 359);
		intManager->setValue(angle, qRound(region->rotation()));
		addProperty(angle, 0, property);

		// ͸����
		QtProperty *opacity = intManager->addProperty(UDQ_TR("͸����"));
		opacity->setToolTip(UDQ_TR("����ʵ����͸���ȣ�0Ϊ��ȫ͸����100Ϊ��͸����"));
		opacity->setWhatsThis(UDQ_T("Opacity"));
		intManager->setRange(opacity, 0, 100);
		intManager->setValue(opacity, qRound(region->opacity() * 100));
		addProperty(opacity, 0, property);

		// ��С
		QtProperty *size = sizeManager->addProperty(UDQ_TR("��С"));
		size->setToolTip(UDQ_TR("����ʵ���Ĵ�С"));
		size->setWhatsThis(UDQ_T("Size"));
		sizeManager->setValue(size, region->boundingRect().size().toSize());
		addProperty(size, 0, property);

	}
	
	///////////////////////////////////////////////////////////////////////////
	// ʵ������
	property = groupManager->addProperty(UDQ_TR("ʵ������"));
	addProperty(property, hashString(UDQ_T("ObjectTypeInstance")));

	QtProperty *instance = NULL;
	foreach(PluginVariable* def, region->typePrototype_->variables_.values())
	{
		instance = addInstance(def, region->getParameterString(def->name));
		addProperty(instance, 0, property);
	}

	//////////////////////////////////////////////////////////////////////////
	// ��Ϊ����
	property = groupManager->addProperty(UDQ_TR("��Ϊ����"));
	addProperty(property, hashString(UDQ_T("BehaviorProperty")));

	PluginResourceObject* pluginatt = NULL;
	QtProperty* behProp = NULL;
	foreach(QString name, region->typePrototype_->behaviors_){
		pluginatt = region->typePrototype_->attachments_.value(name);

		// ������������
		behProp = groupManager->addProperty(name);
		addProperty(behProp, 0, property);

		foreach(PluginVariable* def, pluginatt->variables_.values())
		{
			instance = addInstance(def, region->getParameterString(def->name, name), name);
			addProperty(instance, 0, behProp);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ��Ч����
	property = groupManager->addProperty(UDQ_TR("��Ч����"));
	addProperty(property, hashString(UDQ_T("EffectProperty")));

	QStringList blendMode;
	blendMode.append(UDQ_TR("Normal"));
	blendMode.append(UDQ_TR("Additive"));
	blendMode.append(UDQ_TR("XOR"));
	blendMode.append(UDQ_TR("Copy"));
	blendMode.append(UDQ_TR("Destination Over"));
	blendMode.append(UDQ_TR("Source In"));
	blendMode.append(UDQ_TR("Destination In"));
	blendMode.append(UDQ_TR("Source Out"));
	blendMode.append(UDQ_TR("Destination Out"));
	blendMode.append(UDQ_TR("Source Atop"));
	blendMode.append(UDQ_TR("Destination Atop"));

	// ���ģʽ
	QtProperty *blend = enumManager->addProperty(UDQ_TR("���ģʽ"));
	blend->setToolTip(UDQ_TR("��Ч�����Ⱦģʽ"));
	blend->setWhatsThis(UDQ_T("Blend"));
	enumManager->setEnumNames(blend, blendMode);
	enumManager->setValue(blend, region->blend_);
	addProperty(blend, 0, property);

	// �ص�����
	blendMode.append(UDQ_TR("Destroy"));
	QtProperty *fallback = enumManager->addProperty(UDQ_TR("�ص�����"));
	fallback->setToolTip(UDQ_TR("�����Ч�����ڣ����ȡ�Ĵ���ģʽ"));
	fallback->setWhatsThis(UDQ_T("Fallback"));
	enumManager->setEnumNames(fallback, blendMode);
	enumManager->setValue(fallback, region->fallback_);
	addProperty(fallback, 0, property);

	foreach(QString name, region->typePrototype_->effects_){
		pluginatt = region->typePrototype_->attachments_.value(name);

		// ������������
		behProp = groupManager->addProperty(name);
		addProperty(behProp, 0, property);

		foreach(PluginVariable* def, pluginatt->variables_.values())
		{
			instance = addInstance(def, region->getParameterString(def->name, name), name);
			addProperty(instance, 0, behProp);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ���ԭ������
	property = groupManager->addProperty(UDQ_TR("���ԭ������"));
	addProperty(property, hashString(UDQ_T("PluginProperty")));

	foreach(ResourceHash key, region->typePrototype_->prototype_->varIndex_)
	{
		PluginVariable* def = region->typePrototype_->prototype_->variables_.value(key);
		Q_ASSERT(def != NULL);

		if (def->flags & VAR_EXTEND){
			// ��չ���Բ���ʾ
		}
		else{
			instance = addInstance(def, region->getParameterString(def->name, region->typePrototype_->prototype_->objectName()), region->typePrototype_->prototype_->objectName());
			addProperty(instance, 0, property);
		}

	}

}

void ObjectPropertyWindow::prepareObjectTypeProperty(ObjectTypeResourceObject* obj){
	QtProperty *property = NULL;
	QtGroupPropertyManager* groupManager = (QtGroupPropertyManager*)managers_[MT_GROUP];
	QtStringPropertyManager* stringManager = (QtStringPropertyManager*)managers_[MT_STRING];
	QtColorPropertyManager* colorManager = (QtColorPropertyManager*)managers_[MT_COLOR];
	QtIntPropertyManager* intManager = (QtIntPropertyManager*)managers_[MT_INT];
	QtEnumPropertyManager* enumManager = (QtEnumPropertyManager*)managers_[MT_ENUM];
	QtDoublePropertyManager* floatManager = (QtDoublePropertyManager*)managers_[MT_DOUBLE];
	QtPointPropertyManager* pointManager = (QtPointPropertyManager*)managers_[MT_POINT];
	QtDateTimePropertyManager* datetimeManager = (QtDateTimePropertyManager*)managers_[MT_DATETIME];

	QStringList boolEnums;
	boolEnums << UDQ_TR("��") << UDQ_TR("��");

	// ͼ��
	property = groupManager->addProperty(UDQ_TR("������������"));
	addProperty(property, obj->hashKey());

	// ���������
	QtProperty *name = stringManager->addProperty(UDQ_TR("����"));
	name->setToolTip(UDQ_TR("������������"));
	name->setWhatsThis(UDQ_T("ObjectType"));
	stringManager->setValue(name, obj->description());
	addProperty(name, 0, property);

	QtProperty *plugin = stringManager->addProperty(UDQ_TR("�������"));
	plugin->setToolTip(UDQ_TR("�������͵Ĳ������"));
	plugin->setWhatsThis(UDQ_T("Plugin"));
	plugin->setEnabled(false);
	stringManager->setValue(plugin, obj->prototype_->title_);
	addProperty(plugin, 0, property);

	QtProperty *global = enumManager->addProperty(UDQ_TR("�Ƿ�ȫ��"));
	global->setToolTip(UDQ_TR("���������Ƿ�Ϊȫ�ֶ���(ȫ�ֶ����ڳ��������󲻻ᱻɾ��)"));
	global->setWhatsThis(UDQ_T("Global"));
	enumManager->setEnumNames(global, boolEnums);
	enumManager->setValue(global, obj->global_ ? 1 : 0);
	addProperty(global, 0, property);

	///////////////////////////////////////////////////////////////////////////
	// ʵ������
	property = groupManager->addProperty(UDQ_TR("ʵ������"));
	addProperty(property, hashString(UDQ_T("ObjectTypeInstance")));

	QtProperty *instance = NULL;
	foreach(PluginVariable* def, obj->variables_.values())
	{
		instance = addInstance(def, obj->getParameter(def->name));
		addProperty(instance, 0, property);
	}

	//////////////////////////////////////////////////////////////////////////
	// ��Ϊ����
	property = groupManager->addProperty(UDQ_TR("��Ϊ����"));
	addProperty(property, hashString(UDQ_T("BehaviorProperty")));

	PluginResourceObject* pluginatt = NULL;
	QtProperty* behProp = NULL;
	foreach(QString name, obj->behaviors_){
		pluginatt = obj->attachments_.value(name);

		// ������������
		behProp = groupManager->addProperty(name);
		addProperty(behProp, 0, property);

		foreach(PluginVariable* def, pluginatt->variables_.values())
		{
			instance = addInstance(def, obj->getParameter(def->name, name), name );
			addProperty(instance, 0, behProp);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ��Ч����
	property = groupManager->addProperty(UDQ_TR("��Ч����"));
	addProperty(property, hashString(UDQ_T("EffectProperty")));

	foreach(QString name, obj->effects_){
		pluginatt = obj->attachments_.value(name);
		
		// ������������
		behProp = groupManager->addProperty(name);
		addProperty(behProp, 0, property);

		foreach(PluginVariable* def, pluginatt->variables_.values())
		{
			instance = addInstance(def, obj->getParameter(def->name, name), name);
			addProperty(instance, 0, behProp);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// ���ԭ������
	property = groupManager->addProperty(UDQ_TR("���ԭ������"));
	addProperty(property, hashString(UDQ_T("PluginProperty")));

	foreach(ResourceHash key, obj->prototype_->varIndex_)
	{
		PluginVariable* def = obj->prototype_->variables_.value(key);
		Q_ASSERT(def != NULL);

		if (def->flags & VAR_EXTEND){
			// ��չ���Բ���ʾ
		}
		else{
			instance = addInstance(def, obj->getParameter(def->name, obj->prototype_->objectName()), obj->prototype_->objectName());
			addProperty(instance, 0, property);
		}

	}
}


void ObjectPropertyWindow::updateInstance(QtProperty* prop, PluginVariable* var, const QString& value){
	QtStringPropertyManager* stringManager = (QtStringPropertyManager*)managers_[MT_STRING];
	QtIntPropertyManager* intManager = (QtIntPropertyManager*)managers_[MT_INT];
	QtEnumPropertyManager* enumManager = (QtEnumPropertyManager*)managers_[MT_ENUM];
	QtDoublePropertyManager* floatManager = (QtDoublePropertyManager*)managers_[MT_DOUBLE];
	QtColorPropertyManager* colorManager = (QtColorPropertyManager*)managers_[MT_COLOR];
	QtFontPropertyManager* fontManager = (QtFontPropertyManager*)managers_[MT_FONT];

	QString v = value;
	if (v.isEmpty()){
		v = var->value;
	}

	Q_ASSERT(var != NULL);
	if (var->datatype == UDQ_T("bool")){
		enumManager->setValue(prop, v == UDQ_T("true") ? 1 : 0);
	}
	else if (var->datatype == UDQ_T("int")){
		intManager->setValue(prop, v.toInt());
	}
	else if (var->datatype == UDQ_T("float")){
		floatManager->setValue(prop, v.toDouble());
	}
	else if (var->datatype == UDQ_T("string")){
		stringManager->setValue(prop, v);
	}
	else if (var->datatype == UDQ_T("enum")){
		enumManager->setValue(prop, var->enums.indexOf(v));
	}
	else if (var->datatype == UDQ_T("percent")){
		floatManager->setValue(prop, v.toDouble());
	}
	else if (var->datatype == UDQ_T("font")){
		QFont ft;
		ft.fromString(v);
		fontManager->setValue(prop, ft);
	}
	else if (var->datatype == UDQ_T("color")){
		colorManager->setValue(prop, QColor(v));
	}
	else{
		Q_UNREACHABLE();
	}
}

QtProperty* ObjectPropertyWindow::addInstance(PluginVariable* var, const QString& value, const QString& prefix){
	QtProperty *property = NULL;

	QtStringPropertyManager* stringManager = (QtStringPropertyManager*)managers_[MT_STRING];
	QtIntPropertyManager* intManager = (QtIntPropertyManager*)managers_[MT_INT];
	QtEnumPropertyManager* enumManager = (QtEnumPropertyManager*)managers_[MT_ENUM];
	QtDoublePropertyManager* floatManager = (QtDoublePropertyManager*)managers_[MT_DOUBLE];
	QtColorPropertyManager* colorManager = (QtColorPropertyManager*)managers_[MT_COLOR];
	QtFontPropertyManager* fontManager = (QtFontPropertyManager*)managers_[MT_FONT];

	QStringList boolEnums;
	boolEnums << UDQ_TR("��") << UDQ_TR("��");

	QString text = var->title;
	if (text.isEmpty()){
		text = var->name;
	}

	QString v = value;
	if (v.isEmpty()){
		v = var->value;
	}

	Q_ASSERT(var != NULL);
	if (var->datatype == UDQ_T("bool")){
		property = enumManager->addProperty(text);
		enumManager->setEnumNames(property, boolEnums);
		enumManager->setValue(property, v == UDQ_T("true") ? 1 : 0);
	}
	else if (var->datatype == UDQ_T("int")){
		property = intManager->addProperty(text);
		intManager->setValue(property, v.toInt());
	}
	else if (var->datatype == UDQ_T("float")){
		property = floatManager->addProperty(text);
		floatManager->setValue(property, v.toDouble());
	}
	else if (var->datatype == UDQ_T("string")){
		property = stringManager->addProperty(text);
		stringManager->setValue(property, v);
	}
	else if (var->datatype == UDQ_T("enum")){
		property = enumManager->addProperty(text);
		enumManager->setEnumNames(property, var->enums);
		enumManager->setValue(property, var->enums.indexOf(v));
	}
	else if (var->datatype == UDQ_T("percent")){
		property = floatManager->addProperty(text);
		floatManager->setValue(property, v.toDouble());
	}
	else if (var->datatype == UDQ_T("font")){
		property = fontManager->addProperty(text);
		QFont ft;
		ft.fromString(v);
		fontManager->setValue(property, ft);
	}
	else if (var->datatype == UDQ_T("color")){
		property = colorManager->addProperty(text);
		colorManager->setValue(property, QColor(v));
	}
	else{
		Q_UNREACHABLE();
	}

	property->setEnabled(true);
	property->setToolTip(var->desc);

	if (prefix.isEmpty()){
		property->setWhatsThis(UDQ_T("@")+ var->name);  // ʵ������ʹ��
	}
	else{
		property->setWhatsThis(UDQ_T("@") + prefix + UDQ_T(".") + var->name);  // ʵ������ʹ��
	}

	return property;
}


void ObjectPropertyWindow::prepareFamilyProperty(FamilyResourceObject* family){
	QtProperty *property = NULL;
	QtGroupPropertyManager* groupManager = (QtGroupPropertyManager*)managers_[MT_GROUP];
	QtStringPropertyManager* stringManager = (QtStringPropertyManager*)managers_[MT_STRING];
	QtColorPropertyManager* colorManager = (QtColorPropertyManager*)managers_[MT_COLOR];
	QtIntPropertyManager* intManager = (QtIntPropertyManager*)managers_[MT_INT];
	QtEnumPropertyManager* enumManager = (QtEnumPropertyManager*)managers_[MT_ENUM];
	QtDoublePropertyManager* floatManager = (QtDoublePropertyManager*)managers_[MT_DOUBLE];
	QtPointPropertyManager* pointManager = (QtPointPropertyManager*)managers_[MT_POINT];
	QtDateTimePropertyManager* datetimeManager = (QtDateTimePropertyManager*)managers_[MT_DATETIME];

	QStringList boolEnums;
	boolEnums << UDQ_TR("��") << UDQ_TR("��");

	// ͼ��
	property = groupManager->addProperty(UDQ_TR("������������"));
	addProperty(property, family->hashKey());

	// ���������
	QtProperty *name = stringManager->addProperty(UDQ_TR("����"));
	name->setToolTip(UDQ_TR("��������"));
	name->setWhatsThis(UDQ_T("Name"));
	stringManager->setValue(name, family->description());
	addProperty(name, 0, property);

	///////////////////////////////////////////////////////////////////////////
	// ֻ��
	// ʵ������

	// ��Ϊ

	// ��Ч
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

void ObjectPropertyWindow::propertySelected(QtBrowserItem *item){
	ui->desclabel->clear(); 
	if (item == NULL){
		return;
	}

	QtProperty* prop = item->property();
	if (prop){
		QString desc = prop->toolTip();
		if (desc.isEmpty()){
			desc = prop->propertyName();
		}
		ui->desclabel->setText(desc);
	}

}


void ObjectPropertyWindow::updateFamilyProperty(FamilyResourceObject* family, const QString& varid, const QString& value){

}

void ObjectPropertyWindow::updateObjectTypeProperty(ObjectTypeResourceObject* obj, const QString& varid, const QString& value){
	QStringList chunks = varid.split(UDQ_T("."));
	Q_ASSERT(chunks.size() <= 2);

	if (chunks.size() == 1){
		obj->setParameter(chunks[0], value);
	}
	else{
		// ��ȡ�������ͣ������ö�����͵Ļ�
		PluginResourceObject* plugin = obj->getPlugin(chunks[0]);
		if (plugin){
			PluginVariable* var = plugin->getVariable(chunks[1]);
			if (var){
				if (var->datatype == UDQ_T("enum")){
					obj->setParameter(chunks[1], var->enums.at(value.toInt()), chunks[0]);
				}
				else{
					obj->setParameter(chunks[1], value, chunks[0]);
				}
			}
		}
	}
}

void ObjectPropertyWindow::updateRegionProperty(EditorRegion* region, const QString& varid, const QString& value){
	QStringList chunks = varid.split(UDQ_T("."));
	Q_ASSERT(chunks.size() <= 2);
	if (chunks.size() == 1){
		PluginVariable* var = region->typePrototype_->getVariableDefine(chunks[0]);
		if (var){
			if (var->datatype == UDQ_T("bool")){
				int index = value.toInt();
				region->setParameter(chunks[0], index ? UDQ_T("true") : UDQ_T("false"));
			}
			else{
				region->setParameter(chunks[0], value);
			}
		}
	}
	else{
		// ��ȡ�������ͣ������ö�����͵Ļ�
		PluginResourceObject* plugin = region->typePrototype_->getPlugin(chunks[0]);
		if (plugin){
			PluginVariable* var = plugin->getVariable(chunks[1]);
			if (var){
				if (var->datatype == UDQ_T("enum")){
					region->setParameter(chunks[1], var->enums.at(value.toInt()), chunks[0]);
				}
				else if (var->datatype == UDQ_T("bool")){
					int index = value.toInt();
					region->setParameter(chunks[1], index ? UDQ_T("true") : UDQ_T("false"), chunks[0]);
				}
				else{
					region->setParameter(chunks[1], value, chunks[0]);
				}
			}
		}
	}
}


void ObjectPropertyWindow::updateProperty(EditorItem* obj , bool isAll){

	QtIntPropertyManager* intManager = (QtIntPropertyManager*)managers_[MT_INT];
	QtPointPropertyManager* pointManager = (QtPointPropertyManager*)managers_[MT_POINT];
	QtSizePropertyManager* sizeManager = (QtSizePropertyManager*)managers_[MT_SIZE];
	QtEnumPropertyManager* enumManager = (QtEnumPropertyManager*)managers_[MT_ENUM];

	isUpdating_ = true;

	if (obj == NULL || currentSceneItem_ != obj){
		isUpdating_ = false;
		return;
	}


	EditorRegion* region = (EditorRegion*)obj;
	if (region){
		// ֻ����λ�ô�С�Ƕ�
		foreach(QtProperty* prop, subproperties_){
			QString propId = prop->whatsThis();
			if (propId == UDQ_T("Position")){
				pointManager->setValue(prop, region->pos().toPoint());
			}
			else if (propId == UDQ_T("Size")){
				sizeManager->setValue(prop, region->boundingRect().size().toSize());
			}
			else if (propId == UDQ_T("Angle")){
				intManager->setValue(prop, qRound(region->rotation()));
			}
			else if (propId == UDQ_T("Layer")){
				// ��õ�ǰ������ͼ���б�
				LevelScene* scene = (LevelScene*)currentSceneItem_->scene();
				Q_ASSERT(scene != NULL);
				QStringList layerNames = scene->layerNames();

				EditorLayer* l = (EditorLayer*)region->parentItem();
				if (l){
					enumManager->setEnumNames(prop, layerNames);
					enumManager->setValue(prop, layerNames.indexOf(l->layerName()));
				}
			}
			else if (isAll){

				if (propId == UDQ_T("Opacity")){
					intManager->setValue(prop, qRound(region->opacity() * 100));
				}
				else if (propId == UDQ_T("Blend")){
					enumManager->setValue(prop, region->blend_);
				}
				else if (propId == UDQ_T("Fallback")){
					enumManager->setValue(prop, region->fallback_);
				}
				else if (propId.startsWith(UDQ_T("@"))){  // ����
					// ���±���ֵ
					// ��ȡ�����������ֵ
					QString fullname = propId.mid(1);
					QStringList entry = fullname.split(UDQ_T("."));
					Q_ASSERT(entry.size() <= 2);
					QString pluginName;
					if (entry.size() > 1){
						pluginName = entry.takeFirst();
					}
					PluginVariable* var = region->typePrototype_->getVariableDefine(entry[0], pluginName);
					Q_ASSERT(var != NULL);
					QString value = region->getParameterString(entry[0], pluginName);
					updateInstance(prop, var, value);
				}
			}	
		}
	}
	isUpdating_ = false;
}