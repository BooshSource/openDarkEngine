/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
 *
 *    This program is free software; you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation; either version 2 of the License, or
 *    (at your option) any later version.
 *
 *    This program is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with this program; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 *****************************************************************************/


#include "LinkService.h"
#include "BinaryService.h"
#include "logger.h"
#include "ServiceCommon.h"

using namespace std;

namespace Opde {
	/// helper string iterator over map keys
	class RelationNameMapKeyIterator : public StringIterator {
		public:
			RelationNameMapKeyIterator(LinkService::RelationNameMap& relationMap) :
                            mRelationMap(relationMap) {
				mIter = mRelationMap.begin();
				mEnd = mRelationMap.end();
            }

            virtual const std::string& next() {
				assert(!end());
				
				const std::string& s = mIter->first;

				++mIter;

				return s;
            }

            virtual bool end() const {
                return (mIter == mEnd);
            }

        protected:
			LinkService::RelationNameMap::iterator mIter, mEnd;
            LinkService::RelationNameMap& mRelationMap;
	};

	/*----------------------------------------------------*/
	/*-------------------- LinkService -------------------*/
	/*----------------------------------------------------*/
	LinkService::LinkService(ServiceManager *manager, const std::string& name) : Service(manager, name), mDatabaseService(NULL) {
	}

	//------------------------------------------------------
	LinkService::~LinkService() {
		mRelationNameMap.clear();
		mRelationIDMap.clear();
		mNameToFlavor.clear();
		mFlavorToName.clear();
	}

	//------------------------------------------------------
	bool LinkService::init() {
	    return true;
	}

	//------------------------------------------------------
	void LinkService::bootstrapFinished() {
		// Ensure link listeners are created
		mServiceManager->createByMask(SERVICE_LINK_LISTENER);

		mDatabaseService = static_pointer_cast<DatabaseService>(ServiceManager::getSingleton().getService("DatabaseService"));
	}

	//------------------------------------------------------
	void LinkService::shutdown() {
		mDatabaseService = NULL;
	}

	//------------------------------------------------------
	void LinkService::load(const FileGroupPtr& db, const BitArray& objMask) {
		LOG_INFO("LinkService: Loading link definitions from file group '%s'", db->getName().c_str());

		// First, try to build the Relation Name -> flavor and reverse records
		/*
		The Relations chunk should be present, and the same for all File Groups
		As we do not know if something was already initialised or not, we just request mapping and see if it goes or not
		*/
		// BinaryServicePtr bs = static_pointer_cast<DatabaseService>(ServiceManager::getSingleton().getService("BinaryService"));

		FilePtr rels = db->getFile("Relations");

		int count = rels->size() / 32;

		LOG_DEBUG("LinkService: Loading Relations map (%d items - %d size)", count, rels->size());

		for (int i = 1; i <= count; i++) {
			char text[32];

			rels->read(text, 32);

			std::string stxt = text;

			if (stxt.substr(0,1) == "~")
				OPDE_EXCEPT("Conflicting name. Character ~ is reserved for inverse relations. Conflicting name : " + stxt, "LinkService::_load");

			// Look for relation with the specified Name

			// TODO: Look for the relation name. Have to find it.
			RelationNameMap::iterator rnit = mRelationNameMap.find(text);

			if (rnit == mRelationNameMap.end())
				OPDE_EXCEPT(string("Could not find relation ") + text + " predefined. Could not continue", "LinkService::_load");

			RelationPtr rel = rnit->second;

			// Request the mapping to ID
			if (!requestRelationFlavorMap(i, text, rel))
				OPDE_EXCEPT(string("Could not map relation ") + text + " to flavor. Name/ID conflict", "LinkService::_load");

			LOG_DEBUG("Mapped relation %s to flavor %d", text, i);
			
			std::string inverse = "~" + stxt;
			
			// --- Assign the inverse relation as well here:
			rnit = mRelationNameMap.find(inverse);

			if (rnit == mRelationNameMap.end())
				OPDE_EXCEPT(string("Could not find inverse relation ") + inverse + " predefined. Could not continue", "LinkService::_load");

			RelationPtr irel = rnit->second;

			// Request the mapping to ID
			if (!requestRelationFlavorMap(-i, inverse, irel))
				OPDE_EXCEPT(string("Could not map inverse relation ") + inverse + " to flavor. Name/ID conflict", "LinkService::_load");

			LOG_DEBUG("Mapped relation pair %s, %s to flavor %d, %d", text, inverse.c_str(), i, -i);

			// TODO: request relation ID map, must not fail

			// Now load the data of the relation
			LOG_DEBUG("Loading relation %s", text);

			try {
				rel->load(db, objMask); // only normal relation is loaded. Inverse is mapped automatically
			} catch (BasicException &e) {
				LOG_FATAL("LinkService: Caught a fatal exception while loading Relation %s : %s", text, e.getDetails().c_str() );
			}
		}
	}

