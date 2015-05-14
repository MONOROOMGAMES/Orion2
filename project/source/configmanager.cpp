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

#include "configmanager.h"
#include <QtGui>
#include "commondefine.h"
#include "logmanager.h"
#include <QtGlobal>
using namespace ORION;

static const int Config_Version = 100;
static const QString Config_Prefix = UDQ_T("Orion2/");
static const QString Build_ID = UDQ_T("3.484");
static const QString Build_Version = UDQ_T("0.2");

static const QString Config_WinReg = UDQ_T("HKEY_CURRENT_USER\\SOFTWARE\\Unique\\Orion2");
static const QString Config_ShowLogo = UDQ_T("@showLogo");
static const QString Config_UseLocalBase = UDQ_T("@useLocalBase");
static const QString Config_DevMode = UDQ_T("@devMode");
static const QString Config_BaseAdmin = UDQ_T("@baseAdmin");
static const QString Config_BuildId = UDQ_T("@build");
static const QString Config_BuildVersion = UDQ_T("@version");

static const QString Config_BasePackagePath = UDQ_T("settings/@basepackagepath");
static const QString Config_PackagePaths = UDQ_T("settings/@packagepaths");
static const QString Config_WorkspacePath = UDQ_T("workspace/@basepath");
static const QString Config_BaseFont = UDQ_T("settings/@basefont");
static const QString Config_DefaultStyle = UDQ_T("settings/@defaultstyle");

static const QString Config_DefaultMime = UDQ_T("settings/@defaultmime");
static const QString Config_ExtendMime = UDQ_T("settings/@extendmime");

static const QString Font_YaHei = UDQ_L8("΢���ź�");
static const QString Font_Gothic = UDQ_T("Century Gothic");
static const QString Style_Default = UDQ_T("default.qss");
static const QString Project_LRU = UDQ_T("workspace/@lru");
static const QString Mime_Default = UDQ_T("mime.json");

static const QString Config_LoginCode = UDQ_T("settings/@logincode");

static const QString Config_ServerRootPath = UDQ_T("settings/@serverrootpath");
static const QString Config_ServerPort = UDQ_T("settings/@serverport");
static const QString Config_ServerCmd = UDQ_T("settings/@servercmd");

static const QString Config_WinState = UDQ_T("settings/@windowstate");
static const QString Config_WinGeometry = UDQ_T("settings/@windowgeometry");

static int Project_LRUNum = 100;
static QStringList keyboard_Names;
static QMap<QString, int> Keyboard_Map;

void ConfigManager::setGlobalSettings(const QString& id, bool value){
#if defined(Q_OS_WIN)
	QSettings settings(Config_WinReg, QSettings::NativeFormat);
	QString existValue = settings.value(id).toString();
	settings.setValue(id, value ? 1 : 0);
#else
	QSettings settings;
	QString existValue = settings.value(Config_Prefix + id).toString();
	settings.setValue(Config_Prefix + id, value ? 1 : 0);
#endif
}

void ConfigManager::setShowLogo( bool show ){
	setGlobalSettings(Config_ShowLogo, show);
}

void ConfigManager::setUseLocalBase(bool use){
	setGlobalSettings(Config_UseLocalBase, use);
}

void ConfigManager::setDevMode(bool use){
	setGlobalSettings(Config_DevMode, use);
}

