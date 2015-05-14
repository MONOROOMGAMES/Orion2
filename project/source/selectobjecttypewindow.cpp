#include "selectobjecttypewindow.h"
#include "ui_selectobjecttypewindow.h"
#include "tooldefine.h"


SelectObjectTypeWindow::SelectObjectTypeWindow(QWidget *parent) :
QDialog(parent),
    ui(new Ui::SelectObjectTypeWindow)
{
    ui->setupUi(this);
	setFixedSize(size());

	// ��������������
	ui->treeWidget->setDragEnabled(false);
	ui->treeWidget->setAssociateItem(false);
	// ���浽�ļ�
	WorkspaceResourceObject*  ws = ResourceObject::currentWorkspace();
	Q_ASSERT(ws != NULL);
	ProjectResourceObject* proj = ws->currentProject();
	if (proj){
		ui->treeWidget->init(proj);
	}
}

SelectObjectTypeWindow::~SelectObjectTypeWindow()
{
    delete ui;
}


void SelectObjectTypeWindow::on_treeWidget_itemClicked(QTreeWidgetItem * item, int column){
	// ����Ƿ�ѡ��Ϸ�����
	if ( item->type() != ITEM_OBJECTTYPE &&  item->type() != ITEM_FAMILY)
		return;
	//
	typeHash_ = item->data(0, ITEM_HASHKEY).toUInt();
	accept();
}
