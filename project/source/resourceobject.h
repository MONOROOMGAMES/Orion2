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

#ifndef ORION_RESOURCEOBJECT_H
#define ORION_RESOURCEOBJECT_H

#include <QString>
#include <QHash>
#include <QMultiHash>
#include <QLinkedList>
#include <QList>
#include <QMap>
#include <QPair>
#include <QVector>
#include <QDataStream>
#include <QByteArray>
#include <QVector4D>
#include <QDate>
#include <QDateTime>
#include <QFont>
#include <QPixmap>
#include <QPointF>
#include <QAudioBuffer>
#include <QGraphicsItem>
#include "commondefine.h"
#include "editordefine.h"
#include "resourcertti.h"
#include <QGraphicsItem>
#include <QTreeWidgetItem>

#include "resourceversion.h"
#include "resourcedata.h"

namespace ORION{
	//////////////////////////////////////////////////////////////////////////
	// ��Դ��˵����
	// �ز���Դ�������棬ÿ������Ӧһ��Ŀ¼��Ŀ¼�±�������Դ�ļ���
	// �����鹤�̰������ռ䱣�棬ÿ�������ռ��Ӧһ��Ŀ¼��Ŀ¼�¿��԰���
	// ��������飻
	//////////////////////////////////////////////////////////////////////////

    #define  NATIVE_READONLY  0   // ϵͳ��Դֻ��

    #define  NULL_FILENAME UDQ_T("null")

    #define PROJECT_FILENAME UDQ_T("project")
    #define  GALLERY_IMAGE_FILENAME UDQ_T("images")
    #define  GALLERY_AUDIO_FILENAME UDQ_T("audio")
    #define  GALLERY_VIDEO_FILENAME UDQ_T("video")
    #define  GALLERY_SOUND_FILENAME UDQ_T("sound")
    #define  GALLERY_MOVIE_FILENAME UDQ_T("movie")

    #define  SPRITE_FILENAME UDQ_T("sprites")
    #define  PLUGIN_FILENAME UDQ_T("plugins")

	#define  DEFAULT_PROJECT UDQ_L8("�¹���")
    #define  DEFAULT_AUTHOR UDQ_L8("ʨ��")

    #define  DEFAULT_PACKAGE UDQ_T("Base")
    #define  DEFAULT_WORKSPACE UDQ_T("TempWorkspace")

		// �ļ����Ͷ���
	#define DECLARE_RESOURCE_FILETYPE( t ) \
	public:\
		virtual QString fileType(){return staticFileType();} \
		static QString staticFileType(){ return t; }

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	#define 	URT_Object            0x0000000000000001   // �������
	#define 	URT_Linker              0x0000000000000002   // ���ݿ�ӿ�
    #define 	URT_NullObject     0x0000000000000004   // �ն���

	#define 	URT_Package       0x0000000000000010  // ��Դ��
	#define 	URT_Field               0x0000000000000020   // �ɷ���������
    #define URT_Struct              0x0000000000000040   // �����Խṹ��

    #define 	URT_WorkSpace  0x0000000000000100  // �����ռ�
    #define 	URT_Project          0x0000000000000200  // ����
    #define 	URT_Level             0x0000000000000400  // ����
    #define 	URT_Layer             0x0000000000000800  // ͼ��

    #define 	URT_Gallery          0x0000000000010000  // �ز�
    #define 	URT_Sound            0x0000000000020000 // �������������ָ�ʽ�ļ���
    #define 	URT_Movie           0x0000000000040000  // ��Ӱ���������ָ�ʽ�ļ���

    #define 	URT_Image          0x0000000000100000  // ͼƬ
	#define 	URT_Video           0x0000000000200000  // ��Ƶ
	#define 	URT_Audio           0x0000000000400000  // ��Ƶ
    #define 	URT_Sprite            0x0000000000800000  // ����

   #define   URT_Plugin              0x0000000001000000  // ���
   #define   URT_ObjectType    0x0000000002000000  // ��������
   #define   URT_Blueprint         0x0000000004000000  //  �ű�
   #define   URT_Family              0x0000000008000000  //  ����

   #define 	URT_SpriteSequence   0x0000001000000000  // ��������


	enum ParameterType{   // ���ֻ�������
		PAT_BOOL,
		PAT_INT,
		PAT_DOUBLE,
		PAT_STRING,
	};

	enum PluginType{
		PIT_ENTITY,
		PIT_BEHAVIOR,
		PIT_EFFECT,
		PIT_NATIVE,
	};

	enum FileType{
		FIT_RUNTIME,
		FIT_FX,

		FIT_JAVASCRIPT,
		FIT_CSS,
		FIT_HTML,
		FIT_TEXT,
		FIT_DATA,
		FIT_IMAGE,
		FIT_AUDIO,
		FIT_VIDEO,
		FIT_OTHER,
	};


	enum AttributeType{
		EF_NONE = 0,
		EF_WORLD,
		EF_ROTATABLE,
		EF_SINGLEGLOBAL,
		EF_TEXTURE,
		EF_TILING,
		EF_ANIMATIONS,
		EF_NOSIZE,
		EF_EFFECTS,
		EF_PREDRAW,
		EF_POSITION,
		EF_SIZE,
		EF_ANGLE,
		EF_ZORDER,
		EF_APPEARANCE,
		EF_ONCE,
		EF_BLENDBG,
		EF_CROSSSAMPLE,
		EF_ANIMATIONDRAW,
		EF_BEHAVIORS,

		FT_NONE = 100,
		FT_DEPRECATED,
		FT_TRIGGER,
		FT_FAKE_TRIGGER,
		FT_STATIC,
		FT_INVERTIBLE,
		FT_INCOMPATIBLE,
		FT_LOOPING,
		FT_FAST_TRIGGER,

		FT_RETURN_INT = 200,
		FT_RETURN_FLOAT,
		FT_RETURN_STRING,
		FT_RETURN_ANY,
		FT_VARIADIC
	};

	inline QString Value2String(int value){
		QString str;
#define TOSTRING(x) case x: {str = UDQ_T(#x); str = str.right(str.size() - 3).toLower();} break;\

		switch (value){
			TOSTRING(EF_WORLD);
			TOSTRING(EF_ROTATABLE);
			TOSTRING(EF_SINGLEGLOBAL);
			TOSTRING(EF_TEXTURE);
			TOSTRING(EF_TILING);
			TOSTRING(EF_ANIMATIONS);
			TOSTRING(EF_NOSIZE);
			TOSTRING(EF_EFFECTS);
			TOSTRING(EF_PREDRAW);
			TOSTRING(EF_POSITION);
			TOSTRING(EF_SIZE);
			TOSTRING(EF_ANGLE);
			TOSTRING(EF_ZORDER);
			TOSTRING(EF_APPEARANCE);
			TOSTRING(EF_ONCE);
			TOSTRING(EF_BLENDBG);
			TOSTRING(EF_CROSSSAMPLE);
			TOSTRING(EF_ANIMATIONDRAW);
			TOSTRING(EF_BEHAVIORS);

			TOSTRING(FT_NONE);
			TOSTRING(FT_DEPRECATED);
			TOSTRING(FT_TRIGGER);
			TOSTRING(FT_FAKE_TRIGGER);
			TOSTRING(FT_STATIC);
			TOSTRING(FT_INVERTIBLE);
			TOSTRING(FT_INCOMPATIBLE);
			TOSTRING(FT_LOOPING);
			TOSTRING(FT_FAST_TRIGGER);

			TOSTRING(FT_RETURN_INT);
			TOSTRING(FT_RETURN_FLOAT);
			TOSTRING(FT_RETURN_STRING);
			TOSTRING(FT_RETURN_ANY);
			TOSTRING(FT_VARIADIC);
		default: return QString::number(value);
		}
#undef TOSTRING
		return str;
	}

