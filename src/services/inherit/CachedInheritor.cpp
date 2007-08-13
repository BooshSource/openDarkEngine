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

#include "CachedInheritor.h"

using namespace std;

namespace Opde {
    /*---------------------------------------------------------*/
    /*--------------------- CachedInheritor -------------------*/
	/*---------------------------------------------------------*/
    CachedInheritor::CachedInheritor(InheritService* is) : mInheritService(is) {
        mInheritListener.listener = this;
        mInheritListener.method = (InheritChangeMethodPtr)(&CachedInheritor::onInheritMsg);
        mInheritService->registerListener(&mInheritListener);
    };

    //------------------------------------------------------
    CachedInheritor::~CachedInheritor() {
        mInheritService->unregisterListener(&mInheritListener);
    }

    //------------------------------------------------------
	void CachedInheritor::setImplements(int objID, bool impl) {
	    ImplementsMap::iterator it = mImplements.find(objID);
        bool modified = false;

        if (impl) {

            if (it != mImplements.end()) {
                if (!it->second) {
                    it->second = true; // just set the new value
                    modified = true;
                }
            } else {
               mImplements.insert(make_pair(objID, impl)); // insert if non-present
               modified = true;
            }

        } else { // false - only remove if present

            if (it != mImplements.end()) {
                mImplements.erase(it);
                modified = true;
            }

        }

        // Did we change something?
        if (modified) {
            refresh(objID); // refresh the object ID and all the inheritance targets
        }
	}

	//------------------------------------------------------
	bool CachedInheritor::getImplements(int objID) const {
	    ImplementsMap::const_iterator it = mImplements.find(objID);

        if (it != mImplements.end()) {
            return it->second;
        } else {
            // does not have a record, so does not implement
            return false;
        }
	}

    //------------------------------------------------------
    int CachedInheritor::getEffectiveID(int srcID) const {
        EffectiveObjectMap::const_iterator it = mEffObjMap.find(srcID);

        if (it != mEffObjMap.end()) {
            return it->second;
        } else
            return 0; // Can't be self, as that is also recorded
    }

    //------------------------------------------------------
    bool CachedInheritor::setEffectiveID(int srcID, int effID) {
        EffectiveObjectMap::iterator it = mEffObjMap.find(srcID);
        bool changed = false;

        if (it != mEffObjMap.end()) {
            if (it->second != effID) {
                it->second = effID;
                changed = true;
            }
        } else {
            mEffObjMap.insert(make_pair(srcID, effID));
            changed = true; // there was no record yet
        }

        return changed;
    }

    //------------------------------------------------------
    void  CachedInheritor::unsetEffectiveID(int srcID) {
        EffectiveObjectMap::iterator it = mEffObjMap.find(srcID);
        bool changed = false;

        if (it != mEffObjMap.end()) {
            mEffObjMap.erase(it);
        }
    }

    //------------------------------------------------------
    bool CachedInheritor::refresh(int srcID) {
        // First, get the old effective ID
        int oldEffID = getEffectiveID(srcID);

        // Let' vote for a new effective object ID
        InheritQueryPtr sources = mInheritService->getSources(srcID);

        int maxPrio = -1; // no inheritance indicator itself

        InheritLinkPtr effective(NULL);
        // now for each of the sources, find the one with the max. priority.
        // Some logic to accept the self assigned is also present
        while (!sources->end()) {
            InheritLinkPtr il = sources->next();

            // validate and compare to the maximal. If priority is bigger, we have a new winner
            if (validate(il->srcID, il->dstID, il->priority) && il->priority > maxPrio) {
                effective = il;
                maxPrio = il->priority;
            }
        }


        /* Let's list possible situations:
        1. Object does not implement, and no inh. source does supply - removeEffectiveID record then!
        2. Object does not implement, but inh. source does supply - add effective ID from the source with max. priority
        3. Object does implement and:
            a/ No source supplies - effective ID is the objects self ID
            b/ Source supplies
                I/ Source supplies max priority 0 - effective ID is self obj. ID
                II/ Source supplies priority >0 - effective ID is inherited from the loudest source (max prio.)
        */

        int newEffID = 0;

        if (getImplements(srcID)) {
            // If there is no source supply, set self. Otherwise prio decides
            if (!effective.isNull() && maxPrio > 0) {
                newEffID = getEffectiveID(effective->srcID);
            } else
                newEffID = srcID;
        } else {
            // no self implementation. choose the max source if even present
            if (!effective.isNull())
                newEffID = getEffectiveID(effective->srcID);
        }

        if (newEffID != 0)
            setEffectiveID(srcID, newEffID);
        else
            unsetEffectiveID(srcID);

        // If there was a change, propagate
        if (newEffID != oldEffID) {
            InheritQueryPtr targets = mInheritService->getTargets(srcID);

            InheritLinkPtr effective(NULL);

            while (!targets->end()) {
                InheritLinkPtr il = sources->next();

                refresh(il->dstID); // refresh the target object
            }
        }
    }

    //------------------------------------------------------
    bool CachedInheritor::validate(int srcID, int dstID, unsigned int priority) {
        return true;
    }

    //------------------------------------------------------
    void CachedInheritor::onInheritMsg(const InheritChangeMsg& msg) {
        // Received an even about inheritance change. Must refresh target object of such change
        refresh(msg.dstID);
    }

    //------------------------------------------------------
    void CachedInheritor::clear() {
        mEffObjMap.clear();
        mImplements.clear();
    }

    //------------------------------------------------------- Cached Inheritor Factory:
    string CachedInheritorFactory::mName = "always";

    CachedInheritorFactory::CachedInheritorFactory() {
    }

	string CachedInheritorFactory::getName() const {
	    return mName;
	}

	InheritorPtr CachedInheritorFactory::createInstance(InheritService* is) const {
	    return new CachedInheritor(is);
	}
}

