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
#include <QApplication>
#include <QDateTime>

#include <string.h>
#include <zip.h>
#ifdef _WIN32
#include <iowin32.h>
#endif


#include <QDir>
#include <QDirIterator>
#include <QFile>
#include <QFileInfo>
#include <QTemporaryFile>
#include <QTextStream>
#include "exporth5.h"
#include "tooldefine.h"
#include "leveldefine.h"
#include "componentdefine.h"
#include "mimemanager.h"
#include "kismetdefine.h"
#include "logmanager.h"
#include "kismetobject.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
using namespace ORION;

enum{
	ECT_LATIN1,
	ECT_UTF8,
	ECT_LOCAL8BIT,
};


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////


HTML5Exporter::HTML5Exporter(ProjectResourceObject* proj)
    :project_(proj)
{
	dbPath_ = QString(UDQ_T("%1/Exporter/%2.db")).arg(QApplication::applicationDirPath()).arg(name());
	exportPath_ = project_->exportDir_ + UDQ_T("/") + project_->objectName();

}


HTML5Exporter::~HTML5Exporter()
{
}

QString HTML5Exporter::name(){
	return UDQ_T("html5");
}

void HTML5Exporter::write(int mode)
{
	mode_ = mode;
	// �޻�������

	// ȷ������Ŀ¼����
	QDir dir(exportPath_);
	if (!dir.exists()){
		dir.mkpath(exportPath_);
	}
	else{
		// ���Ŀ¼
		deleteDir(exportPath_, false);
	}

	// ����ǰ׼������
	prepare();
	emit progress(10);
	// ����runtime�ļ�
	makeRuntimeFile();
	emit progress(30);
	// ������Դ�ļ�
	makeResourceFile();
	emit progress(50);
	// ���ɹ��������ļ�
	makeDataFile();
	emit progress(70);
	// ����ͼ���ļ�
	makeIconFile();
	emit progress(80);
	// ���ɹ����ű��ļ�
	makeCommonScriptFile();
	emit progress(90);
}

void  HTML5Exporter::generateFile(int type, const QString& desPath, bool text, bool cache){

	QByteArray code;
	// ��DB�м���ģ��
	loadTextFromDB(dbPath_, type, code);

	// �����滻
	if (text){
		code = replaceTextContent(code, ECT_UTF8);
		saveTextToDisk(desPath, code);
	}
	else{
		saveDataToDisk(desPath, code);
	}

	if (cache){
		QDir dir(exportPath_);
		QString relpath = dir.relativeFilePath(desPath);
		if (relpath.startsWith(UDQ_T("/")) || relpath.startsWith(UDQ_T("\\"))){
			relpath = relpath.mid(1);
		}
		cacheFiles_.append(relpath);
	}

}



const QString OBJECTREF_BEGIN = UDQ_T("cr.getObjectRefTable = function() {return[\n");
const QString OBJECTREF_END = UDQ_T("];};\n");


// �滻����
QByteArray HTML5Exporter::replaceTextContent(const QByteArray& code, int	encodeType){

	QString codeTxt;
	QString tmp;
	switch (encodeType){
	case ECT_LATIN1:
		codeTxt = QString::fromLatin1(code.data(), code.size());
		break;
	case ECT_UTF8:
		codeTxt = QString::fromUtf8(code.data(), code.size());
		break;
	case ECT_LOCAL8BIT:
		codeTxt = QString::fromLocal8Bit(code.data(), code.size());
		break;
	}

	//�滻����
	// %ORION_NAME% 
	QString name = project_->getExportName();
	codeTxt.replace(UDQ_T("%ORION_NAME%"), name);

	// %ORION_WIDTH%
	QString width = QString::number(project_->resolution_.width());
	codeTxt.replace(UDQ_T("%ORION_WIDTH%"), width);

	// %ORION_HEIGHT%
	QString height = QString::number(project_->resolution_.height());
	codeTxt.replace(UDQ_T("%ORION_HEIGHT%"), height);

	// %ORION_ORIENTATION% 
	QString orient = UDQ_T("any");
	if (project_->orientation_ == ORIENT_PORTRAIT){
		orient = UDQ_T("portrait");
	}
	else if (project_->orientation_ == ORIENT_LANDSCAPE){
		orient = UDQ_T("landscape");
	}
	codeTxt.replace(UDQ_T("%ORION_ORIENTATION%"), orient);

	//  %ORION_TIMESTAMP% 
	QString timestamp = QDateTime::currentDateTime().toString(UDQ_T("yyyyMMddhhmmss"));
	codeTxt.replace(UDQ_T("%ORION_TIMESTAMP%"), timestamp);


	// %ORION_FILELIST%  �ļ��嵥�б�
	int index = codeTxt.indexOf(UDQ_T("%ORION_FILELIST%"));
	if (index >= 0){
		codeTxt.replace(UDQ_T("%ORION_FILELIST%"), UDQ_T(""));
		// ȥ���ظ���
		cacheFiles_.removeDuplicates();
		
		foreach(QString item, cacheFiles_){
			tmp = item + UDQ_T("\n");
			codeTxt.insert(index, tmp);
			index += tmp.size();
		}
	}

	//%ORION_OBJECTREFS%  ���������б�
	index = codeTxt.indexOf(UDQ_T("%ORION_OBJECTREFS%"));
	if (index >= 0){
		codeTxt.replace(UDQ_T("%ORION_OBJECTREFS%"), OBJECTREF_BEGIN);
		index += OBJECTREF_BEGIN.size();
		for (int i = 0; i < objectRefs_.size(); i++){
			tmp = UDQ_T("\t") + objectRefs_[i];
			if (i < objectRefs_.size() - 1){
				tmp += UDQ_T(",");
			}
			tmp += UDQ_T("\n");
			codeTxt.insert(index, tmp);
			index += tmp.size();
		}
		codeTxt.insert(index, OBJECTREF_END);
	}

	//%ORION_EFFECTS%  ��Ч
	index = codeTxt.indexOf(UDQ_T("%ORION_EFFECTS%"));
	if (index >= 0){
		codeTxt.replace(UDQ_T("%ORION_EFFECTS%"), UDQ_T(""));
		foreach(PluginResourceObject* plugin, effects_){
			tmp = generateEffectCode(plugin);
			codeTxt.insert(index, tmp);
			index += tmp.size();
		}
	}

	//%ORION_ENTITIES%  ʵ��
	index = codeTxt.indexOf(UDQ_T("%ORION_ENTITIES%"));
	if (index >= 0){
		codeTxt.replace(UDQ_T("%ORION_ENTITIES%"), UDQ_T(""));
		foreach(PluginResourceObject* plugin, entities_){
			tmp = generatePluginCode(plugin);
			codeTxt.insert(index, tmp);
			index += tmp.size();
		}
	}

	//%ORION_BEHAVIORS%  ��Ϊ
	index = codeTxt.indexOf(UDQ_T("%ORION_BEHAVIORS%"));
	if (index >= 0){
		codeTxt.replace(UDQ_T("%ORION_BEHAVIORS%"), UDQ_T(""));
		foreach(PluginResourceObject* plugin, behaviors_){
			tmp = generatePluginCode(plugin);
			codeTxt.insert(index, tmp);
			index += tmp.size();
		}
	}

	if (encodeType == ECT_LOCAL8BIT){
		return codeTxt.toLocal8Bit();
	}
	else if (encodeType == ECT_LATIN1){
		return codeTxt.toLatin1();
	}

	return codeTxt.toUtf8();

}


