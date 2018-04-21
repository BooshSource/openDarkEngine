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
 *	  $Id$
 *
 *****************************************************************************/

#ifndef __OPDESERVICEMANAGER_H
#define __OPDESERVICEMANAGER_H

#include <memory>

#include "config.h"

#include "Array.h"
#include "OpdeService.h"
#include "OpdeServiceFactory.h"
#include "OpdeSingleton.h"
#include <map>

namespace Opde {

/** Central manager for the Services. Each service must have the ServiceFactory
 * implemented, and must implement Service class.
 * @see ServiceFactory
 * @see Service
 * @note Two phases exist in the service manager. Bootstrap and normal run.
 * Bootstrap phase is used to initialize services without any dependencies
 * @note Services are guaranteed to receive calls in this order: Constructor,
 * init(), bootstrapFinished()
 * @todo Introduce a quicker way of getting service - use service IDs instead of
 * Names
 */
class ServiceManager : public Singleton<ServiceManager>,
                       public NonCopyable {
private:
    typedef SimpleArray<ServiceFactory *> ServiceFactoryMap;
    typedef SimpleArray<ServicePtr> ServiceInstanceMap;

    ServiceFactoryMap mServiceFactories;
    ServiceInstanceMap mServiceInstances;

    bool mBootstrapFinished;

    ServiceFactory *findFactory(size_t sid);
    ServicePtr findService(size_t sid);
    ServicePtr createInstance(size_t sid);

    const uint mGlobalServiceMask;

    static const uint msMaxServiceSID;

public:
    ServiceManager(uint serviceMask);

    /// Destructor. Deletes all registered factories
    ~ServiceManager(void);

    // Singleton related
    static ServiceManager &getSingleton(void);
    static ServiceManager *getSingletonPtr(void);

    /** Registration for the services */
    void addServiceFactory(ServiceFactory *factory);

    /** Returns the service, named name, pointer.
     * @param name The service type name (The name returned by the
     * ServiceFactory)
     * @see ServiceFactory
     */
    ServicePtr getService(size_t sid);

    /** Mass creation of services. All services which will have nonzero binary
     * and operation with the factorie's service mask will be created. Typical
     * usage: Creation of listeners
     * @param mask The mask to use */
    void createByMask(uint mask);

    /** Inform the services that bootstraping has finished
     */
    void bootstrapFinished();
};

// Shortcut to service getter... Once all services are given a unique ID as
// well, we can redo this to getServiceByID(ID_##__sname__) or alike
#define GET_SERVICE(__sname__)                                                 \
    std::static_pointer_cast<__sname__>(                                       \
        ServiceManager::getSingleton().getService(__sname__::SID));

} // namespace Opde

#endif
