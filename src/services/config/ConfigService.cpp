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

#include "ConfigService.h"
#include "OpdeException.h"
#include "logger.h"

#include <OgreConfigFile.h>
#include <OgreException.h>

using namespace std;
using namespace Ogre;

namespace Opde {

	/*----------------------------------------------------*/
	/*-------------------- ConfigService -------------------*/
	/*----------------------------------------------------*/
	ConfigService::ConfigService(ServiceManager *manager) : Service(manager), mConfigFileName("opde.cfg") {
	}

	//------------------------------------------------------
	ConfigService::~ConfigService() {
	}

    //------------------------------------------------------
    void ConfigService::init() {
        DVariant acfg;

        // Reinitialize to the config_file parameter, if such is found
        if (false) { // TODO: Get a commandline parameter specifying the config file, or do the loading of this file externally while bootstrapping, as the first thing
            mConfigFileName = acfg;

            if (!loadParams(mConfigFileName)) {
                LOG_INFO("Did not find the alternate config file, defaulting to opde.cfg");
                mConfigFileName = "opde.cfg";
            }  else {
                return;
            }
        }

        if (!loadParams(mConfigFileName)) {
            LOG_FATAL("Did not find the opde.cfg file. Can be fatal!");
        }
    }


    //------------------------------------------------------
    void ConfigService::setParam(const std::string& param, const std::string& value) {
        Parameters::iterator it = mParameters.find(param);

        if (it != mParameters.end()) {
            it->second = value;
        } else {
            mParameters.insert(make_pair(param, value));
        }
    }

    //------------------------------------------------------
    DVariant ConfigService::getParam(const std::string& param) {
        Parameters::const_iterator it = mParameters.find(param);

        if (it != mParameters.end()) {
            return DVariant(it->second);
        } else {
            return DVariant();
        }
    }

    //------------------------------------------------------
    bool ConfigService::getParam(const std::string& param, DVariant& tgt) {
        Parameters::const_iterator it = mParameters.find(param);

        if (it != mParameters.end()) {
            tgt = it->second;
            return true;
        } else {
            return false;
        }
    }

    //------------------------------------------------------
    bool ConfigService::hasParam(const std::string& param) {
        Parameters::const_iterator it = mParameters.find(param);

        if (it != mParameters.end()) {
            return true;
        } else {
            return false;
        }
    }

    //------------------------------------------------------
    bool ConfigService::loadParams(const std::string& cfgfile) {
        try  {  // load a few options
            Ogre::ConfigFile cf;
            cf.load(cfgfile);

            // Get the iterator over values - no section
            ConfigFile::SettingsIterator it = cf.getSettingsIterator();


            while (it.hasMoreElements()) {
                std::string key = it.peekNextKey();
                std::string val = it.peekNextValue();

                setParam(key, val);

                it.moveNext();
            }

            return true;
        }
        catch (Ogre::Exception e) {
            LOG_ERROR("Config file '%s' was not found", cfgfile.c_str());
            return false;
            // Guess the file didn't exist
        }
    }
//-------------------------- Factory implementation
	std::string ConfigServiceFactory::mName = "ConfigService";

	ConfigServiceFactory::ConfigServiceFactory() : ServiceFactory() {
		ServiceManager::getSingleton().addServiceFactory(this);
	};

	const std::string& ConfigServiceFactory::getName() {
		return mName;
	}

	Service* ConfigServiceFactory::createInstance(ServiceManager* manager) {
		return new ConfigService(manager);
	}

}
