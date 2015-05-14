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

#ifndef ORION_RESOURCELINK_H
#define ORION_RESOURCELINK_H

#include "resourceobject.h"
#include <db_cxx.h>

namespace ORION{

	class ResourceLinker : public ResourceObject{
		DeclareRTTI()
	public:
		ResourceLinker( ResourceObject* inRoot, const QString& inFilename, unsigned int flags =  URF_Transient | URF_Public );
		
		// �������ж���(������������)
		template<class T>
		void loadAll( unsigned int loadFlags ){

			// �����б�
			Db db_Dat(NULL, 0); 
			Dbc *db_Cursor = 0;
			Dbt key, data;

			u_int32_t oFlags = DB_CREATE; 
			try {
				std::string arrstr = filePath_.toStdString();
				db_Dat.open(NULL, 
					arrstr.c_str(),/*arr.data(), */
					NULL, 
					DB_BTREE, 
					oFlags, 
					0); 

				// ��ȡ����
				db_Dat.cursor( NULL, &db_Cursor, 0);
				memset(&key, 0, sizeof(key));
				memset(&data, 0, sizeof(data));

				int ret;

				while ((ret = db_Cursor->get( &key, &data, DB_NEXT)) == 0){
					// ����hash����
					unsigned int hashKey = *((unsigned int*)key.get_data());
					ResourceObject* res = ResourceObject::findResource( hashKey );

					// ����hash�ı仯
					if( !res ){
						res = new T();

						if( loadFlags & RLF_LoadIndex ){
							res->setFlags( URF_NeedLoad );
						}
						else{
							res->clearFlags( URF_NeedLoad );
						}

						QByteArray buff( (char*)data.get_data(), data.get_size() );
						QDataStream stream( buff );

						if( !res->decode(stream) ){
							delete res;
							break;
						}

						// 
						Q_ASSERT( linkerRoot_ != 0 );
						res->setOuter(linkerRoot_);
						res->addObject();
						res->setLinker( this );
						resourceMap_.append( res );
					}
					else{
						if( !(loadFlags & RLF_LoadIndex ) ){
							res->clearFlags( URF_NeedLoad );
							QByteArray buff( (char*)data.get_data(), data.get_size() );
							QDataStream stream( buff );
							res->decode(stream);
						}
					}
				}

				if (db_Cursor != NULL)
					db_Cursor->close();
				db_Dat.close(0);
			} catch(DbException &e) {
			} catch(std::exception &e) {
			}
		}

		// �������ж���
		template<class T>
		void saveAll(){
			Db db_Dat(NULL, 0); 
			u_int32_t oFlags = DB_CREATE; 
			try {
				std::string arrstr = filePath_.toStdString();
				db_Dat.open(NULL, 
					arrstr.c_str(),
					NULL, 
					DB_BTREE, 
					oFlags, 
					0); 

				foreach( ResourceObject* obj, resourceMap_){
					Q_ASSERT( obj != 0 );
					if( obj->getRTTI() == &T::RTTI_ ){
						// 
						if( obj->flags() & URF_TagSave ){
							// ����
							unsigned int hashkey = obj->hashKey();
							Dbt key(&hashkey, sizeof(unsigned int));

							T* res = (T*)obj;

							// ���涨��
							QByteArray defdat;
							QDataStream stream( &defdat, QIODevice::WriteOnly);
							res->encode( stream );

							char* buff = defdat.data();
							Dbt data( buff, defdat.size() );
							db_Dat.put(NULL, &key, &data, 0 );

							// ������λ
							obj->clearFlags(URF_TagSave);
						}
						else if( obj->flags() & URF_TagGarbage ){
							// ɾ��
							ResourceHash hashkey = obj->hashKey();
							Dbt key(&hashkey, sizeof(unsigned int));
							db_Dat.del( NULL, &key,  0 );
						}
					}
				}

				db_Dat.close(0);

			} catch(DbException &e) {
			} catch(std::exception &e) {
			}

		}

