/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2009 openDarkEngine team
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
 *
 *		$Id$
 *
 *****************************************************************************/


#ifndef __PLATFORMSERVICE_H
#define __PLATFORMSERVICE_H

#include "config.h"

#include "OpdeServiceManager.h"
#include "OpdeService.h"
#include "SharedPtr.h"

namespace Opde {
	// Forward decl.
	class Platform;

	/** @brief Platform service. Service that gives the engine an abstracted way to work with various OSes and File Systems they introduce.
	*/
	class OPDELIB_EXPORT PlatformService : public ServiceImpl<PlatformService> {
		public:
			PlatformService(ServiceManager *manager, const std::string& name);
			virtual ~PlatformService();
			
		protected:
			bool init();
			void bootstrapFinished();
			void shutdown();
			
		private:
			Platform *mPlatform;
	};

	/// Shared pointer to Platform service
	typedef shared_ptr<PlatformService> PlatformServicePtr;

	/// Factory for the PlatformService objects
	class OPDELIB_EXPORT PlatformServiceFactory : public ServiceFactory {
		public:
			PlatformServiceFactory();
			~PlatformServiceFactory() {};

			/** Creates a PlatformService instance */
			Service* createInstance(ServiceManager* manager);

			virtual const std::string& getName();

			virtual const uint getMask();
			
			virtual const size_t getSID();
		private:
			static std::string mName;
	};
}


#endif