QString HTML5Exporter::processFxCode(QByteArray& data){
	static QString lineTmpl = UDQ_T("\"%1\",");

	if (data.isEmpty()){
		return QString();
	}

	QByteArray filecontent;
	{
		QString filename;
		QString datetime;
		bool isText;
		QDataStream stream(data);
		stream >> filename >> datetime >> isText >> filecontent;
	}

	if (filecontent.isEmpty()){
		return QString();
	}

	QString codeTxt = QString::fromUtf8(filecontent.data(), filecontent.size());
	// ���ݻس��ָ�Ϊ�б�
	QStringList lines = codeTxt.split(UDQ_T("\n"), QString::SkipEmptyParts);

	codeTxt.clear();
	QString tmp;

	for (int i = 0; i < lines.size(); i++){
		tmp = lines[i].simplified();
		if (tmp.isEmpty() || tmp.startsWith(UDQ_T("//"))){  // ��֧�� // ע��
			continue;
		}
		codeTxt.append(lineTmpl.arg(tmp));
		if (i ==  lines.size() - 1){
			codeTxt.chop(1); // ȥ��ĩβ�Ķ���
		}
		codeTxt.append(UDQ_T("\n"));
	}

	return codeTxt;
}

QString bool2String(bool v){
	return v ? UDQ_T("true") : UDQ_T("false");
}

QString HTML5Exporter::generateEffectCode(PluginResourceObject* effect){
	static QString shaderTmpl = UDQ_T("cr.shaders[\"%1\"] = {src: [%2].join(\"\\n\"),\n%3\n\tparameters: [%4] };\n");
	static QString paramTmpl = UDQ_T("[\"%1\", 0, 1],");

	QString shaderCode;
	QList<QByteArray> values = effect->filedata_.values(FIT_FX);
	Q_ASSERT(values.size() <= 1);

	if (values.size() == 1 ){
		shaderCode = processFxCode(values[0]);
	}

	// ��������ֵ
	QString props;
	props.append(UDQ_T("\textendBoxHorizontal: ") + QString::number(effect->extendBound_.width()) + UDQ_T(",\n"));
	props.append(UDQ_T("\textendBoxVertical: ") + QString::number(effect->extendBound_.height() )+ UDQ_T(",\n"));
	props.append(UDQ_T("\tcrossSampling: ") + bool2String(effect->hasAttribute(EF_CROSSSAMPLE)) + UDQ_T(",\n"));
	props.append(UDQ_T("\tanimated: ") + bool2String(effect->hasAttribute(EF_ANIMATIONDRAW)) + UDQ_T(","));

	// ���ɲ����б�
	QString params;
	foreach(PluginVariable* v, effect->variables_.values()){
		params.append(paramTmpl.arg(v->name));
	}
	if (params.size() > 0){
		params.chop(1);  // ȥ��ĩβ����
	}

	return shaderTmpl.arg(effect->objectName())
		.arg(shaderCode)
		.arg(props)
		.arg(params);
}


QString HTML5Exporter::processScriptCode(QByteArray& data){
	static QString lineTmpl = UDQ_T("\"%1\",");

	if (data.isEmpty()){
		return QString();
	}

	QByteArray filecontent;
	{
		QString filename;
		QString datetime;
		bool isText;
		QDataStream stream(data);
		stream >> filename >> datetime >> isText >> filecontent;
	}

	if (filecontent.isEmpty()){
		return QString();
	}

	QString codeTxt = QString::fromUtf8(filecontent.data(), filecontent.size());

	// ���ݻس��ָ�Ϊ�б�
	QStringList lines = codeTxt.split(UDQ_T("\n"), QString::SkipEmptyParts);

	codeTxt.clear();
	QString tmp;

	for (int i = 0; i < lines.size(); i++){
		tmp = lines[i].simplified();
		if ( tmp.isEmpty() || tmp.startsWith(UDQ_T("//")) || tmp.startsWith(UDQ_T("assert"))  || tmp.contains(UDQ_T("use strict"))){  // ��֧�� // ע��
			continue;
		}
		codeTxt.append(lines[i]);
		codeTxt.append(UDQ_T("\n"));
	}

	return codeTxt;
}



QString HTML5Exporter::generatePluginCode(PluginResourceObject* plugin){
	
	QString shaderCode;
	QList<QByteArray> values = plugin->filedata_.values(FIT_RUNTIME);
	Q_ASSERT(values.size() <= 1);

	if (values.size() == 1){
		shaderCode = processScriptCode(values[0]);
	}

	return shaderCode;
}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////
const QString NAME_COMMON = UDQ_TR("Common");
const QString NAME_SYSTEM = UDQ_TR("System");
const QString NAME_GROUPACTIVE = UDQ_T("cr.system_object.prototype.cnds.IsGroupActive");
const QString NAME_ELSE = UDQ_T("cr.system_object.prototype.cnds.Else");

void HTML5Exporter::prepare(){
	objectRefs_.clear();
	entities_.clear();
	behaviors_.clear();
	effects_.clear();
	objectRefMap_.clear();
	resourceMap_.clear();
	emptyGalleries_.clear();
	nextUID_ = 0;

	// ���ҹ������
	PluginResourceObject* commonPlugin = PluginResourceObject::acquireCommonPlugin();

	// �������б�
	foreach(ObjectTypeResourceObject* objtype, project_->objTypes_){
		if (objtype->flags() & URF_TagGarbage)
			continue;

		Q_ASSERT(objtype->prototype_ != NULL);
		if (!entities_.contains(objtype->prototype_)){
			entities_.append(objtype->prototype_);
		}


		foreach(PluginResourceObject* plugin, objtype->attachments_.values()){
			if (plugin->type_ == PIT_BEHAVIOR){
				if (!behaviors_.contains(plugin)){
					behaviors_.append(plugin);
				}
			}
			else if (plugin->type_ == PIT_EFFECT){
				if (!effects_.contains(plugin)){
					effects_.append(plugin);
				}
			}
		}

		// ���Ҿ����ͼ����Դ
		if (objtype->animation_){
			sprites_.insert(objtype->animation_);
			// ��Ӷ����е������ز�
			foreach(ResourceHash key, objtype->animation_->imgSources_.values()){
				galleries_.insert(key);
			}
		}

		if (objtype->staticImage_){
			galleries_.insert(objtype->staticImage_->hashKey());
		}

		// ���û�о�����Դ
		if ( objtype->prototype_->hasAttribute(EF_ANIMATIONS) && objtype->animation_ == NULL && objtype->staticImage_ == NULL){
			//����ȱʡ͸��ͼƬ
			emptyGalleries_.insert(objtype);
		}
		else if ( objtype->prototype_->hasAttribute(EF_TEXTURE) &&  objtype->staticImage_ == NULL){  // û������ͼƬ
			emptyGalleries_.insert(objtype);
		}

	}

	foreach(PluginResourceObject* plugin, entities_){
		objectRefMap_.insert(plugin->hashKey(), objectRefs_.size());
		objectRefs_.append(plugin->runtimeString());
	}

	foreach(PluginResourceObject* plugin, behaviors_){
		objectRefMap_.insert(plugin->hashKey(), objectRefs_.size());
		objectRefs_.append(plugin->runtimeString());
	}

	groupRef_ = -1;
	hasGroup_ = false;
	hasElse_ = true;
	// �����ű�����������API�ӿڡ�����Ƶ��Դ
	foreach(BlueprintResourceObject* blueprint, project_->blueprints_){
		if (blueprint->flags() & URF_TagGarbage)
			continue;
		// ���ģ��
		foreach(QByteArray arr, blueprint->events_){
			processModel(arr);
		}
		if (!hasGroup_ && blueprint->groups_.size() > 0){
			hasGroup_ = true;
		}
	}

	if (hasGroup_){  // ����麯���ӿ�
		groupRef_ = objectRefs_.size();
		objectRefMap_.insert(hashString(NAME_GROUPACTIVE), objectRefs_.size());
		objectRefs_.append(NAME_GROUPACTIVE);
	}

	if (hasElse_){
		groupRef_ = objectRefs_.size();
		objectRefMap_.insert(hashString(NAME_ELSE), objectRefs_.size());
		objectRefs_.append(NAME_ELSE);
	}

}