		// ����
		template<class T>
		void save( unsigned int inkey ){
			// �����б�
			Db db_Dat(NULL, 0); 
			Dbt key(&inkey, sizeof(unsigned int));

			u_int32_t oFlags = DB_CREATE; 
			try {
				std::string arrstr = filePath_.toStdString();
				db_Dat.open(NULL, 
					arrstr.c_str(),
					NULL, 
					DB_BTREE, 
					oFlags, 
					0); 

				ResourceObject* obj = ResourceObject::findResource( inkey );
				Q_ASSERT( obj != 0  );

				if( obj->flags() & URF_TagSave ){	
					// ���涨��
					QByteArray defdat;
					QDataStream stream( &defdat, QIODevice::WriteOnly);
					obj->encode( stream );

					char* buff = defdat.data();
					Dbt data( buff, defdat.size() );
					db_Dat.put(NULL, &key, &data, 0 );
				}
				else if( obj->flags() & URF_TagGarbage ){
					// ɾ��
					db_Dat.del( NULL, &key,  0 );
				}

				db_Dat.close(0);
				obj->clearFlags( URF_TagSave );

			} catch(DbException &e) {
			} catch(std::exception &e) {
			}
		}

		// ����
		template<class T>
		void load( ResourceHash inkey, unsigned int loadFlags ){
			// �����б�
			Db db_Dat(NULL, 0); 
			Dbt key(&inkey, sizeof(unsigned int));
			Dbt data;

			u_int32_t oFlags = DB_CREATE; 
			try {
				std::string arrstr = filePath_.toStdString();
				db_Dat.open(NULL, 
					arrstr.c_str(),
					NULL, 
					DB_BTREE, 
					oFlags, 
					0); 

				db_Dat.get( NULL, &key, &data, 0 );

				if( data.get_size() <= 0 ){
					db_Dat.close(0);
					return;
				}

				ResourceObject* res = ResourceObject::findResource( inkey );
				// ����hash�ı仯
				if( !res ){
					res = new T();
					
					if( loadFlags & RLF_LoadIndex ){
						res->setFlags( URF_NeedLoad );
					}
					else{
						res->clearFlags( URF_NeedLoad );
					}

					QByteArray buff( (char*)data.get_data(), data.get_size() );
					QDataStream stream( buff );

					if( !res->decode(stream) ){
						delete res;
					}
					else{
						Q_ASSERT( linkerRoot_ != 0 );
						res->setOuter( linkerRoot_);		
						res->addObject();
						res->setLinker( this );
						resourceMap_.append( res );
					}

				}
				else{
					if( !(loadFlags & RLF_LoadIndex ) ){
						res->clearFlags( URF_NeedLoad );
						QByteArray buff( (char*)data.get_data(), data.get_size() );
						QDataStream stream( buff );
						res->decode(stream);
					}
				}

				db_Dat.close(0);

			} catch(DbException &e) {
			} catch(std::exception &e) {
			}
		}

		// ɾ��
		template<class T>
		void remove( unsigned int inkey ){
			// �����б�
			Db db_Dat(NULL, 0); 
			Dbt key(&inkey, sizeof(unsigned int));
			Dbt data;

			u_int32_t oFlags = DB_CREATE; 
			try {
				std::string arrstr = filePath_.toStdString();
				db_Dat.open(NULL, 
					arrstr.c_str(),
					NULL, 
					DB_BTREE, 
					oFlags, 
					0); 

				db_Dat.del( NULL, &key, 0 );
				ResourceObject* res = ResourceObject::findResource( inkey );
				Q_ASSERT( res != 0 );
				res->setLinker( 0 );
				resourceMap_.removeAll( res );

				db_Dat.close(0);

			} catch(DbException &e) {
			} catch(std::exception &e) {
			}
		}

		// ���
		void clear();

		QString filePath(){
			 return filePath_;
		}

		ResourceObject* linkerRoot(){
			return linkerRoot_; 
		}

		void detach(ResourceObject* inObj ){
			resourceMap_.removeAll( inObj );
		}

		void attach(ResourceObject* inObj ){
			resourceMap_.append( inObj );
		}

	protected:
		ResourceObject* linkerRoot_;  //  ������
		QString filePath_;  // �ļ�����·��
		unsigned int contextFlags_;
		QList<ResourceObject*> resourceMap_;	// ��Դӳ��
	};

	// ����ָ�����͵�������Դ
	template<class T>
	void ApplyResourceLinkers(){
		for( TResourceObjectIterator<ResourceLinker> it; it; ++it ){
			ResourceLinker* linker = (ResourceLinker*)(*it);
			linker->saveAll<T>();
		}
		for( TResourceObjectIterator<T> it; it; ++it ){
			T* res = (T*)(*it);
			if( res->flags() & URF_TagGarbage ){
				res->unhashObject();
				res->setLinker(0);
			}
		}
		ResourceObject::purgeGarbage();
	}

}


#endif
