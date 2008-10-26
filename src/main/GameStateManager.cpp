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

#include "config.h"

#include "GameStateManager.h"
#include "OpdeException.h"
#include "logger.h"
#include "stdlog.h"
#include "filelog.h"

// All the services
#include "WorldRepService.h"
#include "BinaryService.h"
#include "GameService.h"
#include "ConfigService.h"
#include "LinkService.h"
#include "PropertyService.h"
#include "InheritService.h"
#include "RenderService.h"
#include "DatabaseService.h"
#include "InputService.h"
#include "LoopService.h"
#include "ObjectService.h"

#include "GameLoadState.h"
#include "GamePlayState.h"

#include "ProxyArchive.h"

// If custom codec is to be used
#include "CustomImageCodec.h"

#include <OgreRoot.h>
#include <OgreWindowEventUtilities.h>
#include <OgreConfigFile.h>

using namespace Ogre;

namespace Opde {

	// The instance owner
	template<> GameStateManager* Singleton<GameStateManager>::ms_Singleton = 0;

	GameStateManager::GameStateManager(std::string GameType) :
			mStateStack(),
			mTerminate(false),
			mLogger(NULL),
			mRoot(NULL),
			mStdLog(NULL),
			mConsoleBackend(NULL),
			mServiceMgr(NULL),
			mDTypeScriptLdr(NULL),
			mPLDefScriptLdr(NULL),
			mDirArchiveFactory(NULL),
			mCrfArchiveFactory(NULL),
			mConfigService(NULL) {
				mGameType = GameType;
	}

	GameStateManager::~GameStateManager() {
		if (!mInputService.isNull())
			mInputService->unsetDirectListener();
			
		mInputService.setNull();
		mConfigService.setNull();

		while (!mStateStack.empty()) {
			GameState* state = mStateStack.top();
			mStateStack.pop();

			state->exit();
		}

		delete mDTypeScriptLdr; // Unregisters itself
		mDTypeScriptLdr = NULL;

		delete mPLDefScriptLdr; // Unregisters itself
		mPLDefScriptLdr = NULL;

		// Delete the service manager
		delete mServiceMgr;
		mServiceMgr = NULL;

		CustomImageCodec::shutdown();

		delete mConsoleBackend;
		delete mRoot;

		delete mDirArchiveFactory;
		delete mCrfArchiveFactory;

		// Release the loggers
		delete mLogger;
		delete mStdLog;
        delete mFileLog;
	}

	GameStateManager& GameStateManager::getSingleton(void) {
		assert( ms_Singleton );  return ( *ms_Singleton );
	}

	GameStateManager* GameStateManager::getSingletonPtr(void) {
		return ms_Singleton;
	}

	//------------------ Main implementation ------------------
	void GameStateManager::terminate() {
		mTerminate = true;
	}

	void GameStateManager::pushState(GameState* state) {
		if (!mStateStack.empty()) {
			mStateStack.top()->exit();
		}

		mStateStack.push(state);

		state->start();
	}

	void GameStateManager::popState() {
		if (!mStateStack.empty()) {
			GameState* old = mStateStack.top();

			mStateStack.pop();

			old->exit();

			if (!mStateStack.empty()) { // There is another state in the stack
				// mStateStack.top()->start();
			} else {
				// mTerminate = true; // Terminate automatically if this state was the last?
			}
		} else {
			OPDE_EXCEPT("State stack was empty, nothing could be done.","StateManager::popState");
		}
	}