void HTML5Exporter::processModel(const QByteArray& data){
	QDataStream stream(data);
	int type;
	stream >> type;
	if (type == KISMET_MODEL){
		unsigned int ver;
		ResourceHash iid;
		int flags;
		QPointF pos;
		ResourceHash key;
		ResourceHash interfaceID;

		stream >>ver >> iid >> flags >> pos >> key >> type >> interfaceID;

		DEFINE_RESOURCE_HASHKEY_VALID(PluginResourceObject, plugin, key);
		PluginInterface* inter = NULL;
		if (type == PT_CONDITION){
			inter = plugin->getCondition(interfaceID);
		}
		else if (type == PT_ACTION){
			inter = plugin->getAction(interfaceID);
		}
		else if (type == PT_EXPRESS){
			inter = plugin->getExpress(interfaceID);
		}

		// ���û���ҵ��ӿڣ���˵���ű�ģ��������
		if (inter == NULL){
			gLogManager->logError(UDQ_TR("�������̣��ű���ʹ�õĲ��<%1>�ӿ�<%2>û���ҵ���").arg(plugin->objectName()).arg(interfaceID));
			return;
		}

		QString apistr = plugin->runtimeString(inter);
		ResourceHash  mapkey = hashString(apistr);
		if (!objectRefMap_.contains(mapkey)){
			objectRefMap_.insert(mapkey, objectRefs_.size());
			objectRefs_.append(apistr);
		}
	

		//////////////////////////////////////////////////////////////////////////
		// �������Ƶ����Ƶ������������Բ�������ȡ�ļ�ID

	}

}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

void HTML5Exporter::makeRuntimeFile(){
	QString desPath = exportPath_ + UDQ_T("/");

	if (project_->exportType_ == 2){
		generateFile(FCT_RUNTIMEDEBUG, desPath + UDQ_T("orionruntime.js"));
	}
	else{
		generateFile(FCT_RUNTIME, desPath + UDQ_T("orionruntime.js"));
	}
}

void HTML5Exporter::makeDataFile(){
	QString desPath = exportPath_ + UDQ_T("/data.js");

	QJsonObject root;

	// ��Ŀģ��
	QJsonArray projectData;
	// ��Ϸ��
	projectData.append(QJsonValue());
	// ��ʼ����
	if (project_->startLevel_.isEmpty()){
		projectData.append(QJsonValue());
	}
	else{
		projectData.append(project_->startLevel_);
	}

	// ����б�
	projectData.append(generatePlugins());

	// ���������б�
	projectData.append(generateObjectTypes());

	// �����б�
	projectData.append(generateFamilies());

	// ����
	projectData.append(generateLevels());

	// �ű�
	projectData.append(generateBlueprints());

	// ��Ƶ�����б�
	projectData.append(QJsonArray());
	// ��Դ�ļ����·��
	projectData.append(project_->getDefMediaDir(MEDIA_AUDIO) + UDQ_T("/"));
	// ���ض���
	projectData.append(project_->pixelRounding_);
	// ��ƴ�С
	projectData.append(project_->resolution_.width());
	projectData.append(project_->resolution_.height());
	// ȫ��ģʽ
	projectData.append(project_->fullscreenMode_);
	// 3D����
	projectData.append(project_->useWebGL_);
	// ���Բ���
	projectData.append(project_->sampleType_ == SAMPLE_LINEAR);
	// �������
	projectData.append(project_->clearBackground_);
	// �汾
	projectData.append(UDQ_T("1.0.0"));
	// ��DPI
	projectData.append(project_->useHighDPI_);
	// ����Layout����
	projectData.append(project_->useLoaderLayout_);
	// Loader���
	projectData.append(project_->loaderStyle_);
	// ��ʾ����
	projectData.append(project_->orientation_);
	// ��һ��UID
	projectData.append(nextUID_);
	// ʧȥ������ͣ��ģ��
	projectData.append(false);
	// ȫ��������ʾ����
	projectData.append(project_->fullscreenQuality_ == QUALITY_HIGH);
	// ��С��ʾ����
	projectData.append(project_->downScaling_);
	// �Ƿ�Ԥ���������ļ�
	projectData.append(project_->preloadSound_);
	// ��Ϸ��Ŀ��
	projectData.append(project_->objectName());
	// �����б�
	projectData.append(QJsonArray());

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	QFile saveFile(desPath);
	if (!saveFile.open(QIODevice::WriteOnly)) {
		return;
	}

	root.insert(UDQ_T("project"), projectData);

	QJsonDocument saveDoc(root);
	saveFile.write(saveDoc.toJson(project_->minfyScript_ ? QJsonDocument::Compact : QJsonDocument::Indented));
	saveFile.close();

	cacheFiles_.append(UDQ_T("data.js"));

}

QJsonArray HTML5Exporter::generatePlugins(){
	QJsonArray arr;

	foreach(PluginResourceObject* plugin, entities_){
		QJsonArray def;
		Q_ASSERT(objectRefMap_.contains(plugin->hashKey()));
		def.append(objectRefMap_[plugin->hashKey()]);  // ����
		def.append(plugin->hasAttribute(EF_SINGLEGLOBAL));
		def.append(plugin->hasAttribute(EF_WORLD));
		def.append(plugin->hasAttribute(EF_POSITION));
		def.append(plugin->hasAttribute(EF_SIZE));
		def.append(plugin->hasAttribute(EF_ANGLE));
		def.append(plugin->hasAttribute(EF_APPEARANCE));
		def.append(plugin->hasAttribute(EF_ZORDER));
		def.append(plugin->hasAttribute(EF_EFFECTS));
		def.append(plugin->hasAttribute(EF_ANIMATIONDRAW));
		arr.append(def);
	}

	return arr;
}

QJsonArray HTML5Exporter::generateObjectTypes(){
	objectTypeMap_.clear();
	objectVariableMap_.clear();
	QJsonArray arr;

	foreach(ObjectTypeResourceObject* objtype, project_->objTypes_){

		// ����ɾ������
		if (objtype->flags() & URF_TagGarbage)
			continue;

		objectTypeMap_.insert(objtype->hashKey(), objectTypeMap_.size());

		QJsonArray def;

		def.append(objtype->description());
		def.append(objectRefMap_[objtype->prototype_->hashKey()]);  // ����
		def.append(false);
		//�ڲ���������
		def.append(generateVariables(objtype));
		def.append(objtype->behaviors_.size());
		def.append(objtype->effects_.size());
		// ��������
		def.append(generateTexture(objtype));
		// ��������
		def.append(generateAnimations(objtype));
		// ��Ϊ����
		def.append(generateBehaviors(objtype));

		def.append(objtype->global_);
		// �Ƿ���loader������
		def.append(false);
		def.append((qint64)objtype->hashKey());
		////////////////////////////////////////////////
		// ��Ч����
		QJsonArray effs;
		foreach(QString name, objtype->effects_){
			QJsonArray e;
			PluginResourceObject* effplugin = objtype->attachments_[name];
			Q_ASSERT(effplugin != NULL);
			e.append(name);
			e.append(effplugin->objectName());
			effs.append(e);
		}
		def.append(effs);

		// tilemap��ײ��������
		ResourceHash typeH = hashStringLower(objtype->prototype_->objectName());
		if (typeH == COMP_TILEMAP){
			QMap<int, QList<QPointF> > collision;
			QDataStream stream(objtype->tilemapData_);
			stream >> collision;

			// ��ȡ����������
			int maxIndex = -1;
			foreach(int k, collision.keys()){
				if (k > maxIndex){
					maxIndex = k;
				}
			}

			if (maxIndex >= 0 ){
				maxIndex++;
				QJsonArray polys;
				for (int m = 0; m < maxIndex; m++){
					if (collision.contains(m)){
						QJsonArray pts;
						foreach(QPointF p, collision[m]){
							pts.append(p.x());
							pts.append(p.y());
						}
						polys.append(pts);
					}
					else{
						polys.append(QJsonValue()); // ������
					}
				}
				def.append(polys);
			}
			else{
				def.append(QJsonArray());
			}
		}
		else{
			def.append(QJsonArray());
		}

		// �������
		if (objtype->prototype_->hasAttribute(EF_SINGLEGLOBAL)){
				QMap<ResourceHash, QString> params;
				def.append(processPluginProperty(QString(), objtype, params));
		}

		arr.append(def);
	}

	//  ��������
	//  ["t36", 1, true, [], 0, 0, null, null, [], false, false, 5008010868559932, [], null]
	foreach(FamilyResourceObject* family, project_->families_){

		if (family->members_.size() == 0){ // ���û�г�Ա�����Family��Ч
			continue;
		}

		objectTypeMap_.insert(family->hashKey(), objectTypeMap_.size());

		QJsonArray def;
		def.append(family->description());
		// family��ֻ����һ��ʵ������
		def.append(objectRefMap_[family->members_[0]->prototype_->hashKey()]);  // ����
		def.append(true);

		//�ڲ���������todo
		def.append(QJsonArray());
		def.append(family->behaviors_.size());
		def.append(family->effects_.size());
		// ��������
		def.append(QJsonValue());
		// ��������
		def.append(QJsonValue());
		// ��Ϊ����
		def.append(generateBehaviors(family));
		def.append(false);
		// �Ƿ���loader������
		def.append(false);
		def.append((qint64)family->hashKey());
		// ��Ч����todo
		def.append(QJsonArray());
		// tilepoly����
		def.append(QJsonValue());
		arr.append(def);
	}

	return arr;
}