	enum PropertyType{
		PT_VARIABLE,
		PT_CONDITION,
		PT_ACTION,
		PT_EXPRESS
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	enum ResourceFlags{
		URF_Obsolete			      = 0x00000001,        // ��ʱ
		URF_Public			              = 0x00000002,	     // �������󣬰���ɼ�
		URF_Local			              = 0x00000004,       // ����
	
		URF_TagSave			      = 0x00000010,	// �����־
		URF_TagLoad			      = 0x00000020,	// ���ر�־
		URF_TagGarbage		  = 0x00000040,	// �������ս��м��

		URF_Transient                  = 0x00000100,	  // ��ʱ���󣬲�����
		URF_Destroyed               = 0x00000200,    // ������
		URF_Native			          = 0x00000400,    // ����(һ��ֻ����������)
		URF_NeedLoad              = 0x00000800,    // ��Ҫ���أ�������������

		URF_Error                          = 0x10000000,	 // ��Ч����
	};

	//////////////////////////////////////////////////////////////////////////
	// ���ض����ʶ
	//////////////////////////////////////////////////////////////////////////
	enum ResourceLoadFlags{
		RLF_None			      = 0x0000,	// �ޱ�ʶ( ȫ������ )
		RLF_NoFail			  = 0x0001,	// �������ʧ����Ϊ�������� 
		RLF_Verify			      = 0x0002,	// ȷ�ϴ��ڣ������м���
		RLF_LoadIndex       = 0x0008,	// ��������
	};

#ifndef DEF_RESOURCEHASH
   #define DEF_RESOURCEHASH
	typedef unsigned int ResourceHash;  // ��Դ���
#endif

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	class PackageResourceObject;
	class FieldResourceObject;

	class WorkspaceResourceObject;
	class ProjectResourceObject;
	class GalleryResourceObject;
	class SoundResourceObject;
	class MovieResourceObject;

	class LevelResourceObject;
	class LayerResourceObject;

	class PluginResourceObject;
	class ObjectTypeResourceObject;
	class BlueprintResourceObject;
	class FamilyResourceObject;

	class SpriteResourceObject;
	class SpriteSequenceResourceObject;

	class ResourceLinker;

	// ��Դ������
	class ResourceObject{
		DeclareRootRTTI( ResourceObject )
		friend class ResourceObjectIterator;
	public:
		ResourceObject( unsigned int flags = URF_Public );
		ResourceObject( const QString& inName, unsigned int flags = URF_Public );

		virtual ~ResourceObject(){}

		ResourceObject* outer() const{
			return outer_;
		}

		void setOuter( ResourceObject* inOuter ){
			outer_ = inOuter;
		}

		// ��Ӻ���
		virtual ResourceObject* addChild( const QString& inName,  const ResourceRTTI* RTTI );
		virtual ResourceObject* createChild( const QString& inName,  RTTIType childType ){ return 0; }

		// ��ȡ����Χ����
		ResourceObject* getOutermost() const{
			ResourceObject* top;
			for( top = outer() ; top && top->outer() ; top = top->outer() );
			return top;
		}

		// ��ȡָ���ĸ�����
		bool hasOuter( ResourceObject* stop ) const{
			if (stop == NULL)
				return false;

			bool res = false;
			ResourceObject* top = outer(); 
			while (top){
				if (top == stop){
					res = true;
					break;
				}
				top = top->outer();
			}
			return res;
		}

		// ������className@·����pathName��
		QString fullName() const;

		// ��ȡʵ��·������������Χ����,��ʽ����Parent.child��
		QString pathName( ResourceObject* stopOuter = NULL ) const;

		QString objectName() const{
			return name_;
		}

		void setObjectName( const QString& inName ){
			name_ = inName;
		}

		QString description() const{
			return description_;
		}

		void setDescription( const QString& desp ){
			description_ = desp;
		}

		virtual void rename( const QString& inName );

		ResourceHash hashKey() const{
			return hashKey_;
		}

		unsigned int flags() const{
			return objectFlags_;
		}

		void setFlags( unsigned int newFlags ){
			objectFlags_ |= newFlags;
		}

		void clearFlags( unsigned int newFlags ){
			objectFlags_ &= ~newFlags;
		}

		bool isNative(){
			return objectFlags_ & URF_Native;
		}

		// ��ȡ������
		ResourceLinker* getLinker() const{
			return linker_;
		}

		void setLinker( ResourceLinker* inLinker );

		// ��Ӷ���ʵ��
		void addObject();
		void hashObject();
		void unhashObject();

		// ����hash�Ӷ���
		void hashChildren();

		virtual void encode( QDataStream& stream );
		virtual bool decode( QDataStream& stream );
		// ��ʼ������
		virtual void postInit(){}

		unsigned int version( void ){
			return version_;
		};

		void updateVersion(unsigned int ver){
			version_ = ver;
		}

		virtual bool checkVersion( QDataStream& stream );

		// ʱ��
		qint64 timeStamp(){
			return timestamp_;
		}

		// �����ڲ�ģ��
		template<class T>
		T* decodeChild(QDataStream& stream);

		// ����
		virtual void clear(){}

		// �����¶���
		virtual void copyObject(ResourceObject* obj);

		// ��¡�¶���
		virtual ResourceObject* clone(const QString& inName){
			return NULL;
		}

	protected:
		ResourceObject* outer_;   // ���ڵ�
		ResourceHash hashKey_;  // ��ϣֵ
		QString name_;  // ����
		QString description_;  // ����
		unsigned int objectFlags_;  // ��־
		ResourceLinker* linker_;  // ������
		unsigned int version_;  // ��ǰ�汾(�����ݿ��ȡ�İ汾)
		qint64 timestamp_;  // ʱ��
	
	    //////////////////////////////////////////////////////////////////////////
		// ��̬
		//////////////////////////////////////////////////////////////////////////
	public:
		// ��̬��ʼ��
		static void staticInit();
		// ��̬�˳�
		static void staticExit();
		// ������Դ
		static ResourceObject* findResource( ResourceObject* inOuter, const QString& resPath, bool exactClass = true );
		static ResourceObject* findResource( ResourceHash hashkey );
		// ����������
		static PackageResourceObject* createPackage(  const QString& pkgName, const QString& pkgPath, bool bNative = false );
		// ������Դ��
		static ResourceObject* loadPackage(const QString& pkgPath, unsigned int loadFlags = RLF_None, bool bNative = false);
		// �������
		static void purgeGarbage();
		//  ����Ψһ������
		static QString makeUniqueObjectName( ResourceObject* inOuter, ResourceObject* cls );
		static QString makeUniqueObjectName( const QString& path,  const QString& baseName, ResourceObject* cls );