void ConfigManager::staticInit(){

    setShowLogo(false);
	setUseLocalBase(false);

#if defined( DEBUG ) || defined( _DEBUG )
	setDevMode(true); 
#else
	setDevMode(false);  // releaseģʽ�¸�Ϊfalse
#endif
	
	QString appPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());

    QSettings settings;
	// ����д��Base��
	settings.setValue(Config_Prefix + Config_BaseAdmin, 1);

	// ���ð汾
    int version = settings.value(Config_Prefix + Config_BuildVersion).toInt();

    if( version != Config_Version ){
        settings.setValue(Config_Prefix + Config_BuildVersion, Config_Version );
    }

    QString buildver = settings.value(Config_Prefix + Config_BuildId).toString();
    if( buildver != Build_Version ){
        settings.setValue(Config_Prefix + Config_BuildId, QString(UDQ_T("%1.%2")).arg(Build_Version).arg(Build_ID) );
    }

	QString basePath = settings.value(Config_Prefix + Config_BasePackagePath).toString();
	if ( useLocalBase() || basePath.isEmpty() ){ 	// ʹ�ñ���Base��
		basePath = QString(UDQ_T("Base@%1/Data/Base")).arg(appPath);
		settings.setValue(Config_Prefix + Config_BasePackagePath, basePath);
	}
	
	//   ����ȱʡ��·��
	QStringList packagePahts = settings.value(Config_Prefix + Config_PackagePaths).toStringList();
	if (packagePahts.size() == 0){
		// ��ӻ�����·��
		packagePahts.append(QString(UDQ_T("%1/Data/User")).arg(appPath));
		settings.setValue(Config_Prefix + Config_PackagePaths, packagePahts);
	}

    QDir dir;
    // ��ȡ�����ռ����·��
    QString wsPath = settings.value( QString( Config_Prefix + Config_WorkspacePath )).toString();
    if( wsPath.isEmpty() ){
		QString basewsPath = QString(UDQ_T("%1/Workspace")).arg(appPath);
        settings.setValue(Config_Prefix + Config_WorkspacePath, basewsPath );
    }
    // ȷ�������ռ�Ŀ¼����
    if( !dir.exists ( wsPath ) )
        dir.mkpath( wsPath );

    QString baseFont = settings.value(Config_Prefix + Config_BaseFont).toString();
    if( baseFont.isEmpty()  ){
        settings.setValue(Config_Prefix +Config_BaseFont, Font_YaHei);
    }

	// ȱʡ��ʽ
	QString defStyle = settings.value(Config_Prefix + Config_DefaultStyle).toString();
	if (defStyle.isEmpty()){
		settings.setValue(Config_Prefix + Config_DefaultStyle, Style_Default);
	}

	// MIME
	QString defMime = settings.value(Config_Prefix + Config_DefaultMime).toString();
	if (defMime.isEmpty()){
		settings.setValue(Config_Prefix + Config_DefaultMime, Mime_Default);
	}

	//  ������չMIME
	QString extMime = settings.value(Config_Prefix + Config_ExtendMime).toString();
	if (extMime.isEmpty()){
		settings.setValue(Config_Prefix + Config_ExtendMime, 1);
	}


	// ����ӳ��
	QString tmp;