QJsonArray HTML5Exporter::generateVariables(ObjectTypeResourceObject* obj){
	QJsonArray arr;
	QList<ResourceHash> keys = obj->variables_.keys();
	for (int i = 0; i < keys.size(); i++){
		arr.append((qint64)keys[i]);
		PluginVariable* var = obj->variables_.value(keys[i]);
		objectVariableMap_.insert(hashString(obj->description() + UDQ_T(".") + var->name), i);
	}
	return arr;
}

QJsonArray HTML5Exporter::generateTexture(ObjectTypeResourceObject* obj){
	QJsonArray arr;
	if (obj->prototype_->hasAttribute(EF_TEXTURE)){
		if (obj->staticImage_ == NULL){
			// ����ȱʡͼƬ����
			arr.append(resourceMap_[obj->hashKey()]);
			arr.append(emptysize_);
			arr.append(4);  // ȱʡͼƬ��ʽ������������������
		}
		else{
			arr.append(resourceMap_[obj->staticImage_->hashKey()]);
			arr.append(obj->staticImage_->content_.size());
			arr.append(pixelFormat(obj->staticImage_->image_));
		}
	}

	return arr;
}

QJsonArray HTML5Exporter::generateBehaviors(ObjectTypeResourceObject* obj){

	QJsonArray arr;

	foreach(QString id, obj->behaviors_){
		PluginResourceObject* plugin = obj->attachments_.value(id);
		Q_ASSERT(plugin != NULL);

		QJsonArray beh;
		beh.append(id);
		beh.append(objectRefMap_.value(plugin->hashKey()));
		beh.append((qint64)BlueprintObject::makeUniqueId());
		arr.append(beh);
	}
	return arr;
}

QJsonArray HTML5Exporter::generateBehaviors(FamilyResourceObject* obj){

	QJsonArray arr;

	foreach(QString id, obj->behaviors_){
		PluginResourceObject* plugin = obj->attachments_.value(id);
		Q_ASSERT(plugin != NULL);

		QJsonArray beh;
		beh.append(id);
		beh.append(QString::number(plugin->hashKey()));
		beh.append((qint64)BlueprintObject::makeUniqueId());
		arr.append(beh);
	}
	return arr;
}

int HTML5Exporter::pixelFormat(QImage& img){
	QImage::Format fm = img.format();
	int pixelfm = 0; // Ĭ��RGBA8
	if (fm == QImage::Format_RGB888){   // RGB8
		pixelfm = 1;
	}
	else if (fm == QImage::Format_RGB444){  // RGBA4
		pixelfm = 2;
	}
	else if (fm == QImage::Format_RGB555){  // RGB5_A1
		pixelfm = 3;
	}
	else if (fm == QImage::Format_RGB16){  // RGB565
		pixelfm = 4;
	}
	return pixelfm;
}

QJsonArray HTML5Exporter::generateAnimations(ObjectTypeResourceObject* obj){
	if (!obj->prototype_->hasAttribute(EF_ANIMATIONS)){
		return QJsonArray();
	}

	QJsonArray arr;

	if (obj->animation_){
		//
		foreach(SpriteSequenceResourceObject* seq, obj->animation_->sequences_){
			QJsonArray anim;

			anim.append(seq->objectName());
			anim.append(qRound(seq->speed_ * 60));
			anim.append(seq->repeat_);
			anim.append(seq->count_);
			anim.append(seq->repeatTo_);
			anim.append(seq->pingpong_);
			anim.append((qint64)seq->hashKey());
			// ֡��Ϣ
			QJsonArray frames;

			QList<SequenceFrame*> fs = seq->frames_.values();
			for (int i = 0; i < fs.size(); i++){
				QJsonArray img;
				ResourceHash key = obj->animation_->imgSources_.value(fs[i]->imghost);
				
				//  �ļ�·����
				img.append(resourceMap_[key]);
				DEFINE_RESOURCE_HASHKEY_VALID(ImageResourceObject, image, key);
				img.append(image->content_.size());

				QRect rc = obj->animation_->frameRect(fs[i]->imghost, fs[i]->imgindex);
				img.append(rc.left());
				img.append(rc.top());
				img.append(rc.width());
				img.append(rc.height());
				img.append(1);

				QPointF center = seq->frameCenterAnchor(i);
				img.append(center.x());
				img.append(center.y());

				// ����ê��
				QJsonArray anrs;
				QPointF anchor;
				foreach(QString name, seq->anchors_){
					if (name == UDQ_T("Origin"))
						continue;
					anchor = seq->frameAnchor(i, name);
					QJsonArray a;
					a.append(name);
					a.append(anchor.x());
					a.append(anchor.y());
					anrs.append(a);
				}
				img.append(anrs);

				// ��ײ����
				QJsonArray collision;
				foreach(QPointF p, seq->frameCollision(i)){
					collision.append(p.x() - center.x());
					collision.append(p.y() - center.y());
				}
				img.append(collision);

				// ͼ�θ�ʽ
				img.append(pixelFormat(image->image_));

				frames.append(img);
			}

			anim.append(frames);
			arr.append(anim);
		}	

	}
	else{


		QJsonArray anim;
		//["Default", 5, false, 1, 0, false, 6379470358422484, 
		//  	[["images/sprite-sheet0.png", 168, 0, 0, 250, 250, 1, 0.5, 0.5, [], [], 3]]
		//]

		// ��֡����
		anim.append(UDQ_T("Default"));
		anim.append(5);
		anim.append(false);
		anim.append(1);
		anim.append(0);
		anim.append(false);
		// sid
		anim.append((qint64)obj->staticSID_);
		// ֡��Ϣ
		QJsonArray frames;
		QJsonArray img;

		if (obj->staticImage_){
			img.append(resourceMap_[obj->staticImage_->hashKey()]);
			img.append(obj->staticImage_->content_.size());
		}
		else{
			img.append(resourceMap_[obj->hashKey()]);
			img.append(emptysize_);
		}

		img.append(0);
		img.append(0);

		if (obj->staticImage_){
			img.append(obj->staticImage_->image_.width());
			img.append(obj->staticImage_->image_.height());
		}
		else{
			img.append(64);
			img.append(64);
		}

		img.append(1);
		img.append(0.5);
		img.append(0.5);
		img.append(QJsonArray());
		img.append(QJsonArray());
		if (obj->staticImage_){
			img.append(pixelFormat(obj->staticImage_->image_));
		}
		else{
			img.append(0);
		}

		frames.append(img);
		anim.append(frames);
		arr.append(anim);

	}
	return arr;
}

QJsonArray HTML5Exporter::generateFamilies(){
	QJsonArray arr;  // todo



	return arr;
}

