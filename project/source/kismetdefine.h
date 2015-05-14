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

#ifndef  ORION_KISMET_DEFINE_H
#define ORION_KISMET_DEFINE_H

#include <QImage>
#include <QColor>
#include "commondefine.h"
#include "editordefine.h"

namespace ORION{

	enum{
		KISMET_UNKNOWN = EDITOR_BLUEPRINT_BEGIN,

		KISMET_OBJECT,
		KISMET_GROUP,
		KISMET_GROUP_TEXT, 
		KISMET_COMMENT,

		KISMET_LINK ,       // ����
		KISMET_LINK_ANCHOR,       // ���Ƶ�

		KISMET_LINK_ARRAY,     // �������
		KISMET_LINK_ANCHOR_ARRAY,       // ������ӿ��Ƶ�
		KISMET_LINK_SINGLE,    // ֱ������
		KISMET_LINK_ANCHOR_SINGLE,   // ֱ�����ӿ��Ƶ�

		KISMET_MODEL,     // ʵ��
		KISMET_MODEL_PORT,       // �˿�
		KISMET_MODEL_PORT_WIDGET,  // �˿ڿؼ�
	
		KISMET_VARIABLE,  // ����
		KISMET_INSTANCE,  // ����ʵ��

		KISMET_MODEL_BUTTON,   // ��ť
		KISMET_FLOW_BRANCH,   // ��֧����
	
		KISMET_ARITHMETIC,  // ��������
		KISMET_FLOW_SEQUENCE,   // ˳��ִ��

		KISMET_COLLECTION,  // ����
		KISMET_INSTANCE_VARIABLE,  // ʵ������
		KISMET_INSTANCE_INDEX,  // ʵ������

		KISMET_FUNCTION,   //  ����

	};

	//////////////////////////////////////////////////////////////////////////
	const int KISMET_DEPTH_VARIABLE = 200; 
	const int KISMET_DEPTH_INSTANCE = 200;

	const int KISMET_DEPTH_LINKANCHOR = 150;
	const int KISMET_DEPTH_LINK = 95;
	const int KISMET_DEPTH_MODEL = 90;
	const int KISMET_DEPTH_MODELPORT = 90;

	const int KISMET_DEPTH_GROUP = 80;
	const int KISMET_DEPTH_GROUPTEXT = 80;
	const int KISMET_DEPTH_COMMENT = 81;

#define SELECT_COLOR QColor(255,115,21)

#define CONDITION_COLOR QColor(237,28,36)
#define ACTION_COLOR QColor(63,72,204)
#define EXPRESS_COLOR QColor(34,177,76)

#define CONTROL_COLOR QColor(164,73,164)
#define MATH_COLOR QColor(185,122,87)
#define INSTANCE_COLOR QColor(87,122, 185)

#define OBJECT_COLOR QColor(63,72,204)
#define BOOL_COLOR QColor(237,28,36)
#define INT_COLOR QColor(181, 230, 19)
#define FLOAT_COLOR QColor(34,101,11)
#define STRING_COLOR QColor(255,0,128)
#define TYPE_COLOR QColor(50,50,50)
#define ANY_COLOR QColor(100,100,100)
#define FAMILY_COLOR QColor(101,75,188)

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	const QString ERROR_MSG = UDQ_TR("1@%1");
	const QString WARNING_MSG = UDQ_TR("0@%1");

	////////////////////////////////////////////////////////////////////////////
	// �汾
	//////////////////////////////////////////////////////////////////////////

#define KISMET_DATA_VERSON_BASIC                                    3100        // �����汾
#define KISMET_DATA_VERSION_RELATIVECTRLPOINT           3101        //  ���߿��Ƶ��Ϊ�������

#define KISMET_DATA_VERSON        KISMET_DATA_VERSION_RELATIVECTRLPOINT 

	//////////////////////////////////////////////////////////////////////////

	//////////////////////////////////////////////////////////////////////////
	//
   #define MIME_KISMET_EVENT UDQ_T("orion/event")
   #define MIME_KISMET_VARIABLE UDQ_T("orion/variable")
}

#endif