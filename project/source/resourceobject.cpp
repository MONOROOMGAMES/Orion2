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
**  along with Orion2.  If not, see <http://www.gnu.org/licenses/>.
**
*************************************************************************/
#include "resourceobject.h"
#include <QtGui>
#include <QMapIterator>
#include <boost/crc.hpp> 
#include <boost/cstdint.hpp>
#include "tooldefine.h"
#include "resourcelinker.h"
#include "configmanager.h"
#include "editordefine.h"
#include "imagehelper.h"
#include "mimemanager.h"
#include "componentdefine.h"
#include "logmanager.h"

namespace ORION{

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRootRTTI(ResourceObject, URT_Object, 0);

	QHash<ResourceHash, ResourceObject*> ResourceObject::objectIndex_;
	QList<ResourceObject*> ResourceObject::objects_;
	QLinkedList<ResourceLinker*> ResourceObject::linkers_;
	QLinkedList<ResourceObject*> ResourceObject::rootObjects_;
	WorkspaceResourceObject* ResourceObject::tmpWorkspace_;

	ResourceObject::ResourceObject(unsigned int flags) : hashKey_(0), objectFlags_(flags), outer_(0), linker_(0),timestamp_(0){
	}

	ResourceObject::ResourceObject(const QString& inName, unsigned int flags) : hashKey_(0), objectFlags_(flags), outer_(0), linker_(0), name_(inName),timestamp_(0){
	}

	// ��̬��ʼ��
	void ResourceObject::staticInit(){

		QString basePath = gConfigManager->basePackagePath();
		// ���ػ����زİ�
		loadPackage(basePath, RLF_LoadIndex, true);

		// ������·���������а�
		QStringList paths = gConfigManager->packagePaths();
		foreach(QString path, paths) {
			// ����Ŀ¼
			QDir dir;
			dir.setFilter(QDir::Dirs | QDir::NoDotAndDotDot);
			foreach(QFileInfo info, dir.entryInfoList()) {

				QStringList config;
				if (PackageResourceObject::loadConfig(info.absoluteFilePath(), config)){
					Q_ASSERT(config.size() >= 3);
					// �����زİ�
					loadPackage(QString(UDQ_T("%1@%2")).arg(config.at(0)).arg(config.at(2)), RLF_LoadIndex);
				}
			}
		}

		//  ������ʱ����( Ψһ )
		tmpWorkspace_ = createWorkspace(DEFAULT_WORKSPACE, true);

	}

	// ��̬�˳�
	void ResourceObject::staticExit(){
		// ��־���ж��󲻿ɷ���
		for (ResourceObjectIterator it; it; ++it)
			it->setFlags(URF_TagGarbage);

		purgeGarbage();
		objects_.clear();
		objectIndex_.clear();
		linkers_.clear();
		rootObjects_.clear();
	}

	void ResourceObject::unhashObject(){
		objectIndex_.remove(hashKey_);
	}

	void ResourceObject::rename(const QString& inName){
		unhashObject();
		name_ = inName;
		hashObject();
	}

	// ����+·����
	QString ResourceObject::fullName() const{
		QString result = getRTTI()->getName();
		result += (UDQ_T("@") + pathName());
		return result;
	}

	// ��ȡʵ��·������������Χ����,��ʽ����Parent.child��
	QString ResourceObject::pathName(ResourceObject* stopOuter) const{
		QString result;
		if (this != stopOuter){
			// �ݹ��ȡ������
			if (outer_ != stopOuter)
				result = outer_->pathName(stopOuter) + UDQ_T(".");
			// ׷����������
			result = result + objectName();
		}
		else
			result = UDQ_T("None");
		return result;
	}

	void ResourceObject::hashObject(){
		if (name_.isEmpty()){
			name_ = makeUniqueObjectName(outer_, this);
		}
		hashKey_ = hashString(fullName());
		objectIndex_.insert(hashKey_, this);
	}

	// ��Ӷ���ʵ��
	void ResourceObject::addObject(){
		objects_.append(this);
		hashObject();
	}

	// ����hash�Ӷ���
	void ResourceObject::hashChildren(){
		for (TResourceObjectIterator<ResourceObject> it; it; ++it){
			ResourceObject* child = (ResourceObject*)(*it);
			if (child->hasOuter(this)){
				child->setFlags(URF_TagSave);
				// ����hash
				child->unhashObject();
				child->hashObject();
			}
		}
	}


	QString ResourceObject::makeUniqueObjectName(const QString& path, const QString& baseName, ResourceObject* cls){
		QString newPre = path;
		if (!newPre.isEmpty())
			newPre.append(UDQ_T("."));

		QString tmpStringName;
		QString tmpPath;
		do{
			tmpStringName = QString(UDQ_T("%1%2")).arg(baseName).arg(cls->objectId());
			tmpPath = QString(UDQ_T("%1@%2%3")).arg(cls->getRTTI()->getName()).arg(newPre).arg(tmpStringName);
		} while (findResource(hashString(tmpPath))); // ��������ظ����������ۼ�id

		return tmpStringName;
	}


	QString ResourceObject::makeUniqueObjectName(ResourceObject* inOuter, ResourceObject* cls){
		QString path = inOuter ? inOuter->pathName() : UDQ_T("");
		QString baseName = cls->getRTTI()->getName();
		return makeUniqueObjectName(path, baseName.replace(UDQ_T("ResourceObject"), UDQ_T("")), cls);
	}

	ResourceObject* ResourceObject::findResource(ResourceObject* inOuter, const QString& resPath, bool exactClass){

		if (resPath.isEmpty()){
			return inOuter;
		}
		QString typeString = resPath.section(UDQ_T("@"), 0, 0);
		Q_ASSERT(!typeString.isEmpty());
		// ���Ұ�
		ResourceObject* objResult = findResource(hashString(resPath));
		if (objResult != NULL){
			if ((exactClass && (typeString != objResult->getRTTI()->getName() || objResult->outer() != inOuter)) || objResult->flags() & URF_TagGarbage){
				// ����ʧ��
				return NULL;
			}
		}
		return objResult;
	}

	ResourceObject* ResourceObject::findResource(ResourceHash hashkey){
		if (hashkey != 0 && objectIndex_.contains(hashkey)){
			return objectIndex_[hashkey];
		}
		return NULL;
	}

	// ��Ӻ���
	ResourceObject* ResourceObject::addChild(const QString& inName, const ResourceRTTI* RTTI){
		QString childPath = QString(UDQ_T("%1@%2.%3")).arg(RTTI->getName()).arg(pathName()).arg(inName);
		ResourceObject* child = findResource(hashString(childPath));
		if (child == 0){
			child = createChild(inName, RTTI->getTypeFlag());
			if (child != 0){  // ��������ɹ�
				child->setOuter(this);
				child->addObject();
			}
		}
		else{
			child->clearFlags(URF_TagGarbage);
			child->clear();
		}
		return child;
	}

	PackageResourceObject* ResourceObject::createPackage(const QString& pkgName, const QString& pkgPath, bool bNative){
		// ���Ұ��Ƿ����
		DEFINE_RESOURCE_PATH(PackageResourceObject, pkgObj, pkgName);
		if (pkgObj == 0){
			// ���Ŀ¼�Ƿ����
			QDir dir(pkgPath);
			if (!dir.exists()){
				// ����Ŀ¼
				dir.mkpath(pkgPath);
			}

			pkgObj = new PackageResourceObject(pkgName);
			pkgObj->dirPath_ = pkgPath;

			if (bNative)
				pkgObj->setFlags(URF_Native);

			pkgObj->addObject();
			rootObjects_.push_back(pkgObj);
		}
		return pkgObj;
	}

	WorkspaceResourceObject* ResourceObject::createWorkspace(const QString& wsName, bool temp){
		// �����Ƿ����
		DEFINE_RESOURCE_PATH(WorkspaceResourceObject, wsObj, wsName);
		if (wsObj == 0){
			if (temp){
				wsObj = new WorkspaceResourceObject(wsName, URF_Transient | URF_Public | URF_Native);
			}
			else{
				wsObj = new WorkspaceResourceObject(wsName);
			}
			wsObj->addObject();
			rootObjects_.push_back(wsObj);
		}
		return wsObj;
	}

	// ���ع����ռ�
	WorkspaceResourceObject* ResourceObject::loadWorkspace(const QString& wsPath, unsigned int loadFlags){
		WorkspaceResourceObject* ws = createWorkspace(wsPath);
		Q_ASSERT(ws != 0);
		// ��ȡ̬��
		loadWorkspace(ws, loadFlags);
		return ws;
	}

	void ResourceObject::loadWorkspace(WorkspaceResourceObject* ws, unsigned int loadFlags){

		// �������й���
		{
			QString file = ws->objectName() + UDQ_T(".") + ProjectResourceObject::staticFileType();
			ResourceLinker* linker = ResourceObject::getWorkspaceLinker(ws, file);
			if (linker){
				linker->loadAll<ProjectResourceObject>(loadFlags);
			}
		}
	}


	void ResourceObject::setLinker(ResourceLinker* inLinker){
		// �뵱ǰ�������ѹ�
		if (linker_){
			linker_->detach(this);
		}

		// ������������
		linker_ = inLinker;

		if (linker_)
			linker_->attach(this);
	}

	//��չ����ռ�
	void ResourceObject::resetWorkspace(WorkspaceResourceObject* inOuter){


		for (TResourceObjectIterator<ProjectResourceObject> it; it; ++it){
			ProjectResourceObject* proj = (ProjectResourceObject*)(*it);
			if (proj->getOutermost() == inOuter){
				proj->setFlags(URF_TagGarbage);
				proj->unhashObject();
				proj->setLinker(0);
			}
		}

		inOuter->setFlags(URF_TagGarbage);
		inOuter->unhashObject();
		inOuter->setLinker(0);

		purgeGarbage();
	}

	WorkspaceResourceObject*  ResourceObject::currentWorkspace(){
		// ����һ�������ռ�����Ч��
		for (TResourceObjectIterator<WorkspaceResourceObject> it; it; ++it){
			WorkspaceResourceObject* ws = (WorkspaceResourceObject*)(*it);
			if (!(ws->flags() & (URF_TagGarbage | URF_Native)))
				return ws;
		}
		return NULL;
	}

	//  ��ʱ�����ռ�
	WorkspaceResourceObject* ResourceObject::tempWorkspace(){
		return tmpWorkspace_;
	}



	// ��ȡ�����ռ�������
	ResourceLinker* ResourceObject::getWorkspaceLinker(ResourceObject* inOuter, const QString& inFilename){
		ResourceLinker* linker = NULL;
		QLinkedListIterator<ResourceLinker*> iter(linkers_);
		while (iter.hasNext()){
			ResourceLinker* linker = iter.next();
			if (linker->linkerRoot() == inOuter && linker->filePath() == inFilename){
				linker = linker;
				break;
			}
		}

		if (linker == NULL){
			// ��ȡ�ļ�����("Workspace.Type")
			QString wsName = inFilename.section(UDQ_T("."), 0, 0);
			QString fileType = inFilename.section(UDQ_T("."), -1, -1);

			DEFINE_RESOURCE_PATH(WorkspaceResourceObject, ws, wsName);
			Q_ASSERT(ws != NULL);

			QString filePath = QString(UDQ_T("%1/%2.db"))
				.arg(ws->dirPath_)
				.arg(fileType);

			linker = new ResourceLinker(inOuter, filePath);
			linker->addObject();
			linkers_.push_back(linker);
		}
		return linker;
	}