QJsonArray HTML5Exporter::generateLevels(){
	QJsonArray arr;

	foreach (LevelResourceObject* level, project_->levels_){

		// ����ɾ������
		if (level->flags() & URF_TagGarbage)
			continue;

		QJsonArray leveldata;
		leveldata.append(level->description());
		leveldata.append(level->size_.width());
		leveldata.append(level->size_.height());
		leveldata.append(level->unboundScroll_);
		
		// �����ű�
		DEFINE_RESOURCE_HASHKEY(BlueprintResourceObject, blueprint, level->blueprint_);
		if (blueprint){
			leveldata.append(blueprint->description());
		}
		else{
			leveldata.append(QJsonValue());
		}

		// sid
		leveldata.append((qint64)level->hashKey());

		QJsonArray layers;
		// ͼ��
		foreach (LayerResourceObject* layer, level->layers_){
			// ����ɾ��ͼ��
			if (layer->flags() & URF_TagGarbage)
				continue;
			layers.append(generateLayer(layer));
		}
		leveldata.append(layers);

		// ȫ��ʵ��todo
		QJsonArray inst;
		foreach(QByteArray data, level->instances_){
			inst.append(processInstance(data));
		}
		leveldata.append(inst);

		// ��Чtodo
		leveldata.append(QJsonArray());
		arr.append(leveldata);
	}

	return arr;
}

QJsonArray HTML5Exporter::generateLayer(LayerResourceObject* layer){
	QJsonArray arr;

	arr.append(layer->description());
	arr.append(layer->level_->layers_.indexOf(layer));
	arr.append((qint64)layer->hashKey());
	arr.append(layer->initVisible_);
	QJsonArray bgColor;
	bgColor.append(layer->bgColor_.red());
	bgColor.append(layer->bgColor_.green());
	bgColor.append(layer->bgColor_.blue());
	arr.append(bgColor);
	arr.append(layer->transparent_);// ����͸��

	arr.append(layer->parallax_.x()/ 100.0);
	arr.append(layer->parallax_.y()/100.0);

	arr.append(layer->opacity_ / 100.0);
	arr.append(false);  // �Ƿ�ǿ����Ⱦ������
	arr.append(layer->useRenderCell_);
	arr.append(layer->scale_/100.0); // ���ű���
	//////////////////////////////////////////////////////////////////////////
	// todo
	arr.append(0); // ���ģʽ
	arr.append(0); // ��Ч�����ڻص�����

	// ����ʵ���б�
	QJsonArray inst;
	foreach(QByteArray data, layer->instances_){
		inst.append(processInstance(data));
	}
	arr.append(inst);

	// ��Ч�б�
	arr.append(QJsonArray());

	return arr;
}

QJsonArray HTML5Exporter::processInstance(const QByteArray& data){
	QJsonArray arr;

	int type;  // ��������
	QDataStream stream(data);
	stream >> type;
	Q_ASSERT(type > LEVEL_LAYER);

	unsigned int version;
	stream >> version;

	ResourceHash sid;
	stream >> sid;

	QRectF boundingBox;
	stream >> boundingBox;

	QPointF pos;
	stream >> pos;

	qreal angle;
	stream >> angle;

	QSizeF origin;
	stream >> origin;

	bool isLocking;
	stream >> isLocking;

	qreal alpha;
	stream >> alpha;

	ObjectTypeResourceObject* typePrototype = NULL;
	ResourceHash key;
	stream >> key;
	typePrototype = UDynamicCast(ObjectTypeResourceObject, (ResourceObject::findResource(key)));
	Q_ASSERT(typePrototype != NULL);

	QMap<ResourceHash, QString> parameters;
	stream >> parameters;

	if (typePrototype->prototype_->hasAttribute(EF_WORLD)){

		int blend; // ���ģʽ
		int fallback;  // �ص�����
		stream >> blend;
		stream >> fallback;

		QJsonArray base;
		
		// λ�ã�����ê����ͼ���ϵ�λ�ã�, ��ʱ��������ת
		QRect bbox = boundingBox.toRect();
		// λ��
		base.append(pos.x());
		base.append(pos.y());
		base.append(0);

		// ��С
		base.append(bbox.width());
		base.append(bbox.height());
		base.append(0);

		// ��ת�Ƕ�
		base.append(angle / 180.0 * 3.1415926);
		// ͸����
		base.append(alpha);
		// ���ĵ�λ��
		base.append(origin.width());
		base.append(origin.height());

		// �����Ⱦģʽ
		base.append(blend);
		base.append(fallback);

		// ��Ч��������
		QJsonArray effs;
		foreach(QString bid, typePrototype->effects_){
			effs.append(processPluginProperty(bid, typePrototype, parameters));
		}
		base.append(effs);

		// tilemap��������
		if (type == LEVEL_TILEMAP){
			int gridW;
			int gridH;
			QMap<QPair<int, int>, QSize>  mapData;
			stream >> gridW >> gridH >> mapData;
			// �����ַ���
			base.append(processTileMap(gridW, gridH, mapData));
		}
		arr.append(base);
	}
	else{
		arr.append(QJsonValue());
	}

	Q_ASSERT(typePrototype != NULL);
	//������������
	arr.append(project_->objTypes_.indexOf(typePrototype));
	// sid
	arr.append((qint64)sid);

	// �Զ���ʵ������
	QJsonArray vars;
	foreach(PluginVariable* var, typePrototype->variables_.values()){
		vars.append(processInstanceVariable(var, typePrototype, parameters));
	}
	arr.append(vars);

	// ��Ϊ����
	QJsonArray behs;
	foreach(QString bid, typePrototype->behaviors_){
		behs.append(processPluginProperty(bid, typePrototype, parameters));
	}
	arr.append(behs);

	// �������
	QJsonArray params;
	params = processPluginProperty(QString(), typePrototype, parameters);
	arr.append(params);
	return arr;
}


QString  HTML5Exporter::state2String(int state){
	QString str;
	if (state & 1){
		str.append(UDQ_T("h"));
	}
	if (state & 2){
		str.append(UDQ_T("v"));
	}
	if (state & 4){
		str.append(UDQ_T("d"));
	}
	return str;
}


QJsonArray HTML5Exporter::processTileMap(int w, int h, QMap<QPair<int, int>, QSize>& data){
	QJsonArray arr;
	arr.append(w);
	arr.append(h);

	int length = w *h;
	QStringList block;
	for (int i = 0; i < length; i++){
		block.append(UDQ_T("-1"));
	}

	QMapIterator<QPair<int, int>, QSize> i(data);
	while (i.hasNext()){  // ���Ƶ�ͼ��
		i.next();
		if (i.key().first >= w || i.key().second >= h){
			continue;
		}
		// ���ɵ����ַ���
		block[i.key().second*w + i.key().first] = QString(UDQ_T("%1%2")).arg(i.value().width()).arg(state2String(i.value().height()));
	}

	QString str;
	// ���кϲ�����
	QString base;
	int dupnum = 0;
	for (int i = 0; i < length; i++){
		if (base.isEmpty()){
			base = block[i];
			dupnum = 1;
			continue;
		}

		if (block[i] == base){
			dupnum++;
		}
		else{
			if (dupnum == 1){
				str.append(QString(UDQ_T("%1,")).arg(base));
			}
			else{
				str.append(QString(UDQ_T("%1x%2,")).arg(dupnum).arg(base));
			}

			base = block[i];
			dupnum = 1;
		}
	}

	if (dupnum == 1){
		str.append(QString(UDQ_T("%1")).arg(base));
	}
	else{
		str.append(QString(UDQ_T("%1x%2")).arg(dupnum).arg(base));
	}

	arr.append(str);
	return arr;
}


QString HTML5Exporter::color2String(const QColor& clr){
	return UDQ_TR("rgb(%1,%2,%3)").arg(clr.red()).arg(clr.green()).arg(clr.blue());
}