		// ��ȡ��������
		static ResourceLinker* getPackageLinker(ResourceObject*	inOuter, const QString& inFilename );
		// ���������ռ����
		static WorkspaceResourceObject* createWorkspace(const QString& wsName, bool temp = false );
		// ���ع����ռ����
		static WorkspaceResourceObject* loadWorkspace(const QString& wsName, unsigned int loadFlags = RLF_None);
		static void loadWorkspace(  WorkspaceResourceObject* ws, unsigned int loadFlags = RLF_None );
		// ��ȡ��������
		static ResourceLinker* getWorkspaceLinker(ResourceObject*	inOuter, const QString& inFilename );
		//��չ����ռ�
		static void resetWorkspace( WorkspaceResourceObject* inOuter );
		// ��ǰ�����ռ�
		static WorkspaceResourceObject* currentWorkspace();
		//  ��ʱ�����ռ�
		static WorkspaceResourceObject* tempWorkspace();

	private:
		static QHash<ResourceHash, ResourceObject*> objectIndex_;   // ��Դ����
		static QList<ResourceObject*> objects_; // ��Դ�б�
		static QLinkedList<ResourceLinker*> linkers_;  // ������
		static QLinkedList<ResourceObject*> rootObjects_;  // ������

		static WorkspaceResourceObject* tmpWorkspace_;// ��ʱ�����ռ�
	};


	//////////////////////////////////////////////////////////////////////////
	// ���������
	//////////////////////////////////////////////////////////////////////////

	class ResourceObjectIterator{
	public:
		ResourceObjectIterator(const ResourceRTTI* Rtti = NULL ): RTTI_(Rtti), index_(-1){
			++*this;
		}

		void operator++(){
			while( ++index_<ResourceObject::objects_.size() 
				&& ( !( ResourceObject::objects_[index_] ) ||  ( RTTI_ != NULL && !( ResourceObject::objects_[index_]->isKindOf( RTTI_ ) ) ) ) );
		}
		ResourceObject* operator*(){
			return ResourceObject::objects_[index_];
		}

		ResourceObject* operator->(){
			return ResourceObject::objects_[index_];
		}

		// ����ת��������
		operator bool(){
			return index_<ResourceObject::objects_.size();
		}

	protected:
		const ResourceRTTI* RTTI_;
		int index_; // ����
	};

	template< class T > 
	class TResourceObjectIterator : public ResourceObjectIterator{
	public:
		TResourceObjectIterator():	ResourceObjectIterator( &T::RTTI_ ){}

		T* operator* (){
			return (T*)ResourceObjectIterator::operator*();
		}

		T* operator-> (){
			return (T*)ResourceObjectIterator::operator->();
		}
	};

	//////////////////////////////////////////////////////////////////////////
	//  ��Դ������
	//////////////////////////////////////////////////////////////////////////

	class PackageResourceObject : public ResourceObject{
		DeclareRTTI()
	public:
		PackageResourceObject(){}
		PackageResourceObject( const QString& inName, unsigned int flags =  URF_Transient | URF_Public );
		// ���¼���
		void reload();
		virtual ResourceObject* createChild( const QString& inName,  RTTIType childType );

		QString dirPath_; //   ��Ŀ¼·��

		// ������Ϣ��д
		bool saveConfig();
		// ��ȡ�����ã�config�����ΰ���������������·����
		static bool loadConfig( const QString& path, QStringList& config );
	private: 
	};

	//////////////////////////////////////////////////////////////////////////
	//  ģ����Դ������
	//////////////////////////////////////////////////////////////////////////

	class FieldResourceObject : public ResourceObject{
		DeclareRTTI( )
	public:
		FieldResourceObject(){}
		FieldResourceObject( const QString& inName, unsigned int flags = URF_Transient | URF_Public );

		virtual void encode( QDataStream& stream );
		virtual bool decode( QDataStream& stream );
		// �ļ�����
		virtual QString fileType() = 0;

		bool isBelong(const QString& category);
		bool hasTag(const QString& tag, bool exactlyMatch = false);

		// �����¶���
		virtual void copyObject(ResourceObject* obj);

		QStringList tags_;  //   �����ǩ
		QStringList categories_;  // ��Դ���ͱ�ǩ���ַ������á�parent.child����ʽ��

	};

	class NullResourceObject : public  FieldResourceObject{
		DeclareRTTI( )
		DECLARE_RESOURCE_FILETYPE( NULL_FILENAME )
	public:
		NullResourceObject(){}
	};

	//////////////////////////////////////////////////////////////////////////
	//  ģ����
	//////////////////////////////////////////////////////////////////////////

	// �ַ�����ϣ
	template<typename T>
	unsigned int THashString( const QString& inStr ){
		QString fullStr = QString(UDQ_T("%1@%2")).arg( T::RTTI_.getName()).arg( inStr);
		return hashString( fullStr );
	}

	// ���ݹ�ϣ������Դ
	template<typename T>
	T* TFindResource( unsigned int inHashKey ){
		return  UDynamicCast(T, ResourceObject::findResource( inHashKey ));
	}


#define  FIND_RESOURCE_HASH( T, hashStr ) \
	UDynamicCast(T, ResourceObject::findResource(THashString<T>(hashStr)))

#define  DEFINE_RESOURCE_HASHKEY(T, var, hashValue) \
	T* var = UDynamicCast(T, ResourceObject::findResource(hashValue));

#define  DEFINE_RESOURCE_HASHKEY_VALID(T, var, hashValue) \
	T* var = UDynamicCast(T, ResourceObject::findResource(hashValue)); \
	Q_ASSERT(var != NULL);

	#define  DEFINE_RESOURCE_PATH( T, var, hashStr ) \
	    DEFINE_RESOURCE_HASHKEY( T, var, THashString<T>(hashStr) )

   #define SAVE_PACKAGE_RESOURCE(T, var) \
	    if( var->getLinker() == 0 ){ \
		    QString file =  var->getOutermost()->objectName() + UDQ_T(".") + var->fileType();\
		    ResourceLinker* linker = ResourceObject::getPackageLinker( var->getOutermost(), file );\
			var->setLinker( linker );}\
	    var->getLinker()->save<T>( var->hashKey() );

    #define SAVE_WORKSPACE_RESOURCE(T, var) \
		if( var->getLinker() == 0 ){ \
		QString file =  var->getOutermost()->objectName() + UDQ_T(".") + var->fileType();\
		ResourceLinker* linker = ResourceObject::getWorkspaceLinker( var->getOutermost(), file );\
		var->setLinker( linker );}\
		var->getLinker()->save<T>( var->hashKey() );


