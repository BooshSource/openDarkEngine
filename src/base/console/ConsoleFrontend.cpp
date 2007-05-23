/******************************************************************************
 *
 *    This file is part of openDarkEngine project
 *    Copyright (C) 2005-2006 openDarkEngine team
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
 
 
#include "ConsoleBackend.h"
#include "ConsoleFrontend.h"
#include <OIS.h>

using namespace Ogre;
using namespace OIS;

namespace Opde {
    
	ConsoleFrontend::ConsoleFrontend() : mIsActive(false) {
	    	mRoot = Root::getSingletonPtr();
		mOverlayMgr = OverlayManager::getSingletonPtr();
		
		mConsoleOverlay = OverlayManager::getSingleton().getByName("Opde/Console");
		mConsoleOverlay->hide();
		
		// prepare the handle to the Opde/CommandLine
		mCommandLine = OverlayManager::getSingleton().getOverlayElement("Opde/CommandLine");
		
		mConsoleText = OverlayManager::getSingleton().getOverlayElement("Opde/ConsoleText");
		
		mConsoleBackend = ConsoleBackend::getSingletonPtr();
	}
    
	ConsoleFrontend::~ConsoleFrontend() {
		mConsoleOverlay->hide();
		delete mConsoleOverlay;
	}
	
	void ConsoleFrontend::setActive(bool active) {
		mIsActive = active;
	}
	
	void ConsoleFrontend::injectKeyPress(const OIS::KeyEvent &e) {
		if (e.key == KC_RETURN) {
			ConsoleBackend::getSingleton().executeCommand(mCommand);
			mCommand = "";
		} else if (e.key == KC_BACK) {
			mCommand = mCommand.substr(0, mCommand.length()-1);
		} else if (e.key == KC_PGUP) {
			mConsoleBackend->scroll(-30);
		} else if (e.key == KC_PGDOWN) {
			mConsoleBackend->scroll(30);
		}
		else	
			mCommand += (char)e.text;
		
		mCommandLine->setCaption(">" + mCommand);
		
	}
	
	void ConsoleFrontend::update(int timems) {
		if (mIsActive) {
			mConsoleOverlay->show();
			if (mConsoleBackend->getChanged()) {
				// need to update the text window
				std::vector< Ogre::String > texts;
				mConsoleBackend->pullMessages(texts, 30);
				
				String text;
				
				std::vector< Ogre::String >::iterator it = texts.begin();
				
				for (;it != texts.end(); ++it) {
					text += *it + "\n";
				}
				
				mConsoleText->setCaption(text);
			}
		} else 
			mConsoleOverlay->hide();
		/*
		Real scrollY = mConsoleOverlay->_getTop();
		
		if (mIsActive && scrollY < 0) {
			scrollY += timems/1000;
			
			if (scrollY > 1) 
				scrollY = 1;
				
			mConsoleOverlay->setTop(scrollY);
		}
		
		if (!mIsActive && scrollY > -0.5) {
			scrollY -= timems/1000;
			
			if (scrollY < -0.5) 
				scrollY = -0.5;
				
			mConsoleOverlay->setTop(scrollY);
		}*/
	}
}
