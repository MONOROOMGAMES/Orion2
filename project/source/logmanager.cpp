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
**  Orion2 is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Orion.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#include "logmanager.h"
#include <QtGui>
#include <QApplication>
#include <QtGlobal>
#include <QJsonDocument>
#include "configmanager.h"
#include <QFile>
#include <QDir>

static QString glogFile;  // ��־�ļ�·��

void LogManager::staticInit(){
	// ������־�ļ�
	glogFile = QApplication::applicationDirPath() + UDQ_T("/orionlog.txt");

	if (QFile::exists(glogFile)){
		// �����ļ�
		QString bakpath = QApplication::applicationDirPath() + UDQ_T("/orionlog.bak.txt");
		QFile::copy(glogFile, bakpath);
	}
	else{
		// �����ļ�
		QFile f(glogFile);
		if (f.open(QIODevice::WriteOnly | QIODevice::Text)){
			QString datestr = QDateTime::currentDateTime().toString();
			QTextStream out(&f);
			out << UDQ_T("*********************************************************************************************************\n");
			out << UDQ_TR("Orion2 ��־��¼ \n");
			out << UDQ_T("*********************************************************************************************************\n");
			out << UDQ_TR("����ʱ��:  ") + datestr + UDQ_T("\n");
			out << UDQ_T("*********************************************************************************************************\n\n");
			f.close();
		}
	}
}

void LogManager::log(const QString& text){
	QString datestr = QDateTime::currentDateTime().toString(UDQ_T("yyyy-MM-dd hh:mm:ss"));
	QFile f(glogFile);
	if (f.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Append )){
		QTextStream out(&f);
		out << datestr + UDQ_T(":  ") + text + UDQ_T("\n");
		f.close();
	}
}

void LogManager::logError(const QString& err){
	QString text = UDQ_TR("****����**** ") + err;
	log(text);
}

void LogManager::logWarning(const QString& err){
	QString text = UDQ_TR("[ ���� ] ") + err;
	log(text);
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

static LogManager OrionLogManager;
LogManager* gLogManager = &OrionLogManager;


