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

#include "menumanager.h"
#include <QtGui>
#include "commondefine.h"

#include <QtGlobal>
#include <QLatin1Char>
#include <QLatin1String>


#define ORION_MENUITEM_DEF( id, checkable, enable, img, shortcut, text, tooltip, type) \
	static MenuItemDefine MID_##id = { id, checkable, enable, type, img, shortcut, \
	QT_TR_NOOP(text), QT_TR_NOOP(tooltip) }; \
	menuItems_.insert(id, &MID_##id);

#define EDITOR_MENUITEM_DEF( id, checkable, enable, img, shortcut, text, tooltip) \
	ORION_MENUITEM_DEF(id, checkable, enable, img, shortcut, text, tooltip, EDITOR_LEVEL)

#define SPRITE_MENUITEM_DEF( id, checkable, enable, img, shortcut, text, tooltip) \
	ORION_MENUITEM_DEF(id, checkable, enable, img, shortcut, text, tooltip, EDITOR_SPRITE)

#define KISMET_MENUITEM_DEF( id, checkable, enable, img, shortcut, text, tooltip) \
	ORION_MENUITEM_DEF(id, checkable, enable, img, shortcut, text, tooltip, EDITOR_KISMET)

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

QHash<int, MenuItemDefine*> MenuManager::menuItems_;

void MenuManager::staticInit(){

	EDITOR_MENUITEM_DEF(ORION_PROJECT_NEW, 0, 1, ":/images/project_new.png", "", "�½�", "�½�һ������");
	EDITOR_MENUITEM_DEF(ORION_PROJECT_SAVE, 0, 1, ":/images/project_save.png", "Ctrl+S", "����", "���浱ǰ����");
	EDITOR_MENUITEM_DEF(ORION_PROJECT_SAVEAS, 0, 1, "", "", "���Ϊ...", "��浱ǰ����");
	EDITOR_MENUITEM_DEF(ORION_PROJECT_EXPORT, 0, 1, ":/images/export.png", "", "����", "�����̴���������ļ�");
	EDITOR_MENUITEM_DEF(ORION_PROJECT_IMPORT, 0, 1, ":/images/import.png", "", "����", "�ӱ��ش���ļ�����");

	EDITOR_MENUITEM_DEF(ORION_PROJECT_DELETE, 0, 1, ":/images/delete.png", "", "ɾ��", "ɾ����ǰ");
	EDITOR_MENUITEM_DEF(ORION_PROJECT_OPEN, 0, 1, ":/images/project_open.png", "", "��", "��һ�����й���");
	EDITOR_MENUITEM_DEF(ORION_PROJECT_CLOSE, 0, 1, "", "Ctrl+E", "�ر�", "�رյ�ǰ�򿪵Ĺ���");
	EDITOR_MENUITEM_DEF(ORION_PROJECT_PUBLISH, 0, 1, "", "", "����", "������ǰ�򿪵Ĺ���");
	EDITOR_MENUITEM_DEF(ORION_PROJECT_EXIT, 0, 1, "", "Alt+F4", "�˳�", "�˳�Ӧ�ó���");

	//////////////////////////////////////////////////////////////////////////
	
	EDITOR_MENUITEM_DEF(ORION_EDIT_POINTER, 0, 1, ":/images/pointer.png", "", "ѡ��ģʽ", "�༭������ѡ��ģʽ");
	EDITOR_MENUITEM_DEF(ORION_EDIT_IMAGE, 0, 1, ":/images/image.png", "", "ͼƬ", "��Ӿ�̬ͼƬ");

	EDITOR_MENUITEM_DEF(ORION_EDIT_LOCK, 1, 1, ":/images/locky.png", "", "����", "����ѡ�еĶ����޷��ƶ���");

	EDITOR_MENUITEM_DEF(ORION_EDIT_DELETE, 0, 1, ":/images/crash.png", "", "ɾ��", "ɾ��ѡ�еĶ���");
	EDITOR_MENUITEM_DEF(ORION_EDIT_CLONE, 0, 1, ":/images/clone.png", "", "��¡������", "�ָ�Ϊȱʡ��������");
	EDITOR_MENUITEM_DEF(ORION_EDIT_COPY, 0, 1, ":/images/copy.png", "", "����", "����");
	EDITOR_MENUITEM_DEF(ORION_EDIT_CUT, 0, 1, ":/images/cut.png", "", "����", "����");
	EDITOR_MENUITEM_DEF(ORION_EDIT_PASTE, 0, 1, ":/images/paste.png", "", "ճ��", "ճ��");

	EDITOR_MENUITEM_DEF(ORION_EDIT_MOVEFIRST, 0, 1, ":/images/movefirst.png", "", "�ƶ�������", "����ǰ�����ƶ�������ͼ���������");
	EDITOR_MENUITEM_DEF(ORION_EDIT_MOVELAST, 0, 1, ":/images/movelast.png", "", "�ƶ����ײ�", "����ǰ�����ƶ�������ͼ���������");

	EDITOR_MENUITEM_DEF(ORION_EDIT_PANELOPTION,1, 1, ":/images/objects.png", "", "�ز����", "��ʾ/�����ز����");

	EDITOR_MENUITEM_DEF(ORION_EDIT_ADDEFFECT, 0, 1, "", "", "�����Ч", "��ѡ�еĶ��������Ч");

	EDITOR_MENUITEM_DEF(ORION_EDIT_TODEFAULTOBJ, 0, 1, "", "", "����Ϊȱʡ����", "����Ϊȱʡ����");
	EDITOR_MENUITEM_DEF(ORION_EDIT_FROMDEFAULTOBJ, 0, 1, "", "", "�ָ�ȱʡֵ", "�ָ�Ϊȱʡ��������");

	EDITOR_MENUITEM_DEF(ORION_EDIT_SOUND, 0, 1, ":/images/unique32.png", "", "�༭", "�༭��������");
	EDITOR_MENUITEM_DEF(ORION_EDIT_MOVIE, 0, 1, ":/images/unique32.png", "", "�༭", "�༭��Ӱ����");

	EDITOR_MENUITEM_DEF(ORION_EDIT_OBJECTTYPE, 0, 1, ":/images/locky.png", "", "�༭", "�༭��������");

	//////////////////////////////////////////////////////////////////////////
	//
	EDITOR_MENUITEM_DEF(ORION_OBJECTTYPE_DELETE, 0, 1, "", "", "ɾ��", "ɾ��ѡ�еĶ�������");
	EDITOR_MENUITEM_DEF(ORION_OBJECTTYPE_ADDCATEGORY, 0, 1, "", "", "��ӷ���", "��Ӷ������ͷ���");

	//////////////////////////////////////////////////////////////////////////

	EDITOR_MENUITEM_DEF(ORION_GALLERY_EDIT, 0, 1, ":/images/gallery.png", "", "�زĿ�", "���زĿ������");
	EDITOR_MENUITEM_DEF(ORION_GALLERY_SPRITE, 0, 1, ":/images/macpanel.png", "", "�����༭", "�򿪾��鶯���༭��");
	EDITOR_MENUITEM_DEF(ORION_GALLERY_PLUGIN, 0, 1, ":/images/macpanel.png", "", "�������", "�򿪲��������");
	EDITOR_MENUITEM_DEF(ORION_GALLERY_BLUEPRINT, 0, 1, ":/images/edit_blueprint.png", "", "�ű��༭", "���¼��ű��༭��");

	EDITOR_MENUITEM_DEF(ORION_GALLERY_ADDCATEGORY, 0, 1, ":/images/unique32.png", "", "�����Ŀ", "�ڵ�ǰλ������µ���Ŀ");
	EDITOR_MENUITEM_DEF(ORION_GALLERY_DELETECATEGORY, 0, 1, ":/images/unique32.png", "", "ɾ��", "ɾ����ǰѡ�е���Ŀ");

	EDITOR_MENUITEM_DEF(ORION_GALLERY_MERGE, 0, 1, ":/images/unique32.png", "", "�ϲ�", "�����ͼƬ�ϲ���һ��ͼƬ");
	EDITOR_MENUITEM_DEF(ORION_GALLERY_TRANSPARENT, 0, 1, ":/images/unique32.png", "", "����͸��", "��ͼƬ�ı���ɫ��Ϊ͸��ɫ");
	EDITOR_MENUITEM_DEF(ORION_GALLERY_COPY, 0, 1, ":/images/unique32.png", "", "����", "����ѡ�е���Դ������������");
	EDITOR_MENUITEM_DEF(ORION_GALLERY_SPLIT, 0, 1, ":/images/unique32.png", "", "�ָ�", "�ָ�ѡ�е�ͼ��");
	
	EDITOR_MENUITEM_DEF(ORION_HELP_ABOUT, 0, 1, "", "", "���� Orion Editor", "��ʾ���ڶԻ���");
	EDITOR_MENUITEM_DEF(ORION_HELP_WELCOME, 0, 1, ":/images/welcome.png", "", " ��ӭҳ��", "��ʾ��ӭҳ��");

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	SPRITE_MENUITEM_DEF(ORION_SPRTIE_SAVE, 0, 1, ":/images/save.png", "", "����", "���浱ǰ���ڱ༭�ľ���");
	SPRITE_MENUITEM_DEF(ORION_SPRTIE_SAVEALL, 0, 1, ":/images/saveall.png", "", "����ȫ��", "�������б༭���ľ���");
	SPRITE_MENUITEM_DEF(ORION_SPRTIE_REFRESH, 0, 1, ":/images/refresh.png", "", "ˢ��", "ˢ�µ�ǰѡ��ľ���");

	SPRITE_MENUITEM_DEF(ORION_SPRTIE_EXPORT, 0, 1, ":/images/unique32.png", "", "����", "�����������е��ļ�");
	SPRITE_MENUITEM_DEF(ORION_SPRTIE_COPY, 0, 1, ":/images/unique32.png", "", "����", "���Ƶ�ǰѡ��Ķ���");
	SPRITE_MENUITEM_DEF(ORION_SPRTIE_DELETE, 0, 1, ":/images/unique32.png", "", "ɾ��", "ɾ����ǰѡ��Ķ���");
	
	EDITOR_MENUITEM_DEF(ORION_SPRTIE_LOCKSEQ, 1, 1, ":/images/locky.png", "", "����", "����ѡ�еĶ�������");
	SPRITE_MENUITEM_DEF(ORION_SPRTIE_ADDSEQ, 0, 1, ":/images/animation.png", "", "��Ӷ�������", "��Ӷ�������");
	SPRITE_MENUITEM_DEF(ORION_SPRTIE_DELSEQ, 0, 1, ":/images/delete.png", "", "ɾ��", "ɾ����ǰѡ��Ķ�������");
	SPRITE_MENUITEM_DEF(ORION_SPRTIE_RENAMESEQ, 0, 1, "", "", "������", "��������������");

	SPRITE_MENUITEM_DEF(ORION_SPRTIE_ADDANCHOR, 0, 1, ":/images/unique32.png", "", "���ê��", "����µ�ê��");
	SPRITE_MENUITEM_DEF(ORION_SPRTIE_DELANCHOR, 0, 1, ":/images/unique32.png", "", "ɾ��ê��", "ɾ����ǰѡ���ê��");
	SPRITE_MENUITEM_DEF(ORION_SPRTIE_COPYANCHOR, 0, 1, ":/images/unique32.png", "", "Ӧ�õ�����֡", "����ǰ֡��ê��λ��Ӧ�õ���������֡");
	SPRITE_MENUITEM_DEF(ORION_SPRTIE_DEFAULTANCHOR, 0, 1, ":/images/unique32.png", "", "�ָ�Ĭ��", "����ǰ֡��ê��λ������ΪͼƬ����");

	SPRITE_MENUITEM_DEF(ORION_SPRITE_DEFAULTSEQ, 0, 1, ":/images/unique32.png", "", "����Ĭ������", "����ǰ����������ΪĬ������");

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	KISMET_MENUITEM_DEF(ORION_KISMET_SAVE, 0, 1, ":/images/save.png", "", "����", "�������нű�");
	KISMET_MENUITEM_DEF(ORION_KISMET_DELETE, 0, 1, ":/images/delete.png", "", "ɾ��", "ɾ��ѡ�еĽű�");
	KISMET_MENUITEM_DEF(ORION_KISMET_CLEAR, 0, 1, ":/images/crash.png", "", "���", "��յ�ǰ�༭�Ľű�");
	
	KISMET_MENUITEM_DEF(ORION_KISMET_POINTER, 1, 1, ":/images/pointer.png", "", "ָ��", "�л���ѡ��ģʽ");
	KISMET_MENUITEM_DEF(ORION_KISMET_ADDGROUP,1, 1, ":/images/group.png", "", "�¼���", "��ǰ�༭�����һ���¼���");
	KISMET_MENUITEM_DEF(ORION_KISMET_ADDCOMMENT, 1, 1, ":/images/notes.png", "", "ע��", "��ǰ�༭�����һ��ע��");

	KISMET_MENUITEM_DEF(ORION_KISMET_ADDBRANCH, 0, 1, ":/images/flowcontrol.png", "", "��֧", "��ǰ�༭�����һ����֧����");
	KISMET_MENUITEM_DEF(ORION_KISMET_ADDSEQUENCE, 0, 1, ":/images/sequence.png", "", "����", "��ǰ�༭�����һ��˳��ִ�ж���");
	KISMET_MENUITEM_DEF(ORION_KISMET_ADDMATH, 0, 1, ":/images/math.png", "", "��������", "��ǰ�༭�����һ����������");

	KISMET_MENUITEM_DEF(ORION_KISMET_ADDINSTANCEVAR, 0, 1, ":/images/unique32.png", "", "��Ա����", "���ʶ���ĳ�Ա����");
	KISMET_MENUITEM_DEF(ORION_KISMET_ADDINSTANCEINDEX, 0, 1, ":/images/unique32.png", "", "ʵ������", "��ȡָ�������Ķ���ʵ��");

	KISMET_MENUITEM_DEF(ORION_KISMET_VALIDATE, 0, 1, ":/images/check_script.png", "", "�Ϸ��Լ��", "���ű�ģ�͵ĺϷ���");

	KISMET_MENUITEM_DEF(ORION_KISMET_DELETEPORT, 0, 1, ":/images/unique32.png", "", " ɾ��", "ɾ���˿�");
	KISMET_MENUITEM_DEF(ORION_KISMET_CLEARPORTLINK, 0, 1, ":/images/unique32.png", "", "�������", "ɾ���˿ڵ���������");

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	EDITOR_MENUITEM_DEF(ORION_TOOL_DEVICE, 0, 1, "", "", "�豸", "�豸");
	EDITOR_MENUITEM_DEF(ORION_TOOL_SETUP, 0, 1, ":/images/", "", "����", "����Ԥ��/������Ϣ");
	EDITOR_MENUITEM_DEF(ORION_TOOL_PREVIEW, 0, 1, ":/images/preview.png", "", "Ԥ��", "ͨ�������Ԥ����ǰ����");
	EDITOR_MENUITEM_DEF(ORION_TOOL_DEBUG, 0, 1, ":/images/debugger.png", "", "����", "ʹ����������Ե�ǰ����");

}

void MenuManager::staticExit(){
	menuItems_.clear();
}

void MenuManager::createActions(int scope, QWidget* parent){
	QHashIterator<int, MenuItemDefine*> i(menuItems_);
	while (i.hasNext()) {
		i.next();
		MenuItemDefine* def = i.value();
		if (def->scope == scope){
			QAction* action = new QAction(QIcon(UDQ_ASCII(def->icon)),
				UDQ_TR(def->text), parent);

			action->setCheckable((bool)def->checkable);
			action->setEnabled((bool)def->enabled);

			// ���ÿ�ݼ�
			QString shortCut = QString::fromLatin1(def->shortCut);
			if (!shortCut.isEmpty())
				action->setShortcut(shortCut);
			// ��ʾ
			action->setToolTip(UDQ_TR(def->tooltip));

			actions_.insert(i.key(), action);
		}
	}
}

QAction* MenuManager::getAction(int id){
	if (actions_.contains(id)){
		return actions_[id];
	}
	return NULL;
}


static MenuManager UH5MenuManager;
MenuManager* gMenuManager = &UH5MenuManager;