	template<typename T>
	T* TAddChildObject( ResourceObject* inOuter, const QString& inName ){
		return (T*)inOuter->addChild(inName, &T::RTTI_);
	}

	template<typename T>
	T*  TCreateResource(ResourceObject* inPkg, const QString& inName){
		return TAddChildObject<T>(inPkg, inName);
	}

	// ��¡�¶���
	template<typename T>
	T* TCloneObject(T* prototype, const QString& inName){
		// ���浽���ݿ���
		T* newObj = TCreateResource<T>(prototype->getOutermost(), inName);
		Q_ASSERT(newObj != 0);

		prototype->copyObject(newObj);
		newObj->setFlags(URF_TagSave);
		return newObj;
	}


	template<typename T>
	T* TFindResource(const QString& inPkg,  const QString& inName, bool bCreate = false){
		QString path = QString(UDQ_T("%1.%3")).arg( inPkg).arg( inName );
		DEFINE_RESOURCE_PATH(T, res, path  );

		if (bCreate){
			if (!res){
				// ȷ�����������
				DEFINE_RESOURCE_PATH(PackageResourceObject, pkg, inPkg);
				Q_ASSERT(pkg != NULL);
				res = TCreateResource<T>(pkg, inName);
			}
			else if (res && (res->flags() & URF_TagGarbage))  {
				res->clearFlags(URF_TagGarbage);
				res->setFlags(URF_TagSave);
			}
		}

		if (res && res->flags() & URF_TagGarbage){
			res = NULL;
		}

		return res;
	}


	template<typename T>
	T* TFindWSResource(const QString& inPkg, const QString inBook, bool bCreate = false){
		QString path = QString(UDQ_T("%1.%2")).arg(inPkg).arg(inBook);
		DEFINE_RESOURCE_PATH(T, res, path);

		if (bCreate){
			if (!res){
				// ���û�У��򴴽�
				WorkspaceResourceObject* ws = ResourceObject::createWorkspace(inPkg);
				Q_ASSERT(ws != NULL);
				res = TAddChildObject<T>(ws, inBook);
			}
			else if (res && (res->flags() & URF_TagGarbage))  {
				res->clearFlags(URF_TagGarbage);
				res->setFlags(URF_TagSave);
			}
		}

		if (res && res->flags() & URF_TagGarbage){
			res = NULL;
		}

		return res;
	}

	template<typename T>
	T* TFindWSResource( const QString& inPkg,  const QString inProject, const QString inName,  bool bCreate = false ){
		QString path = QString(UDQ_T("%1.%2.%3")).arg(inPkg).arg(inProject).arg(inName);
		DEFINE_RESOURCE_PATH(T, res, path  );

		if (bCreate){
			if ( !res ){
				// ȷ���м����Project����������
				ProjectResourceObject* proj = TFindWSResource<ProjectResourceObject>(inPkg, inProject, true);
				Q_ASSERT(proj != NULL);
				res = TAddChildObject<T>(proj, inName);
			}
			else if ( res->flags() & URF_TagGarbage ) {
				res->clearFlags(URF_TagGarbage);
				res->setFlags(URF_TagSave);
			}
		}

		if (res && res->flags() & URF_TagGarbage){
			res = NULL;
		}

		return res;
	}


	template<typename T>
	void  TClearResource(){
		for( TResourceObjectIterator<T> it; it; ++it ){
			it->setFlags( URF_TagGarbage );
		}
	}

	template<typename T>
	QStringList TResourceNames(){
		QStringList list;
		for( TResourceObjectIterator<T> it; it; ++it ){
			list<<it->objectName();
		}
		return list;
	}
			
	// ����Ψһʵ����������ṩrefName��������ʹ��refName������������������Զ������µ�����;
	// affixΪ�Զ����ɵ�����ǰ׺
	template<typename T>
	QString makeInstanceName( ResourceObject* inOuter , ResourceObject* prototype, const QString& refName = QString(), const QString& affix = QString() ){
		Q_ASSERT( inOuter != 0 && inOuter->outer() != 0 );

		QString newPre =  inOuter->pathName();
		QString tmpStringName;
		QString tmpPath;

		//  ����ṩ�ο���
		if( !refName.isEmpty() ){
			tmpPath = QString(UDQ_T("%1.%2")).arg( newPre ).arg(refName);
			if( !FIND_RESOURCE_HASH(T, tmpPath) )
				return refName;
		}

		// ���û�вο�������ο����ѱ�ʹ��
		do{
			tmpStringName = QString(UDQ_T("%3%1%2")).arg(prototype->objectName()).arg( prototype->objectId() ).arg(affix);
			tmpPath = QString(UDQ_T("%1.%2")).arg( newPre ).arg(tmpStringName);
		} while( FIND_RESOURCE_HASH(T, tmpPath) ); // ��������ظ����������ۼ�id

		return tmpStringName;
	}