	ResourceLinker* ResourceObject::getPackageLinker(ResourceObject* inOuter, const QString& inFilename){
		ResourceLinker* linker = NULL;
		QLinkedListIterator<ResourceLinker*> iter(linkers_);
		while (iter.hasNext()){
			ResourceLinker* linker = iter.next();
			if (linker->linkerRoot() == inOuter && linker->filePath() == inFilename){
				linker = linker;
				break;
			}
		}

		if (linker == NULL){
			// ��ȡ�ļ�����("Package.Path")
			QString pkgName = inFilename.section(UDQ_T("."), 0, 0);
			QString fileType = inFilename.section(UDQ_T("."), -1, -1);

			DEFINE_RESOURCE_PATH(PackageResourceObject, pkg, pkgName);
			Q_ASSERT(pkg != NULL);

			QString filePath = QString(UDQ_T("%1/%2.db"))
				.arg(pkg->dirPath_)
				.arg(fileType);

			linker = new ResourceLinker(inOuter, filePath);
			linker->addObject();
			linkers_.push_back(linker);
		}
		return linker;
	}

	template<class T>
	void loadResource(ResourceObject* pkg, unsigned int loadFlags){
		QString file = pkg->objectName() + UDQ_T(".") + T::staticFileType();
		ResourceLinker* linker = ResourceObject::getPackageLinker(pkg, file);
		if (linker){
			if (!(loadFlags & RLF_Verify))
				linker->loadAll<T>(loadFlags);
		}
	}

	// ������Դ��
	ResourceObject* ResourceObject::loadPackage(const QString& pkgPath, unsigned int loadFlags, bool bNative){
		if (pkgPath.isEmpty())
			return NULL;

		// ��pkgPath��ȡ����
		int index = pkgPath.indexOf(UDQ_C('@'));
		if (index <= 0)
			return NULL;

		QString pkgName = pkgPath.left(index);
		QString pkgFilePath = pkgPath.right(pkgPath.length() - index - 1);

		PackageResourceObject* package = createPackage(pkgName, pkgFilePath, bNative);
		Q_ASSERT(package != 0);
		package->dirPath_ = pkgFilePath;
		package->saveConfig();

		// ��ȡ��Դ
		loadResource<ImageResourceObject>(package, loadFlags);
		loadResource<AudioResourceObject>(package, loadFlags);
		loadResource<VideoResourceObject>(package, loadFlags);
		loadResource<SoundResourceObject>(package, loadFlags);
		loadResource<MovieResourceObject>(package, loadFlags);

		loadResource<SpriteResourceObject>(package, loadFlags);
		loadResource<PluginResourceObject>(package, loadFlags);

		return package;
	}

	void ResourceObject::purgeGarbage(){
		// ������в��ɴ����
		for (int i = 0; i < objects_.size(); i++){
			if (objects_[i] && (objects_[i]->flags() & URF_TagGarbage)){
				objectIndex_.remove(objects_[i]->hashKey());
				delete objects_[i];
				objects_[i] = 0;
			}
		}
	}

	void ResourceObject::encode(QDataStream& stream){
		unsigned int ver = PACKAGE_RESOURCE_VERSON;
		stream << ver;   // ����ʱ�Զ�����Ϊ���°汾
		stream << hashKey_;
		stream << name_;
		stream << description_;

		// ÿ�α���ʱ����ʱ��
		timestamp_ = QDateTime::currentDateTimeUtc().toMSecsSinceEpoch();
		stream << timestamp_;
	}

	bool ResourceObject::decode(QDataStream& stream){
		if (!checkVersion(stream))
			return false;
		stream >> hashKey_;
		stream >> name_;
		stream >> description_;
		stream >> timestamp_;
	
		return true;
	}

	bool ResourceObject::checkVersion(QDataStream& stream){
		stream >> version_;
		return (version_ <= PACKAGE_RESOURCE_VERSON);  // ���¼���
	}


	void ResourceObject::copyObject(ResourceObject* obj){
		if (this == obj)
			return;

		obj->description_ = description_;
		obj->timestamp_ = timestamp_;
	}
	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(PackageResourceObject, URT_Package, ResourceObject, URT_Image | URT_Video | URT_Audio | URT_Sound | URT_Movie | URT_Sprite | URT_Linker | URT_Plugin );

	PackageResourceObject::PackageResourceObject(const QString& inName, unsigned int flags) : ResourceObject(inName, flags){

	}

	void PackageResourceObject::reload(){

	}

	ResourceObject* PackageResourceObject::createChild(const QString& inName, RTTIType childType){
		if (!(childType & allowChildFlags_))
			return NULL;

		ResourceObject* child = NULL;

		if (childType == URT_Image){
			child = new ImageResourceObject(inName);
		}
		else if (childType == URT_Video){
			child = new VideoResourceObject(inName);
		}
		else if (childType == URT_Audio){
			child = new AudioResourceObject(inName);
		}
		else if (childType == URT_Sound ){
			child = new SoundResourceObject(inName);
		}
		else if (childType == URT_Movie){
			child = new MovieResourceObject(inName);
		}
		else if (childType == URT_Sprite){
			child = new SpriteResourceObject(inName);
		}
		else if (childType == URT_Plugin){
			child = new PluginResourceObject(inName);
		}
		return child;
	}

	// ������Ϣ��д
	bool PackageResourceObject::saveConfig(){

		if (isNative())
			return false;

		// ȷ��Ŀ¼����
		QDir dir(dirPath_);
		if (!dir.exists()){
			dir.mkpath(dirPath_);
		}

		QByteArray data;
		QDataStream stream(&data, QIODevice::WriteOnly);
		stream << objectName();
		stream << description();
		stream << dirPath_;

		QFile file(QString(UDQ_T("%1/package.idx")).arg(dirPath_));
		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
			file.write(data);
			file.close();
		}

