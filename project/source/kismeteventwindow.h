#ifndef KISMETEVENTWINDOW_H
#define KISMETEVENTWINDOW_H

#include <QWidget>
#include <QTreeWidget>

#include "resourceobject.h"
using namespace ORION;

namespace Ui {
class KismetEventWindow;
}

class KismetEventWindow : public QWidget
{
    Q_OBJECT

public:
    explicit KismetEventWindow(QWidget *parent = 0);
    ~KismetEventWindow();

	// ׼���¼��嵥
	void collectCommonEvent();
	// ��ʾϵͳ���
	void preparePluginList();
	void prepareEventList(PluginResourceObject* plugin);

	QTreeWidgetItem* findCateory(const QString& cate, QTreeWidgetItem* rootItem, bool create = true);

	// ��ӹ����ӿ�
	void addCommonConditionEvent(PluginResourceObject* plugin, const QList<int>& attrs, QTreeWidgetItem* rootItem);
	void addConditionEvent(PluginResourceObject* plugin,  QTreeWidgetItem* rootItem);
	void addCommonActionEvent(PluginResourceObject* plugin, const QList<int>& attrs, QTreeWidgetItem* rootItem);
	void addActionEvent(PluginResourceObject* plugin, QTreeWidgetItem* rootItem);
	void addCommonExpressEvent(PluginResourceObject* plugin, const QList<int>& attrs, QTreeWidgetItem* rootItem);
	void addExpressEvent(PluginResourceObject* plugin, QTreeWidgetItem* rootItem);

	// �������Item
	QTreeWidgetItem* addConditionItem(PluginInterface* inter, QTreeWidgetItem* rootItem);
	QTreeWidgetItem* addActionItem(PluginInterface* inter, QTreeWidgetItem* rootItem);
	QTreeWidgetItem* addExpressItem(PluginInterface* inter, QTreeWidgetItem* rootItem);

	QTreeWidgetItem* findPropertyItem(int type, const QString& id, QTreeWidgetItem* parent, bool create = true);

protected:
	virtual void resizeEvent(QResizeEvent * event);

private:
    Ui::KismetEventWindow *ui;
	QMap<QString, int> aces_;

	// ��ӹ����ӿ�
	void addCommonEventItem(int type, PluginInterface* inter);
	QMultiHash<int, PluginInterface*> commonCondition_;  // �����ӿ�
	QMultiHash<int, PluginInterface*> commonAction_;  // �����ӿ�
	QMultiHash<int, PluginInterface*> commonExpress_;  // �����ӿ�

	QMap<ResourceHash, QTreeWidgetItem*>  PluginConditions_;
	QMap<ResourceHash, QTreeWidgetItem*>  PluginActions_;
	QMap<ResourceHash, QTreeWidgetItem*>  PluginExpresses_;

	void clear();
	QIcon folderIcon_;
	PluginResourceObject* currentPlugin_;   // ��ǰ��ʾ���


	QList<QTreeWidgetItem*> interfaceItems_;  // ���нӿ���
private slots:
	void on_refreshpushButton_clicked();
	void on_pluginpushButton_clicked();

	void on_plugintreeWidget_itemClicked(QTreeWidgetItem * item, int column);
	void on_filterlineEdit_textEdited(const QString & text);


};

#endif // KISMETEVENTWINDOW_H