QString HTML5Exporter::font2String(const QFont& fnt){
	QString str;
	if (fnt.bold()){
		str.append(UDQ_T("bold "));
	}

	if (fnt.italic()){
		str.append(UDQ_T("italic "));
	}
	// ��ȡ��������ش�С
	QFontMetrics fm(fnt);
	str.append(UDQ_TR("%1pt %2").arg(fnt.pointSize()).arg(fnt.family()));
	return str;
}

QJsonValue HTML5Exporter::processInstanceVariable(PluginVariable* var, ObjectTypeResourceObject* objType, QMap<ResourceHash, QString>& parameters){
	Q_ASSERT(var != NULL);

	QJsonArray value;
	QString tmpstr = var->value;
	ResourceHash varkey = hashString(var->name);
	// ��ȡ����ֵ
	if (parameters.contains(varkey)){
		tmpstr = parameters.value(varkey);
	}
	else{
		tmpstr = objType->getParameter(var->name); // ��ȡȱʡֵ
	}
	
	if (var->datatype == UDQ_T("bool")){
		value.append(tmpstr == UDQ_T("true") ? 1 : 0);
	}
	else if (var->datatype == UDQ_T("int")){
		value.append(tmpstr.toInt());
	}
	else if (var->datatype == UDQ_T("float")){
		value.append(tmpstr.toDouble());
	}
	else{
		value.append(tmpstr);
	}

	return value;
}

QJsonArray HTML5Exporter::processPluginProperty(const QString& name, ObjectTypeResourceObject* objType, QMap<ResourceHash, QString>& parameters){
	QJsonArray params;
	PluginResourceObject* plugin = NULL;
	QString pluginName = name;
	if (name.isEmpty() || name == objType->prototype_->objectName()){
		plugin = objType->prototype_;
		pluginName = objType->prototype_->objectName();
	}
	else{
		plugin = objType->attachments_.value(name);
	}
	Q_ASSERT(plugin != NULL);

	ResourceHash varkey = 0;
	PluginVariable* var = NULL;
	QString tmpstr;
	foreach(ResourceHash key, plugin->varIndex_){
		var = plugin->variables_.value(key);
		if (var->flags & VAR_EXTEND){
			continue;
		}
		// �Ȳ����޸ĵ�ֵ�����û����ȡȱʡֵ
		varkey = hashString(pluginName + UDQ_T(".") + var->name);  // ������������

		// ��ȡ����ֵ
		if (parameters.contains(varkey)){
			tmpstr = parameters.value(varkey);
		}
		else{  // �����ȡȱʡֵ
			tmpstr = objType->getParameter(var->name, pluginName);
		}

		if (var->datatype == UDQ_T("bool")){
			params.append(tmpstr == UDQ_T("true") ? 1 : 0 );
		}
		else if (var->datatype == UDQ_T("int")){
			params.append(tmpstr.toInt());
		}
		else if (var->datatype == UDQ_T("float")){
			params.append(tmpstr.toDouble());
		}
		else if (var->datatype == UDQ_T("color")){  // ��ɫ rgb(r,g,b)
			params.append(color2String(QColor(tmpstr)));
		}
		else if (var->datatype == UDQ_T("font")){  // ���� bold italic 11pt Arial
			QFont fnt;
			fnt.fromString(tmpstr);
			params.append(font2String(fnt));
		}
		else if (var->datatype == UDQ_T("enum")){ // ö��
			params.append(var->enums.indexOf(tmpstr));
		}
		else{  // ����һ�ɰ��ַ����Դ�
			params.append(tmpstr);
		}
	}

	return params;
}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////


QJsonArray HTML5Exporter::generateBlueprints(){
	QJsonArray arr;

	foreach(BlueprintResourceObject* blueprint, project_->blueprints_){
		if (blueprint->flags() & URF_TagGarbage)
			continue;

		QJsonArray script;

		// �ű���
		script.append(blueprint->description());

		// �¼����б�
		QJsonDocument doc = QJsonDocument::fromJson(blueprint->scriptJson_);
		QJsonArray blocks = doc.array();

		// �滻����API����
		for (int i = 0; i < blocks.size(); i++){
			Q_ASSERT(blocks[i].isArray());
			QJsonArray arr = blocks[i].toArray();
			replaceBlock(arr);
			blocks.replace(i, arr);	
		}
	
		script.append(blocks);
		arr.append(script);
	}

	return arr;
}

void HTML5Exporter::replaceBlock(QJsonArray& block){
	int type = block[0].toInt();
	if (type == 0){
		Q_ASSERT(block.size() >= 7);

		//5
		QJsonArray cnds = block[5].toArray();
		for (int i = 0; i < cnds.size(); i++){
			QJsonArray cnd = cnds[i].toArray();
			Q_ASSERT(cnd.size() >= 2);

			if (cnd[0].isString()){  // ��ϵͳ��������
				ResourceHash key = (ResourceHash)cnd[0].toString().toUInt();

				DEFINE_RESOURCE_HASHKEY(ResourceObject, res, key);
				if (UIsExactKindOf(PluginResourceObject, res) ){  // ����ǲ���������Ψһʵ��
					PluginResourceObject* plugin = UDynamicCast(PluginResourceObject, res);
					Q_ASSERT(plugin && plugin->hasAttribute(EF_SINGLEGLOBAL));
					ObjectTypeResourceObject* single = project_->getObjectType(plugin);
					Q_ASSERT(single != NULL);
					key = single->hashKey();
				}

				cnd.replace(0, objectTypeMap_.value(key));
			}
	
			ResourceHash key = (ResourceHash)cnd[1].toString().toUInt();
			cnd.replace(1, objectRefMap_.value(key));


			// �滻����
			if (cnd.size() >= 10){  // �в���
				QJsonArray params = cnd[9].toArray();
				replaceParam(params);
				cnd.replace(9, params);
			}

			cnds.replace(i, cnd);
		}
		block.replace(5, cnds);

		//6 
		QJsonArray acts = block[6].toArray();
		for (int i = 0; i < acts.size(); i++){
			QJsonArray act = acts[i].toArray();
			Q_ASSERT(act.size() >= 2);

			if (act[0].isString()){  // ��ϵͳ��������
				ResourceHash key = (ResourceHash)act[0].toString().toUInt();

				DEFINE_RESOURCE_HASHKEY(ResourceObject, res, key);
				if (UIsExactKindOf(PluginResourceObject, res)){  // ����ǲ���������Ψһʵ��
					PluginResourceObject* plugin = UDynamicCast(PluginResourceObject, res);
					Q_ASSERT(plugin && plugin->hasAttribute(EF_SINGLEGLOBAL));
					ObjectTypeResourceObject* single = project_->getObjectType(plugin);
					Q_ASSERT(single != NULL);
					key = single->hashKey();
				}

				act.replace(0, objectTypeMap_.value(key));
			}

			ResourceHash key = (ResourceHash)act[1].toString().toUInt();
			act.replace(1, objectRefMap_.value(key));

			// �滻����
			if (act.size() >= 6){  // �в���
				QJsonArray params = act[5].toArray();
				replaceParam(params);
				act.replace(5, params);
			}

			acts.replace(i, act);
		}
		block.replace(6, acts);

		//7 
		if (block.size() >= 8){
			QJsonArray subs = block[7].toArray();
			for (int i = 0; i < subs.size(); i++){
				QJsonArray sub = subs[i].toArray();
				replaceBlock(sub);
				subs.replace(i, sub);
			}
			block.replace(7, subs);
		}
	}
}