	//------------------------------------------------------
	void LinkService::save(const FileGroupPtr& db, uint saveMask) {
		// Iterates through all the relations. Writes the name into the Relations file, and writes the relation's data using relation->save(db, saveMask) call

		// I do not want to have gaps in the relations indexing, which is implicit given the record order
		int order = 1;

		FilePtr rels = db->createFile("Relations", mRelVMaj, mRelVMin);

		RelationIDMap::iterator it = mRelationIDMap.begin();

		for (; it != mRelationIDMap.end(); ++it, ++order) {
			if (order != it->first)
				OPDE_EXCEPT("Index order mismatch, could not continue...", "LinkService::save");

			// Write the relation's name
			char title[32];

			// get the name, and write
			const std::string& rname = it->second->getName();

			rname.copy(title, 32 - 1);

			// write
			rels->write(title, 32);

			// write the relation
			it->second->save(db, saveMask);
		}
	}

	//------------------------------------------------------
	int LinkService::nameToFlavor(const std::string& name) {
		NameToFlavor::const_iterator it = mNameToFlavor.find(name);

		if (it != mNameToFlavor.end()) {
			return it->second;
		} else {
			LOG_ERROR("LinkService: Relation not found : %s", name.c_str());
			return 0; // just return 0, so no exception will be thrown
		}
	}
	
	//------------------------------------------------------
	std::string LinkService::flavorToName(int flavor) {
		FlavorToName::const_iterator it = mFlavorToName.find(flavor);

		if (it != mFlavorToName.end()) {
			return it->second;
		} else {
			LOG_ERROR("LinkService: Relation not found by flavor : %d", flavor);
			return ""; // just return empty string
		}
	}

	//------------------------------------------------------
	RelationPtr LinkService::createRelation(const std::string& name, const DataStoragePtr& stor, bool hidden) {
		if (name.substr(0,1) == "~")
			OPDE_EXCEPT("Name conflict: Relation can't use ~ character as the first one, it's reserved for inverse relations. Conflicting name: " + name, "LinkService::createRelation");
		
		std::string inverse = "~" + name;
		
		RelationPtr nr = new Relation(name, stor, false, hidden);
		RelationPtr nrinv = new Relation(inverse, stor, true, hidden);

		// Assign inverse relations...
		nr->setInverseRelation(nrinv.ptr());
		nrinv->setInverseRelation(nr.ptr());

		// insert both
		std::pair<RelationNameMap::iterator, bool> res = mRelationNameMap.insert(make_pair(name, nr));

		if (!res.second)
			OPDE_EXCEPT("Failed to insert new instance of Relation named " + name, "LinkService::createRelation");
			
		// Inverse relation now
		res = mRelationNameMap.insert(make_pair(inverse, nrinv));

		if (!res.second)
			OPDE_EXCEPT("Failed to insert new instance of Relation", "LinkService::createRelation");

		return nr;
	}

	

