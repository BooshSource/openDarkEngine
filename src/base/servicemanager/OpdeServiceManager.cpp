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
 
#include <iostream>

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "OpdeException.h"
#include "logger.h"

using namespace std;

namespace Opde {
	
	template<> ServiceManager* Singleton<ServiceManager>::ms_Singleton = 0;
		
	ServiceManager::ServiceManager() : mServiceFactories(), mServiceInstances() {
		
	}
	
	ServiceManager::~ServiceManager() {
		// Will release all services
		LOG_DEBUG("ServiceManager: Releasing all services");
		
		ServiceInstanceMap::iterator s_it = mServiceInstances.begin();
		
		for (; s_it != mServiceInstances.end(); ++s_it) {
			std::cerr << " * Delete of " << s_it->first << std::endl;
			s_it->second.setNull();
		}
		
		mServiceInstances.clear();
		LOG_DEBUG("ServiceManager: Services released");
		
		// delete all factories registered
		
		LOG_DEBUG("ServiceManager: Deleting all service factories");
		ServiceFactoryMap::iterator factory_it = mServiceFactories.begin();
		
		for (; factory_it != mServiceFactories.end(); ++factory_it) {
			delete factory_it->second;
		}
		
		mServiceFactories.clear();
		LOG_DEBUG("ServiceManager: Service factories deleted");
	}
	
	ServiceManager& ServiceManager::getSingleton(void) {
		assert( ms_Singleton );  return ( *ms_Singleton );  
	}
	
	ServiceManager* ServiceManager::getSingletonPtr(void) {
		return ms_Singleton;
	}
	
	
	//------------------ Main implementation ------------------
	void ServiceManager::addServiceFactory(ServiceFactory* factory) {
		mServiceFactories.insert(make_pair(factory->getName(), factory));
	}
	
	ServiceFactory* ServiceManager::findFactory(const std::string& name) {
		
		ServiceFactoryMap::const_iterator factory_it = mServiceFactories.find(name);
		
		if (factory_it != mServiceFactories.end()) {
			return factory_it->second;
		} else {
			return NULL;
		}
	}
	
	ServicePtr ServiceManager::findService(const std::string& name) {
		
		ServiceInstanceMap::iterator service_it = mServiceInstances.find(name);
		
		if (service_it != mServiceInstances.end()) {
			return service_it->second;
		} else {
			ServicePtr n(NULL);
			return n;
		}
	}
	
	ServicePtr ServiceManager::createInstance(const std::string& name) {
		ServiceFactory* factory = findFactory(name);
		
		LOG_DEBUG("ServiceManager: Service instance for %s not found, will create.", name.c_str());
		
		if (factory != NULL) { // Found a factory for the Service name
			ServicePtr ns = factory->createInstance(this);
			mServiceInstances.insert(make_pair(name, ns));
			return ns;
		} else {
			OPDE_EXCEPT(string("Factory named ") + name + string(" not found"), "OpdeServiceManager::getService");
		}
	}
	
	ServicePtr ServiceManager::getService(const std::string& name) {
		ServicePtr service = findService(name);
		
		if (!service.isNull()) 
			return service;
		else {
			return createInstance(name);
		}
	}
	
} 