void HTML5Exporter::replaceParam(QJsonArray& param){
	if (param.isEmpty())
		return;

	for (int i = 0; i < param.size(); i++){
		QJsonArray p = param[i].toArray();
		if ( p.isEmpty())
			continue;

		Q_ASSERT(p.size() >= 2);

		int type = p[0].toInt();
		if (type == 4){  // ��������
			ResourceHash key = (ResourceHash)p[1].toString().toUInt();
			p.replace(1, objectTypeMap_.value(key));
		}
		else if (type == 13){  // �䳤��������

		}
		else if (type == 10){  // ����ʵ����������
			QJsonValue v = p[1];
			if (v.isString()){
				ResourceHash key = (ResourceHash)v.toString().toUInt();
				p.replace(1, objectVariableMap_.value(key));
			}
		}
		else if (type == 2 || type == 3 || type == 5 || type == 6 || type == 8 || type == 9 || type == 11 || type == 12){  // ����Ҫ�����

		}
		else if (type == 0){
			QJsonValue v = p[1];
			if (v.isArray()){
				QJsonArray exp = v.toArray();
				replaceExpNode(exp);
				p.replace(1, exp);
			}
		}
		else if (type == 19 || type == 20 || type == 21  ){  //ϵͳ���ʽ
			replaceExpNode(p);
		}
		else{
			// ��Ҫ�ݹ鴦���
			QJsonArray exp = p[1].toArray();
			replaceExpNode(exp);
			p.replace(1, exp);
		}

		param.replace(i, p);

	}

}

void HTML5Exporter::replaceExpNode(QJsonArray& exp){
	if (exp.isEmpty())
		return;

	int type = exp[0].toInt();

	if (type >= 3 && type <= 17){  // ��������
		QJsonArray node = exp[1].toArray();
		replaceExpNode(node);
		exp.replace(1, node);

		if (exp.size() >= 3){
			node = exp[2].toArray();
			replaceExpNode(node);
			exp.replace(2, node);
		}
	}
	else if (type == 18){  // �������ʽ

	}
	else if (type == 19){  // ϵͳ���ʽ
		ResourceHash key = (ResourceHash)exp[1].toString().toUInt();
		exp.replace(1, objectRefMap_.value(key));
		qDebug() << "19:" << objectRefMap_.value(key) << "@" << exp;

		// �滻����
		if (exp.size() >= 3){
			QJsonArray params = exp[2].toArray();
			for (int i = 0; i < params.size(); i++){
				QJsonArray p = params.at(i).toArray();
				replaceExpNode(p);
				params.replace(i, p);
			}
			exp.replace(2, params);
		}
	}
	else if (type == 20){  // ������ʽ
		Q_ASSERT(exp.size() >= 5);

		// ��������
		ResourceHash key = (ResourceHash)exp[1].toString().toUInt();
		if (objectTypeMap_.contains(key)){
			exp.replace(1, objectTypeMap_.value(key));
		}
		else{  // ��ʵ������
			DEFINE_RESOURCE_HASHKEY_VALID(PluginResourceObject, plugin, key);
			ObjectTypeResourceObject* single = project_->getObjectType(plugin);
			Q_ASSERT(single != NULL);
			exp.replace(1, objectTypeMap_.value(single->hashKey()));
		}

		// ����
		key = (ResourceHash)exp[2].toString().toUInt();
		exp.replace(2, objectRefMap_.value(key));

		// ʵ�����ʽ
		QJsonArray arr = exp[4].toArray();
		if (arr.size() >  0){
			replaceInstanceIndex(arr);
			exp.replace(4, arr);
		}

		// �������ʽ
		if (exp.size() >= 6){
			arr = exp[5].toArray();
			if (arr.size() > 0){
				replaceParam(arr);
				exp.replace(5, arr);
			}
		}
	}
	else if (type == 21){  // ����ʵ���������ʽ
		Q_ASSERT(exp.size() >= 5);
		// ������������
		ResourceHash key = exp[1].toString().toUInt();
		exp.replace(1, objectTypeMap_[key]);
		// �滻�������ʽ
		key = exp[4].toString().toUInt();
		exp.replace(4, objectVariableMap_[key]);
	}
	else if (type == 22){  // ������Ϊ���ʽ
		Q_ASSERT(exp.size() >= 5);
		// ������������
		ResourceHash key = exp[1].toString().toUInt();
		exp.replace(1, objectTypeMap_[key]);

		// ��Ϊ��������
		key = exp[3].toString().toUInt();
		exp.replace(3, objectRefMap_[key]);

		// �滻�������ʽ

	}
	else if (type == 23){  // �������ʽ

	}

}

void HTML5Exporter::replaceInstanceIndex(QJsonArray& param){

}


//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

void HTML5Exporter::makeResourceFile(){

	// NWģʽ�²�����Ŀ¼
	QString imgPath = exportPath_ + UDQ_T("/");
	QString videoPath = exportPath_ + UDQ_T("/");
	QString audioPath = exportPath_+ UDQ_T("/");
	QString miscPath = exportPath_  + UDQ_T("/");

	if (mode_ != 2){
		imgPath +=  project_->getDefMediaDir(MEDIA_IMAGE) + UDQ_T("/");
		videoPath += project_->getDefMediaDir(MEDIA_VIDEO) + UDQ_T("/");
		audioPath +=  project_->getDefMediaDir(MEDIA_AUDIO) + UDQ_T("/");
		miscPath += project_->getDefMediaDir(MEDIA_MISC) + UDQ_T("/");
	}

	QString filePath;
	// �������ж�������
	foreach(ResourceHash key, galleries_.values()){
		DEFINE_RESOURCE_HASHKEY(GalleryResourceObject, gallery, key);

		if(UIsExactKindOf(ImageResourceObject, gallery)){
			filePath = saveImageFile(key, imgPath);
			if (mode_ != 2){
				filePath.prepend(project_->getDefMediaDir(MEDIA_IMAGE) + UDQ_T("/"));
			}
		}
		else if (UIsExactKindOf(AudioResourceObject, gallery)){
			filePath = saveAudioFile(key, audioPath);
			if (mode_ != 2){
				filePath.prepend(project_->getDefMediaDir(MEDIA_AUDIO) + UDQ_T("/"));
			}
		}
		else if (UIsExactKindOf(VideoResourceObject, gallery)){
			filePath = saveVideoFile(key, videoPath);
			if (mode_ != 2){
				filePath.prepend(project_->getDefMediaDir(MEDIA_VIDEO) + UDQ_T("/"));
			}
		}
		resourceMap_.insert(key, filePath);
		cacheFiles_.append(filePath);
	}

	// ���ɿ�ռλ������Դ
	if (emptyGalleries_.size() > 0){
		// ����͸��ͼƬ
		filePath = saveEmptyImageFile(imgPath);
		if (mode_ != 2){
			filePath.prepend(project_->getDefMediaDir(MEDIA_IMAGE) + UDQ_T("/"));
		}
		cacheFiles_.append(filePath);

		foreach(ObjectTypeResourceObject* obj, emptyGalleries_){
			resourceMap_.insert(obj->hashKey(), filePath);
		}
	}

}

void HTML5Exporter::makeIconFile(){
	QString desPath = exportPath_ + UDQ_T("/");
	generateFile(FCT_ICON16, desPath + UDQ_T("icon-16.png"), false);
	generateFile(FCT_ICON32, desPath + UDQ_T("icon-32.png"), false);
	generateFile(FCT_ICON128, desPath + UDQ_T("icon-128.png"), false);
	generateFile(FCT_ICON256, desPath + UDQ_T("icon-256.png"), false);
	generateFile(FCT_ICONLOADING, desPath + UDQ_T("loading-logo.png"), false);

	if (mode_ == 2){
		generateFile(FCT_ICONLOADING, desPath + UDQ_T("logo.png"), false);
	}

}