	// �����ڲ�ģ��
	template<class T>
	T* ResourceObject::decodeChild(QDataStream& stream){
		// ��¼��ʼλ��
		qint64 pos = stream.device()->pos();
		unsigned int ver, hashkey;
		stream >> ver >> hashkey;
		// ���ݹ�ϣ���Ҷ���
		DEFINE_RESOURCE_HASHKEY(T, child, hashkey);
		if (child == NULL){
			child = new T();
		}
		else{
			child->clearFlags(URF_TagGarbage);
			child->setFlags(URF_TagSave);
		}
		// �ص���ʼλ��
		stream.device()->seek(pos);

		if (!child->decode(stream)){
			delete child;
			// ��ʾ����
			return NULL;
		}

		child->clearFlags(URF_NeedLoad);

		if (child->outer() == NULL){
			child->setOuter(this);
			child->addObject();
		}
		else if (child->outer() != this){
			child->setOuter(this);
		}

		return child;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	class GalleryResourceObject : public FieldResourceObject{
		DeclareRTTI( )
	public:
		GalleryResourceObject(){}
		GalleryResourceObject(const QString& inName, unsigned int flags = URF_Transient | URF_Public);

		virtual void encode( QDataStream& stream );
		virtual bool decode( QDataStream& stream );

		// �����¶���
		virtual void copyObject(ResourceObject* obj);

		void setContent(const QByteArray& data);

		QPixmap thumbnail_;   // ����ͼ
		QByteArray content_;  // ������������
		QString format_;  // ԭʼ��ʽ
	
		// ��������ͼ
		virtual void  makeThumbnail(int size = 128);

		// ��ȡԭʼ�ļ���ʽ
		virtual QString  getFormat(){
			return format_;
		}

		// ��ȡ�����ļ��Ի���Ĺ�����
		static QString fileFilters();

		QByteArray hashsha1_;  // ���ݹ�ϣ
	private:

	};

	class ImageResourceObject : public GalleryResourceObject{
		DeclareRTTI()
		DECLARE_RESOURCE_FILETYPE(GALLERY_IMAGE_FILENAME)
	public:
		ImageResourceObject(){}
		ImageResourceObject(const QString& inName, unsigned int flags = URF_Public);

		virtual void encode(QDataStream& stream);
		virtual bool decode(QDataStream& stream);

		// �����¶���
		virtual void copyObject(ResourceObject* obj);

		bool isGroup_;  // ͼƬ������
		QList<QRect> frames_;  // ͼƬ֡λ��

		// ��������ʾ���ǳ־û���
		QImage image_;  // ԭʼͼ��

		// ͼƬ��Ŀ
		int imageCount();
		// ��ȡ��ͼƬ
		QImage getImage(int index);
		// ��ȡԭʼ�ļ���ʽ
		virtual QString  getFormat();

		// ��������ͼ
		virtual void  makeThumbnail(int size = 128);

		// ������ͼƬ
		void setImage(QImage& newImage);

		// ��ȡ�����ļ��Ի���Ĺ�����
		static QString fileFilter();
		// �ж��ļ���չ�����Ժ��Ǹ����ļ����ݣ�
		static bool isValidFile( const QString& ext );
	private:
	};


	class AudioResourceObject : public GalleryResourceObject{
		DeclareRTTI()
		DECLARE_RESOURCE_FILETYPE(GALLERY_AUDIO_FILENAME)
	public:
		AudioResourceObject(){}
		AudioResourceObject(const QString& inName, unsigned int flags = URF_Public);

		virtual QString  getFormat();

		// ��ȡ�����ļ��Ի���Ĺ�����
		static QString fileFilter();
		// �ж��ļ���չ�����Ժ��Ǹ����ļ����ݣ�
		static bool isValidFile(const QString& ext);
	private:
	};

	class VideoResourceObject : public GalleryResourceObject{
		DeclareRTTI()
		DECLARE_RESOURCE_FILETYPE(GALLERY_VIDEO_FILENAME)
	public:
		VideoResourceObject(){}
		VideoResourceObject(const QString& inName, unsigned int flags = URF_Public);

		virtual QString  getFormat();

		// ��ȡ�����ļ��Ի���Ĺ�����
		static QString fileFilter();
		// �ж��ļ���չ�����Ժ��Ǹ����ļ����ݣ�
		static bool isValidFile(const QString& ext);
	private:
	};

	///////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	class SoundResourceObject : public FieldResourceObject{
		DeclareRTTI()
		DECLARE_RESOURCE_FILETYPE(GALLERY_SOUND_FILENAME)
	public:
		SoundResourceObject(){}
		SoundResourceObject(const QString& inName, unsigned int flags = URF_Public);

		virtual void encode(QDataStream& stream);
		virtual bool decode(QDataStream& stream);

		// ��ȡĿǰ���ļ���ʽ
		QStringList formats();
		QList<ResourceHash>  audios_;  // �ļ��б�
	private:
	};

	class MovieResourceObject : public FieldResourceObject{
		DeclareRTTI()
		DECLARE_RESOURCE_FILETYPE(GALLERY_MOVIE_FILENAME)
	public:
		MovieResourceObject(){}
		MovieResourceObject(const QString& inName, unsigned int flags = URF_Public);


		virtual void encode(QDataStream& stream);
		virtual bool decode(QDataStream& stream);

		// ��ȡĿǰ���ļ���ʽ
		QStringList formats();
		QList<ResourceHash>  videos_;  // �ļ��б�
	private:
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	class SpriteResourceObject : public FieldResourceObject{
		DeclareRTTI()
		DECLARE_RESOURCE_FILETYPE(SPRITE_FILENAME)
	public:
		SpriteResourceObject(){}
		SpriteResourceObject(const QString& inName, unsigned int flags = URF_Public);

		virtual void encode(QDataStream& stream);
		virtual bool decode(QDataStream& stream);
		// �����¶���
		virtual void copyObject(ResourceObject* obj);
		// ��¡�¶���
		virtual ResourceObject* clone(const QString& inName);

		virtual ResourceObject* createChild(const QString& inName, RTTIType childType);

		// �����ڲ�ģ��
		template<class T>
		bool decodeChild(QDataStream& stream){
			// ��¼��ʼλ��
			qint64 pos = stream.device()->pos();
			unsigned int ver, hashkey;
			stream >> ver >> hashkey;
			// ���ݹ�ϣ���Ҷ���
			DEFINE_RESOURCE_HASHKEY(T, child, hashkey);
			if (child == NULL){
				child = new T();
			}
			else{
				child->clearFlags(URF_TagGarbage);
				child->setFlags(URF_TagSave);
			}
			// �ص���ʼλ��
			stream.device()->seek(pos);

			if (!child->decode(stream)){
				delete child;
				// ��ʾ����
				return false;
			}

			child->clearFlags(URF_NeedLoad);

			if (child->outer() == NULL){
				child->setOuter(this);
				child->addObject();
			}
			else if (child->outer() != this){
				child->setOuter(this);
			}

			return true;
		}

		void saveViewPort(int lod, const QTransform& trans, const QPointF& center);

		void addImageSource(ResourceHash id, ResourceHash key);
		void removeImageSource(ResourceHash id);

		// ��ȡ��������
		SpriteSequenceResourceObject* sequence(const QString& name);

		// ��ȡ֡����λ��
		QRect frameRect(ResourceHash key, int index);

		QByteArray viewData_;  // �ӿ���Ϣ
		QMap<ResourceHash, ResourceHash> imgSources_;  // �����ļ�Դ

		QString defaultSeq_;  //  ȱʡ����֡
		// �ǳ־û�
		QList<SpriteSequenceResourceObject*> sequences_;  // ���������ж���
	};


	struct SequenceFrame : public ResourceData
	{
		QPointF pos;
		ResourceHash imghost;  // �ز�ID
		int imgindex;  // �ز�������λ��
		QMap<ResourceHash, QPointF> anchors;  // ê��λ��
		QList<QPointF> collisionRegion; // ��ײ����
		QBitArray state;  // ��ת״̬��0��ת�ߵ���1��ת�ߵ���  4ˮƽ��ת  5��ֱ��ת�� 6 �ԽǷ�ת ��

		QImage buffer;  // ͼ�񻺴棨�����棩

		void save(QDataStream& stream){
			stream << pos << imghost << imgindex << anchors << collisionRegion << state;
		}
		void load(QDataStream& stream){
			stream >> pos >> imghost >> imgindex >> anchors >> collisionRegion >> state;
		}
	};


	const QString ANCHOR_CENTER = UDQ_T("Origin");

	class SpriteSequenceResourceObject : public ResourceObject{
		DeclareRTTI()
	public:
		SpriteSequenceResourceObject(){}
		SpriteSequenceResourceObject(const QString& inName, unsigned int flags = URF_Public);

		virtual void encode(QDataStream& stream);
		virtual bool decode(QDataStream& stream);
		// �����¶���
		virtual void copyObject(ResourceObject* obj);

		// ��ȡ����֡ͼƬ��û���ҵ��򷵻ؿ�ͼ��
		void refreshImage(SequenceFrame* frame);

		// ��ȡ֡ͼƬ
		QImage frameSourceImage(int index);
		QImage frameSourceImage(SequenceFrame* frame);