#define KEYTOSTRING(x) {tmp = UDQ_T(#x); tmp = tmp.right(tmp.size() - 8); keyboard_Names.append(tmp);Keyboard_Map.insert(tmp, x);}\


	KEYTOSTRING(Qt::Key_Escape);
	KEYTOSTRING(Qt::Key_Tab);
	KEYTOSTRING(Qt::Key_Backtab);
	KEYTOSTRING(Qt::Key_Backspace);
	KEYTOSTRING(Qt::Key_Return);
	KEYTOSTRING(Qt::Key_Enter);
	KEYTOSTRING(Qt::Key_Insert);
	KEYTOSTRING(Qt::Key_Delete);
	KEYTOSTRING(Qt::Key_Pause);
	KEYTOSTRING(Qt::Key_Print);
	KEYTOSTRING(Qt::Key_SysReq);
	KEYTOSTRING(Qt::Key_Clear);
	KEYTOSTRING(Qt::Key_Home);
	KEYTOSTRING(Qt::Key_End);
	KEYTOSTRING(Qt::Key_Left);
	KEYTOSTRING(Qt::Key_Up);
	KEYTOSTRING(Qt::Key_Right);
	KEYTOSTRING(Qt::Key_Down);
	KEYTOSTRING(Qt::Key_PageUp);
	KEYTOSTRING(Qt::Key_PageDown);
	KEYTOSTRING(Qt::Key_Shift);
	KEYTOSTRING(Qt::Key_Meta);
	KEYTOSTRING(Qt::Key_Alt);
	KEYTOSTRING(Qt::Key_CapsLock);
	KEYTOSTRING(Qt::Key_NumLock);
	KEYTOSTRING(Qt::Key_ScrollLock);
	KEYTOSTRING(Qt::Key_F1);
	KEYTOSTRING(Qt::Key_F3);
	KEYTOSTRING(Qt::Key_F4);
	KEYTOSTRING(Qt::Key_F5);
	KEYTOSTRING(Qt::Key_F6);
	KEYTOSTRING(Qt::Key_F7);
	KEYTOSTRING(Qt::Key_F8);
	KEYTOSTRING(Qt::Key_F9);
	KEYTOSTRING(Qt::Key_F10);
	KEYTOSTRING(Qt::Key_F11);
	KEYTOSTRING(Qt::Key_F12);
	KEYTOSTRING(Qt::Key_Space);
	KEYTOSTRING(Qt::Key_Exclam);
	KEYTOSTRING(Qt::Key_QuoteDbl);
	KEYTOSTRING(Qt::Key_NumberSign);
	KEYTOSTRING(Qt::Key_Dollar);
	KEYTOSTRING(Qt::Key_Percent);
	KEYTOSTRING(Qt::Key_Ampersand);
	KEYTOSTRING(Qt::Key_Apostrophe);
	KEYTOSTRING(Qt::Key_ParenLeft);
	KEYTOSTRING(Qt::Key_ParenRight);
	KEYTOSTRING(Qt::Key_Asterisk);
	KEYTOSTRING(Qt::Key_Plus);
	KEYTOSTRING(Qt::Key_Comma);
	KEYTOSTRING(Qt::Key_Minus);
	KEYTOSTRING(Qt::Key_Period);
	KEYTOSTRING(Qt::Key_Slash);
	KEYTOSTRING(Qt::Key_0);
	KEYTOSTRING(Qt::Key_1);
	KEYTOSTRING(Qt::Key_2);
	KEYTOSTRING(Qt::Key_3);
	KEYTOSTRING(Qt::Key_4);
	KEYTOSTRING(Qt::Key_5);
	KEYTOSTRING(Qt::Key_6);
	KEYTOSTRING(Qt::Key_7);
	KEYTOSTRING(Qt::Key_8);
	KEYTOSTRING(Qt::Key_9);
	KEYTOSTRING(Qt::Key_Colon);
	KEYTOSTRING(Qt::Key_Semicolon);
	KEYTOSTRING(Qt::Key_Less);
	KEYTOSTRING(Qt::Key_Equal);
	KEYTOSTRING(Qt::Key_Greater);
	KEYTOSTRING(Qt::Key_Question);
	KEYTOSTRING(Qt::Key_At);
	KEYTOSTRING(Qt::Key_A);
	KEYTOSTRING(Qt::Key_B);
	KEYTOSTRING(Qt::Key_C);
	KEYTOSTRING(Qt::Key_D);
	KEYTOSTRING(Qt::Key_E);
	KEYTOSTRING(Qt::Key_F);
	KEYTOSTRING(Qt::Key_G);
	KEYTOSTRING(Qt::Key_H);
	KEYTOSTRING(Qt::Key_I);
	KEYTOSTRING(Qt::Key_J);
	KEYTOSTRING(Qt::Key_K);
	KEYTOSTRING(Qt::Key_L);
	KEYTOSTRING(Qt::Key_M);
	KEYTOSTRING(Qt::Key_N);
	KEYTOSTRING(Qt::Key_O);
	KEYTOSTRING(Qt::Key_P);
	KEYTOSTRING(Qt::Key_Q);
	KEYTOSTRING(Qt::Key_R);
	KEYTOSTRING(Qt::Key_S);
	KEYTOSTRING(Qt::Key_T);
	KEYTOSTRING(Qt::Key_U);
	KEYTOSTRING(Qt::Key_V);
	KEYTOSTRING(Qt::Key_W);
	KEYTOSTRING(Qt::Key_X);
	KEYTOSTRING(Qt::Key_Y);
	KEYTOSTRING(Qt::Key_Z);

#undef KEYTOSTRING

	gLogManager->log(UDQ_TR("ConfigManager��ʼ�����"));
}

bool ConfigManager::globalSettings(const QString& id){
#if defined(Q_OS_WIN)
	QSettings settings(Config_WinReg, QSettings::NativeFormat);
	return settings.value(id).toBool();
#else
	QSettings settings;
	return settings.value(Config_Prefix + id).toBool();
#endif
}

