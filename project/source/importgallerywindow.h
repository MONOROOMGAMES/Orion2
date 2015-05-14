#ifndef IMPORTGALLERYWINDOW_H
#define IMPORTGALLERYWINDOW_H

#include <QDialog>
#include <QListWidgetItem>
#include "tooldefine.h"

namespace Ui {
class ImportGalleryWindow;
}

class ImportGalleryWindow : public QDialog
{
    Q_OBJECT

public:
    explicit ImportGalleryWindow(const QString& category, int filter = FILTER_ALL, QWidget *parent = 0);
    ~ImportGalleryWindow();

private:
    Ui::ImportGalleryWindow *ui;

	QStringList loadFiles_;  // ����ȡ���ļ��б�

	QStringList fileext_;  //   �ļ���չ��
	QStringList imgAttributes_;  // ��Ҫ���ʵı�ǩ�����б�

	QString cate_;  // ������Ŀ

	// ��Ч�Լ��
	bool vaildCheck(QListWidgetItem* item, const QString& pkg, bool overwrite = false);
	void saveToDb(QListWidgetItem* item, const QString& pkg, const QString& path);
	void saveToDb(QListWidgetItem* item, const QString& pkg, const QByteArray& data, const QString& originType );

	private slots:
	void on_savepushButton_clicked();
	void on_cancelpushButton_clicked();
	void on_selectallcheckBox_stateChanged(int state);
	void on_pathpushButton_clicked();
	void on_filepushButton_clicked();

	void on_imagecheckBox_stateChanged(int state);
	void on_audiocheckBox_stateChanged(int state);
	void on_videocheckBox_stateChanged(int state);

	void loadFile(const QString& localFile, int type);
};

#endif // IMPORTGALLERYWINDOW_H