void HTML5Exporter::makeCommonScriptFile(){
	QString desPath = exportPath_ + UDQ_T("/");
	generateFile(FCT_JQUERY, desPath + UDQ_T("jquery.min.js"));

	if (mode_ == 2){  //NW ģʽ
		generateFile(FCT_INDEXNW, desPath + UDQ_T("index.html"));
	}
	else if (mode_ == 1){  //CORDOVA ģʽ
		generateFile(FCT_INDEXCORDOVA, desPath + UDQ_T("index.html"));
	}
	else{
		generateFile(FCT_INDEX, desPath + UDQ_T("index.html"));
		if (project_->exportType_ == 2){  // ����
			generateFile(FCT_INDEXDEBUG, desPath + UDQ_T("debug"));
			generateFile(FCT_DEBUGGER, desPath + UDQ_T("debugger.js"));
			generateFile(FCT_DEBUGGER_CSS, desPath + UDQ_T("debugger.css"));
			generateFile(FCT_DEBUGGER_COLLAPSED, desPath + UDQ_T("debugger-collapsed.png"), false);
			generateFile(FCT_DEBUGGER_DEL, desPath + UDQ_T("debugger-delete.png"), false);
			generateFile(FCT_DEBUGGER_EXPANEDED, desPath + UDQ_T("debugger-expanded.png"), false);
			generateFile(FCT_DEBUGGER_INSPECT, desPath + UDQ_T("debugger-inspect.png"), false);
			generateFile(FCT_DEBUGGER_WATCH, desPath + UDQ_T("debugger-watch.png"), false);
		}
	}

	if (mode_ == 0 ){
		generateFile(FCT_APPMANIFEST, desPath + UDQ_T("app.manifest"), true, false);
		if (project_->exportType_ == 0){  // ��ʽ����ʱ���������߻���
			generateFile(FCT_APPCACHE, desPath + UDQ_T("offline.appcache"), true, false);
		}
	}

}

//////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

// ����ͼƬ
QString HTML5Exporter::saveImageFile(ResourceHash hash, const QString& rootPath ){
	QString desFile;

	DEFINE_RESOURCE_HASHKEY(ImageResourceObject, image, hash);
	if (image != NULL){  // �����ͼƬ
		QString ext = image->getFormat().toLower();

		// ȷ��Ŀ¼����
		QDir dir(rootPath);
		if (!dir.exists()){
			dir.mkpath(rootPath);
		}

		QImage resultImage = image->image_;
		IMAGE2BYTEARRAY(resultImage);

		desFile = QString(UDQ_T("image_%1.%2")).arg(image->objectName()).arg(ext);
		QString desPath = QString(UDQ_T("%1/%2")).arg(rootPath).arg(desFile);
		if (ext == UDQ_T("jpg")){
			resultImage.save(desPath, "JPG");
		}
		else if (ext == UDQ_T("gif")){
			// ����ԭʼ������
			QFile file(desPath);
			if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
				file.write(image->content_);
				file.close();
			}
		}
		else{  // ����һ�ɰ�PNG����
			resultImage.save(desPath, "PNG");
		}
	}

	return desFile;
}


QString HTML5Exporter::saveEmptyImageFile(const QString& rootPath){

	// ȷ��Ŀ¼����
	QDir dir(rootPath);
	if (!dir.exists()){
		dir.mkpath(rootPath);
	}

	QImage resultImage(64, 64, QImage::Format_ARGB32);
	resultImage.fill(Qt::transparent);

	QString desFile = QString(UDQ_T("orion_empty.png"));
	QString desPath = QString(UDQ_T("%1/%2")).arg(rootPath).arg(desFile);
	resultImage.save(desPath, "PNG");

	QFileInfo info(desPath);
	emptysize_ = info.size();

	return desFile;
}

QString HTML5Exporter::saveAudioFile(ResourceHash hash, const QString& rootPath){
	QString desFile;

	DEFINE_RESOURCE_HASHKEY(AudioResourceObject, audio, hash);
	if (audio != NULL){  // �����ͼƬ
		QString ext = audio->getFormat().toLower();

		// ȷ��Ŀ¼����
		QDir dir(rootPath);
		if (!dir.exists()){
			dir.mkpath(rootPath);
		}

		desFile = QString(UDQ_T("audio_%1.%2")).arg(audio->objectName()).arg(ext);
		QString desPath = QString(UDQ_T("%1/%2")).arg(rootPath).arg(desFile);

		// �Զ����������ļ�·��
		QFile file(desPath);
		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
			file.write(audio->content_);
			file.close();
		}
	}

	return desFile;
}

QString HTML5Exporter::saveVideoFile(ResourceHash hash, const QString& rootPath){
	QString desFile;

	DEFINE_RESOURCE_HASHKEY(VideoResourceObject, video, hash);
	if (video != NULL){ 
		QString ext = video->getFormat().toLower();  // ������Դ����ԭʼ��չ��

		// ȷ��Ŀ¼����
		QDir dir(rootPath);
		if (!dir.exists()){
			dir.mkpath(rootPath);
		}

		desFile = QString(UDQ_T("video_%1.%2")).arg(video->objectName()).arg(ext);
		QString desPath = QString(UDQ_T("%1/%2")).arg(rootPath).arg(desFile);

		// �Զ����������ļ�·��
		QFile file(desPath);
		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
			file.write(video->content_);
			file.close();
		}
	}

	return desFile;
}

////////////////////////////////////////////////////////////////////////////
//
//////////////////////////////////////////////////////////////////////////

void HTML5Exporter::makeTemplateDB(const QString& file, const QString& src, bool afterDelete){

	// ����Ŀ��Ŀ¼
	QFileInfo fileInfo(file);
	QDir dir;
	if (!dir.exists(fileInfo.absolutePath()))
		dir.mkpath(fileInfo.absolutePath());

	Db db(NULL, 0);
	u_int32_t oFlags = DB_CREATE;
	try {

		QString datFile = file;
		std::string arrstr = datFile.toStdString();

		db.open(NULL,
			arrstr.c_str(),
			NULL,
			DB_BTREE,
			oFlags,
			0);

		// ѭ��ÿ��Ŀ¼�����ļ������ݿ�
		QString dirPath;
		if (dir.exists(src)){

			saveFile(db, FCT_INDEX, UDQ_T("index.html"), src);
			saveFile(db, FCT_INDEXNW, UDQ_T("indexnw.html"), src);
			saveFile(db, FCT_INDEXCORDOVA, UDQ_T("indexcordova.html"), src);

			saveFile(db, FCT_APPMANIFEST, UDQ_T("app.manifest"), src);
			saveFile(db, FCT_APPCACHE, UDQ_T("offline.appcache"), src);
			saveFile(db, FCT_RUNTIME, UDQ_T("runtime.js"), src);
			saveFile(db, FCT_JQUERY, UDQ_T("jquery.min.js"), src);
			saveFile(db, FCT_ICON16, UDQ_T("icon-16.png"), src, false);
			saveFile(db, FCT_ICON32, UDQ_T("icon-32.png"), src, false);
			saveFile(db, FCT_ICON128, UDQ_T("icon-128.png"), src, false);
			saveFile(db, FCT_ICON256, UDQ_T("icon-256.png"), src, false);
			saveFile(db, FCT_ICONLOADING, UDQ_T("loading.png"), src, false);

			saveFile(db, FCT_INDEXDEBUG, UDQ_T("indexdebug.html"), src);
			saveFile(db, FCT_RUNTIMEDEBUG, UDQ_T("runtimedebug.js"), src);
			saveFile(db, FCT_DEBUGGER, UDQ_T("debugger.js"), src);
			saveFile(db, FCT_DEBUGGER_CSS, UDQ_T("debugger.css"), src);
			saveFile(db, FCT_DEBUGGER_COLLAPSED, UDQ_T("debugger-collapsed.png"), src, false);
			saveFile(db, FCT_DEBUGGER_DEL, UDQ_T("debugger-delete.png"), src, false);
			saveFile(db, FCT_DEBUGGER_EXPANEDED, UDQ_T("debugger-expanded.png"), src, false);
			saveFile(db, FCT_DEBUGGER_INSPECT, UDQ_T("debugger-inspect.png"), src, false);
			saveFile(db, FCT_DEBUGGER_WATCH, UDQ_T("debugger-watch.png"), src, false);

			if (afterDelete){
				// ��յ�ǰĿ¼
				QDir d(dirPath);
				QStringList files = d.entryList(QDir::Files | QDir::Hidden);
				foreach(QString f, files){
					d.remove(f);
				}
			}
		}
		db.close(0);
	}
	catch (DbException &e) {
	}
	catch (std::exception &e) {
	}
}

