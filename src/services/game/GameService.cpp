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


#include "GameService.h"
#include "OpdeException.h"

using namespace std;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- GameService -------------------*/
	/*----------------------------------------------------*/
	GameService::GameService(ServiceManager *manager, const std::string& name) : Service(manager, name) {
	    mDbService = ServiceManager::getSingleton().getService("DatabaseService").as<DatabaseService>();
	}

    //------------------------------------------------------
	bool GameService::init() {
	    return true;
	}

	//------------------------------------------------------
	GameService::~GameService() {
	}

	//------------------------------------------------------
	void GameService::load(const std::string& filename) {
		mDbService->load(filename);
	}

	//-------------------------- Factory implementation
	std::string GameServiceFactory::mName = "GameService";

	GameServiceFactory::GameServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& GameServiceFactory::getName() {
		return mName;
	}

	Service* GameServiceFactory::createInstance(ServiceManager* manager) {
		return new GameService(manager, mName);
	}

}
