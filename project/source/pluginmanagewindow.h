#ifndef PLUGINMANAGEWINDOW_H
#define PLUGINMANAGEWINDOW_H

#include <QDialog>
#include <QMultiMap>
#include <QListWidgetItem>
#include "resourceobject.h"
#include "resourcelinker.h"
using namespace ORION;

namespace Ui {
class PluginManageWindow;
}

class PluginManageWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PluginManageWindow(QWidget *parent = 0);
    ~PluginManageWindow();

	void initFromDb(void);

	// ��Ч�Լ��
	bool checkValid();

private:
    Ui::PluginManageWindow *ui;

	QTreeWidgetItem* nativeRootItem_;
	QTreeWidgetItem* entityRootItem_;
	QTreeWidgetItem* behaviorRootItem_;
	QTreeWidgetItem* effectRootItem_;

	QTreeWidgetItem* variableRootItem_;
	QTreeWidgetItem* conditionRootItem_;
	QTreeWidgetItem* actionRootItem_;
	QTreeWidgetItem* expressRootItem_;

	QMultiMap<int, QListWidgetItem*> attrItemsGroup_;  // ����Item
	QMap<int, QListWidgetItem*> attrItems_;  // ����Item
	QMap<QString, QListWidgetItem*> attrStringItems_;  // ����Item
	void prepareAttrItems(int type, const QMap<int, QString>& items);

	// ������Ŀ
	QTreeWidgetItem* findCateory(int type, const QString& cate, bool create = true );

	// ��������Item
	QTreeWidgetItem* findPropertyItem(int type, const QString& id, bool create = true);
	QTreeWidgetItem* findInterfaceItem(int type, const QString& id, bool create = true);

	// �������Item
	QTreeWidgetItem* addVariableItem(PluginVariable* var);
	QTreeWidgetItem* addVariableItem( const QJsonObject& data );

	QTreeWidgetItem* addInterfaceItem(PluginInterface* inter);
	QTreeWidgetItem* addInterfaceItem(int type, const QJsonObject& data);


	QByteArray tmpFiledat_;  // ��ʱ�ļ�����
	QByteArray tmpJsonDat_; // ��������

	QMultiHash<int, QByteArray> assosiateFiles_;   // �����ļ��б�

	void clearChildItem(QTreeWidgetItem* parent);



private slots:
	void on_savepushButton_clicked();
	void on_cancelpushButton_clicked();
	void on_deletepushButton_clicked();
	void on_filepushButton_clicked();


	void on_plugintreeWidget_itemSelectionChanged();
	void on_plugintreeWidget_itemDoubleClicked(QTreeWidgetItem * item, int column);

	void on_typecomboBox_currentIndexChanged(int index);
	void on_iconpushButton_clicked();

	void on_importpushButton_clicked();
	void on_exportpushButton_clicked();
	void on_clearpushButton_clicked();

	void on_addproppushButton_clicked();
	void on_delproppushButton_clicked();

	void on_proptreeWidget_itemDoubleClicked(QTreeWidgetItem * item, int column);

	void on_jsonimppushButton_clicked();
	void on_jsonexppushButton_clicked();
};

#endif // PLUGINMANAGEWINDOW_H
