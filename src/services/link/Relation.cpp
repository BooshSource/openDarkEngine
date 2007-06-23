/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2007 openDarkEngine team
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place - Suite 330, Boston, MA 02111-1307, USA, or go to
 * http://www.gnu.org/copyleft/lesser.txt.
 *****************************************************************************/
 
#include "integers.h"
#include "BinaryService.h"
#include "Relation.h"
#include "DTypeDef.h"
#include "LinkCommon.h"

#include "logger.h"

using namespace std;

namespace Opde {
	
	// --------------------------------------------------------------------------
	Relation::Relation(const std::string& name, DTypeDefPtr type, bool hidden) : 
			mID(-1), 
			mName(name), 
			mType(type), 
			mHidden(hidden), 
			mLinkMap(),
			mSrcDstLinkMap(),
			mDstSrcLinkMap() {
			
		// clear out the maximal ID info
		for (int i = 0; i < 16; ++i) {
			mMaxID[i] = 0;
		}
		
		if (mType.isNull())
				mFakeSize = 0;
			else
				mFakeSize = mType->size();
		
		// Some default version values
		mLCVMaj = 2;
		mLCVMin = 0;
		mDCVMaj = 2;
		mDCVMin = 0;
	}
	
	// --------------------------------------------------------------------------
	Relation::~Relation() {
		// deletion of Relation instance will not cause a messagging havok, as with only deleting a single Link
		clear();
	}

	// --------------------------------------------------------------------------
	void Relation::load(FileGroup* db) {
		// load the links and thei're data
		BinaryServicePtr bs = ServiceManager::getSingleton().getService("BinaryService").as<BinaryService>();
		
		// Link chunk name
		string lchn = "L$" + mName;
		string ldchn = "LD$" + mName;
		
		FilePtr flink = NULL;
		FilePtr fldata = NULL;

		// let's open the L$NAME and LD$NAME files in the db
		try {
			flink = db->getFile(lchn);
		} catch (BasicException& e) {
			LOG_FATAL("Relation::load : Could not find the Link chunk %s", lchn.c_str());
			return;
		}
		
		// Link data file:
		try {
			fldata = db->getFile(ldchn);
		} catch (BasicException& e) {
			if (mType.isNull()) {
				LOG_INFO("Relation::load : Link data chunk %s (It's ok since data type not registered either)", ldchn.c_str());
			} else {
				LOG_FATAL("Relation::load : Could not find the Link data chunk %s with : %s", ldchn.c_str(), e.getDetails().c_str());
				return;
			}
		}
		
		DTypeDefPtr linkstruct = bs->getType("links", "LINK");
		
		size_t lsize = linkstruct->size();
		
		// now load the data
		size_t link_count = flink->size() / lsize;
		
		// calculate what the size of the data should be, based on the link count and the Link data file size
		size_t real_dsize = 0;
		if (link_count > 0) {
			real_dsize = (fldata->size() - sizeof(uint32_t)) / link_count;
		}
		
		// if the chunk LD exists, and contains at least the data size, load the data size, and set to load data as well
		bool load_data = false;
		size_t dsize = 0;
		
		if (fldata->size() > sizeof(uint32_t)) {
			fldata->readElem(&dsize, sizeof(uint32_t));
			
			if (mType.isNull())
				LOG_FATAL("Data for relation %s exist, but dyntype not set", mName.c_str()); // Maybe I just should stop with exception
			else {
				load_data = true;
			
				// check for data len
				if (dsize != mType->size()) {
					LOG_FATAL("Data size for relation %s differ : Type: %d, Chunk: %d", mName.c_str(), mType->size(), dsize);
					// we respect our data size
					dsize = mType->size();
				}
				
				/* Too often to fill log with. Seems not to matter (?!) - the question is why the chunks are often bigger in size than expected
				if (dsize != real_dsize && real_dsize != 0) {
					LOG_ERROR("Data size for relation %s differ : Calculated: %d, Chunk info: %d (DType: %d)", mName.c_str(), real_dsize, dsize, mType->size());
				}
				*/
			}
		}
		
		for (int idx = 0; idx < link_count; idx++) {
			char* dlink = new char[lsize];
			
			flink->read(dlink, lsize);
			
			LinkPtr link = LinkPtr(new Link(
				linkstruct->get(dlink, "id").toUInt(),
				linkstruct->get(dlink, "src").toInt(),
				linkstruct->get(dlink, "dest").toInt(),
				linkstruct->get(dlink, "flavor").toUInt()
			));
			
			LOG_DEBUG("Relation: Read link [%s - %d] ID %d, from %d to %d (F,C,IX: %d, %d, %d)", 
				  mName.c_str(), 
				  mID, 
				  link->mID, 
				  link->mSrc, 
				  link->mDst,
				  LINK_ID_FLAVOR(link->mID),
				  LINK_ID_CONCRETE(link->mID),
				  LINK_ID_INDEX(link->mID)
				 );
			
			// Check if the flavor fits
			assert(LINK_ID_FLAVOR(link->mID) == mID);
			
			if (load_data) {
				assert(dsize > 0);
				
				// Will get deleted automatically once the LinkPtr is released...
				link->mData = new char[dsize];
				
				// NOTE: It does not happen that the link data would be variable length, but if it would, reading size, then data into the buffer would be needed. It could be fine to write some serializing helpers...
				fldata->read(link->mData, dsize); 
			}
			
			_addLink(link);
		}
	}
	