		QImage frameImage(int index);
		QPointF frameAnchor(int index, const QString& anchor);  // 0 ��ʾ����ê��
		QPointF frameCenterAnchor(int index);
		// ��ײ����
		QList<QPointF> frameCollision(int index);

		QPointF pos_;  // λ��
		bool isLocking_;  //
		qreal speed_;  // �����ٶȣ�0~1, ���60FPS��
		bool playback_; // �Ƿ񵹷�
		bool pingpong_;  // ��������
		bool repeat_;  // �ظ�����
		int count_;  // �ظ�����
		int repeatTo_;  // ���Ž���ֹͣ֡��
		QStringList anchors_;  // ê���б�
		TResourceList<SequenceFrame> frames_;  // ����֡

	private:
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	enum{
		VAR_EXTEND = 1,  // ��չ����
	};

	struct PluginVariable : public ResourceData
	{
		QString name;
		QString title;
		QString datatype;
		QStringList enums;
		QString desc;
		QString value;
		int flags;  // ������־

		PluginVariable() : flags(0){}

		void save(QDataStream& stream){
			stream << name << datatype << title << enums << value << desc << flags;
		}
		void load(QDataStream& stream){
			stream >> name >> datatype >> title >> enums >> value >> desc >> flags;
		}
	};


	struct PluginInterface : public ResourceData
	{
		int type;
		QString name;
		QString category;
		QStringList flags;
		QString desc;
		QString func;
		TResourceList<PluginVariable> params;  // ����������

		PluginVariable* findParam(const QString& name){
			foreach(PluginVariable* var, params.values()){
				if (var->name == name){
					return var;
				}
			}
			return NULL;
		}

		bool hasFlag(int flag){
			return flags.contains(Value2String(flag));
		}
			
		void save(QDataStream& stream){
			stream << name << type <<  flags << category << func << desc;
			stream << params.toByteArray();
		}
		void load(QDataStream& stream){
			stream >> name >> type >> flags >> category >> func >> desc;
			QByteArray data;
			stream >> data;
			params.fromByteArray(data);
		}
	};


	class PluginResourceObject : public FieldResourceObject{
		DeclareRTTI()
		DECLARE_RESOURCE_FILETYPE(PLUGIN_FILENAME)
	public:
		PluginResourceObject() : commonPlugin_(NULL){}
		PluginResourceObject(const QString& inName, unsigned int flags = URF_Public);

		virtual void encode(QDataStream& stream);
		virtual bool decode(QDataStream& stream);

		// ��ʼ������
		void postInit();
		// �������ȱʡֵ
		void processParameterDefault(const QString& prefix, PluginInterface* inter);
		QString getParameterDefault(PluginVariable* var);

		// ��ȡ
		QString getVariableDefault(const QString& prop);

		QString getParameterDefault(unsigned int key);
		QString getConditionParameterDefault(const QString& prop, const QString& parent);
		QString getActionParameterDefault(const QString& prop, const QString& parent);
		QString getExpressParameterDefault(const QString& prop, const QString& parent);

		// ��ȡ��������
		PluginVariable* getVariable(const QString& prop);

		PluginVariable* getConditionVariable(const QString& prop, const QString& parent);
		PluginVariable* getActionVariable(const QString& prop, const QString& parent);
		PluginVariable* getExpressVariable(const QString& prop, const QString& parent);

		void setVariableValue(const QString& name, const QString& value);
	
		void copyObject(ResourceObject* obj);
		int type_;  // ������ͣ�0����1��Ϊ��2��Ч��
		bool devmode_;  // ϵͳ���������ֻ���ڿ�����ģʽ�±༭��

		QString title_;  // ��ʾ����
		QString modelVer_;  // ģ�Ͱ汾
		int eid_;  // ģ�͵���ID��ÿ�ε���ʱ�ۼӣ�
		QImage icon_;  // ICON

		QSize  extendBound_;  // ��Χ����չ
		QMap<int, int>  attributes_;  // ����

		// �Ƿ��������
		bool hasAttribute(int);

		QMultiHash<int, QByteArray>  filedata_;  // �����ļ����ݣ�Ŀǰֻ��1����
		QList<ResourceHash> varIndex_; //  ����˳������
		TResourceMap<PluginVariable>  variables_;
		TResourceMap<PluginInterface>  conditions_;  // ����
		TResourceMap<PluginInterface>  actions_;    // ����
		TResourceMap<PluginInterface>  expresses_;   // ���ʽ

		PluginInterface* getAction(ResourceHash key);
		PluginInterface* getCondition(ResourceHash key);
		PluginInterface* getExpress(ResourceHash key);

		static PluginResourceObject* acquireCommonPlugin();
		static PluginResourceObject* acquirePlugin(const QString& name );

		//  runtime�ַ���
		QString runtimeString(PluginInterface* inter = NULL);
		QString runtimeString(int type, ResourceHash key);

		//  �ǳ־û�
		QMap<ResourceHash, QString>  varDefaults_;  // ���в���ȱʡֵ
		QMap<ResourceHash, QString>  paramDefaults_;  // ���в���ȱʡֵ
		PluginVariable* getVariableDefine(const QString& prop, const QString& parent, TResourceMap<PluginInterface>& container);
		PluginResourceObject* commonPlugin_;  // ��������ӿ�
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	class WorkspaceResourceObject : public ResourceObject{
		DeclareRTTI()
	public:
		WorkspaceResourceObject(){}
		WorkspaceResourceObject( const QString& inName,  unsigned int flags =  URF_Transient | URF_Public );

		virtual ResourceObject* createChild( const QString& inName,  RTTIType childType );

		void reset();

		// ���ص�ǰ�̬�ƣ�Ŀǰһ�������ռ��½�����һ����Ϸ��
		ProjectResourceObject* currentProject();
		void activiteProject(unsigned int hash);
		ProjectResourceObject* findProject(const QString& inName, bool create = false);
		void deleteProject(unsigned int hash);

		QString dirPath_;    // �����ռ�Ŀ¼ 
		int projectType_;  // ��Ŀ����
		QString tmpName_; // ��ʱ��Ŀ��

		// ������Ϣ��д
		bool saveConfig();
		bool loadConfig();

	protected:
		ResourceHash  activitedH_;  // ��ǰ���������

	};

	//  ȫ��ģʽ
	enum{
		FULLSCR_NONE = 0,
		FULLSCR_CROP,
		FULLSCR_SCALE_INNER,
		FULLSCR_SCALE_OUTER,
		FULLSCR_LETTERBOX_SCALE,
		FULLSCR_LETTERBOX_INTSCALE,
	};

	// ��������
	enum{
		PHYENG_BOX2D = 0,
	};

	// ý������
	enum{
		MEDIA_IMAGE = 0,
		MEDIA_AUDIO,
		MEDIA_VIDEO,
		MEDIA_MISC,
		MEDIA_PROJECT,  // ������
	};

	enum{
		ORIENT_ANY,
		ORIENT_PORTRAIT,
		ORIENT_LANDSCAPE,
	};

	enum{
		QUALITY_LOW,
		QUALITY_HIGH,
	};