	//------------------------------------------------------
	void LinkService::clear() {
		// clear all the mappings
		mRelationIDMap.clear();
		mFlavorToName.clear();
		mNameToFlavor.clear();

		// clear all the relations
		RelationNameMap::iterator it = mRelationNameMap.begin();

		for (; it != mRelationNameMap.end(); ++it ) {
			//  request clear on the relation
			it->second->clear();
		}
	}

	//------------------------------------------------------
	LinkQueryResultPtr LinkService::getAllLinks(int flavor, int src, int dst) const {
		// find the relation with the specified flavor. 
		//If none such found, return empty iterator
		RelationIDMap::const_iterator it = mRelationIDMap.find(flavor);
		
		if (it != mRelationIDMap.end()) {
			// dedicate to the given relation
			return it->second->getAllLinks(src, dst);
		} else {
			return new EmptyLinkQueryResult();
		}
	}

	//------------------------------------------------------
	LinkPtr LinkService::getOneLink(int flavor, int src, int dst) const {
		// find the relation with the specified flavor. 
		//If none such found, return NULL link
		RelationIDMap::const_iterator it = mRelationIDMap.find(flavor);
		
		if (it != mRelationIDMap.end()) {
			// dedicate to the given relation
			return it->second->getOneLink(src, dst);
		} else {
			return NULL;
		}
	}

	//------------------------------------------------------
	LinkPtr LinkService::getLink(link_id_t id) const {
		// get relation flavor from link id
		int flavor = LINK_ID_FLAVOR(id);
		
		// find relation
		RelationIDMap::const_iterator it = mRelationIDMap.find(flavor);
		
		if (it != mRelationIDMap.end()) {
			// dedicate to the given relation
			return it->second->getLink(id);
		} else {
			return NULL;
		}
	}


	// --------------------------------------------------------------------------
	StringIteratorPtr LinkService::getAllLinkNames() {
		return new RelationNameMapKeyIterator(mRelationNameMap);
	}

	//------------------------------------------------------
	bool LinkService::requestRelationFlavorMap(int id, const std::string& name, RelationPtr& rel) {
		std::pair<FlavorToName::iterator, bool> res1 = mFlavorToName.insert(make_pair(id, name));

		if (!res1.second) {
			if (res1.first->second != name)
				return false;
		}

		std::pair<NameToFlavor::iterator, bool> res2 = mNameToFlavor.insert(make_pair(name, id));

		if (!res2.second) {
			if (res2.first->second != id)
				return false;
		}

		std::pair<RelationIDMap::iterator, bool> res3 = mRelationIDMap.insert(make_pair(id, rel));

		if (!res3.second) { // failed to map the relation's instance to the ID
			if (res3.first->second != rel)
				return false;
		}

		rel->setID(id);

		return true;
	}

	//------------------------------------------------------
	RelationPtr LinkService::getRelation(const std::string& name) {
		RelationNameMap::iterator rnit = mRelationNameMap.find(name);

		if (rnit == mRelationNameMap.end())
			return RelationPtr(); // return NULL pointer
		else
			return rnit->second;
	}

	//------------------------------------------------------
	RelationPtr LinkService::getRelation(int flavor) {
		RelationIDMap::iterator rnit = mRelationIDMap.find(flavor);

		if (rnit == mRelationIDMap.end())
			return RelationPtr(); // return NULL pointer
		else
			return rnit->second;
	}
	
	//------------------------------------------------------
	void LinkService::objectDestroyed(int id) {
		RelationIDMap::iterator it = mRelationIDMap.begin();

		for (; it != mRelationIDMap.end();++it)
			it->second->objectDestroyed(id); // Will call the opposing relation ~ as well
	}
	
	//-------------------------- Factory implementation
	std::string LinkServiceFactory::mName = "LinkService";

	LinkServiceFactory::LinkServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& LinkServiceFactory::getName() {
		return mName;
	}

	Service* LinkServiceFactory::createInstance(ServiceManager* manager) {
		return new LinkService(manager, mName);
	}

	const uint LinkServiceFactory::getMask() {
	    return SERVICE_DATABASE_LISTENER | SERVICE_CORE;
	}

}