	bool GameStateManager::run() {
		// Initialize opde logger and console backend
		mLogger = new Logger();

		mStdLog = new StdLog();
        mFileLog = new FileLog("opde.log");

		mLogger->registerLogListener(mFileLog);

		LOG_INFO("Starting openDarkEngine %d.%d.%d (%s)\n", OPDE_VER_MAJOR, OPDE_VER_MINOR, OPDE_VER_PATCH, OPDE_CODE_NAME);

		// Create an ogre's root
		mRoot = new Root();
		
		mDirArchiveFactory = new Ogre::CaseLessFileSystemArchiveFactory();
		// TODO: mCrfArchiveFactory = new Ogre::CrfArchiveFactory();

		Ogre::ArchiveManager::getSingleton().addArchiveFactory(mDirArchiveFactory);
		// TODO: Ogre::ArchiveManager::getSingleton().addArchiveFactory(mCrfArchiveFactory);
		
		CustomImageCodec::startup();

		mConsoleBackend = new Opde::ConsoleBackend();

		// So console will write the messages from the logger
		mLogger->registerLogListener(mConsoleBackend);

		mConsoleBackend->putMessage("==Console Starting==");

		// Create the service manager
		mServiceMgr = new ServiceManager(SERVICE_ALL);
		assert(ServiceManager::getSingletonPtr() != 0);

		if (ServiceManager::getSingletonPtr() == 0)
			LOG_FATAL("Rotten tomatoes!");

		// Register the worldrep service factory
		registerServiceFactories();

		mConfigService = static_pointer_cast<ConfigService>(ServiceManager::getSingleton().getService("ConfigService"));
		
		mConfigService->loadParams("opde.cfg");
		
		RenderServicePtr rends;

		rends = static_pointer_cast<RenderService>(ServiceManager::getSingleton().getService("RenderService"));

		// Setup resources.
		setupResources();

		// Initialise resources
		ResourceGroupManager::getSingleton().initialiseAllResourceGroups();


        if (!mConfigService->hasParam("mission")) // Failback
		{
			if((mGameType == "SS2") || (mGameType == "ss2"))
				mConfigService->setParam("mission", "earth.mis");
			else
				mConfigService->setParam("mission", "miss1.mis");
		}

        // TODO: Remove this temporary nonsense. In fact. Remove the whole class this method is in!
        GamePlayState* ps = new GamePlayState();
        GameLoadState* ls = new GameLoadState();

		// Set default mipmap level (NB some APIs ignore this)
		TextureManager::getSingleton().setDefaultNumMipmaps(5);

		setupInputSystem();

        // TODO: Broadcast to all services: bootstrapFinished
        mServiceMgr->bootstrapFinished();

		ps->bootstrapFinished();

		// Push the initial state
		pushState(ls);

		// Run the game loop
		// Main while-loop
		unsigned long lTimeCurrentFrame = 0;

		while( !mTerminate ) {
			// Calculate time since last frame and remember current time for next frame
			mTimeLastFrame = lTimeCurrentFrame;
			lTimeCurrentFrame = mRoot->getTimer()->getMilliseconds();
			
			unsigned long lTimeSinceLastFrame = lTimeCurrentFrame - mTimeLastFrame;

            // Update current state
			mStateStack.top()->update( lTimeSinceLastFrame );

			// InputService::captureInputs()
			mInputService->captureInputs();

			// Render next frame
			mRoot->renderOneFrame();

			// Deal with platform specific issues
			Ogre::WindowEventUtilities::messagePump();
		}

        while (!mStateStack.empty()) {
			GameState* state = mStateStack.top();
			mStateStack.pop();

			state->exit();
		}

        delete ps;
        delete ls;

		return true;
	}

	void GameStateManager::registerServiceFactories() {
		// register the service factories
		new WorldRepServiceFactory();
		new BinaryServiceFactory();
		new GameServiceFactory();
		new ConfigServiceFactory();
		new LinkServiceFactory();
		new PropertyServiceFactory();
		new InheritServiceFactory();
		new RenderServiceFactory();
		new DatabaseServiceFactory();
		new InputServiceFactory();
		new LoopServiceFactory();
		new ObjectServiceFactory();
	}

	/// Method which will define the source of resources (other than current folder)
	void GameStateManager::setupResources(void) {
		// First, register the script loaders...

		// Allocate and register the DType script loader. Registers itself
		mDTypeScriptLdr = new DTypeScriptLoader();

		// Allocate and register the PLDef script loader. Registers itself
		mPLDefScriptLdr = new PLDefScriptLoader();

		// Load resource paths from config file
		ConfigFile cf;
		
		//Load the resources according to the game type, if game type not specified, load the default
		if((mGameType == "T1") || (mGameType == "t1"))
			cf.load("thief1.cfg");
		else if((mGameType == "T2") || (mGameType == "t2"))
			cf.load("thief2.cfg");
		else if((mGameType == "SS2") || (mGameType == "ss2"))
			cf.load("shock2.cfg");
		else
			cf.load("resources.cfg");
		
		// Go through all sections & settings in the file
		ConfigFile::SectionIterator seci = cf.getSectionIterator();

		String secName, typeName, archName;

		while (seci.hasMoreElements()) {
			secName = seci.peekNextKey();
			ConfigFile::SettingsMultiMap *settings = seci.getNext();
			ConfigFile::SettingsMultiMap::iterator i;

			for (i = settings->begin(); i != settings->end(); ++i) {
				typeName = i->first;
				archName = i->second;
				ResourceGroupManager::getSingleton().addResourceLocation(
					archName, typeName, secName);
			}
		}
	}

	void GameStateManager::setupInputSystem() {
		mInputService = static_pointer_cast<InputService>(ServiceManager::getSingleton().getService("InputService"));

		mInputService->createBindContext("game");
		mInputService->setBindContext("game");

		mInputService->setInputMode(IM_DIRECT);
		// Commented out for now, as it is not needed
		// mInputService->loadBNDFile("dark.bnd");

		mInputService->setDirectListener(this);
	}

	bool GameStateManager::keyPressed( const OIS::KeyEvent &e ) {
		if (!mStateStack.empty()) {
			mStateStack.top()->keyPressed(e);
		}
		return false;
	}

	bool GameStateManager::keyReleased( const OIS::KeyEvent &e ) {
		if (!mStateStack.empty()) {
			mStateStack.top()->keyReleased(e);
		}
		return false;
	}

	bool GameStateManager::mouseMoved( const OIS::MouseEvent &e ) {
		if (!mStateStack.empty()) {
			mStateStack.top()->mouseMoved(e);
		}
		return false;
	}

	bool GameStateManager::mousePressed( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
		if (!mStateStack.empty()) {
			mStateStack.top()->mousePressed(e, id);
		}
		return false;
	}

	bool GameStateManager::mouseReleased( const OIS::MouseEvent &e, OIS::MouseButtonID id ) {
		if (!mStateStack.empty()) {
			mStateStack.top()->mouseReleased(e, id);
		}
		return false;
	}

}