		return true;
	}

	bool PackageResourceObject::loadConfig(const QString& path, QStringList& config){
		// ȷ���ļ�����
		QFile file(QString(UDQ_T("%1/package.idx")).arg(path));
		if (!file.exists()){
			return false;
		}
		QByteArray data;
		if (file.open(QIODevice::ReadOnly)){
			data = file.readAll();
			file.close();
		}

		config.clear();

		QString tmpStr;
		QDataStream stream(&data, QIODevice::ReadOnly);
		stream >> tmpStr;  // ����
		config.append(tmpStr);

		stream >> tmpStr;  // ����
		config.append(tmpStr);

		stream >> tmpStr;  // ����·��
		config.append(tmpStr);

		return true;

	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	ImplementRTTI(FieldResourceObject, URT_Field, ResourceObject, 0);

	FieldResourceObject::FieldResourceObject(const QString& inName, unsigned int flags) : ResourceObject(inName, flags) {

	}

	void FieldResourceObject::encode(QDataStream& stream){
		ResourceObject::encode(stream);
		stream << categories_;
		stream << tags_;
	}

	bool FieldResourceObject::decode(QDataStream& stream){
		if (!ResourceObject::decode(stream))
			return false;
		stream >> categories_;
		stream >> tags_;

		categories_.removeDuplicates();
		return true;
	}


	bool FieldResourceObject::isBelong(const QString& category){
		if (category.isEmpty() && categories_.size() == 0){
			return true;
		}
		return categories_.contains(category);
	}

	bool FieldResourceObject::hasTag(const QString& tag, bool exactlyMatch){
		if (tag.isEmpty())
			return true;

		if (exactlyMatch){
			return tags_.contains(tag, Qt::CaseInsensitive);  // ��Сд������
		}
		else{
			foreach(QString t, tags_){
				if (t.contains(tag, Qt::CaseInsensitive)){
					return true;
				}
			}
		}
		return false;
	}

	void FieldResourceObject::copyObject(ResourceObject* obj){
		ResourceObject::copyObject(obj);

		FieldResourceObject* field = UDynamicCast(FieldResourceObject, obj);
		if (field){
			field->categories_ = categories_;  // λ��
			field->tags_ = tags_;  //
		}

	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	ImplementRTTI(NullResourceObject, URT_NullObject, FieldResourceObject, 0);

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(GalleryResourceObject, URT_Gallery, FieldResourceObject, 0);

	GalleryResourceObject::GalleryResourceObject(const QString& inName, unsigned int flags) : FieldResourceObject(inName, flags){

	}

	void GalleryResourceObject::encode(QDataStream& stream){
		FieldResourceObject::encode(stream);
		stream << thumbnail_;
		stream << content_;
		stream << format_;

	}

	bool GalleryResourceObject::decode(QDataStream& stream){
		if (!FieldResourceObject::decode(stream))
			return false;
		stream >> thumbnail_;
		stream >> content_;
		stream >> format_;
		hashsha1_ = QCryptographicHash::hash(content_, QCryptographicHash::Sha1);

		return true;
	}

	// ��������ͼ
	QString  GalleryResourceObject::fileFilters(){
		QString filters;

		filters.append(ImageResourceObject::fileFilter());
		filters.append(UDQ_T(" ;;"));
		filters.append(AudioResourceObject::fileFilter());
		filters.append(UDQ_T(" ;;"));
		filters.append(VideoResourceObject::fileFilter());
		return filters;
	}

	// ��������ͼ
	void  GalleryResourceObject::makeThumbnail(int size){
		// ����ȱʡ����ͼ
	}

	void GalleryResourceObject::setContent(const QByteArray& data){
		content_ = data;
		hashsha1_ = QCryptographicHash::hash(content_, QCryptographicHash::Sha1);
	}

	void GalleryResourceObject::copyObject(ResourceObject* obj){
		FieldResourceObject::copyObject(obj);

		GalleryResourceObject* gallery = UDynamicCast(GalleryResourceObject, obj);
		if (gallery){
			gallery->thumbnail_ = thumbnail_;  // λ��
			gallery->setContent(content_);  //
			gallery->format_ = format_;  //
		}
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(ImageResourceObject, URT_Image, GalleryResourceObject, 0);
	static QStringList Filter_Image = QStringList() << UDQ_T("png") << UDQ_T("jpg") << UDQ_T("gif");

	ImageResourceObject::ImageResourceObject(const QString& inName, unsigned int flags) : GalleryResourceObject(inName, flags),isGroup_(false){

	}

	QString  ImageResourceObject::fileFilter(){

		QString res;
		res.append(UDQ_TR("ͼƬ ("));
		foreach(QString f, Filter_Image){
			res.append(UDQ_T("*") + f + UDQ_T(" "));
		}
		res.append(UDQ_T(")"));
		return res;
	}

	QString  ImageResourceObject::getFormat(){
		if (format_.isEmpty() || !Filter_Image.contains(format_)){
			format_ = UDQ_T("png");
		}
		return format_;
	}

	void ImageResourceObject::encode(QDataStream& stream){
		GalleryResourceObject::encode(stream);
		stream << isGroup_;
		stream << frames_;
	}

	// ͼƬ��Ŀ
	int ImageResourceObject::imageCount(){
		if (isGroup_){
			return frames_.size();
		}

		return 1;
	}

	// ��ȡ��ͼƬ
	QImage ImageResourceObject::getImage(int index){
		if (!isGroup_){
			return image_;
		}

		if (image_.isNull()){
			return QImage();
		}

		QRect r = frames_[index];
		return image_.copy(r);
	}


	bool ImageResourceObject::decode(QDataStream& stream){
		if (!GalleryResourceObject::decode(stream))
			return false;

		image_ = QImage::fromData(content_);
		makeThumbnail();

		if (version_ >= PACKAGE_RESOURCE_ADD_IMAGEGROUP){
			stream >> isGroup_;
			stream >> frames_;
		}
		else{
			isGroup_ = false;
			frames_.clear();
		}

		if (!isGroup_ && frames_.size() == 0){
			frames_.append(QRect(0, 0, image_.width(), image_.height()));
		}

		return true;
	}


	// �ж��ļ���չ�����Ժ��Ǹ����ļ����ݣ�
	bool ImageResourceObject::isValidFile(const QString& ext){
		return Filter_Image.contains(ext.toLower());
	}

	// ��������ͼ
	void  ImageResourceObject::makeThumbnail(int size){
		thumbnail_ = QPixmap::fromImage(thnumImage(size, size, image_));
	}

	void ImageResourceObject::copyObject(ResourceObject* obj){
		GalleryResourceObject::copyObject(obj);

		ImageResourceObject* img = UDynamicCast(ImageResourceObject, obj);
		if (img){
			img->image_ = image_;  // λ��
		}
	}


	// ������ͼƬ
	void ImageResourceObject::setImage(QImage& newImage){
		content_.clear();
		QBuffer buffer(&content_);
		buffer.open(QIODevice::WriteOnly);
		newImage.save(&buffer, "PNG");

		image_ = QImage::fromData(content_);
		makeThumbnail();
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(AudioResourceObject, URT_Audio, GalleryResourceObject, 0);

	static QStringList Filter_Audio = QStringList() << UDQ_T("mp3") << UDQ_T("wav") << UDQ_T("ogg") << UDQ_T("m4a");

	AudioResourceObject::AudioResourceObject(const QString& inName, unsigned int flags) : GalleryResourceObject(inName, flags){

	}

	QString  AudioResourceObject::fileFilter(){
		QString res;
		res.append(UDQ_TR("��Ƶ ("));
		foreach(QString f, Filter_Audio){
			res.append(UDQ_T("*") + f + UDQ_T(" "));
		}
		res.append(UDQ_T(")"));
		return res;
	}

	// �ж��ļ���չ�����Ժ��Ǹ����ļ����ݣ�
	bool AudioResourceObject::isValidFile(const QString& ext){
		return Filter_Audio.contains(ext.toLower());
	}

	QString  AudioResourceObject::getFormat(){
		if (format_.isEmpty() || !Filter_Audio.contains(format_)){
			format_ = UDQ_T("mp3");
		}
		return format_;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(VideoResourceObject, URT_Video, GalleryResourceObject, 0);

	static QStringList Filter_Video = QStringList() << UDQ_T("mp4") << UDQ_T("ogv") << UDQ_T("webm");
	VideoResourceObject::VideoResourceObject(const QString& inName, unsigned int flags) : GalleryResourceObject(inName, flags){

	}

	QString  VideoResourceObject::fileFilter(){
		QString res;
		res.append(UDQ_TR("��Ƶ ("));
		foreach(QString f, Filter_Video){
			res.append(UDQ_T("*") + f + UDQ_T(" "));
		}
		res.append(UDQ_T(")"));
		return res;
	}

	// �ж��ļ���չ�����Ժ��Ǹ����ļ����ݣ�
	bool VideoResourceObject::isValidFile(const QString& ext){
		return Filter_Video.contains(ext.toLower());
	}

	QString  VideoResourceObject::getFormat(){
		if (format_.isEmpty() || !Filter_Video.contains(format_)){
			format_ = UDQ_T("mp4");
		}
		return format_;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(SoundResourceObject, URT_Sound, FieldResourceObject, 0);

	SoundResourceObject::SoundResourceObject(const QString& inName, unsigned int flags) : FieldResourceObject(inName, flags){

	}

	QStringList SoundResourceObject::formats(){
		QStringList types;
		foreach(ResourceHash key, audios_){
			DEFINE_RESOURCE_HASHKEY(AudioResourceObject, audio, key);
			if (audio){
				types.append(audio->getFormat());
			}
		}
		return types;
	}

	void SoundResourceObject::encode(QDataStream& stream){
		FieldResourceObject::encode(stream);
		stream << audios_;
	}

	bool SoundResourceObject::decode(QDataStream& stream){
		if (!FieldResourceObject::decode(stream))
			return false;

		stream >> audios_;
		return true;
	}
	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(MovieResourceObject, URT_Movie, FieldResourceObject, 0);

	MovieResourceObject::MovieResourceObject(const QString& inName, unsigned int flags) : FieldResourceObject(inName, flags){

	}

	QStringList MovieResourceObject::formats(){
		QStringList types;
		foreach(ResourceHash key, videos_){
			DEFINE_RESOURCE_HASHKEY(VideoResourceObject, video, key);
			if (video){
				types.append(video->getFormat());
			}
		}
		return types;
	}

	void MovieResourceObject::encode(QDataStream& stream){
		FieldResourceObject::encode(stream);
		stream << videos_;
	}

	bool MovieResourceObject::decode(QDataStream& stream){
		if (!FieldResourceObject::decode(stream))
			return false;

		stream >> videos_;
		return true;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(SpriteResourceObject, URT_Sprite, FieldResourceObject, URT_SpriteSequence);

	SpriteResourceObject::SpriteResourceObject(const QString& inName, unsigned int flags) : FieldResourceObject(inName, flags){
		defaultSeq_ = UDQ_T("Default");
	}

	void SpriteResourceObject::encode(QDataStream& stream){
		FieldResourceObject::encode(stream);

		stream << viewData_;
		stream << imgSources_;
		stream << defaultSeq_;

		stream << sequences_.size();
		foreach(SpriteSequenceResourceObject* seq, sequences_){
			QByteArray data;
			{
				QDataStream stream(&data, QIODevice::WriteOnly);
				seq->encode(stream);
			}
			stream << data;
		}

	}

	bool SpriteResourceObject::decode(QDataStream& stream){
		if (!FieldResourceObject::decode(stream))
			return false;

		stream >> viewData_;
		stream >> imgSources_;

		if (version_ >= PACKAGE_RESOURCE_ADD_DEFAULTSEQUENCE){
			stream >> defaultSeq_;
		}
		else{
			defaultSeq_ = UDQ_T("Default");
		}

		int seqnum;
		stream >> seqnum;
		QByteArray data;
		for (int i = 0; i < seqnum; i++){
			stream >> data;
			{
				QDataStream stream(data);
				if (!decodeChild<SpriteSequenceResourceObject>(stream)){
					break;
				}				
			}
		}

		// ��ȡ�б�
		for (TResourceObjectIterator<SpriteSequenceResourceObject> it; it; ++it){
			SpriteSequenceResourceObject* seq = (SpriteSequenceResourceObject*)(*it);
			if ( (seq->flags() & URF_TagGarbage) || seq->outer() != this )
				continue;
			sequences_.append(seq);
		}

		return true;
	}

	ResourceObject* 	SpriteResourceObject::createChild(const QString& inName, RTTIType childType){

		if (!(childType & allowChildFlags_))
			return NULL;

		ResourceObject* child = NULL;

		if (childType == URT_SpriteSequence){
			child = new SpriteSequenceResourceObject(inName);
			sequences_.append((SpriteSequenceResourceObject*)child);
		}

		return child;

	}

	void SpriteResourceObject::saveViewPort(int lod, const QTransform& trans, const QPointF& center){
		QDataStream stream(&viewData_, QIODevice::WriteOnly | QIODevice::Truncate);
		stream << lod << trans << center;
	}

	void SpriteResourceObject::addImageSource(ResourceHash id, ResourceHash key ){
		imgSources_.insert(id, key);
	}

	void SpriteResourceObject::removeImageSource(ResourceHash id){
		imgSources_.remove(id);
	}

	void SpriteResourceObject::copyObject(ResourceObject* obj){
		FieldResourceObject::copyObject(obj);

		SpriteResourceObject* sprite = UDynamicCast(SpriteResourceObject, obj);
		if (sprite){
			sprite->viewData_ = viewData_;  // �ӿ���Ϣ
			sprite->imgSources_ = imgSources_;  // ͼƬ��Դ

			// ����
			foreach(SpriteSequenceResourceObject* seq, sequences_){
				// ���浽���ݿ���
				SpriteSequenceResourceObject* newSeq = TAddChildObject<SpriteSequenceResourceObject>(sprite, seq->objectName());
				Q_ASSERT(newSeq != 0);
				seq->copyObject(newSeq);
			}

			sprite->setFlags(URF_TagSave);
		}
	}

	ResourceObject* SpriteResourceObject::clone(const QString& inName){
		// ���浽���ݿ���
		SpriteResourceObject* sprite = TCreateResource<SpriteResourceObject>(getOutermost(), inName);
		Q_ASSERT(sprite != 0);

		copyObject(sprite);
		sprite->setFlags(URF_TagSave);

		return sprite;
	}

	SpriteSequenceResourceObject* SpriteResourceObject::sequence(const QString& name){
		SpriteSequenceResourceObject* seq = NULL;
		for (int i = 0, num = sequences_.size(); i < num; i++){
			if (sequences_[i]->objectName() == name){
				seq = sequences_[i];
				break;
			}
		}
		return seq;
	}

	QRect SpriteResourceObject::frameRect(ResourceHash id, int index){
		QRect rect;
		ResourceHash  key= imgSources_.value(id);
		DEFINE_RESOURCE_HASHKEY(ImageResourceObject, image, key);
		if (image){
			if (index >= 0 && index < image->imageCount()){
				rect = image->frames_[index];
			}
		}
		return rect;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(SpriteSequenceResourceObject, URT_SpriteSequence, ResourceObject, 0);

	SpriteSequenceResourceObject::SpriteSequenceResourceObject(const QString& inName, unsigned int flags) : ResourceObject(inName, flags){
		isLocking_ = false;
		speed_ = 0.5f;  //30FPS
		playback_ = false;
		pingpong_ = false;
		repeat_ = false;  // �ظ�����
		count_ = 1;  // �ظ�����
		repeatTo_ = 0;  // ���Ž���ֹͣ֡��
		anchors_.append(ANCHOR_CENTER);  // Ĭ�ϱ����������ê��
	}

	void SpriteSequenceResourceObject::encode(QDataStream& stream){
		ResourceObject::encode(stream);

		stream << pos_;
		stream << isLocking_;
		stream << speed_;
		stream << playback_;
		stream << anchors_;
		stream << pingpong_;
		stream << repeat_; 
		stream << count_;
		stream << repeatTo_;
		stream << frames_.toByteArray();

	}

	bool SpriteSequenceResourceObject::decode(QDataStream& stream){
		if (!ResourceObject::decode(stream))
			return false;

		stream >> pos_;
		stream >> isLocking_;
		stream >> speed_;
		stream >> playback_;
		stream >> anchors_;
		stream >> pingpong_;
		stream >> repeat_;
		stream >> count_;
		stream >> repeatTo_;
	
		QByteArray data;
		stream >> data;
		frames_.fromByteArray(data);

		foreach(SequenceFrame* f, frames_.values()){
			if (f->state.size() == 0){
				f->state.fill(false, 8);
			}
		}

		return true;
	}


	QImage SpriteSequenceResourceObject::frameSourceImage(int index){
		SequenceFrame* frame = frames_.at(index);
		return frameSourceImage(frame);
	}

	QImage SpriteSequenceResourceObject::frameSourceImage(SequenceFrame* frame){
		if (frame == NULL)
			return QImage();

		SpriteResourceObject* sprite = (SpriteResourceObject*)outer();
		ResourceHash key = sprite->imgSources_.value(frame->imghost);

		QImage srcImage;
		DEFINE_RESOURCE_HASHKEY(ImageResourceObject, img, key);
		if (img){
			if (frame->imgindex < img->imageCount()){
				srcImage = img->getImage(frame->imgindex);
			}
		}
		return srcImage;
	}

	void SpriteSequenceResourceObject::refreshImage(SequenceFrame* frame){
		QImage srcImage = frameSourceImage(frame);
		// Ӧ��״̬
		if (!srcImage.isNull()){
			srcImage = srcImage.mirrored(frame->state[4], frame->state[5]);
			// �ԽǷ�ת
			if (frame->state[6]){
				QTransform trans;
				trans.rotate(90);
				srcImage = srcImage.transformed(trans);
				srcImage = srcImage.mirrored(false, true);
			}
		}
		frame->buffer = srcImage;
	}

	QImage SpriteSequenceResourceObject::frameImage(int index){
		QImage img;

		if (index < frames_.size()){
			SequenceFrame* frame = frames_.at(index);
			if (frame->buffer.isNull()){
				refreshImage(frame);
			}
			img = frame->buffer;
		}
		return img;
	}

	QPointF SpriteSequenceResourceObject::frameAnchor(int index, const QString& anchor){
		QPointF pt(0.5,0.5);

		if (index < frames_.size()){
			ResourceHash key = hashString(anchor);
			SequenceFrame* frame = frames_.at(index);
			if (frame->anchors.contains(key)){
				pt = frame->anchors.value(key);
			}
			else{
				frame->anchors.insert(key, pt);  // ���û�У���ȱʡΪ���ĵ�
			}
		}

		return pt;
	}

	QPointF SpriteSequenceResourceObject::frameCenterAnchor(int index){
		return frameAnchor(index, ANCHOR_CENTER);
	}

	QList<QPointF> SpriteSequenceResourceObject::frameCollision(int index){
		QList<QPointF> pts;
		if (index < frames_.size()){
			SequenceFrame* frame = frames_.at(index);
			if (frame->collisionRegion.size() == 0){
				frame->collisionRegion.append(QPointF(0, 0));
				frame->collisionRegion.append(QPointF(1, 0));
				frame->collisionRegion.append(QPointF(1, 1));
				frame->collisionRegion.append(QPointF(0, 1));
			}
			pts = frame->collisionRegion;
		}

		return pts;
	}


	// �����¶���
	void SpriteSequenceResourceObject::copyObject(ResourceObject* obj){
		ResourceObject::copyObject(obj);

		SpriteSequenceResourceObject* seq = UDynamicCast(SpriteSequenceResourceObject, obj);
		if (seq){
			seq->pos_ = pos_;  // λ��
			seq->isLocking_ = isLocking_;  //
			seq->frames_ = frames_;  // ����֡
			seq->speed_ = speed_;  // �����ٶȣ�0~1, ���60FPS��
			seq->playback_ = playback_; // �Ƿ񵹷�
		}
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	ImplementRTTI(PluginResourceObject, URT_Plugin, FieldResourceObject, 0);

	PluginResourceObject::PluginResourceObject(const QString& inName, unsigned int flags) : FieldResourceObject(inName, flags)
	{
		eid_ = 0;
		type_ = PIT_ENTITY;
		devmode_ = false;
		commonPlugin_ = NULL;
	}

	void PluginResourceObject::encode(QDataStream& stream){
		FieldResourceObject::encode(stream);

		stream << type_;
		stream << devmode_;
		stream << title_;
		stream << modelVer_;
		stream << eid_;
		stream << extendBound_;
		stream << icon_;


		stream << attributes_;
		stream << filedata_;

		// Ŀǰ�༭����֧�ֲ���༭��ͨ���޸�JSON�ļ�ʵ�֣����������ϱ༭����
		stream << varIndex_;
		stream << variables_.toByteArray();
		stream << conditions_.toByteArray();
		stream << actions_.toByteArray();
		stream << expresses_.toByteArray();

	}

	bool PluginResourceObject::decode(QDataStream& stream){
		if (!FieldResourceObject::decode(stream))
			return false;

		stream >> type_;
		stream >> devmode_;
		stream >> title_;
		stream >> modelVer_;
		stream >> eid_;
		stream >> extendBound_;
		stream >> icon_;

		stream >> attributes_;
		stream >> filedata_;
		stream >> varIndex_;


		QByteArray data;
		stream >> data;
		variables_.fromByteArray(data);

		stream >> data;
		conditions_.fromByteArray(data);

		stream >> data;
		actions_.fromByteArray(data);

		stream >> data;
		expresses_.fromByteArray(data);

		postInit();

		return true;
	}

	void PluginResourceObject::copyObject(ResourceObject* obj){
		FieldResourceObject::copyObject(obj);

	}

	void PluginResourceObject::processParameterDefault(const QString& prefix, PluginInterface* inter){
		QString interName = prefix + inter->name + UDQ_T(".");
		foreach(PluginVariable* var, inter->params.values()){
			paramDefaults_.insert(hashString(interName + var->name), getParameterDefault(var));
		}
	}

	void PluginResourceObject::postInit(){
		paramDefaults_.clear();
		varDefaults_.clear();

		foreach(PluginVariable* var, variables_.values()){
			varDefaults_.insert(hashString(var->name), getParameterDefault(var));
		}

		foreach(PluginInterface* var, conditions_.values()){
			processParameterDefault(UDQ_T("Cnd."), var);
		}

		foreach(PluginInterface* var, actions_.values()){
			processParameterDefault(UDQ_T("Act."), var);
		}

		foreach(PluginInterface* var, expresses_.values()){
			processParameterDefault(UDQ_T("Exp."), var);
		}
	}

	// ��ȡȱʡֵ
	QString PluginResourceObject::getParameterDefault(PluginVariable* var){
		QString v = var->value;
		if (v.isEmpty()){
			// ����ȱʡֵ
			if (var->datatype.size() > 0){
				unsigned int key = hashStringLower(var->datatype);
				if (key == DATATYPE_BOOL){
					v = UDQ_T("false");
				}
				else if (key == DATATYPE_INT){
					v = UDQ_T("0");
				}
				else if (key == DATATYPE_DOUBLE){
					v = UDQ_T("0.0");
				}
				else if (key == DATATYPE_ENUM){
					//  ��ȡ��һ��ö��ֵ
					if (var->enums.size() > 0){
						v = var->enums[0];
					}
				}
			}
		}
		return v;
	}

	QString PluginResourceObject::getVariableDefault(const QString& prop){
		ResourceHash key = hashString(prop);
		if (prop.isEmpty() || !varDefaults_.contains(key)){
			return QString();
		}
		return varDefaults_.value(key);
	}

	QString PluginResourceObject::getParameterDefault(unsigned int key){
		QString v;
		if (paramDefaults_.contains(key)){
			v = paramDefaults_.value(key);
		}
		return v;
	}

	QString PluginResourceObject::getConditionParameterDefault(const QString& prop, const QString& parent){
		QString path = UDQ_T("Cnd.") + parent + UDQ_T(".") + prop;
		return getParameterDefault(hashString(path));
	}

	QString PluginResourceObject::getActionParameterDefault(const QString& prop, const QString& parent){
		QString path = UDQ_T("Act.") + parent + UDQ_T(".") + prop;
		return getParameterDefault(hashString(path));
	}

	QString PluginResourceObject::getExpressParameterDefault(const QString& prop, const QString& parent){
		QString path = UDQ_T("Exp.") + parent + UDQ_T(".") + prop;
		return getParameterDefault(hashString(path));
	}

	bool PluginResourceObject::hasAttribute(int type ){
		if (attributes_.contains(type)){
			return attributes_[type] == 1;
		}
		return false;
	}

	
	PluginVariable* PluginResourceObject::getVariable(const QString& prop){
		if (prop.isEmpty()){
			return NULL;
		}
		ResourceHash key = hashString(prop);
		return variables_.value(key);
	}

	PluginVariable* PluginResourceObject::getVariableDefine(const QString& prop, const QString& parent, TResourceMap<PluginInterface>& container){

		ResourceHash parentH = hashString(parent);
		if ( container.value(parentH)  == NULL ){
			return NULL;
		}
		PluginVariable* var = NULL;
		PluginInterface* inter = container.value(parentH);
		if (inter){
			for (int i = 0; i < inter->params.size(); i++){
				if (inter->params.at(i)->name == prop){
					var = inter->params.at(i);
					break;
				}
			}
		}
		return var;
	}


	PluginVariable* PluginResourceObject::getConditionVariable(const QString& prop, const QString& parent){
		return getVariableDefine(prop, parent, conditions_);
	}

	PluginVariable* PluginResourceObject::getActionVariable(const QString& prop, const QString& parent){
		return getVariableDefine(prop, parent, actions_);
	}

	PluginVariable* PluginResourceObject::getExpressVariable(const QString& prop, const QString& parent){
		return getVariableDefine(prop, parent, expresses_);
	}


	void PluginResourceObject::setVariableValue(const QString& name, const QString& value){
		PluginVariable* var = variables_.value(name);
		Q_ASSERT(var != NULL);
		var->value = value;
		varDefaults_[hashString(name)] = value;
	}

	const QString PREFIX_ENTITY = UDQ_T("cr.plugins_.");
	const QString PREFIX_BEHAVIOR = UDQ_T("cr.behaviors.");
	const QString NAME_SYSTEM = UDQ_TR("System");

	QString PluginResourceObject::runtimeString(PluginInterface* inter ){
		QString str;
		if (inter == NULL ){

			if (type_ == PIT_ENTITY){
				str = PREFIX_ENTITY;
			}
			else if (type_ == PIT_BEHAVIOR){
				str = PREFIX_BEHAVIOR;
			}
			else if (type_ == PIT_NATIVE && objectName() == NAME_SYSTEM){
				return UDQ_T("cr.system_object");
			}
			else{
				return QString();
			}
			str += objectName();
		}
		else{  // API�ַ���
			str = runtimeString() + UDQ_T(".prototype.");

			if (inter->type == PT_CONDITION){
				str.append(UDQ_T("cnds."));
			}
			else if (inter->type == PT_ACTION){
				str.append(UDQ_T("acts."));
			}
			else if (inter->type == PT_EXPRESS){
				str.append(UDQ_T("exps."));
			}
			else{
				Q_UNREACHABLE();
			}
			str.append(inter->func);
		}
		return str;
	}

	QString PluginResourceObject::runtimeString(int type, ResourceHash key){
		PluginInterface* inter = NULL;
		if (type == PT_CONDITION){
			inter = conditions_.value(key);
		}
		else if (type == PT_ACTION){
			inter = actions_.value(key);
		}
		else if (type == PT_EXPRESS){
			inter = expresses_.value(key);
		}

		if (inter == NULL){
			return QString();
		}

		return runtimeString(inter);
	}

	PluginResourceObject* PluginResourceObject::acquireCommonPlugin(){
		return acquirePlugin(UDQ_TR("Common"));
	}

	PluginResourceObject* PluginResourceObject::acquirePlugin(const QString& name){
		PluginResourceObject* plugin = NULL;
		// ���ҹ������
		for (TResourceObjectIterator<PluginResourceObject> it; it; ++it){
			plugin = (PluginResourceObject*)(*it);
			if (plugin->flags() & URF_TagGarbage)
				continue;
			if (plugin->objectName() == name){
				return plugin;
				break;
			}
		}
		return NULL;
	}


	PluginInterface* PluginResourceObject::getAction(ResourceHash key){
		PluginInterface* inter = actions_.value(key);
		if (inter == NULL){
			if (commonPlugin_ == NULL){
				commonPlugin_ = acquireCommonPlugin();
			}

			if (commonPlugin_){
				inter = commonPlugin_->actions_.value(key);
			}
		}
		return inter;
	}

	PluginInterface* PluginResourceObject::getCondition(ResourceHash key){
		PluginInterface* inter = conditions_.value(key);
		if (inter == NULL){
			if (commonPlugin_ == NULL){
				commonPlugin_ = acquireCommonPlugin();
			}

			if (commonPlugin_){
				inter = commonPlugin_->conditions_.value(key);
			}
		}
		return inter;

	}
	PluginInterface* PluginResourceObject::getExpress(ResourceHash key){
		PluginInterface* inter = expresses_.value(key);
		if (inter == NULL){
			if (commonPlugin_ == NULL){
				commonPlugin_ = acquireCommonPlugin();
			}

			if (commonPlugin_){
				inter = commonPlugin_->expresses_.value(key);
			}
		}
		return inter;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(WorkspaceResourceObject, URT_WorkSpace, ResourceObject, URT_Project);

	WorkspaceResourceObject::WorkspaceResourceObject(const QString& inName, unsigned int flags) : ResourceObject(inName, flags)
		, activitedH_(0){
	}

	void WorkspaceResourceObject::reset(){
		activitedH_ = 0;
	}

	// ������Ϣ��д
	bool WorkspaceResourceObject::saveConfig(){

		if (isNative())
			return false;

		// ȷ��Ŀ¼����
		QDir dir(dirPath_);
		if (!dir.exists()){
			dir.mkpath(dirPath_);
		}

		QByteArray data;
		QDataStream stream(&data, QIODevice::WriteOnly);
		stream << objectName();
		stream << description();
		stream << projectType_;

		QFile file(QString(UDQ_T("%1/project.cfg")).arg(dirPath_));
		if (file.open(QIODevice::WriteOnly | QIODevice::Truncate)){
			file.write(data);
			file.close();
		}

		return true;
	}

	bool WorkspaceResourceObject::loadConfig(){
		// ȷ���ļ�����
		QFile file(QString(UDQ_T("%1/project.cfg")).arg(dirPath_));
		if (!file.exists()){
			return false;
		}
		QByteArray data;
		if (file.open(QIODevice::ReadOnly)){
			data = file.readAll();
			file.close();
		}

		QDataStream stream(&data, QIODevice::ReadOnly);
		stream >> tmpName_;
		if (!isNative() && tmpName_ != objectName()){
			// ��������
			rename(tmpName_);
		}
		stream >> description_;
		stream >> projectType_;

		return true;

	}

	ResourceObject* WorkspaceResourceObject::createChild(const QString& inName, RTTIType childType){
		if (!(childType & allowChildFlags_))
			return NULL;

		ResourceObject* child = NULL;
		if (childType == URT_Project){
			child = new ProjectResourceObject(inName);
		}

		return child;
	}

	ProjectResourceObject* WorkspaceResourceObject::currentProject(){

		if (activitedH_ == 0)
			return NULL;

		ProjectResourceObject* proj = (ProjectResourceObject*)ResourceObject::findResource(activitedH_);
		Q_ASSERT(proj != 0);

		return proj;
	}

	ProjectResourceObject* WorkspaceResourceObject::findProject(const QString& inName, bool create){
		ProjectResourceObject* proj = NULL;
		// ����һ�������ռ�����Ч��
		for (TResourceObjectIterator<ProjectResourceObject> it; it; ++it){
			proj = (ProjectResourceObject*)(*it);
			if (proj->outer() && proj->outer() == this  && proj->objectName() == inName){

				if (create && (proj->flags() & URF_TagGarbage)){		// ���book���Ѿ�ɾ���ģ���ָ�
					proj->clearFlags(URF_TagGarbage);
					proj->setFlags(URF_TagSave);
				}
				return  (proj->flags() & URF_TagGarbage) ? NULL : proj;
			}
		}

		// ���û���򴴽�
		if (create){
			proj = TAddChildObject<ProjectResourceObject>(this, inName);
		}
		return proj;
	}

	void WorkspaceResourceObject::activiteProject(ResourceHash hash){

		activitedH_ = hash;
		if (activitedH_ == 0)
			return;

		ProjectResourceObject* proj = (ProjectResourceObject*)ResourceObject::findResource(activitedH_);
		Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));

		// ���̬�Ƽ������
		if (proj->flags() & URF_NeedLoad){
			// ������������
			ResourceLinker* linker = proj->getLinker();
			Q_ASSERT(linker != 0);
			linker->load<ProjectResourceObject>(proj->hashKey(), RLF_None);
		}
	}


	void WorkspaceResourceObject::deleteProject(ResourceHash hash){

		ProjectResourceObject* proj = (ProjectResourceObject*)ResourceObject::findResource(hash);
		Q_ASSERT(proj != 0 && !(proj->flags()  & URF_TagGarbage));
		Q_ASSERT(activitedH_ != proj->hashKey());

		proj->setFlags(URF_TagGarbage);
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(ProjectResourceObject, URT_Project, ResourceObject, URT_Level | URT_Layer | URT_ObjectType | URT_Blueprint | URT_Family );

	ProjectResourceObject::ProjectResourceObject(const QString& inName, unsigned int flags) : ResourceObject(inName, flags),
		leveltreeItem_(NULL), currentLayer_(NULL),
		createDate_(QDateTime::currentDateTime()), exportType_(0),
		lastModifyDate_(QDateTime::currentDateTime()), minfyScript_(true)
	{
	}

	void ProjectResourceObject::encode(QDataStream& stream){

		ResourceObject::encode(stream);

		stream << createDate_;
		stream << lastModifyDate_;
		stream << author_;
		stream << pubVersion_;
		stream << email_;
		stream << website_;
		stream << domainId_;

		stream << thumbnail_;
		stream << resolution_;
		stream << aspect_;
		stream << startLevel_;

		stream << fullscreenMode_;  // ȫ��ģʽ
		stream << fullscreenQuality_;  // ��������
		stream << useHighDPI_;
		stream << sampleType_;
		stream << downScaling_;  // ��С
		stream << physicalEngine_;  // ��������
		stream << loaderStyle_;  // ����ҳ����ʽ
		stream << preloadSound_; //  Ԥ��������
		stream << pauseOnUnfocus_;  // ʧȥ������ֹͣ����
		stream << clearBackground_;  // �Ƿ�����������رտ����Ч�ʣ�

		stream << useLoaderLayout_;
		stream << useWebGL_;
		stream << pixelRounding_;
		stream << orientation_;

		stream << exportDir_;
		stream << mediaDirs_;
		stream << minfyScript_;

		stream << objectGroups_;

		// ��������
		stream << objTypes_.size();
		foreach(ObjectTypeResourceObject* type, objTypes_)
		{
			if (type->flags() & URF_TagGarbage)
				continue;

			QByteArray dat;
			{
				QDataStream stream(&dat, QIODevice::WriteOnly);
				type->encode(stream);
			}
			stream << dat;
		}

		// ����
		stream << families_.size();
		foreach(FamilyResourceObject* family, families_)
		{
			QByteArray dat;
			{
				QDataStream stream(&dat, QIODevice::WriteOnly);
				family->encode(stream);
			}
			stream << dat;
		}

		// ����
		stream << sounds_.size();
		foreach(SoundResourceObject* sound, sounds_)
		{
			QByteArray dat;
			{
				QDataStream stream(&dat, QIODevice::WriteOnly);
				sound->encode(stream);
			}
			stream << dat;
		}

		// ��Ӱ
		stream << movies_.size();
		foreach(MovieResourceObject* movie, movies_)
		{
			QByteArray dat;
			{
				QDataStream stream(&dat, QIODevice::WriteOnly);
				movie->encode(stream);
			}
			stream << dat;
		}

		// ����
		QList<LayerResourceObject*> layers;

		stream << levels_.size();
		foreach(LevelResourceObject* lev, levels_)
		{
			if (lev->flags() & URF_TagGarbage)
				continue;

			QByteArray dat;
			{
				QDataStream stream(&dat, QIODevice::WriteOnly);
				lev->encode(stream);
			}
			stream << dat;

			//
			layers.append(lev->layers_);
		}

		// ͼ��
		stream << layers.size();
		foreach(LayerResourceObject* layer, layers)
		{
			if (layer->flags() & URF_TagGarbage)
				continue;

			QByteArray dat;
			{
				QDataStream stream(&dat, QIODevice::WriteOnly);
				layer->encode(stream);
			}
			stream << dat;
		}

		// blueprint����
		stream << blueprints_.size();
		foreach(BlueprintResourceObject* script, blueprints_)
		{
			if (script->flags() & URF_TagGarbage)
				continue;

			QByteArray dat;
			{
				QDataStream stream(&dat, QIODevice::WriteOnly);
				script->encode(stream);
			}
			stream << dat;
		}

		// ȷ��������������
		Q_ASSERT(!(flags() & URF_NeedLoad));

	}

	bool ProjectResourceObject::decode(QDataStream& stream){
		if (!ResourceObject::decode(stream))
			return false;

		stream >> createDate_;
		stream >> lastModifyDate_;
		stream >> author_;
		stream >> pubVersion_;
		stream >> email_;
		stream >> website_;
		stream >> domainId_;
	
		stream >> thumbnail_;
		stream >> resolution_;
		stream >> aspect_;
		stream >> startLevel_;

		stream >> fullscreenMode_;  // ȫ��ģʽ
		stream >> fullscreenQuality_;  // ��������
		stream >> useHighDPI_;
		stream >> sampleType_;
		stream >> downScaling_;  // ��С
		stream >> physicalEngine_;  // ��������
		stream >> loaderStyle_;  // ����ҳ����ʽ
		stream >> preloadSound_; //  Ԥ��������
		stream >> pauseOnUnfocus_;  // ʧȥ������ֹͣ����
		stream >> clearBackground_;  // �Ƿ�����������رտ����Ч�ʣ�

		stream >> useLoaderLayout_;
		stream >> useWebGL_;
		stream >> pixelRounding_;
		stream >> orientation_;
		stream >> exportDir_;
		stream >> mediaDirs_;
		stream >> minfyScript_;

		if (version_ >= PACKAGE_RESOURCE_ADD_OBJECTGROUP){
			stream >> objectGroups_;
		}
		else{
			objectGroups_.clear();
		}

		if ((flags() & URF_NeedLoad)){  // �����ع�������
			return true;
		}

		// ��������
		int num;
		QByteArray dat;
		objTypes_.clear();
		stream >> num;
		for (int i = 0; i < num; i++)
		{
			stream >> dat;

			if( dat.size() > 0 ){
				ObjectTypeResourceObject* res = new ObjectTypeResourceObject();
				res->clearFlags(URF_NeedLoad);

				QDataStream stream(dat);
				if (!res->decode(stream)){
					delete res;
					continue;
				}
				res->setOuter(this);
				res->addObject();
				objTypes_.append(res);
			}
		}


		// ����
		families_.clear();
		stream >> num;
		for (int i = 0; i < num; i++)
		{
			stream >> dat;

			if (dat.size() > 0){
				FamilyResourceObject* res = new FamilyResourceObject();
				res->clearFlags(URF_NeedLoad);

				QDataStream stream(dat);
				if (!res->decode(stream)){
					delete res;
					continue;
				}
				res->setOuter(this);
				res->addObject();
				families_.append(res);
			}
		}


		// ����
		sounds_.clear();
		stream >> num;
		for (int i = 0; i < num; i++)
		{
			stream >> dat;

			if (dat.size() > 0){
				SoundResourceObject* res = new SoundResourceObject();
				res->clearFlags(URF_NeedLoad);

				QDataStream stream(dat);
				if (!res->decode(stream)){
					delete res;
					continue;
				}
				res->setOuter(this);
				res->addObject();
				sounds_.append(res);
			}
		}

		// ��Ӱ
		movies_.clear();
		stream >> num;
		for (int i = 0; i < num; i++)
		{
			stream >> dat;

			if (dat.size() > 0){
				MovieResourceObject* res = new MovieResourceObject();
				res->clearFlags(URF_NeedLoad);

				QDataStream stream(dat);
				if (!res->decode(stream)){
					delete res;
					continue;
				}
				res->setOuter(this);
				res->addObject();
				movies_.append(res);
			}
		}




		// ����
		levels_.clear();
		stream >> num;
		for (int i = 0; i < num; i++)
		{
			stream >> dat;

			if (dat.size() > 0){
				LevelResourceObject* res = new LevelResourceObject();
				res->clearFlags(URF_NeedLoad);

				QDataStream stream(dat);
				if (!res->decode(stream)){
					delete res;
					continue;
				}
				res->setOuter(this);
				res->addObject();
				levels_.append(res);
			}
		}


		// ͼ��
		stream >> num;
		for (int i = 0; i < num; i++)
		{
			stream >> dat;

			if (dat.size() > 0){
				LayerResourceObject* res = new LayerResourceObject();
				res->clearFlags(URF_NeedLoad);

				QDataStream stream(dat);
				if (!res->decode(stream)){
					delete res;
					continue;
				}
				res->setOuter(this);
				res->addObject();
			}
		}

		// blueprint
		blueprints_.clear();
		stream >> num;
		for (int i = 0; i < num; i++)
		{
			stream >> dat;

			if (dat.size() > 0){
				BlueprintResourceObject* res = new BlueprintResourceObject();
				res->clearFlags(URF_NeedLoad);

				QDataStream stream(dat);
				if (!res->decode(stream)){
					delete res;
					continue;
				}
				res->setOuter(this);
				res->addObject();
				blueprints_.append(res);
			}
		}

	

		return true;
	}

	QString  ProjectResourceObject::getDefMediaDir(int type){
		Q_ASSERT(type >= 0 && type <= MEDIA_MISC);
		QString dir = mediaDirs_[type];
		if (dir.isEmpty()){
			if (type == MEDIA_IMAGE){
				dir = UDQ_T("images");
			}
			else if (type == MEDIA_AUDIO || type == MEDIA_VIDEO ){
				dir = UDQ_T("media");
			}
			else{
				dir = UDQ_T("misc");
			}
		}

		return dir;
	}

	QString ProjectResourceObject::getExportName(){
		return mediaDirs_[MEDIA_PROJECT];
	}


	ResourceObject* ProjectResourceObject::createChild(const QString& inName, RTTIType childType){
		if (!(childType & allowChildFlags_))
			return NULL;

		ResourceObject* child = NULL;

		if (childType == URT_Level){
			child = new LevelResourceObject(inName);
		}
		else if (childType == URT_Layer){
			child = new LayerResourceObject(inName);
		}
		else if (childType == URT_ObjectType){
			child = new ObjectTypeResourceObject(inName);
		}
		else if (childType == URT_Blueprint){
			child = new BlueprintResourceObject(inName);
		}
		else if (childType == URT_Family){
			child = new FamilyResourceObject(inName);
		}
		return child;
	}


	LevelResourceObject* ProjectResourceObject::addNewLevel(const QString& inName, int index){
		QString levelName;
		if (!inName.isEmpty()){
			LevelResourceObject* oldLevel = findChild<LevelResourceObject>(inName);
			if (oldLevel == NULL){
				levelName = inName;
			}
		}
		// ˵��ҳ���Ѿ����ڣ��Զ�����������
		if (levelName.isEmpty()){
			levelName = makeUniqueName<LevelResourceObject>(inName.isEmpty() ? UDQ_TR("Level") : inName);
		}

		LevelResourceObject* level = TAddChildObject<LevelResourceObject>(this, levelName);
		Q_ASSERT(level != NULL);
		level->size_ = resolution_ * 2;  // Ĭ��ͼ���СΪ���ڵ�2��
		level->setDescription(levelName);
		levels_.append(level);
		return level;
	}

	LevelResourceObject* ProjectResourceObject::getLevel(const QString& name){
		foreach(LevelResourceObject* l, levels_){
			if (l->flags() & URF_TagGarbage)
				continue;

			if (l->description() == name){
				return l;
			}
		}
		return NULL;
	}

	BlueprintResourceObject* ProjectResourceObject::addNewScript(const QString& inName ){
		QString scriptName;
		if (!inName.isEmpty()){
			BlueprintResourceObject* oldScript = findChild<BlueprintResourceObject>(inName);
			if (oldScript == NULL){
				scriptName = inName;
			}
		}
		// ˵��ҳ���Ѿ����ڣ��Զ�����������
		if (scriptName.isEmpty()){
			scriptName = makeUniqueName<BlueprintResourceObject>(inName.isEmpty() ? UDQ_TR("Script") : inName);
		}

		BlueprintResourceObject* script = TAddChildObject<BlueprintResourceObject>(this, scriptName);
		Q_ASSERT(script != NULL);
		script->setDescription(scriptName);
		blueprints_.append(script);
		return script;
	}

	FamilyResourceObject* ProjectResourceObject::addNewFamily(const QString& inName ){
		QString familyName;
		if (!inName.isEmpty()){
			FamilyResourceObject* oldScript = findChild<FamilyResourceObject>(inName);
			if (oldScript == NULL){
				familyName = inName;
			}
		}
		// ˵��ҳ���Ѿ����ڣ��Զ�����������
		if (familyName.isEmpty()){
			familyName = makeUniqueName<FamilyResourceObject>(inName.isEmpty() ? UDQ_TR("Family") : inName);
		}

		FamilyResourceObject* family = TAddChildObject<FamilyResourceObject>(this, familyName);
		Q_ASSERT(family != NULL);
		family->setDescription(familyName);
		families_.append(family);
		return family;
	}

	void ProjectResourceObject::removeLevel(LevelResourceObject* level){
		levels_.removeOne(level);
	}

	int ProjectResourceObject::indexOf(LevelResourceObject* group){
		return levels_.indexOf(group);
	}

	int ProjectResourceObject::indexOf(ResourceHash hash){
		DEFINE_RESOURCE_HASHKEY(LevelResourceObject, group, hash);
		return indexOf(group);
	}

	// �ƶ�����
	void ProjectResourceObject::moveLevel(int src, int dest){
		if (src < 0){
			src = levels_.size() - 1;
		}

		LevelResourceObject* group = levels_.at(src);
		moveLevel(group, dest);

	}

	void ProjectResourceObject::moveLevel(LevelResourceObject* group, int index){
		levels_.removeOne(group);

		if (index < 0 || index >= levels_.size()){
			levels_.append(group);
		}
		else{
			levels_.insert(index, group);
		}
	}


	ObjectTypeResourceObject* ProjectResourceObject::addNewType(PluginResourceObject* prototype, const QString& inName ){

		Q_ASSERT(prototype != NULL);

		QString typeName;
		if (!inName.isEmpty()){
			ObjectTypeResourceObject* oldLevel = findChild<ObjectTypeResourceObject>(inName);
			if (oldLevel == NULL){
				typeName = inName;
			}
		}
		// ˵���Ѿ����ڻ�Ϊ�գ��Զ�����������
		if (typeName.isEmpty()){
			typeName = makeUniqueName<ObjectTypeResourceObject>(inName.isEmpty() ? prototype->objectName() : inName);
		}

		ObjectTypeResourceObject* type = TAddChildObject<ObjectTypeResourceObject>(this, typeName);
		Q_ASSERT(type != NULL);
		type->prototype_ = prototype;
		objTypes_.append(type);
		return type;
	}

	ObjectTypeResourceObject* ProjectResourceObject::getObjectType(PluginResourceObject* prototype, const QString& inName){
		if (prototype == NULL)
			return NULL;

		bool cmpName = true;
		if (prototype->hasAttribute(EF_SINGLEGLOBAL)){  // ���ñȽ�����
			cmpName = false;
		}

		foreach(ObjectTypeResourceObject* obj, objTypes_){
			if (obj->flags() & URF_TagGarbage)
				continue;
			if (obj->prototype_ == prototype){
				if (!cmpName || obj->description() == inName){
					return obj;
				}
			}
		}

		return NULL;
	}

	ObjectTypeResourceObject*  ProjectResourceObject::getObjectType(const QString& name){

		foreach(ObjectTypeResourceObject* obj, objTypes_){
			if (obj->flags() & URF_TagGarbage)
				continue;

			if (obj->description() == name){
				return obj;
			}
		}
		return NULL;
	}


	FamilyResourceObject* ProjectResourceObject::getFamily(const QString& name){
		foreach(FamilyResourceObject* l, families_){
			if (l->description() == name){
				return l;
			}
		}
		return NULL;
	}


	bool ProjectResourceObject::validScript(){
		foreach(BlueprintResourceObject* blueprint, blueprints_){
			if (blueprint->flags() & URF_TagGarbage)
				continue;

			if (blueprint->events_.size() > 0 && blueprint->scriptJson_.isEmpty()){
				return false;
			}
		}
		return true;
	}


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(LevelResourceObject, URT_Level, ResourceObject,  0);

	LevelResourceObject::LevelResourceObject(const QString& inName, unsigned int flags) : ResourceObject(inName, flags), treeItem_(NULL)
	{
		unboundScroll_ = false;
		margin_ = QSize(500, 500);
		size_ = QSize(0, 0);
		zoomStep_ = 0;
		matrix_.reset();
		viewport_.setX(0);
		viewport_.setY(0);
		blueprint_ = 0;
	}

	void LevelResourceObject::encode(QDataStream& stream){

		ResourceObject::encode(stream);

		stream << thumbnail_;
		stream << size_;
		stream << margin_;
		stream << unboundScroll_;
		stream << blueprint_;

		stream << matrix_;
		stream << viewport_;
		stream << zoomStep_;

		stream << effects_;
		stream << attachments_.size();
		QMapIterator <QString, PluginResourceObject*> iter(attachments_);
		while (iter.hasNext()){
			iter.next();
			stream << iter.key();
			stream << iter.value()->hashKey();
		}

		stream << instances_;

		// ȷ��������������
		Q_ASSERT(!(flags() & URF_NeedLoad));

	}

	bool LevelResourceObject::decode(QDataStream& stream){
		if (!ResourceObject::decode(stream))
			return false;

		stream >> thumbnail_;
		stream >> size_;
		stream >> margin_;
		stream >> unboundScroll_;
		stream >> blueprint_;

		stream >> matrix_;
		stream >> viewport_;
		stream >> zoomStep_;

		stream >> effects_;
		int num;
		stream >> num;
		QString name;
		ResourceHash key;
		for (int i = 0; i < num; i++){
			stream >> name >> key;
			attachments_.insert(name, UDynamicCast(PluginResourceObject, ResourceObject::findResource(key)));
		}

		if (version_ >= PACKAGE_RESOURCE_ADD_ENTITYOBJECT){
			stream >> instances_;
		}

		layers_.clear();



		return true;
	}

	void LevelResourceObject::clear(){
		// �������ͼ����Ч
		foreach(LayerResourceObject* layer, layers_){
			layer->setFlags(URF_TagGarbage);
		}
		treeItem_ = NULL;
	}

	LayerResourceObject* LevelResourceObject::addNewLayer(const QString& inName, int index){
		QString layerlName;

		ProjectResourceObject* proj = UDynamicCast(ProjectResourceObject, outer());
		Q_ASSERT(proj != NULL);

		if (!inName.isEmpty()){
			LayerResourceObject* oldLayer = proj->findChild<LayerResourceObject>(inName);
			if (oldLayer == NULL){
				layerlName = inName;
			}
		}
		// ˵��ҳ���Ѿ����ڣ��Զ�����������
		if (layerlName.isEmpty()){
			layerlName = proj->makeUniqueName<LayerResourceObject>(UDQ_TR("Layer"));
		}

		LayerResourceObject* layer = TAddChildObject<LayerResourceObject>(proj, layerlName);
		Q_ASSERT(layer != NULL);
		layer->setDescription(layerlName);
		layer->level_ = this;
		return layer;
	}



	//  ��¡
	void LevelResourceObject::copyObject(ResourceObject* obj){
		if (this == obj)
			return;
		ResourceObject::copyObject(obj);

		LevelResourceObject* level = UDynamicCast(LevelResourceObject, obj);
		Q_ASSERT(level != NULL);

		level->thumbnail_ = thumbnail_;
		level->size_ = size_;
		level->margin_ = margin_;
		level->unboundScroll_ = unboundScroll_;
		level->blueprint_ = blueprint_;

		int i = 0;
		// ͼ��
		for (; i < layers_.size(); ++i)
		{
			if (i >= level->layers_.size()){
				level->addNewLayer(UDQ_T(""));
			}
			layers_[i]->copyObject(level->layers_[i]);
		}

	}

	void LevelResourceObject::removeLayer(LayerResourceObject* layer){
		layers_.removeOne(layer);
	}

	const int THUMB_SIZE = 128;
	void LevelResourceObject::setThumb(QImage& img){
		if (thumbnail_.isNull()){
			thumbnail_ = QPixmap(THUMB_SIZE, THUMB_SIZE);
		}

		thumbnail_.fill(Qt::transparent);
		QPainter painter(&thumbnail_);

		QImage sizedImg;
		// ת��Ϊ128*128��С������ͼ
		if (img.width() >= img.height() && img.width() > THUMB_SIZE){
			sizedImg = img.scaledToWidth(THUMB_SIZE);
		}
		else if (img.width() < img.height() && img.height() > THUMB_SIZE){
			sizedImg = img.scaledToHeight(THUMB_SIZE);
		}
		// 
		painter.drawImage((THUMB_SIZE - sizedImg.width()) / 2, (THUMB_SIZE - sizedImg.height()) / 2, sizedImg);

	}

	void LevelResourceObject::updateThumb(){
		if (treeItem_ && !thumbnail_.isNull() ){
			treeItem_->setIcon(0, thumbnail_);
		}
	}

	int LevelResourceObject::insertLayer(LayerResourceObject* layer, int index ){
		if (index < 0){
			layers_.append(layer);
		}
		else{
			layers_.insert(index, layer);
		}

		layer->level_ = this;
		return layers_.indexOf(layer);
	}

	// �ƶ�����
	void LevelResourceObject::moveLayer(int src, int dest){
		if (src < 0){
			src = layers_.size() - 1;
		}

		if (dest < 0){
			dest = layers_.size() - 1;
		}

		if (src == dest)
			return;

		Q_ASSERT(src < layers_.size() && dest < layers_.size());

		LayerResourceObject* layer = layers_.takeAt(src);
		layers_.insert(dest, layer);
	
	}

	// ��������
	void LevelResourceObject::exchangeLayer(int src, int dest){
		if (src < 0){
			src = layers_.size() - 1;
		}

		if (dest < 0){
			dest = layers_.size() - 1;
		}

		if (src == dest)
			return;

		Q_ASSERT(src < layers_.size() && dest < layers_.size());
		layers_.swap(src, dest);

	}


	int LevelResourceObject::indexOf(LayerResourceObject* layer){
		return layers_.indexOf(layer);
	}

	int LevelResourceObject::indexOf(ResourceHash hash){
		DEFINE_RESOURCE_HASHKEY(LayerResourceObject, layer, hash);
		return indexOf(layer);
	}


	LayerResourceObject* LevelResourceObject::findLayer(const QString& inName){
		LayerResourceObject* layer = NULL;
		foreach(LayerResourceObject* l, layers_){
			if ( !(l->flags() & URF_TagGarbage) && l->description() == inName){
				layer = l;
				break;
			}
		}
		return layer;
	}
	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////
	ImplementRTTI(LayerResourceObject, URT_Layer, ResourceObject, 0);

	LayerResourceObject::LayerResourceObject(const QString& inName, unsigned int flags) : ResourceObject(inName, flags), treeItem_(NULL)
	{
		level_ = NULL;
		isLocking_ = false;
		isVisible_ = true;
		isGlobal_ = false;

		parallax_ = QPoint(100, 100);
		bgColor_ = QColor(255, 255, 255, 255);
		transparent_ = true;
		opacity_ = 100;

		initVisible_ = true;
		scale_ = 100;
		ownTexture_ = false;
		useRenderCell_ = false;

	}

	void LayerResourceObject::encode(QDataStream& stream){

		ResourceObject::encode(stream);

		Q_ASSERT(level_ != NULL);
		stream << level_->hashKey();

		stream << thumbnail_;
		stream << isLocking_;
		stream << isVisible_;
		stream << isGlobal_;

		stream << parallax_;
		stream << bgColor_;
		stream << opacity_;
		stream << transparent_;
		stream << initVisible_;
		stream << scale_;
		stream << ownTexture_;
		stream << useRenderCell_;

		stream << blend_;
		stream << fallback_;
		stream << effects_;
		stream << attachments_.size();
		QMapIterator <QString, PluginResourceObject*> iter(attachments_);
		while (iter.hasNext()){
			iter.next();
			stream << iter.key();
			stream << iter.value()->hashKey();
		}

		stream << instances_;

		// ȷ��������������
		Q_ASSERT(!(flags() & URF_NeedLoad));

	}

	bool LayerResourceObject::decode(QDataStream& stream){
		if (!ResourceObject::decode(stream))
			return false;

		ResourceHash key;
		stream >> key;
		level_ = UDynamicCast(LevelResourceObject, ResourceObject::findResource(key));
		Q_ASSERT(level_ != NULL);
		level_->layers_.append(this);

		stream >> thumbnail_;
		stream >> isLocking_;
		stream >> isVisible_;

		stream >> isGlobal_;
		stream >> parallax_;
	
		stream >> bgColor_;
		stream >> opacity_;
		
		stream >> transparent_;
		stream >> initVisible_;
		stream >> scale_;	
		stream >> ownTexture_;
		stream >> useRenderCell_;

		stream >> blend_;
		stream >> fallback_;

		stream >> effects_;
		int num;
		stream >> num;
		QString name;
		for (int i = 0; i < num; i++){
			stream >> name >> key;
			attachments_.insert(name, UDynamicCast(PluginResourceObject, ResourceObject::findResource(key)));
		}
	
		stream >> instances_;

		return true;
	}


	QByteArray LayerResourceObject::saveProperty(){
		QByteArray data;
		QDataStream  stream(&data, QIODevice::WriteOnly);

		stream << description();
		stream << isLocking_;
		stream << isVisible_;
		stream << isGlobal_;

		stream << parallax_;
		stream << bgColor_;
		stream << opacity_;
		stream << transparent_;
		stream << initVisible_;
		stream << scale_;
		stream << ownTexture_;
		stream << useRenderCell_;

		stream << blend_;
		stream << fallback_;
	
		return data;
	}

	void LayerResourceObject::restoreProperty(QByteArray& data){
		QDataStream stream(data);

		stream >> description_;
		stream >> isLocking_;
		stream >> isVisible_;
		stream >> isGlobal_;

		stream >> parallax_;
		stream >> bgColor_;
		stream >> opacity_;
		stream >> transparent_;
		stream >> initVisible_;
		stream >> scale_;
		stream >> ownTexture_;
		stream >> useRenderCell_;

		stream >> blend_;
		stream >> fallback_;
	}


	void LayerResourceObject::copyObject(ResourceObject* obj){
		if (this == obj)
			return;

		LayerResourceObject* layer = UDynamicCast(LayerResourceObject, obj);
		Q_ASSERT(layer != NULL);
		layer->timestamp_ = timestamp_;
		layer->thumbnail_ = thumbnail_;
		layer->isLocking_ = isLocking_;
		layer->isVisible_ = isVisible_;
		layer->instances_ = instances_;
		layer->parallax_ = parallax_;
		layer->bgColor_ = bgColor_;
		layer->transparent_ = transparent_;
		layer->initVisible_ = initVisible_;
		layer->scale_ = scale_;
		layer->ownTexture_ = ownTexture_;

	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	ImplementRTTI(ObjectTypeResourceObject, URT_ObjectType, ResourceObject, 0);

	ObjectTypeResourceObject::ObjectTypeResourceObject(const QString& inName, unsigned int flags) : ResourceObject(inName, flags), global_(false)
	{
		animation_ = NULL;
		staticImage_ = NULL;
		treeItem_ = NULL;
		staticSID_ = hashString(QDateTime::currentDateTime().toString());
	}

	void ObjectTypeResourceObject::encode(QDataStream& stream){
		ResourceObject::encode(stream);

		stream << prototype_->hashKey();
		stream << global_;

		stream << behaviors_;
		stream << effects_;

		stream << attachments_.size();
		QMapIterator <QString, PluginResourceObject*> iter(attachments_);
		while (iter.hasNext()){
			iter.next();
			stream << iter.key();
			stream << iter.value()->hashKey();
		}

		stream << variables_.toByteArray();
		stream << defaults_;
		stream << defaultData_;  // ȱʡʵ������
		stream << tilemapData_;
		stream << categroy_;
	}

	bool ObjectTypeResourceObject::decode(QDataStream& stream){
		if (!ResourceObject::decode(stream))
			return false;

		ResourceHash key;
		stream >> key;
		prototype_ = UDynamicCast(PluginResourceObject, ResourceObject::findResource(key));
		if (prototype_ == NULL){
			gLogManager->logError(UDQ_TR("postInit�� ��������(%1)��yԭ�Ͳ��(%2)��Ч").arg(objectName()).arg(key));
		}

		stream >> global_;

		stream >> behaviors_;
		stream >> effects_;

		int num;
		stream >> num;
		QString name;
		for (int i = 0; i < num; i++){
			stream >> name >> key;
			attachments_.insert(name, UDynamicCast(PluginResourceObject, ResourceObject::findResource(key)));
		}

		QByteArray data;
		stream >> data;
		variables_.fromByteArray(data);

		stream >> defaults_;
		stream >> defaultData_;
		stream >> tilemapData_;
		stream >> categroy_;

		postInit();

		return true;
	}

	void ObjectTypeResourceObject::copyObject(ResourceObject* obj){
		ResourceObject::copyObject(obj);

		ObjectTypeResourceObject* type = UDynamicCast(ObjectTypeResourceObject, obj);
		Q_ASSERT(type != NULL);

		type->prototype_ = prototype_;
		type->global_ = global_;
		type->behaviors_ = behaviors_;
		type->effects_ = effects_;
		type->attachments_ = attachments_;  // ��Ϊ����Ч
		type->defaults_ = defaults_;  // ����ȱʡֵ��keyΪ pluginName.paramName��
		QByteArray tmparr = variables_.toByteArray();
		type->variables_.fromByteArray(tmparr);
	
		// �ǳ־�����
		type->animation_ = animation_;  // ���鶯��
		type->staticImage_ = staticImage_;  // ��̬ͼƬ

	}

	void ObjectTypeResourceObject::postInit(){
		animation_ = NULL;
		staticImage_ = NULL;

		if (prototype_ == NULL){
			return;
		}

		// ������ʼ��
		QString pluginName = prototype_->objectName();
		ResourceHash lowerKey = hashStringLower(pluginName);

		if (lowerKey == COMP_SPRITE ){
			// ��ȡ����֡��Դ
			QString animName = getParameter(UDQ_T("Animation"), pluginName);
			if (!animName.isEmpty()){
				ResourceHash key = animName.toUInt();

				QString staticImage = getParameter(UDQ_T("StaticImage"), pluginName);
				if (staticImage != UDQ_T("true")){
					animation_ = UDynamicCast(SpriteResourceObject, ResourceObject::findResource(key));
					staticImage_ = NULL;
				}
				else{
					staticImage_ = UDynamicCast(ImageResourceObject, ResourceObject::findResource(key));
					animation_ = NULL;
				}

				if ( key > 0 && animation_ == NULL && staticImage_ == NULL ){
					gLogManager->logError(UDQ_TR("postInit�� ��������(%1)��ȱʡ������Դ(%2)��Ч").arg(objectName()).arg(key));
					clearImageResource();  // �����Ч��ͼ����Դ
				}
			}
		}
		else if( lowerKey == COMP_TILEDBG || lowerKey == COMP_NINEPATCH || lowerKey == COMP_PARTICLES || lowerKey == COMP_TILEMAP ){
			animation_ = NULL;
			QString animName = getParameter(UDQ_T("Animation"), pluginName);
			if (!animName.isEmpty()){
				ResourceHash key = animName.toUInt();
				staticImage_ = UDynamicCast(ImageResourceObject, ResourceObject::findResource(key));

				if (key > 0 && staticImage_ == NULL){
					gLogManager->logError(UDQ_TR("postInit�� ��������(%1)��ȱʡ������Դ(%2)��Ч").arg(objectName()).arg(key));
					clearImageResource();  // �����Ч��ͼ����Դ
				}
			}	
		}
	}


	void ObjectTypeResourceObject::clearImageResource(){
		animation_ = NULL;
		staticImage_ = NULL;
		setParameter(UDQ_T("Animation"),  QString());
	}

	bool ObjectTypeResourceObject::hasImageResource(){
		return animation_ != NULL || staticImage_ != NULL;
	}


	void ObjectTypeResourceObject::clear(){
		variables_.clear();
		defaults_.clear();

		global_ = false;
		behaviors_.clear();
		effects_.clear();
		attachments_.clear();  // ��Ϊ����Ч

		 treeItem_ = NULL;  // Ŀ¼����Ӧ��
		 animation_ = NULL;  // ���鶯��
		 staticImage_ = NULL;  // ��̬ͼƬ
	}

	QString ObjectTypeResourceObject::getParameter(const QString& prop, const QString& plugin ){
		QString v;
	
		if (plugin.isEmpty() ){
			// ��鵱ǰ����ֵ
			PluginVariable* var = variables_.value(prop);
			if (var){
				v = var->value;
			}
		}
		else{
			ResourceHash key = hashString(plugin + UDQ_T(".") + prop);

			// ��鵱ǰ����ֵ
			if (defaults_.contains(key)){
				v = defaults_.value(key);
			}
			else{
				// ʹ��ȱʡֵ
				if (prototype_->objectName() == plugin){
					v = prototype_->getVariableDefault(prop);
				}
				else{
					if (!attachments_.contains(plugin)){
						gLogManager->logWarning(UDQ_TR(" ���ʶ�������( %1 )����Ч���(%2)").arg(objectName()).arg(plugin));
					}
					else{
						v = attachments_[plugin]->getVariableDefault(prop);
					}
				}
			}
		}
		return v;
	}

	void ObjectTypeResourceObject::setParameter(const QString& prop, const QString& value, const QString& plugin ){

		if (plugin.isEmpty()){
			PluginVariable* var = variables_.value(prop);
			if (var){
				var->value = value;
			}
		}
		else{
			ResourceHash key = hashString(plugin + UDQ_T(".") + prop);
			defaults_.insert(key, value);
		}

	}

	SpriteSequenceResourceObject* ObjectTypeResourceObject::getDefaultAnimation(){
		Q_ASSERT(animation_ != NULL && prototype_ != NULL );

		QString seqName = getParameter(UDQ_T("InitSequence"), prototype_->objectName());
		if (seqName.isEmpty()){
			// ��ȡ��һ������
			return animation_->sequences_.size() > 0 ? animation_->sequences_[0] : NULL;
		}
	
		return animation_->sequence(seqName);
	}


	bool ObjectTypeResourceObject::setImageResource(ResourceHash key){
		if (key == 0)
			return false;

		DEFINE_RESOURCE_HASHKEY(ImageResourceObject, img, key);
		if (img == NULL)
			return false;

		staticImage_ = img;
		setParameter(UDQ_T("StaticImage"), UDQ_T("true"), prototype_->objectName());
		setParameter(UDQ_T("Animation"), QString::number(staticImage_->hashKey()),  prototype_->objectName());

		setFlags(URF_TagSave);

		return true;
	}

	PluginVariable* ObjectTypeResourceObject::getVariableDefine(const QString& prop, const QString& pluginName){
		if (prop.isEmpty()){
			return NULL;
		}

		PluginVariable* def = NULL;
		if (pluginName.isEmpty()){
			def = variables_.value(prop);
		}
		else if (pluginName == prototype_->objectName()){
			def = prototype_->getVariable(prop);
		}
		else{  // ��Ϊ����Ч
			PluginResourceObject* plugatt = attachments_.value(pluginName);
			if (plugatt){
				def = plugatt->getVariable(prop);
			}
		}
		return def;
	}

	QPixmap ObjectTypeResourceObject::thumbnailImage(int size){

		QImage initImage;
		if (animation_ != NULL){
			SpriteSequenceResourceObject* seq = getDefaultAnimation();
			if (seq){
				initImage = seq->frameImage(0);
			}
		}
		else if (staticImage_ != NULL){  // ��̬ͼƬ������ê��ʼ��Ϊ����
			initImage = staticImage_->image_;
		}

		if (initImage.isNull()){
			initImage = prototype_->icon_;
		}

		QPixmap pixmap(64, 64);
		pixmap.fill(Qt::transparent);
		QPainter painter(&pixmap);
		QImage sizedImg;
		if (initImage.width() >= initImage.height() && initImage.width() > size){
			sizedImg = initImage.scaledToWidth(size);
		}
		else if (initImage.width() < initImage.height() && initImage.height() > size){
			sizedImg = initImage.scaledToHeight(size);
		}
		else{
			sizedImg = initImage;
		}
		painter.drawImage((size - sizedImg.width()) / 2, (size - sizedImg.height()) / 2, sizedImg);
		return pixmap;
	}


	PluginResourceObject* ObjectTypeResourceObject::getPlugin(const QString& pluginName){
		if (pluginName.isEmpty())
			return NULL;

		if (pluginName == prototype_->objectName()){
			return prototype_;
		}

		if (attachments_.contains(pluginName)){
			return attachments_[pluginName];
		}

		return NULL;
	}



	bool ObjectTypeResourceObject::hasAttachment(ResourceHash key){
		DEFINE_RESOURCE_HASHKEY(PluginResourceObject, plugin, key);
		if (plugin == NULL)
			return false;
		return hasAttachment(plugin);
	}

	bool ObjectTypeResourceObject::hasAttachment(PluginResourceObject* plugin){
		return attachments_.values().contains(plugin);
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	ImplementRTTI(FamilyResourceObject, URT_Family, ResourceObject, 0);

	FamilyResourceObject::FamilyResourceObject(const QString& inName, unsigned int flags) : ResourceObject(inName, flags), treeItem_(NULL)
	{
	}

	void FamilyResourceObject::encode(QDataStream& stream){
		ResourceObject::encode(stream);

		stream << members_.size();
		foreach(ObjectTypeResourceObject* type, members_){
			stream << type->hashKey();
		}

		stream << behaviors_;
		stream << effects_;
		stream << attachments_.size();

		QMapIterator <QString, PluginResourceObject*> iter(attachments_);
		while (iter.hasNext()){
			iter.next();
			stream << iter.key();
			stream << iter.value()->hashKey();
		}

		stream << variables_.toByteArray();
		stream << defaults_;
	}

	bool FamilyResourceObject::decode(QDataStream& stream){
		if (!ResourceObject::decode(stream))
			return false;

		members_.clear();
		int num;
		stream >> num;
		ResourceHash key;
		for (int i = 0; i < num; i++){
			stream >> key;
			DEFINE_RESOURCE_HASHKEY(ObjectTypeResourceObject, type, key);
			if (type){
				members_.append(type);
			}
		}
	
		stream >> behaviors_;
		stream >> effects_;

		stream >> num;
		QString name;
		for (int i = 0; i < num; i++){
			stream >> name >> key;
			attachments_.insert(name, UDynamicCast(PluginResourceObject, ResourceObject::findResource(key)));
		}

		QByteArray data;
		stream >> data;
		variables_.fromByteArray(data);

		stream >> defaults_;

		postInit();

		return true;
	}

	void FamilyResourceObject::copyObject(ResourceObject* obj){
		ResourceObject::copyObject(obj);

		FamilyResourceObject* family = UDynamicCast(FamilyResourceObject, obj);
		Q_ASSERT(family != NULL);

		family->members_ = members_;
		family->defaults_ = defaults_;
		family->behaviors_ = behaviors_;
		family->effects_ = effects_;
		family->attachments_ = attachments_;

		// ��������


	}

	void FamilyResourceObject::postInit(){

	}


	void FamilyResourceObject::clear(){
		variables_.clear();
		defaults_.clear();

		behaviors_.clear();
		effects_.clear();
		attachments_.clear();  // ��Ϊ����Ч

		treeItem_ = NULL;  // Ŀ¼����Ӧ��
	}

	QString FamilyResourceObject::getParameter(const QString& prop, const QString& plugin){
		QString v;

		if (plugin.isEmpty()){
			// ��鵱ǰ����ֵ
			PluginVariable* var = variables_.value(prop);
			if (var){
				v = var->value;
			}
		}
		else{
			ResourceHash key = hashString(plugin + UDQ_T(".") + prop);

			// ��鵱ǰ����ֵ
			if (defaults_.contains(key)){
				v = defaults_.value(key);
			}
			else{
				if (!attachments_.contains(plugin)){
					gLogManager->logWarning(UDQ_TR(" ���ʶ�������( %1 )����Ч���(%2)").arg(objectName()).arg(plugin));
				}
				else{
					v = attachments_[plugin]->getVariableDefault(prop);
				}
			}
		}
		return v;
	}

	void FamilyResourceObject::setParameter(const QString& prop, const QString& value, const QString& plugin){

		if (plugin.isEmpty()){
			PluginVariable* var = variables_.value(prop);
			if (var){
				var->value = value;
			}
		}
		else{
			ResourceHash key = hashString(plugin + UDQ_T(".") + prop);
			defaults_.insert(key, value);
		}

	}


	PluginVariable* FamilyResourceObject::getVariableDefine(const QString& prop, const QString& pluginName){
		if (prop.isEmpty()){
			return NULL;
		}

		PluginVariable* def = NULL;
		if (pluginName.isEmpty()){
			def = variables_.value(prop);
		}
		else{  // ��Ϊ����Ч
			PluginResourceObject* plugatt = attachments_.value(pluginName);
			if (plugatt){
				def = plugatt->getVariable(prop);
			}
		}
		return def;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	ImplementRTTI(BlueprintResourceObject, URT_Blueprint, ResourceObject, 0);

	BlueprintResourceObject::BlueprintResourceObject(const QString& inName, unsigned int flags) : ResourceObject(inName, flags), treeItem_(NULL)
	{
		hasError_ = false;
		func_ = false;
	}

	void BlueprintResourceObject::encode(QDataStream& stream){
		ResourceObject::encode(stream);

		stream << func_;
		stream << includes_;
		stream << variables_.toByteArray();
		stream << events_;
		stream << links_;
		stream << groups_;
		stream << assets_;
		stream << scriptJson_;

		stream << matrix_;
		stream << viewport_;
		stream << zoomStep_;
		stream << func_;
	}

	bool BlueprintResourceObject::decode(QDataStream& stream){
		if (!ResourceObject::decode(stream))
			return false;

		if (version_ >= PACKAGE_RESOURCE_ADD_SCRIPTFUNCTION){
			stream >> func_;
		}

		stream >> includes_;
		QByteArray data;
		stream >> data;
		variables_.fromByteArray(data);

		stream >> events_;
		stream >> links_;
		stream >> groups_;
		stream >> assets_;
		stream >> scriptJson_;

		stream >> matrix_;
		stream >> viewport_;
		stream >> zoomStep_;

		return true;
	}

	void BlueprintResourceObject::copyObject(ResourceObject* obj){
		ResourceObject::copyObject(obj);

		BlueprintResourceObject* bp = UDynamicCast(BlueprintResourceObject, obj);
		Q_ASSERT(bp != NULL);

		bp->includes_ = includes_;
		QByteArray tmparr = variables_.toByteArray();
		bp->variables_.fromByteArray(tmparr);
		bp->events_ = events_;
		bp->links_ = links_;
		bp->groups_ = groups_;
		bp->assets_ = assets_;

		bp->matrix_ = matrix_;
		bp->viewport_ = viewport_;
		bp->zoomStep_ = zoomStep_;

	}

	QString BlueprintResourceObject::makeVariableName(){
		static QString basename = UDQ_T("var%1");
		int index = 0;
		QString name = basename.arg(index++);
		while (variables_.value(name) != NULL){
			name = basename.arg(index++);
		}
		return name;
	}

	ScriptVariable* BlueprintResourceObject::addNewVariable( const QString& name ){
		QString varName = name;
		if (varName.isEmpty()){
			varName = makeVariableName();
		}
		ScriptVariable* var = variables_.value(varName, true);
		var->datatype = UDQ_T("string");
		var->name = varName;
		return var;
	}

	void BlueprintResourceObject::clearAll(){
		variables_.clear();
		events_.clear();
		links_.clear();
		groups_.clear();
		assets_.clear();
		includes_.clear();
		scriptJson_.clear();
	}

}