	// --------------------------------------------------------------------------
	// TODO: Save mask. Archetypal/non-archetypal/both binary encoded pattern
	void Relation::save(FileGroup* db, uint saveMask) {
		LOG_DEBUG("Relation::save starting");
		
		// load the links and thei're data
		BinaryServicePtr bs = ServiceManager::getSingleton().getService("BinaryService").as<BinaryService>();
		
		// Link chunk name
		string lchn = "L$" + mName;
		string ldchn = "LD$" + mName;
		
		FilePtr flnk = db->createFile(lchn, mLCVMaj, mLCVMin);
		FilePtr fldt = db->createFile(ldchn, mDCVMaj, mDCVMin);
		
		uint32_t dtsz = 0;
		
		if (!mType.isNull()) {
			uint32_t dtsz = mType->size();
			assert(dtsz > 0);
			
			fldt->writeElem(&mFakeSize, sizeof(uint32_t));
		}
		
		// No need to order those. If it shows up that it would be actually better, no problem sorting those by link_id_t
		DTypeDefPtr linkstruct = bs->getType("links", "LINK");
		
		char* lnkdta = linkstruct->create();
		
		// just write the links as they go, and write the data in parallel
		LinkMap::const_iterator it = mLinkMap.begin();
		
		for (; it != mLinkMap.end(); it++) {
			LinkPtr link = it->second;
			
			// Test against the link write mask
			int conc = LINK_ID_CONCRETE(link->mID);
			
			
			if (saveMask & 1 << conc) { // mask says save!
				// write according to the structure
				linkstruct->set(lnkdta, "id", link->mID);
				linkstruct->set(lnkdta, "src", link->mSrc);
				linkstruct->set(lnkdta, "dest", link->mDst);
				linkstruct->set(lnkdta, "flavor", link->mFlavor);
				
				
				flnk->write(lnkdta, linkstruct->size());
				
				if (!mType.isNull()) { // write the data into the data chunk
					fldt->write(lnkdta, dtsz);
				}
			}
		}
		
		delete lnkdta;
		
		LOG_DEBUG("Relation::save ended ok.");
	}
	
	// --------------------------------------------------------------------------
	void Relation::clear() {
		// first, broadcast that we're gonna erase
		LinkChangeMsg m;
	
		m.change = LNK_RELATION_CLEARED;
		m.linkID = 0; // all links, no meaning
	
		// Inform the listeners about the change of data
		broadcastLinkMessage(m);
		
		mLinkMap.clear();
		mSrcDstLinkMap.clear();
		mDstSrcLinkMap.clear();
	}
	
	
	// --------------------------------------------------------------------------
	link_id_t Relation::create(int from, int to) {
		char* data = NULL;
		
		// allocate the data if needed
		if (!mType.isNull()) {
			data = mType->create();
		}
		
		return create(from, to, data);
	}
	
	// --------------------------------------------------------------------------
	link_id_t Relation::create(int from, int to, char* data) {
		// Request an id. First let's see what concreteness we have
		unsigned int cidx = 0;
		
		if (from >= 0 || to >= 0)
			cidx = 1;
		
		link_id_t id = getFreeLinkID(cidx);
		
		LinkPtr newl = new Link(id, from, to, mID);
		
		newl->mData = data;
		
		// Last, insert the link to the database and notify
		_addLink(newl);
		
		return id;
	}
	
