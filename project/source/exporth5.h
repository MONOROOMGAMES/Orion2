/************************************************************************
**
**  Copyright (C) 2015  guobin  <guobin.gb@alibaba-inc.com>
**
**  This file is part of Orion2.
**
**  Orion is free software: you can redistribute it and/or modify
**  it under the terms of the GNU General Public License as published by
**  the Free Software Foundation.
**
**  Orion2 is distributed in the hope that it will be useful,
**  but WITHOUT ANY WARRANTY; without even the implied warranty of
**  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
**  GNU General Public License for more details.
**
**  You should have received a copy of the GNU General Public License
**  along with Orion2.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/

#ifndef ORION_EXPORTH5_H
#define ORION_EXPORTH5_H


#include "exporter.h"
#include "resourceobject.h"
#include "resourcelinker.h"
using namespace ORION;


namespace ORION{

	// Native�ļ��б�
	enum{
		FCT_INDEX,
		FCT_APPMANIFEST,
		FCT_APPCACHE,
		FCT_RUNTIME,
		FCT_JQUERY,

		FCT_ICON16,
		FCT_ICON32,
		FCT_ICON128,
		FCT_ICON256,
		FCT_ICONLOADING,

		FCT_INDEXNW,
		FCT_INDEXCORDOVA,

		FCT_INDEXDEBUG,
		FCT_RUNTIMEDEBUG,
		FCT_DEBUGGER,
		FCT_DEBUGGER_CSS,
		FCT_DEBUGGER_COLLAPSED,
		FCT_DEBUGGER_DEL,
		FCT_DEBUGGER_EXPANEDED,
		FCT_DEBUGGER_INSPECT,
		FCT_DEBUGGER_WATCH,

	};

}

class HTML5Exporter : public Exporter
{
	Q_OBJECT
public:
	HTML5Exporter(ProjectResourceObject* proj);
	virtual ~HTML5Exporter();

    virtual void write(int mode = 0);
	virtual QString name();

protected:
	virtual void makeTemplateDB(const QString& file, const QString& src, bool afterDelete = false);

private:

	// ����ͼƬ( ����ͼƬ�ļ���) usestyle �������ļ����б�עv��h��˵��ͼƬ����
	QString saveImageFile(ResourceHash hash, const QString& desPath);
	QString saveEmptyImageFile(const QString& desPath);
	QString saveAudioFile(ResourceHash hash, const QString& desPath );
	QString saveVideoFile(ResourceHash hash, const QString& desPath);

	//�����ļ�
	void generateFile(int type,  const QString& filePath, bool text = true, bool cache = true );
	QByteArray replaceTextContent(const QByteArray& code, int	encodeType);


	// ׼������
	void prepare();
	void processModel(const QByteArray& data);
	
	void prepareData();

	// ����runtime.js�ļ�
	void makeRuntimeFile();
	// ���������ļ�
	void makeDataFile();
	QJsonArray generatePlugins();	// ���ɲ�������б�
	QJsonArray generateObjectTypes();	// ���ɶ��������б�
	QJsonArray generateVariables( ObjectTypeResourceObject* obj);	// ����ʵ���б�
	QJsonArray generateAnimations(ObjectTypeResourceObject* obj);	// ���ɶ����б�
	QJsonArray generateBehaviors(ObjectTypeResourceObject* obj);	// ������Ϊ�б�
	QJsonArray generateTexture(ObjectTypeResourceObject* obj);	// ������������
	QJsonArray generateBehaviors(FamilyResourceObject* obj);	// ������Ϊ�б�

	QJsonArray generateFamilies();  // ���ɼ����б�
	QJsonArray generateLevels(); // ���ɳ����б�
	QJsonArray generateLayer(LayerResourceObject* layer);  // ����ͼ��
	// �������ʵ��
	QJsonArray processInstance(const QByteArray& data);
	// ����ʵ������
	QJsonValue processInstanceVariable(PluginVariable* var, ObjectTypeResourceObject* objType, QMap<ResourceHash, QString>& parameters);
	// �������
	QJsonArray processPluginProperty(const QString& name, ObjectTypeResourceObject* objType, QMap<ResourceHash, QString>& parameters);
	// Tilemap����
	QJsonArray processTileMap(int w, int h, QMap<QPair<int, int>, QSize>& data);

	QString state2String(int state);
	QString color2String(const QColor& clr);
	QString font2String(const QFont& fnt);
	int pixelFormat(QImage& img);

	QJsonArray generateBlueprints(); // ���ɽű��б�
	void replaceBlock(QJsonArray& block);
	void replaceParam(QJsonArray& param);
	void replaceExpNode(QJsonArray& param);
	void replaceInstanceIndex(QJsonArray& param);

	// ������Դ�ļ�
	void makeResourceFile();
	// ����ͼ���ļ�
	void makeIconFile();
	// ���ɹ���JS�ļ�
	void makeCommonScriptFile();

	// ������Ч�������
	QString generateEffectCode(PluginResourceObject* effect);
	QString processFxCode(QByteArray& data);

	// ���ɲ������
	QString generatePluginCode(PluginResourceObject* plugin);
	QString processScriptCode(QByteArray& data);

	ProjectResourceObject* project_;  // ����

	// �ڲ�ʹ��
	QString exportPath_;  // ��������·��
	QString dbPath_;  // ���ݿ��ļ�·��
	QStringList cacheFiles_;  // �����ļ��嵥

	 int groupRef_;
	bool hasGroup_;  // �Ƿ������
	bool hasElse_;   // �Ƿ����Elseģ��

	QList<PluginResourceObject*> entities_;
	QList<PluginResourceObject*> behaviors_;
	QList<PluginResourceObject*> effects_;

	QSet<ResourceHash>  galleries_;  // ������Դ
	QSet<ObjectTypeResourceObject*>  emptyGalleries_;  // �����վ�����Դ
	int emptysize_; 

	QSet<SpriteResourceObject*> sprites_;  // ������Դ
	QSet<SoundResourceObject*> sounds;  // ������Դ
	QSet<MovieResourceObject*> movies_;  // ��Ӱ��Դ

	int nextUID_;  // ʵ��ID
	QStringList objectRefs_;
	QMap<ResourceHash, QString> resourceMap_;  // ��Դ����ӳ�䣨���·���ļ�����
	QMap<ResourceHash, int>  objectRefMap_;  // ��������ӳ��
	QMap<ResourceHash, int>  objectTypeMap_;  // ������������ӳ��
	QMap<ResourceHash, int> objectVariableMap_; // ����ʵ����������ӳ��


	//////////////////////////////////////////////////////////////////////////
	int mode_;   //  ����ģʽ�� 0��ʾH5�� 1��ʾCordova,  2��ʾnw
};

#endif // ORION_EXPORTEPUB_H

