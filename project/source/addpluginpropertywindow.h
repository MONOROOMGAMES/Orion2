#ifndef ADDPLUGINPROPERTYWINDOW_H
#define ADDPLUGINPROPERTYWINDOW_H

#include <QDialog>
#include <QMultiHash>
#include <QListWidgetItem>
#include "commondefine.h"
#include "resourceobject.h"
using namespace ORION;

namespace Ui {
class AddPluginPropertyWindow;
}

class AddPluginPropertyWindow : public QDialog
{
    Q_OBJECT

public:
	explicit AddPluginPropertyWindow(int pluginType, const QMultiHash<int, QString>& existIds, QWidget *parent = 0);
	explicit AddPluginPropertyWindow(int pluginType, const QMultiHash<int, QString>& existIds, const QByteArray& editprop, QWidget *parent = 0);  // �༭ģʽ

    ~AddPluginPropertyWindow();

	bool checkValid();

	// ������������
	void loadProperty(const QByteArray& editprop);

	QMap<QString, QByteArray> newVar_;  // ����ӵ�����
	QMap<QString, QByteArray> newCnd_;  // ����ӵ�����
	QMap<QString, QByteArray> newAct_;  // ����ӵ�����
	QMap<QString, QByteArray> newExp_;  // ����ӵ�����

	QMultiHash<int, QString> existIds_;  // ��������ID����
	QMultiHash<int, QString> newIds_;  // �¼ӵ�����ID����

	QMap<int, QStringList> flags_;  // ���ͱ�־

private:
    Ui::AddPluginPropertyWindow *ui;

	bool editmode_;
	void init( int type);
	// ����ظ�ID
	bool duplicatedId(int type, const QString& id);
	QMultiMap<int, QListWidgetItem*> flagItemsGroup_;  // ����Item
	QMap<int, QListWidgetItem*> flagItems_;  // ����Item
	void prepareFlagItems(int type, const QMap<int, QString>& items);
	QListWidgetItem* addEnum(const QString& value);
private slots:
	void on_addpushButton_clicked();
	void on_cancelpushButton_clicked();

	void on_propcomboBox_currentIndexChanged(int index);

	void on_datatypecomboBox_currentIndexChanged(const QString & text);
	void on_addenumpushButton_clicked();
	void on_delenumpushButton_clicked();
	void on_enumlistWidget_itemDoubleClicked(QListWidgetItem * item);

	void on_addparampushButton_clicked();
	void on_delparampushButton_clicked();
};

#endif // ADDPLUGINPROPERTYWINDOW_H