	// --------------------------------------------------------------------------
	void Relation::remove(link_id_t id) {
		// A waste I smell here. Maybe there will be a difference in Broadcasts later
		_removeLink(id);
	}
	
	// --------------------------------------------------------------------------
	void Relation::setLinkField(link_id_t id, const std::string& field, const DVariant& value) {
		LinkMap::iterator it = mLinkMap.find(id);
		
		if (it != mLinkMap.end()) {
			mType->set(it->second->mData, field, value);
			
			LinkChangeMsg m;
		
			m.change = LNK_CHANGED;
			m.linkID = id;
		
			// Inform the listeners about the change of data
			broadcastLinkMessage(m);
		} else {
			LOG_ERROR("Relation::setLinkData : Link %d was not found in relation %d", id, mID);
		}
	}
		
	// --------------------------------------------------------------------------
	DVariant Relation::getLinkField(link_id_t id, const std::string& field) {
		LinkMap::iterator it = mLinkMap.find(id);
		
		if (it != mLinkMap.end()) {
			return mType->get(it->second->mData, field);
		} else {
			LOG_ERROR("Relation::getLinkData : Link %d was not found in relation %d", id, mID);
			return DVariant();
		}
	}
	
	// --------------------------------------------------------------------------
	void Relation::setLinkData(link_id_t id, char* data) {
		LinkMap::iterator it = mLinkMap.find(id);
		
		if (it != mLinkMap.end()) {
			// swap the data
			char* odl = it->second->mData;
			it->second->mData = data;
			delete odl;
			
			LinkChangeMsg m;
		
			m.change = LNK_CHANGED;
			m.linkID = id;
		
			// Inform the listeners about the change of data
			broadcastLinkMessage(m);
		} else {
			LOG_ERROR("Relation::setLinkData : Link %d was not found in relation %d", id, mID);
		}
	}
	
	// --------------------------------------------------------------------------
	LinkQueryResultPtr Relation::getAllLinks(int src, int dst) const {
		// based on case of the query, return result
		assert(src != 0 || dst != 0 ); // No all-links query
		
		if (src == 0) { // all link sources
			LinkQueryResultPtr res = new LinkQueryResult();
			
			ObjectLinkMap::const_iterator r = mDstSrcLinkMap.find(dst);
			
			if (r != mDstSrcLinkMap.end()) {
				// all the components of the secondary map go into the set
				ObjectIDToLinks::const_iterator ri = r->second.begin();
			
				for (; ri != r->second.end(); ++ri) {
					res->insert(ri->second.begin(), ri->second.end());
				}
				
				return res;
			}
			
		} else if (dst == 0) { // all link destinations
			
			LinkQueryResultPtr res = new LinkQueryResult();
			
			ObjectLinkMap::const_iterator r = mSrcDstLinkMap.find(src);
			
			if (r != mSrcDstLinkMap.end()) {
				// all the components of the secondary map go into the set
				ObjectIDToLinks::const_iterator ri = r->second.begin();
			
				for (; ri != r->second.end(); ++ri) {
					res->insert(ri->second.begin(), ri->second.end());
				}
				
				return res;
			}
			
		} else { // both src and dst are nonzero
			LinkQueryResultPtr res = new LinkQueryResult();
			
			ObjectLinkMap::const_iterator r = mSrcDstLinkMap.find(src);
			
			if (r != mSrcDstLinkMap.end()) {
				// all the components of the secondary map go into the set
				ObjectIDToLinks::const_iterator ri = r->second.find(dst);
			
				if (ri != r->second.end())
					res->insert(ri->second.begin(), ri->second.end());
				
				
				return res;
			}
		}
		
	}
		
	// --------------------------------------------------------------------------
	LinkPtr Relation::getOneLink(int src, int dst) const {
		LinkQueryResultPtr res = getAllLinks(src, dst);
		
		// I also could just return the first even if there would be more than one, but that could lead to programmers headaches
		assert(res->size() > 1);
		
		if (res->size() > 0)
			return *res->begin();
		else
			return NULL;
	}
	
