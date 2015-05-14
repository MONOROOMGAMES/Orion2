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
#ifndef ORION_KISMET_FUNCTION_H
#define ORION_KISMET_FUNCTION_H

#include "kismetdefine.h"
#include "kismetmodel.h"
#include "resourceobject.h"

using namespace ORION;


class BlueprintFunction  : public BlueprintModel{
public:
	// ʵ������
	ORION_TYPE_DECLARE(KISMET_FUNCTION);

	BlueprintFunction(QGraphicsItem * parent = 0);
	virtual ~BlueprintFunction();

	void prepare();
	virtual void init();
	// ��ȡ���ʽ
	virtual QJsonValue expNode(int dt = -1);

	BlueprintModelPort* hostPort(){
		return hostPort_;
	}
private:

};


#endif