//  �Ƿ���ʾ��˾LOGO
bool ConfigManager::showLogo(){
	return globalSettings(Config_ShowLogo);
}

bool ConfigManager::useLocalBase(){
	return globalSettings(Config_UseLocalBase);
}

bool ConfigManager::devMode(){
	return globalSettings(Config_DevMode);
}

bool ConfigManager::isBaseAdmin(){
	QSettings settings;
	return settings.value(Config_Prefix + Config_BaseAdmin).toBool();
}

QString ConfigManager::buildVersion(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_BuildId).toString();
}

QString buildId(){
    return Build_ID;
}

QString ConfigManager::baseWorkspacePath(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_WorkspacePath).toString();
}

void ConfigManager::setWorkspaceConfig(const QString& path){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_WorkspacePath), path);
}

QString ConfigManager::baseFont(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_BaseFont).toString();
}

void ConfigManager::setBaseFont(const QString& ft){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_BaseFont), ft);
}

QString ConfigManager::basePackagePath(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_BasePackagePath).toString();
}

QStringList ConfigManager::packagePaths(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_PackagePaths).toStringList();
}

void ConfigManager::setPackagePaths(const QStringList& paths){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_PackagePaths), paths);
}

QString ConfigManager::defaultStyle(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_DefaultStyle).toString();
}

void ConfigManager::setDefaultStyle(const QString& style){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_DefaultStyle), style);
}

QString ConfigManager::defaultMimeJson(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_DefaultMime).toString();
}

void ConfigManager::setMimeJson(const QString& json){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_DefaultMime), json);
}

// �Ƿ�������չMIME
bool ConfigManager::useExtentedMime(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_ExtendMime).toBool();
}

void ConfigManager::enableExtentedMime(bool flag){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_ExtendMime), flag ? 1 : 0);
}

QByteArray ConfigManager::loginCode(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_LoginCode).toByteArray();
}

void ConfigManager::setLoginCode(const QByteArray& code){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_LoginCode), code);
}


QStringList ConfigManager::projectLRU(){
	QSettings setting;
	return setting.value(Config_Prefix + Project_LRU).toStringList();
}

void ConfigManager::setProjectLRU(const QStringList& lru){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Project_LRU), lru);
}

// �����Ŀ
void ConfigManager::addProject(const QString& name){
	QSettings setting;
	QStringList lru = setting.value(Config_Prefix + Project_LRU).toStringList();

	lru.removeOne(name);
	lru.push_front(name);
	if (lru.size() > Project_LRUNum){
		lru.pop_back();
	}

	setting.setValue(QString(Config_Prefix + Project_LRU), lru);
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

QStringList& ConfigManager::keyboardMap(){
	return keyboard_Names;
}

int ConfigManager::keyboardValue(const QString& key){
	return Keyboard_Map.value(key);
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

// ��������
QString ConfigManager::serverRootPath(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_ServerRootPath).toString();
}

void ConfigManager::setServerRootPath(const QString& path){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_ServerRootPath), path);
}

// �����
int ConfigManager::serverPort(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_ServerPort).toInt();
}

void ConfigManager::setServerPort(int port){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_ServerPort), port);
}

// �����������
QString ConfigManager::explorerCmd(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_ServerCmd).toString();
}

void ConfigManager::setExplorerCmd(const QString& cmd){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_ServerCmd), cmd);
}

//////////////////////////////////////////////////////////////////////////
//

void ConfigManager::saveWindowState(const QByteArray& data){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_WinState), data);
}

QByteArray ConfigManager::windowState(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_WinState).toByteArray();
}

void ConfigManager::saveWindowGeometry(const QByteArray& data){
	QSettings setting;
	setting.setValue(QString(Config_Prefix + Config_WinGeometry), data);
}

QByteArray ConfigManager::windowGeometry(){
	QSettings setting;
	return setting.value(Config_Prefix + Config_WinGeometry).toByteArray();
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

static ConfigManager OrionConfigManager;
ConfigManager* gConfigManager = &OrionConfigManager;