	// --------------------------------------------------------------------------
	// --------------------------------------------------------------------------
	void Relation::_addLink(LinkPtr link) {
		// Insert, and detect the presence of such link already inserted (same ID)
		std::pair<LinkMap::iterator, bool> ires = mLinkMap.insert(make_pair(link->mID, link));
		
		if (!ires.second) {
			LOG_ERROR("Relation: Found link with conflicting ID in relation %d (%s): ID: %d (stored %d) - link already existed", mID, mName.c_str(), link->mID, ires.first->second->mID );
		} else {
			// Update the free link info
			allocateLinkID(link->mID);
			
			// Update the query databases
			// Src->Dst->LinkList
			pair<ObjectLinkMap::iterator, bool> r = mSrcDstLinkMap.insert(make_pair(link->mSrc, ObjectIDToLinks()));
			pair<ObjectIDToLinks::iterator, bool> ri = r.first->second.insert(make_pair(link->mDst, LinkSet()));
			
			ri.first->second.insert(link);
			
			
			r = mDstSrcLinkMap.insert(make_pair(link->mDst, ObjectIDToLinks()));
			ri = r.first->second.insert(make_pair(link->mDst, LinkSet()));
			
			ri.first->second.insert(link);
			
			// fire the notification about inserted link
			LinkChangeMsg m;
		
			m.change = LNK_ADDED;
			m.linkID = link->mID;
		
			// Inform the listeners about the change
			broadcastLinkMessage(m);
		}
	}
	
	// --------------------------------------------------------------------------
	void Relation::_removeLink(link_id_t id) {
		LinkMap::iterator it = mLinkMap.find(id);
		
		if (it != mLinkMap.end()) {
			unallocateLinkID(id);
			
			LinkPtr to_remove = it->second;
			
			// Update the query databases
			// SrcDst
			ObjectLinkMap::iterator r = mSrcDstLinkMap.find(to_remove->mSrc);
			
			assert(r != mSrcDstLinkMap.end());
			
			ObjectIDToLinks::iterator ri = r->second.find(to_remove->mDst);
			
			assert(ri != mSrcDstLinkMap.end());
			
			ri->second.erase(to_remove);
			
			// DstSrc
			r = mSrcDstLinkMap.find(to_remove->mSrc);
			
			assert(r != mSrcDstLinkMap.end());
			
			ri = r->second.find(to_remove->mDst);
			
			assert(ri != mSrcDstLinkMap.end());
			
			ri->second.erase(to_remove);
			
			// fire the notification about inserted link
			LinkChangeMsg m;
		
			m.change = LNK_REMOVED;
			m.linkID = id;
		
			// Inform the listeners about the change
			broadcastLinkMessage(m);
			
			// last, erase the link
			mLinkMap.erase(it);
		} else {
			LOG_ERROR("Relation %d: Link requested for removal was not found :ID: %d", mID, id);
		} 
	}
	
	// --------------------------------------------------------------------------
	link_id_t Relation::getFreeLinkID(uint cidx) {
		link_id_t id = LINK_MAKE_ID(mID, cidx, mMaxID[cidx] + 1);
		allocateLinkID(id);
		return id;
	}
	
	// --------------------------------------------------------------------------
	void Relation::allocateLinkID(link_id_t id) {
		unsigned int cidx = LINK_ID_CONCRETE(id);
		
		assert(cidx < 16);
		
		link_id_t lidx = LINK_ID_INDEX(id);
		
		if (mMaxID[cidx] < lidx) {
			mMaxID[cidx] = lidx;
		}
	}
	
	// --------------------------------------------------------------------------
	void Relation::unallocateLinkID(link_id_t id) {
		unsigned int cidx = LINK_ID_CONCRETE(id);
		
		assert(cidx < 16);
		
		link_id_t lidx = LINK_ID_INDEX(id);
		
		// I could make this customisible via some constructor parameter (having index array and bool, iterating)
		if (mMaxID[cidx] == lidx) {
			mMaxID[cidx]--;
		}
	}
	
	// --------------------------------------------------------------------------
	void Relation::broadcastLinkMessage(const LinkChangeMsg& msg) const {
		LinkListeners::iterator it = mLinkListeners.begin();
		
		for (; it != mLinkListeners.end(); ++it) {
			// Call the method on the listener pointer 
			((*it)->listener->*(*it)->method)(msg);
		}
	}
	
	void Relation::registerListener(LinkChangeListenerPtr* listener) {
		mLinkListeners.insert(listener);
	}
	
	void Relation::unregisterListener(LinkChangeListenerPtr* listener) {
		mLinkListeners.erase(listener);
	}
}