	enum{
		MIPMAP_NO,
		MIPMAP_DENSE,
		MIPMAP_SPARSE,
	};

	enum{
		SAMPLE_LINEAR,
		SAMPLE_POINT,
	};

	enum{
		LOADER_PROGLOGO,
		LOADER_PROGRESS,
		LOADER_PERCENT,
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	class ProjectResourceObject : public ResourceObject{
		DeclareRTTI()
		DECLARE_RESOURCE_FILETYPE( PROJECT_FILENAME )

	public:
		ProjectResourceObject(){}
		ProjectResourceObject(const QString& inName, unsigned int flags = URF_Public);
	
		virtual void encode( QDataStream& stream );
		virtual bool decode( QDataStream& stream );

		ResourceObject* createChild(const QString& inName, RTTIType childType);

		QString pubVersion_; // �����汾
		QString email_;  // ����
		QString website_;  // ��Ŀ����վ
		QString domainId_; // ID

		QDateTime createDate_;  // ��������
		QDateTime lastModifyDate_;  // ��һ���޸�����
		QString author_;  // ����
		QPixmap thumbnail_; // ����ͼ��128*128��
		QSize resolution_;  // �ֱ���
		bool aspect_;  // �Ƿ�̶������
		QString startLevel_;  // ����������

		int fullscreenMode_;  // ȫ��ģʽ
		int fullscreenQuality_;  // ��������
		bool useHighDPI_;
		int sampleType_;
		int orientation_;  // ����
		int downScaling_;  // ��С
		int physicalEngine_;  // ��������
		int loaderStyle_;  // ����ҳ����ʽ
		bool useLoaderLayout_;  //
		bool useWebGL_;
		bool pixelRounding_;
		bool preloadSound_; //  Ԥ��������
		bool pauseOnUnfocus_;  // ʧȥ������ֹͣ����
		bool clearBackground_;  // �Ƿ�����������رտ����Ч�ʣ�

		// ��������
		QString exportDir_;
		QMap<int, QString> mediaDirs_;
		bool minfyScript_; 

		QStringList objectGroups_;  // �������

		QString getDefMediaDir(int type);
		QString getExportName();

		// �����³���
		LevelResourceObject* addNewLevel(const QString& inName, int index = -1);
		// ��ȡ����
		LevelResourceObject* getLevel(const QString& name);

		// �����¶�������
		ObjectTypeResourceObject* addNewType(PluginResourceObject* prototype, const QString& inName = QString() );
		// ���Ҷ�������
		ObjectTypeResourceObject* getObjectType(PluginResourceObject* prototype, const QString& inName = QString());
		ObjectTypeResourceObject* getObjectType(const QString& inName);


		// �����½ű�
		BlueprintResourceObject* addNewScript(const QString& inName = QString());

		// �����¼���
		FamilyResourceObject* addNewFamily( const QString& inName = QString());

		FamilyResourceObject* getFamily(const QString& inName);


		// ���ҳ���
		int indexOf(LevelResourceObject* level);
		int indexOf(ResourceHash hash);

		// �ƶ�����
		void moveLevel(int src, int dest);
		void moveLevel(LevelResourceObject* level, int index);
		// ɾ������
		void removeLevel(LevelResourceObject* level);

		// �ű��Ƿ���Ч
		bool validScript();

		// �ǳ־�����
		QList<LevelResourceObject*> levels_;  // �����б�0Ϊȱʡ��ʼ�����������޴���
		QList<ObjectTypeResourceObject*> objTypes_;  // ��������
		QList<BlueprintResourceObject*> blueprints_; // �ű�
		QList<FamilyResourceObject*> families_;  // ����
		QList<SoundResourceObject*> sounds_;
		QList<MovieResourceObject*> movies_;

		QTreeWidgetItem* leveltreeItem_;  //

		LayerResourceObject* currentLayer_;  // ��ǰ���ڱ༭��ͼ��
		QString exportTitle_;  // ����������
		int exportType_;  // �������� 0��ʽ������ 1Ԥ���� 2����

		// ���Һ��Ӷ���
		template<class T>
		T* findChild(const QString& inName, bool create = false ){
			return TFindWSResource<T>(outer()->objectName(), objectName(), inName, create );
		}

		//  ����Ψһ����
		template<class T>
		QString makeUniqueName(const QString& baseName){
			static int index = 0;
			QString newName = baseName;

			QString path;
			while (true){
				if (index == 0){
					newName = baseName;
					index++;
				}
				else{
					newName = QString(UDQ_T("%1%2")).arg(baseName).arg(index++);
				}

				T* t = TFindWSResource<T>(outer()->objectName(), objectName(), newName);
				if (t == NULL){
					break;
				}
			}
			return newName;
		}

	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	class LevelResourceObject : public ResourceObject{
		DeclareRTTI()
	public:
		LevelResourceObject(){}
		LevelResourceObject(const QString& inName, unsigned int flags = URF_Public);

		QPixmap thumbnail_; // ����ͼ��128*128��

		QSize size_;  //  ������С�������ƶ���
		QSize margin_;  // �����߿�
		bool unboundScroll_;  // ���޹���
		ResourceHash blueprint_;  // �����¼�ģ��

		QList<QString> effects_;  // ��Ч
		QMap<QString, PluginResourceObject*> attachments_;  //��Ч
		QMap<ResourceHash, QString>  defaults_;  // ����ȱʡֵ��keyΪ pluginName.paramName��
		QList<QByteArray> instances_;  // ʵ������

		virtual void encode(QDataStream& stream);
		virtual bool decode(QDataStream& stream);

		// ����
		virtual void clear();

		int indexOf(LayerResourceObject* layer);
		int indexOf(ResourceHash hash);

		// �������ҳ��
		int insertLayer(LayerResourceObject* layer, int index = -1);
		void removeLayer(LayerResourceObject* layer);

		// �ƶ�����
		void moveLayer(int src, int dest);
		// ��������
		void exchangeLayer(int src, int dest);

		// ��������
		void copyObject(ResourceObject* obj);

		// ������ͼ��
		LayerResourceObject* addNewLayer(const QString& inName, int index = -1);

		QTransform matrix_;
		QPointF viewport_;
		int zoomStep_;

		LayerResourceObject* findLayer(const QString& inName);

		// �ǳ־�����
		QList<LayerResourceObject*> layers_;   // ҳ���б�
		QTreeWidgetItem* treeItem_;  // Ŀ¼����Ӧ��

		// ��������ͼ
		void setThumb(QImage& img);
		void updateThumb();
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	class LayerResourceObject : public ResourceObject{
		DeclareRTTI()
	public:
		LayerResourceObject(){}
		LayerResourceObject(const QString& inName, unsigned int flags = URF_Public);

		virtual void encode(QDataStream& stream);
		virtual bool decode(QDataStream& stream);

		// ����ֵ����
		QByteArray saveProperty();
		void restoreProperty(QByteArray& data);

		void copyObject(ResourceObject* obj);

		QPixmap thumbnail_; // ����ͼ��128*128��
		bool isLocking_;  // �Ƿ�����
		bool isVisible_;    //�Ƿ�ɼ�
		bool isGlobal_;  //  �Ƿ�ȫ��

		bool initVisible_;  // ��ʼ�ɼ�
		int scale_; // ������ű�����0��ʾ������)
		QPoint parallax_;   // �����ٶȣ��ٷֱȣ�
		QColor bgColor_;  // ������ɫ
		int opacity_;  // ͸����
		bool transparent_; // �Ƿ񱳾�͸��
		bool ownTexture_;  // ������
		bool useRenderCell_;  // �Ƿ�ʹ����Ⱦ����
		QList<QByteArray> instances_;  // ͼ��ʵ������

		int blend_; // ���ģʽ
		int fallback_;  // �ص�����

		QList<QString> effects_;  // ��Ч
		QMap<QString, PluginResourceObject*> attachments_;  //��Ч
		QMap<ResourceHash, QString>  defaults_;  // ����ȱʡֵ��keyΪ pluginName.paramName��

		// �ǳ־�����
		QTreeWidgetItem* treeItem_;  // Ŀ¼����Ӧ��
		LevelResourceObject* level_; // ͼ�����ڳ���
	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	enum{
		FBT_NORMAL = 0,
		FBT_ADDITIVE,
		FBT_XOR,
		FBT_COPY,
		FBT_DEST_OVER,
		FBT_SRC_IN,
		FBT_DEST_IN,
		FBT_SRC_OUT,
		FBT_DEST_OUT,
		FBT_SRC_ATOP,
		FBT_DEST_ATOP,
		FBT_DESTORY,
	};


	class ObjectTypeResourceObject : public ResourceObject{
		DeclareRTTI()
	public:
		ObjectTypeResourceObject(){}
		ObjectTypeResourceObject(const QString& inName, unsigned int flags = URF_Public);

		virtual void encode(QDataStream& stream);
		virtual bool decode(QDataStream& stream);

		void copyObject(ResourceObject* obj);

		PluginResourceObject*  prototype_;   // ԭ��
		bool global_;  // ȫ�ֶ���
		QList<QString> behaviors_;
		QList<QString> effects_;
	
		QMap<QString, PluginResourceObject*> attachments_;  // ��Ϊ����Ч

		TResourceMap<PluginVariable> variables_;  // ʵ���������壨ֻ�в��������������������ַ���4�����ͣ�
		QMap<ResourceHash, QString>  defaults_;  // ����ȱʡֵ��keyΪ pluginName.paramName��

		PluginResourceObject* getPlugin(const QString& pluginName);

		// �Ƿ��ָ�����͵���Ϊ
		bool hasAttachment(ResourceHash key);
		bool hasAttachment(PluginResourceObject* plugin);

		// ��ȡ��������
		PluginVariable* getVariableDefine(const QString& prop, const QString& pluginName = QString());

		// ��д����
		QString getParameter(const QString& prop, const QString& pluginName = QString());
		void setParameter(const QString& prop, const QString& value, const QString& pluginName = QString());
	
		// ��ʼ������
		void postInit();

		// ����
		void clear();

		// ��ȡȱʡ��������
		SpriteSequenceResourceObject* getDefaultAnimation();

		// �л�Ϊ��̬ͼƬģʽ
		bool setImageResource(ResourceHash key);
		bool hasImageResource();
		// ���ͼ����Դ
		void clearImageResource();

		//����ͼ
		QPixmap thumbnailImage(int size = 64);

		QByteArray defaultData_;  // ȱʡ��������
		QByteArray tilemapData_;  // TileMap�������ݣ���ײ����

		QString categroy_;  // �������

		// �ǳ־�����
		QTreeWidgetItem* treeItem_;  // Ŀ¼����Ӧ��
		SpriteResourceObject* animation_;  // ���鶯��
		ImageResourceObject* staticImage_;  // ��̬ͼƬ
		ResourceHash staticSID_;  // ��̬SID

	};

	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	class FamilyResourceObject : public ResourceObject{
		DeclareRTTI()
	public:
		FamilyResourceObject(){}
		FamilyResourceObject(const QString& inName, unsigned int flags = URF_Public);

		virtual void encode(QDataStream& stream);
		virtual bool decode(QDataStream& stream);

		void copyObject(ResourceObject* obj);

		QList<QString> behaviors_;
		QList<QString> effects_;
		QMap<QString, PluginResourceObject*> attachments_;  // ��Ϊ����Ч

		TResourceMap<PluginVariable> variables_;  // ʵ���������壨ֻ�в��������������������ַ���4�����ͣ�
		QMap<ResourceHash, QString>  defaults_;  // ����ȱʡֵ��keyΪ pluginName.paramName��

		// ��ȡ��������
		PluginVariable* getVariableDefine(const QString& prop, const QString& pluginName = QString());
		// ��д����
		QString getParameter(const QString& prop, const QString& pluginName = QString());
		void setParameter(const QString& prop, const QString& value, const QString& pluginName = QString());
		// ��ʼ������
		void postInit();
		// ����
		void clear();

		// �ǳ־�����
		QTreeWidgetItem* treeItem_;  // Ŀ¼����Ӧ��
		QList<ObjectTypeResourceObject*> members_;  //  ��Ա����
	};


	//////////////////////////////////////////////////////////////////////////
	//
	//////////////////////////////////////////////////////////////////////////

	struct ScriptVariable : public ResourceData
	{
		QString name;
		QString datatype;
		QString value;
		QString desc;

		void save(QDataStream& stream){
			stream << name << datatype << value << desc;
		}
		void load(QDataStream& stream){
			stream >> name >> datatype >> value >> desc;
		}
	};

	class BlueprintResourceObject : public ResourceObject{
		DeclareRTTI()
	public:
		BlueprintResourceObject(){}
		BlueprintResourceObject(const QString& inName, unsigned int flags = URF_Public);

		void clearAll();

		virtual void encode(QDataStream& stream);
		virtual bool decode(QDataStream& stream);

		void copyObject(ResourceObject* obj);

		// ����±�����ȱʡ�ַ������ͣ�
		ScriptVariable* addNewVariable(const QString& name =QString());

		//  ������Ч������
		QString makeVariableName();

		// ��ʾ����
		bool func_;  // �Ƿ�Ϊ����
		QTransform matrix_;
		QPointF viewport_;
		int zoomStep_;

		TResourceMap<ScriptVariable> variables_;  // ����������Ǹ��ű�����Ϊȫ�ֱ���������Ϊ�ֲ�������
		// �����б�
		QList<QByteArray> events_;  // �¼�������
		QList<QByteArray> links_;  // ���߶����б�
		QList<QByteArray> groups_; // ������б�
		QList<QByteArray> assets_; // ���������б�
		QList<ResourceHash> includes_;  // ����������Blueprint����( ��ֹѭ������ )

		QByteArray scriptJson_;  // �ű�����
		
		// �ǳ־�����
		BlueprintResourceObject* parent_;  // ���ű���������
		QTreeWidgetItem* treeItem_;  // Ŀ¼����Ӧ��
		bool hasError_;  // �д���

	};

}

#endif
